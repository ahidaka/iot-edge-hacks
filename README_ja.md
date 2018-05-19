# iot-edge-hacks

## [ de:code 2018 シアターセッション DA61](https://www.microsoft.com/ja-jp/events/decode/2018/sessions.aspx#DA61)

### どんなデータもAzure IoT Hubに転送！IoTデバイスで汎用的に利用できるサンプルコード紹介 の日本語解説

#### 株式会社デバイスドライバーズ所属 日高亜友 提供

## 解決する課題
IoTデバイスからMicrosoft Azure IoT Hubへの接続は、MQTT等のセキュアなプロトコル対応が必要など面倒であった。
IoT Hub接続用ライブラリとしてDevice SDKやIoT Edgeがあるが、複数の任意デバイスですぐに利用できるサンプルまでは用意されていない。
ここで提供する「iot-edge-hacks」は、ゲートウェイデバイスが何らかの方法で入手したセンサー値を、とにかくセキュアにIoT Hubに送ることを目的としている。
IoTデバイスのデータをIoT Hubに転送して確認するプロトタイプとして利用できるほか、リアルタイム処理が重要ではないセンサーデータの送信にそのまま利用できる。

## 構成とヒント
Azure IoT Edge V1の [Simulated device-to-cloud sample](https://github.com/Azure/iot-edge/tree/master/v1/samples/simulated_device_cloud_upload) を改造して、実データをIoT Hubに送る様にしている。
IoTデバイスとのデータ受け渡しに「ブリッジファイル」と呼ぶ数値入りテキストファイルを使用する。
デバイスデータ受信時に特定のテキストファイルにデータ値を書き出すだけで良いため、汎用的に利用可能。
ファイル名や送信間隔の設定は JSON ファイルで指定して、複数のデータポイントを定義できる。
Simulated device-to-cloud sample の差分で提供のため、Windows、各種 Linux、WSL 環境で動作可能。

## 前提条件
Azure IoT Edge V1の Simulated device-to-cloud sample をベースにして開発したため、 動作対象環境では、すでに Azure
IoT Edge V1 がビルドできて正常動作していることが重要な前提条件。 Ubuntu系の一部の Linux、Windows でのインストールツールの問題により正常にビルドできない環境があるので注意が必要。

## 利用方法
+ Simulated device-to-cloud sampleのビルドと動作確認
  [READMEの手順](https://github.com/Azure/iot-edge/blob/master/v1/samples/simulated_device_cloud_upload/README.md) に従ってSimulated device-to-cloud sampleをビルドし、Device Explore等でシミュレーションの温度値などが送信されていることを確認。

+ ファイルの入れ替えとビルド

    次のファイルを入れ替えた後でビルドし直す。
    - iot-edge/modules/simulated_device/src/simulated_device.c
    - iot-edge/samples/simulated_device_cloud_upload/src/以下の **JSON** ファイル
    
+ ファイルの設定

  Simulated device-to-cloud sampleと同様に、"IoTHubName", "IoTHubSuffix", "deviceId", "deviceKey"を設定後、対応するデバイスの"filename", "messagePeriod" を記述する。"filename"はセンサー値を持つファイルの絶対パス名、"messagePeriod" はミリ秒単位での読み取り周期を記述。

+ 起動

  Simulated device-to-cloud sampleと同じ手順で json ファイルを指定して起動。

+ テスト

  Device Explorer等でモニターしながら、エディタでファイル内の数値を修正して動作検証。

## ライセンス

Azure IoT Edge V1を継承して [MITライセンス](https://github.com/Azure/iot-edge/blob/master/v1/License.txt) を適用。

## 問い合わせ先
株式会社デバイスドライバーズ E-Kit事業部 (e-kit@decdrv.co.jp) 宛にメールで問い合わせ。
