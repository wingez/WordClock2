#define CLOCK_PIN 5
#define DATA_PIN 8
#define LATCH_PIN 7

//Array to store which words to light, anything but 0 means light. Use shift() to write to display
byte display[20];

//RTC-object
#include <DS3231.h>
DS3231 rtc(SDA, SCL);

void DisplayTime(Time time);

#pragma region Word to pin mapping
/*
pin		word
|-------|--
0		4
1		�ver
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
12		klockan �r
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
#define TIO 15
#define KVART 13
#define TJUGO 14
#define OVER 1
#define I 17
#define HALV 16

//Array to map digits to their pin
const byte digitToDisplay[12] = { 5,10,2,0,3,8,6,19,4,7,18,9 };



#pragma endregion

#pragma region Birthdays definition

typedef struct Day
{
	byte date;
	byte month;
};
const Day birthdays[]
{
	{10,2},
	{16,2},
	{14,4},
	{29,8}
};
const byte birthdayCount = sizeof(birthdays) / 2;

bool IsBirthday(byte date, byte month)
{
	byte i = 0;
	for (i = 0; i < birthdayCount; i++)
	{
		Day birthday = birthdays[i];
		if (birthday.date == date && birthday.month == month)
			return true;
	}
	return false;
}


#pragma endregion

#pragma region ShiftPWM Library Things

const int ShiftPWM_latchPin = 9;
const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;
#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 3;
int numRGBleds = numRegisters * 8 / 3;





#pragma endregion

#pragma region SummerTimeCalculations
/*
Algoritms used:

The formula used to calculate the beginning of European Summer Time is
Sunday (31 − ((((5 × y) ÷ 4) + 4) mod 7)) March at 01:00 UTC

To calculate the end of European Summer Time, a variant of the formula above used for October:
Sunday (31 − ((((5 × y) ÷ 4) + 1) mod 7)) October at 01:00 UTC

Taken from
https://en.wikipedia.org/wiki/Summer_Time_in_Europe
*/

byte summerTimeStartDate;
byte summerTimeEndDate;

uint16_t lastYearChecked;


void CalculateSummerTimeDates(uint16_t year)
{
	summerTimeStartDate = 31 - ((((5 * year) / 4) + 4) % 7);
	summerTimeEndDate = 31 - ((((5 * year) / 4) + 1) % 7);

	lastYearChecked = year;
}

bool IsSummerTime(byte date, byte month, uint16_t year)
{
	if (year != lastYearChecked)
	{
		CalculateSummerTimeDates(year);
	}

	if (month >= 4 && month <= 9)
	{
		return true;
	}
	if (month == 3 && date > summerTimeStartDate)
	{
		return true;
	}
	if (month == 10 && date < summerTimeEndDate)
	{
		return true;
	}
	return false;
}


#pragma endregion




void DisplayTime(Time time)
{
	byte hour;
	byte min;


	//Turn off all rgb-lights
	ShiftPWM.SetAll(0);

	//Turn off all words
	for (byte i = 0; i < 20; i++)
		display[i] = 0;
	display[KLOCKAN] = 1;

	hour = time.hour;
	min = time.min;






	if (min < 3)
	{

	}
	else if (min < 8)
	{
		display[FEM] = 1;
		display[OVER] = 1;
	}
	else if (min < 13)
	{
		display[TIO] = 1;
		display[OVER] = 1;
	}
	else if (min < 18)
	{
		display[KVART] = 1;
		display[OVER] = 1;
	}
	else if (min < 23)
	{
		display[TJUGO] = 1;
		display[OVER] = 1;
	}
	else if (min < 28)
	{
		display[FEM] = 1;
		display[I] = 1;
		display[HALV] = 1;
	}
	else if (min < 33)
	{
		display[HALV] = 1;
	}
	else if (min < 38)
	{
		display[FEM] = 1;
		display[OVER] = 1;
		display[HALV] = 1;
	}
	else if (min < 43)
	{
		display[TJUGO] = 1;
		display[I] = 1;
	}
	else if (min < 48)
	{
		display[KVART] = 1;
		display[I] = 1;
	}
	else if (min < 53)
	{
		display[TIO] = 1;
		display[I] = 1;
	}
	else if (min < 58)
	{
		display[FEM] = 1;
		display[I] = 1;
	}

	if (IsSummerTime(time.date, time.mon, time.year))
		hour++;

	if (min >= 23)
		hour++;

	hour %= 12;

	if (hour == 0)
		hour += 12;

	hour--;



	display[digitToDisplay[hour]] = 1;

	shift();



	delay(1000);
}




void Party()
{
	//Celear the display
	ClearDisplay();

	for (int hue = 0; hue < 360; hue++) {
		ShiftPWM.SetAllHSV(hue, 255, 255);
		delay(50);
	}

	for (int shift = 0; shift < 6; shift++) {
		for (int led = 0; led < numRGBleds; led++) {
			switch ((led + shift) % 6) {
			case 0:
				ShiftPWM.SetRGB(led, 255, 0, 0);    // red
				break;
			case 1:
				ShiftPWM.SetRGB(led, 0, 255, 0);    // green
				break;
			case 2:
				ShiftPWM.SetRGB(led, 0, 0, 255);    // blue
				break;
			case 3:
				ShiftPWM.SetRGB(led, 255, 128, 0);  // orange
				break;
			case 4:
				ShiftPWM.SetRGB(led, 0, 255, 255);  // turqoise
				break;
			case 5:
				ShiftPWM.SetRGB(led, 255, 0, 255);  // purple
				break;
			}
		}
		delay(2000);
	}



	//  A moving rainbow for RGB leds:
	rgbLedRainbow(numRGBleds, 5, 10, numRegisters * 8 / 3); // Fast, over all LED's



	rgbLedRainbow(numRGBleds, 10, 10, numRegisters * 8 / 3 * 4); //slower, wider than the number of LED's


	delay(1000);
}
void rgbLedRainbow(int numRGBLeds, int delayVal, int numCycles, int rainbowWidth) {
	// Displays a rainbow spread over a few LED's (numRGBLeds), which shifts in hue.
	// The rainbow can be wider then the real number of LED's.

	ShiftPWM.SetAll(0);
	for (int cycle = 0; cycle < numCycles; cycle++) { // loop through the hue shift a number of times (numCycles)
		for (int colorshift = 0; colorshift < 360; colorshift++) { // Shift over full color range (like the hue slider in photoshop)
			for (int led = 0; led < numRGBLeds; led++) { // loop over all LED's
				int hue = ((led) * 360 / (rainbowWidth - 1) + colorshift) % 360; // Set hue from 0 to 360 from first to last led and shift the hue
				ShiftPWM.SetHSV(led, hue, 255, 255); // write the HSV values, with saturation and value at maximum
			}
			delay(delayVal); // this delay value determines the speed of hue shift
		}
	}
}

void setup()
{
	//Initialize pins
	pinMode(CLOCK_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);
	pinMode(LATCH_PIN, OUTPUT);

	byte i;
	for (i = 0; i < 20; i++)
		display[i] = 0;
	shift();


	//Initialize Serial
	Serial.begin(128000);

	//Initialize Timer0 responsible for serial communications
	//SetupTimer0();

	//Initialize the rtc
	rtc.begin();


	//Initialize ShiftPWM library
	ShiftPWM.SetAmountOfRegisters(numRegisters);
	ShiftPWM.SetPinGrouping(1);
	ShiftPWM.Start(pwmFrequency, maxBrightness);

	CalculateSummerTimeDates(rtc.getTime().year);

}


void loop()
{
	HandleSerial();



	Time time = rtc.getTime();

	if (IsBirthday(time.date, time.mon))
	{
		Party();
	}
	else
	{
		DisplayTime(time);
	}
}





void SetupTimer0()
{
	cli();

	TCCR0A = 0;
	TCCR0B = 0;
	TCNT0 = 0;

	OCR0A = 254;//todo

	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS02) | (1 << CS00);

	TIMSK0 |= (1 << OCIE0A);

	sei();
	Serial.println("TimerStarted");
}
ISR(TIMER0_COMPA_vect)
{

}
unsigned long lastMessageTime;
void HandleSerial()
{

	Serial.print(rtc.getDateStr());
	Serial.print(" -- ");
	Serial.println(rtc.getTimeStr());

	if (Serial.available() > 0)
	{
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

			if (IsSummerTime(day, month, year))
			{
				hour--;
			}

			rtc.setDate(day, month, year);
			rtc.setTime(hour, min, sec);





			Serial.println("Time Set!");
		}



	}


}
void WaitForSerial()
{
	while (Serial.available() == 0) {}
}





//Clears the display, sets all lamps to 0
void ClearDisplay()
{
	byte i;

	digitalWrite(LATCH_PIN, 0);
	digitalWrite(DATA_PIN, 0);
	for (i = 0; i < 20; i++)
	{
		digitalWrite(CLOCK_PIN, 1);
		digitalWrite(CLOCK_PIN, 0);
	}
	digitalWrite(LATCH_PIN, 1);
}

//displays whats stored in (display)
void shift()
{
	uint8_t i;

	digitalWrite(LATCH_PIN, 0);

	for (i = 0; i < 20; i++) {
		digitalWrite(DATA_PIN, display[i]);

		digitalWrite(CLOCK_PIN, 1);
		digitalWrite(CLOCK_PIN, 0);
	}

	digitalWrite(LATCH_PIN, 1);
}
