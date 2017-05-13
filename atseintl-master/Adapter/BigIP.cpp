// ----------------------------------------------------------------------------
//
//  File:         BigIP.cpp
//  Author:       John Watilo
//  Created:      October 2007
//  Description:  BigIP interface
//
//  Copyright Sabre 2007
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

#include "Adapter/BigIP.h"

#include "Common/FallbackUtil.h"
#include "Common/Thread/TSELockGuards.h"

#include <boost/tokenizer.hpp>

#include <cerrno>
#include <cstring>
#include <mutex>
#include <sstream>
#include <string>

#include <stdio.h>
#include <unistd.h>

namespace tse
{
FIXEDFALLBACK_DECL(replaceFastMutexInBigIP)

// assumption is that this script will be somewhere in PATH
static const char* SCRIPT = "bigipsvc.sh";

BigIP::BigIP(const std::string& device, const std::string& systype, const std::string& logFileName)
  : mDevice(device), mSystype(systype)
{
  char* pwd = getcwd(nullptr, 0);
  if (pwd != nullptr)
  {
    mLogFile = pwd;
    free(pwd);
  }
  else
  {
    mLogFile = ".";
  }
  mLogFile += '/';
  mLogFile.append(logFileName);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function BigIP::getScriptPath
//
// Description: Determine the full path to the SCRIPT
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
BigIP::getScriptPath(STRINGVEC& responses)
{
  std::string retval;
  std::string path;
  const char* envPath = getenv("PATH");
  if (envPath != nullptr)
  {
    path.append(envPath);
  }
  path.append(":.");

  boost::char_separator<char> sep(":", "\n", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > tokenizer(path, sep);
  for (boost::tokenizer<boost::char_separator<char> >::iterator i = tokenizer.begin();
       ((i != tokenizer.end()) && (*i != "\n"));
       ++i)
  {
    std::string temp(*i);
    temp += '/';
    temp += SCRIPT;
    if (access(temp.c_str(), F_OK) == 0)
    {
      retval = temp;
      break;
    }
  }

  if (retval.empty())
  {
    std::string msg = "ERROR!  Unable to locate script [";
    msg.append(SCRIPT);
    msg.append("] in PATH!");
    responses.push_back(msg);
  }

  return retval;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function BigIP::callScript
//
// Description: Run the bigipsvc.sh script
//
// </PRE>
// ----------------------------------------------------------------------------

bool
BigIP::callScript(const std::string& cmd, STRINGVEC& responses)
{
  bool retval = false;
  char buf[1024];
  responses.clear();

  FILE* fp = popen(cmd.c_str(), "r");
  if (fp != nullptr)
  {
    while (fgets(buf, sizeof(buf) - 1, fp) != nullptr)
    {
      responses.push_back(buf);
    }

    int status = pclose(fp);

    // true if the child exited normally
    if (WIFEXITED(status))
    {
      // child's exit code, can only be evaluated if WIFEXITED is true
      retval = (WEXITSTATUS(status) == 0);
    }
    else if (WIFSIGNALED(status))
    {
      if ((WTERMSIG(status) == 127) || (WTERMSIG(status) == 128))
      {
        // These signals *could* be erroneously thrown, even though the child
        // process finished OK
        retval = true;
      }
      else
      {
        // Uncaught signal in child process
        // Can be determined via WTERMSIG( status )
        responses.push_back("Uncaught signal in child process!");
        std::stringstream ss;
        ss << "Signal number:" << WTERMSIG(status);
        responses.push_back(ss.str());
      }
    }
    else
    {
      responses.push_back("Child process terminated abnormally!");
      responses.push_back(strerror(errno));
    }
  }
  else
  {
    responses.push_back("Unable to allocate memory for file pointer!");
    responses.push_back(strerror(errno));
  }

  return retval;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function BigIP::registerMember
//
// Description: Register member in a pool
//
// </PRE>
// ----------------------------------------------------------------------------

bool
BigIP::registerMember(const std::string& pool,
                      uint32_t port,
                      uint32_t connLimit,
                      const std::string& connLimitBasis,
                      STRINGVEC& responses)
{
  bool retval = false;

  std::string script = getScriptPath(responses);
  if (!script.empty())
  {
    char cmd[1024];
    sprintf(cmd,
            "%s -v -connlimit %d -connlimitBasis:%s -logfile %s -noshowstamp -keeplog %s %s "
            "register %s %d",
            script.c_str(),
            connLimit,
            connLimitBasis.c_str(),
            mLogFile.c_str(),
            mSystype.c_str(),
            mDevice.c_str(),
            pool.c_str(),
            port);

    if ((retval = callScript(cmd, responses)))
    {
      if (!fallback::fixed::replaceFastMutexInBigIP())
      {
        std::lock_guard<std::mutex> lock(_poolMapMutex);
        mPoolMap[pool].insert(port);
      }
      else
      {
        TSEGuard lock(mMutex);
        mPoolMap[pool].insert(port);
      }
    }
  }

  return retval;
}


// ----------------------------------------------------------------------------
// <PRE>
//
// @function BigIP::deregisterMember
//
// Description: Deregister member from a pool
//
// </PRE>
// ----------------------------------------------------------------------------

bool
BigIP::deregisterMember(const std::string& pool, uint32_t port, bool remove, STRINGVEC& responses)
{
  bool retval = false;

  std::string script = getScriptPath(responses);
  if (!script.empty())
  {
    std::string remFlag = (remove ? " -remove -waitnoconn:30" : "");

    char cmd[1024];
    sprintf(cmd,
            "%s%s -v -logfile %s -noshowstamp -keeplog %s %s deregister %s %d",
            script.c_str(),
            remFlag.c_str(),
            mLogFile.c_str(),
            mSystype.c_str(),
            mDevice.c_str(),
            pool.c_str(),
            port);

    if ((retval = callScript(cmd, responses)))
    {
      if (!fallback::fixed::replaceFastMutexInBigIP())
      {
        std::lock_guard<std::mutex> lock(_poolMapMutex);
        mPoolMap[pool].erase(port);
      }
      else
      {
        TSEGuard lock(mMutex);
        if (mPoolMap[pool].find(port) != mPoolMap[pool].end())
        {
          mPoolMap[pool].erase(port);
        }
      }
    }
  }

  return retval;
}
}
