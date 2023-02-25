#!/bin/bash

sftp root@192.168.1.12 <<EOF
cd /usr/share/FileManager/
put EnhancedFileManager
EOF
