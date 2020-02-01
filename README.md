# Aaron Static Modules
My modules for VCV-Rack (http://vcvrack.com). Just "ChordCV" for now, but more coming soon. If you have any ideas for music theory based modules get me on twitter @aaronstatic

Introduction video: https://www.youtube.com/watch?v=SSoJW4VzTp8

To install, just go to the plugin library (https://vcvrack.com/plugins) and search for "Aaron Static" and click the +Add button

## ChordCV
![ChordCV](https://i.imgur.com/pdB5wFX.png "Logo Title Text 1")

Generates a chord and ouputs up to 4 x 1v/oct voltages and a polyphonic 1v/oct signal seperately.

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
