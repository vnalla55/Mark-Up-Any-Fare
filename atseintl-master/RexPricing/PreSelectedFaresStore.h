//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/FareCompInfo.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/PaxTypeFareWrapper.h"

#include <map>
#include <vector>

namespace tse
{

class PaxTypeFare;
class FareMarket;

class PreSelectedFaresStore
{
public:
  bool
  restore(const FareCompInfo& fc, RexDateSeqStatus date, std::vector<PaxTypeFareWrapper>& fares)
      const
  {
    boost::lock_guard<boost::mutex> guard(_mutex);

    Key key(&fc, fc.fareMarket(), date);
    std::map<Key, Item>::const_iterator it = _store.find(key);
    if (it == _store.end())
      return false;

    fares = it->second;

    return true;
  }

  void
  store(const FareCompInfo& fc, RexDateSeqStatus date, const std::vector<PaxTypeFareWrapper>& fares)
  {
    boost::lock_guard<boost::mutex> guard(_mutex);

    Key key(&fc, fc.fareMarket(), date);
    _store[key] = fares;
  }

  void clear()
  {
    boost::lock_guard<boost::mutex> guard(_mutex);
    _store.clear();
  }

  bool empty() const { return _store.empty(); }

  typedef std::vector<PaxTypeFareWrapper> Item;

  class Key
  {
  public:
    Key(const FareCompInfo* fc, const FareMarket* fm, RexDateSeqStatus date)
      : _fc(fc), _fm(fm), _date(date)
    {
    }

    bool operator<(const Key& key) const
    {
      if (_fc < key._fc)
        return true;
      if (_fc == key._fc && _fm < key._fm)
        return true;
      if (_fc == key._fc && _fm == key._fm && _date < key._date)
        return true;
      return false;
    }

  private:
    const FareCompInfo* _fc;
    const FareMarket* _fm;
    RexDateSeqStatus _date;
  };

protected:
  std::map<Key, Item> _store;
  mutable boost::mutex _mutex;

  friend class PreSelectedFaresStoreTest;
};
}


