/* Drum Kit for Arduino, based on VS1053 DSP
 * James Lyden <james@lyden.org>
 *
 * drums.h contains the mapping from MIDI note to drum name, as well as a
 * number of drum name macros to map data into structures
 */

#ifndef drums_h
#define drums_h

// Drum name to MIDI note
#define PLAY_BASS			35
#define PLAY_SNARE		38
#define PLAY_RIMSHOT		40

#define PLAY_HIHAT_C		42
#define PLAY_HIHAT_O		46
#define PLAY_HIHAT_P		44

#define PLAY_CRASH		49
#define PLAY_RIDE			51
#define PLAY_SPLASH		55

#define PLAY_TOM_LO		45
#define PLAY_TOM_LM		47
#define PLAY_TOM_HM		48
#define PLAY_TOM_HI		50
#define PLAY_TOM_F_LO	41
#define PLAY_TOM_F_HI	43

// Drum indexing/channel assignment
#define BASS		0
#define SNARE		1
#define RIMSHOT	2

#define HIHAT		4
#define HIHAT_P	5

#define CRASH		7
#define RIDE		8
#define SPLASH		9

#define TOM_LO		10
#define TOM_LM		11
#define TOM_HM		12
#define TOM_HI		13
#define TOM_F_LO	14
#define TOM_F_HI	15

#define DRUM_CHANNELS	16

// Drum sensors
#define SENSOR_BASS		10
#define SENSOR_SNARE		3
#define SENSOR_RIMSHOT	-1

#define SENSOR_HIHAT		2
#define SENSOR_HIHAT_P	11

#define SENSOR_CRASH		-1
#define SENSOR_RIDE		-1
#define SENSOR_SPLASH	-1

#define SENSOR_TOM_LO	1
#define SENSOR_TOM_LM	-1
#define SENSOR_TOM_HM	0
#define SENSOR_TOM_HI	-1
#define SENSOR_TOM_F_LO	-1
#define SENSOR_TOM_F_HI	-1

#endif // drums_h
