#!/bin/sh

make clean

./configure --with-redhat=no --with-suse=no --with-kernel=/lib/modules/3.16.5-PoniX-4/source --with-oss=yes \
--with-isapnp=no --with-moddir=/lib/modules/3.16.5-PoniX-4/mymodules/alsa

#  --with-kernel=dir       give the directory with kernel sources
#                          [/usr/src/linux]
#  --with-build=dir        give the directory with kernel build tree
#  --with-redhat=no,yes,auto  specify Red Hat kernel build
#  --with-suse=no,yes,auto  specify SUSE kernel build
#  --with-moddir=/path     give the path for the alsa driver kernel modules
#                          [/lib/modules/<KVER>/misc]
#  --with-debug=level      give the debug level (none,basic,full,verbose)
#  --with-isapnp=yes,no,auto  driver will (not) be compiled with ISA PnP support
#  --with-extra-version=VERSION  specifies the extra version string
#  --with-sequencer=yes,no  driver will (not) be compiled with sequencer support
#  --with-oss=no,yes       driver will (not) be compiled with OSS/Free emulation
#  --with-pcm-oss-plugins=no,yes       driver will (not) be compiled with OSS PCM plugins
#  --with-pcmcia=kernel,external   support kernel PCMCIA driver or external PCMCIA driver
#  --with-pcmcia-root=dir  specify the root directory of external PCMCIA source-tree
#  --with-cards=<list> compile driver for cards and options in <list>;
#                          cards may be separated with commas;
#                          'all' compiles all drivers;
#                          Possible cards are:
#                            seq-dummy, hrtimer, rtctimer, sbawe, emu10k1,
#                            hpet, pcsp, dummy, virmidi, mtpav, mts64,
#                            serial-u16550, mpu401, portman2x4, ml403-ac97cr,
#                            serialmidi, loopback, adlib, ad1816a, ad1848,
#                            als100, azt2320, cmi8330, cs4231, cs4236, es968,
#                            es1688, es18xx, sc6000, gusclassic, gusextreme,
#                            gusmax, interwave, interwave-stb, jazz16,
#                            opl3sa2, opti92x-ad1848, opti92x-cs4231, opti93x,
#                            miro, sb8, sb16, sgalaxy, sscape, wavefront,
#                            msnd-pinnacle, msnd-classic, pc98-cs4232, ad1889,
#                            als300, als4000, ali5451, atiixp, atiixp-modem,
#                            au8810, au8820, au8830, aw2, azt3328, bt87x,
#                            ca0106, cmipci, oxygen, cs4281, cs46xx, cs5530,
#                            cs5535audio, ctxfi, darla20, gina20, layla20,
#                            darla24, gina24, layla24, mona, mia, echo3g,
#                            indigo, indigoio, indigodj, indigoiox, indigodjx,
#                            emu10k1x, ens1370, ens1371, es1938, es1968,
#                            fm801, hda-intel, hdsp, hdspm, hifier, ice1712,
#                            ice1724, intel8x0, intel8x0m, korg1212, lx6464es,
#                            maestro3, mixart, nm256, pcxhr, riptide, rme32,
#                            rme96, rme9652, sis7019, sonicvibes, trident,
#                            via82xx, via82xx-modem, virtuoso, vx222, ymfpci,
#                            pdplus, asihpi, powermac, ps3, aoa,
#                            aoa-fabric-layout, aoa-onyx, aoa-tas, aoa-toonie,
#                            aoa-soundbus, aoa-soundbus-i2s, armaaci,
#                            pxa2xx-ac97, sa11xx-uda1341, s3c2410,
#                            atmel-abdac, atmel-ac97c, at73c213, sgi-o2,
#                            sgi-hal2, au1x00, aica, sh-dac-audio, usb-audio,
#                            usb-ua101, usb-usx2y, usb-caiaq, usb-us122l,
#                            vxpocket, pdaudiocf, sun-amd7930, sun-cs4231,
#                            sun-dbri, harmony, soc, atmel-soc,
#                            at91-soc-sam9g20-wm8731, at32-soc-playpaq,
#                            at91-soc-afeb9260, soc-au1xpsc,
#                            soc-sample-psc-ac97, bf5xx-i2s,
#                            bf5xx-soc-ssm2602, bf5xx-soc-ad73311, bf5xx-tdm,
#                            bf5xx-soc-ad1836, bf5xx-soc-ad1938, bf5xx-ac97,
#                            bf5xx-soc-ad1980, davinci-soc, davinci-soc-evm,
#                            dm6467-soc-evm, davinci-soc-sffsdr,
#                            da830-soc-evm, da850-soc-evm, soc-mpc8610-hpcd,
#                            soc-mpc5200-i2s, soc-mpc5200-ac97,
#                            mpc52xx-soc-pcm030, mpc52xx-soc-efika,
#                            mx1-mx2-soc, soc-mx27vis-wm8974, omap-soc,
#                            omap-soc-n810, omap-soc-ams-delta,
#                            omap-soc-osk5912, omap-soc-overo,
#                            omap-soc-omap2evm, omap-soc-omap3evm,
#                            omap-soc-am3517evm, omap-soc-sdp3430,
#                            omap-soc-omap3-pandora, omap-soc-omap3-beagle,
#                            omap-soc-zoom2, omap-soc-igep0020, pxa2xx-soc,
#                            pxa2xx-soc-corgi, pxa2xx-soc-spitz,
#                            pxa2xx-soc-poodle, pxa2xx-soc-tosa,
#                            pxa2xx-soc-e740, pxa2xx-soc-e750,
#                            pxa2xx-soc-e800, pxa2xx-soc-em-x270,
#                            soc-zylonite, soc-raumfeld, pxa2xx-soc-magician,
#                            pxa2xx-soc-mioa701, s3c24xx-soc,
#                            s3c24xx-soc-neo1973-wm8753,
#                            s3c24xx-soc-neo1973-gta02-wm8753,
#                            s3c24xx-soc-jive-wm8750, s3c64xx-soc-wm8580,
#                            s3c24xx-soc-smdk2443-wm9710,
#                            s3c24xx-soc-ln2440sbc-alc650,
#                            s3c24xx-soc-s3c24xx-uda134x,
#                            s3c24xx-soc-simtec-tlv320aic23,
#                            s3c24xx-soc-simtec-hermes, s6000-soc,
#                            s6000-soc-s6ipcam, soc-pcm-sh7760, soc-sh4-fsi,
#                            sh7760-ac97, soc-txx9aclc, soc-txx9aclc-generic,
#                            soc-tlv320aic26
#  --with-card-options=<list> enable driver options in <list>;
#                          options may be separated with commas;
#                          'all' enables all options;
#                          Possible options are:
#                            seq-hrtimer-default, seq-rtctimer-default,
#                            support-old-api, pcm-xrun-debug, ac97-power-save,
#                            sb16-csp, bt87x-overclock, cs46xx-new-dsp,
#                            fm801-tea575x-bool, hda-hwdep, hda-reconfig,
#                            hda-input-beep, hda-input-jack, hda-patch-loader,
#                            hda-codec-realtek, hda-codec-analog,
#                            hda-codec-sigmatel, hda-codec-via,
#                            hda-codec-atihdmi, hda-codec-nvhdmi,
#                            hda-codec-intelhdmi, hda-codec-cirrus,
#                            hda-codec-conexant, hda-codec-ca0110,
#                            hda-codec-cmedia, hda-codec-si3054, hda-generic,
#                            hda-power-save, powermac-auto-drc,
#                            usb-caiaq-input, at32-soc-playpaq-slave,
#                            bf5xx-mmap-support, bf5xx-multichan-support,
#                            bf5xx-have-cold-reset, pxa2xx-soc-palm27x,
#                            fsi-ak4642, fsi-da7210
