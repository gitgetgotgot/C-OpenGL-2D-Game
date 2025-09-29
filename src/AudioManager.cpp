#include "AudioManager.h"

AudioManager::AudioManager() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error initializing SDL: " + std::string(SDL_GetError()) << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Error initializing SDL_mixer: " + std::string(Mix_GetError()) << std::endl;
    }
}

AudioManager::~AudioManager() {
    for (auto sound : sounds) {
        Mix_FreeChunk(sound);
    }
    sounds.clear();

    for (auto track : musicTracks) {
        Mix_FreeMusic(track);
    }
    musicTracks.clear();

    Mix_CloseAudio();
    SDL_Quit();
}

void AudioManager::load_Sound(const std::string& filePath) {
    Mix_Chunk* sound = Mix_LoadWAV(filePath.c_str());
    if (!sound) {
        std::cerr << "Can't load sound: " + filePath + std::string(Mix_GetError()) << std::endl;
    }
    sounds.push_back(sound);
}

void AudioManager::play_Sound(int sound_id) {
    if (Mix_PlayChannel(-1, sounds[sound_id], 0) == -1) {
        std::cerr << "Can't play sound: " + std::string(Mix_GetError()) << std::endl;
    }
}

void AudioManager::load_Music(const std::string& filePath) {
    Mix_Music* music = Mix_LoadMUS(filePath.c_str());
    if (!music) {
        std::cerr << "Can't load music: " + filePath + std::string(Mix_GetError()) << std::endl;
    }
    musicTracks.push_back(music);
}

void AudioManager::play_Music(int music_id, bool loop) {
    int loops = loop ? -1 : 1; // -1 means looping
    if (Mix_PlayMusic(musicTracks[music_id], loops) == -1) {
        std::cerr << "Can't play music: " + std::string(Mix_GetError()) << std::endl;
    }
}

void AudioManager::stop_Music() {
    Mix_HaltMusic();
}

void AudioManager::set_Music_Volume(int volume) {
    Mix_VolumeMusic(volume);
}

int AudioManager::get_sounds_size() const {
    return static_cast<int>(sounds.size());
}

int AudioManager::get_music_size() const {
    return static_cast<int>(musicTracks.size());
}