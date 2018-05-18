# iot-edge-hacks

## [ de:code 2018 シアターセッション DA21](readme_ja.html)

### [どんなデータもAzure IoT Hubに転送！IoTデバイスで汎用的に利用できるサンプルコード紹介 の日本語解説はこちら](readme_ja.html)

## Azure IoT Edge Tiny Hacks

## Goal
The connection from the IoT device to Microsoft Azure IoT Hub was troublesome, it needs to support secure protocols such as MQTT. Even if there are Device SDK and IoT Edge as libraries for connecting IoT Hub, but samples that can be used immediately by multiple arbitrary devices are not prepared. 

 "iot-edge-hacks" provided here aims to send the sensor value obtained by the gateway device in secure to IoT Hub anyway. It can be used as a prototype that transfers the data of the IoT device to the IoT Hub for confirmation, and can be directly used for sending sensor data for which real time processing is not important.

## Architecture and Hitnts
We modified the [simulated device-to-cloud sample](https://github.com/Azure/iot-edge/tree/master/v1/samples/simulated_device_cloud_upload) of Azure IoT Edge V1 and send real data to IoT Hub. It uses plane text files because it uses numerically-entered files for delivery, and JSON file setting of file name and transmission interval. Works in Windows, various Linux, WSL environments because it is provided as a difference (patch) file of [Simulated device-to-cloud sample](https://github.com/Azure/iot-edge/tree/master/v1/samples/simulated_device_cloud_upload).

## Precondition
In the operation target environment, it is an important precondition that Azure IoT Edge V1 can already be built and working properly. Some Ubuntu-based Linux, Windows installation tools problems due to some environment that can not be built properly.

## How to Use
+ Building and checking operation of Simulated device-to-cloud sample

	Build a simulated device-to-cloud sample according to the procedure of [README](https://github.com/Azure/iot-edge/blob/master/v1/samples/simulated_device_cloud_upload/README.md) and confirm that simulation temperature values...etc. are transmitted by Device Explorer.

+ File replacement and build

	Rebuild after replacing the following file.
	- iot-edge/modules/simulated_device/src/simulated_device.c
	- **JSON** file at  iot-edge/samples/simulated_device_ cloud_upload/src/

+ File settings

	Like "Simulated device-to-cloud sample", after setting "IoTHubName", "IoTHubSuffix", "deviceId", "deviceKey", describe "filename" and "messagePeriod" of the corresponding device.

+ Startup

	Start with json file specified by the same procedure as Simulated device-to-cloud sample.

+ Test

	While monitoring with Device Explorer etc., verify the operation by modifying the numerical value in the file with the editor.

## License
Applied [MIT license inheriting Azure IoT Edge V1](https://github.com/Azure/iot-edge/blob/master/v1/License.txt).
