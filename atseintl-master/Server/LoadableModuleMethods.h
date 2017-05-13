// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Server/LoadableModule.h"

#include <iostream>
#include <stdexcept>

namespace tse
{

template <class BaseT>
typename LoadableModule<BaseT>::Module*
LoadableModule<BaseT>::findModule(const std::string& modName)
{
  for (Module* module = _moduleList; module != nullptr; module = module->next)
    if (module->modName == modName)
      return module;
  return nullptr;
}

template <class BaseT>
bool
LoadableModule<BaseT>::load(
    const std::string& modName, TseServer& srv, const std::string& name, int argc, char* argv[])
{
  _name = name;
  _modName = modName;

  if (_module)
  {
    LOG4CXX_FATAL(_logger, "LoadableModule already loaded, must unload first");
    std::cerr << "LoadableModule already loaded, must unload first" << std::endl;
    return false;
  }

  _module = findModule(modName);
  if (!_module)
  {
    if (!_library.load(modName, RTLD_GLOBAL | RTLD_NOW))
    {
      LOG4CXX_FATAL(_logger, "LoadableModule load failed. " << _library.errorMsg());
      std::cerr << "LoadableModule load failed. " << _library.errorMsg() << std::endl;
      return false;
    }

    _module = findModule(modName);
    if (!_module)
    {
      LOG4CXX_FATAL(_logger, "LoadableModule: Module has not been registered: " << _modName);
      std::cerr << "LoadableModule: Module has not been registered: " << _modName << std::endl;
      return false;
    }
  }
  CreateT* create = _module->create;

  _instance = (*create)(name, srv);
  if (!_instance)
  {
    LOG4CXX_FATAL(_logger,
                  "LoadableModule - Error creating instance variable for module "
                  "["
                      << _modName << "]");
    std::cerr << "LoadableModule - Error creating instance variable for module "
                 "[" << _modName << "]" << std::endl;
    unload();
    return false;
  }

  if (!_instance->initialize(argc, argv))
  {
    LOG4CXX_FATAL(_logger,
                  "LoadableModule() - Unable to initialize module "
                  "["
                      << _modName << "]");
    std::cerr << "LoadableModule() - Unable to initialize module "
                 "[" << _modName << "]" << std::endl;
    unload();
    return false;
  }

  return true;
}

template <class BaseT>
bool
LoadableModule<BaseT>::postLoad(const std::string& modName, TseServer& srv, const std::string& name)
{
  if (!_module)
  {
    LOG4CXX_FATAL(_logger, "LoadableModule not loaded, must load first");
    std::cerr << "LoadableModule not loaded, must load first" << std::endl;
    return false;
  }

  _instance->postInitialize();
  return true;
}

template <class BaseT>
void
LoadableModule<BaseT>::preUnload()
{
  if (!_module)
    return;

  if (_instance)
    _instance->preShutdown();
}

template <class BaseT>
void
LoadableModule<BaseT>::unload()
{
  if (!_module)
    return;

  if (_instance)
    delete _instance;

  // Don't unload the library. It will be dlclosed automatically on exit().

  _instance = nullptr;
  _module = nullptr;
}
} // tse
