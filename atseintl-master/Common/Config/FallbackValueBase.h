//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#ifndef CONFIG_HIERARCHY_REFACTOR

#include <string>

namespace tse
{
class Logger;
class ConfigMan;

namespace fallback
{

class FallbackValueBase
{
public:
  FallbackValueBase();
  virtual ~FallbackValueBase();

  virtual void set(const tse::ConfigMan* config) = 0;

protected:
  static const std::string CONFIG_SECTION;
  static Logger _logger;

  void loggerMessage(const std::string& option, bool value) const;
};

void
configure(const tse::ConfigMan& config);

} // fallback

} // tse

#endif
