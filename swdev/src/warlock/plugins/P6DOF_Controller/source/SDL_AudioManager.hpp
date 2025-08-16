// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SDL_AUDIOMANAGER_HPP
#define SDL_AUDIOMANAGER_HPP

#include <SDL2/SDL.h>

#include "SDL_Manager.hpp"

// sdl::AudioManager is a simple means of audio support through the use of
// Simple Direct Media Layer (SDL). It provides a means to play (or pause)
// sounds by command. However, it lacks any sound mixing, 3D sound support,
// frequency control, or Doppler effects, so it is best for basic warning
// tones as opposed to environmental sounds such as air rush wind noise or
// engine sounds.

namespace sdl
{
class AudioManager
{
public:
   AudioManager(Manager& aManager);
   virtual ~AudioManager();

   bool InitAudio();
   void QuitAudio();

   void PlaySound(size_t aSoundNum, double aCurrentTime_sec);
   void PauseSound();

private:
   SDL_AudioDeviceID mAudioDeviceId;
   Manager&          mManager;

   Uint8* mWavBuffer1    = nullptr;
   Uint32 mWavLength1    = 0;
   bool   mBufferLoaded1 = false;

   Uint8* mWavBuffer2    = nullptr;
   Uint32 mWavLength2    = 0;
   bool   mBufferLoaded2 = false;

   double mLastTime_sec = 0.0;
};
} // namespace sdl

#endif // SDL_AUDIOMANAGER_HPP
