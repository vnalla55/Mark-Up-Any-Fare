// Manager.h
//
#pragma once

#include "Common/Config/ConfigMan.h"

#include <string>

class Manager
{
public:
  Manager(const std::string& name, tse::ConfigMan& config) : _name(name), _config(config) {}
  virtual ~Manager() = default;

  Manager(const Manager&) = delete;
  Manager& operator=(const Manager&) = delete;

  // Data interface
  //
  std::string& name() { return _name; }
  const std::string& name() const { return _name; }

  // Abstract interface
  //
  virtual bool initialize(int argc = 0, char* argv[] = nullptr) = 0;
  virtual void postInitialize() {}
  virtual void preShutdown() {}

  tse::ConfigMan& config() { return _config; }
  const tse::ConfigMan& config() const { return _config; }

protected:
  std::string _name;
  tse::ConfigMan& _config;
};

