#include <EEPROM.h>
#define CLOCK_PIN 5
#define DATA_PIN 8
#define LATCH_PIN 7



//RTC-object
#include <DS3231.h>
DS3231 rtc(SDA, SCL);


#pragma region Word mapping
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
const byte digitToDisplay[12] = { 9, 5,10,2,0,3,8,6,19,4,7,18 };



#pragma endregion

#pragma region Birthdays related methods

uint8_t birthdays[32];

void LoadBirthdaysFromEEPROM()
{
	for (uint8_t i = 0; i < 32; i++)
	{
		birthdays[i] = EEPROM.read(i);
	}
}

void AddBirthday(uint8_t day, uint8_t month)
{
	for (uint8_t i = 0; i < 32; i += 2)
	{
		if (birthdays[i]==0)
		{
			birthdays[i] = month;
			birthdays[i + 1] = day;
			EEPROM.write(i, month);
			EEPROM.write(i + 1, day);
			return;
		}
	}
	Serial.println("Only 16 birthdays supported");
}
void RemoveBirthday(uint8_t day, uint8_t month)
{
	for (uint8_t i = 0; i < 32; i += 2)
	{
		if (birthdays[i] == month && birthdays[i + 1] == day)
		{
			birthdays[i] = 0;
			birthdays[i + 1] = 0;

			EEPROM.write(i, 0);
			EEPROM.write(i + 1, 0);

			break;
		}
	}


}
void PrintBirthdays()
{
	uint8_t counter;
	for (uint8_t i = 0; i < 32; i += 2)
	{
		if (birthdays[i])
		{
			Serial.print(birthdays[i+1]);
			Serial.print("/");
			Serial.println(birthdays[i]);
			counter++;
		}
	}
	Serial.print("Total: ");
	Serial.println(counter);

}

uint8_t IsBirthday(uint8_t date, uint8_t month)
{
	for (uint8_t i = 0; i < 32; i += 2)
		if (month == birthdays[i] && date == birthdays[i + 1])
			return true;

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
Sunday (31 − ((((5 × year) ÷ 4) + 4) mod 7)) March at 01:00 UTC

To calculate the end of European Summer Time, a variant of the formula above used for October:
Sunday (31 − ((((5 × year) ÷ 4) + 1) mod 7)) October at 01:00 UTC

Taken from
https://en.wikipedia.org/wiki/Summer_Time_in_Europe
*/

uint8_t summerTimeStartDate;
uint8_t summerTimeEndDate;

uint16_t lastYearChecked;


void CalculateSummerTimeDates(uint16_t year)
{
	summerTimeStartDate = 31 - ((((5 * year) / 4) + 4) % 7);
	summerTimeEndDate = 31 - ((((5 * year) / 4) + 1) % 7);

	lastYearChecked = year;
}

bool IsSummerTime(uint8_t date, uint8_t month, uint16_t year)
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


//Methods for displaying rgb-light
#pragma region BirthdayLight

void Partytest()
{
	bitSet(TIMSK1, OCIE1A);
	//Celear the display
	ClearDisplay();

	ShiftPWM.SetAllRGB(0, 255, 0);

	delay(1000);
	ShiftPWM.SetAll(0);
	delay(10);
	bitClear(TIMSK1, OCIE1A);
}

void Party()
{
	bitSet(TIMSK1, OCIE1A);
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


	ShiftPWM.SetAll(0);
	delay(10);
	bitClear(TIMSK1, OCIE1A);
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

#pragma endregion

//Method that returns what words should be turned on at the time
unsigned long GetActiveWords(Time time)
{
	unsigned long display = 0;

	uint8_t	hour = time.hour;
	uint8_t min = time.min;

	if (min < 3)
	{

	}
	else if (min < 8)
	{
		display |= 1L << FEM;
		display |= 1L << OVER;
	}
	else if (min < 13)
	{
		display |= 1L << TIO;
		display |= 1L << OVER;
	}
	else if (min < 18)
	{
		display |= 1L << KVART;
		display |= 1L << OVER;
	}
	else if (min < 23)
	{
		display |= 1L << TJUGO;
		display |= 1L << OVER;
	}
	else if (min < 28)
	{
		display |= 1L << FEM;
		display |= 1L << I;
		display |= 1L << HALV;
	}
	else if (min < 33)
	{
		display |= 1L << HALV;
	}
	else if (min < 38)
	{
		display |= 1L << FEM;
		display |= 1L << OVER;
		display |= 1L << HALV;
	}
	else if (min < 43)
	{
		display |= 1L << TJUGO;
		display |= 1L << I;
	}
	else if (min < 48)
	{
		display |= 1L << KVART;
		display |= 1L << I;
	}
	else if (min < 53)
	{
		display |= 1L << TIO;
		display |= 1L << I;
	}
	else if (min < 58)
	{
		display |= 1L << FEM;
		display |= 1L << I;
	}


	if (IsSummerTime(time.date, time.mon, time.year))
		hour++;

	if (min >= 23)
		hour++;

	hour %= 12;

	display |= 1L << digitToDisplay[hour];
	return display;
}



#define WAITSERIAL() while(Serial.available()==0){if(millis()>timeout){return;}}
void HandleSerial()
{
	Serial.print(rtc.getDateStr());
	Serial.print(" -- ");
	Serial.println(rtc.getTimeStr());

	if (Serial.available() > 0)
	{
		unsigned long timeout = millis() + 100000UL;

		String message = Serial.readString();
		message.toLowerCase();

		if (message == "settime")
		{
			uint8_t hour, min;

			Serial.println("Hour:");
			WAITSERIAL();
			hour = Serial.parseInt();

			Serial.println("Minute:");
			WAITSERIAL();
			min = Serial.parseInt();


			Time time = rtc.getTime();
			if (IsSummerTime(time.date, time.mon, time.year))
				hour--;

			rtc.setTime(hour, min, 0);
		}
		else if (message == "setdate")
		{
			uint16_t year;
			uint8_t mon, day;

			Serial.println("Year:");
			WAITSERIAL();
			year = Serial.parseInt();

			Serial.println("Month:");
			WAITSERIAL();
			mon = Serial.parseInt();

			Serial.println("Day:");
			WAITSERIAL();
			day = Serial.parseInt();

			rtc.setDate(day, mon, year);
		}
		else if (message == "birthdays")
		{
			PrintBirthdays();
		}
		else if (message == "addbirthday")
		{
			uint8_t day, month;
			Serial.println("Month:");
			WAITSERIAL();
			month = Serial.parseInt();

			Serial.println("Day:");
			WAITSERIAL();
			day = Serial.parseInt();

			AddBirthday(day, month);
		}
		else if (message=="removebirthday")
		{
			uint8_t day, month;
			Serial.println("Month:");
			WAITSERIAL();
			month = Serial.parseInt();

			Serial.println("Day:");
			WAITSERIAL();
			day = Serial.parseInt();

			RemoveBirthday(day, month);
		}
		else
		{
			Serial.println("Command not recognized");
		}

	}


}


void setup()
{
	//Initialize pins
	pinMode(CLOCK_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);
	pinMode(LATCH_PIN, OUTPUT);

	ClearDisplay();

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
	ShiftPWM.SetAll(0);
	delay(10);
	//Turn off the timer so it doesnt steal our precous cpu-cycles when it shouldnt
	bitClear(TIMSK1, OCIE1A);

	CalculateSummerTimeDates(rtc.getTime().year);
	LoadBirthdaysFromEEPROM();
	InitTimer2();
}

void loop()
{
	HandleSerial();

	Time time = rtc.getTime();

	if (IsBirthday(time.date, time.mon))
	{
		//	Party();
		Partytest();
	}
	else
	{
		unsigned long words = GetActiveWords(time);
		UpdateDisplay(words);

		delay(1000);
	}


}




unsigned long previousWords = 0L;
void UpdateDisplay(unsigned long words)
{
	if (words == previousWords)
	{
		return;
	}



	Fade(previousWords, words | (1L << KLOCKAN));
	delay(20000L);
	Fade(words | (1L << KLOCKAN), words);


	previousWords = words;
}

unsigned long activeWords;
unsigned long allWords;
unsigned long risingWords;
unsigned long fallingWords;
uint8_t risingTrigger;
uint8_t fallingTrigger;
uint8_t timer_counter;

volatile uint8_t cycleDone;

void Fade(unsigned long fadefrom, unsigned long fadeto)
{
	//Serial.print("Fading from: ");
	//Serial.println(fadefrom, HEX);
	//Serial.print("To: ");
	//Serial.println(fadeto, HEX);

	timer_counter = 0;

	allWords = fadefrom | fadeto;
	risingWords = ~fadefrom & fadeto;
	fallingWords = fadefrom & ~fadeto;
	risingTrigger = 0;
	fallingTrigger = 0;

	uint8_t numIntervals = 100;
	float R = (numIntervals*log10(2)) / log10(255);


	for (uint8_t i = 0; i < numIntervals; i++)
	{
		uint8_t brightness = pow(2, (i / R));

		risingTrigger = pow(2, (i / R));
		fallingTrigger = pow(2, ((numIntervals - i) / R));
		bitSet(TIMSK2, OCIE2A);

		delay(50);
		cycleDone = 1;
		while (cycleDone) {}
	}

	bitClear(TIMSK2, OCIE2A);
	shift(fadeto);
}

ISR(TIMER2_COMPA_vect)
{
	uint8_t shouldShift = 0;
	if (timer_counter == 0)
	{
		activeWords = allWords;
		shouldShift = 1;
	}
	if (timer_counter == risingTrigger)
	{
		activeWords &= ~risingWords;
		shouldShift = 1;
	}
	if (timer_counter == fallingTrigger)
	{
		activeWords &= ~fallingWords;
		shouldShift = 1;
	}

	if (shouldShift)
		shift(activeWords);

	timer_counter++;
	cycleDone = 0;
}

//Initializes timer2 which is used in fade transitions
void InitTimer2()
{
	cli();

	TCCR2A = 0;// set entire TCCR2A register to 0
	TCCR2B = 0;// same for TCCR2B
	TCNT2 = 0;//initialize counter value to 0
			  // set compare match register for 8khz increments
	OCR2A = 20;//
			   // turn on CTC mode
	TCCR2A |= (1 << WGM21);
	// Set CS21 bit for 8 prescaler
	TCCR2B |= (1 << CS21);

	sei();
}



//Clears the display, sets all lamps to 0
void ClearDisplay()
{
	shift(0);
}

//Writes data to the display registers
void shift(unsigned long data)
{
	for (uint8_t i = 0; i < 20; i++) {
		if (data & 0b0001)
			PORTB |= (1 << PB0);
		else
			PORTB &= (~(1 << PB0));

		PORTD |= (1 << PD5);
		PORTD &= (~(1 << PD5));

		data = data >> 1;
	}

	PORTD |= (1 << PD7);
	PORTD &= (~(1 << PD7));
}
