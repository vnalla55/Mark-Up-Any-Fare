//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Common/TrxUtil.h"
#include "DataModel/TrxAborter.h"
#include "Pricing/Shopping/FiltersAndPipes/FilterObserverList.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"

#include <boost/utility.hpp>

#include <vector>

namespace tse
{
class PricingTrx;
namespace utils
{

template <typename T>
class BasicEmptyPolicy
{
public:
  static bool elementEmpty(const T& t) { return t.empty(); }
};

// A generator filtering elements using registered predicates
// (DP filter)
template <typename T, typename EmptyPolicy = BasicEmptyPolicy<T> >
class GeneratingFilter : public IGenerator<T>, public IFilter<T>, boost::noncopyable
{
public:
  // Constructor accepting a source generator
  // where elements are taken from

  // maxRetries: tells how many times next() from the source
  // generator is called before giving up. Zero means
  // that we never give up.
  // On give up, an empty combination is returned and
  // a message is logged to logger, if provided.
  GeneratingFilter(IGenerator<T>& source, size_t maxRetries = 0,
      ILogger* logger = nullptr): _source(source), _maxRetries(maxRetries)
  {
    if (logger != nullptr)
    {
      _log.install(logger);
    }
  }

  // Adds a new predicate to this object
  void addPredicate(INamedPredicate<T>* predicate) override
  {
    TSE_ASSERT(nullptr != predicate);
    _predicates.push_back(predicate);
  }

  // Adds an observer notified about failed elements
  void addObserver(IFilterObserver<T>* observer) override
  {
    _observers.addFilterObserver(observer);
  }

  T next() override
  {
    T t;
    bool valid;
    size_t attempts = 0;

    do
    {
      if ((_maxRetries != 0) && (attempts > _maxRetries))
      {
        _log->info(Fmt("Quitting after %u invalid elements") % _maxRetries);
        return T();
      }

      ++attempts;
      t = _source.next();
      valid = isElementValid(t);
    } while (!valid);

    return t;
  }

  // Reads elements from the source collector
  // until it finds the first element which is valid,
  // i.e. all predicates return true for it, and
  // return this element
  T next(PricingTrx& trx)
  {
    T t;
    bool valid;
    size_t attempts = 0;
    uint64_t trialCnt = 0, abortCheckInterval = TrxUtil::abortCheckInterval(trx);

    do
    {
      if (UNLIKELY(++trialCnt == abortCheckInterval))
      {
        try
        {
          // Replace with trx.checkTrxAborted() when removing reworkTrxAborter fallback.
          checkTrxAborted(trx);
        }
        catch (ErrorResponseException& ex) { return T(); }
        trialCnt = 0;
      }

      if (UNLIKELY((_maxRetries != 0) && (attempts > _maxRetries)))
      {
        _log->info(Fmt("Quitting after %u invalid elements") % _maxRetries);
        return T();
      }

      ++attempts;
      t = _source.next();
      valid = isElementValid(t);
    } while (!valid);

    return t;
  }

private:
  // An element is valid only if all registered
  // predicates return true for it
  bool isElementValid(const T& t)
  {
    // An empty element is always valid,
    // being a marker of the sequence end
    if (EmptyPolicy::elementEmpty(t))
    {
      return true;
    }

    // (Just return true for non-empty element if no predicates registered)
    for (auto& elem : _predicates)
    {
      if (!(*elem)(t))
      {
        _observers.elementInvalid(t, *elem);
        return false;
      }
    }
    return true;
  }

  IGenerator<T>& _source;
  std::vector<INamedPredicate<T>*> _predicates;
  FilterObserverList<T> _observers;
  size_t _maxRetries;
  LoggerHandle _log;
};

} // namespace utils

} // namespace tse

