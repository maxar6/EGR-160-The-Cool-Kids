#include <servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Password.h>

/*
 LCD RS pin to digital pin 22
 LCD Enable pin to digital pin 24
 LCD D4 pin to digital pin 26
 LCD D5 pin to digital pin 28
 LCD D6 pin to digital pin 30
 LCD D7 pin to digital pin 32
 */

//This is all the DIO for the sensors
int switch1 = 5;
int switch2 = 6;
int buzzer = 2;
int ledGreen = 8;
int ledYellow = 9;
int ledRed = 10;
int motion1 = 12;
int LEDrelay = 13;

//This is all the different zones of the house
bool zone1, zone2, zone3, zone4;

//This sets up the display
LiquidCrystal lcd(22, 24, 26, 28, 30, 32);

//This sets up the keypad
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

//Setting up the string's for sending data back and forth between the Arduino and Pi
String msgRecieve, msgSend, m_msgSend;

//This sets up the password for the system
Password password = Password("2389");
int passwordCharacters = 0;
int _passwordCharacters = 0;
bool pinCorrect = false;

//This sets the default system state where 1 = disarmed, 2 = armeds, and 0 = idle state
int systemState = 1;
bool armed = false;
bool triggered = false;
bool messageDisplayed = false;

void setup() {
	//This sets up the the pins to inputs or outputs
	pinMode(switch1, INPUT);
	pinMode(switch2, INPUT);
	pinMode(buzzer, OUTPUT);
	pinMode(ledGreen, OUTPUT);
	pinMode(ledYellow, OUTPUT);
	pinMode(ledRed, OUTPUT);
	pinMode(motion1, INPUT);
	pinMode(LEDrelay, OUTPUT);

	//This starts the display and sets its size
	lcd.begin(16, 2);

	//This starts the Serail connection to and from the board
	Serial.begin(9600);

	//This adds a event for the keypad when keys get pressed
	keypad.addEventListener(keypadEvent);

	//This sets the default state for some of the outputs that need to be set
	digitalWrite(buzzer, LOW);
	digitalWrite(ledGreen, LOW);
	digitalWrite(ledYellow, LOW);
	digitalWrite(ledRed, LOW);
	digitalWrite(LEDrelay, LOW);
}

void loop() {
	//This check for any new Serial messages
	recieveMessage();

	//This set the char key to whatever the most recent keypress was and prints it out over Serial
	char key = keypad.getKey();
	if (key != NO_KEY)
	{
		Serial.println(key);
	}

	//This if group handles the arming and disarming of the system by setting the system status
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

	if ((securityCheck() && armed) || triggered)
	{
		soundAlarm(true);
		if (messageDisplayed == false)
		{
			systemState = 3;
			messageDisplayed = true;
		}
	}
}

//This check to see if anything has been breached
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

//This sounds the alarm
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

//This will recieve and available Serial messages
void recieveMessage()
{
	msgRecieve = "";
	if (Serial.available())
	{
		msgRecieve = Serial.readString();
	}
}

//This will send any message over Serial we want
void sendMessage(String msgSend)
{
	Serial.println(msgSend);
}

//This is will print whatever we want on the display
void writeDisplay(String displayMsg1, String displayMsg2)
{
	String _displayMsg1 = displayMsg1;
	String _displayMsg2 = displayMsg2;
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

//This check the password inputed on the keypad
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

//This clears the star symbols from the screen for resetting the password
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