#include <Arduino.h>

#include <SPI.h>
#include <SD.h>
#include <SHT2x.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CS_PIN 4
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SHT2x sht;
File root;
long last_file_no = 0;

void setup()
{
  Serial.begin(115200);

  sht.begin();

  uint8_t stat = sht.getStatus();
  Serial.println("Getting SHT20 Status");
  Serial.print(stat, HEX);
  Serial.println();

  Serial.print("Initializing SD card...");

  if (!SD.begin(CS_PIN))
  {
    Serial.println("initialization failed. Things to check:");

    Serial.println("1. is a card inserted?");

    Serial.println("2. is your wiring correct?");

    Serial.println("3. did you change the chipSelect pin to match your shield or module?");

    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");

    while (true)
      ;
  }
  Serial.println("SD initialization done.");

  root = SD.open("/");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
}

void loop()
{
  sht.read();

  float temp = sht.getTemperature();
  float hum = sht.getHumidity();

  Serial.print(temp, 1);
  Serial.print("\t");
  Serial.println(hum, 1);

  char str[256];
  snprintf(str, 256, "%6.3f, %6.3f", temp, hum);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(str);
  display.display();
  delay(1000);
}

char *getNameWithoutExtension(const char *name)
{
  int last_index_dot = -1;
  const size_t name_len = strlen(name);
  for (last_index_dot = name_len - 1; last_index_dot >= 0; last_index_dot--)
  {
    if (name[last_index_dot] == '.')
    {
      break;
    }
  }

  if (last_index_dot > 0)
  {
    char *new_str = (char *)malloc(last_index_dot);
    strncpy(new_str, name, last_index_dot);

    return new_str;
  }

  return NULL;
}

void findLastFile(File dir, int numTabs)
{

  while (true)
  {

    File entry = dir.openNextFile();

    if (!entry)
    {

      // no more files

      break;
    }

    for (uint8_t i = 0; i < numTabs; i++)
    {

      Serial.print('\t');
    }

    Serial.print(entry.name());

    if (entry.isDirectory())
    {

      Serial.println("/");

      findLastFile(entry, numTabs + 1);
    }
    else
    {

      // files have sizes, directories do not

      Serial.print("\t\t");

      Serial.println(entry.size(), DEC);

      const char *name = entry.name();
      char *name_no_ext = getNameWithoutExtension(name);

      if (name_no_ext)
      {
        const long file_no = strtol(name_no_ext, NULL, 10);
        free(name_no_ext);

        if (file_no > 0 && last_file_no < file_no)
        {
          last_file_no = file_no;
        }
      }
    }

    entry.close();
  }
}