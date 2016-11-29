# autd3

## 依存ライブラリ

* Eigen3
* Boost
* Doxygen (オプション)

## ビルド

```
mkdir build
cd build
cmake ..
make
```

### ビルドに関する注意

* Windowsでは, 環境変数`BOOST_ROOT`を設定してBoostライブラリの位置を指定してください.

## 使い方

`example/`フォルダ内のサンプルプログラムを参照してください. 

### simple.cpp

サーバーに接続, デバイスを追加, 焦点位置の更新や変調の制御方法などが書かれています.

### holo.cpp

複数焦点の提示について例示しています.