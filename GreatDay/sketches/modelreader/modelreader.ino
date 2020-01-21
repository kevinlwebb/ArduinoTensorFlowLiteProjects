#include <Arduino_APDS9960.h> //Include library for colour, proximity and gesture recognition
#include <Arduino_HTS221.h> //Include library to read Temperature and Humidity

#include <TensorFlowLite.h>
#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h"

const float accelerationThreshold = 2.5; // threshold of significant in G's
const int numSamples = 119;

int input_length;
int input_length2;

int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::ops::micro::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflInputTensor2 = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize];

// array to map gesture index to a name
const char* GESTURES[] = {
  "punch",
  "flex"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!HTS.begin()) //Initialize Temperature and Humidity sensor
  { Serial.println("Failed to initialize Temperature and Humidity Sensor!"); while (1);}
  if (!APDS.begin()) //Initialize Colour, Proximity and Gesture sensor
  { Serial.println("Failed to initialize Colour, Proximity and Gesture Sensor!"); while (1);}

  Serial.println();

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflInputTensor2 = tflInterpreter->input(1);
  tflOutputTensor = tflInterpreter->output(0);

  input_length = tflInputTensor->bytes / sizeof(float);
  input_length2 = tflInputTensor2->bytes / sizeof(float);
}

float Temperature, Humidity;
int r, g, b, c;


void loop() {
  while (! APDS.colorAvailable()) {
    delay(5);
  }

  Serial.print("input_length: ");

  Serial.print(input_length);

   Serial.print("input_length2: ");

  Serial.print(input_length2);

  //Read Temperature value
  Temperature = HTS.readTemperature();
  delay (200);

  // read the color
  APDS.readColor(r, g, b, c);

  // Place our calculated x value in the model's input tensor
  tflInputTensor->data.f[0] = Temperature;
  tflInputTensor2->data.f[0] = c;

  // Run inference, and report any error
  TfLiteStatus invoke_status = tflInterpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.print("Invoke failed on x_val");
    return;
  }

  // Read the predicted y value from the model's output tensor
  float y_val = tflOutputTensor->data.f[0];

  Serial.print(y_val);

}
