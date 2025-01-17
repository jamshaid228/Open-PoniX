
top_builddir=/source/util-linux-2.21.2
top_srcdir=/source/util-linux-2.21.2

# Misc settings
TS_TESTUSER=${TS_TESTUSER:-"test"}

# helpers
TS_HELPER_SYSINFO="$top_builddir/tests/helpers/test_sysinfo"
TS_HELPER_PATHS="$top_builddir/tests/helpers/test_pathnames"
TS_HELPER_BYTESWAP="$top_builddir/tests/helpers/test_byteswap"
TS_HELPER_MD5="$top_builddir/tests/helpers/test_md5"

TS_HELPER_ISMOUNTED="$top_builddir/lib/test_ismounted"
TS_HELPER_STRUTILS="$top_builddir/lib/test_strutils"
TS_HELPER_CPUSET="$top_builddir/lib/test_cpuset"

# libmount
TS_HELPER_LIBMOUNT_OPTSTR="$top_builddir/libmount/src/test_optstr"
TS_HELPER_LIBMOUNT_TAB="$top_builddir/libmount/src/test_tab"
TS_HELPER_LIBMOUNT_UTILS="$top_builddir/libmount/src/test_utils"
TS_HELPER_LIBMOUNT_LOCK="$top_builddir/libmount/src/test_lock"
TS_HELPER_LIBMOUNT_UPDATE="$top_builddir/libmount/src/test_tab_update"
TS_HELPER_LIBMOUNT_CONTEXT="$top_builddir/libmount/src/test_context"
TS_HELPER_LIBMOUNT_TABDIFF="$top_builddir/libmount/src/test_tab_diff"

TS_HELPER_ISLOCAL="$top_builddir/login-utils/test_islocal"
TS_HELPER_LOGINDEFS="$top_builddir/login-utils/test_logindefs"

# TODO: use partx
TS_HELPER_PARTITIONS="$top_builddir/libblkid/samples/partitions"

U_L_LIBRARY_PATH="$top_builddir/libblkid/src/.libs:$top_builddir/libuuid/src/.libs"

# paths to commands
if [ -x "$top_builddir/sys-utils/mount" ]; then
 TS_CMD_MOUNT=${TS_CMD_MOUNT:-"$top_builddir/sys-utils/mount"}
 TS_CMD_UMOUNT=${TS_CMD_UMOUNT:-"$top_builddir/sys-utils/umount"}
else
 TS_CMD_MOUNT=${TS_CMD_MOUNT:-"$top_builddir/mount/mount"}
 TS_CMD_UMOUNT=${TS_CMD_UMOUNT:-"$top_builddir/mount/umount"}
 TS_CMD_MTABLOCK=${TS_CMD_MTABLOCK:-"$top_builddir/mount/mtab_lock_test"}
fi

TS_CMD_SWAPON=${TS_CMD_SWAPON:-"$top_builddir/sys-utils/.libs/swapon"}
TS_CMD_SWAPOFF=${TS_CMD_SWAPOFF:-"$top_builddir/sys-utils/.libs/swapoff"}
TS_CMD_LOSETUP=${TS_CMD_LOSETUP:-"$top_builddir/sys-utils/losetup"}

TS_CMD_MKSWAP=${TS_CMD_MKSWAP:-"$top_builddir/disk-utils/mkswap"}
TS_CMD_MKCRAMFS=${TS_CMD_MKCRAMFS:-"$top_builddir/disk-utils/mkfs.cramfs"}
TS_CMD_MKMINIX=${TS_CMD_MKMINIX:-"$top_builddir/disk-utils/mkfs.minix"}
TS_CMD_FSCKCRAMFS=${TS_CMD_FSCKCRAMFS:-"$top_builddir/disk-utils/fsck.cramfs"}
TS_CMD_FSCKMINIX=${TS_CMD_FSCKMINIX:-"$top_builddir/disk-utils/fsck.minix"}

TS_CMD_IPCS=${TS_CMD_IPCS:-"$top_builddir/sys-utils/ipcs"}

TS_CMD_COL=${TS_CMD_COL:-"$top_builddir/text-utils/col"}
TS_CMD_COLUMN=${TS_CMD_COLUMN:-"$top_builddir/text-utils/column"}
TS_CMD_COLRM=${TS_CMD_COLRM:-"$top_builddir/text-utils/colrm"}

TS_CMD_NAMEI=${TS_CMD_NAMEI-"$top_builddir/misc-utils/namei"}
TS_CMD_LOOK=${TS_CMD_LOOK-"$top_builddir/misc-utils/look"}
TS_CMD_CAL=${TS_CMD_CAL-"$top_builddir/misc-utils/cal"}
TS_CMD_SCRIPT=${TS_CMD_SCRIPT-"$top_builddir/term-utils/script"}


TS_CMD_HWCLOCK=${TS_CMD_HWCLOCK-"$top_builddir/hwclock/hwclock"}
TS_CMD_LSCPU=${TS_CMD_LSCPU-"$top_builddir/sys-utils/lscpu"}

TS_CMD_BLKID=${TS_CMD_BLKID-"$top_builddir/misc-utils/blkid"}
TS_CMD_PARTX=${TS_CMD_PARTX-"$top_builddir/partx/partx"}
TS_CMD_FINDMNT=${TS_CMD_FINDMNT-"$top_builddir/misc-utils/findmnt"}

TS_CMD_FDISK=${TS_CMD_FDISK-"$top_builddir/fdisk/fdisk"}

