#!/bin/bash
PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig" make
7zz a FileManager.7z EnhancedFileManager FileManager/
echo "Upload and release FileManager.7z to https://github.com/leonkasovan/351Files"
