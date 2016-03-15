#include <servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

//LCD RS pin to digital pin 22
//* LCD Enable pin to digital pin 24
//* LCD D4 pin to digital pin 26
//* LCD D5 pin to digital pin 28
//* LCD D6 pin to digital pin 30
//* LCD D7 pin to digital pin 32

int switch1 = 5;
int switch2 = 6;
int buzzer = 2;
int RGB_Red = 8;
int RGB_Green = 9;
int RGB_Blue = 10;
int motion1 = 12;
int motion2 = 13;

bool zone1, zone2, zone3, zone4;

LiquidCrystal lcd(22, 24, 26, 28, 30, 32);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = { // Define the Keymap
	{'1','2','3','A' }
	,
	{'4','5','6','B' }
	,
	{'7','8','9','C' }
	,
	{'*','0','#','D' }
};
byte rowPins[ROWS] = { 23, 25, 27, 29 };
byte colPins[COLS] = { 33, 34, 35, 36 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


// the setup function runs once when you press reset or power the board
void setup(){
	pinMode(switch1, INPUT);
	pinMode(switch2, INPUT);
	pinMode(buzzer, OUTPUT);
	pinMode(RGB_Red, OUTPUT);
	pinMode(RGB_Green, OUTPUT);
	pinMode(RGB_Blue, OUTPUT);
	pinMode(motion1, INPUT);
	pinMode(motion2, INPUT);
	lcd.begin(16, 2);

	Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
	securityCheck();
}

void securityCheck()
{
	zone1 = digitalRead(switch1);
	zone2 = digitalRead(switch2);
	zone3 = digitalRead(motion1);
	zone4 = digitalRead(motion2);
}

void soundAlarm(bool buzzerState)
{
	if(buzzerState)
	{ 
		digitalWrite(buzzer, HIGH);
	}
	else
	{
		digitalWrite(buzzer, LOW);
	}
}
