# BuildAll.slnのビルド方法

## 1. DCMTKをビルドする

dcmtk_src: DcmTK ソースコード（Ver3.6.0 ダウンロードしたもの）
dcmtk_proj: CMakeで生成されたVC2005プロジェクト
 注：
　　CFlag は、MT,  MTDにする。

QtはPXDcmSAdminプロジェクトに必要

CMake 2.8以上が必要。
http://www.cmake.org/

1. CMake(GUI)を起動する。
2. Source code欄に、dcmtk_srcフォルダを指定
3. build the binaries欄に、dcmtk_projフォルダを指定(すでにある)
4. Configureボタンをクリック、VC2005を選択
5. Generateボタンをクリックするとプロジェクトファイルが作成される。


## 2. PXDcmSAdminwをqmakeする
 - runQMake.batのqmakeのパスを変更する
 - runQMake.batを実行する

## 3. BuildAll.slnを開く
 - DCMLibプロジェクトの -> ｀Use of MFC` を `Use MFC in a Shared DLL` に変更する
 - PXDcmJocProcプロジェクトの -> ｀Use of MFC` を `Use MFC in a Shared DLL` に変更する
 - ソリューションをビルドする
