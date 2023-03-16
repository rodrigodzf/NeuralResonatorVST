<div  align="center">

# Neural Resonator VST
> Generate and use filters based on arbitrary 2D shapes and materials

![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/rodrigodzf/NeuralResonatorVST/build)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/rodrigodzf/NeuralResonatorVST)
![GitHub](https://img.shields.io/github/license/rodrigodzf/NeuralResonatorVST)
[![arXiv](https://img.shields.io/badge/arXiv-2210.15306-b31b1b.svg)](https://arxiv.org/abs/2210.15306)
</div>

<!-- <video src='https://youtu.be/8sANJMmIxBQ' width=180/> -->
[![Watch the video](https://img.youtube.com/vi/T-D1KVIuvjA/maxresdefault.jpg)](https://youtu.be/T-D1KVIuvjA)

## About

This is a VST plugin that uses a neural network to generate filters based on arbitrary 2D shapes and materials. It is possible to use midi to trigger simple impulses to excite these filters. Additionally any audio signal can be used as input to the filters.

## Clone

```bash
git clone --recurse-submodules
```

## Build

First we need to install the dependencies of [juce](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md):

For **Linux**:

```bash
sudo apt update
sudo apt install libasound2-dev libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev  \
    libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev mesa-common-dev
```

For debug, we can build the project:

```bash
sh ./bin/build.sh --test
cd ui && npm run dev
```

This will open a server on `localhost:3000` and you can start developing the UI. The UI will automatically reload when you change something in the source code.

The VST and the UI communicate via a websocket. The websocket server is started by the VST and the UI connects to it. The websocket server is started on `localhost:8000/ui`.

Finally, the project can be built for release using:

```bash
sh ./bin/build.sh
```

See the `.github/workflows/build.yml` for more details.

### Pytorch

For now the pytorch path **must** must be passed to `cmake`. This is because the pytorch library is not installed on the system. The path should be set to the `libtorch` folder in the pytorch installation.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=YOUR_PYTORCH_PATH
```

## Logs and config files

The logs are written to the user application data path.

In Mac this is:

```bash
tail -F ~/Library/Logs/NeuralResonatorVST/log.txt
```

Note that the config file is the folder

```bash
~/Library/NeuralResonatorVST
```

In Linux this is:

```bash
tail -F ~/.config/NeuralResonatorVST/log.txt
```

with the config file in the same folder.

## Related projects

This repo uses extensively the Typescript code from [here](https://github.com/tomduncalf/tomduncalf_juce_web_ui) for the user interface.

For the websocket server it uses the code from [here](https://gitlab.com/eidheim/Simple-WebSocket-Server).
Because this project does not depend on openssl, it uses instead:
 - [for sha1 hashing](https://github.com/zaphoyd/websocketpp)
 - [for base64 encoding](https://github.com/gaspardpetit/base64)

It also uses the code from [here](https://github.com/rodrigodzf/torchplugins) for the pytorch plugin and the filterbank implementation.