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

bool zone1, zone2, zone3, zone4, armed;

LiquidCrystal lcd(22, 24, 26, 28, 30, 32);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = { // Define the Keymap
	{ '1','2','3','A' }
	,
	{ '4','5','6','B' }
	,
	{ '7','8','9','C' }
	,
	{ '*','0','#','D' }
};
byte rowPins[ROWS] = { 23, 25, 27, 29 };
byte colPins[COLS] = { 33, 34, 35, 36 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//setting up the string for sending data back and forth between the Arduino and Pi
String msgRecieve, msgSend, m_msgSend;

// the setup function runs once when you press reset or power the board
void setup() {
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
	if (recieveMessage() = "arm")
	{
		writeDisplay("ARMED", "Enter Pin to Un-Arm");
		armed = true;
	}
	sendMessage("armed");
	if (securityCheck() && armed) soundAlarm(true);
	else soundAlarm(false);h

}

bool securityCheck()
{
	zone1 = digitalRead(switch1);
	zone2 = digitalRead(switch2);
	zone3 = digitalRead(motion1);
	zone4 = digitalRead(motion2);
	if (zone1 || zone2 || zone3 || zone4)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void soundAlarm(bool buzzerState)
{
	if (buzzerState)
	{
		digitalWrite(buzzer, HIGH);
	}
	else
	{
		digitalWrite(buzzer, LOW);
	}
}

String recieveMessage()
{
	msgRecieve = "";
	if (Serial.available())
	{
		msgRecieve = Serial.readString();
	}
	return msgRecieve;
}

void sendMessage(String msgSend)
{
	Serial.println(msgSend);
}

void checkKeypad()
{
	//no clue what to put here yet on how to check the keypad
}

void writeDisplay(String displayMsg1, String displayMsg2)
{
	//This will print whatever we want onto the display
}