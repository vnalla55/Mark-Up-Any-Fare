#pragma once
// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include <string>

namespace tse
{

/*
 * This class imitates indent and should be used when printing diagnostic information.
 * Indent is increased automatically when instance of this class is copy-constructed,
 * so one can pass such instance to method taking parameters by value (not by reference)
 * and indent will automatically increment. This keeps user of class away from
 * manually incrementing/decrementing indent.
 */

class DiagnosticAutoIndent
{

public:

  DiagnosticAutoIndent() : _baseIndent("  "), _currentIndent("") {}

  DiagnosticAutoIndent(const DiagnosticAutoIndent& diagAutoIndent)
  {
    this->_baseIndent = diagAutoIndent._baseIndent;
    this->_currentIndent = diagAutoIndent._currentIndent + diagAutoIndent._baseIndent;
  }

  DiagnosticAutoIndent& operator++()
  {
    _currentIndent += _baseIndent;
    return *this;
  }

  DiagnosticAutoIndent& operator--()
  {
    if ( _currentIndent.length() >= _baseIndent.length() )
      _currentIndent.erase(_currentIndent.length()-_baseIndent.length(), _baseIndent.length());
    else
      _currentIndent = "";
    return *this;
  }

  operator std::string() const { return _currentIndent; }

  std::string toString() const { return _currentIndent; }

  std::string operator+(std::string input) const { return _currentIndent + input; }

  std::string operator+(const char * input) const { return _currentIndent + input; }

private:
  std::string _baseIndent;
  std::string _currentIndent;
};

}

