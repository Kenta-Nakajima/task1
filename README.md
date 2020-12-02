# task1
A task for '"robot system 2020".

### 引用
myled.cは2020年度ロボットシステム学の講義で上田先生が作成したコードを改変したものです。<br>
myled.cの168, 169, 170行目のコードはRT Corporation様のRaspberry Pi Mouse devide driver</br>
(https://github.com/rt-net/RaspberryPiMouse/blob/master/src/drivers/rtmouse.c) の585, 587行目からお借りしました。</br>

### task1
Raspberry Piにつないだスイッチで10進数の数値を入力すると</br>
2進数に変換して7セグメントLEDに表示するプログラムと、LED制御とスイッチの状態を読み取るデバイスドライバです。

#### myled.c
デバイスドライバのソースファイルです。
1. copy_from_user()で読んだ文字をもとにLEDを制御します。



#### Main.c
スイッチからの入力を二進数に変換して表示するプログラムです。
デバイスファイルmyled0からスイッチの状態を得て、LED制御のための文字を送ります。

