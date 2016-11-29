# autd3

空中超音波触覚ディスプレイ(AUTD) バージョン3ドライバの開発レポジトリです.

## 定義

### 単位

特に断りのない限り, 物理量には以下の単位を用います.

* 長さ - millimeter[mm]
* 角度 - radian[rad]
* 時間 - millisecond[msec]

### デバイスおよび超音波振動子のID

#### デバイスID

各デバイスは, EtherCATマスターに近い順番に0, 1, ...とIDが割り振られます.

#### 振動子ID

各デバイスの中で, 振動子は249個搭載されています. IDは以下のように割り振られます. ただし`*`は抜け(ネジ穴)を表します.

```
87 88 89 90 91 92 93 94 95 96 97 98 99 .......
69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86
51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68
33 34 35 36 37 38 39 40 41 43 43 44 45 46 47 48 49 50
18  *  * 19 20 21 22 23 24 25 26 27 28 29 30 31  * 32
 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
```

### 座標系

右手系の直交座標系を用います. 各AUTDデバイスはローカルに座標系を持ち, 下記のように各軸と原点が定義されます.

* xy平面は振動子表面に接する
* 長軸に平行にx軸を置く
* 短軸と平行にy軸を置く
* 原点は振動子ID-0の表面の中心

### Gain

このプロジェクトでは, `Gain`は**すべての振動子**の振幅と位相をあらわします. すなわち, 一般的に言う個々の振動子の複素ゲイン(振幅と位相)を全振動子分束ねたものになります. これは, 焦点位置などの空間変調を表すことになります.

### Modulation

このプロジェクトでは, `Modulation`は**時間方向の**振幅変調を表します. この変調は, 厳密なクロックによって駆動され, 全振動子に一斉に適用されます.

## インストール(アプリケーション開発者向け)

### TwinCAT3 and Visual Studio 2013

AUTDサーバの起動には, TwinCAT3 Engineering Environment と Visual Studio 2013 が必要です. 特にVisual Studioについては2013を必ず用いるようにしてください. その他のバージョンでは起動しません.

http://www.beckhoff.com/english.asp?download/tc3-download-xae.htm
https://www.visualstudio.com/ja-jp/downloads/download-visual-studio-vs

### クライアントライブラリ

[master/dist](master/dist)以下にヘッダファイルとコンパイル済みのstatic libraryファイルが置いてあります. autd3.hpp, autd3.lib, ads.lib, および標準のWinSock(ws2_32.libなど)をリンクして使用してください. またEigen3が別途必要です.

### コンパイル(advanced)

クライアントライブラリをコンパイルするためにはCMake(Binary distributions)が必要です. cmake-gui.exeを起動し，Source codeのところにmasterフォルダのパスを入れてください．the binariesにはmasterフォルダパス+/buildを入力し，Configureを実行後，Generateを実行すると~/build以下にautd3.slnなどが生成されます． 依存ライブラリなど詳しくは`master/README_ja.md`を参照してください.

https://cmake.org/download/

## インストール(デバイス開発者向け)

### Vivado

* Config ROM - n25q128-3.3v

### e2 Studio

## 起動手順

1. 初回のみ, `dist/AUTDServer/install.bat`を実行してください.
2. AUTDデバイスとサーバホストとをカテゴリー5e以上のRJ45ケーブルで接続してください. CN10(三つのポートのうち、真ん中のもの)はMasterに近いケーブルを、CN11は末端側へのケーブルを接続してください。
3. AUTDデバイスに24VのDC電源を接続してください. 一台のデバイスで消費する最大電流は2Aです.
4. AUTDServer起動プログラム`dist/AUTDServer/AUTDServer.exe`を起動してください. 接続されているAUTDを自動で検出しセットアップを行います. 途中でIPアドレスを聞かれた場合は, クライアントプログラムを実行するホストのIPアドレスを入力してください. ローカルホストからもTCP/IP上のリモートホストからも接続できますが, リモートの場合接続できるホストは同時に1台までです. 複数台からアクセスする場合は, TwinCATのADS Route Tableを手動で設定してください. また, リモートから接続する場合は, TCPのポート48898とUDPのポート48899を受信できるようにサーバホストのファイヤウォールを設定してください.
5. クライアントプログラムを上記で設定したホストで起動してください. クライアントアプリケーションの開発については, `client/README.md`を参照してください.

## トラブルシューティング

### AUTDServerが起動しない

TwinCATのライセンスが切れている可能性があります. AUTDServer.exeの起動後にNoを入力しTwinCAT設定画面を表示させたあと, LICENSEタブからTrial Licenseを発行してください.

下記PDFの11ページに画面つきで手順が説明されています.
https://download.beckhoff.com/download/document/catalog/TwinCAT_3_Booklet.pdf

### AUTDServer起動時のエラー：AdsWarning:4115
win8settick.batを実行していない可能性があります．エラーに表示されるパスをたどり，「管理者として実行」してください．

### AUTDがTwinCAT3上で認識されない
「I/O」→「Devices」右クリック→「Scan」で「No new I/O devices found」のときは，
ネットワークチップが認識されてないので，「TWINCAT」→「Show Realtime Ethernet Compatible Devices」を開き，現在のネットワークアダプタをdisable -> enableとしてください.

### 実行時エラー：Error on sending data: 6
AUTDServerが起動していません. 上記を参照

### 実行時エラー：Error on sending data: 705
AddDeviceの数＞実際に接続したAUTDで認識されている数　の場合に出るエラー．

### Error on sending data:
その他のエラーコード
https://infosys.beckhoff.com/english.php?content=../content/1033/tcadscommon/html/tcadscommon_intro.htm&id=
