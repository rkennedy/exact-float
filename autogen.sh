#!/bin/sh

git submodule init
git submodule update
mkdir -p m4
cp \
    autoconf-archive/m4/ax_cxx_compile_stdcxx_11.m4 \
    autoconf-archive/m4/ax_append_flag.m4 \
    autoconf-archive/m4/ax_check_compile_flag.m4 \
    autoconf-archive/m4/ax_append_compile_flags.m4 \
    autoconf-archive/m4/ax_save_flags.m4 \
    autoconf-archive/m4/ax_restore_flags.m4 \
    autoconf-archive/m4/ax_require_defined.m4 \
    boost.m4/build-aux/boost.m4 \
    m4
autoreconf --force --install
