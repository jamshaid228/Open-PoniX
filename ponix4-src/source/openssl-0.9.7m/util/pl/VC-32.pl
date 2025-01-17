#!/usr/local/bin/perl
# VCw32lib.pl - the file for Visual C++ 4.[01] for windows NT, static libraries
#

$ssl=	"ssleay32";

my $mwex =" \$(FIPSLIB_D)${o}_chkstk.o \$(FIPSLIB_D)${o}_udivdi3.o \$(FIPSLIB_D)${o}_umoddi3.o";

if ($fips && !$shlib)
	{
	$crypto="libeayfips32";
	$crypto_compat = "libeaycompat32.lib";
	}
else
	{
	$crypto="libeay32";
	}

$o='\\';
$cp='copy nul+';	# Timestamps get stuffed otherwise
$rm='del';

$zlib_lib="zlib1.lib";

# C compiler stuff
$cc='cl';
$cflags=' /MD /W3 /WX /Ox /O2 /Ob2 /Gs0 /GF /Gy /nologo -DOPENSSL_SYSNAME_WIN32 -DWIN32_LEAN_AND_MEAN -DL_ENDIAN -DDSO_WIN32';
$cflags.=' -D_CRT_SECURE_NO_DEPRECATE';		# shut up VC8
$cflags.=' -D_CRT_NONSTDC_NO_DEPRECATE';	# shut up VC8
$lflags="/nologo /subsystem:console /machine:I386 /opt:ref";
$mlflags='';

$out_def="out32";
$tmp_def="tmp32";
$inc_def="inc32";

if ($debug)
	{
	$cflags=" /MDd /W3 /WX /Zi /Yd /Od /nologo -DOPENSSL_SYSNAME_WIN32 -D_DEBUG -DL_ENDIAN -DWIN32_LEAN_AND_MEAN -DDEBUG -DDSO_WIN32";
	$lflags.=" /debug";
	$mlflags.=' /debug';
	}
$cflags .= " -DOPENSSL_SYSNAME_WINNT" if $NT == 1;

$obj='.obj';
$ofile="/Fo";

# EXE linking stuff
$link="link";
$efile="/out:";
$exep='.exe';
if ($no_sock)
	{ $ex_libs=""; }
else	{ $ex_libs="wsock32.lib user32.lib gdi32.lib"; }

# static library stuff
$mklib='lib';
$ranlib='';
$plib="";
$libp=".lib";
$shlibp=($shlib)?".dll":".lib";
$lfile='/out:';

$shlib_ex_obj="";
$app_ex_obj="setargv.obj";
if ($nasm) {
	$asm='nasmw -f win32';
	$afile='-o ';
} else {
	$asm='ml /Cp /coff /c /Cx';
	$asm.=" /Zi" if $debug;
	$afile='/Fo';
}

$bn_asm_obj='';
$bn_asm_src='';
$des_enc_obj='';
$des_enc_src='';
$bf_enc_obj='';
$bf_enc_src='';


if (!$no_asm && !$fips)
	{
	$bn_asm_obj='crypto\bn\asm\bn_win32.obj';
	$bn_asm_src='crypto\bn\asm\bn_win32.asm';
	$des_enc_obj='crypto\des\asm\d_win32.obj crypto\des\asm\y_win32.obj';
	$des_enc_src='crypto\des\asm\d_win32.asm crypto\des\asm\y_win32.asm';
	$bf_enc_obj='crypto\bf\asm\b_win32.obj';
	$bf_enc_src='crypto\bf\asm\b_win32.asm';
	$cast_enc_obj='crypto\cast\asm\c_win32.obj';
	$cast_enc_src='crypto\cast\asm\c_win32.asm';
	$rc4_enc_obj='crypto\rc4\asm\r4_win32.obj';
	$rc4_enc_src='crypto\rc4\asm\r4_win32.asm';
	$rc5_enc_obj='crypto\rc5\asm\r5_win32.obj';
	$rc5_enc_src='crypto\rc5\asm\r5_win32.asm';
	$md5_asm_obj='crypto\md5\asm\m5_win32.obj';
	$md5_asm_src='crypto\md5\asm\m5_win32.asm';
	$sha1_asm_obj='crypto\sha\asm\s1_win32.obj';
	$sha1_asm_src='crypto\sha\asm\s1_win32.asm';
	$rmd160_asm_obj='crypto\ripemd\asm\rm_win32.obj';
	$rmd160_asm_src='crypto\ripemd\asm\rm_win32.asm';
	$cflags.=" -DBN_ASM -DMD5_ASM -DSHA1_ASM -DRMD160_ASM";
	}

if ($shlib)
	{
	$mlflags.=" $lflags /dll";
#	$cflags =~ s| /MD| /MT|;
	$lib_cflag=" -D_WINDLL";
	$out_def="out32dll";
	$tmp_def="tmp32dll";
	}

$cflags.=" /Fd$out_def";

sub do_lib_rule
	{
	local($objs,$target,$name,$shlib,$ign,$base_addr) = @_;
	local($ret,$Name);

	$taget =~ s/\//$o/g if $o ne '/';
	($Name=$name) =~ tr/a-z/A-Z/;
	my $base_arg;
	if ($base_addr ne "")
		{
		$base_arg= " /base:$base_addr";
		}
	else
		{
		$base_arg = "";
		}


#	$target="\$(LIB_D)$o$target";
	if (!$shlib)
		{
#		$ret.="\t\$(RM) \$(O_$Name)\n";
		$ret.="$target: $objs\n";
		$ex =' advapi32.lib';
		$ex.= $mwex if $fips && !$fipscanisterbuild && $target =~ /O_CRYPTO/;
		$ret.="\t\$(MKLIB) $lfile$target @<<\n  $objs $ex\n<<\n";
		}
	else
		{
		local($ex)=($target =~ /O_SSL/)?' $(L_CRYPTO)':'';
		$ex.=' wsock32.lib gdi32.lib advapi32.lib user32.lib';
 		$ex.=" $zlib_lib" if $zlib_opt == 1 && $target =~ /O_CRYPTO/;
 		if ($fips && $target =~ /O_CRYPTO/)
			{
			$ex.= $mwex unless $fipscanisterbuild;
			$ret.="$target: $objs \$(PREMAIN_DSO_EXE)\n";
			$ret.="\tSET FIPS_LINK=\$(LINK)\n";
			$ret.="\tSET FIPS_CC=\$(CC)\n";
			$ret.="\tSET FIPS_CC_ARGS=/Fo\$(OBJ_D)${o}fips_premain.obj \$(SHLIB_CFLAGS) -c\n";
			$ret.="\tSET PREMAIN_DSO_EXE=\$(PREMAIN_DSO_EXE)\n";
			$ret.="\tSET FIPS_SHA1_EXE=\$(FIPS_SHA1_EXE)\n";
			$ret.="\tSET FIPS_TARGET=$target\n";
			$ret.="\tSET FIPSLIB_D=\$(FIPSLIB_D)\n";
			$ret.="\t\$(FIPSLINK) \$(MLFLAGS) $base_arg $efile$target ";
			$ret.="/def:ms/${Name}.def @<<\n  \$(SHLIB_EX_OBJ) $objs ";
			$ret.="\$(OBJ_D)${o}fips_premain.obj $ex\n<<\n";
			}
		else
			{
			$ret.="$target: $objs\n";
			$ret.="\t\$(LINK) \$(MLFLAGS) $base_arg $efile$target /def:ms/${Name}.def @<<\n  \$(SHLIB_EX_OBJ) $objs $ex\n<<\n";
			}
		}
	$ret.="\n";
	return($ret);
	}

sub do_link_rule
	{
	local($target,$files,$dep_libs,$libs,$standalone)=@_;
	local($ret,$_);
	$file =~ s/\//$o/g if $o ne '/';
	$n=&bname($targer);
	$ret.="$target: $files $dep_libs\n";
	if ($standalone)
		{
		$ret.="  \$(LINK) \$(LFLAGS) $efile$target @<<\n\t";
		$ret.= "$mwex advapi32.lib " if ($files =~ /O_FIPSCANISTER/ && !$fipscanisterbuild);
		$ret.="$files $libs\n<<\n";
		}
	elsif ($fips && !$shlib)
		{
		$ret.="\tSET FIPS_LINK=\$(LINK)\n";
		$ret.="\tSET FIPS_CC=\$(CC)\n";
		$ret.="\tSET FIPS_CC_ARGS=/Fo\$(OBJ_D)${o}fips_premain.obj \$(SHLIB_CFLAGS) -c\n";
		$ret.="\tSET PREMAIN_DSO_EXE=\n";
		$ret.="\tSET FIPS_TARGET=$target\n";
		$ret.="\tSET FIPS_SHA1_EXE=\$(FIPS_SHA1_EXE)\n";
		$ret.="\tSET FIPSLIB_D=\$(FIPSLIB_D)\n";
		$ret.="  \$(FIPSLINK) \$(LFLAGS) $efile$target @<<\n";
		$ret.="  \$(APP_EX_OBJ) $files \$(OBJ_D)${o}fips_premain.obj $libs\n<<\n";
		}
	else
		{
		$ret.="  \$(LINK) \$(LFLAGS) $efile$target @<<\n";
		$ret.="  \$(APP_EX_OBJ) $files $libs\n<<\n";
		}
	$ret.="\n";
	return($ret);
	}

sub do_rlink_rule
	{
	local($target,$files,$dep_libs,$libs)=@_;
	local($ret,$_);

	$file =~ s/\//$o/g if $o ne '/';
	$n=&bname($targer);
	$ret.="$target: $files $dep_libs \$(FIPS_SHA1_EXE)\n";
	$ret.="\t\$(MKCANISTER) $target <<\n";
	$ret.="INPUT($files)\n<<\n";
	$ret.="\t\$(FIPS_SHA1_EXE) $target > ${target}.sha1\n";
	$ret.="\tperl util${o}copy.pl -stripcr fips-1.0${o}fips_premain.c \$(LIB_D)${o}fips_premain.c\n";
	$ret.="\t\$(CP) fips-1.0${o}fips_premain.c.sha1 \$(LIB_D)${o}fips_premain.c.sha1\n";
	$ret.="\n";
	return($ret);
	}


1;
