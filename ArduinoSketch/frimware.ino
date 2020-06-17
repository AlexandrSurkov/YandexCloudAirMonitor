#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT Client

// Handy timers
#include <SimpleTimer.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// SW Serial
#include <SoftwareSerial.h>

const char* ssid = "<WifiSSID>";
const char* password = "<WIFIPassword>";

const char* mqttserver = "130.193.44.244"; //mqtt.cloud.yandex.net
const char* yandexIoTCoreDeviceId = "<Yandex IoT Core Cloud Device ID>";
const char* mqttpassword = "<Yandex IoT Core Device pawssword>";
const int mqttport=8883;

String topicCommands = String("$devices/")+String(yandexIoTCoreDeviceId)+String("/commands/#");
String topicEvents = String("$devices/")+String(yandexIoTCoreDeviceId)+String("/events/");

// assign the SERIAL PORT to pins
#define TX 12
#define RX 14

// assign the SPI bus to pins
#define BME_SCK 4  //SCL pin
#define BME_MISO 0 //SDO pin
#define BME_MOSI 5 //SDA pin
#define BME_CS 2   //CSB pin

#define SEALEVELPRESSURE_HPA (1013.25)

// CO2 SERIAL
#define DEBUG_SERIAL Serial
#define SENSOR_SERIAL swSer
SoftwareSerial swSer;//(13, 15, false); // GPIO15 (TX) and GPIO13 (RX)
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
unsigned char response[7];

// Pressure, humidity and temperature
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

// Handy timer
SimpleTimer timer;

// Sensors data
float t {-100};
float p {-1};
float h {-1};
int co2 {-1};

char humidityChars[10];
char temperatreChars[10];
char pressureChars[10];
char carbonDioxideChars[10];

//Yandex IoT Core RootCA
const char* test_root_ca = \
"-----BEGIN CERTIFICATE-----\n \
MIIFGTCCAwGgAwIBAgIQJMM7ZIy2SYxCBgK7WcFwnjANBgkqhkiG9w0BAQ0FADAf\
MR0wGwYDVQQDExRZYW5kZXhJbnRlcm5hbFJvb3RDQTAeFw0xMzAyMTExMzQxNDNa\
Fw0zMzAyMTExMzUxNDJaMB8xHTAbBgNVBAMTFFlhbmRleEludGVybmFsUm9vdENB\
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAgb4xoQjBQ7oEFk8EHVGy\
1pDEmPWw0Wgw5nX9RM7LL2xQWyUuEq+Lf9Dgh+O725aZ9+SO2oEs47DHHt81/fne\
5N6xOftRrCpy8hGtUR/A3bvjnQgjs+zdXvcO9cTuuzzPTFSts/iZATZsAruiepMx\
SGj9S1fGwvYws/yiXWNoNBz4Tu1Tlp0g+5fp/ADjnxc6DqNk6w01mJRDbx+6rlBO\
aIH2tQmJXDVoFdrhmBK9qOfjxWlIYGy83TnrvdXwi5mKTMtpEREMgyNLX75UjpvO\
NkZgBvEXPQq+g91wBGsWIE2sYlguXiBniQgAJOyRuSdTxcJoG8tZkLDPRi5RouWY\
gxXr13edn1TRDGco2hkdtSUBlajBMSvAq+H0hkslzWD/R+BXkn9dh0/DFnxVt4XU\
5JbFyd/sKV/rF4Vygfw9ssh1ZIWdqkfZ2QXOZ2gH4AEeoN/9vEfUPwqPVzL0XEZK\
r4s2WjU9mE5tHrVsQOZ80wnvYHYi2JHbl0hr5ghs4RIyJwx6LEEnj2tzMFec4f7o\
dQeSsZpgRJmpvpAfRTxhIRjZBrKxnMytedAkUPguBQwjVCn7+EaKiJfpu42JG8Mm\
+/dHi+Q9Tc+0tX5pKOIpQMlMxMHw8MfPmUjC3AAd9lsmCtuybYoeN2IRdbzzchJ8\
l1ZuoI3gH7pcIeElfVSqSBkCAwEAAaNRME8wCwYDVR0PBAQDAgGGMA8GA1UdEwEB\
/wQFMAMBAf8wHQYDVR0OBBYEFKu5xf+h7+ZTHTM5IoTRdtQ3Ti1qMBAGCSsGAQQB\
gjcVAQQDAgEAMA0GCSqGSIb3DQEBDQUAA4ICAQAVpyJ1qLjqRLC34F1UXkC3vxpO\
nV6WgzpzA+DUNog4Y6RhTnh0Bsir+I+FTl0zFCm7JpT/3NP9VjfEitMkHehmHhQK\
c7cIBZSF62K477OTvLz+9ku2O/bGTtYv9fAvR4BmzFfyPDoAKOjJSghD1p/7El+1\
eSjvcUBzLnBUtxO/iYXRNo7B3+1qo4F5Hz7rPRLI0UWW/0UAfVCO2fFtyF6C1iEY\
/q0Ldbf3YIaMkf2WgGhnX9yH/8OiIij2r0LVNHS811apyycjep8y/NkG4q1Z9jEi\
VEX3P6NEL8dWtXQlvlNGMcfDT3lmB+tS32CPEUwce/Ble646rukbERRwFfxXojpf\
C6ium+LtJc7qnK6ygnYF4D6mz4H+3WaxJd1S1hGQxOb/3WVw63tZFnN62F6/nc5g\
6T44Yb7ND6y3nVcygLpbQsws6HsjX65CoSjrrPn0YhKxNBscF7M7tLTW/5LK9uhk\
yjRCkJ0YagpeLxfV1l1ZJZaTPZvY9+ylHnWHhzlq0FzcrooSSsp4i44DB2K7O2ID\
87leymZkKUY6PMDa4GkDJx0dG4UXDhRETMf+NkYgtLJ+UIzMNskwVDcxO4kVL+Hi\
Pj78bnC5yCw8P5YylR45LdxLzLO68unoXOyFz1etGXzszw8lJI9LNubYxk77mK8H\
LpuQKbSbIERsmR+QqQ==\
-----END CERTIFICATE-----\n";

WiFiClientSecure  net;
PubSubClient client(net);
BearSSL::X509List x509(test_root_ca);

void connect() {
  delay(5000);
  DEBUG_SERIAL.print("Conecting to wifi ...");
  while (WiFi.status() != WL_CONNECTED) {
    DEBUG_SERIAL.print(".");
    delay(1000);
  }
  Serial.println(" Connected");
  
  net.setInsecure();
  client.setBufferSize(1024);
  client.setKeepAlive(15);

  
  DEBUG_SERIAL.print("Connecting to Yandex IoT Core as");
  DEBUG_SERIAL.print(yandexIoTCoreDeviceId);
  DEBUG_SERIAL.print(" ...");
  while (!client.connect(yandexIoTCoreDeviceId, yandexIoTCoreDeviceId, mqttpassword)) {
    DEBUG_SERIAL.print(".");
    delay(1000);
  }
  DEBUG_SERIAL.println(" Connected");
  DEBUG_SERIAL.println("Subscribe to: ");
  DEBUG_SERIAL.println(topicCommands.c_str());
  client.subscribe(topicCommands.c_str());
}

void readCO2() {
  // CO2
  bool header_found {false};
  char tries {0};
  
  SENSOR_SERIAL.write(cmd, 9);
  memset(response, 0, 7);
  DEBUG_SERIAL.println("CO2: CMD Write");  
  // Looking for packet start
  while(SENSOR_SERIAL.available() && (!header_found)) {
    byte rb = SENSOR_SERIAL.read();
    DEBUG_SERIAL.println(String(rb));
    if(rb == 0xff ) {
            if(SENSOR_SERIAL.read() == 0x86 ) header_found = true;
    }
  }  
  if (header_found) {
    SENSOR_SERIAL.readBytes(response, 7);  
    byte crc = 0x86;
    for (char i = 0; i < 6; i++) {
            crc+=response[i];
    }
    crc = 0xff - crc;
    crc++;  
    if ( !(response[6] == crc) ) {
            DEBUG_SERIAL.println("CO2: CRC error: " + String(crc) + " / "+ String(response[6]));
    } else {
            unsigned int responseHigh = (unsigned int) response[0];
            unsigned int responseLow = (unsigned int) response[1];
            unsigned int ppm = (256*responseHigh) + responseLow;
            co2 = ppm;
            DEBUG_SERIAL.println("CO2:" + String(co2));
    }
  } else {
    DEBUG_SERIAL.println("CO2: Header not found");
  }
}

void messageReceived(char* topic, byte* payload, unsigned int length) {
  String topicString = String(topic);
  DEBUG_SERIAL.print("Message received. Topic: ");
  DEBUG_SERIAL.println(topicString.c_str());
  String payloadStr = "";
  for (int i=0;i<length;i++)
  {
    payloadStr += (char*)payload;
  }
  DEBUG_SERIAL.println(payloadStr);  
  DEBUG_SERIAL.print("Payload: ");
  DEBUG_SERIAL.println(payloadStr);
}

void sendMeasurements() {  
  // Read data
  // Temperature
  t = bme.readTemperature();  
  // Humidity
  h = bme.readHumidity();  
  // Pressure (in mmHg)
  p = bme.readPressure() * 760.0 / 101325;  
  // CO2
  readCO2();
  // Write to debug console
  DEBUG_SERIAL.println("H: " + String(h) + " %");
  DEBUG_SERIAL.println("T: " + String(t) + " C");
  DEBUG_SERIAL.println("P: " + String(p) + " mmHg");
  DEBUG_SERIAL.println("CO2: " + String(co2) + " ppm");  
  // Send to server  
  sprintf(humidityChars, "%f", h);
  sprintf(temperatreChars, "%f", t);
  sprintf(pressureChars, "%f", p);
  sprintf(carbonDioxideChars, "%d", co2);
  String message = String("{")+
    String("\"DeviceId\":\"") + yandexIoTCoreDeviceId + String("\",")+
    String("\"Values\":[")+
        String("{\"Type\":\"Float\",\"Name\":\"Humidity\",\"Value\":\"") + String(h) + String("\"},")+
        String("{\"Type\":\"Float\",\"Name\":\"CarbonDioxide\",\"Value\":\"") + String(co2) + String("\"},")+
        String("{\"Type\":\"Float\",\"Name\":\"Pressure\",\"Value\":\"") + String(p) + String("\"},")+
        String("{\"Type\":\"Float\",\"Name\":\"Temperature\",\"Value\":\"") + String(t) + String("\"}")+
        String("]")+
  String("}");
  DEBUG_SERIAL.println("Publish "+ message);
  if (client.publish(topicEvents.c_str(), message.c_str())) {
    DEBUG_SERIAL.println("Publish ok");
  }
  else {
    DEBUG_SERIAL.println("Publish failed");
  }
}

void setup() {
  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println("Init device");  
  SENSOR_SERIAL.begin(9600, SWSERIAL_8N1, RX, TX, false, 256, 0);  
  // Init Pressure/Temperature sensor
  if (!bme.begin()) {
    DEBUG_SERIAL.println("Could not find a valid BMe280 sensor, check wiring!");
  }        
  delay(10);
  DEBUG_SERIAL.println("Device started");
  WiFi.begin(ssid, password);
  client.setServer(mqttserver, mqttport);
  client.setCallback(messageReceived);
  client.setBufferSize(1024);
  client.setKeepAlive(15);
  connect();
  // Setup a function to be called every 10 second
  timer.setInterval(10000L, sendMeasurements);  
  sendMeasurements();
}

void loop() {
  // put your main code here, to run repeatedly:
  timer.run();
  client.loop();
  if (!client.connected()) {
    connect();
  }
}
