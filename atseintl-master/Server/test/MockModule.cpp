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

#include "Server/LoadableModule.h"
#include "Server/test/MockModule.h"

using namespace tse;

extern "C" {

bool
mockFunction()
{
  return true;
}

bool
mockFunction2()
{
  return false;
}
}

static LoadableModuleRegister<Module, MockModule>
_("libMockModule.so");

MockModule::MockModule(const std::string& name, TseServer& srv)
  : Module(name, srv), _postInitialized(false), _preShutdown(false)
{
}

MockModule::~MockModule() {}

bool
MockModule::initialize(int argc, char** argv)
{
  for (int i = 0; i < argc; ++i)
    _args.push_back(argv[i]);
  return true;
}

void
MockModule::postInitialize()
{
  _postInitialized = true;
}

void
MockModule::preShutdown()
{
  _preShutdown = true;
}
