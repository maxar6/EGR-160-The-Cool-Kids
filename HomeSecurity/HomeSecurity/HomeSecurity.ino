#include <servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Password.h>

//LCD RS pin to digital pin 22
//* LCD Enable pin to digital pin 24
//* LCD D4 pin to digital pin 26
//* LCD D5 pin to digital pin 28
//* LCD D6 pin to digital pin 30
//* LCD D7 pin to digital pin 32

int switch1 = 5;
int switch2 = 6;
int buzzer = 2;
int ledGreen = 8;
int ledYellow = 9;
int ledRed = 10;
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

//This sets up the password for the system
Password password = Password("1875");
int passwordCharacters = 0;
bool pinCorrect = false;

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(switch1, INPUT);
	pinMode(switch2, INPUT);
	pinMode(buzzer, OUTPUT);
	pinMode(ledGreen, OUTPUT);
	pinMode(ledYellow, OUTPUT);
	pinMode(ledRed, OUTPUT);
	pinMode(motion1, INPUT);
	pinMode(motion2, INPUT);
	lcd.begin(16, 2);

	Serial.begin(9600);

	keypad.addEventListener(keypadEvent);
	
	armed = false;
}

// the loop function runs over and over again forever
void loop() {
	if (recieveMessage() == "arm" || pinCorrect)
	{
		lcd.clear();
		password.reset();
		writeDisplay("ARMED Enter Pin to Un-Arm", "Pin:");
		armed = true;
		sendMessage("armed");
		digitalWrite(ledRed, HIGH);
		digitalWrite(ledGreen, LOW);
	}
	else if (recieveMessage() == "disarm" || pinCorrect)
	{
		lcd.clear();
		password.reset();
		writeDisplay("DISARMED Enter Pin to Arm", "Pin:");
		armed = false;
		sendMessage("disarmed");
		digitalWrite(ledRed, LOW);
		digitalWrite(ledGreen, HIGH);
	}

	if (securityCheck() && armed)
	{
		soundAlarm(true);
	}
	else
	{
		soundAlarm(false);
	}

	
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
		return msgRecieve;
	}
}

void sendMessage(String msgSend)
{
	Serial.println(msgSend);
}

void writeDisplay(String displayMsg1, String displayMsg2)
{
	//This will print whatever we want onto the display
	lcd.autoscroll();
	lcd.setCursor(0, 0);
	lcd.print(displayMsg1);
	lcd.setCursor(0, 1);
	lcd.print(displayMsg2);
}

void keypadEvent(KeypadEvent key)
{
	//char keypressed = keypad.getKey();
	switch (keypad.getState())
	{
	case PRESSED:
		switch (key)
		{
		case '#':                 //# is to validate password 
			passwordCheck();
			break;
		case '*':                 //* is to reset password attempt
			password.reset();
			pinClear();
			break;
		default:
			password.append(key);
			passwordCharacters++;
			lcd.setCursor(passwordCharacters + 5, 0);
			lcd.print("*");
			break;
		}
		break;
	}
}

void passwordCheck()
{
	if (password.evaluate())
	{
		pinCorrect = true;
	}
	else
	{
		pinCorrect = false;
		digitalWrite(ledYellow, HIGH);
		delay(100);
		digitalWrite(ledYellow, LOW);
		delay(100);
		digitalWrite(ledYellow, HIGH);
		delay(100);
		digitalWrite(ledYellow, LOW);
	}
}

void pinClear()
{
	for (passwordCharacters = 0; passwordCharacters < 16; passwordCharacters++) //Clears the stars from the screen if there is any
	{
		lcd.setCursor(passwordCharacters + 5, 0);
		lcd.print("");
	}
	passwordCharacters = 0;
}