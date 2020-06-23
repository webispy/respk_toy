#!/bin/sh
set -e

mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_SYSCONFDIR=/etc -DCMAKE_TOOLCHAIN_FILE=/opt/Toolchain.cmake
DESTDIR=../OUTPUT make install

