//----------------------------------------------------------------------------
//
//  File:               TseManagerUtil.h
//  Description:        Base execution routine for Manager objects
//  Created:            03/24/2005
//  Authors:            Mark Kasprowicz
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"

#include <boost/thread/mutex.hpp>

#include <string>

#include <stdint.h>

namespace ZThread
{
class CountingSemaphore;
}

namespace tse
{
class Xform;
class Service;
class ErrorResponseException;
class Trx;

class TseManagerUtil
{
public:
  friend class TseManagerUtilTest;
  class TrxCounter;

  static bool process(DateTime startTime,
                      TrxCounter& trxCounter,
                      std::string& req,
                      std::string& rsp,
                      Xform& xform,
                      Service& service,
                      bool throttled,
                      ZThread::CountingSemaphore* processedRequestsCS = nullptr,
                      int requestID = 0);
  static void deinitialize();

  static void setTrxTimeout(Trx& trx);

  class TrxCounter
  {
  public:
    TrxCounter();
    ~TrxCounter();
    uint16_t localCount() const { return _localCount; }
    static uint16_t count();

  private:
    uint16_t _localCount;

    static volatile uint16_t _count;
    static boost::mutex _mutex;
  };

private:
  TseManagerUtil();
  TseManagerUtil(const TseManagerUtil& rhs);
  TseManagerUtil& operator=(const TseManagerUtil& rhs);

  static void checkTrxThreshold(uint16_t count,
                                bool throttled);
  static void handleException(std::string& rsp,
                              Xform& xform,
                              Trx* trx,
                              ErrorResponseException& ere,
                              bool rc = false);
  static void setTrxActivationFlags(Trx* trx);
  static void setShoppingGsaActivationFlags(Trx* trx);
};
}

