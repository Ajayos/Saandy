#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo servo;                                // Create Servo object
const int servoPin = 2;                     // GPIO pin for servo signal
const char *ssid = "AURORA";                // WiFi SSID
const char *password = "keerthanaajay@123"; // WiFi Password

int angle = 90;    // Servo angle (centered)
int angleStep = 5; // Servo movement step
bool movingRight = true;
unsigned long lastMoodChangeTime = 0;             // Servo direction flag
unsigned long moodInterval = random(5000, 15000); // Random mood change interval
unsigned long lastWakeTime = 0;
bool isSleeping = false;

void setup()
{
    Serial.begin(115200);

    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.clearDisplay();

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
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

void loop()
{
    unsigned long currentTime = millis();

    if (!isSleeping)
    {
        animateEyesAndServo(); // Run main eye/servo sequence

        // Blink randomly after every few sequences
        if (random(1, 10) < 3)
        {
            randomBlink();
        }
    }

    if (!isSleeping)
    {
        // Change mood at random intervals
        if (currentTime - lastMoodChangeTime > moodInterval)
        {
            if (random(1, 10) <= 2)
            { // Random chance to go to sleep
                goToSleep();
            }
            else
            {
                changeMood();
            }
            lastMoodChangeTime = currentTime;
            moodInterval = random(5000, 15000); // Reset with new random interval
        }
    }
    else
    {
        // Check if time to wake up
        if (currentTime - lastWakeTime > 15000)
        { // 15 seconds sleep
            wakeUp();
        }
    }
}

// Display IP on OLED
void displayIP()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 50);
    display.print("IP: ");
    display.print(WiFi.localIP());
    display.display();
}

// Animate eyes, mouth, and servo movement
void animateEyesAndServo()
{
    drawEyeOpen();
    drawSmile();
    moveServoLeft();
    randomBlink();
    moveServoRight();
    randomBlink();
    centerServo();
}

// Change mood (random smile or frown)
void changeMood()
{
    display.clearDisplay();
    display.drawRoundRect(20, 22, 30, 20, 10, SSD1306_WHITE); // Left eye
    display.fillCircle(35, 32, 5, SSD1306_WHITE);             // Left pupil
    display.drawRoundRect(78, 22, 30, 20, 10, SSD1306_WHITE); // Right eye
    display.fillCircle(93, 32, 5, SSD1306_WHITE);             // Right pupil

    // Randomly choose between a smile or frown
    if (random(0, 2) == 0)
    {
        drawSmile(); // Draw smile
    }
    else
    {
        drawDisSmile(); // Draw frown
    }
    display.display();
    Serial.println("Mood changed");
}

// Draw a smiling mouth
void drawSmile()
{
    for (int i = 0; i <= 10; i++)
    {
        int x = 55 + i;
        int y = 52 + (int)(2 * sin(i * PI / 10));
        display.fillCircle(x, y, 1, SSD1306_WHITE); // Draw smile
    }
}

// Draw a frowning mouth
void drawDisSmile()
{
    for (int i = 0; i <= 10; i++)
    {
        int x = 55 + i;
        int y = 58 - (int)(2 * sin(i * PI / 10));
        display.fillCircle(x, y, 1, SSD1306_WHITE); // Draw frown
    }
}

// Draw eyes open
void drawEyeOpen()
{
    display.clearDisplay();
    display.drawRoundRect(20, 22, 30, 20, 10, SSD1306_WHITE); // Left eye
    display.fillCircle(35, 32, 5, SSD1306_WHITE);             // Left pupil
    display.drawRoundRect(78, 22, 30, 20, 10, SSD1306_WHITE); // Right eye
    display.fillCircle(93, 32, 5, SSD1306_WHITE);             // Right pupil
    display.display();
}

// Move servo left and animate eyes and mouth
void moveServoLeft()
{
    for (int i = angle; i >= 0; i -= angleStep)
    {
        lookLeft();
        drawSmile(); // Add smile during movement
        servo.write(i);
        delay(50);
    }
    angle = 0;
}

// Move servo right and animate eyes and mouth
void moveServoRight()
{
    for (int i = angle; i <= 180; i += angleStep)
    {
        lookRight();
        drawSmile(); // Add smile during movement
        servo.write(i);
        delay(50);
    }
    angle = 180;
}

// Center the servo and animate eyes and mouth
void centerServo()
{
    for (int i = angle; i != 90; i += (angle < 90 ? angleStep : -angleStep))
    {
        drawEyeOpen();
        drawSmile(); // Add smile during centering
        servo.write(i);
        delay(50);
    }
    angle = 90;
}

// Random blinking
void randomBlink()
{
    for (int i = 0; i < random(1, 4); i++)
    {
        drawEyeClosed();
        delay(100);
        drawEyeOpen();
        delay(100);
    }
}

// Draw eyes closed
void drawEyeClosed()
{
    display.clearDisplay();
    display.drawLine(20, 32, 50, 32, SSD1306_WHITE);  // Left eye closed
    display.drawLine(78, 32, 108, 32, SSD1306_WHITE); // Right eye closed
    drawDisSmile();                                   // Frown while eyes are closed
    display.display();
}

// Enter sleep mode
void goToSleep()
{
    Serial.println("Entering sleep mode...");
    isSleeping = true;
    lastWakeTime = millis();

    // Random mouth on sleep
    if (random(0, 2) == 0)
    {
        drawSmile();
    }
    else
    {
        drawDisSmile();
    }
    display.display();

    for (int i = 0; i < 20; i++)
    {
        drawEyeClosed();
        display.setTextSize(1);
        display.setCursor(60, 40 - i); // Moving "Z" animation
        display.print("Z");
        display.display();
        delay(200);
        display.clearDisplay();
    }
}

// Wake up from sleep
void wakeUp()
{
    Serial.println("Waking up...");
    lastWakeTime = millis();
    isSleeping = false;
    drawEyeOpen();
    delay(500);
}
