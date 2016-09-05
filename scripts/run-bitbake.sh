#!/bin/bash

set -e

cd $HOME/poky
. oe-init-build-env build
echo $PWD
bitbake "$@"
