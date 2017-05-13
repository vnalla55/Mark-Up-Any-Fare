
#include "DataModel/TrxAborter.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DataModel/AborterTasks.h"
#include "DataModel/Trx.h"

#include <chrono>
#include <time.h>

namespace tse
{
FALLBACK_DECL(reworkTrxAborter);

namespace
{
Logger
logger("atseintl.DataModel.TrxAborter");

std::chrono::milliseconds
calculateDuration(uint32_t formerDuration)
{
  TimerTask::Clock::duration duration = TimerTask::Clock::now().time_since_epoch();
  TimerTask::Clock::duration durationInSecondsCeil =
      std::chrono::duration_cast<std::chrono::seconds>(duration) + std::chrono::seconds(1);
  return std::chrono::duration_cast<std::chrono::milliseconds>(durationInSecondsCeil - duration) +
         std::chrono::seconds(formerDuration);
}
}

TrxAborter::TrxAborter(Trx* trx)
  : _trx(trx),
    _timeoutAt(0),
    _hurryAt(0),
    _hurryWithCond(0),
    _abortOnHurry(false),
    _timeout(0),
    _hurryLogicActivatedFlag(false),
    _errorCode(ErrorResponseException::REQUEST_TIMEOUT),
    _errorMsg("Request processing time exceeded client-specified "
              "timeout and was terminated"),
    _errors{
      {ErrorResponseException::REQUEST_TIMEOUT,
       "Request processing time exceeded client-specified timeout and was terminated"},
      {ErrorResponseException::MEMORY_EXCEPTION,
       "Request processing consumed too much memory and was terminated"}
    }
{
}

void
TrxAborter::setTimeout(int timeout)
{
  _timeout = timeout;
  _timeoutAt = time(nullptr) + timeout;
  if (_hurryAt == 0)
  {
    _hurryAt = _timeoutAt;
  }
  if (!fallback::reworkTrxAborter(_trx) && !_trx->isForceNoTimeout())
    _abortTask.reset(new AborterTask<AborterTaskMode::ABORT>(this, calculateDuration(timeout)));
}

void
TrxAborter::setHurry(int hurry)
{
  _hurryAt = time(nullptr) + hurry;
  if (!fallback::reworkTrxAborter(_trx) && !_trx->isForceNoTimeout())
    _hurryTask.reset(new AborterTask<AborterTaskMode::HURRY>(this, calculateDuration(hurry)));
}

void
TrxAborter::setHurryWithCond(int hurry)
{
  _hurryWithCond = time(nullptr) + hurry;
  if (!fallback::reworkTrxAborter(_trx) && !_trx->isForceNoTimeout())
    _hurryCondTask.reset(
        new AborterTask<AborterTaskMode::HURRY_COND>(this, calculateDuration(hurry)));
}

void
TrxAborter::setHurryLogicActivatedFlag()
{
  _hurryLogicActivatedFlag = true;
}

void
TrxAborter::setAbortOnHurry(bool setting)
{
  _abortOnHurry = setting;
}

bool
TrxAborter::mustHurryWithCond() const
{
  return isHurryWithCondSet() && isPastHurryWithCond(time(nullptr));
}

bool
TrxAborter::mustHurryResponse() const
{
  return isHurrySet() && isPastHurry(time(nullptr));
}

void
TrxAborter::checkTrxAborted() const
{
  if (!isTimeoutSet() && !isHurrySet())
  {
    return;
  }

  const time_t now = time(nullptr);
  if ((isTimeoutSet() && isPastTimeout(now)) || (_abortOnHurry && isHurrySet() && isPastHurry(now)))
  {
    if (_errorMsg.empty())
      throw ErrorResponseException(_errorCode);
    else
      throw ErrorResponseException(_errorCode, _errorMsg.c_str());
  }
}

void
TrxAborter::terminateAborterTasks()
{
  _abortTask.reset();
  _hurryTask.reset();
  _hurryCondTask.reset();
}

void
TrxAborter::addChildTrx(Trx* trx)
{
  std::lock_guard<std::mutex> guard(_mutex);

  trx->aborter() = this;
  trx->setForceNoTimeout(_trx->isForceNoTimeout());
  trx->setAborterFlagsAs(_trx);
  _childTrx.push_back(trx);
}

void
TrxAborter::abort()
{
  setHurryLogicActivatedFlag();
  if (_errorMsg.empty())
    throw ErrorResponseException(_errorCode);
  else
    throw ErrorResponseException(_errorCode, _errorMsg.c_str());
}

void
TrxAborter::abort(uint8_t reason)
{
  const ErrorKind kind = reason2kind(reason);
  TSE_ASSERT(kind != ErrorKind::NO_ERROR);

  setHurryLogicActivatedFlag();
  const Error& error = _errors[int(kind)];
  if (error.errorMsg.empty())
    throw ErrorResponseException(error.errorCode);
  else
    throw ErrorResponseException(error.errorCode, error.errorMsg.c_str());
}

void
TrxAborter::hurry()
{
  setHurryLogicActivatedFlag();
  LOG4CXX_DEBUG(logger, "Returning hurry response");
}

void
checkTrxAborted(Trx& trx)
{
  if (!fallback::reworkTrxAborter(&trx))
  {
    trx.checkTrxAborted();
    return;
  }

  if (trx.isForceNoTimeout())
    return;

  TrxAborter* const aborter = trx.aborter();
  if (UNLIKELY(aborter))
  {
    try { aborter->checkTrxAborted(); }
    catch (...)
    {
      aborter->setHurryLogicActivatedFlag();
      throw;
    }
  }
}

bool
checkTrxMustHurry(Trx& trx)
{
  if (LIKELY(trx.isForceNoTimeout()))
    return false;

  TrxAborter* const aborter = trx.aborter();
  if (aborter)
  {
    bool result = aborter->mustHurryResponse();

    if (result)
    {
      aborter->setHurryLogicActivatedFlag();
      LOG4CXX_DEBUG(logger, "Returning hurry response");
    }

    return result;
  }
  return false;
}

bool
checkTrxMustHurryWithCond(Trx& trx)
{
  if (LIKELY(trx.isForceNoTimeout()))
    return false;

  TrxAborter* const aborter = trx.aborter();
  if (aborter)
  {
    bool result = aborter->mustHurryWithCond();

    if (result)
      aborter->setHurryLogicActivatedFlag();

    return result;
  }
  return false;
}
} // namespace tse

