cmd_usr/gen_init_cpio := gcc -Wp,-MD,usr/.gen_init_cpio.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer     -o usr/gen_init_cpio usr/gen_init_cpio.c  

source_usr/gen_init_cpio := usr/gen_init_cpio.c

deps_usr/gen_init_cpio := \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/stdio.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/features.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/predefs.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/sys/cdefs.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/wordsize.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/gnu/stubs.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/gnu/stubs-32.h \
  /toolchain/i486-TSOL-linux-gnu/bin/../lib/gcc/i486-TSOL-linux-gnu/4.6.3/include/stddef.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/types.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/typesizes.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/libio.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/_G_config.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/wchar.h \
  /toolchain/i486-TSOL-linux-gnu/bin/../lib/gcc/i486-TSOL-linux-gnu/4.6.3/include/stdarg.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/stdio_lim.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/sys_errlist.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/stdio.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/stdlib.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/waitflags.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/waitstatus.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/endian.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/endian.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/byteswap.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/sys/types.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/time.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/sys/select.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/select.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/sigset.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/time.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/sys/sysmacros.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/pthreadtypes.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/alloca.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/sys/stat.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/stat.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/string.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/xlocale.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/string.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/string2.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/unistd.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/posix_opt.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/environments.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/confname.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/getopt.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/fcntl.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/fcntl.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/errno.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/errno.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/linux/errno.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/asm/errno.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/asm-generic/errno.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/asm-generic/errno-base.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/ctype.h \
  /toolchain/i486-TSOL-linux-gnu/bin/../lib/gcc/i486-TSOL-linux-gnu/4.6.3/include-fixed/limits.h \
  /toolchain/i486-TSOL-linux-gnu/bin/../lib/gcc/i486-TSOL-linux-gnu/4.6.3/include-fixed/syslimits.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/limits.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/posix1_lim.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/local_lim.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/linux/limits.h \
  /toolchain/i486-TSOL-linux-gnu/sysroot/usr/include/bits/posix2_lim.h \

usr/gen_init_cpio: $(deps_usr/gen_init_cpio)

$(deps_usr/gen_init_cpio):
