#ifndef AUDIO_H
#define AUDIO_H

#include <SDL_mixer.h>

// Sound effect IDs
typedef enum {
    SOUND_LASER_PLAYER,
    SOUND_LASER_ENEMY,
    SOUND_EXPLOSION_SMALL,
    SOUND_EXPLOSION_LARGE,
    SOUND_POWERUP,
    SOUND_GAME_OVER,
    NUM_SOUNDS
} SoundID;

typedef struct {
    Mix_Chunk* sounds[NUM_SOUNDS];
    Mix_Music* music;
    int initialized;
} AudioSystem;

int audio_init(AudioSystem* audio);
void audio_shutdown(AudioSystem* audio);
void audio_play_sound(AudioSystem* audio, SoundID id);
void audio_play_music(AudioSystem* audio);
void audio_stop_music(AudioSystem* audio);

// Generate procedural sound effects
Mix_Chunk* generate_laser_sound(int player); // 1 = player, 0 = enemy
Mix_Chunk* generate_explosion_sound(int large); // 1 = large, 0 = small
Mix_Chunk* generate_powerup_sound(void);

#endif
