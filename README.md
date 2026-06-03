# Qt Universal Input

Qt Universal Input is an experimental module that explores cross-platform
handling of game controllers, joysticks and other exotic input devices. Its
input layer is derived from the input handling of the Godot engine and uses the
community [SDL game controller database](https://github.com/mdqinc/SDL_GameControllerDB)
to map physical devices onto a common gamepad layout.

![Stylistic Gamepads](src/universalinput/doc/images/gamecontrollers.jpg)

## Modules

This repository builds three closely related areas of functionality:

- **QtUniversalInput** — the core module. `QUniversalInput` is a singleton that
  reports the live state of connected joysticks and gamepads, and
  `QActionStore` maps raw input onto named, game-level actions.
- **QtGamepad** — a thin compatibility module providing the `QGamepad`
  convenience class for reading a single gamepad. It is implemented on top of
  QtUniversalInput.
- **QML modules** (`QtUniversalInput`, `QtActionStore`, `QtGamepad`) that expose
  the same functionality to Qt Quick.

Platform backends are provided as plugins for Android, iOS, Linux (evdev),
macOS and Windows.

## Building

The module builds like any other Qt module:

```
qt-configure-module /path/to/qtgamepad
cmake --build .
```

## Examples

The `examples/universalinput` directory contains several examples, including a
console joystick monitor, an on-screen virtual gamepad, and games driven
through the action-mapping API.

## License

Qt Universal Input is available under commercial licenses, and under the
LGPL 3.0, GPL 2.0 and GPL 3.0 open source licenses. Bundled third-party
components are covered by their own licenses; see the `LICENSES` directory and
the `qt_attribution.json` files for details.
