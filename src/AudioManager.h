#pragma once
#include <vector>
#include <string>
#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    void load_Sound(const std::string& filePath);
    void play_Sound(int sound_id);
    void load_Music(const std::string& filePath);
    void play_Music(int music_id, bool loop);
    void stop_Music();
    void set_Music_Volume(int volume);
    int get_sounds_size() const;
    int get_music_size() const;

private:
    std::vector<Mix_Chunk*> sounds;
    std::vector<Mix_Music*> musicTracks;
};