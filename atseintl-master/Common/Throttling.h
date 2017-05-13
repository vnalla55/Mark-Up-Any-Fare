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
#pragma once

#include <atomic>
#include <boost/tokenizer.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tse
{
class ParseCustomer;
class Trx;

class CounterTag
{
public:
  CounterTag(const uint16_t con) : _counter(0), _concurrentTrxLimit(con) {}
  CounterTag(const CounterTag& tag)
    : _counter(static_cast<uint16_t>(tag._counter)), _concurrentTrxLimit(tag._concurrentTrxLimit)
  {
  }

  bool incrementCounter();
  void decrementCounter();

private:
  std::atomic<uint16_t> _counter;
  const uint16_t _concurrentTrxLimit;
};

class Throttling
{
  typedef std::map<const std::size_t, CounterTag> CounterMap;

public:
  Throttling(Trx& trx);
  ~Throttling();

  void checkThrottlingConditions() const;
  CounterTag& addCounterTag(const std::string& keyCondStr, CounterTag& tag);

private:
  void updateCache();

  Trx& _trx;
  std::vector<std::size_t> _localHashKeys;
  static boost::shared_mutex _mutex;
  static boost::shared_mutex _predMutex;
  static CounterMap _tagsCounter;
  static std::string _throttleConfig;
  static std::vector<std::unique_ptr<ParseCustomer>> _predCache;
};

} // end tse
