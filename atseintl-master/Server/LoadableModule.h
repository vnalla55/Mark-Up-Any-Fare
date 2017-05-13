//----------------------------------------------------------------------------
//
//  File:        LoadableModule.h
//  Created:     Dec 11, 2003
//  Authors:     Mark Kasprowicz, Gustaw Smolarczyk
//
//  Description: This class provides a way to load optionally separate
//               modules, create instance and initialize it with TseServer.
//               These modules can be built-in or placed in separate
//               dynamic libraries.
//
//               ** NOTE **
//
//               In order for a module to be sucessfully loaded it must
//               implement the following global object to register it:
//
//               static LoadableModuleRegister<BaseType, DerivedType>
//                 _("libDerivedType.so");
//
//               The name, scope and linkage of this global object
//               is not significant, but it should be static.
//
//               ** END NOTE **
//
//  Copyright Sabre 2003, 2013
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

#include "Common/Logger.h"
#include "Server/DynamicLibrary.h"

#include <string>

namespace tse
{

class TseServer;

template <class BaseT>
class LoadableModuleRegisterBase
{
public:
  typedef BaseT* CreateT(const std::string&, TseServer&);

  LoadableModuleRegisterBase* const next;
  const std::string modName;
  CreateT* const create;

  LoadableModuleRegisterBase(const std::string& modName, CreateT* create);
};

template <class BaseT, class ModuleT>
class LoadableModuleRegister : public LoadableModuleRegisterBase<BaseT>
{
public:
  LoadableModuleRegister(const std::string& modName)
    : LoadableModuleRegisterBase<BaseT>(modName, &createModule)
  {
  }

private:
  static BaseT* createModule(const std::string& name, TseServer& srv)
  {
    return new ModuleT(name, srv);
  }
};

template <class BaseT>
class LoadableModule
{
  friend class LoadableModuleRegisterBase<BaseT>;

public:
  typedef LoadableModuleRegisterBase<BaseT> Module;
  typedef typename Module::CreateT CreateT;

private:
  static Module* _moduleList;
  static Logger _logger;

  std::string _name;
  std::string _modName;
  Module* _module = nullptr;
  DynamicLibrary _library;
  BaseT* _instance = nullptr;

  static Module* findModule(const std::string& modName);

public:
  LoadableModule() = default;

  LoadableModule(const LoadableModule& rhs)
    : _name(rhs._name), _modName(rhs._modName), _module(0), _instance(rhs._instance)
  {
  }

  LoadableModule& operator=(const LoadableModule& rhs)
  {
    if (&rhs == this)
      return *this;

    unload();

    _name = rhs._name;
    _modName = rhs._modName;
    _module = rhs._module;
    _instance = rhs._instance;

    return *this;
  }

  ~LoadableModule() { unload(); }

  /**
   * Return the instance pointer of the modules loaded class
   *
   * @return Pointer if successful, 0 otherwise
   */
  BaseT* instance() { return _instance; }

  /**
   * Load a dynamic module, get an object instance, and initialize it
   *
   * @param modName    Library name
   * @param srv        TseServer reference (used for call to initialize)
   * @param name       Module Name (used in call to initialize)
   * @param argc       command line argument count (used in call to initialize)
   * @param argv       command line arguments (used in call to initialize)
   *
   * @return true if successful, false otherwise
   */
  bool
  load(const std::string& modName, TseServer& srv, const std::string& name, int argc, char* argv[]);

  /**
   * Post Load a dynamic module, get an object instance, and initialize it
   *
   * @param modName    Library name, may include absolute path information
   * @param srv        TseServer reference (used for call to initialize)
   * @param name       Module Name (used in call to initialize)
   *
   * @return true if successful, false otherwise
   */
  bool postLoad(const std::string& modName, TseServer& srv, const std::string& name);

  void preUnload();

  void unload();
};

template <class BaseT>
LoadableModuleRegisterBase<BaseT>::LoadableModuleRegisterBase(const std::string& modName,
                                                              CreateT* create)
  : next(LoadableModule<BaseT>::_moduleList), modName(modName), create(create)
{
  LoadableModule<BaseT>::_moduleList = this;
}

template <class BaseT>
typename LoadableModule<BaseT>::Module* LoadableModule<BaseT>::_moduleList = nullptr;

template <class BaseT>
Logger
LoadableModule<BaseT>::_logger("atseintl.Server.LoadableModule");

} // tse

