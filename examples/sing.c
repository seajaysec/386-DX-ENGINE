/* SPDX-License-Identifier: GPL-3.0-only */
/* Sing a line, a syllable a note, into a WAV file (11025 Hz, mono, 16-bit).
 *   build/sing [out.wav] */
#include "speech.h"
#include <stdio.h>
#include <string.h>

void dx_speech_idle(void) {}            /* called while the engine works: a watchdog would be fed here */

static void u16(FILE *f, unsigned n) { fputc(n & 255, f); fputc((n >> 8) & 255, f); }
static void u32(FILE *f, unsigned n) { u16(f, n); u16(f, n >> 16); }

int main(int argc, char **argv)
{
    static const char text[] = "hello robot friend";
    static const uint8_t tune[] = {60, 64, 67, 64, 72};          /* a MIDI note for each syllable */
    static int16_t out[11025 * 8];
    unsigned at = 0;
    if (dx_speech_init() != 0) {
        fprintf(stderr, "the engine did not start\n");
        return 1;
    }
    static const uint8_t sound[4] = {50, 20, 20, 20};             /* body, breath, edge, glide: 0..100 */
    int syllables = dx_speech_scan(text);
    printf("\"%s\": %d syllables\n", text, syllables);
    for (int s = 0; s < syllables && s < (int)sizeof tune; s++) {
        dx_sing_note_t note = {22050u * 45u / 100u, tune[s]};   /* 0.45 s, counted at the engine's 22050 Hz */
        dx_speech_sound(sound);
        unsigned room = sizeof out / 2 - at;                      /* one syllable: at most 4.5 s */
        int rc = dx_speech_sing_into(0, text, (unsigned)s, &note, 1, 0, out + at,
                                     room < DX_SPEECH_LONG_SAMPLES ? room : DX_SPEECH_LONG_SAMPLES);
        if (rc != 0) {
            fprintf(stderr, "syllable %d was not sung (%d)\n", s, rc);
            return 1;
        }
        at += dx_speech_length[0];
    }
    const char *path = argc > 1 ? argv[1] : "build/sing.wav";
    FILE *f = fopen(path, "wb");
    if (!f)
        return 1;
    fwrite("RIFF", 1, 4, f); u32(f, 36 + at * 2); fwrite("WAVEfmt ", 1, 8, f);
    u32(f, 16); u16(f, 1); u16(f, 1); u32(f, 11025); u32(f, 22050); u16(f, 2); u16(f, 16);
    fwrite("data", 1, 4, f); u32(f, at * 2);
    for (unsigned i = 0; i < at; i++)
        u16(f, (uint16_t)out[i]);
    fclose(f);
    printf("%.2f s sung: %s\n", at / 11025.0, path);
    return 0;
}
