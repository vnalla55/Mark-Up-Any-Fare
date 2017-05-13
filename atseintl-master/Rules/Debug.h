#ifndef DEBUG_H
#define DEBUG_H

#include "Common/TseEnums.h"

#include <iostream>

class FunctionPrinter
{
  std::string _message;

public:
  FunctionPrinter(std::string message) : _message(message)
  {
    std::cout << "Entering " << _message << std::endl;
  };
  ~FunctionPrinter()
  {
    std::cout << "Exiting " << _message << std::endl;
  };
};

class PredicateOutputPrinter
{
  tse::Record3ReturnTypes& _retval;
  std::string _message;

public:
  PredicateOutputPrinter(tse::Record3ReturnTypes& retval, std::string message)
    : _retval(retval), _message(message) {};

  ~PredicateOutputPrinter()
  {
#ifdef DEBUG
    std::cout << _message << " returns: ";
    switch (_retval)
    {
    case tse::NOTPROCESSED:
      std::cout << "Not processed";
      break;
    case tse::FAIL:
      std::cout << "Fail";
      break;
    case tse::PASS:
      std::cout << "Pass";
      break;
    case tse::SKIP:
      std::cout << "Skip";
      break;
    case tse::STOP:
      std::cout << "Stop";
      break;
    }
    std::cout << std::endl;
#endif
  }
};

#endif // DEBUG_H
