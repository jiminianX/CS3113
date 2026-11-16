/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "Scene.h"

int Scene::sLives = Scene::STARTING_LIVES;

Scene::Scene(Vector2 origin, const char *bgHexCode, const SoundBank *sounds)
    : mGameState{}, mOrigin{ origin }, mBGColourHexCode{ bgHexCode },
      mSounds{ sounds }
{}

void Scene::startMusic(const char *musicPath)
{
    if (musicPath == nullptr) return;

    mGameState.bgm = LoadMusicStream(musicPath);
    if (mGameState.bgm.frameCount == 0)
    {
        LOG("WARNING: could not load music " << musicPath);
        return;
    }

    mGameState.bgm.looping = true;
    mGameState.hasMusic    = true;
    SetMusicVolume(mGameState.bgm, MUSIC_VOLUME);
    PlayMusicStream(mGameState.bgm);
}

void Scene::updateMusic()
{
    if (mGameState.hasMusic) UpdateMusicStream(mGameState.bgm);
}

void Scene::stopMusic()
{
    if (!mGameState.hasMusic) return;

    StopMusicStream(mGameState.bgm);
    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm      = {};
    mGameState.hasMusic = false;
}

void Scene::playSound(const Sound &sound) const
{
    if (sound.frameCount > 0) PlaySound(sound);
}
