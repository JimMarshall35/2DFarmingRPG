# Audio

The Game uses the OpenAL-Soft library and has a separate thread to which audio commands are dispatched. So far it supports synthesizing sound effects on the fly using my ZzFX C port, in the future it will support wav sound effects loaded on a per game layer basis and streamed audio.

## ZzFX-C

You can use ZzFX to generate sound effects that sound like 8 bit arcade sounds, cd into the ZzFX directory once the submodule has been initialized and run GUI.sh to run a gui from which you can create new sounds - for this to work you need to install matplotlib - in the bottom is a string you can use to inialize a struct of type `struct ZZFXSound` from the ZzFX.h header. 

https://www.youtube.com/watch?v=UhUfriUSWX4

You can then include the "Audio.h" header and call `Au_PlayZzFX` from your C code passing a pointer to the sound effect struct to play it.
