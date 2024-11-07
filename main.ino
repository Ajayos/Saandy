#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo servo;                 // Create Servo object
const int servoPin = 2;      // GPIO pin for servo signal
const char* ssid = "AURORA"; // WiFi SSID
const char* password = "keerthanaajay@123"; // WiFi Password

int angle = 90;              // Servo angle (centered)
int angleStep = 5;           // Servo movement step
bool movingRight = true;     // Servo direction flag
unsigned long lastWakeTime = 0;
bool isSleeping = false;

void setup() {
  Serial.begin(115200);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  // Show IP on OLED
  displayIP();

  // Attach servo and set initial angle
  servo.attach(servoPin);
  servo.write(angle);

  // Initialize sleep timer
  lastWakeTime = millis();
}

void loop() {
  if (!isSleeping) {
    animateEyesAndServo(); // Run main eye/servo sequence

    // Blink randomly after every few sequences
    if (random(1, 10) < 3) {
      randomBlink();
    }

    // Check if time to sleep
    if (millis() - lastWakeTime > 15 * 60 * 1000) { // After 15 minutes
      goToSleep();
    }
  } else {
    if (millis() - lastWakeTime > 15000) { // 15 seconds sleep
      wakeUp();
    }
  }
}

// Display IP on OLED
void displayIP() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 50);
  display.print("IP: ");
  display.print(WiFi.localIP());
  display.display();
}

// Animate eyes and servo movement
void animateEyesAndServo() {
  drawEyeOpen();
  moveServoLeft();
  randomBlink();
  moveServoRight();
  randomBlink();
  centerServo();
}

// Draw eyes open
void drawEyeOpen() {
  display.clearDisplay();
  display.drawRoundRect(20, 22, 30, 20, 10, SSD1306_WHITE); // Left eye
  display.fillCircle(35, 32, 5, SSD1306_WHITE);             // Left pupil
  display.drawRoundRect(78, 22, 30, 20, 10, SSD1306_WHITE); // Right eye
  display.fillCircle(93, 32, 5, SSD1306_WHITE);             // Right pupil
  display.display();
}

// Draw eyes closed
void drawEyeClosed() {
  display.clearDisplay();
  display.drawLine(20, 32, 50, 32, SSD1306_WHITE); // Left eye closed
  display.drawLine(78, 32, 108, 32, SSD1306_WHITE); // Right eye closed
  display.display();
}

// Draw eyes looking left
void lookLeft() {
  display.clearDisplay();
  display.drawRoundRect(20, 22, 30, 20, 10, SSD1306_WHITE); // Left eye
  display.fillCircle(28, 32, 5, SSD1306_WHITE);             // Left pupil
  display.drawRoundRect(78, 22, 30, 20, 10, SSD1306_WHITE); // Right eye
  display.fillCircle(86, 32, 5, SSD1306_WHITE);             // Right pupil
  display.display();
}

// Draw eyes looking right
void lookRight() {
  display.clearDisplay();
  display.drawRoundRect(20, 22, 30, 20, 10, SSD1306_WHITE); // Left eye
  display.fillCircle(42, 32, 5, SSD1306_WHITE);             // Left pupil
  display.drawRoundRect(78, 22, 30, 20, 10, SSD1306_WHITE); // Right eye
  display.fillCircle(100, 32, 5, SSD1306_WHITE);            // Right pupil
  display.display();
}

// Move servo left and animate eyes
void moveServoLeft() {
  lookLeft();
  for (int i = angle; i >= 0; i -= angleStep) {
    servo.write(i);
    delay(50);
  }
  angle = 0;
}

// Move servo right and animate eyes
void moveServoRight() {
  lookRight();
  for (int i = angle; i <= 180; i += angleStep) {
    servo.write(i);
    delay(50);
  }
  angle = 180;
}

// Center the servo and reset eyes
void centerServo() {
  drawEyeOpen();
  for (int i = angle; i != 90; i += (angle < 90 ? angleStep : -angleStep)) {
    servo.write(i);
    delay(50);
  }
  angle = 90;
}

// Random blinking
void randomBlink() {
  for (int i = 0; i < random(1, 4); i++) {
    drawEyeClosed();
    delay(100);
    drawEyeOpen();
    delay(100);
  }
}

// Enter sleep mode
void goToSleep() {
  Serial.println("Entering sleep mode...");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.print("ZZZ");
  display.display();
  isSleeping = true;
  lastWakeTime = millis(); // Reset wake timer
}

// Wake up from sleep
void wakeUp() {
  Serial.println("Waking up...");
  lastWakeTime = millis();
  isSleeping = false;
  drawEyeOpen();
  delay(500); // Quick start delay
}
