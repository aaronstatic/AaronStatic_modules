#include <math.h>
#include <stdio.h>
#include "musiclib.hpp"

//Note <-> Voltage conversion
float note_to_voltage(int v) {
	float octave = floor((float)v / 12.0) - 4.0;
	float semi = round(v % 12);

	return 1.0*octave + semi/12.0;
}

float voltage_to_note(float value) {
	float octave = round(value);
	float rest = value - 1.0*octave;
	float semi = round( rest*12.0 );
	return semi;
}

int voltage_to_note_int(float value) {
	float octave = round(value);
	float rest = value - 1.0*octave;
	int semi = (int)round( rest*12.0 );
	if(semi < 0) semi += 12;
	return semi;
}



//Chords
static const int CHORD_DEGREES[9][3] = {
    {4,7,0}, //Major
    {3,7,0}, //Minor
    {4,7,10}, //Dominant 7
    {3,7,10}, //Minor 7
    {4,7,11}, //Major 7
    {2,7,0}, //sus2
    {5,7,0}, //sus4
    {3,6,0}, //dim
    {4,8,0} //aug
};

struct chord get_chord(int root_note, int type, int inversion, int voicing){
    struct chord return_chord;

    const int *degrees = CHORD_DEGREES[type];

	int third_note = root_note + degrees[0];
	int fifth_note = root_note + degrees[1];
	int seventh_note = root_note + degrees[2];

	if(degrees[2] == 0){     //Has 3 notes
		if(inversion == 3){
			inversion = 2;
		}
		if(inversion == 1){
            root_note += 12;
            int tmp = root_note;
            root_note = third_note;
            third_note = fifth_note;
            fifth_note = tmp;
		}
		if(inversion == 2){
            root_note += 12;
            third_note += 12;
            int tmp = root_note;
            root_note = fifth_note;
            third_note = fifth_note;
            fifth_note = third_note;
            third_note = tmp;
		}

		if(voicing == 1 && root_note > 11){
			root_note -= 12;
		}
		if(voicing == 2 && root_note > 11){
			root_note -= 12;
			fifth_note -= 12;
		}
		if((voicing == 3 || voicing == 4) && root_note > 11){
			root_note -= 12;
			third_note += 12;
		}
        return_chord.notes[0] = root_note;
        return_chord.notes[1] = third_note;
        return_chord.notes[2] = fifth_note;
        return_chord.num_notes = 3;

	}else{             //Has 4 notes
		if(inversion == 1){
            root_note += 12;
            int tmp = root_note;
            root_note = third_note;
            third_note = fifth_note;
            fifth_note = seventh_note;
            seventh_note = tmp;
		}
		if(inversion == 2){
            root_note += 12;
            third_note += 12;
            int tmp = third_note;
            int r = root_note;
            root_note = fifth_note;
            fifth_note = r;
            third_note = seventh_note;
            seventh_note = tmp;
		}
		if(inversion == 3){
            root_note += 12;
            third_note += 12;
            fifth_note += 12;
            int tmp = fifth_note;
            int r = root_note;
            root_note = seventh_note;
            seventh_note = tmp;
            fifth_note = third_note;
            third_note = r;
		}

		if(voicing == 1 && root_note > 11){
			root_note -= 12;
		}
		if(voicing == 2 && root_note > 11){
			root_note -= 12;
			fifth_note -= 12;
		}
		if(voicing == 3 && root_note > 11){
			root_note -= 12;
			third_note += 12;
		}
		if(voicing == 4 && root_note > 11){
			root_note -= 12;
			seventh_note -= 12;
		}
        return_chord.notes[0] = root_note;
        return_chord.notes[1] = third_note;
        return_chord.notes[2] = fifth_note;
        return_chord.notes[3] = seventh_note;
        return_chord.num_notes = 4;
	}
    return return_chord;
}

static const char * noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const char * chordTypes[] = {
  "",
  "m",
  "7",
  "m7",
  "maj7",
  "sus2",
  "sus4",
  "dim",
  "+"
};

void get_chord_name(int root_semi, int chord_type, bool inverted, int bass_note, char* text) {
    int note = root_semi;
    int type = chord_type;
    char inv[4];
    if(inverted){
        sprintf(inv,"/%s",noteNames[bass_note]);
    }
    sprintf(text, "%s%s%s", noteNames[note], chordTypes[type], inv);
}
