# Manual hardware validation for Qt Universal Input

The automated tests under `tests/auto/` cover everything reachable through
synthetic input injection: the action store, the mapping parser and transforms,
the `QUniversalInput` and `QGamepad` APIs, the plugin factories, and the QML
layer. They run without any hardware.

What they cannot cover is the part of the module that only exists on real
hardware: device enumeration and hot-plug, the platform backends translating OS
events, force-feedback actuation, and OS-level relative mouse mode. Those need a
human with a controller. This document is the checklist for that, and it is run
once per supported platform whenever the input backends change.

## Driver applications

Rather than a bespoke tool, the existing examples are the validation drivers:

| Example | Path | Use for |
| --- | --- | --- |
| Console joystick monitor | `examples/universalinput/consolejoystickmonitor` | Connection, button and axis events on the command line. Also calls `addForce()` on every button press, so it doubles as a rumble test. |
| Virtual gamepad | `examples/universalinput/virtualgamepad` | A visual, on-screen view of the live controller state. |
| Mouse grab | `examples/universalinput/mousegrab` | Relative mouse mode (`setMouseDisabled()` / `mouseMovedWithDeltas()`). |
| Simple monitor | `examples/universalinput/simple` | The `QGamepad` convenience API. |
| Action pong / Quick action | `examples/universalinput/actionpong`, `examples/universalinput/quickaction` | `QActionStore` end to end with real input. |

Build the module with examples enabled (`-DQT_BUILD_EXAMPLES=ON`) and run the
relevant example from the build tree.

## Checklist

Run every item with at least one controller that is present in the bundled SDL
game controller database (for example an Xbox or DualShock pad) and, where
noted, one obscure device that is not.

1. **Enumeration at startup** - with a controller already connected, launch
   `consolejoystickmonitor`. A `joyConnectionChanged` is reported with the
   correct device index and name.
2. **Hot-plug** - with the monitor running, disconnect and reconnect the
   controller. Disconnect and reconnect are both reported, and the index is
   reused on reconnect.
3. **Every button** - press each button in turn and confirm it is reported:
   A/B/X/Y, Back/Guide/Start, left/right shoulder, left/right stick press.
4. **Hat / D-pad** - press each of the four D-pad directions and the four
   diagonals; confirm the diagonals report both component directions.
5. **Thumbsticks** - sweep the left and right sticks through their full range on
   both axes; confirm values span roughly -1.0 to 1.0 and return to ~0 at rest.
6. **Analog triggers** - pull the left and right triggers; confirm the value
   moves from 0.0 at rest to 1.0 fully pressed.
7. **Mapped vs unmapped** - a database controller reports the normalized layout
   above and `isGamepad` is true; an unknown device reports raw button indices
   and `isGamepad` is false.
8. **Force feedback** - on hardware that supports rumble, pressing a button in
   `consolejoystickmonitor` produces a vibration. On hardware without rumble,
   nothing happens and the application does not crash.
9. **Relative mouse mode** - run `mousegrab`, enable relative mouse mode, and
   confirm motion is reported as deltas and the cursor behaves as expected.
10. **Multiple devices** - connect two controllers; confirm they get distinct
    device indices and that input from one does not affect the other.
11. **Convenience and action APIs** - exercise `simple` (QGamepad properties
    track the device) and `actionpong` / `quickaction` (named actions fire from
    real input).

## Results

Record the outcome per platform for each run. Note the controller models used.

| # | Item | Linux | macOS | Windows | Android | iOS |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Enumeration | | | | | |
| 2 | Hot-plug | | | | | |
| 3 | Buttons | | | | | |
| 4 | Hat / D-pad | | | | | |
| 5 | Thumbsticks | | | | | |
| 6 | Triggers | | | | | |
| 7 | Mapped vs unmapped | | | | | |
| 8 | Force feedback | | | | | |
| 9 | Relative mouse | | | | | |
| 10 | Multiple devices | | | | | |
| 11 | Convenience / actions | | | | | |
