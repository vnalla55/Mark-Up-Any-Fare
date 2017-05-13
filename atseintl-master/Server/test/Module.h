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

#ifndef TEST_MODULE_H
#define TEST_MODULE_H

namespace tse
{

class TseServer;

class Module
{
public:
  Module(const std::string& name, TseServer& srv) : _name(name), _srv(srv) {}
  virtual ~Module() {}

  virtual bool initialize(int argc, char** argv) = 0;
  virtual void postInitialize() = 0;
  virtual void preShutdown() = 0;

  const std::string& getName() const { return _name; }
  const TseServer& getServer() const { return _srv; }

protected:
  const std::string _name;
  TseServer& _srv;
};
}

#endif /* TEST_MODULE_H */
