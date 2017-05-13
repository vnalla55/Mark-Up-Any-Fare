#include "Tools/ldc/ldc-utils/CacheEvent.h"
#include "Tools/ldc/ldc-utils/CommandLine.h"
#include "Tools/ldc/ldc-utils/FileAccess.h"
#include "Tools/ldc/ldc-utils/LDCUtil.h"
#include "Tools/ldc/ldc-utils/mytimes.h"
#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <cstring>
#include <limits>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static std::string directory1;
static std::string directory2;
static std::string bdbFile = "all";
static std::string skipHeaderFlag;
static std::string verboseFlag;
static std::string showMatchingKeys;
static std::string dumpFlag;

struct ArgProc argProcInfo[] = { { true, 'd', directory1, "1st bdb directory" },
                                 { true, 'D', directory2, "2nd bdb directory" },
                                 { false, 'f', bdbFile, "bdb file (default = all)" },
                                 { false, 'v', verboseFlag, "verbose" },
                                 { false, 'h', dumpFlag, "hex dump of records" },
                                 { false, 's', showMatchingKeys, "show matching keys" },
                                 { false, 'z', skipHeaderFlag, "skip header" } };
size_t argProcInfoLen = sizeof argProcInfo / sizeof(struct ArgProc);

void
parseXML(bool showMatchingKeys);

int
main(int argc, char** argv)
{
  std::string dbtable;
  std::string filename;

  CommandLine cmd(argc, argv, argProcInfo, argProcInfoLen);
  bool verbose = (verboseFlag.length() > 0);
  bool skipHeader = (skipHeaderFlag.length() > 0);
  bool showMatchingKeysOpt = (showMatchingKeys.length() > 0);
  bool dumpOpt = (dumpFlag.length() > 0);

  if (!skipHeader)
  {
    fprintf(stdout, "bdb-file,bdb-key,compare-status\n");
  }

  if (verbose)
    fprintf(stderr, "Berkeley db directory - %s\n", directory1.c_str());

  LDCUtil ldc1(directory1, verbose);
  if (!ldc1.isValid())
  {
    fprintf(stderr, "cannot open Berkeley db directory - %s\n", directory1.c_str());
    exit(1);
  }

  if (verbose)
    fprintf(stderr, "Berkeley db directory - %s\n", directory2.c_str());

  LDCUtil ldc2(directory2, verbose);
  if (!ldc2.isValid())
  {
    fprintf(stderr, "cannot open Berkeley db directory - %s\n", directory2.c_str());
    exit(1);
  }

  std::string pattern = ".*\\.[0-9]*\\.db$";
  std::vector<std::string> fileList;
  if (bdbFile == "all")
  {
    FileAccess::getFiles(directory1, pattern, fileList);
  }
  else
  {
    fileList.push_back(bdbFile);
  }

  for (size_t item = 0; item < fileList.size(); ++item)
  {
    std::string fname = fileList[item];
    if (verbose)
      fprintf(stderr, " process file %s\n", fname.c_str());

    if (ldc1.openDB(fname))
    {
      if (ldc2.openDB(fname))
      {
        std::map<std::string, LDCUtil::RecInfo*, std::less<std::string> > map1;
        std::map<std::string, LDCUtil::RecInfo*, std::less<std::string> > map2;

        std::vector<LDCUtil::RecInfo> list1;
        std::vector<LDCUtil::RecInfo> list2;
        size_t index;
        size_t sz;

        ldc1.getKeyList(list1);
        sz = list1.size();

        // put keys in the to maps

        for (index = 0; index < sz; ++index)
        {
          std::string key;
          if (list1[index].keyPtr == NULL)
            continue;
          key = std::string(list1[index].keyPtr);
          map1[key] = &list1[index];
        }

        ldc2.getKeyList(list2);
        sz = list2.size();

        for (index = 0; index < sz; ++index)
        {
          std::string key;
          if (list2[index].keyPtr == NULL)
            continue;
          key = std::string(list2[index].keyPtr);
          map2[key] = &list2[index];
        }

        // compare maps

        LDCUtil::RecInfo::BlobHeader* ptr1;
        LDCUtil::RecInfo::BlobHeader* ptr2;
        size_t size1;
        size_t size2;
        size_t firstDiff = std::numeric_limits<size_t>::max();
        std::map<std::string, LDCUtil::RecInfo*, std::less<std::string> >::iterator iter1;
        std::map<std::string, LDCUtil::RecInfo*, std::less<std::string> >::iterator iter2;
        enum
        { NO_DATA,
          MATCH,
          SIZE,
          NOMATCH,
          READ_ERROR } state = NO_DATA;

        for (iter1 = map1.begin(); iter1 != map1.end(); ++iter1)
        {
          ptr1 = NULL;
          ptr2 = NULL;
          std::string key = iter1->first;

          iter2 = map2.find(key);

          if (iter2 != map2.end())
          {
            size1 = iter1->second->data.size;
            size2 = iter2->second->data.size;

            ptr1 = iter1->second->header;
            ++ptr1;

            ptr2 = iter2->second->header;
            ++ptr2;

            if (size1 == size2)
            {
              size_t compareSize = size1 - sizeof(LDCUtil::RecInfo::BlobHeader);

              if (memcmp(ptr1, ptr2, compareSize) == 0)
              {
                state = MATCH;
              }
              else
              {
                for (firstDiff = 0; firstDiff < compareSize; ++firstDiff)
                {
                  char* c1 = (char*)ptr1 + firstDiff;
                  char* c2 = (char*)ptr2 + firstDiff;
                  if (*c1 != *c2)
                    break;
                }
                state = NOMATCH;
              }
            }
            else
            {
              state = SIZE;
            }
          }
          else
          {
            state = NO_DATA;
          }

          const char* result = NULL;

          switch (state)
          {
          case NO_DATA:
          {
            if (verbose)
            {
              result = "No Matching Data";
            }
            break;
          }
          case MATCH:
          {
            if (showMatchingKeysOpt)
            {
              result = "Data Matches";
            }
            break;
          }
          case SIZE:
          {
            result = "Size of Data Differs";
            break;
          }
          case NOMATCH:
          {
            result = "Data Does not Match";
            break;
          }
          case READ_ERROR:
          {
            result = "Read Error";
            break;
          }
          }

          if (result)
          {
            fprintf(stdout, "%s,%s,%s\n", fname.c_str(), key.c_str(), result);

            if (dumpOpt)
            {
              if (firstDiff != std::numeric_limits<size_t>::max())
              {
                fprintf(
                    stdout, "\n    byte offset of first difference %X\n\n", unsigned(firstDiff));
              }
              else
              {
                fprintf(stdout, "\n");
              }

              if (ptr1)
              {
                fprintf(stdout,
                        "       ======== record from %s ================\n",
                        directory1.c_str());
                dumpHexData(ptr1, size1, true, stdout);
                fprintf(stdout, "\n");
              }
              if (ptr2)
              {
                fprintf(stdout,
                        "       ======== record from %s ================\n",
                        directory2.c_str());
                dumpHexData(ptr2, size2, true, stdout);
                fprintf(stdout, "\n");
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
