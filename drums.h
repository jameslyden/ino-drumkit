/* drums.h: Configures drum assignments
 * James Lyden <james@lyden.org>
 *
 * This file contains the mapping from MIDI note to drum name, as well as a
 * number of drum name macros to map data into structures
 */

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

#define HIHAT_C	3
#define HIHAT_O	4
#define HIHAT_P	5

#define CRASH		6
#define RIDE		7
#define SPLASH		8

#define TOM_LO		9
#define TOM_LM		10
#define TOM_HM		11
#define TOM_HI		12
#define TOM_F_LO	13
#define TOM_F_HI	14

// Drum masking
#define MASK_BASS			0x0001
#define MASK_SNARE		0x0002
#define MASK_RIMSHOT		0x0004

#define MASK_HIHAT_C		0x0008
#define MASK_HIHAT_O		0x0010
#define MASK_HIHAT_P		0x0020

#define MASK_CRASH		0x0040
#define MASK_RIDE			0x0080
#define MASK_SPLASH		0x0100

#define MASK_TOM_LO		0x0200
#define MASK_TOM_LM		0x0400
#define MASK_TOM_HM		0x0800
#define MASK_TOM_HI		0x1000
#define MASK_TOM_F_LO	0x2000
#define MASK_TOM_F_HI	0x4000

