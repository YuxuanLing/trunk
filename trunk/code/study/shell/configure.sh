#!/bin/bash
cat <<EOF
Usage: ./configure --prefix=/usr/bin
EOF

for opt do
    optarg="${opt#*=}"
    echo opt = $opt
    echo optarg = $optarg
done
