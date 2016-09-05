#!/bin/bash

set -e

echo $PWD
meta_rust=`pwd`

case "$TRAVIS_BRANCH" in
    krogoth)
        OE_BRANCH="krogoth"
        ;;
    jethro)
        OE_BRANCH="jethro"
        ;;
    *)
        OE_BRANCH="master"
        ;;
esac

cd $HOME
if [ ! -e poky ]; then
    git clone git://git.yoctoproject.org/poky
    cd poky
else
    cd poky
    git fetch origin
fi
git reset --hard origin/$OE_BRANCH

cd $HOME
if [ ! -e meta-openembedded ]; then
    git clone https://github.com/openembedded/meta-openembedded.git
    cd meta-openembedded
else
    cd meta-openembedded
    git fetch origin
fi
git reset --hard origin/$OE_BRANCH

cd $HOME/poky
rm -rf build
. oe-init-build-env
echo "BBLAYERS += \"$HOME/meta-openembedded/meta-oe\"" >> conf/bblayers.conf
echo "BBLAYERS += \"$meta_rust\"" >> conf/bblayers.conf
