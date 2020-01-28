# Aaron Static Modules
My modules for VCV-Rack. Just "ChordVC" for now, but more coming soon. If you have any ideas for music theory based modules get me on twitter @aaronstatic

## ChordCV
![ChordCV](https://i.imgur.com/pdB5wFX.png "Logo Title Text 1")

Generates a chord and ouputs up to 4 x 1v/oct voltages and a polyphonic 1v/oct signal seperately.

* **Root**: Chooses the root note (1v/oct, input range -4v to 4v)
* **Type**: Chooses the chord type between major, minor, dominant 7, minor 7, major 7, suspended 2nd, suspended 4th, diminished, augmented (input range -4v to 4v)
* **Inversion**: Chooses chord inversion (input range 0v to 3v)
* **Voicing**: Chooses the chord voicing (input range 0v to 4v)

The first 1v/oct output (and first poly channel) will always be the bass note, in case you wish to route that to a bass oscillator.
