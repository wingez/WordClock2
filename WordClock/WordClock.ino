#include <DS3231.h>

#define clock_pin 5
#define data_pin 8
#define latch_pin 7


void setup()
{
	pinMode(clock_pin, OUTPUT);
	pinMode(data_pin, OUTPUT);
	pinMode(latch_pin, OUTPUT);




	Serial.begin(9600);

}

byte counter1 = 1;
byte counter2 = 0;
byte counter3 = 0;


void loop()
{
	

	shift(counter3);
	shift(counter2);
	shift(counter1);

	if (counter2 == 128)
		counter3 = 1;
	else
		counter3 = counter3 << 1;


	if (counter1 == 128)
		counter2 = 1;
	else
		counter2 = counter2 << 1;

	
	counter1 = counter1 << 1;

	while (Serial.available() == 0)
	{

	}
	Serial.read();



}



void shift(uint8_t val)
{
	uint8_t i;

	digitalWrite(latch_pin, 0);

	for (i = 0; i < 8; i++) {
		digitalWrite(data_pin, !!(val & (1 << i)));

		//	digitalWrite(data_pin, !!(val & (1 << (7 - i))));

		digitalWrite(clock_pin, 1);
		digitalWrite(clock_pin, 0);
	}

	digitalWrite(latch_pin, 1);
}