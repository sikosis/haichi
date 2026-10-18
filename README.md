# haichi

haichi is a native Haiku utility for placing Desktop icons at exact pixel
coordinates and saving named Desktop layouts.

The current implementation is the Phase 1 command-line application described
in the design document. It provides one shared engine and an attribute backend
that follows Tracker's current `PoseInfo` format.

The Phase 1 resource uses `B_MULTIPLE_LAUNCH` so independent terminal commands
work normally. Phase 2 will change the installed background application to
`B_SINGLE_LAUNCH` and forward command requests to it.

Tracker does not reload an existing pose's location for an ordinary attribute
notification. After a successful write, haichi asks the Desktop pose view to
remove and recreate only that in-memory pose. This makes the new coordinates
visible without renaming the file or restarting Tracker.

## Build

Build on Haiku with the system makefile engine:

```sh
make
```

Run the automated checks on Haiku:

```sh
make check
```

## Usage

```text
haichi list
haichi move "Notes.txt" 120 340
haichi save work
haichi restore work
haichi help
```

Layouts are flattened `BMessage` files stored in
`~/config/settings/haichi/`. Restoring a layout scales positions when the
current screen dimensions differ from the saved dimensions. Missing Desktop
items are reported and skipped.

haichi currently handles entries in the Desktop directory. Tracker's special
volume and Trash poses are intentionally outside the Phase 1 scope.

## Status

- Phase 0: Tracker pose format confirmed against current Haiku source; live
  stable/nightly validation remains.
- Phase 1: engine and command-line interface foundation implemented; undo,
  Haiku build, and live tests remain.
- Phase 2 onward: Deskbar replicant, graphical window, undo UI, and layout
  management UI remain to be implemented.

Version: v0.01
