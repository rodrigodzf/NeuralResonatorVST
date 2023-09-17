<div  align="center">

# Neural Resonator VST

> Generate and use filters based on arbitrary 2D shapes and materials

![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/rodrigodzf/NeuralResonatorVST?include_prereleases)
![GitHub](https://img.shields.io/github/license/rodrigodzf/NeuralResonatorVST)
[![arXiv](https://img.shields.io/badge/arXiv-2210.15306-b31b1b.svg)](https://arxiv.org/abs/2210.15306)

Demo video 🎥

[![Watch the video](https://i.ytimg.com/vi/HnUc3VTUReo/maxresdefault.jpg)](https://youtu.be/HnUc3VTUReo)

</div>

## About

This is a VST plugin that uses a neural network to generate filters based on arbitrary 2D shapes and materials. It is possible to use midi to trigger simple impulses to excite these filters. Additionally any audio signal can be used as input to the filters.

This plugin uses LibTorch for the neural network and JUCE for the VST plugin. It also features an easily extensible web-based user interface built with React and Typescript. However, it is possible to build the plugin with a traditional C++ GUI (Linux will use this as default).

See also the implementation of this project on the **Bela** platform:

https://github.com/rodrigodzf/NeuralResonatorBela

## Installation

The plugin is currently available for **Mac** and **Linux** only. The plugin is available as a VST3 plugin. Simply download the [latest release](https://github.com/rodrigodzf/NeuralResonatorVST/releases) and copy the plugin to your preferred VST3 folder.

In **Mac** this is:

```bash
~/Library/Audio/Plug-Ins/VST3
```

In **Linux** this is:

```bash
~/.vst3
```

## Building from source

First, clone this repo with its submodules:

```bash
git clone --recurse-submodules git@github.com:rodrigodzf/NeuralResonatorVST.git
```

For all platforms the plugin can be built using:

```bash
bash ./bin/build.sh
```

It is also possible to build the plugin with a traditional C++ GUI using:

```bash
bash ./bin/build.sh -s
```

### Dependencies

For building on **Linux** we need the dependencies of [juce](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md):

```bash
sudo apt update
sudo apt install libasound2-dev \
    libfreetype6-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxext-dev \
    libxinerama-dev \
    libxrandr-dev \
    libwebkit2gtk-4.0-dev
```

Libtorch is automatically downloaded by the build script. However, in case you want to use your own libtorch installation it is possible to manually set it as:

```bash
cmake -S . -B build  -DCMAKE_PREFIX_PATH=YOUR_LIBTORCH_PATH
```

### Development

For development, we can build the project using:

```bash
sh ./bin/build.sh --test
cd ui && npm run dev
```

This will open a server on `localhost:3000` and you can start developing the UI. The UI will automatically reload when you change something in the source code.

The VST and the UI communicate via a websocket. The websocket server is started by the VST and the UI connects to it. The websocket server is started on `localhost:8000/ui`.

## Logs and config files

The logs are written to the user application data path.

In Mac this is:

```bash
tail -F ~/Library/Logs/NeuralResonatorVST/log.txt
```

and in Linux this is:

```bash
tail -F ~/.config/NeuralResonatorVST/log.txt
```

## Related projects

This repo uses extensively the Typescript code from [here](https://github.com/tomduncalf/tomduncalf_juce_web_ui) for the user interface.

For the websocket server it uses the code from [here](https://gitlab.com/eidheim/Simple-WebSocket-Server).
Because this project does not depend on openssl, it uses instead:

- [for sha1 hashing](https://github.com/zaphoyd/websocketpp)
- [for base64 encoding](https://github.com/gaspardpetit/base64)

It also uses the code from [here](https://github.com/rodrigodzf/torchplugins) for the pytorch plugin and the filterbank implementation.
