SUMMARY = "LLVM compiler framework (packaged with rust)"
LICENSE = "NCSA"

LIC_FILES_CHKSUM = "file://LICENSE.TXT;md5=47e311aa9caedd1b3abf098bd7814d1d"

SRC_URI = "gitsm://github.com/rust-lang/rust.git;protocol=https"
SRCREV = "a59de37e99060162a2674e3ff45409ac73595c0e"

S = "${WORKDIR}/git/src/llvm"
PV .= "+git${SRCPV}"

inherit autotools
inherit native

EXTRA_OECONF += "--enable-targets=x86,x86_64,arm,aarch64,mips,powerpc"
EXTRA_OECONF += "--enable-optimized"

do_install_append () {
	cd ${D}${bindir}
	ln -s *-llc llc
	for i in *-llvm-*; do
		link=$(echo $i | sed -e 's/.*-llvm-\(.*\)/\1/')
		ln -s $i llvm-$link
	done
}
