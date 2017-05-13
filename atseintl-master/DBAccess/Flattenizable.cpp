//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DBAccess/Flattenizable.h"

namespace tse
{
bool
Flattenizable::Archive::pushClassRef(const char* prettyFunc)
{
  std::string prettyFunction(prettyFunc);

  if (prettyFunction.find("static") != std::string::npos)
    return false;

  size_t pos = prettyFunction.find_first_of("(");
  if (pos != std::string::npos)
    prettyFunction.erase(pos);

  pos = prettyFunction.find_last_of(" ");
  if (pos != std::string::npos)
    prettyFunction.erase(0, pos + 1);

  pos = prettyFunction.find_last_of(":");
  if (pos != std::string::npos)
    prettyFunction.erase(pos - 1);

  if (prettyFunction.substr(0, 5) == "tse::")
    prettyFunction.erase(0, 5);

  pushFieldRef("[" + prettyFunction + "]");
  return true;
}

void
Flattenizable::Archive::handleIndent()
{
  if (_newline)
  {
    (*_os) << std::endl;
    if (!isPrettyPrint())
    {
      for (size_t i = 0; i < _indentLevel; ++i)
      {
        (*_os) << ' ';
      }
    }
    _fieldCount = 0;
    _newline = false;
  }

  if (isPrettyPrint())
  {
    (*_os) << "\n" << fieldReference() << "=";
  }

  ++_fieldCount;
}

void
Flattenizable::Archive::printToStream(const char& t, size_t n)
{
  if (usingOStream())
  {
    handleIndent();
    char* p(const_cast<char*>(&t));
    for (size_t i = 0; i < n; ++i, ++p)
    {
      if (*p == '\0')
      {
        (*_os) << ' ';
      }
      else
      {
        (*_os) << *p;
      }
    }
    (*_os) << "|";
  }
}

std::string
Flattenizable::Archive::fieldReference()
{
  std::string retval;
  for (FIELDNAMESTACK::const_iterator it = _stack.begin(); it != _stack.end(); ++it)
  {
    const std::string& ref(*it);
    if ((!retval.empty()) && (ref[0] != '['))
    {
      retval.append(".");
    }
    retval.append(ref);
  }
  return retval;
}
}
