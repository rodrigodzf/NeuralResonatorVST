# NeuralResonator

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

Additionally we need openssl:

For **Linux**:

```bash
sudo apt install libssl-dev
```
For **Mac**:

From [here](https://stackoverflow.com/a/29792635)

```bash
brew install openssl
brew link --force openssl
pkg-config --modversion openssl
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

## Pytorch

For now the pytorch path **must** be set in the `CMakeLists.txt` file. This is because the pytorch library is not installed on the system. The path should be set to the `libtorch` folder in the pytorch installation. The `CMakeLists.txt` file should look like this:

```cmake
list(APPEND CMAKE_PREFIX_PATH "YOUR_PATH_TO_LIBTORCH/libtorch/share/cmake/Torch")
```

The models to load are configured in the application data path. In linux this is `~.config/config.json`.

The models can be downloaded from here:

https://drive.google.com/drive/folders/1dafuKfNBS-jLKJJZo5iVsO3d6K84xEmh?usp=share_link