//----------------------------------------------------------------------------
//
//  File:        DynamicLibrary.C
//  Description: DynamicLibrary class
//  Created:     Jan 07, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: See .h
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Server/DynamicLibrary.h"

#include "Allocator/TrxMalloc.h"

#include <iostream>
#include <string>

using namespace std;

namespace tse
{
//----------------------------------------------------------------------------
// unload_int() - Unloads a library
//----------------------------------------------------------------------------
bool
DynamicLibrary::unload_int()
{
  const MallocContextDisabler context;

  if (!loaded())
  {
    return true;
  }

  if (dlclose(_handle) != 0)
  {
    const char* error = dlerror();

    _error.append(" Error unloading module");

    if (error != nullptr)
    {
      _error.append(": ");
      _error.append(error);
    }
    return false;
  }

  _handle = nullptr;

  return true;
}

//----------------------------------------------------------------------------
// load() - Loads a library
//----------------------------------------------------------------------------
bool
DynamicLibrary::load(const string& modName, const int loadflags)
{
  const MallocContextDisabler context;

  _error.clear();

  if (loaded())
  {
    // We're already loaded must unload first
    _error.append(" load failed.  Module already loaded");
    return false; // failure
  }

  if (modName.size() == 0)
  {
    _error.append(" modName is required and cannot be zero length");
    return false; // failure
  }

  _handle = dlopen(modName.c_str(), loadflags);
  if (_handle == nullptr)
  {
    const char* error = dlerror();
    _error.append(" Error loading module [");
    _error.append(modName);
    _error.append("]");

    if (error != nullptr)
    {
      _error.append(" - ");
      _error.append(error);
    }

    return false; // failure
  }

  return true; // success
}

//----------------------------------------------------------------------------
// symbol() - Loads a symbol
//----------------------------------------------------------------------------
void*
DynamicLibrary::symbol(const string& name)
{
  const MallocContextDisabler context;

  _error.clear();

  if (!loaded())
  {
    // We must load first
    _error.append(" No module is loaded");
    return nullptr; // failure
  }

  void* fnc = dlsym(_handle, name.c_str());

  const char* error = nullptr;
  if ((error = dlerror()) != nullptr)
  {
    _error.append(" Error resolving symbol [");
    _error.append(name);
    _error.append("] - ");
    _error.append(error);
  }

  return fnc;
}
}
