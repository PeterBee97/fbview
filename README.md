# FBView - Framebuffer Viewer

A simple SDL2-based utility to view Linux framebuffer devices in a window.

## Features

- View any framebuffer device on your system
- Auto-detects resolution and color depth
- Resizable window with scaling support
- Real-time display of framebuffer content

## Building

```bash
make
```

## Usage

```bash
# View default framebuffer (/dev/fb0)
./fbview

# View specific framebuffer by ID
./fbview 1    # This will view /dev/fb1

# View specific framebuffer by full path
./fbview /dev/fb1
```

## Controls

- `+` / `=`: Increase scaling
- `-`: Decrease scaling
- `F`: Toggle fullscreen mode
- Close window to exit

## Requirements

- SDL2 library
- Linux with framebuffer devices