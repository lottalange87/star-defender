#include "audio.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Generate a laser/pew sound using frequency sweep
Mix_Chunk* generate_laser_sound(int player) {
    // 44.1kHz, 16-bit, mono, ~0.2 seconds
    int sample_rate = 44100;
    float duration = 0.15f;
    int samples = (int)(sample_rate * duration);
    
    Uint16* data = (Uint16*)malloc(samples * sizeof(Uint16));
    if (!data) return NULL;
    
    // Frequency sweep parameters
    float start_freq = player ? 800.0f : 400.0f;
    float end_freq = player ? 200.0f : 100.0f;
    
    for (int i = 0; i < samples; i++) {
        float t = (float)i / samples;
        float freq = start_freq + (end_freq - start_freq) * t;
        
        // Sawtooth wave with decay
        float phase = fmodf(t * freq, 1.0f);
        float sample = (phase * 2.0f - 1.0f);
        
        // Add some noise for texture
        sample += ((float)(rand() % 100) / 100.0f - 0.5f) * 0.3f;
        
        // Exponential decay
        float envelope = expf(-t * 8.0f);
        sample *= envelope;
        
        // Clip and convert to 16-bit
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        data[i] = (Uint16)(sample * 32767.0f + 32768.0f);
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)data, samples * sizeof(Uint16));
    // Note: data is now owned by SDL_mixer, will be freed when chunk is freed
    return chunk;
}

// Generate explosion sound using noise
Mix_Chunk* generate_explosion_sound(int large) {
    int sample_rate = 44100;
    float duration = large ? 0.4f : 0.25f;
    int samples = (int)(sample_rate * duration);
    
    Uint16* data = (Uint16*)malloc(samples * sizeof(Uint16));
    if (!data) return NULL;
    
    for (int i = 0; i < samples; i++) {
        float t = (float)i / samples;
        
        // White noise
        float sample = ((float)(rand() % 1000) / 500.0f - 1.0f);
        
        // Low-pass filter simulation (simple averaging)
        if (i > 0) {
            sample = sample * 0.7f + ((float)(data[i-1] - 32768) / 32767.0f) * 0.3f;
        }
        
        // Exponential decay - slower for large explosions
        float decay = large ? 4.0f : 8.0f;
        float envelope = expf(-t * decay);
        sample *= envelope;
        
        // Clip and convert
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        data[i] = (Uint16)(sample * 32767.0f + 32768.0f);
    }
    
    return Mix_QuickLoad_RAW((Uint8*)data, samples * sizeof(Uint16));
}

// Generate powerup pickup sound
Mix_Chunk* generate_powerup_sound(void) {
    int sample_rate = 44100;
    float duration = 0.3f;
    int samples = (int)(sample_rate * duration);
    
    Uint16* data = (Uint16*)malloc(samples * sizeof(Uint16));
    if (!data) return NULL;
    
    // Arpeggio: C - E - G
    float freqs[] = {523.25f, 659.25f, 783.99f};
    
    for (int i = 0; i < samples; i++) {
        float t = (float)i / samples;
        int note = (int)(t * 3.0f);
        if (note > 2) note = 2;
        
        float freq = freqs[note];
        float phase = fmodf(t * freq, 1.0f);
        
        // Square wave
        float sample = phase < 0.5f ? 0.5f : -0.5f;
        
        // Envelope
        float envelope = expf(-t * 3.0f);
        sample *= envelope;
        
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        data[i] = (Uint16)(sample * 32767.0f + 32768.0f);
    }
    
    return Mix_QuickLoad_RAW((Uint8*)data, samples * sizeof(Uint16));
}

int audio_init(AudioSystem* audio) {
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 1024) < 0) {
        fprintf(stderr, "Failed to open audio: %s\n", Mix_GetError());
        audio->initialized = 0;
        return 0;
    }
    
    // Allocate channels
    Mix_AllocateChannels(16);
    
    // Generate all sounds
    audio->sounds[SOUND_LASER_PLAYER] = generate_laser_sound(1);
    audio->sounds[SOUND_LASER_ENEMY] = generate_laser_sound(0);
    audio->sounds[SOUND_EXPLOSION_SMALL] = generate_explosion_sound(0);
    audio->sounds[SOUND_EXPLOSION_LARGE] = generate_explosion_sound(1);
    audio->sounds[SOUND_POWERUP] = generate_powerup_sound();
    audio->sounds[SOUND_GAME_OVER] = generate_explosion_sound(1); // Reuse for now
    
    audio->music = NULL;
    audio->initialized = 1;
    
    return 1;
}

void audio_shutdown(AudioSystem* audio) {
    if (!audio->initialized) return;
    
    for (int i = 0; i < NUM_SOUNDS; i++) {
        if (audio->sounds[i]) {
            // Free the actual data buffer
            free(audio->sounds[i]->abuf);
            Mix_FreeChunk(audio->sounds[i]);
            audio->sounds[i] = NULL;
        }
    }
    
    if (audio->music) {
        Mix_FreeMusic(audio->music);
        audio->music = NULL;
    }
    
    Mix_CloseAudio();
    audio->initialized = 0;
}

void audio_play_sound(AudioSystem* audio, SoundID id) {
    if (!audio->initialized || id < 0 || id >= NUM_SOUNDS) return;
    if (!audio->sounds[id]) return;
    
    // Play on first available channel
    Mix_PlayChannel(-1, audio->sounds[id], 0);
}

void audio_play_music(AudioSystem* audio) {
    // Procedural music not implemented yet
    (void)audio;
}

void audio_stop_music(AudioSystem* audio) {
    (void)audio;
    Mix_HaltMusic();
}
