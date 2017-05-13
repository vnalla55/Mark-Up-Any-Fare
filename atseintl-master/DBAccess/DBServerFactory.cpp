#include "DBAccess/DBServerFactory.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/DBServer.h"
#include "DBAccess/ORACLEDBServer.h"

#include <time.h>

using namespace std;

namespace tse
{

log4cxx::LoggerPtr&
DBServerFactory::getLogger()
{
  static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DBServerFactory"));
  return logger;
}

bool
DBServerFactory::isTimeForRefresh(void)
{
  bool rc = false;

  if (UNLIKELY(_refreshTimeout > 0))
  {
    if (time(nullptr) > _refreshTime)
    {
      if (_refreshTime > 0)
      {
        LOG4CXX_INFO(getLogger(), "DB Connection Refresh Time has been reached.  DB Connections "
                                  "will now be reconnected.");
        rc = true;
      }

      short randomVary = 1 + (int)(_refreshTimeoutPercentVary * (rand() / (RAND_MAX + 1.0)));

      short secs = (short)((_refreshTimeout * 60) * (1.0 + (randomVary / 100.0)));

      _refreshTime = time(nullptr) + secs;

      struct tm* timeinfo = localtime(&_refreshTime);
      char nextTime[30];
      sprintf(nextTime,
              "%04d-%02d-%02d %02d:%02d:%02d",
              timeinfo->tm_year + 1900,
              timeinfo->tm_mon + 1,
              timeinfo->tm_mday,
              timeinfo->tm_hour,
              timeinfo->tm_min,
              timeinfo->tm_sec);

      LOG4CXX_INFO(getLogger(), "Database Connection Next Refresh Time is " << nextTime << ".");
    }
  }

  return (rc);
}

DBServerFactory::DBServerFactory(tse::ConfigMan& config)
  : _config(config), _refreshTime(0), _refreshTimeout(0), _refreshTimeoutPercentVary(0)
{
  if (!config.getValue("refreshtimeout", _refreshTimeout))
  {
    _refreshTimeout = 0;
  }

  if ((_refreshTimeout > 0) && (_refreshTimeout < 5))
  {
    LOG4CXX_WARN(
        getLogger(),
        "Database Connection Refresh Timeout ["
            << _refreshTimeout
            << "] is configured below threshold of 5 minutes.  Setting value to 5 minutes.");
    _refreshTimeout = 5;
  }

  if (!config.getValue("refreshtimeoutpercentvary", _refreshTimeoutPercentVary))
  {
    _refreshTimeoutPercentVary = 0;
  }

  LOG4CXX_INFO(getLogger(),
               "Database Connection Refresh Timeout is set to " << _refreshTimeout << " minutes.");
  LOG4CXX_INFO(getLogger(),
               "Database Connection Refresh Timeout Variation is set to "
                   << _refreshTimeoutPercentVary << " percent.");
}
DBServerFactory::~DBServerFactory() {}

} // namespace tse
