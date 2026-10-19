# haichi

haichi is a native Haiku utility for placing Desktop icons at exact pixel
coordinates and saving named Desktop layouts.

The current implementation includes the Phase 1 command-line application and
the first Phase 2 graphical interface described in the design document. It
provides one shared engine and an attribute backend that follows Tracker's
current `PoseInfo` format.

The resource temporarily uses `B_MULTIPLE_LAUNCH` so independent terminal
commands continue to work while Phase 2 command forwarding is implemented.

Tracker does not reload an existing pose's location for an ordinary attribute
notification. After a successful write, haichi asks the Desktop pose view to
remove and recreate only that in-memory pose. This makes the new coordinates
visible without renaming the file or restarting Tracker.

## Build

Build on Haiku with the system makefile engine:

```sh
make
```

The tracked HVIF application icon is ready to build. After changing its SVG
master, regenerate it on Haiku with `scripts/regenerate-icon.sh` (install
`hvif_tools` first).

Run the automated checks on Haiku:

```sh
make check
```

Prepare release checks and refresh the README screenshot from the real Haiku
window:

```sh
make prepare-release
```

The command leaves haichi open for visual inspection and writes
`docs/images/haichi.png`. Review and commit that generated image with every
release.

## Screenshot

![haichi main window](docs/images/haichi.png)

## Usage

Launch `haichi` without arguments to open the graphical interface and install
the Deskbar item. Select a Desktop icon, then click or drag the pinpointer over
the monitor preview—or enter X and Y coordinates directly—and choose Apply.
The preview grid shows the relative placement of every positioned Desktop
icon. Clicking an icon square selects that exact icon before dragging it, and
the selected icon's name remains visible above the preview. Closing the window
hides it; the Deskbar item can reopen it.

The command-line interface remains available:

```text
haichi list
haichi move "Notes.txt" 120 340
haichi move "Notes.txt" -120 -40
haichi save work
haichi restore work
haichi help
```

`haichi list` reports each positioned icon as X, Y, and name.

Layouts are flattened `BMessage` files stored in
`~/config/settings/haichi/`. Restoring a layout scales positions when the
current screen dimensions differ from the saved dimensions. Missing Desktop
items are reported and skipped. Exact negative coordinates are preserved;
coordinates outside Tracker's safe range of -50000 to 50000 are rejected.

haichi currently handles entries in the Desktop directory. Tracker's special
volume and Trash poses are intentionally outside the Phase 1 scope.

## Status

- Phase 0: Tracker pose format confirmed against current Haiku source; live
  stable/nightly validation remains.
- Phase 1: engine and command-line interface implemented and live-tested on
  Haiku; undo remains.
- Phase 2: Deskbar item and graphical X/Y placement foundation implemented;
  anchors, multi-selection, undo, and layout management UI remain.
- Phase 3 onward: expanded layout UI and optional placement features remain.

Version: v0.13
