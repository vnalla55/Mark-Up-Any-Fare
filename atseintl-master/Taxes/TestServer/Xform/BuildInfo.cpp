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
#include <boost/date_time/posix_time/posix_time.hpp>
#include "TestServer/Xform/BuildInfo.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#ifndef BUILD_USER
#define BUILD_USER unknown
#endif

#ifndef BUILD_HOST
#define BUILD_HOST unknown
#endif

#ifndef BUILD_COMMIT
#define BUILD_COMMIT unknown
#endif

namespace tax
{

namespace
{

std::string currentTime()
{
  boost::posix_time::ptime now_p = boost::posix_time::second_clock::local_time();
  struct tm nowS_p = boost::posix_time::to_tm(now_p);
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &nowS_p);
  return buffer;
}

} // namespace

std::string BuildInfo::_date = __DATE__ " " __TIME__;
std::string BuildInfo::_user = STRINGIFY(BUILD_USER);
std::string BuildInfo::_host = STRINGIFY(BUILD_HOST);
std::string BuildInfo::_commit = STRINGIFY(BUILD_COMMIT);
std::string BuildInfo::_startTime = currentTime();

} // namespace tax
