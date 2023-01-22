#!/bin/bash

sftp root@192.168.0.114 <<EOF
cd /usr/share/FileManager/
put EnhancedFileManager
EOF
