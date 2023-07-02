# 351Files
File Manager for Anbernic device RG353P based on SDL2

### Built on WSL2 Debian
1. `git clone https://github.com/leonkasovan/351Files.git`  
2. `cd 351Files`  
3. Build using make: `PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig" make`  
4. Compress package: `7zz a FileManager.7z EnhancedFileManager FileManager/`  

### Upload package to github  
1. Go to https://github.com/leonkasovan/351Files/releases/edit/v202307  

### Manual Install on RG353P  
1. copy binary `EnhanceFileManager` to `/userdata/roms/bin`  
2. copy resource directory `res`  to `/userdata/roms/bin/FileManager`

### Install from github Release with bash script on RG353P
```bash
cd /userdata/roms/bin/
wget --show-progress -N https://github.com/leonkasovan/351Files/releases/download/v202307/FileManager.7z
7zr x -y FileManager.7z
mv magic /etc/magic
mv magic.mime /etc/magic.mime
mv file /usr/bin/file
mv libmagic.so.1 /usr/lib/libmagic.so.1
mkdir -p /usr/share/misc/
mv magic.mgc /usr/share/misc/magic.mgc
rm FileManager.7z
```

### Full Emulation Station Installation include FileManager
`wget https://raw.githubusercontent.com/leonkasovan/RG353P/main/rg353p_setup_batocera_es.sh -O - | /bin/bash`  
