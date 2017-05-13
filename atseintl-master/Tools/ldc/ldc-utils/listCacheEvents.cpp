#include "Tools/ldc/ldc-utils/CacheEvent.h"
#include "Tools/ldc/ldc-utils/CommandLine.h"
#include "Tools/ldc/ldc-utils/mytimes.h"
#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static std::string cacheEventTable;
static std::string databaseHost;
static std::string databaseUser = "hybfunc";
static std::string databasePassword = "";
static std::string databaseSchema = "ATSEHYB1";
static std::string databasePort = "3306";
static std::string verboseFlag;
static std::string dbtable = "all";
static std::string minAgeSeconds;
static std::string maxAgeSeconds;

//| FARECACHENOTIFY            |
//| HISTORICALCACHENOTIFY      |
//| INTLCACHENOTIFY            |
//| LBCACHENOTIFY              |
//| LBTCONTRACTNOTIFY          |
//| LIMITNOTVIAROUTING         |
//| MERCHANDISINGCACHENOTIFY   |
//| ROUTINGCACHENOTIFY         |
//| RULECACHENOTIFY            |
//| SCHEDULECACHENOTIFY        |
//| SUPPORTCACHENOTIFY         |
//| VISITANOTHERCNTRY          |

struct ArgProc argProcInfo[] = {
  { true, 't', cacheEventTable, "cache event table (RULECACHENOTIFY)" },
  { true, 'h', databaseHost, "database host machine name" },
  { false, 'P', databasePort, "database port number (default = 3306)" },
  { false, 'p', databasePassword, "database password (default blank)" },
  { false, 's', databaseSchema, "database schema (default = ATSEHYB1)" },
  { false, 'u', databaseUser, "database userid (default = hybfunc)" },
  { false, 'e', dbtable, "entity column with '=' (default = all)" },
  { false, 'l', minAgeSeconds, "minimum age of cache events (default = none)" },
  { false, 'x', maxAgeSeconds, "maximum age of cache events (default = none)" },
  { false, 'v', verboseFlag, "verbose" }
};
size_t argProcInfoLen = sizeof argProcInfo / sizeof(struct ArgProc);

void
parseXML();

int
main(int argc, char** argv)
{
  std::string filename;

  CommandLine cmd(argc, argv, argProcInfo, argProcInfoLen);
  bool verbose = (verboseFlag.length() > 0);

  // ===========================================================
  //  print header
  // ===========================================================

  fprintf(stdout, "ce-entitytype,ce-date,ce-time,ce-key\n");

  // ===========================================================
  //  read cache event table
  // ===========================================================

  if (verbose)
    fprintf(stderr, "loading mysql table - %s\n", cacheEventTable.c_str());
  CacheEvent ce(
      databaseHost, databaseUser, databasePassword, databaseSchema, databasePort, cacheEventTable);
  if (ce.load())
  {
    std::set<CacheEventItem, std::less<CacheEventItem> >::iterator iter;
    for (iter = ce.collection.begin(); iter != ce.collection.end(); ++iter)
    {
      if ((dbtable == "all") || (dbtable == iter->table))
      {
        time_t currentTime = time(nullptr);

        int eventAgeSeconds = (int)(currentTime - iter->localtime);
        if ((minAgeSeconds.length() == 0) || (eventAgeSeconds >= atoi(minAgeSeconds.c_str())))
        {
          if ((maxAgeSeconds.length() == 0) || (eventAgeSeconds <= atoi(maxAgeSeconds.c_str())))
          {
            struct tm tm;
            localtime_r(&iter->localtime, &tm);
            fprintf(stdout,
                    "%s,%04d-%02d-%02d,%02d:%02d:%02d,%s\n",
                    iter->table.c_str(),
                    tm.tm_year + 1900,
                    tm.tm_mon + 1,
                    tm.tm_mday,
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec,
                    iter->origKey.c_str());
          }
        }
      }
    }
  }
  return 0;
}
