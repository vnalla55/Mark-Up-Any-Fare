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

#ifndef TEST_MOCK_MODULE_H
#define TEST_MOCK_MODULE_H

#include <vector>
#include <string>

#include "Server/test/Module.h"

namespace tse
{

class MockModule : public Module
{
public:
  MockModule(const std::string& name, TseServer& srv);
  virtual ~MockModule();

  virtual bool initialize(int argc, char** argv);
  virtual void postInitialize();
  virtual void preShutdown();

  const std::vector<std::string>& args() const { return _args; }
  bool isPostInitialized() const { return _postInitialized; }
  bool isPreShutdown() const { return _preShutdown; }

private:
  std::vector<std::string> _args;
  bool _postInitialized;
  bool _preShutdown;
};
}

#endif /* TEST_MOCK_MODULE_H */
