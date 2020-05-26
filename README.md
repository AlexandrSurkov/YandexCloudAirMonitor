Проект содержит код и дополнительные материалы для статьи на Хабре

Формат пример данных от устройства:

{
  "DeviceId":"a4fde400-6a53-44f6-975b-b61c87276beb",
  "TimeStamp":"2020-05-20T10:02:16.217+03:00",
  "Values":
  [
    {"Type":"Float","Name":"Humidity","Value":"25.281837"},
    {"Type":"Float","Name":"CarbonDioxide","Value":"67.96608"},
    {"Type":"Float","Name":"Pressure","Value":"110.7021"},
    {"Type":"Float","Name":"Temperature","Value":"127.708824"}
  ]
}

В качестве эмулятора в статье используется EnvironmentalSensor из https://github.com/AlexandrSurkov/IoTDeviceEmulator

Также эмулятор можно сделать на базе Yandex Cloud Functions
