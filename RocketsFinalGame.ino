// Initial code inspiration for this game came from https://www.instructables.com/The-T-Rex-Game-Chrome-Dino-in-Arduino-Using-OLED-D/
// This code has been partially written by ChatGPT & Bing Co-Pilot

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h> // Include EEPROM library

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3D

#define ROCKET_WIDTH 25
#define ROCKET_HEIGHT 20
#define ROCKET_INIT_X 10
#define ROCKET_INIT_Y 29

#define BASE_LINE_X 0
#define BASE_LINE_Y 54
#define BASE_LINE_X1 127
#define BASE_LINE_Y1 54

#define ASTROID1_WIDTH 11
#define ASTROID1_HEIGHT 22
#define ASTROID2_WIDTH 22
#define ASTROID2_HEIGHT 22

#define JUMP_PIXEL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned char rocket1[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 
    0x3f, 0xc0, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x7f, 0xff, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0x00, 
    0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xf0, 0x00, 
    0x3f, 0xff, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 
};

static const unsigned char PROGMEM astroid1[]={
  // 'astroid1', 11x23px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x0c, 0x18, 0x00, 0x13, 0xcc, 0x00, 0x37, 
	0xe6, 0x00, 0x2f, 0x33, 0x00, 0x2f, 0x31, 0x80, 0x2f, 0xf0, 0xc0, 0x2c, 0xf2, 0x40, 0x24, 0xf3, 
	0x00, 0x13, 0xe1, 0x80, 0x18, 0x08, 0x00, 0x0c, 0x04, 0x40, 0x06, 0x62, 0x00, 0x03, 0x11, 0x00, 
	0x01, 0x98, 0x80, 0x00, 0xcc, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM astroid2[]={
  // 'astroid2', 22x23px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x0c, 0x18, 0x00, 0x13, 0xcc, 0x00, 0x37, 
	0xe6, 0x00, 0x2f, 0x33, 0x00, 0x2f, 0x31, 0x80, 0x2f, 0xf0, 0xc0, 0x2c, 0xf2, 0x40, 0x24, 0xf3, 
	0x00, 0x13, 0xe1, 0x80, 0x18, 0x08, 0x00, 0x0c, 0x04, 0x40, 0x06, 0x62, 0x00, 0x03, 0x11, 0x00, 
	0x01, 0x98, 0x80, 0x00, 0xcc, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const int BUTTON_PIN = 4;
const int BUTTON_PIN_DOWN = 5;
const int VIBRATION_PIN = 6; // Vibration motor pin
const int PIEZO_PIN = 9;     // Piezo buzzer pin
const int explosionStartFrequency = 100;  // Initial frequency of the explosion
const int explosionEndFrequency = 2000;   // Final frequency of the explosion
const int explosionDuration = 1000;       // Duration of the explosion (in milliseconds)
const int explosionNumSteps = 100;         // Number of frequency steps during the explosion
const int LED_TOP_PIN = 10;    // LED pin for indicating top half of the screen
const int LED_BOTTOM_PIN = 11; // LED pin for indicating bottom half of the screen

int highScoreAddress = 0; // Address in EEPROM to store high score
int highScore = 0; // Define high score variable
int ASTROID_Y;

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUTTON_PIN_DOWN, INPUT);
    pinMode(VIBRATION_PIN, OUTPUT);
    pinMode(PIEZO_PIN

, OUTPUT);
    pinMode(LED_TOP_PIN, OUTPUT);
    pinMode(LED_BOTTOM_PIN, OUTPUT);
    EEPROM.get(highScoreAddress, highScore);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;) {} // Don't proceed, loop forever
    }

    display.clearDisplay();
    introMessage();

    while (1) {
        if (digitalRead(BUTTON_PIN) == LOW) {
            play();
        }
    }
}

void loop() {}


void moveRocket(int16_t *y, int type = 0) {
    display.drawBitmap(ROCKET_INIT_X, *y, rocket1, ROCKET_WIDTH, ROCKET_HEIGHT, SSD1306_WHITE);
}

void playExplosionSound() {
    int stepDuration = explosionDuration / explosionNumSteps;  // Duration of each step
    int frequencyStep = (explosionEndFrequency - explosionStartFrequency) / explosionNumSteps; // Frequency step size

    // Gradually increase the frequency
    for (int i = 0; i < explosionNumSteps; i++) {
        int frequency = explosionStartFrequency + i * frequencyStep;
        tone(PIEZO_PIN, frequency, stepDuration);
        delay(stepDuration);
    }

    // Gradually decrease the frequency
    for (int i = explosionNumSteps - 1; i >= 0; i--) {
        int frequency = explosionStartFrequency + i * frequencyStep;
        tone(PIEZO_PIN, frequency, stepDuration);
        delay(stepDuration);
    }
}

void introMessage() {
    display.clearDisplay();

    // Set text size and color for "Rockets" title
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    // Draw "Rockets" title
    display.setCursor(0, 10);
    display.println("Rockets");

    // Display the "Rockets" title
    display.display();

    // Delay for 1 second
    delay(1000);

    // Number of falling pixels
    int numPixels = 50;

    // Animation loop for falling pixels
    for (int i = 0; i < 50; i++) { // Increase the duration of the animation
        // Clear the display
        display.clearDisplay();

        // Draw "Rockets" title
        display.setCursor(0, 10);
        display.println("Rockets");

        // Generate and draw multiple falling pixels
        for (int j = 0; j < numPixels; j++) {
            // Generate random position for falling pixel
            int x = random(0, SCREEN_WIDTH);
            int y = random(20, SCREEN_HEIGHT);

            // Draw larger falling pixel
            display.fillRect(x, y, 3, 3, SSD1306_WHITE); // Increase the size of the falling pixel
        }

        // Display the falling pixels
        display.display();

        // Delay for a short time to control the speed of the animation
        delay(50);
    }

    // Clear the display after the animation
    display.clearDisplay();

    display.setCursor(0, 10);
    display.println("Rockets");
    // Set text size and color for "Press button to start game"
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Draw "Press button to start game" message at the bottom of the screen
    display.setCursor(0, SCREEN_HEIGHT - 10);
    display.println("Press button to start game");
    display.display();
}

void moveAstroid(int16_t *x, int type = 0) {
    if (type == 0) {
        display.drawBitmap(*x, ASTROID_Y, astroid1, ASTROID1_WIDTH, ASTROID1_HEIGHT, SSD1306_WHITE);
    } else if (type == 1) {
        display.drawBitmap(*x, ASTROID_Y, astroid2, ASTROID2_WIDTH, ASTROID2_HEIGHT, SSD1306_WHITE);
    }
}

void moveAstroid(int16_t *x, int16_t y, int type = 0) {
    if (type == 0) {
        display.drawBitmap(*x, y, astroid1, ASTROID1_WIDTH, ASTROID1_HEIGHT, SSD1306_WHITE);
    } else if (type == 1) {
        display.drawBitmap(*x, y, astroid2, ASTROID2_WIDTH, ASTROID2_HEIGHT, SSD1306_WHITE);
    }
}

void gameOver(int score = 0) {
    // Update high score if the current score is higher
    if (score > highScore) {
        highScore = score;
        // Write the new high score to EEPROM
        EEPROM.put(highScoreAddress, highScore);
    }

    playExplosionSound();
    display.clearDisplay();

    // Set text size for "Game Over" line
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    // Draw "Game Over" text
    display.setCursor(0, 10);
    display.println("Game Over");
    display.display();

    // Delay for 1 second
    delay(1000);

    // Set text size for score and play again lines
    display.setTextSize(1);

    // Draw "Score: XX" text
    display.setCursor(0, 30);
    display.println("Score: " + String(score));

    // Display high score on the game over screen
    display.setCursor(0, 40);
    display.print("High Score: ");
    display.print(highScore);

    // Draw "Press 1 To Play Again" text
    display.setCursor(0, 55);
    display.println("Press to Restart ->");

    display.display();

    // Ensure the final text stays on the screen for a moment
    delay(1000); // 1 second delay before returning
}

void displayScore(int score) {
    display.setTextSize(1);
    display.setCursor(64, 10);
    display.print("Score: ");
    display.print(score);
}

void countdownBeeps() {
    // Countdown beeps
    for (int i = 3; i >= 1; i--) {
        tone(PIEZO_PIN, 1000, 200); // Beep at 1000Hz for 200ms
        delay(1000); // Delay for 500ms between beeps
    }
    // Last long beep
    tone(PIEZO_PIN, 1000, 1000); // Beep at 1000Hz for 500ms
}

void play() {
    int16_t astroid_x = 127;
    int16_t astroid1_x = 195;  // Separate variable for the second astroid

    // Define different speed values for each astroid
    int astroid_speed = 3;
    int astroid1_speed = 2;

    int astroid_type = random(0, 2);
    int astroid_type1 = random(0, 2);

    int16_t rocket_y = ROCKET_INIT_Y;
    int score = 0;
    bool isJumping = false;
    int frameDelay = 50;
    int speedIncrement = 10;
    int minFrameDelay = 5;

    // Adjust the range to cover the entire vertical screen
    int16_t astroid_y

 = random(0, SCREEN_HEIGHT - ASTROID1_HEIGHT);
    int16_t astroid1_y = random(0, SCREEN_HEIGHT - ASTROID2_HEIGHT);

    // Set the equal rise and fall speed
    int jumpSpeed = 4; // Adjust as needed
    // Call countdown beeps function
    countdownBeeps();
    bool astroidUpdatedLast = true; // Assume astroid_y was updated last initially

    for (;;) {
        display.clearDisplay();
        // Determine if astroids are displayed on the top or bottom half of the screen based on the most recently updated astroid
        int astroidPosition = (astroidUpdatedLast && (astroid_y < SCREEN_HEIGHT / 2)) || (!astroidUpdatedLast && (astroid1_y < SCREEN_HEIGHT / 2)) ? 0 : 1; // 0 for top, 1 for bottom

        // Activate the corresponding LED
        digitalWrite(LED_TOP_PIN, (astroidPosition == 0) ? HIGH : LOW);
        digitalWrite(LED_BOTTOM_PIN, (astroidPosition == 1) ? HIGH : LOW);

        if (digitalRead(BUTTON_PIN) == LOW) {
            if (!isJumping && rocket_y > 0) {
                isJumping = true;
                digitalWrite(VIBRATION_PIN, HIGH); // Turn on vibration motor when jumping
            }
        } else {
            isJumping = false;
            digitalWrite(VIBRATION_PIN, LOW); // Turn off vibration motor when not jumping
        }

        // Update rocket position based on jump status
        if (isJumping) {
            // Increase the rate of ascent
            if (rocket_y > 0) {
                rocket_y -= jumpSpeed;
            }
        } else {
            // Decrease the rate of descent
            rocket_y += jumpSpeed; // Set the fall speed to be the same as the rise speed
            // Ensure rocket does not fall off the bottom of the screen
            if (rocket_y > (SCREEN_HEIGHT - ROCKET_HEIGHT)) {
                rocket_y = SCREEN_HEIGHT - ROCKET_HEIGHT;
            }
        }

        score++;

        int rocket_center_x = ROCKET_INIT_X + (ROCKET_WIDTH / 2);
        int rocket_center_y = rocket_y + (ROCKET_HEIGHT / 2);

        int collision_tolerance_x_astroid = 5;
        int collision_tolerance_y_astroid = 2;
        int collision_tolerance_x_astroid1 = 5;
        int collision_tolerance_y_astroid1 = 2;

        // Check collision with the first astroid
        if ((rocket_center_x >= (astroid_x - collision_tolerance_x_astroid)) &&
            (rocket_center_x <= (astroid_x + ASTROID1_WIDTH + collision_tolerance_x_astroid)) &&
            (rocket_center_y >= (astroid_y - collision_tolerance_y_astroid)) &&
            (rocket_center_y <= (astroid_y + ASTROID1_HEIGHT + collision_tolerance_y_astroid))) {
            Serial.println("Collision Happened with astroid 1");
            // Display explosion animation
            displayExplosion();
            delay(500); // Delay for explosion effect
            break;
        }

        // Check collision with the second astroid
        if ((rocket_center_x >= (astroid1_x - collision_tolerance_x_astroid1)) &&
            (rocket_center_x <= (astroid1_x + ASTROID2_WIDTH + collision_tolerance_x_astroid1)) &&
            (rocket_center_y >= (astroid1_y - collision_tolerance_y_astroid1)) &&
            (rocket_center_y <= (astroid1_y + ASTROID2_HEIGHT + collision_tolerance_y_astroid1))) {
            Serial.println("Collision Happened with astroid 2");
            // Display explosion animation
            displayExplosion();
            delay(500); // Delay for explosion effect
            break;
        }

        // Adjusted collision detection to prevent game over when rocket hits top of the screen
        // Check collision with the top of the screen
        if (rocket_y <= 0) {
            // Don't break the loop, continue the game
            rocket_y = 0; // Reset rocket's y position to top of the screen
        }

        displayScore(score);
        moveRocket(&rocket_y);
        moveAstroid(&astroid_x, astroid_y, astroid_type);
        moveAstroid(&astroid1_x, astroid1_y, astroid_type1);

        astroid_x -= astroid_speed;
        astroid1_x -= astroid1_speed;

        // Update the height of each astroid separately when it goes off the screen
        if (astroid_x <= -ASTROID1_WIDTH) {
            astroid_x = SCREEN_WIDTH;
            astroid_y = random(0, SCREEN_HEIGHT - ASTROID1_HEIGHT); // Adjusted for entire vertical range
            astroidUpdatedLast = true; 
        }

        if (astroid1_x <= -ASTROID2_WIDTH) {
            astroid1_x = SCREEN_WIDTH;
            astroid1_y = random(0, SCREEN_HEIGHT - ASTROID2_HEIGHT); // Adjusted for entire vertical range
            astroidUpdatedLast = false; 
        }

        display.display();
        delay(frameDelay);

        if (score % 100 == 0) {
            if (frameDelay > minFrameDelay) {
                frameDelay -= speedIncrement;
            }
        }

        if (frameDelay <= minFrameDelay) {
            frameDelay = minFrameDelay;
        }
    }

    Serial.println("Game Over");
    gameOver(score);
}

void displayExplosion() {
    for (int i = 0; i < 3; i++) {
        // Activate vibration motor during flashing screen effect
        digitalWrite(VIBRATION_PIN, HIGH);

        display.clearDisplay();
        // Flashing screen effect
        display.fillScreen(SSD1306_WHITE);
        display.display();
        delay(100);
        
        // Deactivate vibration motor
        digitalWrite(VIBRATION_PIN, LOW);

        display.clearDisplay();
        display.display();
        delay(100);
    }

    // Draw explosion animation
    for (int r = 0; r < SCREEN_HEIGHT+50; r += 4) {
        display.clearDisplay();
        display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, r, SSD1306_WHITE);
        display.display();
        delay(30);
    }
}


void renderScene(int16_t i=0){
  display.drawBitmap(10, 29, rocket1, 25, 26, SSD1306_WHITE);
  display.drawBitmap(50, ASTROID_Y, astroid1, 11, 23, SSD1306_WHITE);
  display.drawBitmap(100, ASTROID_Y, astroid2, 22, 23, SSD1306_WHITE);
  display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
  display.drawPixel(i, 60, SSD1306_WHITE);
}
