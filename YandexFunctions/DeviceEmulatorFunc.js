const {Session} = require("yandex-cloud");
const {FunctionService} = require("yandex-cloud/api/serverless/functions/v1");
const {
    DeviceService,
    DeviceDataService,
} = require("yandex-cloud/api/iot/devices/v1");
 
function publishToDevice(deviceDataService) {
    
    const deviceId = process.env.DEVICE_ID;
    const humiditySensorValue = (parseFloat(process.env.TEMPERATURE_SENSOR_VALUE) + Math.random()).toFixed(2);
    const temperatureSensorValue = (parseFloat(process.env.HUMIDITY_SENSOR_VALUE) + Math.random()).toFixed(2);
    const pressureSensorValue = (parseFloat(process.env.PRESSURE_SENSOR_VALUE) + Math.random()).toFixed(2);
    const carbonDioxideSensorValue = (parseFloat(process.env.CARBON_DIOXIDE_SENSOR_VALUE) + Math.random()).toFixed(2);
 
    const iotCoreDeviceId = process.env.IOT_CORE_DEVICE_ID;
 
    console.log(`publish to ${iotCoreDeviceId}`);
 
    return deviceDataService.publish({
        deviceId: iotCoreDeviceId,
        topic: `$devices/${iotCoreDeviceId}/events/`,
        data: Buffer.from(
            `{
            "DeviceId":"${deviceId}",
            "Values":[
                {"Type":"Float","Name":"Humidity","Value":"${humiditySensorValue}"},
                {"Type":"Float","Name":"CarbonDioxide","Value":"${carbonDioxideSensorValue}"},
                {"Type":"Bool","Name":"Pressure","Value":"${pressureSensorValue}"},
                {"Type":"Bool","Name":"Temperature","Value":"${temperatureSensorValue}"}
                ]
            }`
            ),
    });
}
 
module.exports.handler = async (event, context) => {
    const session = new Session(context.token);
    const deviceDataService = new DeviceDataService(session);
    await publishToDevice(deviceDataService);
    return {statusCode: 200};
}
 
/* Function result example
{
    "DeviceId":"0e3ce1d0-1504-4325-972f-55c961319814",
    "TimeStamp":"2020-05-21T22:53:16Z",
    "Values":[
        {"Type":"Float","Name":"Humidity","Value":"25.281837"},
        {"Type":"Float","Name":"CarbonDioxide","Value":"67.96608"},
        {"Type":"Float","Name":"Pressure","Value":"110.7021"},
        {"Type":"Float","Name":"Temperature","Value":"127.708824"}
        ]
}
*/
