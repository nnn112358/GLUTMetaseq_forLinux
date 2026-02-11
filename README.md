# sample_GL - MQOモデルビューア

Metasequoia (.mqo) 形式の3DモデルをOpenGLで読み込み・表示するサンプルプログラムです。

## 概要

[GLMetaseq](https://github.com/) ライブラリを使用して MQO ファイルを読み込み、GLUT ウィンドウ上に3Dモデルをレンダリングします。デバッグ用のHUD表示、座標軸、グリッド描画、FPS計測などの機能を備えています。

## ファイル構成

| ファイル | 説明 |
|---|---|
| `sample_GL.cpp` | メインプログラム（描画・入力処理・デバッグ機能） |
| `GLMetaseq.cpp` | MQOファイルローダ本体 |
| `GLMetaseq.h` | GLMetaseqライブラリのヘッダ |
| `ninja.mqo` | サンプル3Dモデル（忍者） |
| `Makefile` | ビルド設定 |
| `LICENCE.txt` | ライセンス情報 |

## 依存ライブラリ

- OpenGL
- GLU
- GLUT (freeglut)

### インストール例 (Ubuntu/Debian)

```bash
sudo apt install freeglut3-dev libgl-dev libglu1-mesa-dev
```

## ビルド

```bash
make
```

## 実行

```bash
./sample_GL
```

`ninja.mqo` が実行ディレクトリに存在する必要があります。

## 操作方法

| キー | 機能 |
|---|---|
| `Q` / `ESC` | 終了 |
| `D` | デバッグ情報をコンソールに出力 |
| `R` | 回転をリセット |
| `W` | ワイヤフレーム表示の切替 |
| `T` | デバッグオーバーレイの表示/非表示 |
| `H` | キーボードヘルプを表示 |

マウスドラッグでモデルを回転できます。

## デバッグ機能

- FPS表示（コンソール出力 + HUD）
- 座標軸表示（X=赤, Y=緑, Z=青）
- XZ平面のグリッド描画
- OpenGLエラーチェック
- OpenGL環境情報の出力
- ワイヤフレームモード

## GLMetaseq 対応仕様

- MQOファイルバージョン: Metasequoia Ver1.0 / 2.0〜2.4
- テクスチャ画像形式: BMP, TGA (JPEG/PNGはオプション)
- テクスチャサイズ: 一辺が2のn乗の正方形に限る
- 曲面・鏡面・回転体・メタボールは非対応

## Linux (WSL2) 対応における GLMetaseq の変更点

オリジナルの GLMetaseq は Windows 向けに書かれており、Linux 環境ではVBO関連のシンボル衝突によりセグメンテーション違反が発生していました。以下のバグ修正を行っています。

### 変更内容

- **`GLMetaseq.h`**: Linux 向けに `GL_GLEXT_PROTOTYPES` を定義し、VBO関数（`glGenBuffersARB` 等）をライブラリから直接リンクするように変更。VBO関数ポインタのグローバル変数定義を `#ifdef WIN32` で囲み、Linux 上での libGL.so とのシンボル衝突を解消。
- **`GLMetaseq.h`**: `IsExtensionSupported` 内で `glGetString(GL_EXTENSIONS)` の NULL チェックを追加。
- **`GLMetaseq.cpp`**: Linux 用 VBO 関数ポインタ初期化コードの整理（`GL_GLEXT_PROTOTYPES` により直接リンクされるため不要）。

いずれもLinux互換性のためのバグ修正であり、ライブラリの機能・APIに変更はありません。

## ライセンス

GLMetaseq ライブラリ部分は MIT License (Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi) です。MIT License の条件に基づき、上記のバグ修正を含む改変を行っています。詳細は `GLMetaseq.h` および `LICENCE.txt` を参照してください。
