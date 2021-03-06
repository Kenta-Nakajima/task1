# task1
A task for "robot system 2020".

### 引用
myled.cは2020年度ロボットシステム学の講義で上田先生が作成されたコードを改変したものです。<br>
myled.cの168, 169, 170行目のコードはRT Corporation様のRaspberry Pi Mouse devide driver</br>
(https://github.com/rt-net/RaspberryPiMouse/blob/master/src/drivers/rtmouse.c) の585, 587行目からお借りしました。</br>
Makefileは講義内で上田先生が作成されたものを流用しています。<br>
Main.c内のファイルを開く関数(36行目)の作成では次のサイトを参考にしました。</br>
https://qiita.com/Reed_X1319RAY/items/2577cb1e2eb670f4bd3f　</br>
https://programming-place.net/ppp/contents/c/040.html#rw_open　</br>

### task1概要
task1にはデバイスドライバ(myled.c)と、myled.cを利用したプログラム(Main.c)、デバイスドライバをコンパイルするためのMakefileなどが含まれます。</br>
  1. myled.cは主に次の2つの役割を持ちます。</br>
  ① Raspberry Piに接続されたスイッチのON/OFF状態をデバイスファイルに書き込む</br>
  ② Raspberry Piに接続されたLEDをデバイスファイルから送られてくる指令に従って制御する</br>
  2. Main.cは10進数を2進数に変換して7セグメントLEDに表示するプログラムです。10進数の入力はRaspberry Piに接続したスイッチで行います。</br>
  3. Makefileはデバイスドライバをコンパイルするためのファイルです。Linuxのシェルで"$ make"を実行するとコンパイルが行われます。また、"$ make clean"で作成したファイルを削除できます。</br>

### 回路
task1のために作成した回路の外観を図1に示します。使用したコンピュータはRaspberry Pi 4、接続した電気部品はLEDとタクトスイッチです。</br>
<img src="https://github.com/Kenta-Nakajima/task1/blob/main/Pictures/Pic1.jpg"> <center>図1: 作成した回路の外観</center> </br>

図1の中で一番左にある部品が7セグメントLEDです。中央の2つ並んだLEDのうち左側がLED1、右側がLED2です。</br>
右側で3つ並んでいる部品がタクトスイッチです。左側からSW1、SW2、SW3です。</br>

次に、回路図を図2に示します。</br>
<img src="https://github.com/Kenta-Nakajima/task1/blob/main/Pictures/Pic2.jpg"> <center>図2: 回路図</center> </br>

図2の各部品に書かれた数字はRaspberry PiのGPIOピンの番号を表しています。各部品はその番号のGPIOピンに接続されています。 </br>
LED1,LED2のGPIOピンの出力をONにする(電圧をかける)とLEDが点灯します。</br>
スイッチに接続されているピンはマイコン内でプルアップされており、スイッチが押されると回路がGNDに繋がって"0"状態となります。</br>
7セグメントLEDに接続されたGPIOピンはLEDのGNDとして機能するので、GPIOピンの出力をOFFにする(電圧をかけない)とLEDが点灯します。</br>

### インストール
Ubuntuでの利用方法を説明します。予め"$ make"でmyled.koが作成されている必要があります。</br>
"$ sudo insmod myled.ko"を実行することでデバイスドライバのインストールが行われます。</br>
"$ sudo chmod 666 /dev/myled0"で、デバイスファイルへの書き込みを許可します。</br>
"$ gcc Main.c -o Main"でMain.cの実行ファイルを作成します。</br>
ここまでの処理は"$ ./first.bash"で1度に行えます。</br>
"$ ./Main"でMain.cの実行ファイルを実行します。</br>
"$ sudo rmmod myled"でデバイスドライバのアンインストールが行えます。</br>

### ファイルの説明
#### myled.c
デバイスドライバのソースファイルです。</br>
##### 機能：</br>
##### 1. スイッチの状態を送信</br>
  1. 関数sushi_read()内の168～170行目で３つのスイッチの状態(押されている：0, 押されていない：1)を確認する。</br>
  2. それぞれのスイッチの状態に応じた文字を指定する。SW1が押されているときは'1'、SW2なら'2'、SW3なら'3'の文字をsendに代入する。</br>
  3. 177行目で送信文字を用意して179行目のcopy_to_user()でデバイスファイルに文字列を書き込む。</br>

##### 2. LEDを制御</br>
  1. デバイスファイルに書き込みがあったときに関数led_write()が呼び出される。</br>
  2. 124行目のcopy_form_user()で1文字読み込んでcに代入する。</br>
  3. 文字が0～9のときは7セグメントLEDをその数字の通りに点灯させ、'.'のときは7セグの'.'を点灯、'-'なら7セグを消灯させる。</br>7セグメントLEDは64行目の関数seg()で行う。</br>
  4. LED1は'a'で点灯、'b'で消灯して、LED2は'c'で点灯、'd'で消灯する。</br>

##### そのほかの関数：</br>
  1. setPin()...  199行目、GPOIピンの入出力を設定する。</br>
  2. output()...  52行目、出力に設定されたGPIOピンの電圧ON/OFFを切り替え。</br>
</br>

#### Main.c
スイッチから入力された10進数を2進数に変換して表示するプログラムです。</br>
デバイスファイルmyled0からスイッチの状態を読み取り、LED制御のための文字を送ります。</br>
##### 処理の流れ：</br>
  スイッチで10進数を入力 → 10進数を2進数に変換して、それぞれのビットを文字列配列に格納 → 1文字ずつデバイスファイルに書き込む。</br>
  1. SW1の入力でnumの数値を0から１ずつインクリメントして、9を超えると0に戻す。これをSW2が押されるまで続ける
  2. SW2が押されるとarray[]にnumを格納して桁数(digits)をインクリメントする。</br>その後、1.の処理に戻って次の桁の入力を受け付ける。
  3. SW3が押されるまで1.,2.の処理を繰り返す。これらの処理は143～166のループで行われる。
  4. array[]の1要素に1桁ずつ格納された数字列を、int型の数値に変換してrltに格納する。(173～190)
  5. 203行目のループで10進数の数値rltを二進数に変換して1ビットずつ文字列配列s2に格納する。</br>rltを10で除算した時の商が0になるまで繰り返し、その時の余りの数値が2進数のビットとして使われる。</br>さらに233行目のfor文でs2の前後を入れ替える。</br>
  6. 2進数の桁数、2進数の数値を表示する。(230～263)</br>数値を表示するための7セグメントLEDは1つしか無いので、1桁ずつ表示したい数字をデバイスファイルに送信する。

##### 関数：</br>
  1. file2()  ...36行目、デバイスファイルを開く関数。エラーチェックを行う。</br>
  2. send_c() ...46行目、デバイスファイルに1文字書き込む関数。</br>
  3. send_n() ...55行目、デバイスファイルに数値から変換した文字を書き込む。</br>
  4. myread() ...66行目、デバイスファイルから1文字読み取る。</br>
  5. sw_check() ...76行目、スイッチの状態を確認する。</br>
  どれかのスイッチが押されたら、それが離されるまでループに入って待機する。これにより1度の押し下げで何度も反応する状態を防ぐ。</br>SW1が押されたら引数sw1に1、SW2ならsw2=1、SW3でsw3=1を代入する。

</br>
</br>
This file is part of task1.</br>
</br>
task1 is free software: you can redistribute it and/or modify</br>
it under the terms of the GNU General Public License as published by</br>
the Free Software Foundation, either version 3 of the License, or</br>
(at your option) any later version.</br>
</br>
task1 is distributed in the hope that it will be useful,</br>
but WITHOUT ANY WARRANTY; without even the implied warranty of</br>
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the</br>
GNU General Public License for more details.</br>
</br>
You should have received a copy of the GNU General Public License</br>
along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.
