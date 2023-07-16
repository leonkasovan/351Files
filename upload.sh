#!/bin/bash

sftp root@192.168.1.12 <<EOF
cd /userdata/roms/bin/
put EnhancedFileManager
EOF
