// ----------------------------------------------------------------------------
//
//  File:         BigIP.h
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

#pragma once

#include "Common/Thread/TSEFastMutex.h"

#include <cstdint>
#include <cstdlib>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace tse
{

class BigIP
{
public:
  using PORTSET = std::set<uint32_t, std::less<int>>;
  using POOLMAP = std::map<std::string, PORTSET, std::less<std::string>>;
  using STRINGVEC = std::vector<std::string>;

  // constructor
  BigIP(const std::string& device, const std::string& systype, const std::string& logFileName);
  BigIP(const BigIP&) = delete;
  BigIP& operator=(const BigIP&) = delete;

  // register a pool member
  bool registerMember(const std::string& pool,
                      uint32_t port,
                      uint32_t connLimit,
                      const std::string& connLimitBasis,
                      STRINGVEC& responses);

  // deregister a pool member
  bool deregisterMember(const std::string& pool, uint32_t port, bool remove, STRINGVEC& responses);

private:
  // member variables
  POOLMAP mPoolMap;
  std::string mDevice;
  std::string mSystype;
  // STRINGVEC   mResponses ;
  std::string mLogFile;
  TSEFastMutex mMutex;
  std::mutex _poolMapMutex;

  // call the bigipsvc.sh script
  bool callScript(const std::string& cmd, STRINGVEC& responses);

  // get full path to the script, using the PATH environment variable
  std::string getScriptPath(STRINGVEC& responses);
};
}
