#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${BLUE}[STEP]${NC} $1"; }

show_menu() {
    echo ""
    echo -e "${CYAN}┌─────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│         BURN-CME BUILD INSTALLER        │${NC}"
    echo -e "${CYAN}│       CD/DVD Burning Utility            │${NC}"
    echo -e "${CYAN}├─────────────────────────────────────────┤${NC}"
    echo -e "${CYAN}│                                         │${NC}"
    echo -e "${CYAN}│  Select installation type:              │${NC}"
    echo -e "${CYAN}│                                         │${NC}"
    echo -e "${CYAN}│  1. Full Installation (CLI + GUI)       │${NC}"
    echo -e "${CYAN}│  2. CLI Only (Command Line Interface)   │${NC}"
    echo -e "${CYAN}│  3. GUI Only (Graphical Interface)      │${NC}"
    echo -e "${CYAN}│  4. Clean Build Artifacts               │${NC}"
    echo -e "${CYAN}│  5. Exit                                │${NC}"
    echo -e "${CYAN}│                                         │${NC}"
    echo -e "${CYAN}└─────────────────────────────────────────┘${NC}"
    echo ""
    echo -n "Enter your choice [1-5]: "
}

detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        log_info "Detected macOS"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
        if [ -f /etc/os-release ]; then
            . /etc/os-release
            DISTRO=$ID
            log_info "Detected Linux: $DISTRO"
        else
            DISTRO="unknown"
            log_info "Detected Linux (unknown distribution)"
        fi
    else
        log_error "Unsupported operating system: $OSTYPE"
        exit 1
    fi
}

check_sudo() {
    if [ "$EUID" -ne 0 ]; then
        if command -v sudo &> /dev/null; then
            SUDO="sudo"
        else
            log_warn "Not running as root and sudo not available"
            log_warn "Some dependency installations may fail"
            SUDO=""
        fi
    else
        SUDO=""
    fi
}

install_macos_deps() {
    log_step "Installing macOS dependencies..."
    
    if ! command -v brew &> /dev/null; then
        log_warn "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    log_info "Installing build tools and Qt6..."
    brew install cmake pkg-config || true
    brew install qt@6 || true
    
    QT_PATH=$(brew --prefix qt@6 2>/dev/null || echo "")
    if [ -n "$QT_PATH" ]; then
        export PATH="$QT_PATH/bin:$PATH"
        export PKG_CONFIG_PATH="$QT_PATH/lib/pkgconfig:$PKG_CONFIG_PATH"
        export CMAKE_PREFIX_PATH="$QT_PATH:$CMAKE_PREFIX_PATH"
    fi
    
    log_info "macOS dependencies installed"
}

install_linux_deps() {
    log_step "Installing Linux dependencies..."
    
    case "$DISTRO" in
        ubuntu|debian|pop|linuxmint|elementary)
            log_info "Using apt package manager..."
            $SUDO apt-get update
            $SUDO apt-get install -y \
                build-essential \
                g++ \
                cmake \
                pkg-config \
                qt6-base-dev \
                libgl1-mesa-dev \
                cdrecord \
                wodim \
                genisoimage \
                xorriso \
                sg3-utils \
                eject \
                ffmpeg \
                dvdauthor \
                || log_warn "Some packages may not be available"
            ;;
        fedora|rhel|centos|rocky|alma)
            log_info "Using dnf package manager..."
            $SUDO dnf install -y \
                gcc-c++ \
                cmake \
                pkg-config \
                qt6-qtbase-devel \
                mesa-libGL-devel \
                wodim \
                genisoimage \
                xorriso \
                sg3_utils \
                eject \
                ffmpeg \
                dvdauthor \
                || log_warn "Some packages may not be available"
            ;;
        arch|manjaro|endeavouros)
            log_info "Using pacman package manager..."
            $SUDO pacman -Syu --noconfirm \
                base-devel \
                cmake \
                pkgconf \
                qt6-base \
                mesa \
                cdrtools \
                xorriso \
                sg3_utils \
                util-linux \
                ffmpeg \
                dvdauthor \
                || log_warn "Some packages may not be available"
            ;;
        opensuse*|suse*)
            log_info "Using zypper package manager..."
            $SUDO zypper install -y \
                gcc-c++ \
                cmake \
                pkg-config \
                qt6-base-devel \
                Mesa-libGL-devel \
                wodim \
                genisoimage \
                xorriso \
                sg3_utils \
                || log_warn "Some packages may not be available"
            ;;
        gentoo)
            log_info "Using portage package manager..."
            $SUDO emerge --ask=n \
                sys-devel/gcc \
                dev-build/cmake \
                dev-util/pkgconf \
                dev-qt/qtbase:6 \
                media-libs/mesa \
                app-cdr/cdrtools \
                dev-libs/libburn \
                sys-apps/sg3_utils \
                || log_warn "Some packages may not be available"
            ;;
        void)
            log_info "Using xbps package manager..."
            $SUDO xbps-install -Sy \
                gcc \
                cmake \
                pkg-config \
                qt6-base-devel \
                MesaLib-devel \
                cdrtools \
                xorriso \
                sg3_utils \
                || log_warn "Some packages may not be available"
            ;;
        alpine)
            log_info "Using apk package manager..."
            $SUDO apk add --no-cache \
                build-base \
                cmake \
                pkgconf \
                qt6-qtbase-dev \
                mesa-dev \
                cdrkit \
                xorriso \
                sg3_utils \
                || log_warn "Some packages may not be available"
            ;;
        solus)
            log_info "Using eopkg package manager..."
            $SUDO eopkg install -y \
                gcc \
                cmake \
                pkg-config \
                qt6-base-devel \
                mesalib-devel \
                cdrtools \
                xorriso \
                || log_warn "Some packages may not be available"
            ;;
        clear-linux-os)
            log_info "Using swupd package manager..."
            $SUDO swupd bundle-add \
                c-basic \
                qt6-basic-dev \
                devpkg-mesa \
                || log_warn "Some packages may not be available"
            ;;
        nixos|nix)
            log_info "NixOS detected - using nix-shell for dependencies..."
            if command -v nix-shell &> /dev/null; then
                log_info "You can run: nix-shell -p gcc cmake pkg-config qt6.full libGL cdrtools xorriso sg3_utils"
                log_info "Or add to configuration.nix: environment.systemPackages"
                nix-env -iA nixpkgs.qt6.full nixpkgs.pkg-config nixpkgs.cmake 2>/dev/null || log_warn "nix-env install failed, please install manually"
            fi
            ;;
        *)
            log_warn "Unknown distribution: $DISTRO"
            log_info "Attempting generic Qt6 installation..."
            
            # Try various package managers as fallback
            if command -v apt-get &> /dev/null; then
                $SUDO apt-get update && $SUDO apt-get install -y qt6-base-dev build-essential cmake pkg-config || true
            elif command -v dnf &> /dev/null; then
                $SUDO dnf install -y qt6-qtbase-devel gcc-c++ cmake pkg-config || true
            elif command -v pacman &> /dev/null; then
                $SUDO pacman -Sy --noconfirm qt6-base base-devel cmake pkgconf || true
            elif command -v zypper &> /dev/null; then
                $SUDO zypper install -y qt6-base-devel gcc-c++ cmake pkg-config || true
            elif command -v apk &> /dev/null; then
                $SUDO apk add qt6-qtbase-dev build-base cmake pkgconf || true
            else
                log_warn "No supported package manager found"
                log_warn "Please install manually: g++, cmake, pkg-config, qt6, libGL, cdrecord/wodim, mkisofs/genisoimage, sg3-utils"
            fi
            ;;
    esac
    
    log_info "Linux dependencies installed"
}

install_dependencies() {
    log_step "Installing dependencies for $OS..."
    
    if [ "$OS" == "macos" ]; then
        install_macos_deps
    else
        install_linux_deps
    fi
}

create_build_dir() {
    BUILD_DIR="$(pwd)/build"
    
    if [ -d "$BUILD_DIR" ]; then
        log_info "Cleaning existing build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    mkdir -p "$BUILD_DIR/bin"
    mkdir -p "$BUILD_DIR/moc"
    
    log_info "Created build directory: $BUILD_DIR"
}

compile_cli() {
    log_step "Compiling CLI version..."
    
    g++ -std=c++17 -Wall -Wextra -O2 \
        -o "$BUILD_DIR/bin/burn-cme" \
        src/CLI/main.cpp \
        2>&1
    
    if [ -f "$BUILD_DIR/bin/burn-cme" ]; then
        chmod +x "$BUILD_DIR/bin/burn-cme"
        log_info "CLI compiled successfully: $BUILD_DIR/bin/burn-cme"
        return 0
    else
        log_error "CLI compilation failed"
        return 1
    fi
}

find_qt6() {
    # Try pkg-config first
    QT_CFLAGS=$(pkg-config --cflags Qt6Widgets Qt6Core Qt6Gui 2>/dev/null || echo "")
    QT_LIBS=$(pkg-config --libs Qt6Widgets Qt6Core Qt6Gui 2>/dev/null || echo "")
    
    if [ -n "$QT_CFLAGS" ] && [ -n "$QT_LIBS" ]; then
        log_info "Found Qt6 via pkg-config"
        return 0
    fi
    
    # Common Qt6 installation paths to search
    QT6_SEARCH_PATHS=(
        "/usr/include/qt6"
        "/usr/include/x86_64-linux-gnu/qt6"
        "/usr/include/aarch64-linux-gnu/qt6"
        "/usr/local/include/qt6"
        "/opt/qt6/include"
        "/opt/Qt/6.*/gcc_64/include"
        "/usr/lib/qt6/include"
        "/usr/lib64/qt6/include"
        "$HOME/Qt/6.*/gcc_64/include"
        "/usr/include/qt"
        "/usr/lib/x86_64-linux-gnu/qt6"
        "/usr/lib/aarch64-linux-gnu/qt6"
    )
    
    QT6_LIB_PATHS=(
        "/usr/lib/x86_64-linux-gnu"
        "/usr/lib/aarch64-linux-gnu"
        "/usr/lib64"
        "/usr/lib"
        "/usr/local/lib"
        "/opt/qt6/lib"
        "/opt/Qt/6.*/gcc_64/lib"
        "/usr/lib/qt6/lib"
        "$HOME/Qt/6.*/gcc_64/lib"
    )
    
    # Search for Qt6 include directory
    QT6_INCLUDE=""
    for pattern in "${QT6_SEARCH_PATHS[@]}"; do
        for path in $pattern; do
            if [ -d "$path" ] && [ -d "$path/QtWidgets" ] || [ -d "$path/QtCore" ]; then
                QT6_INCLUDE="$path"
                log_info "Found Qt6 includes at: $path"
                break 2
            fi
        done
    done
    
    # Also check for QtWidgets subdirectory
    if [ -z "$QT6_INCLUDE" ]; then
        for base in /usr/include /usr/local/include /opt/qt6/include; do
            if [ -d "$base/QtWidgets" ]; then
                QT6_INCLUDE="$base"
                log_info "Found Qt6 includes at: $base"
                break
            fi
        done
    fi
    
    # Search for Qt6 library directory
    QT6_LIBDIR=""
    for pattern in "${QT6_LIB_PATHS[@]}"; do
        for path in $pattern; do
            if [ -f "$path/libQt6Widgets.so" ] || [ -f "$path/libQt6Core.so" ] || [ -f "$path/libQt6Widgets.so.6" ]; then
                QT6_LIBDIR="$path"
                log_info "Found Qt6 libs at: $path"
                break 2
            fi
        done
    done
    
    # Build flags from found paths
    if [ -n "$QT6_INCLUDE" ]; then
        QT_CFLAGS="-I$QT6_INCLUDE -I$QT6_INCLUDE/QtWidgets -I$QT6_INCLUDE/QtCore -I$QT6_INCLUDE/QtGui -fPIC"
    fi
    
    if [ -n "$QT6_LIBDIR" ]; then
        QT_LIBS="-L$QT6_LIBDIR -lQt6Widgets -lQt6Core -lQt6Gui"
    fi
    
    # Final check
    if [ -n "$QT_CFLAGS" ] && [ -n "$QT_LIBS" ]; then
        return 0
    fi
    
    return 1
}

find_moc() {
    # Try pkg-config first
    MOC_PATH=$(pkg-config --variable=libexecdir Qt6Core 2>/dev/null)/moc
    if [ -f "$MOC_PATH" ]; then
        log_info "Found moc via pkg-config: $MOC_PATH"
        return 0
    fi
    
    # Try common moc locations
    MOC_SEARCH_PATHS=(
        "/usr/lib/qt6/libexec/moc"
        "/usr/lib/qt6/moc"
        "/usr/lib/x86_64-linux-gnu/qt6/libexec/moc"
        "/usr/lib/aarch64-linux-gnu/qt6/libexec/moc"
        "/usr/lib64/qt6/libexec/moc"
        "/usr/local/lib/qt6/libexec/moc"
        "/opt/qt6/libexec/moc"
        "/opt/Qt/6.*/gcc_64/libexec/moc"
        "$HOME/Qt/6.*/gcc_64/libexec/moc"
        "/usr/libexec/moc"
        "/usr/lib/qt6/bin/moc"
        "/usr/bin/moc-qt6"
        "/usr/bin/moc"
        "/usr/local/bin/moc"
    )
    
    for pattern in "${MOC_SEARCH_PATHS[@]}"; do
        for path in $pattern; do
            if [ -f "$path" ] && [ -x "$path" ]; then
                MOC_PATH="$path"
                log_info "Found moc at: $path"
                return 0
            fi
        done
    done
    
    # Try which as last resort
    MOC_PATH=$(which moc-qt6 2>/dev/null || which moc 2>/dev/null || echo "")
    if [ -n "$MOC_PATH" ] && [ -f "$MOC_PATH" ]; then
        log_info "Found moc via which: $MOC_PATH"
        return 0
    fi
    
    return 1
}

find_qt6_macos() {
    log_info "Searching for Qt6 on macOS..."
    QT_PATH=""
    
    # Method 1: Try Homebrew (both Intel and Apple Silicon paths)
    if command -v brew &> /dev/null; then
        QT_PATH=$(brew --prefix qt@6 2>/dev/null || echo "")
        if [ -n "$QT_PATH" ] && [ -d "$QT_PATH" ]; then
            log_info "Found Qt6 via Homebrew: $QT_PATH"
        else
            QT_PATH=""
        fi
    fi
    
    # Method 2: Check standard Homebrew installation paths
    if [ -z "$QT_PATH" ]; then
        HOMEBREW_PATHS=(
            "/opt/homebrew/opt/qt@6"
            "/opt/homebrew/opt/qt"
            "/usr/local/opt/qt@6"
            "/usr/local/opt/qt"
            "/opt/homebrew/Cellar/qt@6"/*
            "/opt/homebrew/Cellar/qt"/*
            "/usr/local/Cellar/qt@6"/*
            "/usr/local/Cellar/qt"/*
        )
        for pattern in "${HOMEBREW_PATHS[@]}"; do
            for path in $pattern; do
                if [ -d "$path" ] && [ -d "$path/lib" ]; then
                    QT_PATH="$path"
                    log_info "Found Qt6 at Homebrew path: $QT_PATH"
                    break 2
                fi
            done
        done
    fi
    
    # Method 3: Check Qt official installer paths
    if [ -z "$QT_PATH" ]; then
        QT_INSTALLER_PATHS=(
            "$HOME/Qt/6."*/macos
            "$HOME/Qt/6."*/clang_64
            "/opt/Qt/6."*/macos
            "/opt/Qt/6."*/clang_64
            "/Applications/Qt/6."*/macos
            "/Applications/Qt/6."*/clang_64
        )
        for pattern in "${QT_INSTALLER_PATHS[@]}"; do
            for path in $pattern; do
                if [ -d "$path" ] && [ -d "$path/lib" ]; then
                    QT_PATH="$path"
                    log_info "Found Qt6 at Qt installer path: $QT_PATH"
                    break 2
                fi
            done
        done
    fi
    
    # Method 4: Check MacPorts paths
    if [ -z "$QT_PATH" ]; then
        MACPORTS_PATHS=(
            "/opt/local/libexec/qt6"
            "/opt/local/lib/qt6"
        )
        for path in "${MACPORTS_PATHS[@]}"; do
            if [ -d "$path" ]; then
                QT_PATH="$path"
                log_info "Found Qt6 via MacPorts: $QT_PATH"
                break
            fi
        done
    fi
    
    if [ -z "$QT_PATH" ]; then
        log_warn "Qt6 installation not found on macOS"
        log_info "To install Qt6:"
        log_info "  Homebrew: brew install qt@6"
        log_info "  MacPorts: sudo port install qt6-qtbase"
        log_info "  Or download from: https://www.qt.io/download"
        return 1
    fi
    
    # Build compiler flags - try framework style first, then library style
    if [ -d "$QT_PATH/lib/QtWidgets.framework" ]; then
        # Framework-style installation (Homebrew)
        # For frameworks, headers are inside Framework/Headers/ directories
        QT_CFLAGS="-F$QT_PATH/lib -I$QT_PATH/lib/QtWidgets.framework/Headers -I$QT_PATH/lib/QtCore.framework/Headers -I$QT_PATH/lib/QtGui.framework/Headers -iframework $QT_PATH/lib"
        QT_LIBS="-F$QT_PATH/lib -framework QtWidgets -framework QtCore -framework QtGui"
        log_info "Using Qt6 framework-style linking"
        log_info "Framework path: $QT_PATH/lib"
    elif [ -f "$QT_PATH/lib/libQt6Widgets.dylib" ]; then
        # Library-style installation
        QT_CFLAGS="-I$QT_PATH/include -I$QT_PATH/include/QtWidgets -I$QT_PATH/include/QtCore -I$QT_PATH/include/QtGui"
        QT_LIBS="-L$QT_PATH/lib -lQt6Widgets -lQt6Core -lQt6Gui"
        log_info "Using Qt6 library-style linking"
    elif [ -d "$QT_PATH/include/QtWidgets" ]; then
        # Include-style installation (Qt installer)
        QT_CFLAGS="-I$QT_PATH/include -I$QT_PATH/include/QtWidgets -I$QT_PATH/include/QtCore -I$QT_PATH/include/QtGui"
        QT_LIBS="-L$QT_PATH/lib -lQt6Widgets -lQt6Core -lQt6Gui"
        log_info "Using Qt6 include-style linking"
    else
        # Last resort: try framework style with explicit header paths
        QT_CFLAGS="-F$QT_PATH/lib -iframework $QT_PATH/lib"
        QT_LIBS="-F$QT_PATH/lib -framework QtWidgets -framework QtCore -framework QtGui"
        log_info "Using Qt6 fallback framework-style linking"
    fi
    
    # Find moc tool
    MOC_SEARCH=(
        "$QT_PATH/share/qt/libexec/moc"
        "$QT_PATH/libexec/moc"
        "$QT_PATH/bin/moc"
        "$QT_PATH/../../../libexec/moc"
        "/opt/homebrew/opt/qt@6/share/qt/libexec/moc"
        "/opt/homebrew/share/qt/libexec/moc"
        "/usr/local/opt/qt@6/share/qt/libexec/moc"
        "/usr/local/share/qt/libexec/moc"
        "/opt/local/libexec/qt6/bin/moc"
    )
    
    MOC_PATH=""
    for moc in "${MOC_SEARCH[@]}"; do
        if [ -f "$moc" ] && [ -x "$moc" ]; then
            MOC_PATH="$moc"
            log_info "Found moc at: $MOC_PATH"
            break
        fi
    done
    
    # Try which as last resort
    if [ -z "$MOC_PATH" ]; then
        MOC_PATH=$(which moc 2>/dev/null || echo "")
        if [ -n "$MOC_PATH" ]; then
            log_info "Found moc via PATH: $MOC_PATH"
        fi
    fi
    
    if [ -z "$MOC_PATH" ]; then
        log_warn "Qt moc tool not found"
        log_info "If using Homebrew, try: brew link qt@6 --force"
        return 1
    fi
    
    return 0
}

compile_gui() {
    log_step "Compiling GUI version..."
    
    QT_CFLAGS=""
    QT_LIBS=""
    MOC_PATH=""
    GL_FLAGS=""
    
    if [ "$OS" == "macos" ]; then
        find_qt6_macos
    else
        # Linux: comprehensive Qt6 search
        find_qt6
        find_moc
        GL_FLAGS=$(pkg-config --cflags --libs gl 2>/dev/null || echo "-lGL")
    fi
    
    if [ -z "$QT_CFLAGS" ] || [ -z "$QT_LIBS" ]; then
        log_warn "Qt6 not found - skipping GUI build"
        if [ "$OS" == "macos" ]; then
            log_info "macOS: Install with 'brew install qt@6' then 'brew link qt@6'"
        else
            log_info "Linux install commands:"
            log_info "  Ubuntu/Debian: sudo apt install qt6-base-dev"
            log_info "  Fedora: sudo dnf install qt6-qtbase-devel"
            log_info "  Arch: sudo pacman -S qt6-base"
        fi
        return 1
    fi
    
    if [ -z "$MOC_PATH" ] || [ ! -f "$MOC_PATH" ]; then
        log_warn "Qt MOC tool not found - skipping GUI build"
        if [ "$OS" == "macos" ]; then
            log_info "Try: brew link qt@6 --force"
        fi
        return 1
    fi
    
    log_info "Generating MOC files..."
    $MOC_PATH src/GUI/mainwindow.h -o "$BUILD_DIR/moc/moc_mainwindow.cpp" 2>&1
    $MOC_PATH src/GUI/progressdialog.h -o "$BUILD_DIR/moc/moc_progressdialog.cpp" 2>&1
    
    log_info "Compiling Qt6 GUI..."
    
    if [ "$OS" == "macos" ]; then
        g++ -std=c++17 -O2 -fPIC \
            $QT_CFLAGS \
            -I src/GUI -I src/common \
            src/GUI/gui_main.cpp \
            src/GUI/mainwindow.cpp \
            "$BUILD_DIR/moc/moc_mainwindow.cpp" \
            "$BUILD_DIR/moc/moc_progressdialog.cpp" \
            $QT_LIBS \
            -o "$BUILD_DIR/bin/burn-cme-gui" 2>&1
    else
        g++ -std=c++17 -O2 -fPIC \
            $QT_CFLAGS $GL_FLAGS \
            -I src/GUI -I src/common \
            src/GUI/gui_main.cpp \
            src/GUI/mainwindow.cpp \
            "$BUILD_DIR/moc/moc_mainwindow.cpp" \
            "$BUILD_DIR/moc/moc_progressdialog.cpp" \
            $QT_LIBS $GL_FLAGS \
            -o "$BUILD_DIR/bin/burn-cme-gui" 2>&1
    fi
    
    if [ -f "$BUILD_DIR/bin/burn-cme-gui" ]; then
        chmod +x "$BUILD_DIR/bin/burn-cme-gui"
        log_info "GUI compiled successfully: $BUILD_DIR/bin/burn-cme-gui"
        return 0
    else
        log_warn "GUI compilation failed"
        return 1
    fi
}

create_symlinks() {
    log_step "Creating convenience symlinks..."
    
    if [ -f "$BUILD_DIR/bin/burn-cme" ]; then
        ln -sf "$BUILD_DIR/bin/burn-cme" ./burn-cme
        log_info "Created symlink: ./burn-cme"
    fi
    
    if [ -f "$BUILD_DIR/bin/burn-cme-gui" ]; then
        ln -sf "$BUILD_DIR/bin/burn-cme-gui" ./burn-cme-gui
        log_info "Created symlink: ./burn-cme-gui"
    fi
}

install_icons() {
    log_step "Setting up application icons..."
    
    if [ ! -f "images/burn-cme-icon.png" ]; then
        log_warn "Icon file not found: images/burn-cme-icon.png"
        return 1
    fi
    
    if [ "$OS" == "macos" ]; then
        log_info "Creating macOS app bundle..."
        
        APP_BUNDLE="$BUILD_DIR/bin/Burn-CME.app"
        mkdir -p "$APP_BUNDLE/Contents/MacOS"
        mkdir -p "$APP_BUNDLE/Contents/Resources"
        
        cp "$BUILD_DIR/bin/burn-cme-gui" "$APP_BUNDLE/Contents/MacOS/Burn-CME"
        
        if command -v sips &> /dev/null && command -v iconutil &> /dev/null; then
            ICONSET_DIR="$BUILD_DIR/Burn-CME.iconset"
            mkdir -p "$ICONSET_DIR"
            
            sips -z 16 16     images/burn-cme-icon.png --out "$ICONSET_DIR/icon_16x16.png" 2>/dev/null || true
            sips -z 32 32     images/burn-cme-icon.png --out "$ICONSET_DIR/icon_16x16@2x.png" 2>/dev/null || true
            sips -z 32 32     images/burn-cme-icon.png --out "$ICONSET_DIR/icon_32x32.png" 2>/dev/null || true
            sips -z 64 64     images/burn-cme-icon.png --out "$ICONSET_DIR/icon_32x32@2x.png" 2>/dev/null || true
            sips -z 128 128   images/burn-cme-icon.png --out "$ICONSET_DIR/icon_128x128.png" 2>/dev/null || true
            sips -z 256 256   images/burn-cme-icon.png --out "$ICONSET_DIR/icon_128x128@2x.png" 2>/dev/null || true
            sips -z 256 256   images/burn-cme-icon.png --out "$ICONSET_DIR/icon_256x256.png" 2>/dev/null || true
            sips -z 512 512   images/burn-cme-icon.png --out "$ICONSET_DIR/icon_256x256@2x.png" 2>/dev/null || true
            sips -z 512 512   images/burn-cme-icon.png --out "$ICONSET_DIR/icon_512x512.png" 2>/dev/null || true
            sips -z 1024 1024 images/burn-cme-icon.png --out "$ICONSET_DIR/icon_512x512@2x.png" 2>/dev/null || true
            
            iconutil -c icns "$ICONSET_DIR" -o "$APP_BUNDLE/Contents/Resources/Burn-CME.icns" 2>/dev/null || true
            rm -rf "$ICONSET_DIR"
        else
            log_warn "sips/iconutil not available - using PNG icon directly"
            cp images/burn-cme-icon.png "$APP_BUNDLE/Contents/Resources/"
        fi
        
        cat > "$APP_BUNDLE/Contents/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>Burn-CME</string>
    <key>CFBundleIconFile</key>
    <string>Burn-CME</string>
    <key>CFBundleIdentifier</key>
    <string>com.burncme.app</string>
    <key>CFBundleName</key>
    <string>Burn-CME</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF
        
        log_info "Created macOS app bundle: $APP_BUNDLE"
        
    else
        log_info "Setting up Linux desktop integration..."
        
        mkdir -p "$BUILD_DIR/share/icons"
        mkdir -p "$BUILD_DIR/share/applications"
        
        cp images/burn-cme-icon.png "$BUILD_DIR/share/icons/burn-cme.png"
        cp burn-cme.desktop "$BUILD_DIR/share/applications/"
        
        log_info "Desktop files created in: $BUILD_DIR/share/"
        log_info "To install system-wide, run:"
        log_info "  sudo cp $BUILD_DIR/share/icons/burn-cme.png /usr/share/pixmaps/"
        log_info "  sudo cp $BUILD_DIR/share/applications/burn-cme.desktop /usr/share/applications/"
    fi
    
    return 0
}

print_summary() {
    echo ""
    echo "=========================================="
    echo "  Build Complete"
    echo "=========================================="
    echo ""
    echo "Operating System: $OS"
    [ -n "$DISTRO" ] && echo "Distribution: $DISTRO"
    echo ""
    echo "Build Output: $BUILD_DIR/bin/"
    echo ""
    
    if [ -f "$BUILD_DIR/bin/burn-cme" ]; then
        echo -e "${GREEN}[OK]${NC} CLI:  $BUILD_DIR/bin/burn-cme"
    else
        echo -e "${RED}[FAIL]${NC} CLI build failed"
    fi
    
    if [ -f "$BUILD_DIR/bin/burn-cme-gui" ]; then
        echo -e "${GREEN}[OK]${NC} GUI:  $BUILD_DIR/bin/burn-cme-gui"
    else
        echo -e "${YELLOW}[SKIP]${NC} GUI (Qt6 not available or not selected)"
    fi
    
    echo ""
}

prompt_run_program() {
    echo ""
    echo -e "${CYAN}┌─────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│       Installation Complete!            │${NC}"
    echo -e "${CYAN}└─────────────────────────────────────────┘${NC}"
    echo ""
    
    HAS_CLI=false
    HAS_GUI=false
    
    [ -f "$BUILD_DIR/bin/burn-cme" ] && HAS_CLI=true
    [ -f "$BUILD_DIR/bin/burn-cme-gui" ] && HAS_GUI=true
    
    if [ "$HAS_CLI" = true ] && [ "$HAS_GUI" = true ]; then
        echo "Would you like to run the program now?"
        echo ""
        echo "  1. Run CLI version"
        echo "  2. Run GUI version"
        echo "  3. Exit without running"
        echo ""
        echo -n "Enter your choice [1-3]: "
        read -r run_choice
        
        case $run_choice in
            1)
                echo ""
                log_info "Launching CLI version..."
                echo ""
                exec "$BUILD_DIR/bin/burn-cme"
                ;;
            2)
                echo ""
                log_info "Launching GUI version..."
                echo ""
                exec "$BUILD_DIR/bin/burn-cme-gui"
                ;;
            *)
                echo ""
                log_info "Exiting. You can run the program later with:"
                echo "  ./burn-cme        (CLI)"
                echo "  ./burn-cme-gui    (GUI)"
                ;;
        esac
    elif [ "$HAS_CLI" = true ]; then
        echo -n "Would you like to run the CLI program now? [y/N]: "
        read -r run_choice
        
        if [[ "$run_choice" =~ ^[Yy]$ ]]; then
            echo ""
            log_info "Launching CLI version..."
            echo ""
            exec "$BUILD_DIR/bin/burn-cme"
        else
            echo ""
            log_info "Exiting. You can run the program later with: ./burn-cme"
        fi
    elif [ "$HAS_GUI" = true ]; then
        echo -n "Would you like to run the GUI program now? [y/N]: "
        read -r run_choice
        
        if [[ "$run_choice" =~ ^[Yy]$ ]]; then
            echo ""
            log_info "Launching GUI version..."
            echo ""
            exec "$BUILD_DIR/bin/burn-cme-gui"
        else
            echo ""
            log_info "Exiting. You can run the program later with: ./burn-cme-gui"
        fi
    else
        log_error "No programs were built successfully."
    fi
}

show_help() {
    echo "Burn-CME Build Script"
    echo ""
    echo "Usage: ./build.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --help, -h       Show this help message"
    echo "  --no-deps        Skip dependency installation"
    echo "  --clean          Clean build directory only"
    echo "  --cli-only       Build CLI version only (non-interactive)"
    echo "  --gui-only       Build GUI version only (non-interactive)"
    echo "  --full           Build both CLI and GUI (non-interactive)"
    echo "  --no-run         Skip the run prompt at the end"
    echo ""
    echo "Interactive mode:"
    echo "  Run ./build.sh without options for interactive menu"
    echo ""
}

NO_DEPS=false
CLEAN_ONLY=false
BUILD_CLI=false
BUILD_GUI=false
INTERACTIVE=true
NO_RUN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --no-deps)
            NO_DEPS=true
            shift
            ;;
        --clean)
            CLEAN_ONLY=true
            INTERACTIVE=false
            shift
            ;;
        --cli-only)
            BUILD_CLI=true
            BUILD_GUI=false
            INTERACTIVE=false
            shift
            ;;
        --gui-only)
            BUILD_CLI=false
            BUILD_GUI=true
            INTERACTIVE=false
            shift
            ;;
        --full)
            BUILD_CLI=true
            BUILD_GUI=true
            INTERACTIVE=false
            shift
            ;;
        --no-run)
            NO_RUN=true
            shift
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

if [ "$INTERACTIVE" = true ]; then
    show_menu
    read -r menu_choice
    
    case $menu_choice in
        1)
            BUILD_CLI=true
            BUILD_GUI=true
            ;;
        2)
            BUILD_CLI=true
            BUILD_GUI=false
            ;;
        3)
            BUILD_CLI=false
            BUILD_GUI=true
            ;;
        4)
            CLEAN_ONLY=true
            ;;
        5)
            echo ""
            log_info "Exiting..."
            exit 0
            ;;
        *)
            log_error "Invalid choice. Please enter 1-5."
            exit 1
            ;;
    esac
fi

echo ""
echo "=========================================="
echo "  Burn-CME Build Script"
echo "  CD/DVD Burning Utility"
echo "=========================================="
echo ""

detect_os
check_sudo

if [ "$CLEAN_ONLY" = true ]; then
    log_step "Cleaning build artifacts..."
    rm -rf build burn-cme burn-cme-gui
    log_info "Clean complete"
    exit 0
fi

if [ "$NO_DEPS" = false ]; then
    install_dependencies
fi

create_build_dir

CLI_OK=false
GUI_OK=false

if [ "$BUILD_CLI" = true ]; then
    if compile_cli; then
        CLI_OK=true
    fi
fi

if [ "$BUILD_GUI" = true ]; then
    if compile_gui; then
        GUI_OK=true
        install_icons
    fi
fi

create_symlinks
print_summary

if [ "$CLI_OK" = false ] && [ "$GUI_OK" = false ]; then
    log_error "Build failed - no programs were compiled successfully"
    exit 1
fi

if [ "$NO_RUN" = false ]; then
    prompt_run_program
fi

exit 0
