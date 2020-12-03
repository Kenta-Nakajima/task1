obj-m := myled.o	#myled.oファイルを作る

myled.ko: myled.c	# カーネルモジュールをmyled.cを使って作る
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 clean

