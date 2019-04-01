# iot-edge-hacks

## [de:code 2019 サンプルコード EnOcean IoT GatewayとAzure IoTツール群](https://www.microsoft.com/ja-jp/events/decode/2019/)

### [このリポジトリは IoT Edge (V1) に対する差分だけの公開です](https://github.com/Azure/iot-edge-v1)

### [EnOcean Gateway 全体コードとツール群はこちらへ](https://github.com/ahidaka/EnOceanGateways)

#### 株式会社デバイスドライバーズ所属 日高亜友 提供

## 解決する課題
IoTデバイスからMicrosoft Azure IoT Hubへの接続は、MQTT等のセキュアなプロトコル対応が必要など面倒であった。
IoT Hub接続用ライブラリとしてDevice SDKやIoT Edgeがあるが、複数の任意デバイスですぐに利用できるサンプルまでは用意されていない。「iot-edge-hacks」は、EnOcean デバイスを事例にして、ゲートウェイデバイスが入手したセンサー値をセキュアかつ、柔軟にIoT Hubに送ることを目的としている。[以前のバージョン](https://github.com/ahidaka/iot-edge-hacks/tree/decode-2018)では出来なかった、リアルタイム的な転送をサポートしている。

## 構成とヒント
Azure IoT Edge V1の [Simulated device-to-cloud sample](https://github.com/Azure/iot-edge-v1/tree/master/v1/samples/simulated_device_cloud_upload) を改造して、実データをIoT Hubに送る様にしている。
IoTデバイスとのデータ受け渡しに「ブリッジファイル」と呼ぶ数値入りテキストファイルを使用する。
デバイスデータ受信時に特定のテキストファイルにデータ値を書き出すだけで良いため、汎用的に利用可能。
ファイル名や Device ID の設定は JSON ファイルで指定して、複数のデータポイントを定義できる。
別に開発したツール群を使用して、この JSON ファイルを自動編集、Device ID を IoT Hubに登録することが可能。
各EnOcean デバイスやセンサーの **LEARN ボタン** と組み合わせて、Plug and Play的な Azure IoT システムを構築できる。

## 前提条件
Azure IoT Edge V1の Simulated device-to-cloud sample をベースにして開発したため、 動作対象環境では、すでに Azure IoT Edge V1 がビルドできて正常動作していることが重要な前提条件。 Windows と Intel Edidon での動作はサポートしてない。Ubuntu 18.04 と Raspbian 9 で動作確認済。 

## 利用方法
+ Simulated device-to-cloud sampleのビルドと動作確認
  [READMEの手順](https://github.com/Azure/iot-edge-v1/blob/master/v1/samples/simulated_device_cloud_upload/README.md) に従ってSimulated device-to-cloud sampleをビルドし、Device Explore等でシミュレーションの温度値などが送信されていることを確認。

+ ファイルの入れ替えとビルド

    次のファイルを入れ替え、追加した後でビルドし直す。
	- iot-edge/modules/simulated_device/src/simulated_device.c
	- iot-edge/samples/simulated_device_cloud_upload/src/main.c
	- iot-edge/samples/simulated_device_cloud_upload/src/dpride.h
	- iot-edge/samples/simulated_device_cloud_upload/src/typedefs.h
	- iot-edge/samples/simulated_device_cloud_upload/src/EoControl.c
	- iot-edge/samples/simulated_device_cloud_upload/src/module_service_config_enabled.c
	- iot-edge/samples/simulated_device_cloud_upload/src/module_service_config_disabled.c
	- iot-edge/samples/simulated_device_cloud_upload/src/sim.json
    
+ ファイルの設定

  個別に指定する場合は、Simulated device-to-cloud sampleと同様に、"IoTHubName", "IoTHubSuffix", "deviceId", "deviceKey"を設定後、対応するデバイスの"filename", "messagePeriod" を記述する。"filename"はセンサー値を持つファイルの絶対パス名を記述。

+ 起動

  Simulated device-to-cloud sampleと同じ手順で json ファイルを指定して起動。

+ テスト

  Device Explorer等でモニターしながら、エディタでファイル内の数値を修正して動作検証。

## ライセンス

Azure IoT Edge V1を継承して [MITライセンス](https://github.com/Azure/iot-edge-v1/blob/master/v1/License.txt) を適用。

## 問い合わせ先
株式会社デバイスドライバーズ E-Kit事業部 (e-kit@decdrv.co.jp) 宛にメールで問い合わせ。
