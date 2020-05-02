#include "TasmotaDeviceType.h"

#include <absl/strings/match.h>

#include "../../api/Resources.h"
#include "../../utility/Logger.h"

TasmotaDeviceType::TasmotaDeviceType(MQTTClient& mqttClient, UserId apiUser)
    : m_MQTTClient(mqttClient), m_apiUser(apiUser)
{
    // MetadataEntry nameMeta = MetadataEntry::Builder()
    //                              .SetType(MetadataEntry::DataType::string)
    //                              .SetSave(MetadataEntry::DBSave::save)
    //                              .Create();

    MetadataEntry onlineMeta = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::boolean)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .Create();

    //*** Sensors ***//

    // ANALOG
    MetadataEntry ANALOGA0 = MetadataEntry::Builder()
                                 .SetType(MetadataEntry::DataType::integer)
                                 .SetSave(MetadataEntry::DBSave::save_log)
                                 .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                 .SetOptional(true)
                                 .Create();
    MetadataEntry ANALOGTemperature = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::integer)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry ANALOGIlluminance = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::integer)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();

    // AM2301 -> has TempUnit
    MetadataEntry AM2301Temperature = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry AM2301Humidity = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::floatingPoint)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();

    // APDS9960 -> Maybe need to split gesture into single ones
    MetadataEntry APDS9960Gesture = MetadataEntry::Builder()
                                        .SetType(MetadataEntry::DataType::string)
                                        .SetSave(MetadataEntry::DBSave::save_log)
                                        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                        .SetOptional(true)
                                        .Create();
    MetadataEntry APDS9960Red = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::integer)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();
    MetadataEntry APDS9960Green = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::integer)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                      .SetOptional(true)
                                      .Create();
    MetadataEntry APDS9960Blue = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::integer)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry APDS9960Ambient = MetadataEntry::Builder()
                                        .SetType(MetadataEntry::DataType::integer)
                                        .SetSave(MetadataEntry::DBSave::save_log)
                                        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                        .SetOptional(true)
                                        .Create();
    MetadataEntry APDS9960CCT = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::integer)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();
    MetadataEntry APDS9960Proximity = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::integer)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();

    // AZ7798 -> no example mqtt

    // BH1750
    MetadataEntry BH1750Illuminance = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::integer)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();

    // BME280  -> has TempUnit, has PressureUnit -> TODO theoretically there are 2 possible
    MetadataEntry BME280Temperature = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry BME280Humidity = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::floatingPoint)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();
    MetadataEntry BME280Pressure = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::floatingPoint)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();
    MetadataEntry BME280DewPoint = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::floatingPoint)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();

    // BME680 -> has TempUnit, has PressureUnit
    MetadataEntry BME680Temperature = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry BME680Humidity = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::floatingPoint)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();
    MetadataEntry BME680Pressure = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::floatingPoint)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();
    MetadataEntry BME680Gas = MetadataEntry::Builder()
                                  .SetType(MetadataEntry::DataType::floatingPoint)
                                  .SetSave(MetadataEntry::DBSave::save_log)
                                  .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                  .SetOptional(true)
                                  .Create();

    // CC2530 -> ZigBee -> https://tasmota.github.io/docs/Zigbee/ -> complex -> must be seen as own devices
    // Chirp! -> No mqtt example

    // DHT11 -> properties assuemed, also assuming has TempUnit, has PressureUnit
    MetadataEntry DHT11Temperature = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry DHT11Humidity = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::floatingPoint)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                      .SetOptional(true)
                                      .Create();

    // DS18x20 -> has TempUnit -> up to 8 sensors
    MetadataEntry DS18x20Temperature1
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature2
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature3
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature4
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature5
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature6
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature7
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();
    MetadataEntry DS18x20Temperature8
        = MetadataEntry::Builder()
              .SetType(MetadataEntry::DataType::floatingPoint)
              .SetSave(MetadataEntry::DBSave::save_log)
              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
              .SetOptional(true)
              .Create();

    // DS3231 -> No mqtt example
    // HM-10 -> No mqtt example
    // HM-17 - > No mqtt example

    // HC-SR04
    MetadataEntry SR04Distance = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();

    // HIH -> No mqtt example

    // HTU21 -> Assuming has TempUnit
    MetadataEntry HTU21Temperature = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry HTU21Humidity = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::floatingPoint)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                      .SetOptional(true)
                                      .Create();

    // IRRemote
    //"IrReceived": {
    //"Protocol": "NEC",
    //"Bits": 32,
    //"Data": "0x00FF00FF"
    //}

    // LM75AD -> properties assuemed, also assuming has TempUnit
    MetadataEntry LM75ADTemperature = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();

    // MCP23008/MCP23017 -> very complex
    // MGC3130 -> very complex
    // MH-Z19B -> No mqtt example

    // MLX90614
    MetadataEntry MLX90614ObjectT = MetadataEntry::Builder()
                                        .SetType(MetadataEntry::DataType::floatingPoint)
                                        .SetSave(MetadataEntry::DBSave::save_log)
                                        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                        .SetOptional(true)
                                        .Create();
    MetadataEntry MLX90614AmbientT = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();

    //// MPR121 -> No mqtt example and touch sensor (buttons are never transmitted)

    //// MPU-6050 -> has TempUint
    MetadataEntry MPU6050Temperature = MetadataEntry::Builder()
                                           .SetType(MetadataEntry::DataType::floatingPoint)
                                           .SetSave(MetadataEntry::DBSave::save_log)
                                           .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                           .SetOptional(true)
                                           .Create();
    MetadataEntry MPU6050AccelXAxis = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry MPU6050AccelYAxis = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry MPU6050AccelZAxis = MetadataEntry::Builder()
                                          .SetType(MetadataEntry::DataType::floatingPoint)
                                          .SetSave(MetadataEntry::DBSave::save_log)
                                          .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                          .SetOptional(true)
                                          .Create();
    MetadataEntry MPU6050GyroXAxis = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry MPU6050GyroYAxis = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry MPU6050GyroZAxis = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry MPU6050Yaw = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::floatingPoint)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry MPU6050Pitch = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry MPU6050Roll = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::floatingPoint)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();

    // NRF24L01 -> No mqttt examples, but should send data
    // -> also each sensor json obj should technically be it's own devices
    // MA105C -> Very complex
    // PAJ7620U2 -> No mqtt examples

    // PMS5003
    MetadataEntry PMS5003CF1 = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::integer)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry PMS5003CF2_5 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::integer)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry PMS5003CF10 = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::integer)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();
    MetadataEntry PMS5003PM1 = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::integer)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry PMS5003PM2_5 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::integer)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry PMS5003PM10 = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::integer)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();
    MetadataEntry PMS5003PB0_3 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::integer)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry PMS5003PB0_5 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::integer)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry PMS5003PB1 = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::integer)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry PMS5003PB2_5 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::integer)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry PMS5003PB5 = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::integer)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry PMS5003PB10 = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::integer)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();

    // PN532
    MetadataEntry PN532UID = MetadataEntry::Builder()
                                 .SetType(MetadataEntry::DataType::string)
                                 .SetSave(MetadataEntry::DBSave::save_log)
                                 .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                 .SetOptional(true)
                                 .Create();
    MetadataEntry PN532Data = MetadataEntry::Builder()
                                  .SetType(MetadataEntry::DataType::string)
                                  .SetSave(MetadataEntry::DBSave::save_log)
                                  .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                  .SetOptional(true)
                                  .Create();

    // PIR -> includes more thann one sensor (but is abstracted for us), but only if a user sets up a rule
    MetadataEntry PIR1 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR2 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR3 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR4 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR5 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR6 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR7 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();
    MetadataEntry PIR8 = MetadataEntry::Builder()
                             .SetType(MetadataEntry::DataType::boolean)
                             .SetSave(MetadataEntry::DBSave::save_log)
                             .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                             .SetOptional(true)
                             .Create();

    // RDM6300 -> reports SSerialReceived, but this is also used by other sensors

    // SDS011
    MetadataEntry SD0X1PM2_5 = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::floatingPoint)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry SD0X1PM10 = MetadataEntry::Builder()
                                  .SetType(MetadataEntry::DataType::floatingPoint)
                                  .SetSave(MetadataEntry::DBSave::save_log)
                                  .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                  .SetOptional(true)
                                  .Create();

    // SHT3X -> values assumed, also assuming has TempUnit
    MetadataEntry SHT3XTemperature = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::floatingPoint)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry SHT3XHumidity = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::floatingPoint)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                      .SetOptional(true)
                                      .Create();

    // TX23
    MetadataEntry TX23SpeedAct = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry TX23SpeedAvg = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry TX23SpeedMin = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry TX23SpeedMax = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry TX23DirCard = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::string)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                    .SetOptional(true)
                                    .Create();
    MetadataEntry TX23DirDeg = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::floatingPoint)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry TX23DirAvg = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::floatingPoint)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry TX23DirAvgCard = MetadataEntry::Builder()
                                       .SetType(MetadataEntry::DataType::string)
                                       .SetSave(MetadataEntry::DBSave::save_log)
                                       .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                       .SetOptional(true)
                                       .Create();
    MetadataEntry TX23DirMin = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::floatingPoint)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry TX23DirMax = MetadataEntry::Builder()
                                   .SetType(MetadataEntry::DataType::floatingPoint)
                                   .SetSave(MetadataEntry::DBSave::save_log)
                                   .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                   .SetOptional(true)
                                   .Create();
    MetadataEntry TX23DirRange = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::floatingPoint)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                     .SetOptional(true)
                                     .Create();

    // TSL2561
    MetadataEntry TSL2561Illuminance = MetadataEntry::Builder()
                                           .SetType(MetadataEntry::DataType::floatingPoint)
                                           .SetSave(MetadataEntry::DBSave::save_log)
                                           .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                           .SetOptional(true)
                                           .Create();

    // VEML6070 -> No mqtt example

    // VL53L0X
    MetadataEntry VL53L0XDistance = MetadataEntry::Builder()
                                        .SetType(MetadataEntry::DataType::floatingPoint)
                                        .SetSave(MetadataEntry::DBSave::save_log)
                                        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead)
                                        .SetOptional(true)
                                        .Create();

    //*** Actuators ***//
    // Relays
    MetadataEntry POWER1 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER2 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER3 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER4 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER5 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER6 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER7 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry POWER8 = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::boolean)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();

    // Dimmer
    MetadataEntry Dimmer = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::integer)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();
    MetadataEntry Dimmer0 = MetadataEntry::Builder()
                                .SetType(MetadataEntry::DataType::integer)
                                .SetSave(MetadataEntry::DBSave::save_log)
                                .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                    | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                .SetOptional(true)
                                .Create();
    MetadataEntry Dimmer1 = MetadataEntry::Builder()
                                .SetType(MetadataEntry::DataType::integer)
                                .SetSave(MetadataEntry::DBSave::save_log)
                                .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                    | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                .SetOptional(true)
                                .Create();
    MetadataEntry Dimmer2 = MetadataEntry::Builder()
                                .SetType(MetadataEntry::DataType::integer)
                                .SetSave(MetadataEntry::DBSave::save_log)
                                .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                    | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                .SetOptional(true)
                                .Create();

    // Color
    MetadataEntry Color = MetadataEntry::Builder()
                              .SetType(MetadataEntry::DataType::string)
                              .SetSave(MetadataEntry::DBSave::save_log)
                              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                  | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                              .SetOptional(true)
                              .Create();

    // White
    MetadataEntry White = MetadataEntry::Builder()
                              .SetType(MetadataEntry::DataType::integer)
                              .SetSave(MetadataEntry::DBSave::save_log)
                              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                  | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                              .SetOptional(true)
                              .Create();

    // CT
    MetadataEntry CT = MetadataEntry::Builder()
                           .SetType(MetadataEntry::DataType::integer)
                           .SetSave(MetadataEntry::DBSave::save_log)
                           .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                               | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                           .SetOptional(true)
                           .Create();

    // HSBColor
    MetadataEntry HSBColorHue = MetadataEntry::Builder()
                                    .SetType(MetadataEntry::DataType::integer)
                                    .SetSave(MetadataEntry::DBSave::save_log)
                                    .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                        | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                    .SetOptional(true)
                                    .Create();
    MetadataEntry HSBColorSaturation = MetadataEntry::Builder()
                                           .SetType(MetadataEntry::DataType::integer)
                                           .SetSave(MetadataEntry::DBSave::save_log)
                                           .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                               | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                           .SetOptional(true)
                                           .Create();
    MetadataEntry HSBColorBrightness = MetadataEntry::Builder()
                                           .SetType(MetadataEntry::DataType::integer)
                                           .SetSave(MetadataEntry::DBSave::save_log)
                                           .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                               | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                           .SetOptional(true)
                                           .Create();

    // Scheme
    MetadataEntry Scheme = MetadataEntry::Builder()
                               .SetType(MetadataEntry::DataType::integer)
                               .SetSave(MetadataEntry::DBSave::save_log)
                               .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                   | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                               .SetOptional(true)
                               .Create();

    // Speed
    MetadataEntry Speed = MetadataEntry::Builder()
                              .SetType(MetadataEntry::DataType::integer)
                              .SetSave(MetadataEntry::DBSave::save_log)
                              .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                  | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                              .SetOptional(true)
                              .Create();

    // Shutter
    //// ShutterClose
    MetadataEntry ShutterClose1 = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::boolean)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                          | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                      .SetOptional(true)
                                      .Create();
    MetadataEntry ShutterClose2 = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::boolean)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                          | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                      .SetOptional(true)
                                      .Create();
    MetadataEntry ShutterClose3 = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::boolean)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                          | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                      .SetOptional(true)
                                      .Create();
    MetadataEntry ShutterClose4 = MetadataEntry::Builder()
                                      .SetType(MetadataEntry::DataType::boolean)
                                      .SetSave(MetadataEntry::DBSave::save_log)
                                      .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                          | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                      .SetOptional(true)
                                      .Create();

    //// ShutterOpen
    MetadataEntry ShutterOpen1 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry ShutterOpen2 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry ShutterOpen3 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry ShutterOpen4 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();

    //// ShutterStop
    MetadataEntry ShutterStop1 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry ShutterStop2 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry ShutterStop3 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();
    MetadataEntry ShutterStop4 = MetadataEntry::Builder()
                                     .SetType(MetadataEntry::DataType::boolean)
                                     .SetSave(MetadataEntry::DBSave::save_log)
                                     .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                         | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                     .SetOptional(true)
                                     .Create();

    //// ShutterPosition
    MetadataEntry ShutterPosition1 = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::integer)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                             | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry ShutterPosition2 = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::integer)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                             | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry ShutterPosition3 = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::integer)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                             | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                         .SetOptional(true)
                                         .Create();
    MetadataEntry ShutterPosition4 = MetadataEntry::Builder()
                                         .SetType(MetadataEntry::DataType::integer)
                                         .SetSave(MetadataEntry::DBSave::save_log)
                                         .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::userWrite
                                             | MetadataEntry::Access::ruleRead | MetadataEntry::Access::actionWrite)
                                         .SetOptional(true)
                                         .Create();

    // TuyaMCU -> Very complex

    // WS2812B -> no mqtt example

    absl::flat_hash_map<std::string, MetadataEntry> entries {
        // {"name", nameMeta},
        // General
        {"online", onlineMeta},
        // Sensors
        //// ANALOG
        {"ANALOGA0", ANALOGA0},
        {"ANALOGTemperature", ANALOGTemperature},
        {"ANALOGIlluminance", ANALOGIlluminance},
        //// AM2301
        {"AM2301Temperature", AM2301Temperature},
        {"AM2301Humidity", AM2301Humidity},
        //// APDS9960
        {"APDS9960Gesture", APDS9960Gesture},
        {"APDS9960Red", APDS9960Red},
        {"APDS9960Green", APDS9960Green},
        {"APDS9960Blue", APDS9960Blue},
        {"APDS9960Ambient", APDS9960Ambient},
        {"APDS9960CCT", APDS9960CCT},
        {"APDS9960Proximity", APDS9960Proximity},
        //// AZ7798
        //// BH1750
        {"BH1750Illuminance", BH1750Illuminance},
        //// BME280
        {"BME280Temperature", BME280Temperature},
        {"BME280Humidity", BME280Humidity},
        {"BME280Pressure", BME280Pressure},
        {"BME280DewPoint", BME280DewPoint},
        //// BME680
        {"BME680Temperature", BME680Temperature},
        {"BME680Humidity", BME680Humidity},
        {"BME680Pressure", BME680Pressure},
        {"BME680Pressure", BME680Gas},
        //// CC2530
        //// Chirp!
        //// DHT11
        {"DHT11Temperature", DHT11Temperature},
        {"DHT11Humidity", DHT11Humidity},
        //// DS18x20
        {"DS18x20Temperature1", DS18x20Temperature1},
        {"DS18x20Temperature2", DS18x20Temperature2},
        {"DS18x20Temperature3", DS18x20Temperature3},
        {"DS18x20Temperature4", DS18x20Temperature4},
        {"DS18x20Temperature5", DS18x20Temperature5},
        {"DS18x20Temperature6", DS18x20Temperature6},
        {"DS18x20Temperature7", DS18x20Temperature7},
        {"DS18x20Temperature8", DS18x20Temperature8},
        //// DS3231
        //// HM-10
        //// HM-17
        //// HC-SR04
        {"SR04Distance", SR04Distance},
        //// HIH
        //// HTU21
        {"HTU21Temperature", HTU21Temperature},
        {"HTU21Humidity", HTU21Humidity},
        //// IRRemote
        //// LM75AD
        {"LM75ADTemperature", LM75ADTemperature},
        //// MCP23008/MCP23017
        //// MGC3130
        //// MH-Z19B
        //// MLX90614
        {"MLX90614ObjectT", MLX90614ObjectT},
        {"MLX90614AmbientT", MLX90614AmbientT},
        //// MPR121
        //// MPU6050
        {"MPU6050Temperature", MPU6050Temperature},
        {"MPU6050AccelXAxis", MPU6050AccelXAxis},
        {"MPU6050AccelYAxis", MPU6050AccelYAxis},
        {"MPU6050AccelZAxis", MPU6050AccelZAxis},
        {"MPU6050GyroXAxis", MPU6050GyroXAxis},
        {"MPU6050GyroYAxis", MPU6050GyroYAxis},
        {"MPU6050GyroZAxis", MPU6050GyroZAxis},
        {"MPU6050Yaw", MPU6050Yaw},
        {"MPU6050Pitch", MPU6050Pitch},
        {"MPU6050Roll", MPU6050Roll},
        //// NRF24L01
        //// MA105C
        //// PAJ7620U2
        //// PMS5003
        {"PMS5003CF1", PMS5003CF1},
        {"PMS5003CF2_5", PMS5003CF2_5},
        {"PMS5003CF10", PMS5003CF10},
        {"PMS5003CF1", PMS5003PM1},
        {"PMS5003PM2_5", PMS5003PM2_5},
        {"PMS5003PM10", PMS5003PM10},
        {"PMS5003PB0_3", PMS5003PB0_3},
        {"PMS5003PB0_5", PMS5003PB0_5},
        {"PMS5003PB1", PMS5003PB1},
        {"PMS5003PB2_5", PMS5003PB2_5},
        {"PMS5003PB5", PMS5003PB5},
        {"PMS5003PB10", PMS5003PB10},
        //// PN532
        {"PN532UID", PN532UID},
        {"PN532Data", PN532Data},
        //// PIR
        {"PIR1", PIR1},
        {"PIR2", PIR2},
        {"PIR3", PIR3},
        {"PIR4", PIR4},
        {"PIR5", PIR5},
        {"PIR6", PIR6},
        {"PIR7", PIR7},
        {"PIR8", PIR8},
        //// RDM6300
        //// SDS011
        {"SD0X1PM2_5", SD0X1PM2_5},
        {"SD0X1PM10", SD0X1PM10},
        //// SHT3X
        {"SHT3XTemperature", SHT3XTemperature},
        {"SHT3XHumidity", SHT3XHumidity},
        //// TX23
        {"TX23SpeedAct", TX23SpeedAct},
        {"TX23SpeedAvg", TX23SpeedAvg},
        {"TX23SpeedMin", TX23SpeedMin},
        {"TX23SpeedMax", TX23SpeedMax},
        {"TX23DirCard", TX23DirCard},
        {"TX23DirDeg", TX23DirDeg},
        {"TX23DirAvg", TX23DirAvg},
        {"TX23DirAvgCard", TX23DirAvgCard},
        {"TX23DirMin", TX23DirMin},
        {"TX23DirMax", TX23DirMax},
        {"TX23DirRange", TX23DirRange},
        //// TSL2561
        {"TSL2561Illuminance", TSL2561Illuminance},
        //// VEML6070
        //// VL53L0X
        {"VL53L0XDistance", VL53L0XDistance},
        // Actuators
        //// POWER
        {"POWER1", POWER1},
        {"POWER2", POWER2},
        {"POWER3", POWER3},
        {"POWER4", POWER4},
        {"POWER5", POWER5},
        {"POWER6", POWER6},
        {"POWER7", POWER7},
        {"POWER8", POWER8},
        //// Dimmer
        {"Dimmer", Dimmer},
        {"Dimmer0", Dimmer0},
        {"Dimmer1", Dimmer1},
        {"Dimmer2", Dimmer2},
        //// Color
        {"Color", Color},
        //// White
        {"White", White},
        //// CT
        {"CT", CT},
        //// HSBColor
        {"HSBColorHue", HSBColorHue},
        {"HSBColorSaturation", HSBColorSaturation},
        {"HSBColorBrightness", HSBColorBrightness},
        //// Scheme
        {"Scheme", Scheme},
        //// Speed
        {"Speed", Speed},
        //// Shutter
        ////// ShutterClose
        {"ShutterClose1", ShutterClose1},
        {"ShutterClose2", ShutterClose2},
        {"ShutterClose3", ShutterClose3},
        {"ShutterClose4", ShutterClose4},
        ////// ShutterOpen
        {"ShutterOpen1", ShutterOpen1},
        {"ShutterOpen2", ShutterOpen2},
        {"ShutterOpen3", ShutterOpen3},
        {"ShutterOpen4", ShutterOpen4},
        ////// ShutterOpen
        {"ShutterStop1", ShutterStop1},
        {"ShutterStop2", ShutterStop2},
        {"ShutterStop3", ShutterStop3},
        {"ShutterStop4", ShutterStop4},
        ////// ShutterPosition
        {"ShutterPosition1", ShutterPosition1},
        {"ShutterPosition2", ShutterPosition2},
        {"ShutterPosition3", ShutterPosition3},
        {"ShutterPosition4", ShutterPosition4},
    };

    m_meta = Metadata(std::move(entries));
}

std::string jsonToString(const nlohmann::json& json)
{
    switch (json.type())
    {
    case nlohmann::json::value_t::null:
        return "null";
    case nlohmann::json::value_t::boolean:
        return std::to_string(json.get<bool>());
    case nlohmann::json::value_t::string:
        return json;
    case nlohmann::json::value_t::number_integer:
        return std::to_string(json.get<int64_t>());
    case nlohmann::json::value_t::number_unsigned:
        return std::to_string(json.get<uint64_t>());
    case nlohmann::json::value_t::number_float:
        return std::to_string(json.get<double>());
    // case nlohmann::json::value_t::object:
    //     break;
    // case nlohmann::json::value_t::array:
    //     break;
    // case nlohmann::json::value_t::discarded:
    //     break;
    default:
        return "";
    }
}

bool TasmotaDeviceType::ValidateUpdate(absl::string_view property, const nlohmann::json& value, UserId user) const
{
    if (absl::StartsWithIgnoreCase(property, "Dimmer"))
    {
        int i = value;
        return i >= 0 && i <= 100;
    }
    else if (property == "White")
    {
        int i = value;
        return i >= 1 && i <= 100;
    }
    else if (property == "CT")
    {
        int i = value;
        return i >= 153 && i <= 500;
    }
    else if (property == "HSBColorHue")
    {
        int i = value;
        return i >= 0 && i <= 360;
    }
    else if (property == "HSBColorSaturation")
    {
        int i = value;
        return i >= 0 && i <= 100;
    }
    else if (property == "HSBColorBrightness")
    {
        int i = value;
        return i >= 0 && i <= 100;
    }
    else if (property == "Scheme")
    {
        int i = value;
        return i >= 0 && i <= 12;
    }
    else if (property == "Speed")
    {
        int i = value;
        return i >= 1 && i <= 40;
    }
    else if (absl::StartsWithIgnoreCase(property, "ShutterPosition"))
    {
        int i = value;
        return i >= 0 && i <= 100;
    }
    return true;
}

void TasmotaDeviceType::OnUpdate(absl::string_view property, Device& device, UserId user) const
{
    if (user == m_apiUser)
    {
        // The changes were caused by external api requests, light is already updated
        return;
    }
    // if (changedPath != "lightId")
    // {
    std::string name = device.GetName();
    if (property == "name")
    {
        // TODO how to get old name??
    }

    if (absl::StartsWithIgnoreCase(property, "POWER"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), device.GetProperty(property) ? "ON" : "OFF");
    }
    else if (absl::StartsWithIgnoreCase(property, "Dimmer"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), jsonToString(device.GetProperty(property)));
    }
    else if (absl::EqualsIgnoreCase(property, "Color"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/Color", jsonToString(device.GetProperty(property)));
    }
    else if (absl::EqualsIgnoreCase(property, "White"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/White", jsonToString(device.GetProperty(property)));
    }
    else if (absl::EqualsIgnoreCase(property, "CT"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/CT", jsonToString(device.GetProperty(property)));
    }
    else if (absl::EqualsIgnoreCase(property, "HSBColorHue"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/HsbColor1", jsonToString(device.GetProperty(property)));
    }
    else if (absl::EqualsIgnoreCase(property, "HSBColorSaturation"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/HsbColor2", jsonToString(device.GetProperty(property)));
    }
    else if (absl::EqualsIgnoreCase(property, "HSBColorBrightness"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/HsbColor3", jsonToString(device.GetProperty(property)));
    }
    else if (absl::StartsWithIgnoreCase(property, "Scheme"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), jsonToString(device.GetProperty(property)));
    }
    else if (absl::StartsWithIgnoreCase(property, "Speed"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), jsonToString(device.GetProperty(property)));
    }
    else if (absl::StartsWithIgnoreCase(property, "ShutterClose"))
    {
        if (device.GetProperty(property))
        {
            m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), "");
        }
        else
        {
            m_MQTTClient.Publish("cmnd/" + name + "/ShutterStop" + std::string(property.substr(12)), "");
        }
    }
    else if (absl::StartsWithIgnoreCase(property, "ShutterOpen"))
    {
        if (device.GetProperty(property))
        {
            m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), "");
        }
        else
        {
            m_MQTTClient.Publish("cmnd/" + name + "/ShutterStop" + std::string(property.substr(12)), "");
        }
    }
    else if (absl::StartsWithIgnoreCase(property, "ShutterPosition"))
    {
        m_MQTTClient.Publish("cmnd/" + name + "/" + std::string(property), jsonToString(device.GetProperty(property)));
    }
}
// }
