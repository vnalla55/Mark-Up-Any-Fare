//-------------------------------------------------------------------
//
//  File:        TrxAborter.h
//  Created:     May 24, 2005
//  Authors:     David White
//
//  Description: Interface which is used to define methods for
//               aborting transactions mid-course.
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/Thread/TimerTask.h"

#include <memory>
#include <mutex>
#include <vector>

namespace tse
{

class Trx;

void
checkTrxAborted(Trx& trx);
bool
checkTrxMustHurry(Trx& trx);
bool
checkTrxMustHurryWithCond(Trx& trx);

class TrxAborter
{
public:
  enum class Reason : uint8_t
  {
    TIMEOUT = 1 << 0,
    HURRY = 1 << 1,
    MEMORY = 1 << 2,
  };

  enum class ErrorKind
  {
    TIMEOUT,
    MEMORY,

    COUNT,
    NO_ERROR = -1,
  };

  static ErrorKind reason2kind(uint8_t reason)
  {
    if (reason & (uint8_t(Reason::TIMEOUT) | uint8_t(Reason::HURRY)))
      return ErrorKind::TIMEOUT;
    if (reason & uint8_t(Reason::MEMORY))
      return ErrorKind::MEMORY;
    return ErrorKind::NO_ERROR;
  }

  struct Error
  {
    ErrorResponseException::ErrorResponseCode errorCode;
    std::string errorMsg;
  };

  TrxAborter(Trx* trx);

  time_t getTimeOutAt() const { return _timeoutAt; }
  int timeout() const { return _timeout; }
  time_t getHurryAt() const { return _hurryAt; }

  void setTimeout(int timeout);
  void setHurry(int hurry);
  void setHurryWithCond(int hurry);

  void hurry();
  // TODO Remove with unifyMemoryAborter.
  void abort();
  void abort(uint8_t reason);

  bool getHurryLogicActivatedFlag() const { return _hurryLogicActivatedFlag; }

  void setErrorCode(const ErrorResponseException::ErrorResponseCode errorCode)
  {
    std::lock_guard<std::mutex> guard(_mutex);
    _errorCode = errorCode;
  }

  void setErrorMsg(const std::string& errorMsg)
  {
    std::lock_guard<std::mutex> guard(_mutex);
    _errorMsg = errorMsg;
  }

  void setError(ErrorKind kind, Error error)
  {
    std::lock_guard<std::mutex> guard(_mutex);
    _errors[int(kind)] = std::move(error);
  }

  // function which allows the TrxAborter to change its mode so that
  // it will throw an exception in checkTrxAborted() even if it only
  // needs to hurry rather than actually abort. Not all aborters need
  // support this functionality, and may choose not to override this function
  void setAbortOnHurry(bool setting = true);

  void setHurryLogicActivatedFlag();

  void terminateAborterTasks();

  void addChildTrx(Trx* trx);

  template <typename Func>
  void forEachTrx(Func func)
  {
    std::lock_guard<std::mutex> guard(_mutex);
    func(_trx);
    for (Trx* trx : _childTrx)
      func(trx);
  }

private:
  void checkTrxAborted() const;

  // function which returns true if we are almost out of time, and
  // so must hurry our response as fast as possible
  bool mustHurryResponse() const;

  // function which returns true if we reach the percentage of total timeout
  // allowed, and so the process will fall into flight only solution
  bool mustHurryWithCond() const;

  bool isTimeoutSet() const { return _timeoutAt != 0; }
  bool isHurrySet() const { return _hurryAt != 0; }
  bool isHurryWithCondSet() const { return _hurryWithCond != 0; }

  bool isPastTimeout(time_t now) const { return now > _timeoutAt; }
  bool isPastHurry(time_t now) const { return now > _hurryAt; }
  bool isPastHurryWithCond(time_t now) const { return now > _hurryWithCond; }

  Trx* _trx;
  std::vector<Trx*> _childTrx;
  time_t _timeoutAt;
  time_t _hurryAt;
  time_t _hurryWithCond;
  bool _abortOnHurry;
  int _timeout;
  bool _hurryLogicActivatedFlag;

  std::unique_ptr<TimerTask> _abortTask;
  std::unique_ptr<TimerTask> _hurryTask;
  std::unique_ptr<TimerTask> _hurryCondTask;

  // TODO Remove these two with unifyMemoryAborter.
  ErrorResponseException::ErrorResponseCode _errorCode;
  std::string _errorMsg;

  Error _errors[int(ErrorKind::COUNT)];
  std::mutex _mutex;

  friend void checkTrxAborted(Trx& trx);
  friend bool checkTrxMustHurry(Trx& trx);
  friend bool checkTrxMustHurryWithCond(Trx& trx);
};
}


