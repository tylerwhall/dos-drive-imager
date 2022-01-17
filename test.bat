:: bytes/sector, sectors, heads, cyl
:: 10653696 bytes
imgmount 2 DIN.IMG -t hdd -size 512,17,4,306 -fs none
image.exe 80 DOUT.IMG
exit
