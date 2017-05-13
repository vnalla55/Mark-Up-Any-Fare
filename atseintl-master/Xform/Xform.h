//----------------------------------------------------------------------------
//
//  File:               Xform.h
//  Description:        Generic Transform lass to process
//                      requests from a client
//  Created:            03/08/2004
//  Authors:            Mark Kasprowicz
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include <string>

namespace tse
{

class Trx;
class DataHandle;
class ErrorResponseException;
class ConfigMan;

class Xform
{
protected:
  std::string _name;
  ConfigMan& _config;

public:
  Xform(const std::string& name, ConfigMan& config) : _name(name), _config(config) {}

  virtual ~Xform() = default;

  const std::string& name() { return _name; }
  const ConfigMan& config() { return _config; }

  virtual bool convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled) = 0;
  virtual bool convert(Trx& trx, std::string& response) = 0;
  virtual bool convert(ErrorResponseException& ere, Trx& trx, std::string& response) = 0;
  virtual bool convert(ErrorResponseException& ere, std::string& response);

  virtual bool throttle(std::string& request, std::string& response);

  virtual bool initialize();
  virtual bool initialize(int argc, char** argv) = 0;

  virtual void postInitialize() {}
  virtual void preShutdown() {}
};
} /* end namespace tse */
