//----------------------------------------------------------------------------
//
//  File:        DynamicLibary.h
//  Description: DynamicLibary class
//  Created:     Dec 11, 2003
//  Authors:     Mark Kasprowicz
//
//  Description: This class loads a dynamic shared library and resolves
//               create and destroy functions if provided
//
//  Return types:
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
#pragma once

#include <string>

#include <dlfcn.h>

namespace tse
{

class DynamicLibrary
{
private:
  DynamicLibrary(const DynamicLibrary&) = delete;
  DynamicLibrary& operator=(const DynamicLibrary&) = delete;

  void* _handle = nullptr;
  std::string _error;

  /**
   * Unloads a loaded dynamic library
   *
   * @return true if successful, false otherwise
   */
  bool unload_int();

public:
  DynamicLibrary() = default;

  ~DynamicLibrary() = default;
    // Don't call unload here; class user must call it manually.
    // unload();

  /**
   * Check loaded status
   *
   * @return true if module loaded, false otherwise
   */
  bool loaded() { return (_handle != nullptr); }

  /**
   * Check error status
   *
   * @return true if error occurred, false otherwise
   */
  bool error() { return (_error.size() > 0); }

  /**
   * Returns the current error message.
   * Used after either load or unload return false
   *
   * @return const std::string & - Error message
   */
  const std::string& errorMsg() { return _error; }

  /**
   * Loads a dynamic shared library
   *
   * @param modName    Shared library name (may include path information)
   * @param loadflags  RTLD_LAZY   - resolve external dependencies when needed
   *                   RTLD_NOW    - resolve all external dependencies immediately
   *                   RTLD_GLOBAL - expose loaded symbols globally
   *
   *                   Either RTLD_LAZY or RTLD_NOW is required, and either may be
   *                   combined with RTLD_GLOBAL
   *
   * @return true if successful, false otherwise
   */
  bool load(const std::string& modName, const int loadflags = RTLD_GLOBAL | RTLD_NOW);

  /**
   * Unload a loaded module
   *
   * @return true if successful, false otherwise
   */
  bool unload()
  {
    _error.clear();

    return unload_int();
  }

  /**
   * Loads the given symbol in a shared library
   *
   * @param name   Symbol name to load
   *
   * @return pointer to the symbol.
   *
   *         ** NOTE **
   *         0 is a valid symbol, the only way to check
   *         success of this function is to check error()
   */
  void* symbol(const std::string& name);

}; // End class DynamicLibrary

} // end namespace tse

