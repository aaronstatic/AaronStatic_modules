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

int voltage_to_note_with_octave(float value) {
	float octave = round(value);
	float rest = value - 1.0*octave;
	int semi = (int)(octave * 12.0f) + round(rest * 12.0);
	return semi;
}

int voltage_to_note_int(float value) {
	float octave = round(value);
	float rest = value - 1.0*octave;
	int semi = (int)round( rest*12.0 );
	if(semi < 0) semi += 12;
	return semi;
}

//Intervals (for readability)
const int MINOR_SECOND 		= 1;
const int MAJOR_SECOND 		= 2;
const int MINOR_THIRD 		= 3;
const int MAJOR_THIRD 		= 4;
const int PERFECT_FOURTH 	= 5;
const int DIMINISHED_FIFTH 	= 6;
const int PERFECT_FIFTH 	= 7;
const int AUGMENTED_FIFTH 	= 8;
const int MAJOR_SIXTH 		= 9;
const int MINOR_SEVENTH 	= 10;
const int MAJOR_SEVENTH 	= 11;

//Chords
static const int CHORD_DEGREES[9][3] = {
    {MAJOR_THIRD,PERFECT_FIFTH,0}, //Major
    {MINOR_THIRD,PERFECT_FIFTH,0}, //Minor
    {MAJOR_THIRD,PERFECT_FIFTH,MINOR_SEVENTH}, //Dominant 7
    {MINOR_THIRD,PERFECT_FIFTH,MINOR_SEVENTH}, //Minor 7
    {MAJOR_THIRD,PERFECT_FIFTH,MAJOR_SEVENTH}, //Major 7
    {MAJOR_SECOND,PERFECT_FIFTH,0}, //sus2
    {PERFECT_FOURTH,PERFECT_FIFTH,0}, //sus4
    {MINOR_THIRD,DIMINISHED_FIFTH,0}, //dim
    {MAJOR_THIRD,AUGMENTED_FIFTH,0} //aug
};

struct chord get_chord(int root_note, int type, int inversion, int voicing){
    struct chord return_chord;

    const int *degrees = CHORD_DEGREES[type];

	int third_note = root_note + degrees[0];
	int fifth_note = root_note + degrees[1];
	int seventh_note = root_note + degrees[2];

	if(degrees[2] == 0){     //Has 3 notes
		return_chord.notes_pre[0] = root_note;
        return_chord.notes_pre[1] = third_note;
        return_chord.notes_pre[2] = fifth_note;

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
		return_chord.notes_pre[0] = root_note;
        return_chord.notes_pre[1] = third_note;
        return_chord.notes_pre[2] = fifth_note;
        return_chord.notes_pre[3] = seventh_note;

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
	return_chord.inversion = inversion;
    return return_chord;
}

struct chord get_diatonic_chord(int* notes, int num_notes, int octave, int chord, int type, int inversion, int voicing){
	struct chord return_chord;

	int chord_length = 3 + type;
	int actual_length = 0;
	if(inversion > chord_length - 1) inversion = chord_length - 1;
	return_chord.inversion = inversion;

	//Build the chord from the scale

	for(int t=0; t < chord_length; t++){
		int index = chord + (t * 2);
		int transpose = 0;

		//Wrap if needed
		while(index >= num_notes){
			index -= num_notes;
			transpose++;
		}

		if(index < num_notes && index > -1){
			return_chord.notes_pre[t] = (transpose * 12) + notes[index];
			actual_length++;
		}
	}

	return_chord.num_notes = actual_length;

	//Transpose the chord
	for(int t=0; t < actual_length; t++){
		return_chord.notes[t] = (octave * 12) + return_chord.notes_pre[t];
	}

	//Perform inversion
	if(inversion > 0 && actual_length > 1){
		int new_notes[6];
		int index = inversion;
		for(int t=0; t < actual_length; t++){
			int i = index;
			if(i >= actual_length) {
				i -= actual_length;
				new_notes[t] = return_chord.notes[i] + 12;
			}else{
				new_notes[t] = return_chord.notes[i];
			}
			index++;
		}
		for(int t=0; t < actual_length; t++){
			return_chord.notes[t] = new_notes[t];
		}
	}

	//Voice the chord
	if(voicing > 0 && actual_length > 2){
		int root_note = return_chord.notes[0];
		if(root_note > 11){
			if(voicing == 1){
				return_chord.notes[0] -= 12;
			}
			if(voicing == 2){
				return_chord.notes[0] -= 12;
				return_chord.notes[2] -= 12;
			}
			if(voicing == 3){
				return_chord.notes[0] -= 12;
				return_chord.notes[1] += 12;
			}
			if(voicing == 4){
				return_chord.notes[0] -= 12;
				if(actual_length > 3){
					return_chord.notes[3] -= 12;
				}
			}
		}
	}

	return return_chord;
}

static const char * NOTE_NAMES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const char * CHORD_TYPE_NAMES[] = {
  "",		//0
  "m",		//1
  "7",		//2
  "m7",		//3
  "maj7",	//4
  "sus2",	//5
  "sus4",	//6
  "dim",	//7
  "+",		//8
  "9",		//9
  "m9",		//10
  "maj9",	//11
  "+M7",	//12
  "+M9",	//13
  "mM7",	//14
  "mM9",	//15
  "dim7",	//16
  "dim9",	//17
  "m7b5",	//18
  "+7",		//19
  "mM7b5",	//20
  "7b5",	//21
  "M7b5",	//22
  "7b9",	//23
  "6/9",	//24
  "mb9",	//25
  "maj79",	//26
  "dimb9",	//27
  "dim9"	//28
};

void get_chord_name(int root_semi, int chord_type, bool inverted, int bass_note, char* text) {
    char inv[4];
    if(inverted){
        sprintf(inv,"/%s",NOTE_NAMES[bass_note]);
    }
    sprintf(text, "%s%s%s", NOTE_NAMES[root_semi], CHORD_TYPE_NAMES[chord_type], inv);
}

void detect_chord_name_simple(struct chord chord, char* text){
	//Detect the chord type (if we can)
	int chord_type = 0; //default to major

	if(chord.num_notes > 2){
		//don't bother unless theres at least 3 notes

		//build an array of intervals
		int intervals[5] = {0,0,0,0,0};
		for(int t=1; t < chord.num_notes; t++){
			intervals[t-1] = chord.notes_pre[t] - chord.notes_pre[t-1];
		}



		//Is there a better way to do this? if you know please submit a pull request xD
		if(chord.num_notes > 3){
			//Its a 7 or 9 chord
			if(intervals[0] == MAJOR_THIRD){
				if(intervals[1] == MINOR_THIRD){
					//major triad
					if(intervals[2] == MINOR_THIRD) {
						chord_type = 2; //dominant seventh
						if(chord.num_notes == 5 && intervals[3] == MINOR_THIRD) chord_type = 23; //dominant minor ninth
						if(chord.num_notes == 5 && intervals[3] == MAJOR_THIRD) chord_type = 9; //dominant ninth
					}
					if(intervals[2] == MAJOR_THIRD) {
						chord_type = 4; //major seventh
						if(chord.num_notes == 5 && intervals[3] == MAJOR_THIRD) chord_type = 11; //major ninth
						if(chord.num_notes == 5 && intervals[3] == MINOR_THIRD) chord_type = 26; //major seventh ninth
					}
					if(intervals[2] == MAJOR_SECOND) {
						if(chord.num_notes == 5 && intervals[3] == PERFECT_FOURTH) chord_type = 24; //6/9
					}
				}
				if(intervals[1] == MAJOR_THIRD){
					//augmented triad
					if(intervals[2] == MINOR_THIRD) chord_type = 12; //augmented major seventh
					if(intervals[2] == MAJOR_SECOND) chord_type = 19; //augmented seventh
				}
				if(intervals[1] == MAJOR_SECOND){
					if(intervals[2] == MAJOR_THIRD) chord_type = 21; //dominant seventh flat 5
					if(intervals[2] == PERFECT_FOURTH) chord_type = 22; //major seventh flat 5
				}
			}
			if(intervals[0] == MINOR_THIRD){
				if(intervals[1] == MAJOR_THIRD){
					//minor triad
					if(intervals[2] == MINOR_THIRD) {
						chord_type = 3; //minor seventh
						if(chord.num_notes == 5 && intervals[3] == MAJOR_THIRD) chord_type = 10; //minor ninth
						if(chord.num_notes == 5 && intervals[3] == MINOR_THIRD) chord_type = 25; //minor flat ninth
					}
					if(intervals[2] == MAJOR_THIRD) chord_type = 14; //minmaj seventh
				}
				if(intervals[1] == MINOR_THIRD){
					//diminished triad
					if(intervals[2] == MINOR_THIRD) chord_type = 16; //diminished seventh
					if(intervals[2] == MAJOR_THIRD) {
						chord_type = 18; //half diminished seventh
						if(chord.num_notes == 5 && intervals[3] == MINOR_THIRD) chord_type = 27; //diminished flat ninth
						if(chord.num_notes == 5 && intervals[3] == MAJOR_THIRD) chord_type = 28; //diminished ninth
					}
					if(intervals[2] == PERFECT_FOURTH) chord_type = 20; //diminished major seventh
				}
			}
		}else{
			//Its just a triad
			if(intervals[0] == MINOR_THIRD){
				if(intervals[1] == MAJOR_THIRD) chord_type = 1; //minor
				if(intervals[1] == MINOR_THIRD) chord_type = 7; //diminished
			}
			if(intervals[0] == MAJOR_THIRD && intervals[1] == MAJOR_THIRD) chord_type = 8; //augmented
			if(intervals[0] == MAJOR_SECOND && intervals[1] == PERFECT_FOURTH) chord_type = 5; //sus2
			if(intervals[0] == PERFECT_FOURTH && intervals[1] == MAJOR_SECOND) chord_type = 6; //sus4
		}

		int root_note = chord.notes_pre[0] % 12;

	    if(chord.inversion > 0){
			int bass_note = chord.notes[0] % 12;
			sprintf(text, "%s%s/%s", NOTE_NAMES[root_note], CHORD_TYPE_NAMES[chord_type], NOTE_NAMES[bass_note]);
	    }else{
			sprintf(text, "%s%s", NOTE_NAMES[root_note], CHORD_TYPE_NAMES[chord_type]);
		}
	}else{
		sprintf(text, "         ");
	}
}

void get_note_name(int note, char* text) {
    sprintf(text, "%s", NOTE_NAMES[note % 12]);
}

//Scales
static const char * MODE_NAMES[] = {
  "",
  " Dorian",
  " Phrygian",
  " Lydian",
  " Mixolydian",
  " Minor",
  " Locrian"
};

void get_scale_name(int root_semi, int mode, char* text) {
    sprintf(text, "%s%s", NOTE_NAMES[root_semi], MODE_NAMES[mode]);
}

static const int MODE_DEGREES[7][7] = {
    {2,2,1,2,2,2}, //Major (Ionian)
    {2,1,2,2,2,1}, //Dorian
    {1,2,2,2,1,2}, //Phrygian
    {2,2,2,1,2,2}, //Lydian
    {2,2,1,2,2,1}, //Mixolydian
    {2,1,2,2,1,2}, //Minor (Aeolian)
    {1,2,2,1,2,2}, //Locrian
};

struct scale get_scale(int root_note, int mode){
    struct scale return_scale;

    const int *degrees = MODE_DEGREES[mode];

    return_scale.notes[0] = root_note;
    int current_note = root_note;

    int t;
    for(t=1; t<7; t++){
        current_note += degrees[t-1];
        return_scale.notes[t] = current_note;
    }
    return return_scale;
}
