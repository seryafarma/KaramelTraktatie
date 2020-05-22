/*!
 *    \file KaramelTraktatie.ino
 *    \brief Arduino Leonardo Digital Clock with temperature sensor.
 *
 *    For this project, lots of libraries from other people have been included.
 * Thank you so much for your great work . The awesome people with their
 * libraries are:
 *
 *    LedControl by Wayoda
 *    http://playground.arduino.cc/Main/LedControl
 *    https://github.com/wayoda/LedControl/issues
 *
 *    OneWire, maintained by Paul Stoffregen (website)
 *    http://playground.arduino.cc/Learning/OneWire
 *
 *    DS1307, Updated by bricofoy from Arduino Forums
 *    http://forum.arduino.cc/index.php/topic,93077.0.html
 */

#include <DS1307.h>
#include <LedControl.h>
#include <OneWire.h>
#include <Wire.h>

//------------------------------------------------------------------------------
// Class: Timekeeper
// Takes care of the time.
//------------------------------------------------------------------------------
class TimeKeeper {
   private:
    int16_t time[7];

   public:
    int16_t* updateAndGetTime() {
	// Get the data from DS1307 RTC.
	RTC.get(static_cast<int*>(time), true);
	return time;
    }
    void setTime(uint8_t* input_time) {
	RTC.stop();
	RTC.set(DS1307_SEC, input_time[6]);
	RTC.set(DS1307_MIN, input_time[5]);
	RTC.set(DS1307_HR, input_time[4]);
	RTC.set(DS1307_DOW, input_time[3]);
	RTC.set(DS1307_DATE, input_time[2]);
	RTC.set(DS1307_MTH, input_time[1]);
	RTC.set(DS1307_YR, input_time[0]);
	RTC.start();
    }
    TimeKeeper() {
	RTC.stop();
	delay(100);
	RTC.start();
    }
};

//------------------------------------------------------------------------------
// Class: Displayer
// Displays the digits to the seven segment display.
//------------------------------------------------------------------------------
class Displayer {
   private:
    const uint8_t din;
    const uint8_t clk;
    const uint8_t cs;
    const uint8_t numOfController;
    uint8_t temp;
    uint8_t dp;
    LedControl* lc;

   public:
    Displayer() : din(4), clk(6), cs(5), numOfController(1) {
	lc = new LedControl(this->din, this->clk, this->cs,
			    this->numOfController);
	lc->shutdown(0, false);
	lc->setIntensity(0, 8);
	lc->clearDisplay(0);
	this->temp = 0;
	this->dp = 0;
    }
    ~Displayer() { delete lc; }
    void Display(int16_t* time, int16_t* temperature) {
	static uint8_t conversionMatrix[] = {0x7E, 0x30, 0x6D, 0x79, 0x33,
					     0x5B, 0x5F, 0x70, 0x7F, 0x7B};
	// Get the hours.
	uint8_t hours1st = *(time + 2) / 10;
	uint8_t hours2nd = *(time + 2) % 10;
	// Get the minutes.
	uint8_t minutes1st = *(time + 1) / 10;
	uint8_t minutes2nd = *(time + 1) % 10;
	// Get temporary value.
	this->temp = conversionMatrix[minutes2nd];
	// Print out the hours and minutes.
	lc->setRow(0, 7, conversionMatrix[hours1st]);
	lc->setRow(0, 6, conversionMatrix[hours2nd] | 0x80);
	lc->setRow(0, 5, conversionMatrix[minutes1st]);
	lc->setRow(0, 4, conversionMatrix[minutes2nd]);
	// Print out the temperatures.
	if (temperature != NULL) {
	    // Celsius.
	    lc->setRow(0, 0, 0x4E);
	    if (*temperature < 0) {
		lc->setRow(0, 3, 0x01);
	    } else {
		lc->setRow(0, 3, 0x00);
	    }
	    uint8_t temperature1st = *(temperature) / 10;
	    uint8_t temperature2nd = *(temperature) % 10;
	    lc->setRow(0, 2, conversionMatrix[temperature1st]);
	    lc->setRow(0, 1, conversionMatrix[temperature2nd]);
	}
    }
    void ToggleSeconds() {
	// Set the last place.
	lc->setRow(0, 4, temp | dp);
	// Flip dp value.
	dp = (0x00 == dp) ? 0x80 : 0x00;
    }
};

//------------------------------------------------------------------------------
// Class: TemperatureReader
// Reads the temperature from the sensor.
//------------------------------------------------------------------------------
class TemperatureReader {
   private:
    OneWire* ds;
    const uint8_t din;

   public:
    TemperatureReader() : din(8) { ds = new OneWire(din); }
    ~TemperatureReader() { delete ds; }
    float readBack(bool read) {
	static uint8_t addr[] = {0x22, 0xF4, 0x32, 0x1A,
				 0x00, 0x00, 0x00, 0x1F};
	float celsius = 0.0f;
	if (false == read) {
	    ds->reset();
	    ds->select(addr);
	    ds->write(0x44, 1);
	} else {
	    uint8_t data[12];
	    ds->reset();
	    ds->select(addr);
	    ds->write(0xBE);

	    for (int j = 0; j < 9; j++) {
		data[j] = ds->read();
	    }

	    int16_t raw = (data[1] << 8) | data[0];
	    byte cfg = (data[4] & 0x60);
	    if (cfg == 0x00) {
		raw = raw & ~7;
	    } else if (cfg == 0x20) {
		raw = raw & ~3;
	    } else if (cfg == 0x40) {
		raw = raw & ~1;
	    }
	    celsius = (float)raw / 16.0;
	}
	return celsius;
    }
};

//------------------------------------------------------------------------------
// Class: SerialCommandReader
// Reads commands from the Serial Port.
//------------------------------------------------------------------------------
class SerialCommandReader {
   private:
    char raw_commands[200];
    uint8_t raw_commands_index;
    uint8_t time_value[7];

   public:
    SerialCommandReader() {
	Serial.begin(9600);
	delay(1000);
	Serial.println("-----------------------------------------------");
	Serial.println("KaramelTraktatie: Serial Commander Initialized");
	Serial.println("-----------------------------------------------");
    }

    uint8_t* getTimeValue() { return time_value; }

    bool readSerialCommand() {
	raw_commands[0] = '\0';
	raw_commands_index = 0;

	while (Serial.available()) {
	    raw_commands[raw_commands_index++] = Serial.read();
	    if (raw_commands_index >= 200) {
		raw_commands_index = 0;
	    }
	}
	raw_commands[raw_commands_index] = '\0';

	uint8_t raw_commands_length = raw_commands_index;
	raw_commands_index = 0;

	// Example:
	// SET_YYMMDDdd_hhmmss
	// SET_16030201_123001 (19 bytes)
	if ((raw_commands_length == 19) and (raw_commands[0] == 'S') and
	    (raw_commands[1] == 'E') and (raw_commands[2] == 'T')) {
	    Serial.println("Received a SET TIME command.");
	    time_value[0] =
		(raw_commands[4] - '0') * 10 + (raw_commands[5] - '0');
	    time_value[1] =
		(raw_commands[6] - '0') * 10 + (raw_commands[7] - '0');
	    time_value[2] =
		(raw_commands[8] - '0') * 10 + (raw_commands[9] - '0');
	    time_value[3] =
		(raw_commands[10] - '0') * 10 + (raw_commands[11] - '0');

	    time_value[4] =
		(raw_commands[13] - '0') * 10 + (raw_commands[14] - '0');
	    time_value[5] =
		(raw_commands[15] - '0') * 10 + (raw_commands[16] - '0');
	    time_value[6] =
		(raw_commands[17] - '0') * 10 + (raw_commands[18] - '0');
	    Serial.println("Parameters: ");
	    Serial.println(time_value[0]);
	    Serial.println(time_value[1]);
	    Serial.println(time_value[2]);
	    Serial.println(time_value[3]);
	    Serial.println(time_value[4]);
	    Serial.println(time_value[5]);
	    Serial.println(time_value[6]);
	    Serial.println("");

	    return true;
	}

	return false;
    }
};

//------------------------------------------------------------------------------
// Section: Main
// The main program of the Arduino.
//------------------------------------------------------------------------------

TimeKeeper* tk = NULL;
Displayer* dpl = NULL;
TemperatureReader* tr = NULL;
SerialCommandReader* sc = NULL;

void setup() {
    tk = new TimeKeeper();
    dpl = new Displayer();
    tr = new TemperatureReader();
    sc = new SerialCommandReader();
    // Initial readout
    tr->readBack(false);
    delay(1000);
    int16_t temperature = static_cast<int>(tr->readBack(true));
    int16_t* time = tk->updateAndGetTime();
    dpl->Display(time, &temperature);
}

uint32_t previousMillis = 0;
const uint32_t interval = 1000;

void loop() {
    uint32_t currentMillis = millis();

    if (currentMillis - previousMillis > interval) {
	// Read the command and update time if necessary.
	if (sc->readSerialCommand() == true) {
	    tk->setTime(sc->getTimeValue());
	}

	// Obtain the time.
	int16_t* time = tk->updateAndGetTime();
	// Measure the temperature.
	if (0x00 == time[0]) {
	    tr->readBack(false);
	}
	// Get the measurement one second after.
	else if (0x01 == time[0]) {
	    int16_t temperature = static_cast<int>(tr->readBack(true));
	    dpl->Display(time, &temperature);
	}
	// No measurement takes place.
	else {
	    dpl->Display(time, NULL);
	}
	// Toggle led that indicates seconds.
	dpl->ToggleSeconds();
	// Save the last time.
	previousMillis = currentMillis;
    }
}

