#pragma once
#include "rack.hpp"

using namespace rack;

//Note <-> Voltage conversion
float note_to_voltage(int v);
float voltage_to_note(float value);
int voltage_to_note_int(float value);

//DSP Stuff (credit Marc Boule / Impromptu Modular)
struct Trigger : dsp::SchmittTrigger {
	// implements a 0.1V - 1.0V SchmittTrigger (see include/dsp/digital.hpp) instead of
	//   calling SchmittTriggerInstance.process(math::rescale(in, 0.1f, 1.f, 0.f, 1.f))
	bool process(float in) {
		if (state) {
			// HIGH to LOW
			if (in <= 0.1f) {
				state = false;
			}
		}
		else {
			// LOW to HIGH
			if (in >= 1.0f) {
				state = true;
				return true;
			}
		}
		return false;
	}
};

struct RefreshCounter {
	// Note: because of stagger, and asyncronous dataFromJson, should not assume this processInputs() will return true on first run
	// of module::process()
	static const unsigned int displayRefreshStepSkips = 256;
	static const unsigned int userInputsStepSkipMask = 0xF;// sub interval of displayRefreshStepSkips, since inputs should be more responsive than lights
	// above value should make it such that inputs are sampled > 1kHz so as to not miss 1ms triggers

	unsigned int refreshCounter = (random::u32() % displayRefreshStepSkips);// stagger start values to avoid processing peaks when many Geo and Impromptu modules in the patch

	bool processInputs() {
		return ((refreshCounter & userInputsStepSkipMask) == 0);
	}
	bool processLights() {// this must be called even if module has no lights, since counter is decremented here
		refreshCounter++;
		bool process = refreshCounter >= displayRefreshStepSkips;
		if (process) {
			refreshCounter = 0;
		}
		return process;
	}
};

//Chords
struct chord {
    int num_notes; 		//How many notes in this chord
    int notes[6];		//The notes (post-inversion and transpose)
	int notes_pre[6];	//The notes (before inversion and transpose)
	int inversion;		//The inversion of this chord
	int octave;			//Octave of the root note
};

struct chord get_chord(int root_note, int type, int inversion, int voicing);
void get_chord_name(int root_semi, int chord_type, bool inverted, int bass_note, char* text);

struct chord get_diatonic_chord(int* notes, int num_notes, int octave, int chord, int type, int inversion, int voicing);
void detect_chord_name_simple(struct chord chord, char* text);

//Scales
struct scale {
    int notes[7];
};
void get_scale_name(int root_semi, int mode, char* text);
struct scale get_scale(int root_note, int mode);

//Notes
void get_note_name(int note, char* text);
