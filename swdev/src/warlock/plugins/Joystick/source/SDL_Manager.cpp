// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SDL_Manager.hpp"

#include <iostream>

#include <SDL2/SDL.h>

sdl::Manager& sdl::Manager::Instance()
{
   static sdl::Manager sInstance;
   return sInstance;
}

sdl::Manager::Manager() {}

sdl::Manager::~Manager()
{
   // std::cout << "WARNING: SDL Manager detected lingering joysticks or audio devices during shutdown!" << std::endl;
   SDL_Quit();
}

bool sdl::Manager::InitSDL()
{
   if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
   {
      std::cout << "Could not initialize SDL Manager! (" << SDL_GetError() << ")" << std::endl;
      return false;
   }

   return true;
}

void sdl::Manager::QuitSDL()
{
   SDL_Quit();
}
