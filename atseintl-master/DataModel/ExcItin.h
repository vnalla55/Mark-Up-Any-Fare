//-------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "Common/Money.h"
#include "DataModel/Itin.h"
#include "DataModel/RexBaseTrx.h"

#include <map>
#include <vector>

namespace tse
{
class TravelSeg;

class SameSegs : public std::binary_function<const TravelSeg*, const TravelSeg*, bool>
{
public:
  bool operator()(const TravelSeg* newSeg, const TravelSeg* excSeg) const;
};

class ExcItin : public Itin
{
  friend class BaseExchangeTrxTest;

public:
  typedef std::map<const FareMarket*, FareCompInfo*> FmFcCache;

  ExcItin()
    : _stopOverChange(1, false),
      _domesticCouponChange(1, false),
      _someSegmentsChanged(1, false),
      _someSegmentsConfirmed(1, false),
      _sameSegmentsInventoryChanged(1, false),
      _travelCommenceDate(DateTime::emptyDate())
  {
  }

  bool doNotUseForPricing(const FareMarket& fm, const Itin& itin);

  void setTktValidityDate(Year year, Month month, Day day);
  const DateTime& tktValidityDate() const { return _tktValidityDate; }

  FareCompInfo* findFareCompInfo(const FareMarket* fareMarket) const;

  bool& stopOverChange();
  const bool stopOverChange() const;

  bool& domesticCouponChange();
  const bool domesticCouponChange() const;

  DateTime& travelCommenceDate() { return _travelCommenceDate; }
  const DateTime& travelCommenceDate() const { return _travelCommenceDate; }

  bool& someSegmentsChanged();
  const bool someSegmentsChanged() const;

  bool& someSegmentsConfirmed();
  const bool someSegmentsConfirmed() const;

  bool& sameSegmentsInventoryChanged();
  const bool sameSegmentsInventoryChanged() const;

  bool allSegmentsUnchangedOrConfirmed()
  {
    return (!someSegmentsChanged() && !sameSegmentsInventoryChanged());
  }
  const bool allSegmentsUnchangedOrConfirmed() const
  {
    return (!someSegmentsChanged() && !sameSegmentsInventoryChanged());
  }

  bool allSegmentsUnchangedOrInventoryChanged()
  {
    return (!someSegmentsChanged() && !someSegmentsConfirmed());
  }
  const bool allSegmentsUnchangedOrInventoryChanged() const
  {
    return (!someSegmentsChanged() && !someSegmentsConfirmed());
  }

  FareCompInfo* findFareCompInfoInCache(const FareMarket* fm) const
  {
    if (!fm || _fmFcCache.empty())
      return nullptr;
    FmFcCache::const_iterator i = _fmFcCache.find(fm);
    return i == _fmFcCache.end() ? nullptr : (*i).second;
  }

  CurrencyNoDec& calculationCurrencyNoDec() { return _calculationCurrencyNoDec; }
  const CurrencyNoDec& calculationCurrencyNoDec() const { return _calculationCurrencyNoDec; }
  virtual const DateTime& travelDate() const override;
  RexBaseTrx*& rexTrx() { return _rexTrx; }
  const RexBaseTrx* rexTrx() const { return _rexTrx; }

  const uint16_t& getExcItinIndex() const { return _excItinIndex; }
  void setExcItinIndex(uint16_t index, size_t maxIndex = 1);

  // void setItinIndex(uint16_t index,size_t maxIndex = 1 );

  void determineSegsChangesFor988Match();
  const std::vector<bool>& getSegsChangesFor988Match(const FareMarket& fm) const
  {
    return _changedSegs.at(&fm);
  }

  FareMarket* getSiblingMarket(const FareMarket* regularExcMarket) const
  {
    std::map<const FareMarket*, FareMarket*>::const_iterator it =
        _siblingMarketsWithOtherFares.find(regularExcMarket);
    if (it == _siblingMarketsWithOtherFares.end())
      return nullptr;
    return it->second;
  }

  void addSiblingMarket(const FareMarket* regularExcMarket, FareMarket* siblingMarket)
  {
    _siblingMarketsWithOtherFares[regularExcMarket] = siblingMarket;
  }

  Money getNonRefAmount() const;
  Money getNonRefAmountInNUC() const;

protected:
  // uint16_t& getItinIndex(){return _excItinIndex;}
  // const uint16_t& getItinIndex() const {return _excItinIndex;}

  DateTime _tktValidityDate;
  std::deque<bool> _stopOverChange;
  std::deque<bool> _domesticCouponChange;

  std::deque<bool> _someSegmentsChanged;
  std::deque<bool> _someSegmentsConfirmed;
  std::deque<bool> _sameSegmentsInventoryChanged;

  DateTime _travelCommenceDate;
  FmFcCache _fmFcCache;

  CurrencyNoDec _calculationCurrencyNoDec = 2;
  RexBaseTrx* _rexTrx = nullptr;
  uint16_t _excItinIndex = 0;

  std::map<const FareMarket*, FareMarket*> _siblingMarketsWithOtherFares;

private:
  void matchTSs(const FareMarket& efm, std::list<const TravelSeg*>& ntss);
  std::map<const FareMarket*, std::vector<bool> > _changedSegs;
};
} // tse namespace
