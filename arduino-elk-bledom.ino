#include <ArduinoBLE.h>

/*
 * This project wouldn't have been possible without the links below.
 * 
 * https://github.com/FergusInLondon/ELK-BLEDOM/blob/master/PROTCOL.md
 * https://linuxthings.co.uk/blog/control-an-elk-bledom-bluetooth-led-strip
 */

/*
 * Example Peripheral Explorer dump
 * 
Device name: ELK-BLEDOM
Appearance: 0x0

Service 1800
 Characteristic 2a00, properties 0x2, value 0x454C4B2D424C45444F4D
Service fff0
  Characteristic fff4, properties 0x10
    Descriptor 2902, value 0x
    Descriptor 2803, value 0x060900F3FF
    Descriptor fff3, value 0x454C4B503130593630485F53485933525F563034
  Characteristic fff3, properties 0x6, value 0x454C4B503130593630485F53485933525F563034
 */

BLEDevice peripheral1;

bool deviceConnected = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("Bluetooth® Low Energy Central - Strip controller");  

  // find the device
  BLE.scanForName("ELK-BLEDOM");  
}

void loop() {
  // check if the device has become available
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // if so stop the scan
    BLE.stopScan();

    controlStrip(peripheral);

    // the control strip code returnm, which means it disconnected from the device
    // start to scan again.
    BLE.scanForName("ELK-BLEDOM");
  }
}

void controlStrip(BLEDevice peripheral) {

  bool charFound = false;
  bool serviceFound = false;
  
  Serial.println("Connecting ...");

  // connect to the device - nothing else will work otherwiser
  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }
  
  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // the service is fff0 but a scan using the Peripheral Explorer (Arduino sketch) will return it as device 1 (I tried using getting it by UUID, it didn't work)
  BLEService stripService = peripheral.service(1);

  // the characteristic is fff3 but a scan using the Peripheral Explorer (Arduino sketch) will return it as device 1 (I tried using getting it by UUID, but again it didn't work)
  BLECharacteristic stripCharacteristic = stripService.characteristic(1); 

  // check if we have the characteristic
  if (!stripCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    while(1);
    return;
  } else if (!stripCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    while(1);
    return;
  } else {
    Serial.println("LED characteristic found!");
  }

  while(peripheral.connected()) {

    // send white as the colour to the controller
    unsigned int red = 255;
    unsigned int green = 255;
    unsigned int blue = 255;

    byte data[] = {126, 0, 5, 3, red, green, blue, 0, 239 };
    size_t dataLength = sizeof(data) / sizeof(data[0]);
    
    stripCharacteristic.writeValue(data, dataLength); 
  }
}
