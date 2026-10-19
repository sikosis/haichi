# Design Document: haichi, a Deskbar Icon Placer for Haiku OS

Oct 19, 2026 · @Sikosis

## 1. Overview

haichi is a native Haiku Deskbar application that moves chosen Desktop icons to exact coordinates, by name or from a saved layout, without dragging them by hand.

**Deskbar app.** haichi lives in the Deskbar tray as a small icon. It runs in the background, starts with the system, and opens its main window only when asked. Clicking the tray icon gives quick actions (restore a saved layout, save the current one); the window gives exact X and Y control.

**Problem.** Tracker lets you drag icons and offers "Clean up", but has no way to say "put this icon at x=120, y=340" or to restore a known arrangement after a resolution change, a new workspace or a reinstall.

**Goals**

- Move one or many Desktop icons to specific (x, y) pixel positions.
- Save and restore named layouts (for example "work" and "presentation").
- Work from a GUI and from the command line, using one shared engine.
- Make Tracker redraw the icons right away, with no reboot or restart.

**Non-goals (v1)**

- Moving icons in ordinary folder windows (the design allows it later, see section 6).
- Replacing Tracker, or changing its icon grid or snapping behaviour.
- Supporting BeOS R5 / Zeta; the target is current Haiku (R1/beta series and nightly builds).

**Names.** App name: `haichi`. Signature: `application/x-vnd.Sikosis-haichi`. Binary: `haichi`, which also acts as the command-line tool when started with arguments (`haichi move ...`). Deskbar item name: `haichi`. App type: background app with a Deskbar tray replicant.

## 2. How Haiku stores Desktop icon positions

The Desktop is a normal folder, `/boot/home/Desktop` (found with `find_directory(B_DESKTOP_DIRECTORY)`), and Tracker keeps each icon's position as a file attribute on the icon's own node, so a position can be changed by writing that attribute.

**Facts the design relies on**

- Every file, folder and link on the Desktop is a node on a BFS volume with extended attributes (BFS is Haiku's native filesystem).
- Tracker records a pose (icon placement) as attributes in the `_trk/` namespace: `_trk/pinfo` / `_trk/pinfo_le` (basic pose info) and `_trk/xtpinfo` / `_trk/xtpinfo_le` (extended info including screen-frame locations). The native-endian name is selected at compile time.
- A pose holds a location (a `BPoint`, in pixels from the top-left of the Desktop), plus the directory it belongs to, so the same file can sit at different spots in different folders.
- Tracker watches nodes with the node monitor. An attribute change refreshes displayed metadata, but current Tracker does **not** reload an existing pose's location from `_trk/pinfo`; immediate movement therefore needs an explicit pose refresh.
- Volume icons (the boot disk, mounted drives) are not files in the Desktop folder. Tracker stores their positions separately, so they need their own handling (see section 6).

**Two ways to move an icon**

| Approach | How | Pros | Cons |
| --- | --- | --- | --- |
| A. Write the attribute | `BNode::WriteAttr()` on the pose attribute, then refresh that pose in Tracker | Simple persistence, works for scripts | Depends on Tracker's private struct layout and refresh behaviour; may differ across Haiku versions |
| B. Ask Tracker | Send a `BMessage` to the Tracker app (`application/x-vnd.Be-TRAK`) through `BMessenger`, using its scripting suite or a pose-move message | Survives layout changes in Tracker | Current Tracker scripting explicitly lists pose location as a TODO; there is no supported exact-placement property |

**Decision:** implement A behind an interface (`IconPositionBackend`). After a verified write, obtain the Desktop window through Tracker's `Folder` scripting property and send a synthetic remove/create node-monitor pair to its `Poses` handler. This recreates only the in-memory pose and makes it read the new location without renaming the file or restarting Tracker. Keep this private interaction isolated in the backend and test it on stable and nightly Haiku. The rest of the app never touches attributes directly.

**Phase 0 source audit (completed 2026-10-19; live Haiku spike pending)**

1. The Haiku user guide confirms `_trk/pinfo_le` as the little-endian pose attribute.
2. Current Tracker source (`Attributes.h`, `Utilities.h`, `PoseView.cpp`, and `PoseViewScripting.cpp`) confirms the names, fields, byte-order handling and absence of a pose-location scripting property.
3. `PoseInfo` is the target ABI's native layout of `bool fInvisible`, `ino_t fInitedDirectory`, and `BPoint fLocation`. Tracker writes `sizeof(PoseInfo)` as `B_RAW_TYPE`; the parent directory inode must match for the location to be accepted.
4. Desktop extended poses use `ExtendedPoseInfo` in `_trk/xtpinfo[_le]`, with a variable list of `BPoint`/`BRect` frame locations. Phase 1 clears this cache after an external move so a stale resolution-specific position cannot undo it; later UI work can preserve and update each frame record.
5. The source audit disproved the original redraw assumption: `B_ATTR_CHANGED` does not update `BPose::fLocation`. The Phase 1 backend therefore includes the targeted in-memory pose refresh described above. Live validation must still be performed on Haiku hardware or a VM.

## 3. Architecture

The Deskbar icon, the main window and the CLI all call one engine library, which reaches Tracker through a swappable position backend.

&#91;embedded content: architecture · 4 layers, 2 interchangeable backends\]

Read top to bottom: front ends, engine, backends, then the Haiku system. Backend A writes the icon's attribute and Tracker notices through its node monitor; Backend B asks Tracker directly.

**Responsibilities**

- **Deskbar icon, window and CLI:** parse input, show results, never touch attributes.
- **Engine:** list icons, validate and clamp positions, keep the undo stack, load and save layouts.
- **Backends:** the only code that knows how Tracker stores or accepts positions.
- **Layout store:** flattened `BMessage` files in the app's settings folder.

## 4. Core API and code sketches

The engine is a small C++ library using only the Haiku Storage Kit and Application Kit, so the GUI and the CLI share it. The sketches below show intent, not final code, and the attribute name and struct must be replaced with the values confirmed in Phase 0.

**Kits used**

| Kit / class | Purpose |
| --- | --- |
| `BPath`, `find_directory()` | Locate the Desktop folder |
| `BDirectory`, `BEntry` | List Desktop entries, resolve names |
| `BNode` | Read and write attributes (`ReadAttr`, `WriteAttr`) |
| `BNodeInfo` | Icon and MIME type for the list view |
| `BNodeMonitor` / `watch_node()` | Detect external changes while the app runs |
| `BMessenger` | Talk to Tracker (backend B) |
| `BApplication`, `BWindow`, `BView`, `BListView` | GUI |
| `BMessage` | Save and load layouts (`Flatten()` to a file) |

**Position backend interface**

```cpp
struct IconItem {
    BString  name;        // file name on the Desktop
    entry_ref ref;
    BPoint   location;    // current pixel position
};

class IconPositionBackend {
public:
    virtual ~IconPositionBackend() {}
    virtual status_t ListIcons(BObjectList<IconItem>& out) = 0;
    virtual status_t GetLocation(const entry_ref&, BPoint* out) = 0;
    virtual status_t SetLocation(const entry_ref&, BPoint where) = 0;
};
```

**Backend A: write the attribute (sketch)**

```cpp
status_t AttrBackend::SetLocation(const entry_ref& ref, BPoint where)
{
    BNode node(&ref);
    status_t err = node.InitCheck();
    if (err != B_OK)
        return err;

    // PoseInfo layout confirmed in Phase 0 (placeholder shown)
    PoseInfo info;
    ssize_t n = node.ReadAttr(kAttrPoseInfo, B_RAW_TYPE, 0, &info, sizeof(info));
    if (n != (ssize_t)sizeof(info))
        memset(&info, 0, sizeof(info));

    info.fInitedDirectory = DesktopNodeID();   // inode of the Desktop folder
    info.fLocation = where;
    info.fInvisible = false;

    n = node.WriteAttr(kAttrPoseInfo, B_RAW_TYPE, 0, &info, sizeof(info));
    return n == (ssize_t)sizeof(info) ? B_OK : (status_t)n;
}
```

**Listing Desktop icons**

```cpp
BPath path;
find_directory(B_DESKTOP_DIRECTORY, &path);
BDirectory dir(path.Path());
entry_ref ref;
while (dir.GetNextRef(&ref) == B_OK) {
    BPoint p;
    if (backend->GetLocation(ref, &p) == B_OK)
        items.AddItem(new IconItem(ref, p));
}
```

**Layout file format**

A layout is a flattened `BMessage` saved under `/boot/home/config/settings/haichi/<name>.layout`. Each icon is stored by file name, with a fallback of its MIME type and size so renames can be detected.

```cpp
BMessage layout('LAYT');
layout.AddString("name", "My Layout");
layout.AddInt32("screen_w", frame.IntegerWidth() + 1);
layout.AddInt32("screen_h", frame.IntegerHeight() + 1);
for (auto* it : items) {
    BMessage entry;
    entry.AddString("file", it->name);
    entry.AddPoint("loc", it->location);
    layout.AddMessage("icon", &entry);
}
BFile f(layoutPath, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
layout.Flatten(&f);
```

**Command-line interface**

```
haichi list
haichi move "Notes.txt" 120 340
haichi save work
haichi restore work
```

Exit codes: 0 success, 1 bad arguments, 2 icon not found, 3 write failed.

**Deskbar replicant**

haichi is a background app (resource flags `B_BACKGROUND_APP` and `B_SINGLE_LAUNCH`), so it has no window or team-menu entry while idle. It puts a small view into the Deskbar tray. The view is thin: it draws the icon and forwards clicks to the app with `BMessenger`.

```cpp
// TrayView.h: the view Deskbar hosts
class TrayView : public BView {
public:
    TrayView(BRect frame);
    TrayView(BMessage* archive);
    static TrayView* Instantiate(BMessage* archive);
    status_t Archive(BMessage* into, bool deep = true) const;
    void Draw(BRect update);        // draws the vector icon scaled to the frame
    void MouseDown(BPoint where);   // left: open window, right: popup menu
};

// Exported so Deskbar can load the view from the app binary
extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth, float maxHeight);

// In the app, after startup
status_t AddTrayIcon()
{
    BDeskbar deskbar;
    if (deskbar.HasItem("haichi"))
        return B_OK;
    app_info info;
    be_app->GetAppInfo(&info);
    int32 id;
    return deskbar.AddItem(&info.ref, &id);
}

// On quit
BDeskbar().RemoveItem("haichi");
```

The tray view sends `BMessage`s to `BMessenger("application/x-vnd.Sikosis-haichi")`. If the app is not running, the view starts it with `be_roster->Launch()`. The supplied current API confirms `BDeskbar::AddItem(BView*, int32*)` and `BDeskbar::AddItem(entry_ref*, int32*)`; the replicant export will be compiled and tested on the target release during Phase 2.

## 5. User interface

haichi lives in the Deskbar tray and opens one main window on demand. The window follows the Haiku Human Interface Guidelines: a list of Desktop icons on the left, a placement panel on the right, and a menu bar on top.

**Deskbar tray icon**

| Action | Result |
| --- | --- |
| Left-click | Open the main window, or bring it to the front if open |
| Right-click | Menu: Open haichi, Restore layout (submenu of saved names), Save current layout, About, Quit |
| Hover | Tooltip with the number of Desktop icons and the last restored layout |
| Quit (menu) | Removes the tray icon and stops the background app |

Autostart uses a user launch entry so the icon is present after login (format and location to confirm on the target release).

**Main window**

| Area | Control | Behaviour |
| --- | --- | --- |
| Menu bar | File, Layout, Help | File: Refresh, Quit. Layout: Save as, Restore, Delete. |
| Left | `BListView` of icons (small icon + name + current X, Y) | Multi-select; selection updates the right panel |
| Right, top | X and Y `BSpinner` fields (pixels) | Pre-filled from the selected icon; Apply moves it |
| Right, middle | Anchor picker: nine buttons (top-left, top-right, centre, ...) | Fills X and Y from the screen size, with a margin field |
| Right, bottom | Buttons: Apply, Revert, Align column / row | Align spreads several selected icons with a chosen spacing |
| Bottom | Status bar (`BStringView`) | "Moved 3 icons" or the error text |

**Placement by mouse (optional, v1.1)**

A preview pane shows a scaled screen outline; dragging an icon there sets X and Y. Placement still goes through the same `SetLocation()` call.

**Messages (`BMessage` what-codes)**

| Code | Sent by | Handled by |
| --- | --- | --- |
| `'refr'` (refresh list) | Menu, node monitor | MainWindow reloads icons |
| `'sele'` (selection changed) | List view | Updates the panel |
| `'appl'` (apply position) | Apply button | Calls backend `SetLocation()` per selected icon |
| `'save'` / `'rest'` | Layout menu | Layout store writes or reads the file |
| `B_NODE_MONITOR` | System | Refreshes the row whose attribute changed |

**Threading.** The UI runs in the window thread. Moving a large layout (more than about 50 icons) runs in a worker `BLooper` and posts progress back, so the window stays responsive.

**Haiku details**

- Use `BLayoutBuilder` (`BGroupLayout`, `BGridLayout`) so the window scales with the user's font size.
- Strings go through `B_TRANSLATE()` with a catalog so the app can be localised.
- Provide an HVIF vector icon and a `.rdef` resource file (signature, version, supported types).

## 6. Edge cases, risks, testing and roadmap

**Edge cases**

| Case | Handling |
| --- | --- |
| Position off-screen | Preserve exact coordinates, including negative coordinates, but reject values outside Tracker's safe range of -50000 to 50000 |
| Icon has no pose attribute yet | Create it; set the directory field to the Desktop node ID |
| Two icons placed on the same spot | Warn; offer "nudge to nearest free cell" |
| Multiple workspaces | Positions may be stored per workspace in the extended info; v1 edits the current workspace only |
| Different screen resolution on restore | Scale saved positions by the width and height ratio, or restore absolute positions (user option) |
| File renamed or deleted since save | Skip and list it in a result dialog; never fail the whole restore |
| Volume icons (boot disk, drives) | Not Desktop files; v1 skips them and lists them as unsupported |
| Read-only or non-BFS volume | `WriteAttr` fails; show the error and leave the icon untouched |
| Tracker not running | Backend A still works; the new position shows on the next Tracker start |

**Deskbar-specific cases**

- **Deskbar restarts** (crash or restart of the desktop): the tray item disappears, so the app watches with `be_roster->StartWatching()` and adds it again when Deskbar launches.
- **haichi quits or crashes:** the replicant could stay as a dead icon. On click it tries to launch the app and, if that fails, removes itself with `BDeskbar::RemoveItem()`.
- **Second launch:** `B_SINGLE_LAUNCH` makes the second start just open the existing window.
- **Different tray size or Deskbar position:** scale the vector icon to the view frame instead of using a fixed bitmap.
- **Theme or colour changes:** use `ui_color()` constants so the icon background matches the tray.

**Risks**

- **Private struct layout changes in Tracker.** Mitigate with the backend interface, a startup self-test (read, write, read back, restore) and a version check that disables writing if the test fails.
- **Race with Tracker.** If Tracker rewrites the attribute while the app does, the last write wins. Use a short retry and re-read after writing to verify.
- **Data loss.** Always keep a one-step undo: store the previous position of every moved icon in memory and offer Edit > Undo.

**Testing**

- Unit tests for the layout store (save, load, scale) with `CppUnit` or plain assertions, run on Haiku or in the Haiku VM.
- A manual checklist: move one icon, move ten, restore a layout, change resolution then restore, rename a file then restore, run the CLI over SSH.
- Test on the current stable release and a recent nightly build in a VM (QEMU or VirtualBox).

**Build and packaging**

Use a simple `Makefile` based on Haiku's `makefile-engine`, or Jam in the Haiku source tree. Package as an `.hpkg` with a `.PackageInfo` file, with the app in `apps/` and the CLI in `bin/`.

**Roadmap**

| Phase | Deliverable | Done when |
| --- | --- | --- |
| 0 | Spike: confirm attribute layout and Tracker redraw | One icon moves from a test program |
| 1 | Engine library plus `haichi CLI mode` (list, move) | CLI moves icons reliably, with undo |
| 2 | Deskbar tray icon, then main window: list, X and Y fields, anchors, status bar | Matches section 5 |
| 3 | Layouts: save, restore, scaling | Layout survives a resolution change |
| 4 | Preview-pane dragging, folder windows, per-workspace positions, volume icons | Optional extras |

**Open questions**

- Does Tracker accept a scripting message for exact pose placement (backend B)?
- Where exactly are volume icon positions stored on the current release?
- Is the pose attribute name the same on the target release and on nightly builds?
