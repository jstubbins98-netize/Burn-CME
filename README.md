# Burn-CME

## CD/DVD Burning and Extraction Utility

---

### IMPORTANT LEGAL NOTICE

**This software is NOT intended for piracy or any illegal copying of copyrighted material.**

Burn-CME is designed for legitimate purposes only, including:
- Backing up your own personal data
- Creating copies of media you own for personal use
- Burning your own original content
- Creating installation discs from legally obtained software

**Consequences of Piracy:**
- **Criminal Prosecution**: Copyright infringement can result in criminal charges, leading to fines up to $250,000 and imprisonment up to 5 years in the United States, with similar penalties in other countries.
- **Civil Lawsuits**: Copyright holders may sue for damages ranging from $750 to $150,000 per work infringed.
- **Internet Service Termination**: ISPs may terminate your service for repeated copyright violations.
- **Malware Risks**: Pirated content often contains viruses, ransomware, and other malicious software.
- **Ethical Harm**: Piracy deprives creators, artists, and developers of fair compensation for their work.

By using this software, you agree to use it only for lawful purposes and accept full responsibility for your actions. The developers of Burn-CME do not condone, support, or encourage any form of copyright infringement.

---

Burn-CME is a cross-platform utility for burning data to CDs and DVDs, as well as extracting data from optical media. It provides both a command-line interface (CLI) for power users and a graphical user interface (GUI) for ease of use.

---

## Table of Contents

1. [Features](#features)
   - [Drive Detection](#drive-detection)
   - [File Queue Management](#file-queue-management)
   - [ISO Creation](#iso-creation)
   - [Disc Burning](#disc-burning)
   - [Data Extraction](#data-extraction)
   - [Video DVD Creation](#video-dvd-creation)
   - [DVD Video Extraction](#dvd-video-extraction)
   - [Disc Information](#disc-information)
   - [Disc Erasing](#disc-erasing)
   - [Cross-Platform Support](#cross-platform-support)
   - [Apple SuperDrive Support](#apple-superdrive-support)
2. [System Requirements](#system-requirements)
3. [Installation](#installation)
4. [Quick Start](#quick-start)
5. [Command-Line Interface (CLI)](#command-line-interface-cli)
6. [Graphical User Interface (GUI)](#graphical-user-interface-gui)
7. [Troubleshooting](#troubleshooting)
8. [Technical Reference](#technical-reference)

---

## Features

Burn-CME provides a comprehensive set of tools for working with optical media.

---

### Drive Detection

Automatically detect and manage CD/DVD drives on your system.

- Scans for all available optical drives (internal and USB)
- Displays drive paths and capabilities
- Supports multiple drives simultaneously
- Dedicated Drives tab in GUI for easy drive management
- One-click drive selection and ejection

**Supported Devices:**
- Internal SATA/IDE optical drives
- USB external CD/DVD drives
- Apple USB SuperDrive (with automatic wake-up on Linux)

---

### File Queue Management

Organize files and folders before burning to disc.

- Add individual files or entire directories
- Visual queue display with file sizes
- Remove selected items or clear entire queue
- Automatic duplicate name handling
- Preserves original directory structure
- Real-time size calculation with CD/DVD capacity indicator

**Usage:**
1. Click "Add Files" or "Add Folder" to queue items
2. Review the queue to verify contents
3. Remove unwanted items if needed
4. Proceed to ISO creation

---

### ISO Creation

Create standard ISO9660 images from your queued files.

- Generates industry-standard ISO format
- Preserves complete directory hierarchy
- Joliet extensions for long filenames
- Compatible with Windows, macOS, and Linux
- Automatic tool selection (mkisofs, genisoimage, xorrisofs, or hdiutil)

**Output Format:**
- ISO9660 with Joliet extensions
- Maximum compatibility across operating systems
- Suitable for burning or virtual mounting

---

### Disc Burning

Burn ISO images to CD or DVD media.

- Supports CD-R, CD-RW, DVD-R, DVD+R, DVD-RW, DVD+RW
- Configurable burn speed (Auto, 4x, 8x, 16x, Maximum)
- Progress feedback during burning
- Automatic drive capability detection
- Uses platform-native tools (cdrecord/wodim on Linux, hdiutil on macOS)

**Recommended Settings:**
- Use 8x or lower speed for better compatibility
- Use quality media from reputable brands
- Verify disc is compatible with your drive before burning

---

### Data Extraction

Extract data from CDs and DVDs to your computer.

- Create ISO backup images from physical discs
- Proper ISO9660/Joliet format for cross-platform compatibility
- Automatic disc detection and mounting
- Progress feedback during extraction
- Works with data discs, software discs, and backups

**Extraction Methods:**
- **ISO Image**: Creates a complete disc image file
- **File Copy**: Copies files directly to a folder (coming soon)

**macOS Note:** Uses `hdiutil makehybrid` for proper ISO creation, ensuring extracted images work on all platforms.

---

### Video DVD Creation

Convert MP4 and other video files to Video DVD format for playback on standard DVD players.

- Convert MP4, AVI, MKV, MOV, WMV, FLV, WEBM files to DVD-Video format
- Creates proper MPEG-2 video with AC3/MPEG audio
- Generates VIDEO_TS folder structure with VOB files
- Supports multiple video files on a single DVD
- Option to burn immediately after creation
- Uses FFmpeg for high-quality video conversion

**Requirements:**
- FFmpeg (for video conversion)
- dvdauthor (for creating DVD structure - optional but recommended)

**Installation:**
- Linux: `sudo apt install ffmpeg dvdauthor`
- macOS: `brew install ffmpeg dvdauthor`

**Supported Input Formats:**
- MP4, AVI, MKV, MOV, WMV, FLV, WEBM, M4V, MPEG, MPG

---

### DVD Video Extraction

Extract video content from Video DVDs and convert to MP4 format.

- Extract video from physical DVD discs
- Extract from VIDEO_TS folders
- Extract from DVD ISO images
- High-quality H.264/AAC output
- Fast conversion with FFmpeg
- Preserves video quality with adjustable settings

**Extraction Sources:**
- Physical DVD drive
- VIDEO_TS folder on disk
- Mounted ISO images

**Output:**
- MP4 format with H.264 video and AAC audio
- Optimized for web streaming (faststart flag)
- Configurable output quality

---

### Disc Information

View detailed information about your drive and inserted media.

- Drive capabilities (read/write speeds, supported formats)
- Current media type and capacity
- Used and available space
- Media manufacturer information
- Drive firmware details

**Information Displayed:**
- Drive model and path
- Supported disc types
- Current disc status
- Read/write capabilities

---

### Disc Erasing

Erase rewritable discs for reuse.

- Supports CD-RW, DVD-RW, and DVD+RW media
- Quick erase (faster, overwrites TOC only)
- Full erase (complete wipe, more secure)
- Confirmation required to prevent accidents
- Platform-native erasing (cdrecord on Linux, drutil on macOS)

**Erase Types:**
| Type | Speed | Security | Use Case |
|------|-------|----------|----------|
| Quick | ~1 minute | Low | Normal reuse |
| Full | 10-30 minutes | High | Sensitive data |

---

### Cross-Platform Support

Works seamlessly on Linux and macOS.

**Linux:**
- Uses cdrecord/wodim for burning
- Uses mkisofs/genisoimage/xorrisofs for ISO creation
- Drive detection via /dev/sr*, /dev/cdrom, /dev/dvd
- Supports all major distributions (Ubuntu, Fedora, Arch, openSUSE, etc.)

**macOS:**
- Uses hdiutil for burning and ISO creation
- Uses drutil for drive control and erasing
- Automatic optical drive detection via diskutil
- Works with internal and external drives

---

### Apple SuperDrive Support

Special support for Apple USB SuperDrive on non-Apple computers.

The Apple SuperDrive requires a "wake-up" command when used with non-Apple hardware. Burn-CME handles this automatically.

**How It Works:**
1. When you select a drive, Burn-CME checks if it's an Apple SuperDrive
2. If detected, it automatically sends the wake-up command
3. The drive then accepts discs normally

**Automatic Features:**
- Detects Apple SuperDrive by USB vendor/product ID (05ac:1500)
- Sends wake-up command: `sg_raw /dev/srX EA 00 00 00 00 00 01`
- Triggers automatically when drive is selected

**Requirements:**

Install the sg3-utils package:

```bash
# Ubuntu/Debian
sudo apt install sg3-utils

# Fedora/RHEL
sudo dnf install sg3_utils

# Arch Linux
sudo pacman -S sg3_utils
```

**Manual Wake-Up:**

If automatic wake-up doesn't work, manually wake the drive:

```bash
sg_raw /dev/sr0 EA 00 00 00 00 00 01
```

**Automatic Wake-Up with udev:**

Create a udev rule for automatic wake-up when the drive is plugged in:

```bash
sudo tee /etc/udev/rules.d/90-mac-superdrive.rules << EOF
ACTION=="add", ATTRS{idProduct}=="1500", ATTRS{idVendor}=="05ac", DRIVERS=="usb", RUN+="/usr/bin/sg_raw %r/sr%n EA 00 00 00 00 00 01"
EOF

sudo udevadm control --reload-rules
```

---

## System Requirements

### Hardware
- CD/DVD optical drive (internal or external USB)
- Blank or rewritable optical media for burning

### Software

#### Linux
- g++ compiler (C++17 support)
- cdrecord or wodim (burning)
- mkisofs, genisoimage, or xorrisofs (ISO creation)
- sg3-utils (for Apple SuperDrive support)
- Qt6 (for GUI, optional)

#### macOS
- Xcode Command Line Tools
- Homebrew (for Qt6 installation)
- Qt6 (for GUI, optional)

---

## Installation

### Automatic Installation (Recommended)

The easiest way to install Burn-CME is using the included build script:

```bash
# Clone or download the project
cd burn-cme

# Run the build script (installs dependencies and compiles)
./build.sh
```

The script will:
1. Detect your operating system
2. Install all required dependencies
3. Compile both CLI and GUI versions
4. Save binaries to `build/bin/`

### Interactive Build Menu

When you run `./build.sh` without arguments, you'll see an interactive menu:

```
┌─────────────────────────────────────────┐
│         BURN-CME BUILD INSTALLER        │
│       CD/DVD Burning Utility            │
├─────────────────────────────────────────┤
│                                         │
│  Select installation type:              │
│                                         │
│  1. Full Installation (CLI + GUI)       │
│  2. CLI Only (Command Line Interface)   │
│  3. GUI Only (Graphical Interface)      │
│  4. Clean Build Artifacts               │
│  5. Exit                                │
│                                         │
└─────────────────────────────────────────┘
```

After building, you'll be asked if you want to run the program immediately.

### Build Options (Non-Interactive)

```bash
./build.sh              # Interactive menu
./build.sh --full       # Build both CLI and GUI (non-interactive)
./build.sh --cli-only   # Build CLI only (non-interactive)
./build.sh --gui-only   # Build GUI only (non-interactive)
./build.sh --no-deps    # Skip dependency installation
./build.sh --no-run     # Skip the "run now?" prompt
./build.sh --clean      # Remove build artifacts
./build.sh --help       # Show help
```

### Manual Dependency Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential g++ cmake pkg-config \
    qt6-base-dev libgl1-mesa-dev cdrecord wodim \
    genisoimage xorriso sg3-utils eject
```

#### Fedora/RHEL
```bash
sudo dnf install gcc-c++ cmake pkg-config qt6-qtbase-devel \
    mesa-libGL-devel wodim genisoimage xorriso sg3_utils eject
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake pkgconf qt6-base mesa \
    cdrtools xorriso sg3_utils util-linux
```

#### macOS
```bash
brew install cmake pkg-config qt@6
```

---

## Quick Start

### Using the CLI

```bash
# Start interactive mode
./burn-cme

# Show help
./burn-cme --help

# Show version
./burn-cme --version
```

### Using the GUI

```bash
./burn-cme-gui
```

---

## Command-Line Interface (CLI)

The CLI provides a menu-driven interface for all operations.

### Main Menu

When you run `./burn-cme`, you'll see the main menu:

```
┌─────────────────────────────────────┐
│          MAIN MENU                  │
├─────────────────────────────────────┤
│  1. Detect CD/DVD Drives            │
│  2. Add Files to Burn               │
│  3. View Files Queue                │
│  4. Clear Files Queue               │
│  5. Create ISO Image                │
│  6. Burn to CD/DVD                  │
│  7. Extract Data from CD/DVD        │
│  8. View Disc Info                  │
│  9. Erase Rewritable Disc           │
│  0. Exit                            │
└─────────────────────────────────────┘
```

### Step-by-Step: Burning Files to a Disc

1. **Detect Drives** (Option 1)
   - Scans your system for optical drives
   - Select the drive you want to use

2. **Add Files** (Option 2)
   - Enter file or folder paths to add to the burn queue
   - Type `done` when finished adding files

3. **View Queue** (Option 3)
   - Review the files that will be burned
   - Shows file sizes and total size

4. **Create ISO** (Option 5)
   - Enter a name for the ISO image
   - The program creates an ISO from your queued files

5. **Burn to Disc** (Option 6)
   - Insert a blank disc
   - The program burns your ISO to the disc

### Step-by-Step: Extracting Data from a Disc

1. **Detect Drives** (Option 1)
   - Select the drive containing your disc

2. **Extract Data** (Option 7)
   - Choose extraction method:
     - Copy files to a directory
     - Create an ISO backup image
   - Enter the output path

### Step-by-Step: Erasing a Rewritable Disc

1. **Detect Drives** (Option 1)
   - Select the drive containing your rewritable disc

2. **Erase Disc** (Option 9)
   - Choose erase type:
     - Quick erase (faster)
     - Full erase (more thorough)
   - Confirm by typing `ERASE`

---

## Graphical User Interface (GUI)

The GUI provides a visual interface with tabbed navigation.

### File Queue Tab

This is where you manage files to burn:

- **Add Files**: Click to select individual files
- **Add Folder**: Click to add an entire directory
- **Remove Selected**: Remove highlighted items from the queue
- **Clear All**: Empty the entire queue
- **ISO Name**: Enter the name for your ISO image
- **Create ISO**: Generate an ISO from the queued files

The status bar shows the total number of files and size, plus whether it fits on CD or requires DVD.

### Burn Tab

This is where you burn ISOs to disc:

- **Drive Selection**: Choose your optical drive from the dropdown
- **Detect**: Refresh the list of available drives
- **ISO File**: Enter the path or click Browse to select an ISO
- **Speed**: Select burn speed (Auto, 4x, 8x, 16x, Maximum)
- **Burn to Disc**: Start the burning process

### Extract Tab

This is where you extract data from discs:

- **Drive Selection**: Choose the drive containing your disc
- **Output Path**: Enter where to save the extracted data
- **Extract as ISO**: Copy the disc to an ISO file
- **Extract Files**: Copy files to a directory (coming soon)

### Disc Info Tab

View information about your drive and inserted disc:

- **Refresh Info**: Update the displayed information
- **Erase Disc**: Erase rewritable media

### Progress Screen

During long-running operations (burning, extracting, creating ISO, erasing), a dedicated progress screen appears with:

- **Animated CD image**: Visual feedback showing the operation is in progress
- **Operation title**: Clearly shows what's happening (Burning, Extracting, etc.)
- **Status message**: Describes the current step
- **Progress bar**: Shows operation progress
- **Close button**: Appears when the operation completes

This provides clear visual feedback during disc operations that may take several minutes.

---

## Troubleshooting

### "No optical drives detected"

**Cause**: No optical drive is connected or recognized by the system.

**Solutions**:
- Ensure the drive is properly connected
- For USB drives, try a different USB port
- Check if the drive appears in `/dev/` (Linux) or System Information (macOS)
- For Apple SuperDrive on Linux, install sg3-utils and ensure the wake-up command is sent

### "Permission denied" when burning

**Cause**: Insufficient permissions to access the optical drive.

**Solutions**:
- Run with sudo: `sudo ./burn-cme`
- Add your user to the `cdrom` group: `sudo usermod -a -G cdrom $USER` (then log out and back in)

### "cdrecord: command not found"

**Cause**: The cdrecord package is not installed.

**Solutions**:
- Install wodim (Linux): `sudo apt install wodim` or `sudo dnf install wodim`
- On some systems, wodim provides cdrecord compatibility

### ISO creation fails

**Cause**: mkisofs or alternative tool not installed.

**Solutions**:
- Install genisoimage: `sudo apt install genisoimage`
- Or install xorriso: `sudo apt install xorriso`
- On macOS, hdiutil is used and should be available by default

### GUI doesn't start

**Cause**: Qt6 libraries not installed or not found.

**Solutions**:
- Install Qt6: `sudo apt install qt6-base-dev` (Linux) or `brew install qt@6` (macOS)
- Rebuild with `./build.sh`
- Use the CLI version instead: `./burn-cme`

### Apple SuperDrive not accepting discs

**Cause**: The drive hasn't received the wake-up command.

**Solutions**:
- Install sg3-utils: `sudo apt install sg3-utils`
- Manually wake: `sg_raw /dev/sr0 EA 00 00 00 00 00 01`
- Reselect the drive in Burn-CME to trigger automatic wake-up

### Burning fails or produces coasters

**Possible causes and solutions**:
- Use lower burn speed (4x or 8x)
- Use quality media from reputable brands
- Ensure disc is compatible with your drive (check with Option 8 - View Disc Info)
- Clean the drive lens with a cleaning disc

---

## Technical Reference

### Supported Disc Types

| Type | Capacity | Notes |
|------|----------|-------|
| CD-R | 700 MB | Write once |
| CD-RW | 700 MB | Rewritable |
| DVD-R | 4.7 GB | Write once |
| DVD+R | 4.7 GB | Write once |
| DVD-RW | 4.7 GB | Rewritable |
| DVD+RW | 4.7 GB | Rewritable |
| DVD-R DL | 8.5 GB | Dual layer, write once |
| DVD+R DL | 8.5 GB | Dual layer, write once |

### Drive Device Paths

| OS | Typical Path |
|----|--------------|
| Linux | /dev/sr0, /dev/sr1, /dev/cdrom, /dev/dvd |
| macOS | Managed by drutil/diskutil |

### Command-Line Tools Used

| Tool | Purpose | Platform |
|------|---------|----------|
| cdrecord/wodim | Burning | Linux |
| mkisofs/genisoimage/xorrisofs | ISO creation | Linux |
| hdiutil | Burning and ISO creation | macOS |
| drutil | Drive control, erasing | macOS |
| dd | Data extraction | Both |
| sg_raw | SuperDrive wake-up | Linux |

### File Structure

```
Burn-CME/
├── src/burn-cme/
│   ├── main.cpp          # CLI source
│   ├── core.h            # Shared core logic
│   ├── mainwindow.cpp    # GUI implementation
│   ├── mainwindow.h      # GUI header
│   ├── progressdialog.h  # Progress screen dialog
│   └── gui_main.cpp      # GUI entry point
├── images/
│   ├── burn-cme-icon.png # Application icon
│   └── cd-spinning.gif   # Progress animation
├── build.sh              # Build script with dependencies
├── setup.sh              # Quick build script
├── burn-cme.desktop      # Linux desktop entry
├── CMakeLists.txt        # CMake configuration
├── build/bin/            # Compiled binaries
└── README.md             # This file
```

---

## License

Burn-CME is open source software. Feel free to use, modify, and distribute.

---

## Support

If you encounter issues not covered in this manual:

1. Check the troubleshooting section above
2. Review the log output for error messages
3. Ensure all dependencies are properly installed
4. Try rebuilding with `./build.sh --clean && ./build.sh`
