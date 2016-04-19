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
int motion2 = 40;
int LEDrelay = 13;

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
byte colPins[COLS] = { 37, 34, 35, 36 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//setting up the string for sending data back and forth between the Arduino and Pi
String msgRecieve, msgSend, m_msgSend;

//This sets up the password for the system
Password password = Password("2389");
int passwordCharacters = 0;
int _passwordCharacters = 0;
bool pinCorrect = false;

//this sets the default system state where 1 = disarmed, 2 = armeds, and 0 = idle state
int systemState = 1;
bool triggered = false;
bool messageDisplayed = false;

void setup() {
	pinMode(switch1, INPUT);
	pinMode(switch2, INPUT);
	pinMode(buzzer, OUTPUT);
	pinMode(ledGreen, OUTPUT);
	pinMode(ledYellow, OUTPUT);
	pinMode(ledRed, OUTPUT);
	pinMode(motion1, INPUT);
	pinMode(motion2, INPUT);
	pinMode(LEDrelay, OUTPUT);

	lcd.begin(16, 2);

	Serial.begin(9600);

	keypad.addEventListener(keypadEvent);
	
	armed = false;

	digitalWrite(buzzer, LOW);
	digitalWrite(ledGreen, LOW);
	digitalWrite(ledYellow, LOW);
	digitalWrite(ledRed, LOW);
	digitalWrite(LEDrelay, LOW);
}

// the loop function runs over and over again forever
void loop() {
	recieveMessage();

	char key = keypad.getKey();
	if (key != NO_KEY)
	{
		Serial.println(key);
	}

	//this if group handles the arming and disarming of the system
	if(armed && pinCorrect)
	{
		systemState = 1;
		pinCorrect = false;
	}
	else if (!armed && pinCorrect)
	{
		systemState = 2;
		pinCorrect = false;
	}

	if (msgRecieve == "arm" || systemState == 2)
	{
		systemState = 0;
		msgRecieve = "";
		writeDisplay("ARMED", "Pin:");
		armed = true;
		sendMessage("armed");
		digitalWrite(ledRed, HIGH);
		digitalWrite(ledGreen, LOW);
		digitalWrite(LEDrelay, HIGH);
	}
	else if (msgRecieve == "disarm" || systemState == 1)
	{
		systemState = 0;
		msgRecieve = "";
		messageDisplayed = false;
		soundAlarm(false);
		writeDisplay("DISARMED", "Pin:");
		armed = false;
		sendMessage("disarmed");
		digitalWrite(ledRed, LOW);
		digitalWrite(ledGreen, HIGH);
		digitalWrite(LEDrelay, LOW);
	}
	else if (systemState == 3)
	{
		systemState = 0;
		if (digitalRead(switch1) == LOW)
		{
			writeDisplay("TRIG Front Door", "PIN:");
		}
		else if (digitalRead(switch2) == LOW)
		{
			writeDisplay("TRIG Back Door", "PIN:");
		}
		else if (digitalRead(motion1) == HIGH)
		{
			writeDisplay("TRIG Motion", "PIN:");
		}
	}

	if (securityCheck() && armed)
	{
		triggered = true;
	}

	if (triggered)
	{
		soundAlarm(true);
		if (messageDisplayed == false)
		{
			systemState = 3;
			messageDisplayed = true;
		}
	}
	Serial.println(digitalRead(motion1));
}

bool securityCheck()
{
	zone1 = digitalRead(switch1);
	zone2 = digitalRead(switch2);
	zone3 = digitalRead(motion1);
	if (!zone1 || !zone2 || zone3)
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
		digitalWrite(ledRed, HIGH);
		digitalWrite(LEDrelay, HIGH);
		delay(100);
		digitalWrite(ledRed, LOW);
		digitalWrite(LEDrelay, LOW);
		delay(100);

		digitalWrite(buzzer, HIGH);
	}
	else
	{
		digitalWrite(buzzer, LOW);
	}
}


void recieveMessage()
{
	msgRecieve = "";
	if (Serial.available())
	{
		msgRecieve = Serial.readString();
	}
}

void sendMessage(String msgSend)
{
	Serial.println(msgSend);
}

void writeDisplay(String displayMsg1, String displayMsg2)
{
	String _displayMsg1 = displayMsg1;
	String _displayMsg2 = displayMsg2;
	//This will print whatever we want onto the display
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(_displayMsg1);
	lcd.setCursor(0, 1);
	lcd.print(_displayMsg2);
	return;
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
			password.reset();
			pinClear();
			break;
		case 'D':                 //* is to reset password attempt
			password.reset();
			pinClear();
			break;
		default:
			password.append(key);
			passwordCharacters++;
			lcd.setCursor(passwordCharacters + 5, 1);
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
		triggered = false;
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
	for (_passwordCharacters = 0; _passwordCharacters < 16; _passwordCharacters++) //Clears the stars from the screen if there is any
	{
		lcd.setCursor(_passwordCharacters + 5, 1);
		lcd.print(" ");
	}
	passwordCharacters = 0;
	_passwordCharacters = 0;
}