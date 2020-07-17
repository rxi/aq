# aq
![screenshot](https://user-images.githubusercontent.com/3920290/66718248-967d7000-edd9-11e9-96b4-082ef93016d5.png)

A lightweight framework for creating audio toys


## Overview
The project is a lightweight framework for creating small audio toys. The framework binds immediate mode ui, modularly routed audio nodes and midi input to the [fe](https://github.com/rxi/fe) scripting language. A small program that would output a sinewave and change its frequency when a button is clicked would be as follows:
```clojure
(do-file "common.fe")

(= osc (dsp:new 'osc))
(= dac (dsp:new 'dac))

(dsp:link osc 'out dac 'left)
(dsp:link osc 'out dac 'right)

(func on-frame ()
  (when (ui:button "Hello")
    (dsp:set osc 'freq (+ 200 (rand 1000)))
  )
)
```
The demo program pictured in the screenshot at the top of this README file is provided in the `demo` folder. You can run the program on Linux or MacOS by doing:
```bash
./aq demo
```
or, on Windows:
```batch
aq demo
```


## Building
If you don't intend to modify the project you can download binaries for Linux and Windows from the [releases](https://github.com/rxi/aq/releases) page and avoid building it yourself.

The project can be built on Linux for both Linux and Windows. The
`build.py` script should be used to build the project. To perform a release build, run the following:
```bash
./build.py release
```
or, to cross-compile for windows:
```bash
./build.py release windows
```

To build on MacOS, you'll need SDL2 (which you can install via Homebrew `brew install sdl2`) then you can build as normal via:
```bash
./build.py release
```
NOTE: Cross compiling does not work on MacOS, nor does MIDI.

## License
This project is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.
