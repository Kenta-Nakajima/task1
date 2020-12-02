# task1
A task for '"robot system 2020".

### 引用
myled.cは2020年度ロボットシステム学の講義で上田先生が作成したコードを改変したものです。<br>
myled.cの168, 169, 170行目のコードはRT Corporation様のRaspberry Pi Mouse devide driver</br>
(https://github.com/rt-net/RaspberryPiMouse/blob/master/src/drivers/rtmouse.c) の585, 587行目からお借りしました。</br>

### task1概要
Raspberry Pi 4につないだスイッチで10進数の数値を入力すると</br>
2進数に変換して7セグメントLEDに表示するプログラムと、LED制御とスイッチの状態を読み取るデバイスドライバです。</br>

Raspberry Piにつながっている電気部品はLED、タクトスイッチです。外観をpic1に示します。</br>
２つ並んだLEDの左側がLED1、右側がLED2で、３つ並んだタクトスイッチは左側からSW1,SW2,SW3です。</br>

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

##### 3. そのほかの関数</br>
  1. setPin()...199行目、GPOIピンの入出力を設定する。</br>
  2. output()... 52行目、出力に設定されたGPIOピンの電圧ON/OFFを切り替え。</br>


#### Main.c
スイッチからの入力を二進数に変換して表示するプログラムです。</br>
デバイスファイルmyled0からスイッチの状態を得て、LED制御のための文字を送ります。</br>
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
along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.
