# 2015-03-09
SRCREV = "a59de37e99060162a2674e3ff45409ac73595c0e"
require rust-git.inc

SRC_URI_append = "\
	file://0001-filesearch-support-RUST_SYSROOT-env-var.patch \
	file://0002-platform.mk-avoid-choking-on-i586.patch \
	file://0003-Target-add-default-target.json-path-libdir-rust-targ.patch \
	file://0004-Parallelize-submake-invocations.patch \
	file://0005-std-thread_local-workaround-for-NULL-__dso_handle.patch \
	file://0006-configure-install-support-disabling-calling-of-ldcon.patch \
	file://0007-mk-install-use-disable-rewrite-paths.patch \
	file://0008-filesearch-add-info-to-show-path-searching.patch \
	file://0009-jemalloc-don-t-force-g1.patch \
	file://0010-Remove-weak-je_malloc_conf.patch \
	file://0001-src-rt-arch-i386-morestack.S-call-rust_stack_exhaust.patch \
\
	file://rust-installer/0001-add-option-to-disable-rewriting-of-install-paths.patch;patchdir=src/rust-installer \
"
