#include "Tools/ldc/ldc-utils/CacheEvent.h"
#include "Tools/ldc/ldc-utils/CommandLine.h"
#include "Tools/ldc/ldc-utils/FileAccess.h"
#include "Tools/ldc/ldc-utils/LDCUtil.h"
#include "Tools/ldc/ldc-utils/mytimes.h"
#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <cstring>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static std::string directory;
static std::string cacheEventTable;
static std::string cacheMappingXMLFile = "cacheNotify.xml";
static std::string minAgeSeconds = "900";
static std::string maxAgeSeconds;
static std::string ageTolerance = "-1";
static std::string databaseHost;
static std::string databaseUser = "hybfunc";
static std::string databasePassword = "";
static std::string databaseSchema = "ATSEHYB1";
static std::string databasePort = "3306";
static std::string bdbFile = "all";
static std::string skipHeaderFlag;
static std::string verboseFlag;
static std::string showMissingFiles;

static std::multimap<std::string, std::string, std::less<std::string> > eventToCacheMap;
static std::multimap<std::string, std::string, std::less<std::string> >::iterator
eventToCacheMapIter;

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
  { true, 'd', directory, "bdb directory" },
  { true, 't', cacheEventTable, "cache event table (RULECACHENOTIFY)" },
  { true, 'h', databaseHost, "host database machine name" },
  { false, 'f', bdbFile, "bdb file (default = all)" },
  { false, 'x', cacheMappingXMLFile, "cache mapping XML file (default = cacheNotify.xml)" },
  { false, 'P', databasePort, "database port number (default = 3306)" },
  { false, 'p', databasePassword, "database password (default blank)" },
  { false, 's', databaseSchema, "database schema (default = ATSEHYB1)" },
  { false, 'l', minAgeSeconds, "minimum age in seconds of cache events (default = 900)" },
  { false, 'x', maxAgeSeconds, "maximum age in seconds of cache events (default = none)" },
  { false,
    'L',
    ageTolerance,
    "age tolerance 'cache - cache event' in seconds to print (default = -1)" },
  { false, 'u', databaseUser, "database userid (default = hybfunc)" },
  { false, 'v', verboseFlag, "verbose" },
  { false, 'y', showMissingFiles, "show missing cache files" },
  { false, 'z', skipHeaderFlag, "skip header" }
};
size_t argProcInfoLen = sizeof argProcInfo / sizeof(struct ArgProc);

void
parseXML(bool showMissingFiles);

int
main(int argc, char** argv)
{
  std::string dbtable;
  std::string filename;

  CommandLine cmd(argc, argv, argProcInfo, argProcInfoLen);
  bool verbose = (verboseFlag.length() > 0);
  bool skipHeader = (skipHeaderFlag.length() > 0);
  bool showMissingFilesOpt = (showMissingFiles.length() > 0);

  parseXML(showMissingFilesOpt);
  if (showMissingFilesOpt)
    return 0;

  if (!skipHeader)
  {
    fprintf(stdout, "age-diff-sec,bdb-file,bdb-date,bdb-time,bdb-size,bdb-compr,bdb-type,bdb-key,"
                    "ce-entitytype,ce-date,ce-time,ce-key\n");
  }

  if (verbose)
    fprintf(stderr, "Berkeley db directory - %s\n", directory.c_str());
  LDCUtil ldc(directory, verbose);
  if (ldc.isValid())
  {
    // ===========================================================
    //  read cache event table
    // ===========================================================

    if (verbose)
      fprintf(stderr, "loading mysql table - %s\n", cacheEventTable.c_str());
    CacheEvent ce(databaseHost,
                  databaseUser,
                  databasePassword,
                  databaseSchema,
                  databasePort,
                  cacheEventTable);
    if (ce.load())
    {
      // ===========================================================
      //  select berkeley db files from input xml file
      // ===========================================================

      std::string lastFilename;
      static std::multimap<std::string, std::string, std::less<std::string> >::iterator
      mainFileIter;

      for (mainFileIter = eventToCacheMap.begin(); mainFileIter != eventToCacheMap.end();
           ++mainFileIter)
      {
        if (bdbFile == "all")
        {
          filename = mainFileIter->first;
          if (lastFilename == filename)
          {
            continue;
          }
          lastFilename = filename;
        }
        else
        {
          filename = mainFileIter->first;
          if ((filename != bdbFile) || (lastFilename == filename))
          {
            continue;
          }
          lastFilename = filename;
        }

        size_t entityCount = eventToCacheMap.count(filename.c_str());
        if (entityCount == 0)
        {
          fprintf(stderr,
                  "cannot find any matching cache event records for cache [%s]\n",
                  filename.c_str());
          return 1;
        }
        std::multimap<std::string, std::string, std::less<std::string> >::iterator firstTable =
            eventToCacheMap.find(filename.c_str());

        struct stat statBuf;
        std::string fullFilename = directory + "//" + filename;
        if (stat(fullFilename.c_str(), &statBuf) != 0)
        {
          continue;
        }

        if (verbose)
          fprintf(stdout, "\nloading Berkeley db table - %s\n", filename.c_str());
        size_t dbmatch = 0;
        {
          size_t index;
          eventToCacheMapIter = firstTable;
          for (index = 0; index < entityCount; ++index)
          {
            dbtable = eventToCacheMapIter->second;

            if (ce.entities.find(dbtable) != ce.entities.end())
            {
              ++dbmatch;
              if (verbose)
                fprintf(stdout, "\twill process cache event entity type [%s]\n", dbtable.c_str());
            }
            ++eventToCacheMapIter;
          }
        }

        // ===========================================================
        //  skip berkeley db file if no records match entity type
        //  in the mysql cache event table
        // ===========================================================

        if (dbmatch == 0)
        {
          if (verbose)
            fprintf(stdout, "\tno matching  cache event entity types found\n\n");
          continue;
        }
        if (verbose)
          fprintf(stdout, "\n");

        // ===========================================================
        //  process berkeley db file
        // ===========================================================

        if (ldc.openDB(filename))
        {
          std::vector<LDCUtil::RecInfo> list;
          ldc.getKeyList(list);
          std::set<CacheEventItem, std::less<CacheEventItem> >::iterator iter;

          for (size_t idx = 0; idx < list.size(); ++idx)
          {
            CacheEventItem item;

            size_t index;
            eventToCacheMapIter = firstTable;
            for (index = 0; index < entityCount; ++index)
            {
              dbtable = eventToCacheMapIter->second;
              item.table = dbtable;

              if (list[idx].keyPtr == NULL)
                continue;

              item.key = list[idx].keyPtr;

              iter = ce.collection.find(item);

              if (iter != ce.collection.end())
              {
                time_t currentTime = time(NULL);

                int eventAgeSeconds = (int)(currentTime - iter->localtime);
                if ((minAgeSeconds.length() == 0) ||
                    (eventAgeSeconds >= atoi(minAgeSeconds.c_str())))
                {
                  if ((maxAgeSeconds.length() == 0) ||
                      (eventAgeSeconds <= atoi(maxAgeSeconds.c_str())))
                  {
                    // deltaTime will be negative if cache is out of date
                    int deltaTime = (int)(list[idx].header->timestamp - iter->localtime);

                    if (atoi(ageTolerance.c_str()) > deltaTime) // difference db vs cache event
                    {
                      fprintf(stdout, "%d,", deltaTime);
                      fprintf(stdout, "%s,", filename.c_str());
                      list[idx].print();
                      struct tm tm;
                      localtime_r(&iter->localtime, &tm);
                      fprintf(stdout,
                              ",%s,%04d-%02d-%02d,%02d:%02d:%02d,%s\n",
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
              ++eventToCacheMapIter;
            }
          }

          if (false && verbose)
          {
            for (iter = ce.collection.begin(); iter != ce.collection.end(); ++iter)
            {
              size_t index;
              eventToCacheMapIter = firstTable;
              for (index = 0; index < entityCount; ++index)
              {
                dbtable = eventToCacheMapIter->second;

                // fprintf( stderr, "searching for [%s]\n", dbtable.c_str() );

                if (iter->table == dbtable)
                {
                  struct tm tm;
                  localtime_r(&iter->localtime, &tm);
                  fprintf(stdout,
                          "%04d-%02d-%02d,%02d:%02d:%02d,%s,%s\n",
                          tm.tm_year + 1900,
                          tm.tm_mon + 1,
                          tm.tm_mday,
                          tm.tm_hour,
                          tm.tm_min,
                          tm.tm_sec,
                          iter->table.c_str(),
                          iter->key.c_str());
                }
                ++eventToCacheMapIter;
              }
            }
          }
        } // if ( ldc.openDB(filename) )

      } // for ( eventToCacheMapIter = eventToCacheMap.begin() ;  eventToCacheMapIter !=
        // eventToCacheMap.end() ; ++eventToCacheMapIter )
    }
  }
  return 0;
}

void
parseXML(bool showMissingFiles)
{
  std::string cacheEventTable;
  std::string cacheName;
  bool verbose = (verboseFlag.length() > 0);

  FILE* fp = fopen(cacheMappingXMLFile.c_str(), "r");
  if (fp)
  {
    for (;;)
    {
      char buffer[5000];
      buffer[0] = 0;
      fgets(buffer, 5000, fp);

      if (buffer[0] == 0)
      {
        break;
      }

      else if (strstr(buffer, "<Notification "))
      {
        char* begin = strchr(buffer, '"');
        if (begin)
        {
          begin++;
          char* end = strchr(begin, '"');
          if (end)
          {
            *end = 0;
            cacheEventTable = begin;
          }
        }
      }

      else if (strstr(buffer, "<CacheId "))
      {
        char* begin = strchr(buffer, '"');
        if (begin)
        {
          begin++;
          char* end = strchr(begin, '"');
          if (end)
          {
            time_t maxTime = 0;
            *end = 0;
            cacheName = begin;

            std::string winner;
            std::string pattern = "^" + cacheName + "\\..*\\.db$";
            std::vector<std::string> fileList;
            size_t matches = FileAccess::getFiles(directory, pattern, fileList);

            if (showMissingFiles && (matches == 0))
            {
              fprintf(stderr,
                      "no matching file %s.*.db for cache type %s\n",
                      cacheName.c_str(),
                      cacheEventTable.c_str());
            }

            for (size_t k = 0; k < matches; ++k)
            {
              struct stat sbuf;
              std::string tempCacheName = fileList[k];
              std::string tempDirCacheName = directory + "/" + tempCacheName;

              if (stat(tempDirCacheName.c_str(), &sbuf) == 0)
              {
                if (sbuf.st_ctime > maxTime)
                {
                  maxTime = sbuf.st_ctime;
                  winner = tempCacheName;
                }
              }
            }

            if (winner.length())
            {
              cacheName = winner;
              if (verbose)
                fprintf(stderr, "Berkeley db filename: %s\n", cacheName.c_str());
            }
            else
            {
              continue;
            }

            eventToCacheMapIter = eventToCacheMap.find(cacheName);

            if (verbose && (eventToCacheMapIter != eventToCacheMap.end()))
            {
              fprintf(stderr, "dup eventToCacheMap entry [%s]\n", cacheName.c_str());
            }

            std::pair<std::string, std::string> pair = std::make_pair(cacheName, cacheEventTable);
            eventToCacheMap.insert(pair);
          }
        }
      }
    }
    fclose(fp);
  }

  if (verbose)
  {
    for (eventToCacheMapIter = eventToCacheMap.begin();
         eventToCacheMapIter != eventToCacheMap.end();
         ++eventToCacheMapIter)
    {
      fprintf(stderr,
              "eventToCacheMap file [%s] entityType [%s]\n",
              eventToCacheMapIter->first.c_str(),
              eventToCacheMapIter->second.c_str());
    }
  }
}
