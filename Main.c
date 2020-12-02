/*

Copyright (C) 2020, Kenta Nakajima

This file is part of task1.

task1 is free software: you can redistribute it and/or modify
t under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

task1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


#define	MSTS	1000000	// micro sec から sec
#define	L1_ON	'a'	// LED制御の文字
#define	L1_OFF	'b'
#define	L2_ON	'c'
#define	L2_OFF	'd'


/* ファイルopenする関数 */
void file2(char* file_name, FILE** fp){	// FILEポインタのポインタ
	*fp = fopen(file_name, "w+");
	if(fp==NULL){
		printf("Opening device file failed.\n");
		exit(1);
	}
}


/* デバイスファイルに送る関数 */
void send_c(char sc, char* file_name, FILE** fp){	// 文字を1つ送信

	char send_buf[3];
	file2(file_name, fp);
	sprintf(send_buf, "%c\n", sc);
	fwrite(send_buf, sizeof(char), strlen(send_buf), *fp);
	fclose(*fp);
}

void send_n(int x, char* file_name, FILE** fp){		// 数字を1つ送る

	char send_buf[3];
	file2(file_name, fp);
	sprintf(send_buf, "%d\n", x);
	fwrite(send_buf, sizeof(char), strlen(send_buf), *fp);
	fclose(*fp);
}


/* デバイスファイルから読む関数 */
void myread(int* c, char* file_name, FILE** fp){

	file2(file_name, fp);
	*c = fgetc(*fp);

	fclose(*fp);
}


/* スイッチの状態を確認 */
int sw_check(int *sw1, int *sw2, int *sw3, char* file_name, FILE** fp){	// 押されたら1を返す

	int c;
	myread(&c, file_name, fp);

	if(c=='1' || c=='2' || c=='3'){		// 押された

		usleep(0.01*MSTS);		// 待つ (チャタリング防止)

		myread(&c, file_name, fp);	// もう一度読む

		if(c=='1')	*sw1=1;		// 判定
		else		*sw1=0;

		if(c=='2')	*sw2=1;
		else		*sw2=0;

		if(c=='3')	*sw3=1;
		else		*sw3=0;


		if(c=='1'){			// スイッチが離されるまで待機
			do{
				usleep(0.01*MSTS);
				myread(&c, file_name, fp);
			}while(c != '0');
		}
		if(c=='2'){
			do{
				usleep(0.01*MSTS);
				myread(&c, file_name, fp);
			}while(c != '0');
		}
		if(c=='3'){
			do{
				usleep(0.01*MSTS);
				myread(&c, file_name, fp);
			}while(c != '0');
		}

		return 1;
	
	}
	else if(c=='0'){	*sw1=0; *sw2=0;	*sw3=0;}
	return 0;
}



/* デバイスドライバmyledとセット */
int main(void){


	FILE	*fp = NULL;
	char	file_name[] = "/dev/myled0";	// デバイスファイル名
	int	sw1 = 0, sw2 = 0, sw3 = 0;	// スイッチ状態
	/*---------------------------*/
	int	num = 0;	// 入力 (SW1が押された回数)	
	int	digits = 1;	// 桁数
	int	array[20];	// 入力を格納

	send_c(L1_ON, file_name, &fp);	
	send_c(L2_OFF, file_name, &fp);
	send_n(0, file_name, &fp);


	/*** 入力受付 ***/
	while(sw3 == 0){
		
		while(sw_check(&sw1, &sw2, &sw3, file_name, &fp)==0){}	// スイッチの入力チェック
		
		/* SW1が押された */
		if(sw1==1){
			if(num>=9)	num = 0;
			else		num++;

			send_n(num, file_name, &fp);	// 7セグ

			printf("%d\n",num);
		}
		/* SW2が押された */
		else if(sw2==1){
			
			puts("SW2_set");
			array[digits-1] = num;
			num = 0;
			digits++;
			send_n(num, file_name, &fp);	// 7セグ (0)

		}
	}
	puts("sw3_set");
	send_c(L1_OFF, file_name, &fp);
	digits--;

	/* 入力を数値(10進数)へ */
	int rlt = 0;
	for(int i = 0; i < digits; i++){

		int x = 1;	// (10**X)倍する数値
		for(int j = 0; j < digits-1-i; j++){
			x *= 10;
		}

		rlt += array[i] * x;
		
		send_c(L1_ON, file_name, &fp);
		send_n(array[i], file_name, &fp);
		usleep(0.7*MSTS);
		send_c(L1_OFF, file_name, &fp);
		send_c('-', file_name, &fp);
		usleep(0.5*MSTS);


	}
	send_c('-', file_name, &fp);

	printf("入力: %d\n", rlt);

	usleep(1.0*MSTS);


	/* 2進数 */
	char s2[30];
	int remainder = 0;	// 余り
	digits = 0;		// 桁数

	do{

		remainder = rlt % 2;	// 余り
		rlt = rlt / 2;		// 商

		printf("商：%d, 余り：%d\n",rlt, remainder);

		array[digits] = remainder;
		digits++;
	}while(rlt != 0);

	printf("桁：%d\n",digits);

	for(int i = 0; i < (digits/2); i++){	// 2進数の配列をソート

		int temp = array[i];
		array[i] = array[digits-1-i];
		array[digits-1-i] = temp;
	}

	for(int i = 0; i < digits; i++){
		printf("%d", array[i]);
		send_n(array[i], file_name, &fp);
	}
	printf("\n");

	
	while(sw_check(&sw1, &sw2, &sw3, file_name, &fp)==0){


		if((digits/10) > 0){			// 桁数を表示（2桁まで）
			send_c('.', file_name, &fp);
			send_n(digits/10, file_name, &fp);
			usleep(0.7*MSTS);
			send_c('-', file_name, &fp);
			usleep(0.5*MSTS);
			send_c('.', file_name, &fp);
			send_n(digits%10, file_name, &fp);
			usleep(0.7*MSTS);
			send_c('-', file_name, &fp);
			usleep(0.5*MSTS);
		}
		else{					// 1桁
			send_c('.', file_name, &fp);
			send_n(digits, file_name, &fp);
			usleep(0.9*MSTS);
			send_c('-', file_name, &fp);
			usleep(0.8*MSTS);
		}
		usleep(0.5*MSTS);

		for(int i = 0; i < digits; i++){	// 2進数を表示
			send_c(L2_ON, file_name, &fp);
			send_n(array[i], file_name, &fp);
			usleep(0.7*MSTS);
			send_c(L2_OFF, file_name, &fp);
			send_c('-', file_name, &fp);
			usleep(0.5*MSTS);
		}
		usleep(1*MSTS);
	}

	send_c(L2_OFF, file_name, &fp);

	return 0;
}
