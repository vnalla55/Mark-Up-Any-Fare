#include "Tools/ldc/ldc-utils/CacheEvent.h"
#include "Tools/ldc/ldc-utils/CommandLine.h"
#include "Tools/ldc/ldc-utils/FileAccess.h"
#include "Tools/ldc/ldc-utils/LDCUtil.h"
#include "Tools/ldc/ldc-utils/mytimes.h"
#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static std::string directory1;
static std::string bdbFile = "all";
static std::string skipHeaderFlag;
static std::string verboseFlag;
static std::string dumpFlag;

struct ArgProc argProcInfo[] = { { true, 'd', directory1, "1st bdb directory" },
                                 { false, 'f', bdbFile, "bdb file (default = all)" },
                                 { false, 'v', verboseFlag, "verbose" },
                                 { false, 'h', dumpFlag, "hex dump of records" },
                                 { false, 'z', skipHeaderFlag, "skip header" } };
size_t argProcInfoLen = sizeof argProcInfo / sizeof(struct ArgProc);

int
main(int argc, char** argv)
{
  std::string dbtable;
  std::string filename;

  CommandLine cmd(argc, argv, argProcInfo, argProcInfoLen);
  bool verbose = (verboseFlag.length() > 0);
  bool skipHeader = (skipHeaderFlag.length() > 0);
  bool dumpOpt = (dumpFlag.length() > 0);

  // ===========================================================
  //  print header
  // ===================
  //
  if (!skipHeader)
  {
    fprintf(stdout, "bdb-file,bdb-key,compare-status\n");
  }

  // ======================================================================================
  // create bdb object
  // ======================================================================================

  if (verbose)
    fprintf(stderr, "Berkeley db directory - %s\n", directory1.c_str());

  LDCUtil ldc1(directory1, verbose);
  if (!ldc1.isValid())
  {
    fprintf(stderr, "cannot open Berkeley db directory - %s\n", directory1.c_str());
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
      std::vector<LDCUtil::RecInfo> list1;
      size_t index;
      size_t sz;

      ldc1.getKeyList(list1);
      sz = list1.size();

      // put keys in the to maps

      for (index = 0; index < sz; ++index)
      {
        LDCUtil::RecInfo ri(list1[index]);
        bool readOK = ldc1.read(ri, verbose);
        std::string reason = "OK";

        if (readOK)
        {
          if (list1[index].data.size == ri.data.size)
          {
            if (memcmp(list1[index].dataPtr, ri.dataPtr, ri.data.size) != 0)
            {
              reason = "Different Data";
            }
          }
          else
          {
            reason = "Size Difference";
          }
        }
        else
        {
          reason = "Read Error";
        }

        if (verbose || (reason != "OK"))
        {
          fprintf(stdout, "%s,%s,%s\n", fname.c_str(), list1[index].keyPtr, reason.c_str());
        }

        if (dumpOpt && list1[index].dataPtr)
        {
          const char* key = "";
          if (list1[index].keyPtr)
          {
            key = list1[index].keyPtr;
          }
          fprintf(stdout,
                  "       ====== file : %s//%s : key %s\n",
                  directory1.c_str(),
                  fname.c_str(),
                  key);
          dumpHexData(list1[index].dataPtr, list1[index].data.size, true, stdout);
          fprintf(stdout, "\n");
        }
      }
    }
  }
  return 0;
}
