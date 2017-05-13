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
static std::string cacheMappingXMLFile = "cacheNotify.xml";
static std::string bdbFile = "all";

static std::multimap<std::string, std::string, std::less<std::string> > eventToCacheMap;
static std::multimap<std::string, std::string, std::less<std::string> >::iterator
eventToCacheMapIter;
static std::string verboseFlag;
static std::string showMissingFiles;

struct ArgProc argProcInfo[] = {
  { true, 'd', directory, "bdb directory" },
  { false, 'f', bdbFile, "bdb file (default = all)" },
  { false, 'x', cacheMappingXMLFile, "cache mapping XML file (default = cacheNotify.xml)" },
  { false, 'v', verboseFlag, "verbose" },
  { false, 'y', showMissingFiles, "show missing cache files" }
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
  bool showMissingFilesOpt = (showMissingFiles.length() > 0);

  // ======================================================================================
  // get bdb files from command line or xml file
  // ======================================================================================

  if (bdbFile == "all")
  {
    parseXML(showMissingFilesOpt);
    if (showMissingFilesOpt)
      return 0;
  }
  else
  {
    std::pair<std::string, std::string> pair = std::make_pair(bdbFile, bdbFile);
    eventToCacheMap.insert(pair);
  }

  // ===========================================================
  //  print header
  // ===========================================================

  fprintf(stdout, "bdb-file,bdb-date,bdb-time,bdb-size,bdb-compr,bdb-type,bdb-key\n");

  // ======================================================================================
  // create bdb object
  // ======================================================================================

  if (verbose)
    fprintf(stderr, "Berkeley db directory - %s\n", directory.c_str());
  LDCUtil ldc(directory, verbose);
  if (ldc.isValid())
  {
    // ===========================================================
    //  select berkeley db files from input xml file
    // ===========================================================

    std::string lastFilename;
    static std::multimap<std::string, std::string, std::less<std::string> >::iterator mainFileIter;

    for (mainFileIter = eventToCacheMap.begin(); mainFileIter != eventToCacheMap.end();
         ++mainFileIter)
    {
      filename = mainFileIter->first;
      if (lastFilename == filename)
      {
        continue;
      }
      lastFilename = filename;

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

      // ===========================================================
      //  process berkeley db file
      // ===========================================================

      if (ldc.openDB(filename))
      {
        std::vector<LDCUtil::RecInfo> list;
        ldc.getKeyList(list);

        for (size_t idx = 0; idx < list.size(); ++idx)
        {
          fprintf(stdout, "%s,", filename.c_str());
          list[idx].print();
          fprintf(stdout, "\n");
        }
      } // if ( ldc.openDB(filename) )
    } // for ( eventToCacheMapIter = eventToCacheMap.begin() ;  eventToCacheMapIter !=
      // eventToCacheMap.end() ; ++eventToCacheMapIter )
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
