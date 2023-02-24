# NeuralResonator

## Clone

```bash
git clone --recurse-submodules
```

## Build

First we need to install the dependencies of [juce](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md):

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

Then we can build the project:

```bash
mkdir build
cd build
cmake ..
make -J
```

After this we can install and start the UI for development:

```bash
cd ui
npm install --include=dev
npm run dev
```

This will open a server on `localhost:3000` and you can start developing the UI. The UI will automatically reload when you change something in the source code.

The VST and the UI communicate via a websocket. The websocket server is started by the VST and the UI connects to it. The websocket server is started on `localhost:8000/ui`.

