/* Drum Kit for Arduino, based on VS1053 DSP
 * Copyright 2013, James Lyden <james@lyden.org>
 * This code is licensed under the terms of the GNU General Public License.
 * See COPYING, or refer to http://www.gnu.org/licenses, for further details.
 */

// System libraries used for MIDI interface
#include <SPI.h>
#include <SDlite.h>
#include <MidiSynth.h>
#include <SoftwareSerial.h>
// drums.h maps drum notes and channels to names (SNARE, HIHAT, etc)
#include "drums.h"

//----------------------------------------------------------------------------//
// utils contains generic helper functions
//----------------------------------------------------------------------------//

// Quick and dirty check for RAM available after loading program
int FreeRam() {
	char top;
#ifdef __arm__
	return &top - reinterpret_cast<char*>(sbrk(0));
#else  // __arm__
	extern char *__malloc_heap_start;
	extern char *__brkval;
	return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

//----------------------------------------------------------------------------//
// sensorTools provides high-level sensor interface functions
//----------------------------------------------------------------------------//

// Spawn global variables required for hit detection
#define HIT_THRESHOLD 3
#define HIGH_TO_LOW	-1
#define LOW_TO_HIGH	1
#define NO_CHANGE	0
bool playing[DRUM_CHANNELS];
int lastValue[DRUM_CHANNELS];

// gets current sensor value and updates tracking variables. If hit is detected
// (above threshold) between last update and now, returns a positive number.
// If a hit has cleared between last update and now, returns a negative number.
int hitDetected(int sensor) {
	int currentValue;

	// sensors with value of -1 are not used
	if(sensor == -1) return NO_CHANGE;

	// get new reading
	if(sensor < 10) {
		currentValue = analogRead(sensor);
	} else {
		currentValue = digitalRead(sensor);
	}

	// map values from sensor name to array index
	byte index = 128;		// there will never be 128 sensor inputs
	switch(sensor) {
		case SENSOR_BASS:
			index = BASS;
			break;
		case SENSOR_SNARE:
			index = SNARE;
			break;
		case SENSOR_HIHAT:
			index = HIHAT;
			break;
		case SENSOR_HIHAT_P:
			index = HIHAT_P;
			break;
		case SENSOR_CRASH:
			index = CRASH;
			break;
		case SENSOR_TOM_LO:
			index = TOM_LO;
			break;
		case SENSOR_TOM_HM:
			index = TOM_HM;
			break;
		/*
		case SENSOR_RIMSHOT:
			index = RIMSHOT;
			break;
		case SENSOR_RIDE:
			index = RIDE;
			break;
		case SENSOR_SPLASH:
			index = SPLASH;
			break;
		case SENSOR_TOM_LM:
			index = TOM_LM;
			break;
		case SENSOR_TOM_HI:
			index = TOM_HI;
			break;
		case SENSOR_TOM_F_LO:
			index = TOM_F_LO;
			break;
		case SENSOR_TOM_F_HI:
			index = TOM_F_HI;
			break;
		 */
	}

	// return if mapping failed
	if(index == 128) return NO_CHANGE;

	if(sensor >= 10) {
		// transition from low to high
		if(!playing[index] && (currentValue == HIGH)) {
			playing[index] = true;
			lastValue[index] = currentValue; // digital inputs cannot convey force
			return LOW_TO_HIGH;
			// transition from high to low
		} else if(playing[index] && (currentValue == LOW)) {
			playing[index] = false;
			lastValue[index] = currentValue;
			return HIGH_TO_LOW;
		}
		// no transition noted for digital signal
	} else {
		// signal raised above previous by more than threshold
		if(!playing[index] && (currentValue - lastValue[index] > HIT_THRESHOLD)) {
			playing[index] = true;
			lastValue[index] = currentValue; // lastValue now represents force of hit
			return LOW_TO_HIGH;
			// signal dropped below threshold
		} else if(playing[index] && (lastValue[index] - currentValue > HIT_THRESHOLD)) {
			playing[index] = false;
			lastValue[index] = currentValue;
			return HIGH_TO_LOW;
		}
		// no transition noted for analog signal
	}
	// no transitions noted, or some other processing issue occurred
	return NO_CHANGE;
}

// Initializes components of sensorTools to reasonable values
void initSensorTools() {
	for(int i = 0; i < DRUM_CHANNELS; i++) {
		playing[i] = false;
		lastValue[i] = 0;
	}

	for(int pin = 10; pin < 13; pin++)
		pinMode(pin, INPUT);
}

//----------------------------------------------------------------------------//
// midiTools provides high-level MIDI interface objects and functions
//----------------------------------------------------------------------------//

// Spawn global objects required by MP3 library and MIDI handlers
SD sd;
MidiSynth midiSynth;
SoftwareSerial midiPort(2, 3); // RX=2, TX=3

// Sends a MIDI command
// FIXME: Doesn't enforce protocol restrictions. It should check to see that cmd
// is greater than 127, and that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2)
{
	midiPort.write(cmd);
	midiPort.write(data1);

	// Some commands only have one data byte. Most cmds less than 0xBn have 2.
	if( (cmd & 0xF0) <= 0xB0)
		midiPort.write(data2);
}

// Sets the MIDI bank and instrument (0x78 is always percussion)
void selectBank(byte bank = 0x78, byte instrument = 1)
{
	talkMIDI(0xB0, 0, bank); talkMIDI(0xC0, instrument, 0);
}

// Sets master volume for MIDI subsystem
void setMainVolume(byte volume)
{
	talkMIDI(0xB0, 0x07, volume);
}

// Send a MIDI note-on message. Channel ranges from 0-15
void noteOn(byte note, byte channel, byte attack_velocity = 127)
{
	//	talkMIDI( (0x90 | channel), note, attack_velocity);
	talkMIDI( (0x90), note, attack_velocity);
}

// Send a MIDI note-off message.
void noteOff(byte note, byte channel, byte release_velocity = 32)
{
	//	talkMIDI( (0x80 | channel), note, release_velocity);
	talkMIDI( (0x80), note, release_velocity);
}

// Initializes components of midiTools to reasonable values (call from setup())
void initMidiTools()
{
	midiPort.begin(31250);	// MIDI requires 31250 baud

	// Initialize the SD card, used to load patches to DSP
	while(!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
		delay(10000);
	}
	while(!sd.chdir("/")) {
		delay(10000);
	}

	// Initialize the synthesizer
	int result = midiSynth.begin();
	while(result != 0) {
		midiSynth.end();
		delay(10000);
		result = midiSynth.begin();
	}

	// set volume to maximum
	setMainVolume(127);
	selectBank(0x78);

	// play intro now that everything is initialized
	noteOn(PLAY_SNARE, SNARE);
	delay(200);
	noteOn(PLAY_BASS, BASS);
	delay(600);
	noteOn(PLAY_CRASH, CRASH);
	delay(800);
	noteOff(PLAY_SNARE, SNARE);
	noteOff(PLAY_BASS, BASS);
	noteOff(PLAY_CRASH, CRASH);
}

//----------------------------------------------------------------------------//
// Mandatory functions
//----------------------------------------------------------------------------//

void setup()
{
	Serial.begin(115200);	// console I/O

	// Initialize the synthesizer
	initMidiTools();
	// Initialize the sensor tracking
	initSensorTools();

	Serial.print("Free RAM: "); Serial.println(FreeRam());
}

void loop()
{
	// check each sensor (some omitted until additional hardware can be made)
	switch (hitDetected(SENSOR_BASS)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_BASS, BASS);	// digital sensor, no force component
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_BASS, BASS);
			break;
	}

	switch (hitDetected(SENSOR_SNARE)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_SNARE, SNARE, map(lastValue[SNARE],0,1023,64,127));
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_SNARE, SNARE);
			break;
	}

	// skipping rimshot

	switch (hitDetected(SENSOR_HIHAT_P)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_HIHAT_P, HIHAT_P); // digital sensor
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_HIHAT_P, HIHAT_P);
			break;
	}

	switch (hitDetected(SENSOR_HIHAT)) {
		case LOW_TO_HIGH:
			// hihat output varies based on pedal
			if(lastValue[HIHAT_P] == HIGH)
				noteOn(PLAY_HIHAT_C, HIHAT, map(lastValue[HIHAT],0,1023,64,127));
			else
				noteOn(PLAY_HIHAT_O, HIHAT, map(lastValue[HIHAT],0,1023,64,127));
			// stop pedal note if other hihat notes have been triggered
			noteOff(PLAY_HIHAT_P, HIHAT);
			break;
		case HIGH_TO_LOW:
			// current pedal value is no indication of current note, turn both off
			noteOff(PLAY_HIHAT_C, HIHAT);
			noteOff(PLAY_HIHAT_O, HIHAT);
			// stop pedal note if other hihat notes have been triggered
			noteOff(PLAY_HIHAT_P, HIHAT);
			break;
	}

	switch (hitDetected(SENSOR_CRASH)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_CRASH, CRASH, map(lastValue[CRASH],0,1023,64,127));
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_CRASH, CRASH);
			break;
	}

	// skipping ride and splash

	switch (hitDetected(SENSOR_SNARE)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_SNARE, SNARE, map(lastValue[SNARE],0,1023,64,127));
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_SNARE, SNARE);
			break;
	}

	// skipping toms: low/med, high, floor low, and floor high

	switch (hitDetected(SENSOR_TOM_LO)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_TOM_LO, TOM_LO, map(lastValue[TOM_LO],0,1023,64,127));
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_TOM_LO, TOM_LO);
			break;
	}

	switch (hitDetected(SENSOR_TOM_HM)) {
		case LOW_TO_HIGH:
			noteOn(PLAY_TOM_HM, TOM_HM, map(lastValue[TOM_HM],0,1023,64,127));
			break;
		case HIGH_TO_LOW:
			noteOff(PLAY_TOM_HM, TOM_HM);
			break;
	}


	// pause for stability
	delay(100);
}

