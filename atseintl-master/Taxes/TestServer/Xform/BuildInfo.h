// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <string>

namespace tax
{

class BuildInfo
{
public:
  static const std::string& date()
  {
    return _date;
  }
  static const std::string& user()
  {
    return _user;
  }
  static const std::string& host()
  {
    return _host;
  }
  static const std::string& commit()
  {
    return _commit;
  }
  static const std::string& startTime()
  {
    return _startTime;
  }

private:
  static std::string _date;
  static std::string _user;
  static std::string _host;
  static std::string _commit;
  static std::string _startTime;
};

} // namespace tax
