# GrooveKit

GrooveKit is an educational tool designed for new electronic music creators.
Inspired by the interface of the groovebox, GrooveKit provides a structured learning experience,
guiding users through a series of interactive tutorials to progressively build their music production skills.
As users advance, they unlock new features and capabilities within the app, ensuring a gradual and engaging learning process.
![Example image of the Roland groovebox](https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Fstatic.roland.com%2Fassets%2Fimages%2Fproducts%2Fgallery%2Fmc-707_front_gal.jpg&f=1&nofb=1&ipt=3b9a5fe6e158b625a339a702691a65b5b245f43a9e48347aa32b79405cd2bf62 "Roland groovebox")

## Features
- **Intuitive UI** – A user-friendly interface inspired by grooveboxes for a seamless experience.
- **Hands-on Learning** – Users create music while learning, reinforcing concepts in real-time.
- **Built-in Sound Library** – A selection of preloaded sounds and instruments to experiment with.
- **Plugin Support** – Expand functionality with third-party plugins.

## External Libraries Used

GrooveKit makes use of the [JUCE library](https://juce.com/ "JUCE library homepage"), a cross-platform C++ audio framework that is the industry standard for audio
plug-ins and other audio apps. JUCE also handles GUI needs, so we are using it for that as well.
We are also using a high-level model library called [Tracktion Engine](https://github.com/Tracktion/tracktion_engine "GitHub for Tracktion Engine") (built in JUCE) for a lot of our backend
functionality.

## Building from Source

### Prerequisites
- **CMake** 3.15 or higher
- **C++20** compatible compiler (Xcode Command Line Tools on macOS, GCC 10+/Clang 10+ on Linux, Visual Studio 2019+ on Windows)
- **Git** (for cloning with submodules)

### Build Instructions

1. **Clone the repository with submodules**:
   ```bash
   git clone --recurse-submodules git@capstone.cs.utah.edu:groovekit/groovekit.git
   cd groovekit
   ```

   If you already cloned without `--recurse-submodules`, initialize submodules manually:
   ```bash
   git submodule update --init --recursive
   ```

2. **Configure the build**:

   For **Debug** build (recommended for development):
   ```bash
   cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
   ```

   For **Release** build (optimized for performance):
   ```bash
   cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
   ```

3. **Build the application**:

   Debug build:
   ```bash
   cmake --build cmake-build-debug
   ```

   Release build:
   ```bash
   cmake --build cmake-build-release
   ```
   **Note:** the first build off of a fresh install can take a while. Expect about a 15-20 minute first build for either
   debug or release.


4. **Run GrooveKit**:

   Debug build:
   - **macOS**: `./cmake-build-debug/apps/GrooveKit/GrooveKit_artefacts/Debug/GrooveKit.app/Contents/MacOS/GrooveKit`
   - **Linux**: `./cmake-build-debug/apps/GrooveKit/GrooveKit`
   - **Windows**: `.\cmake-build-debug\apps\GrooveKit\Debug\GrooveKit.exe`

   Release build:
   - **macOS**: `./cmake-build-release/apps/GrooveKit/GrooveKit_artefacts/Release/GrooveKit.app/Contents/MacOS/GrooveKit`
   - **Linux**: `./cmake-build-release/apps/GrooveKit/GrooveKit`
   - **Windows**: `.\cmake-build-release\apps\GrooveKit\Release\GrooveKit.exe`

**NOTE:** it is recommended that you run GrooveKit on a computer running macOS. The JUCE library (and by extension,
Tracktion Engine), is cross-platform, but we have developed and tested GrooveKit exclusively on macOS. We make no
guarantees of performance or full functionality for GrooveKit on any other OS.

### Building from Downloaded ZIP

If you downloaded a ZIP file instead of cloning:

1. **Extract the ZIP**
2. **Remove empty submodule directories and clone dependencies manually**:
   ```bash
   cd groovekit

   # Remove the empty placeholder directory
   rm -rf third-party/tracktion_engine

   # Clone Tracktion Engine with its JUCE submodule
   git clone --recurse-submodules https://github.com/Tracktion/tracktion_engine.git third-party/tracktion_engine
   ```

3. **Follow steps 2-4 above** to build and run

**Note**: Building from ZIP requires Git to be installed. For the best experience, we recommend using `git clone --recurse-submodules` instead.

## Getting Started With GrooveKit
1. **Install GrooveKit** – Download and install the application on your device.
2. **Open the app** – You will be given a fresh project that you can immediately start working with.
3. **Add a new track** - Go to "Track" in the menu bar and create a new instrument (or drum) track.
4. **Add instrument** - Select an instrument from the dropdown menu on the new track header, or click on "Instrument" on a drum track to open the drum sampler.
5. **Start playing!** - Once you have a new instrument set up, hit the "R" button on the track header to record arm. Try clicking keys on the ASDF or QWERTY row!
   * If you have an external MIDI controller, hook it up to the computer and it will be automatically connected to your instrument.
6. **Write music** - You can immediately record a clip on an armed track by clicking the circle button in the top bar, or right-click on
   the track to create a new MIDI clip. Double-click on the MIDI clip to add notes.
7. **Mix to taste** - Click on the three columns in the top-right corner to go to the mix view, and adjust the volume and panning. You can also add external 
   plugins if you have any as insert effects.
8. **Save project** - Either click "Save As" or "Save" in the "File" section of the menu bar, and you will be asked to name your project and save it somewhere on your computer. Now you
   can create a new project or load an existing project and come back to your previous work later!
9. **Export to audio** - Lastly, go to the "File" menu again and click "Export Audio" to immediately export your project
   to a .wav file. Now you can play your bop in your favorite .wav compatible audio player!

## Target Audience
GrooveKit is designed for:
- Beginners who want to learn electronic music production from the ground up.
- Hobbyists looking for a structured way to improve their skills.
- Educators and students in music technology programs.

## License
GrooveKit is released under the MIT License. See `LICENSE` for more details.
---

**Join us in making electronic music education more accessible and engaging! :)**
