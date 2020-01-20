#include <Arduino_APDS9960.h> //Include library for colour, proximity and gesture recognition
#include <Arduino_HTS221.h> //Include library to read Temperature and Humidity

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!HTS.begin()) //Initialize Temperature and Humidity sensor
  { Serial.println("Failed to initialize Temperature and Humidity Sensor!"); while (1);}
  if (!APDS.begin()) //Initialize Colour, Proximity and Gesture sensor
  { Serial.println("Failed to initialize Colour, Proximity and Gesture Sensor!"); while (1);}

  // print the header
  Serial.println("temp,bright");
}

float Temperature, Humidity;
int r, g, b, c;

void loop() {
  // check if a color reading is available
  while (! APDS.colorAvailable()) {
    delay(5);
  }

  //Read Temperature value
  Temperature = HTS.readTemperature();
  delay (200);

  // read the color
  APDS.readColor(r, g, b, c);

  // print the values
  Serial.print(Temperature);
  Serial.print(',');
  Serial.print(c);
  Serial.println();

  // wait a bit before reading again
  delay(1000);
}
