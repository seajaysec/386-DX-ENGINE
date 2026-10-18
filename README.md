# 386 DX Engine

This project is inspired by and an homage to Alexei Shulgin, an artist in Russia who performed
briefly as 386 DX, which he billed as “the world’s first cyberpunk band.” At shows, Alexei was
introduced by a computerized voice announcing that the human onstage was merely a decoration.
He’d then “perform” onstage, air-strumming a computer keyboard like an electric guitar as
text-to-speech singing voices played along to MIDI covers of pop songs.

In so many ways, Alexei was—and still is—ahead of his time, and he’s been an inspiration to many
who’ve discovered his music in the decades since 386 DX performed.

Watch it sing: [the 386DX trailer](https://youtu.be/pwLnpyxqBS4).

The singing voice of [386DX](https://github.com/seajaysec/386-DX-FM1-KaraokeBot), the karaoke-robot
firmware for the M-VAVE FM-1, on its own. Give it a line of English words, a syllable number and the
notes that syllable is sung on (one, or several for a held vowel), and it sings that syllable at
those pitches for exactly that long: a robot voice that keeps time.

It is built on [eSpeak NG](https://github.com/espeak-ng/espeak-ng): its English front end turns the
words into phonemes and syllables, and its Klatt cascade-parallel synthesizer makes the sound. Around
them 386DX grew its own singing layer: the line is pronounced once and each syllable placed on its
notes, vowels held across notes and glided between them, consonants kept to their natural length,
the voice shaped by four settings (body, breath, edge, glide). It started close to its origins and
has become its own thing as it was fitted to the FM-1: a small processor, 96 KB of RAM, a second
core to sing on and every byte of flash counted. So it is small: no files at run time (the English
data is compiled in), no allocation once it has started, and plain C.

The engine was written for the firmware, and the firmware is where it shows what it can do: the
[386DX karaoke robot](https://github.com/seajaysec/386-DX-FM1-KaraokeBot) sings songs and typed lines in
it on the M-VAVE FM-1, and its [web page](https://386dx.linecross.ing) runs this same engine, compiled to
WebAssembly, in the browser: write a line, give it notes, and hear it sing before you install anything.

That should make it easy to carry to other projects. `src/` is the engine; `examples/sing.c` sings
"hello robot friend" into a WAV file on a computer.

## The calls

```c
#include "speech.h"
int dx_speech_init(void);                                 /* once: 0 when the voice is ready */
int dx_speech_scan(const char *text);                     /* how many syllables it hears */
void dx_speech_sound(const uint8_t sound[4]);             /* body, breath, edge, glide, 0..100 */
int dx_speech_sing_into(int slot, const char *text, unsigned syllable,
                        const dx_sing_note_t *notes, unsigned count, unsigned expression,
                        int16_t *dst, unsigned capacity); /* 0: dst holds dx_speech_length[slot] samples;
                                                     capacity at most DX_SPEECH_LONG_SAMPLES (4.5 s) */
int dx_speech_articulation(const char *text, uint16_t minimum[64]);  /* each syllable's shortest, in ms */
```

A note is `{samples, note}`: its length counted at 22050 Hz and its MIDI pitch. The sound comes out at
11025 Hz, 16-bit mono. The program supplies `void dx_speech_idle(void)`, called while the engine works
(the FM-1 feeds its watchdog there). Built with `DX_HOST` defined, the engine runs on the calling thread;
without it, `speech_runtime.c` is the FM-1's: the work runs on its second core.

## Building the example

You need Python 3, CMake, a C compiler and eSpeak NG 1.52.0 in `vendor/` (the build checks the revision,
patches a copy in `build/espeak/` and compiles its English data; the checkout itself is never changed):

```sh
git clone --depth 1 --branch 1.52.0 https://github.com/espeak-ng/espeak-ng.git vendor/espeak-ng
tools/build.py
build/sing              # writes build/sing.wav
```

[LICENSING.md](LICENSING.md) says where the code comes from.
