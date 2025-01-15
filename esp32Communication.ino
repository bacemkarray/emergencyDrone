#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

// WiFi credentials
const char* ssid = "Galaxy A02se485";
const char* password = "mfrw6719";

// HTTP server
WebServer server(80);

// Pin Definitions for Joysticks
#define POT_LEFT_VERTICAL 27
#define POT_LEFT_HORIZONTAL 26
#define POT_RIGHT_VERTICAL 35
#define POT_RIGHT_HORIZONTAL 36

// Pin Definitions for Motors
#define MOTOR_FORWARD 5
#define MOTOR_BACKWARD 17
#define MOTOR_UP 16
#define MOTOR_DOWN 4
#define MOTOR_YAW_RIGHT 25
#define MOTOR_YAW_LEFT 33
#define MOTOR_LEFT 22
#define MOTOR_RIGHT 21

// Pin for Launch
#define LAUNCH_PIN 33

// PWM Settings
#define HIGH_PWM 255
#define LOW_PWM 0

// Neutral voltage for potentiometer (steady state)
const float NEUTRAL_VOLTAGE = 1.5;

// Steady state values for joysticks
int steadyStateLV = 150;  // Initial placeholder
int steadyStateLH = 150;  // Initial placeholder
int steadyStateRV = 150;  // Initial placeholder
int steadyStateRH = 150;  // Initial placeholder

// Directional control functions
void moveForward(float voltage) { digitalWrite(MOTOR_FORWARD, HIGH); }
void moveBackward(float voltage) { digitalWrite(MOTOR_BACKWARD, LOW); }
void moveUp(float voltage) { digitalWrite(MOTOR_UP, HIGH); }
void moveDown(float voltage) { digitalWrite(MOTOR_DOWN, LOW); }
void yawRight(float voltage) { digitalWrite(MOTOR_YAW_RIGHT, HIGH); }
void yawLeft(float voltage) { digitalWrite(MOTOR_YAW_LEFT, LOW); }
void moveLeft(float voltage) { digitalWrite(MOTOR_LEFT, HIGH); }
void moveRight(float voltage) { digitalWrite(MOTOR_RIGHT, HIGH); }

// Function to calculate steady-state values
void calculateSteadyState() {
  Serial.println("Calculating steady state values...");
  long sumLV = 0, sumLH = 0, sumRV = 0, sumRH = 0;
  int samples = 500;  // Number of samples over 5 seconds (100ms intervals)
  for (int i = 0; i < samples; i++) {
    sumLV += analogRead(POT_LEFT_VERTICAL);
    sumLH += analogRead(POT_LEFT_HORIZONTAL);
    sumRV += analogRead(POT_RIGHT_VERTICAL);
    sumRH += analogRead(POT_RIGHT_HORIZONTAL);
    delay(10);  // 10ms delay for sampling
  }

  steadyStateLV = sumLV / samples;
  steadyStateLH = sumLH / samples;
  steadyStateRV = sumRV / samples;
  steadyStateRH = sumRH / samples;

  Serial.println("Steady state values set:");
  Serial.print("Left Vertical: "); Serial.println(steadyStateLV);
  Serial.print("Left Horizontal: "); Serial.println(steadyStateLH);
  Serial.print("Right Vertical: "); Serial.println(steadyStateRV);
  Serial.print("Right Horizontal: "); Serial.println(steadyStateRH);
}

// Initialization sequence
void initializationSequence() {
  Serial.println("Starting initialization sequence...");

  // Step 1: Setting Gyros
  Serial.println("Step 1: Setting Gyros");
  analogWrite(POT_LEFT_VERTICAL, HIGH_PWM);
  analogWrite(POT_LEFT_HORIZONTAL, HIGH_PWM);
  analogWrite(POT_RIGHT_VERTICAL, LOW_PWM);
  analogWrite(POT_RIGHT_HORIZONTAL, HIGH_PWM);
  delay(2000);

  // Reset joysticks to steady state
  analogWrite(POT_LEFT_VERTICAL, steadyStateLV);
  analogWrite(POT_LEFT_HORIZONTAL, steadyStateLH);
  analogWrite(POT_RIGHT_VERTICAL, steadyStateRV);
  analogWrite(POT_RIGHT_HORIZONTAL, steadyStateRH);

  // Step 2: Launch Up
  Serial.println("Step 2: Launch Up");
  analogWrite(LAUNCH_PIN, LOW_PWM);
  delay(2000);
  analogWrite(LAUNCH_PIN, HIGH_PWM);
  delay(2000);

  // Step 3: Land Drone
  Serial.println("Step 3: Land Drone");
  analogWrite(LAUNCH_PIN, LOW_PWM);
  delay(2000);
  analogWrite(LAUNCH_PIN, HIGH_PWM);

  Serial.println("Initialization sequence complete!");
}

// Handle keyboard input from HTTP POST requests
void handleKeyboardInput() {
  if (server.hasArg("plain")) {
    String input = server.arg("plain");
    if (input == "k") {

      // Trigger initialization sequence
      calculateSteadyState();  // Calculate steady-state values before initialization
      initializationSequence();
      server.send(200, "text/plain", "this is k");
    } else if (input == "w") moveForward(3.3);       // Move forward
    else if (input == "s") moveBackward(0);         // Move backward
    else if (input == "a") moveLeft(3.3);           // Move left
    else if (input == "d") moveRight(3.3);          // Move right
    else if (input == "u") moveUp(3.3);             // Move up
    else if (input == "j") moveDown(0);             // Move down
    else if (input == "q") yawLeft(3.3);            // Yaw left
    else if (input == "e") yawRight(3.3);           // Yaw right
    else {
      // Stop all motors if no valid input
      moveForward(NEUTRAL_VOLTAGE);
      moveBackward(NEUTRAL_VOLTAGE);
      moveLeft(NEUTRAL_VOLTAGE);
      moveRight(NEUTRAL_VOLTAGE);
      yawLeft(NEUTRAL_VOLTAGE);
      yawRight(NEUTRAL_VOLTAGE);
      moveUp(NEUTRAL_VOLTAGE);
      moveDown(NEUTRAL_VOLTAGE);
    }

    server.send(200, "text/plain", "Input received");
  }
}

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize motor control
  pinMode(LAUNCH_PIN, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  // Set up HTTP server
  server.on("/", HTTP_POST, handleKeyboardInput);
  server.begin();
  Serial.println("HTTP server started!");
}

void loop() {
  // Handle HTTP requests
  server.handleClient();
}
