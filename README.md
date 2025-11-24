# Stem Player

A JUCE-based audio plugin and standalone application for playing and mixing separated audio stems (vocals, drums, bass, etc.) from songs.

## Features

- **Multi-stem playback**: Load and play multiple audio tracks (stems) simultaneously
- **Individual volume control**: Adjust the volume of each stem independently  
- **Waveform visualization**: Interactive waveform display with playback position indicator
- **Click-to-seek**: Click anywhere on the waveform to jump to that position
- **MIDI Learn**: Map MIDI CC controllers to volume sliders for hardware control
- **Configurable stem detection**: Customize patterns to detect stem files with various naming conventions
- **Default folder**: Set a default stems folder for quick access

## Supported Formats

- VST3 Plugin
- Audio Unit (AU) Plugin (macOS)
- Standalone Application

## Building

### Prerequisites

- CMake 3.22 or later
- C++17 compatible compiler
- On macOS: Xcode command line tools
- On Windows: Visual Studio 2019 or later
- On Linux: GCC 9+ and development packages for ALSA, X11, etc.

### Build Steps

```bash
# Clone or navigate to the project directory
cd stem-player

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# Or on macOS/Linux:
make -j$(nproc)
```

### Build Outputs

After building, you'll find:
- **Standalone**: `StemPlayer_artefacts/Standalone/Stem Player`
- **VST3**: `StemPlayer_artefacts/VST3/Stem Player.vst3`
- **AU** (macOS): `StemPlayer_artefacts/AU/Stem Player.component`

## Usage

### 1. Selection Screen

- Click "Browse Folder" to select a directory containing your stem files
- The app will automatically detect songs based on stem naming patterns
- Select a song from the list and click "Load Song" (or double-click)

### 2. Main Screen  

- **Play/Pause**: Start or pause playback
- **Stop**: Stop and reset to beginning
- **Volume Sliders**: Adjust individual stem volumes
- **Waveform**: Click anywhere to seek to that position
- **MIDI Learn**: Right-click on a volume slider to access MIDI Learn

### 3. Settings Screen

- **Default Folder**: Set a default stems folder to load on startup
- **Stem Patterns**: Configure filename patterns for stem detection

## Stem File Naming

The app detects stems using configurable patterns. Default patterns include:

| Pattern | Example |
|---------|---------|
| `_vocals` | `MySong_vocals.mp3` |
| `_drums` | `MySong_drums.wav` |
| `_bass` | `MySong_bass.flac` |
| ` (Vocals)` | `MySong (Vocals).mp3` |
| `-drums` | `MySong-drums.wav` |

All stems with the same base name are grouped as one song.

## MIDI Control

1. Right-click on any volume slider
2. Select "MIDI Learn"
3. Move a MIDI CC controller
4. The slider is now mapped to that CC

To remove a mapping, right-click and select "Reset MIDI Mapping".

## License

This project is provided as-is for educational and personal use.

