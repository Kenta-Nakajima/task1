/*

Copyright (C) 2020, Kenta Nakajima

This file is part of task1.

task1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

task1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/io.h>

// デバイスドライバの情報
MODULE_AUTHOR("Kenta Nakajima");
MODULE_DESCRIPTION("driver for LED control.");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");


static dev_t dev;			//dev_t型のdev変数(デバイス番号を持っている)
static struct cdev cdv;			//キャラクタデバイスの構造体
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;	//GPIOアドレスをマッピングする配列


// ピンの番号
#define SW1	7
#define SW2	8
#define	SW3	14
#define LED1	25	// A...文字列受け取り時の識別
#define LED2	24	// B
#define LED3	99	// C


/* GPIOの出力をON/OFF */
void	output(u16 pin, int x){

	if(x == 0){
		gpio_base[10] = 1 << pin;	// OFF
	}
	else if(x>0){
		gpio_base[7]  = 1 << pin;	// ON
	}
}


/* 7セグメントLED制御 */
void seg(int x){	// pin=0 で LED=1

	// すべてのLEDをOFF
	output(6 , 1); output(20, 1); output(16, 1);
	output(12, 1); output(19, 1); output(13, 1); output(26, 1);
	
	switch(x){
		case 0:
			output(6 , 0); output(20, 0); output(16, 0);
			output(12, 0); output(19, 0); output(13, 0);
			break;
		case 1:
			output(6, 0); output(20, 0);
			break;
		case 2: 
			output(6 , 0); output(13, 0); output(26, 0);
			output(16 , 0); output(12, 0);
			break;
		case 3:
			output(6 , 0); output(13, 0); output(26, 0);
			output(20, 0); output(16, 0);
			break;
		case 4:
			output(6 , 0); output(20, 0); output(19, 0); output(26, 0);
			break;
		case 5:
			output(13, 0); output(19, 0); output(26, 0); output(20, 0);
			output(16, 0);
			break;
		case 6:
			output(26, 0); output(20, 0); output(16, 0); output(12, 0);
			output(13, 0); output(19, 0);
			break;
		case 7:
			output(6 , 0); output(20, 0); output(13, 0); output(19, 0);
			break;
		case 8:
			output(6 , 0); output(20, 0); output(16, 0); output(12, 0);
			output(19, 0); output(13, 0); output(26, 0);
			break;
		case 9:
			output(6 , 0); output(20, 0); output(16, 0);
			output(26, 0); output(13, 0); output(19, 0);
			break;

		case -1: // ALL LED OFF
			output(6 , 1); output(20, 1); output(16, 1); output(12, 1);
			output(19, 1); output(13, 1); output(26, 1); output(21, 1);
			break;
	}
}


/* デバイスファイルに書き込みがあった時の挙動 */
static ssize_t	led_write(struct file* filp, const char* buf, size_t count, loff_t* pos){

/* 1文字受け取る
 * 0～9で7セグON, '-'でOFF
*/
	char c;
	if(copy_from_user(&c, buf, sizeof(char))){	// 2文字読み取り
		return -EFAULT;
	}
	
	/*--- 7セグ制御 ---*/
	if(c >= 0x30 && c <= 0x39){     // 0-9のとき
		int x = c - 0x30;	// 数値に変換
		seg(x);			// 7セグON
	}
	else if(c==0x2d) seg(-1);	// '-'  7セグOFF
	/*--- 個別LED ---*/
	else if(c>=0x61 && c<=0x7a){
		switch(c){
			case 'a':
				output(LED1,1);
				break;
			case 'b':
				output(LED1,0);
				break;
			case 'c':
				output(LED2,1);
				break;
			case 'd':
				output(LED2,0);
				break;
		}
	}
	else if(c == '.'){		// 7セグの'.'
		output(21, 0);
	}

	return 1;
}


/* デバイスファイルからの読み込み */
static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos){

	/*	スイッチの状態を送信	*/
	int size=0;
	unsigned char send_buf[3];
	int sw1 = 0, sw2 = 0, sw3 = 0;
	char send;
	
	sw1 = ((gpio_base[13] & (0x01 << SW1)) != 0);	// SW1の入力, SW_ON:0, SW_OFF:1
	sw2 = ((gpio_base[13] & (0x01 << SW2)) != 0);	// SW2の入力, SW_ON:0, SW_OFF:1
	sw3 = ((gpio_base[13] & (0x01 << SW3)) != 0);	// SW3の入力, SW_ON:0, SW_OFF:1

	if(sw1==0)	send = '1';
	else if(sw2==0)	send = '2';
	else if(sw3==0)	send = '3';
	else		send = '0';
	
	sprintf(send_buf, "%c\n", send);

	if(copy_to_user(buf+size, (const  char *)send_buf, sizeof(send_buf))){
		printk(KERN_ERR "value copy_to_user failed.\n");
		return -EFAULT;
	}

	size += sizeof(send_buf);
	return size;

}


/* カーネルに関数のポインタを伝える構造体 */
static struct	file_operations led_fops = {
		.owner = THIS_MODULE,	//自分のポインタ
		.write = led_write,	//システムコールwriteが呼ばれた時に使う関数
		.read = sushi_read
};


/* ピンの入出力を決定 (初期化関数で使用) */
void	setPin(u32 pin, u16 b){		//0:入力, 1:出力

	u32 index = pin/10;
        u32 shift = (pin%10)*3;
        u32 mask = ~(0x7 << shift);     // ~(111)
        gpio_base[index] = (gpio_base[index] & mask) | (b << shift);
}


/* 初期化する関数 */
static int __init init_mod(void){

	int retval;
	retval = alloc_chrdev_region(&dev, 0, 1, "myled");	// デバイス番号をもらう

	if(retval < 0){		// エラー確認
		printk(KERN_ERR "alloc_chrdev_reagion failed.\n");
		return retval;
	}
	
	printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));
	//__FILE__:ログファイルの場所(マクロ)
	

	cdev_init(&cdv, &led_fops);		//キャラクタデバイスをつくる
	retval = cdev_add(&cdv, dev, 1);	//dev:デバイス番号

        if(retval < 0){		// エラー確認
                printk(KERN_ERR "cdev add failed. major:%d minor:%d\n", MAJOR(dev), MINOR(dev));
                return retval;
	}
	
	// /sys/classの下にmyledディレクトリ
	cls = class_create(THIS_MODULE, "myled");
	
	if(IS_ERR(cls)){	// エラー確認
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	
	// sys/class/myled/myled0  ==> udevが/dev/myled0
	device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));
	
	// ラズパイ4のGPIOのアドレス
	gpio_base = ioremap_nocache(0xfe200000, 0xA0);


	// LED setup	
	setPin(LED1, 1); output(LED1, 0);
	setPin(LED2, 1); output(LED2, 0);
	
	// SW setup	
	setPin(SW1, 0);
	setPin(SW2, 0);
	setPin(SW3, 0);

	// 7SEG LED setup
	setPin(6, 1);
	setPin(12, 1);
	setPin(13, 1);
	setPin(16, 1);
	setPin(19, 1);
	setPin(20, 1);
	setPin(21, 1);
	setPin(26, 1);
	seg(-1);

	return 0;
}


/* 後始末する関数 */
static void __exit cleanup_mod(void){

	// キャラクタデバイスの後始末
	cdev_del(&cdv);
	device_destroy(cls,dev);
	class_destroy(cls);

	//デバイス番号を返す
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}


/* OSに与える(マクロに登録) */
module_init(init_mod);
module_exit(cleanup_mod);
