# Changelog

## v0.12 - 2026-10-20

- Add a Haiku/BeOS-inspired monitor-grid and pinpointer application icon.
- Embed the native HVIF icon for Tracker, the About box, and the Deskbar tray.
- Add a repeatable SVG-to-HVIF regeneration script.

## v0.11 - 2026-10-20

- Give the Desktop icon list a guaranteed visible pane in a larger window.
- Display the selected icon name prominently above the monitor preview.
- Make clicking or dragging an icon marker select that exact icon first.
- Preserve the grab offset while dragging so markers do not jump under the
  pointer.
- Label the preview's X and Y directions to make the grid easier to read.

## v0.09 - 2026-10-19

- Expand the main window into a larger visual Desktop placement interface.
- Add a scaled monitor grid with icon markers and a selected-icon pinpointer.
- Allow click-and-drag coordinate selection in the monitor preview before
  applying an icon move.
- Report the exact failing release-preparation step and status code.

## v0.08 - 2026-10-19

- Add a Haiku-native release preparation script that runs the checks and
  refreshes the README screenshot from the real application window.
- Add the application screenshot to the README at a stable generated path.

## v0.07 - 2026-10-19

- Report Tracker's native X and Y values without transposing the axes in the
  command-line and graphical icon lists.

## v0.06 - 2026-10-19

- Include the complete Haiku `BWindow` declaration required by the Deskbar
  mouse handler.

## v0.05 - 2026-10-19

- Add a graphical icon list with editable X and Y fields, Apply, Refresh, and
  status feedback.
- Add an installable Deskbar item with Open, About, and Quit actions.
- Preserve the command-line interface while Phase 2 command forwarding is
  developed.

## v0.04 - 2026-10-19

- Correct the coordinate order reported by `haichi list` so X appears before Y.

## v0.03 - 2026-10-19

- Accept exact negative coordinates for direct moves and saved layouts.
- Validate coordinates against Tracker's safe range instead of clamping them
  to the visible screen.

## v0.02 - 2026-10-19

- Link the command-line application and tests with Haiku's standard C++
  libraries.
- Remove C++ compiler flags from the resource compiler invocation.
- Initialise new Tracker pose records without clearing a non-trivial object.

## v0.01 - 2026-10-19

- Add the initial Haiku command-line application.
- Add a Tracker attribute backend for listing and moving Desktop icons.
- Add named layout saving and resolution-aware restoration.
- Add layout scaling tests and a repository validation target.
