///----------------------------------------------------------------------------
//
//  File:           Throllting.cpp
//  Created:        10/06/2015
//  Authors:
//
//  Description:    Common functions required throttle transactions.
//
//  Updates:
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/Throttling.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/DynamicConfigurableString.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/ThrottlingPred.h"
#include "DataModel/PricingTrx.h"

#include <boost/functional/hash.hpp>
#include <stdexcept>
#include <utility>

namespace tse
{
static DynamicConfigurableString
throttleDCS("TSE_SERVER", "THROTTLE", "");

boost::shared_mutex Throttling::_mutex;
boost::shared_mutex Throttling::_predMutex;
Throttling::CounterMap Throttling::_tagsCounter;
std::string Throttling::_throttleConfig;
std::vector<std::unique_ptr<ParseCustomer>> Throttling::_predCache;

static Logger
logger("atseintl.Common.Throttling");

Throttling::Throttling(Trx& trx): _trx(trx)
{
  _trx.setThrottling(this);

  try
  {
    updateCache();
  }
  catch (std::logic_error& e)
  {
    LOG4CXX_WARN(logger, e.what());
  }
  catch (boost::bad_lexical_cast& e)
  {
    LOG4CXX_WARN(logger, e.what());
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "THROTTLE: UNKNOWN ERROR");
    throw;
  }
}

void
Throttling::checkThrottlingConditions() const
{
  boost::shared_lock<boost::shared_mutex> lock(_predMutex);

  for (const std::unique_ptr<ParseCustomer>& customer : _predCache)
  {
    if (!customer->test(_trx))
    {
      LOG4CXX_INFO(logger, "THROTTLE: TRX THROTTLED");
      throw ErrorResponseException(ErrorResponseException::TRANSACTION_THRESHOLD_REACHED);
    }
  }
}

Throttling::~Throttling()
{
  boost::shared_lock<boost::shared_mutex> lock(_mutex);

  for (const std::size_t& localHashKey : _localHashKeys)
  {
    Throttling::CounterMap::iterator pos = _tagsCounter.find(localHashKey);
    if (pos != _tagsCounter.end())
      pos->second.decrementCounter();
  }
}

CounterTag&
Throttling::addCounterTag(const std::string& keyCondStr, CounterTag& tag)
{
  boost::hash<std::string> stringHash;
  std::size_t hashKey = stringHash(keyCondStr);

  _localHashKeys.push_back(hashKey);

  boost::unique_lock<boost::shared_mutex> lock(_mutex);
  return _tagsCounter.insert(std::make_pair(hashKey, tag)).first->second;
}

void
Throttling::updateCache()
{
  boost::unique_lock<boost::shared_mutex> lock(_predMutex);

  std::string throttleStr = throttleDCS.getValue(&_trx);
  LOG4CXX_DEBUG(logger, "THROTTLE: READING CONFIGURATION - " << throttleStr);

  if (_throttleConfig == throttleStr)
      return;

  _throttleConfig = throttleStr;

  boost::tokenizer<boost::char_separator<char>> customers(throttleStr,
                                                          boost::char_separator<char>(";"));

  _predCache.clear();
  decltype(_predCache) tmpCache;

  for (const std::string& customer : customers)
    tmpCache.push_back(std::unique_ptr<ParseCustomer>(new ParseCustomer(customer)));

  _predCache = std::move(tmpCache);
}

bool
CounterTag::incrementCounter()
{
  return _counter.fetch_add(1, std::memory_order_release) < _concurrentTrxLimit;
}

void
CounterTag::decrementCounter()
{
  _counter.fetch_sub(1, std::memory_order_release);
}

} // end tse
