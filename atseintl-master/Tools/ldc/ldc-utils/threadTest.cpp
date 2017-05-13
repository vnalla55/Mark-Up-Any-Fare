#include "Tools/ldc/ldc-utils/CommandLine.h"
#include "Tools/ldc/ldc-utils/FileAccess.h"
#include "Tools/ldc/ldc-utils/LDCUtil-mt.h"
#include "Tools/ldc/ldc-utils/mytimes.h"
#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static std::string directory;
static std::string verboseFlag;
static std::string threadCount;
static LDCUtil* globalLDCUtil = NULL;
static bool stopThreads = false;

struct ArgProc argProcInfo[] = { { true, 'd', directory, "bdb directory" },
                                 { false, 't', threadCount, "thread Count" },
                                 { false, 'v', verboseFlag, "verbose" } };
size_t argProcInfoLen = sizeof argProcInfo / sizeof(struct ArgProc);

static pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
static size_t openCount = 0;

void
countOpens()
{
  pthread_mutex_lock(&myMutex);
  ++openCount;
  pthread_mutex_unlock(&myMutex);
}

static void*
workerThread(void* arg)
{
  std::string fname((const char*)arg);

  while (!stopThreads)
  {
    DB* db = globalLDCUtil->openDB(fname);

    if (db == NULL)
    {
      fprintf(stderr, "cannot open db file %s\n", fname.c_str());
      stopThreads = true;
    }
    else
    {
      countOpens();
      globalLDCUtil->closeDB(db);
    }
  }

  return 0;
}

int
main(int argc, char** argv)
{
  std::string dbtable;
  std::string filename;

  CommandLine cmd(argc, argv, argProcInfo, argProcInfoLen);
  bool verbose = (verboseFlag.length() > 0);

  // ======================================================================================
  // create bdb object
  // ======================================================================================

  if (verbose)
    fprintf(stderr, "Berkeley db directory - %s\n", directory.c_str());

  LDCUtil ldc(directory, verbose);
  if (!ldc.isValid())
  {
    fprintf(stderr, "cannot open Berkeley db directory - %s\n", directory.c_str());
    exit(1);
  }
  globalLDCUtil = &ldc;

  std::string pattern = ".*\\.[0-9]*\\.db$";
  std::vector<std::string> fileList;
  FileAccess::getFiles(directory, pattern, fileList);

  // determine thread Count

  size_t tc = fileList.size();

  if (threadCount.length())
  {
    size_t val = (size_t)atoi(threadCount.c_str());
    if (val <= tc)
    {
      tc = val;
    }
  }

  pthread_t* tids = new pthread_t[tc];
  pthread_t* ptids = tids;

  for (size_t item = 0; item < tc; ++item)
  {
    pthread_create(ptids, NULL, workerThread, (void*)fileList[item].c_str());
    ++ptids;
  }
  sleep(2);
  stopThreads = true;

  ptids = tids;
  for (size_t item = 0; item < tc; ++item)
  {
    pthread_join(*ptids, NULL);
    ++ptids;
  }
  delete[] tids;

  fprintf(stderr, "did %u opens\n", static_cast<unsigned>(openCount));

  return 0;
}
