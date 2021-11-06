# Aaron Static Modules
My music theory related modules for VCV-Rack 2.0 (http://vcvrack.com). If you have any ideas for modules or feedback get me on twitter @aaronstatic

ChordCV Introduction video: https://www.youtube.com/watch?v=SSoJW4VzTp8

To install, just go to the plugin library (https://vcvrack.com/plugins) search for "Aaron Static" and click the +Add button

## ChordCV
![ChordCV](https://i.imgur.com/5TeI901.jpg "ChordCV")

Generates a chord and outputs up to 4 x 1v/oct voltages and a polyphonic 1v/oct signal separately.

* **Root**: Chooses the root note (1v/oct, input range -4v to 4v)
* **Type**: Chooses the chord type between major, minor, dominant 7, minor 7, major 7, suspended 2nd, suspended 4th, diminished, augmented (input range -4v to 4v)
* **Inversion**: Chooses chord inversion (input range 0v to 3v)
* **Voicing**: Chooses the chord voicing (input range 0v to 4v)
  * At zero, the chord is voiced as per normal
  * First stop, the root note is dropped -1 octave
  * Second stop, the root and fifth is dropped -1 octave
  * Third stop, the root is dropped 1 octave, the third is raised +1 octave
  * Fourth stop (only if the chord is a 7, m7 or maj7), the root and seventh notes are dropped -1 octave

The first 1v/oct output (and first poly channel) will always be the bass note, in case you wish to route that to a bass oscillator.

If the chord is a triad (3 notes), the 4th 1v/oct output will be the bass note + 1 octave for convenience. The polyphonic output will not include this higher note.

## ScaleCV
![ChordCV](https://i.imgur.com/6X5hZbV.jpg "ScaleCV")

Generates a scale and outputs a polyphonic 1v/oct signal with 7 notes + quantizes the 4 monophonic inputs to that scale if provided.

* **Root**: Chooses the root note (1v/oct, input range -4v to 4v)
* **Mode**: Chooses the scale mode (input range -4v to 4v)
* **Quantizers**: 4 Quantizers that will quantize a monophonic input to the selected scale

## DiatonicCV
![DiatonicCV](https://i.imgur.com/aJNhwkL.jpg "DiatonicCV")

Generates a diatonic chord from the provided scale, if one is provided via the poly input (otherwise C Major is used). Best used in combination with ScaleCV above.

* **Octave**: Which octave to transpose the chord to (1v/oct, input range -4v to 4v)
* **Chord**: Which chord degree in the scale (I - VII, input range 0v to 6v)
* **Type**: Which chord type (triad, seventh, ninth, input range 0v to 3v)
* **Inversion**: Chooses chord inversion (input range 0v to 4v)
* **Voicing**: Chooses the chord voicing (input range 0v to 4v, refer to ChordCV for more info)  

## RandomNoteCV
![RandomNoteCV](https://i.imgur.com/xK91S79.jpg "RandomNoteCV")

Generates a random note when triggered within the selected range and outputs a 1v/oct signal. If a polyphonic input is connected the note is selected from the provided notes (they are quantized).

This module is best used when combined with the above modules.

* **Trigger**: Triggers a new note to be selected
* **Root Bias**: How often should the root (first) note be forced (0-100%, input range -4v to 4v, only valid if polyphonic input is connected)
* **Range**: Range in octaves (Minimum=1, input range -4v to 4v)
