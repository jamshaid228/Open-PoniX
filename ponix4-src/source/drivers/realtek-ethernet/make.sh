#!/bin/sh

#for d425kt this driver not work
#
#cd r8101-1.020.00/src
#make clean
#make
#cp r8101.ko /lib/modules/3.2.12-poniX-3.0/mymodules/realtek-ethernet
#rm /lib/modules/3.2.12-poniX-3.0/kernel/drivers/net/ethernet/realtek/r8101.ko
#cd ../..

cd r8168-8.037.00/src
make clean
make
cp r8168.ko /lib/modules/3.2.12-poniX-3.0/mymodules/realtek-ethernet
rm /lib/modules/3.2.12-poniX-3.0/kernel/drivers/net/ethernet/realtek/r8168.ko
cd ../..

#for d425kt this driver not work (r8169 from kernel work fine)
#
#cd r8169-6.017.00/src
#make clean
#make
#rm /lib/modules/3.2.12-poniX-3.0/kernel/drivers/net/ethernet/realtek/r8169.ko
#cp r8169.ko /lib/modules/3.2.12-poniX-3.0/mymodules/realtek-ethernet
#cd ../..

