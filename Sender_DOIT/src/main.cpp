#include <Arduino.h>

// Board Data
int sender_id = 1;           // Change here for different sender!
int isTTGO = 1;              // Not used now
#define LoRa_frequency 868E6 // Option: 433E6,470E6,868E6,915E6

// I2C SPI
#include <Wire.h>
#include <SPI.h>

// AHT20 1st Temperature & Humidity
//#include <Adafruit_AHTX0.h>
// Adafruit_AHTX0 aht;
// int AHT20_status = 0;

// SHT31 1st Temperature & Humidity
#include "Adafruit_SHT31.h"
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
int SHT31_status = 0;

// HTU21DF 2nd Temperature & Humidity
#include "Adafruit_HTU21DF.h"
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
int HTU21DF_status = 0;

// BME280 Exterior Temperature & Humidity & Pressure
#include "BlueDot_BME280.h"
BlueDot_BME280 bme280; // Object for Sensor 2
int BME280_status = 0;

// BH1750 Exterior Lighting
#include <BH1750.h>
BH1750 lightMeter;
int BH1750_status = 0;

// Lora Libraries
#include <LoRa.h>
int lora_status = 0;

// SD Libraries
#include "FS.h"
#include "SD.h"
SPIClass SDSPI(HSPI);
int sd_status = 0;

// Mic Average per 0.1s
int MicPin = 34; // LORA32
float sound_total;
float sound_sample_count;
float sound_average = -1;
int sound_sample_interval = 100; // 0.1s
unsigned long time_now = 0;

// Weight Sensor
//#include "soc/rtc.h"
#include "HX711.h"
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 35; // LORA32
const int LOADCELL_SCK_PIN = 12;  // LORA32
HX711 scale;
int scale_status = 0;

// RTC Libraries
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int RTC_status = 0;

// Counter
int irPin1 = 36;
int irPin2 = 39;
int count = 0;
boolean state1 = true;
boolean state2 = true;
boolean insideState = false;
boolean outsideIr = false;
boolean isPeopleExiting = false;
int i = 1;
int counter_in = 0;
int counter_out = 0;

// OLED Screen
#include <GyverOLED.h>
// GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;
GyverOLED<SSD1306_128x64, OLED_BUFFER> display;

// Data Recording
String folder_name;
String file_name;
String data_line;
File myFile;

// Variables
int lora_counter = 0;
unsigned long loop_interval = 15000;
int lastDay;
unsigned long previous_time = 0;

void setup()
{
  // Connection Begin ------------------------------------------------------------------------------------
  Serial.begin(115200);
  SPI.end();                // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(5, 19, 27, 18); // map and init SPI pins SCK(18), MISO(19), MOSI(23), SS(5)
  Wire.begin(21, 22);

  // Self Check Start ------------------------------------------------------------------------------------

  // Display Setup ------------------------------------------------------------------------------------
  // display.clear();
  display.init();
  display.home();
  display.update();
  // display.startscrollright(0x00, 0x0F);
  display.setScale(2);
  display.println();
  display.println(" SmartHive");
  display.println();
  display.println("  Sender");
  display.update();
  delay(3000);

  display.clear();
  display.home();
  display.setScale(1);

  // SHT31 SETUP
  Serial.println("SHT31 test");
  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
    Serial.println("NO SHT31");
    display.println("NO SHT31");
  }
  else
  {
    Serial.println("Found SHT31");
    display.println("Found SHT31");
    SHT31_status = 1;
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");

  // HTU21DF SETUP
  if (!htu.begin())
  {
    Serial.println("No HTU21DF");
    display.println("No HTU21DF");
  }
  else
  {
    Serial.println("Found HTU21DF");
    display.println("Found HTU21DF");
    HTU21DF_status = 1;
  }

  // BME280 SETUP ------------------------------------------------------------------------------------
  bme280.parameter.communication = 0;          // I2C communication for bme280
  bme280.parameter.I2CAddress = 0x76;          // I2C Address for bme280
  bme280.parameter.sensorMode = 0b11;          // Setup Sensor mode for bme280
  bme280.parameter.IIRfilter = 0b100;          // IIR Filter for bme280
  bme280.parameter.humidOversampling = 0b101;  // Humidity Oversampling for bme280
  bme280.parameter.tempOversampling = 0b101;   // Temperature Oversampling for bme280
  bme280.parameter.pressOversampling = 0b101;  // Pressure Oversampling for bme280
  bme280.parameter.pressureSeaLevel = 1013.25; // default value of 1013.25 hPa (bme280)
  bme280.parameter.tempOutsideCelsius = 15;    // default value of 15°C
  bme280.parameter.tempOutsideFahrenheit = 59; // default value of 59°F

  if (bme280.init() != 0x60)
  {
    Serial.println("NO BME280");
    display.println("NO BME280");
  }
  else
  {
    Serial.println("Found BME280");
    display.println("Found BME280");
    BME280_status = 1;
  }

  // RTC ClockTest ------------------------------------------------------------------------------------
  if (!rtc.begin())
  {
    Serial.println("No RTC Clock");
    display.println("No RTC Clock");
  }
  else
  {
    Serial.println("Found RTC Clock");
    display.println("Found RTC Clock");
    RTC_status = 1;
    //rtc.adjust(DateTime(__DATE__, __TIME__));
  }

  // BH1750 Light Test ------------------------------------------------------------------------------------
  if (!lightMeter.begin())
  {
    Serial.println("NO BH1750 Light");
    display.println("NO BH1750 Light");
  }
  else
  {
    Serial.println("Found BH1750 Light");
    display.println("Found BH1750 Light");
    BH1750_status = 1;
  }

  // Mic Setup ------------------------------------------------------------------------------------
  pinMode(MicPin, INPUT);

  // Scale Setup ------------------------------------------------------------------------------------
  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  if (scale.wait_ready_retry(10))
  {
    scale_status = 1;
    long reading = scale.read();
    Serial.print("HX711 reading OK: ");
    Serial.println(reading);
    Serial.println("Before setting up the scale:");
    Serial.print("read: \t\t");
    Serial.println(scale.read()); // print a raw reading from the ADC
    Serial.print("read average: \t\t");
    Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC
    Serial.print("get value: \t\t");
    Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight (not set yet)
    Serial.print("get units: \t\t");
    Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight (not set) divided
    // by the SCALE parameter (not set yet)

    // scale.set_scale(0); // Should calibrate later https://randomnerdtutorials.com/esp32-load-cell-hx711/
    scale.set_scale(-471.497); // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.tare();              // reset the scale to 0

    Serial.println("After setting up the scale:");
    Serial.print("read: \t\t");
    Serial.println(scale.read()); // print a raw reading from the ADC
    Serial.print("read average: \t\t");
    Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC
    Serial.print("get value: \t\t");
    Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight, set with tare()
    Serial.print("get units: \t\t");
    Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight, divided
    // by the SCALE parameter set with set_scale
    Serial.println("Scale Start Reading");
    display.println("Found Scale");
    scale_status = 1;
  }
  else
  {
    Serial.println("No Scale");
    display.println("No Scale");
  }

  // RTC Clock Setup  ------------------------------------------------------------------------------------
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    display.println("RTC Clock failed!");
  }
  else
  {
    Serial.println("RTC Clock Found");
    display.println("RTC Clock Found!");
  }

  // SD Setup ------------------------------------------------------------------------------------
  SDSPI.begin(14, 2, 15, 13); // SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS
  if (!SD.begin(13, SDSPI))
  { // SDCARD_CS, SDSPI
    Serial.println("Card Mount Failed");
    display.println("Card Mount Failed");
    // return;  //Continue Operation
  }
  else
  {
    sd_status = 1;
    Serial.println("Card Mount Success");
    display.print("SD OK");
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    display.print("No SD");
    sd_status = 2;
    // return; //Continue Operation
  }
  else
  {
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
      Serial.println("MMC");
      sd_status = 3;
    }
    else if (cardType == CARD_SD)
    {
      Serial.println("SDSC");
      sd_status = 3;
    }
    else if (cardType == CARD_SDHC)
    {
      Serial.println("SDHC");
      sd_status = 3;
    }
    else
    {
      Serial.println("UNKNOWN");
      sd_status = 3;
    }
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  display.print("|Size:");
  display.print(cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  display.print("|Used:");
  display.println(SD.usedBytes() / (1024 * 1024));

  // data recording initiative
  folder_name = String("");
  file_name = String("");
  data_line = String("");

  // Lora Setup ------------------------------------------------------------------------------------
  LoRa.setPins(18, 23, 26); // CS RST DIO0
  Serial.println("LoRa Sender");

  if (!LoRa.begin(LoRa_frequency))
  {
    Serial.println("Starting LoRa failed!");
    display.println("No LoRa");
  }

  else
  {
    Serial.println("Starting LoRa Success!");
    display.println("Data OK!");
    lora_status = 1;
  }

  // Mic Setup ------------------------------------------------------------------------------------
  pinMode(MicPin, INPUT);

  // Counter Setup
  pinMode(irPin1, INPUT);
  pinMode(irPin2, INPUT);

  // Display Status ------------------------------------------------------------------------------------
  display.update();
  delay(5000);

} // End of Setup

void loop()
{
  DateTime now_local = rtc.now();
  
  if (now_local.day() != lastDay) // this happens exactly once a day.
  {
    counter_in = 0; // Reset counter once a day
    counter_out = 0;
  }

  lastDay = now_local.day();

  // Counter Reading
  if (!digitalRead(irPin1) && i == 1 && state1)
  {
    outsideIr = true;
    // delay(100);
    i++;
    state1 = false;
  }

  if (!digitalRead(irPin2) && i == 2 && state2)
  {
    Serial.println("Entering Beehive");
    outsideIr = true;
    // delay(100);
    i = 1;
    count++;
    counter_in++;
    Serial.print("Bees:");
    Serial.println(count);
    Serial.print("Bees_in:");
    Serial.println(counter_in);
    Serial.print("Bees_out:");
    Serial.println(counter_out);
    state2 = false;
  }

  if (!digitalRead(irPin2) && i == 1 && state2)
  {
    outsideIr = true;
    // delay(100);
    i = 2;
    state2 = false;
  }

  if (!digitalRead(irPin1) && i == 2 && state1)
  {
    Serial.println("Exiting Beehive");
    outsideIr = true;
    // delay(100);
    count--;
    counter_out++;
    Serial.print("Bees:");
    Serial.println(count);
    Serial.print("Bees_in:");
    Serial.println(counter_in);
    Serial.print("Bees_out:");
    Serial.println(counter_out);
    i = 1;
    state1 = false;
  }

  if (digitalRead(irPin1))
  {
    state1 = true;
  }

  if (digitalRead(irPin2))
  {
    state2 = true;
  }

  // Start Delay
  // delay(2000); //delay after sound averaging, now total 2s
  unsigned long current_time = millis();

  if (current_time - previous_time >= loop_interval)
  {
    // Print Time -----------------------------------------------------------------
    DateTime now_local = rtc.now();
    if (RTC_status == 1)
    {
      Serial.print(now_local.year(), DEC);
      Serial.print("-");
      Serial.print(now_local.month(), DEC);
      Serial.print("-");
      Serial.print(now_local.day(), DEC);
      Serial.print("|");
      // Serial.setTextSize(1);
      // Serial.setCursor(0, 10);
      Serial.print(now_local.hour(), DEC);
      Serial.print(":");
      Serial.print(now_local.minute(), DEC);
      Serial.print(":");
      Serial.println(now_local.second(), DEC);
    }
    else
    {
      Serial.println("NO RTC Clock");
    }

    // SHT31 Reading -----------------------------------------------------------------
    float SHT31_temp = -1;
    float SHT31_hmd = -1;
    if (SHT31_status == 1)
    {
      SHT31_temp = sht31.readTemperature();
      SHT31_hmd = sht31.readHumidity();
      if (!isnan(SHT31_temp))
      { // check if 'is not a number'
        Serial.print("Temp *C = ");
        Serial.println(SHT31_temp);
        ;
      }
      else
      {
        Serial.println("Failed to read temperature");
      }

      if (!isnan(SHT31_hmd))
      { // check if 'is not a number'
        Serial.print("Hum. % = ");
        Serial.println(SHT31_hmd);
      }
      else
      {
        Serial.println("Failed to read humidity");
      }
    }

    // Toggle heater enabled state every 30 seconds
    // An ~3.0 degC temperature increase can be noted when heater is enabled
    if (loopCnt >= 30)
    {
      enableHeater = !enableHeater;
      sht31.heater(enableHeater);
      Serial.print("Heater Enabled State: ");
      if (sht31.isHeaterEnabled())
        Serial.println("ENABLED");
      else
        Serial.println("DISABLED");
      loopCnt = 0;
    }
    loopCnt++;

    // HTU21DF Reading -----------------------------------------------------------------
    float HTU21DF_temp = -1;
    float HTU21DF_hmd = -1;
    if (HTU21DF_status == 1)
    {
      HTU21DF_temp = htu.readTemperature();
      HTU21DF_hmd = htu.readHumidity();
      Serial.print("HTU21DF_Temp: ");
      Serial.print(HTU21DF_temp);
      Serial.println(" C");
      Serial.print("HTU21DF_Hmd: ");
      Serial.print(HTU21DF_hmd);
      Serial.println(" \%");
    }
    else
    {
      Serial.println("NO HTU21DF");
    }

    // BME280 Reading -----------------------------------------------------------------
    float BME280_temp = -1;
    float BME280_hmd = -1;
    float BME280_pre = -1;
    float BME280_alt = -1;

    if (BME280_status == 1)
    {
      BME280_temp = bme280.readTempC();
      BME280_hmd = bme280.readHumidity();
      BME280_pre = bme280.readPressure();
      BME280_alt = bme280.readAltitudeMeter();

      Serial.print("BME280_Temp: ");
      Serial.println(BME280_temp);
      Serial.print("BME280_Hmd: ");
      Serial.println(BME280_hmd);
      Serial.print("BME280_Pre: ");
      Serial.println(BME280_pre);
      Serial.print("BME280_Alt: ");
      Serial.println(BME280_alt);
    }
    else
    {
      Serial.println("NO BME280");
    }

    // BH1750 Light Sensor Reading -----------------------------------------------------------------
    float BH1750_lux = -1;
    if (BH1750_status == 1)
    {
      BH1750_lux = lightMeter.readLightLevel();
      Serial.print("Light: ");
      Serial.print(BH1750_lux);
      Serial.println(" lx");
    }
    else
    {
      Serial.println("NO BH1750 light");
    }

    // Scale Reading -----------------------------------------------------------------
    float weight = -1;
    if (scale_status == 0)
    {
      Serial.println("NO Scale");
    }
    else
    {
      Serial.print("Scale Average Reading: ");
      weight = scale.get_units(10);
      Serial.println(weight, 5);
    }
    // scale.power_down();             // put the ADC in sleep mode

    // Sound Reading -----------------------------------------------------------------
    // Sound average reading in delay time, interval 0.1s

    sound_total = 0;
    sound_sample_count = 0;
    sound_average = 0;
    time_now = millis();

    while (sound_sample_count < loop_interval / 10 / sound_sample_interval)
    {
      while (millis() >= time_now + sound_sample_interval)
      {
        sound_total += analogRead(MicPin);
        sound_sample_count++;
        time_now += sound_sample_interval;
      }
    }

    sound_average = sound_total / sound_sample_count;
    Serial.print(F("sound_total="));
    Serial.println(sound_total);
    Serial.print(F("sound_sample_count="));
    Serial.println(sound_sample_count);
    Serial.print("Average Sound: ");
    Serial.println(sound_average);

    // Data Transfer ==============================================================================
    data_line = sender_id + String(",") + lora_counter + String(",") + now_local.year() + String(",") + now_local.month() + String(",") + now_local.day() + String(",") + now_local.hour() + String(",") + now_local.minute() + String(",") + now_local.second() + String(",") + SHT31_temp + String(",") + SHT31_hmd + String(",") + HTU21DF_temp + String(",") + HTU21DF_hmd + String(",") + BME280_temp + String(",") + BME280_hmd + String(",") + BME280_pre + String(",") + BME280_alt + String(",") + BH1750_lux + String(",") + weight + String(",") + sound_average + String(",") + counter_in + String(",") + counter_out + String(",");

    // add , at last to fit csvtoarray in receiver code

    // Save data to sd ----------------------------------------------------------------------------
    // listDir(SD, "/", 0);
    if (!SD.begin(13, SDSPI))
    { // if sd doesn't exist, skip this step
      Serial.println("NO SD CARD");
    }

    else
    {
      folder_name = now_local.year() + String("_") + now_local.month() + String("_") + now_local.day();
      file_name = now_local.year() + String("_") + now_local.month() + String("_") + now_local.day() + String("_") + now_local.hour() + String(".csv");
      Serial.print("folder_name=");
      Serial.println(folder_name);
      Serial.println(file_name);
      Serial.println(data_line);

      // see if the id directory exists, create it if not.
      if (!SD.exists(String("/") + sender_id))
      {
        if (SD.mkdir(String("/") + sender_id))
        {
          Serial.println("Sender directory created");
        }
        else
        {
          Serial.println("Sender directory not created(error)");
        }
      }
      else
      {
        Serial.println("Sender directory exists");
      }

      // see if the directory exists, create it if not.
      if (!SD.exists(String("/") + sender_id + String("/") + folder_name))
      {
        if (SD.mkdir(String("/") + sender_id + String("/") + folder_name))
        {
          Serial.println("File directory created");
        }
        else
        {
          Serial.println("File directory not created(error)");
        }
      }
      else
      {
        Serial.println("File directory exists");
      }

      // see if the file exists, create it if not.
      if (!SD.exists(String("/") + sender_id + String("/") + folder_name + String("/") + file_name))
      {
        Serial.println("File doesn't exist");
        myFile = SD.open(String("/") + sender_id + String("/") + folder_name + String("/") + file_name, FILE_WRITE);
        myFile.close();
        Serial.println("File created");
      }
      else
      {
        Serial.println("File exists");
      }

      // Append file
      Serial.print("Appending to file: ");
      Serial.println(file_name);

      myFile = SD.open(String("/") + sender_id + String("/") + folder_name + String("/") + file_name, FILE_APPEND);
      if (!myFile)
      {
        Serial.println("Failed to open file for appending");
      }
      if (myFile.println(data_line))
      {
        Serial.println("Data appended");
      }
      else
      {
        Serial.println("Append failed");
      }
      myFile.close();
    }

    // Lora send packet ----------------------------------------------------------------------------
    LoRa.beginPacket();
    LoRa.println(data_line);
    LoRa.endPacket();

    Serial.print("Sending packet: ");
    Serial.println(data_line);

    lora_counter++;

    // Start Display Loop ============================================================================

    display.clear();
    display.home();
    display.setScale(1);

    display.print("ID:");
    display.print(sender_id);
    display.print("|");

    // Print Lora ----------------------------------------
    if (LoRa.begin(LoRa_frequency))
    {
      display.print("Data OK");
    }
    else
    {
      display.print("Data Fail");
    }

    // Print SD ----------------------------------------
    if (sd_status == 0)
    {
      display.println("|SD Fail");
    }
    else if (sd_status == 1)
    {
      display.println("|SD Init");
    }
    else if (sd_status == 2)
    {
      display.println("|No SD");
    }
    else
    {
      display.println("|SD OK");
    }

    // Time Display ----------------------------------------
    if (RTC_status == 1)
    {
      display.print(now_local.year(), DEC);
      display.print("-");
      display.print(now_local.month(), DEC);
      display.print("-");
      display.print(now_local.day(), DEC);
      display.print("|");
      // display.setScale(1);
      // display.setCursor(0, 10);
      display.print(now_local.hour(), DEC);
      display.print(":");
      display.print(now_local.minute(), DEC);
      display.print(":");
      display.println(now_local.second(), DEC);
    }

    else
    {
      display.println("NO CLOCK");
    }

    // Print SHT31 ----------------------------------------
    if (SHT31_status == 0)
    {
      display.println("NoSHT31");
    }
    else
    {
      display.print("TmpUp=");
      display.print(SHT31_temp, 0);
      display.print("|");
      display.print("HmdUp=");
      display.print(SHT31_hmd, 0);
      display.println();
    }

    // Print HTU21DF ----------------------------------------
    if (HTU21DF_status == 0)
    {
      display.println("NoHTU21DF");
    }
    else
    {
      display.print("TmpDn=");
      display.print(HTU21DF_temp, 0);
      display.print("|");
      display.print("HmdDn=");
      display.print(HTU21DF_hmd, 0);
      display.println();
    }

    // Print BME280 ----------------------------------------
    if (BME280_status == 0)
    {
      display.println("NoBME280");
    }
    else
    {
      display.print("TmpEx=");
      display.print(BME280_temp, 0);
      display.print("|");
      display.print("HmdEx=");
      display.println(BME280_hmd, 0);
      display.print("Pre=");
      display.print(BME280_pre, 0);
      display.print("|");
      display.print("Alt=");
      display.print(BME280_alt, 0);
      display.print("m");
      display.println();
    }

    if (BH1750_status == 0)
    {
      display.print("NoBH1750|");
    }
    else
    {
      display.print(BH1750_lux, 0);
      display.print("lux|");
    }

    display.print("Snd:");
    display.println(sound_average, 0);

    display.print("I:");
    display.print(counter_in, 0);
    display.print("|O:");
    display.print(counter_out, 0);
    display.print("|");
    if (scale_status == 0)
    {
      display.print("NoScale|");
    }
    else
    {
      display.print("Wgt:");
      display.println(weight, 0);
    }

    display.print("DT");
    display.print(lora_counter);
    display.print(":");
    display.println(data_line);
    display.update();

    previous_time = current_time;
    // End of Display ==================================================================================
  }
  // scale.power_up();
}
