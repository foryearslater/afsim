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

#include "SDL_AudioManager.hpp"

#include <iostream>

#include "UtPath.hpp"

sdl::AudioManager::AudioManager(sdl::Manager& aManager)
   : mManager(aManager)
{
   // Initialize SDL (required for audio support)
   if (!mManager.InitSDL())
   {
      std::cout << "ERROR: sdl::AudioManager::AudioManager() was unable to initialize SDL!" << std::endl;
      return;
   }

   UtPath path = UtPath::GetExePath();
   path.Up();
   const char* devDataPath = getenv("SOURCE_ROOT");
   if (nullptr != devDataPath)
   {
      path = std::string(devDataPath) + "/warlock/plugins/P6DOF_Controller/data";
   }
   else
   {
      path = path.GetSystemPath() + "/../resources/data";
   }

   // Load the WAV file(s)
   SDL_AudioSpec wavSpec;
   std::string   audioFile1 = "warble_alert.wav";
   std::string   audioFile2 = "lock-on.wav";
   std::string   slash      = "/";
   audioFile1               = path.GetSystemPath().c_str() + slash + audioFile1;
   audioFile2               = path.GetSystemPath().c_str() + slash + audioFile2;

   if (SDL_LoadWAV(audioFile1.c_str(), &wavSpec, &mWavBuffer1, &mWavLength1) == NULL)
   {
      std::cout << "ERROR (SDL):  Could not open sound file 1 ERR(" << SDL_GetError() << ")" << std::endl;
   }

   if (SDL_LoadWAV(audioFile2.c_str(), &wavSpec, &mWavBuffer2, &mWavLength2) == NULL)
   {
      std::cout << "ERROR (SDL):  Could not open sound file 2 ERR(" << SDL_GetError() << ")" << std::endl;
   }

   // Open audio device
   mAudioDeviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

   /* This was used for testing -- helpful for reference
   // Play audio
   int success = SDL_QueueAudio(mAudioDeviceId, mWavBuffer, mWavLength);
   SDL_PauseAudioDevice(mAudioDeviceId, 0);
   SDL_Delay(3000);
   SDL_PauseAudioDevice(mAudioDeviceId, 1); // paused
   SDL_Delay(1000);
   SDL_PauseAudioDevice(mAudioDeviceId, 0); // play
   SDL_Delay(1000);
   SDL_PauseAudioDevice(mAudioDeviceId, 1); // paused
   */
}

sdl::AudioManager::~AudioManager()
{
   SDL_CloseAudioDevice(mAudioDeviceId);

   SDL_FreeWAV(mWavBuffer1);
   SDL_FreeWAV(mWavBuffer2);

   mManager.QuitSDL();
}

bool sdl::AudioManager::InitAudio()
{
   return true;
}

void sdl::AudioManager::QuitAudio() {}

void sdl::AudioManager::PlaySound(size_t aSoundNum, double aCurrentTime_sec)
{
   if (aSoundNum == 1)
   {
      // If the other buffer was loaded, clear it
      if (mBufferLoaded2)
      {
         SDL_ClearQueuedAudio(mAudioDeviceId);
         mBufferLoaded2 = false;
      }

      // If new buffer or if time is longer than 1/2 second
      if (((aCurrentTime_sec - mLastTime_sec) > 0.5) || !mBufferLoaded1)
      {
         SDL_QueueAudio(mAudioDeviceId, mWavBuffer1, mWavLength1);
         mLastTime_sec  = aCurrentTime_sec;
         mBufferLoaded1 = true;
      }

      SDL_PauseAudioDevice(mAudioDeviceId, 0);
   }
   else if (aSoundNum == 2)
   {
      // If the other buffer was loaded, clear it
      if (mBufferLoaded1)
      {
         SDL_ClearQueuedAudio(mAudioDeviceId);
         mBufferLoaded1 = false;
      }

      // If new buffer or if time is longer than 1/2 second
      if (((aCurrentTime_sec - mLastTime_sec) > 0.5) || !mBufferLoaded2)
      {
         SDL_QueueAudio(mAudioDeviceId, mWavBuffer2, mWavLength2);
         mLastTime_sec  = aCurrentTime_sec;
         mBufferLoaded2 = true;
      }

      SDL_PauseAudioDevice(mAudioDeviceId, 0);
   }
}

void sdl::AudioManager::PauseSound()
{
   SDL_ClearQueuedAudio(mAudioDeviceId);
   SDL_PauseAudioDevice(mAudioDeviceId, 1); // paused
}
