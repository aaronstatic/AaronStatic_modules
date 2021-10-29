#pragma once

//Note <-> Voltage conversion
float note_to_voltage(int v);
float voltage_to_note(float value);
int voltage_to_note_int(float value);

//Chords
struct chord {
    int num_notes;
    int notes[4];
};

struct chord get_chord(int root_note, int type, int inversion, int voicing);
void get_chord_name(int root_semi, int chord_type, bool inverted, int bass_note, char* text);

//Scales
struct scale {
    int notes[8];
};
void get_scale_name(int root_semi, int mode, char* text);
struct scale get_scale(int root_note, int mode);
