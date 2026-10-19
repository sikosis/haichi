# Changelog

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
