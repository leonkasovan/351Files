#!/bin/bash

sftp root@192.168.1.15 <<EOF
cd /usr/share/FileManager/
put EnhancedFileManager
EOF
