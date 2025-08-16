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

#ifndef SDL_MANAGER_HPP
#define SDL_MANAGER_HPP

#include <stddef.h>

// sdl::Manager is a simple class that wraps the Simple Direct Media Layer (SDL)
// initialization and shutdown functions. Joystick and audio support utilizing
// SDL should use the sdl::Manager to handle initialization/shutdown of SDL,
// rather than handling those functions directly.

namespace sdl
{
class Manager
{
public:
   static Manager& Instance();

   Manager(Manager const&) = delete;
   Manager(Manager&&)      = delete;
   Manager& operator=(Manager const&) = delete;
   Manager& operator=(Manager&&) = delete;

   bool InitSDL();
   void QuitSDL();

protected:
   Manager();
   ~Manager();
};
} // namespace sdl

#endif // MANAGER_HPP
