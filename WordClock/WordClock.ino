//Pins used by the word-shift-registers
#define CLOCK_PIN 5
#define DATA_PIN 8
#define LATCH_PIN 7

//Array to store which words to light, anything but 0 means light. Use shift() to write to display
byte wordStatus[20];
#pragma region Word to pin mapping
/*
pin		word
|-------|--
0		4
1		över
2		3
3		5
4		9
5		1
6		7
7		10
8		6
9		12
10		2
11		fem
12		klockan är
13		kvart
14		tjugo
15		tio
16		halv
17		i
18		11
19		8
*/
#define KLOCKAN 12
#define FEM 11
#define TIO 7
#define KVART 13
#define TJUGO 14
#define OVER 1
#define I 17
#define HALV 16

//Array to map digits to their pin
const byte digitTable[12] = { 5,10,2,0,3,8,6,19,4,7,18,9 };



#pragma endregion

#pragma region Birtdays definition

typedef struct Day
{
	byte date;
	byte month;
};
const Day birthdays[]
{
	{10,2 },
	{16,2},
	{14,4},
	{29,8}
};
const byte birtdayCount = sizeof(birthdays) / 2;



#pragma endregion





//RTC-object
#include <DS3231.h>
DS3231 rtc(SDA, SCL);

#include <eeprom.h>


void setup()
{
	pinMode(CLOCK_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);
	pinMode(LATCH_PIN, OUTPUT);

	Serial.begin(9600);

	rtc.begin();

}


void loop()
{

	//Print current time to serialport
	Serial.print(rtc.getDateStr());
	Serial.print(" -- ");
	Serial.println(rtc.getTimeStr());

	if (Serial.available() > 0)
	{
		HandleSerial();
	}

	Time time = rtc.getTime();




	delay(1000);
}
void HandleSerial()
{
	Serial.println("reciving..");
	String str = Serial.readString();
	if (str == "set")
	{
		int day, month, year, hour, min, sec;

		Serial.println("Set day:");
		WaitForSerial();
		day = Serial.parseInt();

		Serial.println("Set month:");
		WaitForSerial();
		month = Serial.parseInt();

		Serial.println("Set year:");
		WaitForSerial();
		year = Serial.parseInt();

		Serial.println("Set hour:");
		WaitForSerial();
		hour = Serial.parseInt();

		Serial.println("Set minute:");
		WaitForSerial();
		min = Serial.parseInt();

		Serial.println("Set second:");
		WaitForSerial();
		sec = Serial.parseInt();

		rtc.setDate(day, month, year);
		rtc.setTime(hour, min, sec);

		Serial.println("Time Set!");
	}
}
void WaitForSerial()
{
	while (Serial.available() == 0) {}
}

bool CheckForBirtday(Time time)
{
	byte i = 0;
	for (i = 0; i < birtdayCount; i++)
	{
		Day birtday = birthdays[i];
		if (birtday.date == time.date&&birtday.month == time.mon)
			return true;
	}





}




void shift()
{
	uint8_t i;

	digitalWrite(LATCH_PIN, 0);

	for (i = 0; i < 20; i++) {
		digitalWrite(DATA_PIN, wordStatus[i]);

		digitalWrite(CLOCK_PIN, 1);
		digitalWrite(CLOCK_PIN, 0);
	}

	digitalWrite(LATCH_PIN, 1);
}