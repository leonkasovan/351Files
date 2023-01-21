#!/bin/bash

sftp root@192.168.0.102 <<EOF
cd /usr/share/FileManager/
put EnhancedFileManager
EOF
