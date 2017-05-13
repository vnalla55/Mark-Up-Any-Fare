#include "Tools/ldc/ldc-utils/CommandLine.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

CommandLine::CommandLine(int argc_, char** argv_, ArgProc* argProcInfo_, size_t argProcInfoLen_)
  : argc(argc_), argv(argv_), argProcInfo(argProcInfo_), argProcInfoLen(argProcInfoLen_)
{
  bool optionSpace = false;
  char option;
  size_t optionIndex;
  for (int iarg = 1; iarg < argc; ++iarg)
  {
    const char* opt = argv[iarg];
    {
      if (*opt == '-')
      {
        size_t prevIndex = optionIndex;
        ++opt;
        option = *opt++;
        optionSpace = ((*opt == 0) || (*opt == ' '));

        for (optionIndex = 0; optionIndex < argProcInfoLen; ++optionIndex)
        {
          if (option == argProcInfo[optionIndex].opt)
          {
            break;
          }
        }
        if (optionIndex == argProcInfoLen)
        {
          if ((prevIndex < argProcInfoLen) && isdigit(option))
          {
            --opt;
            --opt;
            argProcInfo[prevIndex].data = opt;
            continue;
          }
          else
          {
            fprintf(stderr, "option %c undefined\n", option);
            usage();
            exit(-1);
          }
        }
        if (optionSpace)
        {
          argProcInfo[optionIndex].data = "t";
        }
        else
        {
          argProcInfo[optionIndex].data = opt;
        }
      }
      else if (optionSpace)
      {
        argProcInfo[optionIndex].data = opt;
        optionSpace = false;
      }
      else
      {
        unqualified.push_back(opt);
        // usage();
        // exit(-1);
      }
    }
  }

  for (size_t opt = 0; opt < argProcInfoLen; ++opt)
  {
    if (argProcInfo[opt].required && argProcInfo[opt].data.length() == 0)
    {
      usage();
      exit(-1);
    }
  }
}

void
CommandLine::usage()
{
  fprintf(stderr, "\nprogram %s usage\n\nrequired options\n", argv[0]);
  for (size_t sz = 0; sz < argProcInfoLen; ++sz)
  {
    if (argProcInfo[sz].required)
    {
      fprintf(stderr, "\t-%c %s", argProcInfo[sz].opt, argProcInfo[sz].description);

      if (argProcInfo[sz].data.length() == 0)
      {
        fprintf(stderr, " (missing)");
      }
      fprintf(stderr, "\n");
    }
  }
  fprintf(stderr, "optional\n");
  for (size_t sz = 0; sz < argProcInfoLen; ++sz)
  {
    if (!argProcInfo[sz].required)
    {
      fprintf(stderr, "\t-%c %s\n", argProcInfo[sz].opt, argProcInfo[sz].description);
    }
  }
}
