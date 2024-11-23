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
unsigned long lastMoodChangeTime = 0;
unsigned long moodInterval = random(5000, 15000); // Random mood change interval
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
  delay(9000);

  // Attach servo and set initial angle
  servo.attach(servoPin);
  servo.write(angle);

  // Initialize timers
  lastMoodChangeTime = millis();
  lastWakeTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  if (!isSleeping) {
    // Change mood at random intervals
    if (currentTime - lastMoodChangeTime > moodInterval) {
      if (random(1, 10) <= 2) {  // Random chance to go to sleep
        goToSleep();
      } else {
        changeMood();
      }
      lastMoodChangeTime = currentTime;
      moodInterval = random(5000, 15000);  // Reset with new random interval
    }

    // Additional animation or behavior logic can be placed here
  } else {
    // Check if time to wake up
    if (currentTime - lastWakeTime > 15000) { // 15 seconds sleep
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

// Change mood (random smile or frown)
void changeMood() {
  display.clearDisplay();
  display.drawRoundRect(20, 22, 30, 20, 10, SSD1306_WHITE); // Left eye
  display.fillCircle(35, 32, 5, SSD1306_WHITE);             // Left pupil
  display.drawRoundRect(78, 22, 30, 20, 10, SSD1306_WHITE); // Right eye
  display.fillCircle(93, 32, 5, SSD1306_WHITE);             // Right pupil

  // Randomly choose between a smile or frown
  if (random(0, 2) == 0) {
    drawSmile();  // Draw smile
  } else {
    drawDisSmile(); // Draw frown
  }
  display.display();
  Serial.println("Mood changed");
}

// Draw a smiling mouth using a series of small circles
void drawSmile() {
  for (int i = 0; i <= 10; i++) {
    int x = 55 + i;
    int y = 52 + (int)(2 * sin(i * PI / 10));
    display.fillCircle(x, y, 1, SSD1306_WHITE); // Draw small circles to form a smile
  }
}

// Draw a dis-smiling mouth using a series of small circles
void drawDisSmile() {
  for (int i = 0; i <= 10; i++) {
    int x = 55 + i;
    int y = 58 - (int)(2 * sin(i * PI / 10));
    display.fillCircle(x, y, 1, SSD1306_WHITE); // Draw small circles to form a frown
  }
}

// Draw eyes closed
void drawEyeClosed() {
  display.clearDisplay();
  display.drawLine(20, 32, 50, 32, SSD1306_WHITE); // Left eye closed
  display.drawLine(78, 32, 108, 32, SSD1306_WHITE); // Right eye closed
  display.display();
}

// Enter sleep mode with moving "Z" and randomized mouth
void goToSleep() {
  Serial.println("Entering sleep mode...");
  isSleeping = true;
  lastWakeTime = millis();

  // Randomly decide if the mouth should be a smile or a frown when going to sleep
  if (random(0, 2) == 0) {
    drawSmile();
  } else {
    drawDisSmile();
  }
  display.display();

  for (int i = 0; i < 20; i++) { // Run the "Z" animation loop
    drawEyeClosed();  // Draw closed eyes

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    int zY = 40 - i;  // Start from the eye and move up
    display.setCursor(60, zY);
    display.print("Z");

    display.display();
    delay(200);
    display.clearDisplay();
  }
}

// Wake up from sleep with a smile
void wakeUp() {
  Serial.println("Waking up...");
  lastWakeTime = millis();
  isSleeping = false;
  changeMood(); // Reset with a new mood after waking up
  delay(500); // Quick start delay
}
