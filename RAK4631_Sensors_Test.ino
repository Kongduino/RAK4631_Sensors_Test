/**
   @file bme680_basic.ino
   @author rakwireless.com
   @brief Setup and read values from a BME680 environment sensor
   @version 0.1
   @date 2020-07-28
   @copyright Copyright (c) 2020
   @note RAK5005-O GPIO mapping to RAK4631 GPIO ports
   IO1 <-> P0.17 (Arduino GPIO number 17)
   IO2 <-> P1.02 (Arduino GPIO number 34)
   IO3 <-> P0.21 (Arduino GPIO number 21)
   IO4 <-> P0.04 (Arduino GPIO number 4)
   IO5 <-> P0.09 (Arduino GPIO number 9)
   IO6 <-> P0.10 (Arduino GPIO number 10)
   SW1 <-> P0.01 (Arduino GPIO number 1)
*/
#include <Wire.h>
#include "ClosedCube_BME680.h" //https://github.com/closedcube/ClosedCube_BME680_Arduino
#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
#include <ClosedCube_OPT3001.h> // Click here to get the library: http://librarymanager/All#OPT3001
#include <Arduino_LPS22HB.h> // Click here to get the library: http://librarymanager/All#Arduino_LPS22HB

#define OPT3001_ADDRESS 0x44
// Remember to #define WIFIPWD WIFISSID and your Key + IDs.
String WIFIPWD = "didier0barbas";
String WIFISSID = "SecondTry";
String VariKey = "LTcMYOvAvDpfM5X8";
String Var0 = "hJ2VNsZn";
String Var1 = "xg0ssshI";
String Var2 = "V5CCrcUH";
String Var3 = "f1BRHFwB";
String Var4 = "kVC5Nedj";
String Var5 = "QFaaMrqM";
String Var6 = "i5a7p523";
String Var7 = "j7ceumDw";

ClosedCube_OPT3001 opt3001;
SHTC3 mySHTC3; // Declare an instance of the SHTC3 class
ClosedCube_BME680 bme680;
double bme680_temp;
double bme680_pres;
double bme680_hum;
uint32_t bme680_gas;
float opt3001_lux;
float shtc3_temp, shtc3_hum;
float lps22hb_pressure;
uint8_t intervalCount = 9; // so that we can have an upload right away

void execute_at(char *at, char *expect = NULL, int32_t timeout = 1000) {
  String resp = "";
  Serial1.write(at);
  delay(100);
  while (timeout--) {
    if (Serial1.available()) {
      resp += Serial1.readString();
    }
    delay(1);
  }
  Serial.println("+================================================================+");
  Serial.println(resp);
  Serial.println("+================================================================+");
  if (expect != NULL) {
    if (resp.indexOf(expect) != -1) {
      Serial.println("Execute OK.");
    } else {
      Serial.println("Execute Fail.");
    }
  }
  resp = "";
}

void SHTC3_errorDecoder(SHTC3_Status_TypeDef message) {
  // The errorDecoder function prints "SHTC3_Status_TypeDef" results in a human-friendly way
  switch (message) {
    case SHTC3_Status_Nominal:
      Serial.println("Nominal");
      break;
    case SHTC3_Status_Error:
      Serial.println("Error");
      break;
    case SHTC3_Status_CRC_Fail:
      Serial.println("CRC Fail");
      break;
    default:
      Serial.println("Unknown return code");
      break;
  }
}

void bme680_init() {
  bme680.init(0x76); // I2C address: 0x76 or 0x77
  bme680.reset();
  Serial.print("  . Chip ID=0x");
  Serial.println(bme680.getChipID(), HEX);
  // oversampling: humidity = x1, temperature = x2, pressure = x16
  bme680.setOversampling(BME680_OVERSAMPLING_X1, BME680_OVERSAMPLING_X2, BME680_OVERSAMPLING_X16);
  bme680.setIIRFilter(BME680_FILTER_3);
  bme680.setGasOn(300, 100); // 300 degree Celsius and 100 milliseconds
  bme680.setForcedMode();
}

void bme680_get() {
  Serial.println("\n* BME680:");
  bme680_temp = bme680.readTemperature();
  bme680_pres = bme680.readPressure();
  bme680_hum = bme680.readHumidity();
  bme680_gas = bme680.readGasResistance();
  Serial.print(" Temperature: ");
  Serial.print(bme680_temp);
  Serial.print("*C\n Humidity: ");
  Serial.print(bme680_hum);
  Serial.print("%\n Pressure: ");
  Serial.print(bme680_pres, 2);
  Serial.print("hPa");
  Serial.print("\n Gas: ");
  Serial.print(bme680_gas);
  Serial.println(" Ohms");
  bme680.setForcedMode();
}

void shtc3_get(void) {
  Serial.println("\n* SHTC3:");
  mySHTC3.update();
  if (mySHTC3.lastStatus == SHTC3_Status_Nominal) {
    // You can also assess the status of the last command by checking the ".lastStatus" member of the object
    /* Packing LoRa data */
    shtc3_temp = mySHTC3.toDegC();
    shtc3_hum = mySHTC3.toPercent();
    Serial.print(" Humidity: ");
    Serial.print(shtc3_hum, 2); // "toPercent" returns the percent humidity as a floating point number
    Serial.print("%");
    if (!mySHTC3.passRHcrc) {
      // Like "passIDcrc" this is true when the RH value is valid from the sensor
      // (but not necessarily up-to-date in terms of time)
      Serial.print(" (checksum: fail)");
    }
    Serial.print("\n Temperature: ");
    Serial.print(shtc3_temp, 2);
    // "toDegF" and "toDegC" return the temperature as a flaoting point number in deg F and deg C respectively
    Serial.print("*C");
    if (!mySHTC3.passTcrc) {
      // Like "passIDcrc" this is true when the T value is valid from the sensor
      // (but not necessarily up-to-date in terms of time)
      Serial.print(" (checksum: fail)");
    }
    Serial.write('\n');
  } else {
    Serial.print(" Update failed, error: ");
    SHTC3_errorDecoder(mySHTC3.lastStatus);
    Serial.println();
  }
}

void configureOPT3001() {
  OPT3001_Config newConfig;
  newConfig.RangeNumber = B1100;
  newConfig.ConvertionTime = B0;
  newConfig.Latch = B1;
  newConfig.ModeOfConversionOperation = B11;
  OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
  if (errorConfig != NO_ERROR)
    printOPT3001Error("OPT3001 configuration", errorConfig);
  else {
    OPT3001_Config sensorConfig = opt3001.readConfig();
    Serial.println("  OPT3001 Current Config:");
    Serial.println("  ------------------------------");
    Serial.print("  Conversion ready (R):");
    Serial.println(sensorConfig.ConversionReady, HEX);
    Serial.print("  Conversion time (R/W):");
    Serial.println(sensorConfig.ConvertionTime, HEX);
    Serial.print("  Fault count field (R/W):");
    Serial.println(sensorConfig.FaultCount, HEX);
    Serial.print("  Flag high field (R-only):");
    Serial.println(sensorConfig.FlagHigh, HEX);
    Serial.print("  Flag low field (R-only):");
    Serial.println(sensorConfig.FlagLow, HEX);
    Serial.print("  Latch field (R/W):");
    Serial.println(sensorConfig.Latch, HEX);
    Serial.print("  Mask exponent field (R/W):");
    Serial.println(sensorConfig.MaskExponent, HEX);
    Serial.print("  Mode of conversion operation (R/W):");
    Serial.println(sensorConfig.ModeOfConversionOperation, HEX);
    Serial.print("  Polarity field (R/W):");
    Serial.println(sensorConfig.Polarity, HEX);
    Serial.print("  Overflow flag (R-only):");
    Serial.println(sensorConfig.OverflowFlag, HEX);
    Serial.print("  Range number (R/W):");
    Serial.println(sensorConfig.RangeNumber, HEX);
    Serial.println("  ------------------------------");
  }
}

void opt3001_get() {
  OPT3001 result = opt3001.readResult();
  if (result.error == NO_ERROR) {
    opt3001_lux = result.lux;
    Serial.print("\n* OPT3001:\n ");
    Serial.print(opt3001_lux, 2);
    Serial.println(" lux");
  } else {
    printOPT3001Error("OPT3001", result.error);
  }
}

void printOPT3001Result(String text, OPT3001 result) {
  if (result.error == NO_ERROR) {
    Serial.print(text);
    Serial.print(": ");
    Serial.print(result.lux);
    Serial.println(" lux");
  } else {
    printOPT3001Error(text, result.error);
  }
}

void printOPT3001Error(String text, OPT3001_ErrorCode error) {
  Serial.print(text);
  Serial.print(": [ERROR] Code #");
  Serial.println(error);
}

void lps22hb_get() {
  Serial.print("\n* LPS22HB:\n ");
  // read the sensor value
  lps22hb_pressure = BARO.readPressure() * 10;
  // print the sensor value
  Serial.print(" Pressure = ");
  Serial.print(lps22hb_pressure);
  Serial.println(" HPa");
}

void setup() {
  // Initialize Serial for debug output
  delay(3000);
  Serial.begin(115200);
  delay(1000);
  //  Serial.flush();
  Serial.println("\n\nRAK4631 Sensors Test\n");
  Serial.print(" - SHTC3 init.");
  Wire.begin();
  SHTC3_errorDecoder(mySHTC3.begin());
  // To start the sensor you must call "begin()", the default settings use Wire (default Arduino I2C port)
  Wire.setClock(400e3);
  // The sensor is listed to work up to 1 MHz I2C speed, but the I2C clock speed is global for all sensors on that bus
  // so using 400kHz or 100kHz is recommended
  if (mySHTC3.passIDcrc) {
    // Whenever data is received the associated checksum is calculated and verified so you can be sure the data is true
    // The checksum pass indicators are: passIDcrc, passRHcrc, and passTcrc for the ID, RH, and T readings respectively
    Serial.println("  . SHTC3 ID passed checksum. ");
    Serial.print("  . Device ID: 0b");
    Serial.println(mySHTC3.ID, BIN); // The 16-bit device ID can be accessed as a member variable of the object
  } else {
    Serial.println(" - ID Checksum Failed. Abort.");
    while (1) ;
  }
  Serial.println("bme680 init");
  bme680_init();
  Serial.println("OPT3001 init");
  opt3001.begin(OPT3001_ADDRESS);
  Serial.print(" - OPT3001 Manufacturer ID");
  Serial.println(opt3001.readManufacturerID());
  Serial.print(" - OPT3001 Device ID");
  Serial.println(opt3001.readDeviceID());
  configureOPT3001();
  printOPT3001Result(" - High-Limit", opt3001.readHighLimit());
  printOPT3001Result(" - Low-Limit", opt3001.readLowLimit());

  Serial.print("LPS22HB init");
  /* LPS22HB init */
  if (!BARO.begin()) {
    Serial.println("\n - Failed to initialize pressure sensor! Aborting.");
    while (1)
      ;
  }
  Serial.println(" [o]");

  Serial1.begin(115200);
  delay(1000);
  // Set RAK2305 to STA role
  execute_at("AT+CWMODE=1\r\n", "OK");
  delay(1000);
  // Set contry code
  execute_at("AT+CWCOUNTRY=0,\"CN\",1,13\r\n", "OK");
  delay(1000);
  // Connect AP with ssid and password
  String CWJAP = "AT+CWJAP=\"" + WIFISSID + "\",\"" + WIFIPWD + "\"\r\n";
  execute_at((char*)CWJAP.c_str(), "WIFI GOT IP", 3000);
  delay(1000);
  // Get the IP address
  execute_at("AT+CIPSTA?\r\n");
  delay(2000);
  execute_at("AT+CIPDOMAIN=\"api.varipass.org\"\r\n", "OK");
}

void loop() {
  shtc3_get();
  bme680_get();
  lps22hb_get();
  opt3001_get();
  delay(6000);
  intervalCount++;
  if (intervalCount == 10) {
    Serial.println("Uploading data to Varipass:\n");
    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    // In order to be able to produce the AT+CIPSEND=xx command,
    // You need to know the full length of the next command, AT+CIPSEND
    // So here we go:
    // First build the last command
    String rq1 = "GET http://api.varipass.org/?action=write&id=" + Var0 + "&key=" + VariKey + "&value=" + String(bme680_temp, 2) + " HTTP/1.0\r\n\r\n";
    // Then build the AT+CIPSEND command
    String rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    // Need to wait a little longer for this one.
    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var1 + "&key=" + VariKey + "&value=" + String(bme680_hum, 2) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var2 + "&key=" + VariKey + "&value=" + String(bme680_pres, 2) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var6 + "&key=" + VariKey + "&value=" + String(bme680_gas) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var4 + "&key=" + VariKey + "&value=" + String(shtc3_temp, 2) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var5 + "&key=" + VariKey + "&value=" + String(shtc3_hum, 2) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var7 + "&key=" + VariKey + "&value=" + String(lps22hb_pressure, 2) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    execute_at("AT+CIPSTART=\"TCP\",\"45.32.153.147\",80\r\n", "OK");
    rq1 = "GET http://api.varipass.org/?action=write&id=" + Var3 + "&key=" + VariKey + "&value=" + String(opt3001_lux, 2) + " HTTP/1.0\r\n\r\n";
    rq0 = String("AT+CIPSEND=") + String(rq1.length()) + "\r\n";
    execute_at((char*)rq0.c_str(), ">", 2000);
    Serial.println(rq1);
    execute_at((char*)rq1.c_str(), "HTTP/1.1 200 OK", 5000);

    intervalCount = 0;
  } else {
    // execute_at("AT+PING=\"8.8.8.8\"\r\n", "OK", 2000);
  }
}
