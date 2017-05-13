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
#include "DataModel/ExchangeOverrides.h"

#include "Common/Money.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"

namespace tse
{

namespace
{
template <class T>
class TSFinder : public std::unary_function<const T*, bool>
{
public:
  TSFinder(const TravelSeg* travelSeg) : _travelSeg(travelSeg) {}

  bool operator()(const T* so) const
  {
    return !so->fromExchange() || so->travelSeg() != _travelSeg;
  }

private:
  const TravelSeg* _travelSeg;
};

template <class T>
class PlusesFinder
{
public:
  PlusesFinder(const FareUsage& fu) : _fu(fu) {}

  void find(const std::vector<T*>& allPluses)
  {
    std::vector<TravelSeg*>::const_iterator tsI = _fu.travelSeg().begin();
    for (; tsI != _fu.travelSeg().end(); ++tsI)
    {
      std::remove_copy_if(
          allPluses.begin(), allPluses.end(), std::back_inserter(_foundPluses), TSFinder<T>(*tsI));
    }
  }

  std::vector<const T*> _foundPluses;

private:
  const FareUsage& _fu;
};

class MatchDiffToSegment
{
  const DifferentialOverride& _diff;

public:
  MatchDiffToSegment(const DifferentialOverride& diff) : _diff(diff) {}

  bool operator()(const TravelSeg* travelSeg)
  {
    return (travelSeg->boardMultiCity() == _diff.highDiffFareOrigin() &&
            travelSeg->offMultiCity() == _diff.highDiffFareDestination()) ||
           (travelSeg->boardMultiCity() == _diff.highDiffFareDestination() &&
            travelSeg->offMultiCity() == _diff.highDiffFareOrigin());
  }
};

class AdequateDiff
{
  const std::vector<TravelSeg*>& _segments;

public:
  AdequateDiff(const std::vector<TravelSeg*>& segments) : _segments(segments) {}

  bool operator()(const DifferentialOverride* diff) const
  {
    if (!diff->fromExchange() || diff->usedForRecreate())
      return false;

    return std::find_if(_segments.begin(), _segments.end(), MatchDiffToSegment(*diff)) !=
           _segments.end();
  }
};
}

ExchangeOverrides::OrderedTripChunks
ExchangeOverrides::createOrderedTripChunks(FarePath& fPath) const
{
  OrderedTripChunks orderedTripChunks;

  std::vector<PricingUnit*>::iterator puI = fPath.pricingUnit().begin();
  for (; puI != fPath.pricingUnit().end(); ++puI)
    orderedTripChunks.insert((**puI).fareUsage().begin(), (**puI).fareUsage().end());

  return orderedTripChunks;
}

void
ExchangeOverrides::fill(FarePath& farePath, bool copySurcharges) const
{
  OrderedTripChunks orderedTripChunks = createOrderedTripChunks(farePath);

  OrderedTripChunks::iterator fui = orderedTripChunks.begin();
  for (; fui != orderedTripChunks.end(); ++fui)
  {
    if (copySurcharges)
      fillSurcharges(**fui);
    fillStopovers(**fui, farePath);
    fillDifferentials(**fui, farePath);
  }

  std::for_each(_differentialOverride.begin(),
                _differentialOverride.end(),
                std::bind2nd(std::mem_fun<void>(&DifferentialOverride::usedForRecreate), false));
}

void
ExchangeOverrides::fillSurcharges(FareUsage& fu) const
{
  PlusesFinder<SurchargeOverride> finder(fu);
  finder.find(_surchargeOverride);

  std::vector<const SurchargeOverride*>::iterator surI = finder._foundPluses.begin();
  for (; surI != finder._foundPluses.end(); ++surI)
  {
    SurchargeData* surchargeData = _trx.constructSurchargeData();
    (**surI).initialize(*surchargeData);
    fu.surchargeData().push_back(surchargeData);
  }
}

void
ExchangeOverrides::fillStopovers(FareUsage& fu, FarePath& farePath) const
{
  PlusesFinder<StopoverOverride> finder(fu);
  finder.find(_stopoverOverride);

  std::vector<const StopoverOverride*>::iterator stopI = finder._foundPluses.begin();
  for (; stopI != finder._foundPluses.end(); ++stopI)
  {
    FareUsage::StopoverSurcharge* stopoverSurcharge =
        _trx.dataHandle().create<FareUsage::StopoverSurcharge>();
    (**stopI).initialize(*stopoverSurcharge, getFormattedCurrency(farePath));
    fu.addStopoverSurcharge(*stopoverSurcharge, stopoverSurcharge->amount());
    farePath.increaseTotalNUCAmount(stopoverSurcharge->amount());
  }
}

void
ExchangeOverrides::fillDifferentials(FareUsage& fu, FarePath& farePath) const
{
  std::vector<DifferentialOverride*>::const_iterator diff =
      std::find_if(_differentialOverride.begin(),
                   _differentialOverride.end(),
                   AdequateDiff(fu.paxTypeFare()->fareMarket()->travelSeg()));
  // same as TSs in FU ???

  if (diff != _differentialOverride.end())
  {
    DifferentialData* differentialData = _trx.dataHandle().create<DifferentialData>();
    (**diff).initialize(*differentialData, *fu.paxTypeFare()->fareMarket());

    fu.differentialPlusUp().push_back(differentialData);
    fu.differentialAmt() += differentialData->amount();
    farePath.plusUpAmount() += differentialData->amount();
    farePath.increaseTotalNUCAmount(differentialData->amount());
    (**diff).usedForRecreate(true);
  }
}

std::pair<CurrencyCode, CurrencyNoDec>
ExchangeOverrides::getFormattedCurrency(const FarePath& farePath) const
{
  Money money(farePath.itin()->calcCurrencyOverride().empty()
                  ? farePath.itin()->calculationCurrency()
                  : farePath.itin()->calcCurrencyOverride());

  return std::make_pair(money.code(), money.noDec(_trx.ticketingDate()));
}

void
SurchargeOverride::initialize(SurchargeData& surchargeData) const
{
  surchargeData.brdAirport() = _travelSeg->origAirport();
  surchargeData.offAirport() = _travelSeg->destAirport();
  surchargeData.travelPortion() = '1'; // RuleConst::ONEWAY
  surchargeData.itinItemCount() = 1;
  surchargeData.surchargeType() = _type;
  surchargeData.amountSelected() = _amount;
  surchargeData.currSelected() = _currency;
  surchargeData.amountNuc() = _amount;
  surchargeData.singleSector() = _singleSector;
  surchargeData.fcBrdCity() = _fcBrdCity;
  surchargeData.fcOffCity() = _fcOffCity;
  surchargeData.fcFpLevel() = _fcFpLevel;
}

void
StopoverOverride::initialize(FareUsage::StopoverSurcharge& stopoverSurcharges,
                             const std::pair<CurrencyCode, CurrencyNoDec>& formattedCurrency) const
{
  stopoverSurcharges.amount() = _amount;
  stopoverSurcharges.currencyCode() = formattedCurrency.first;
  stopoverSurcharges.noDecimals() = formattedCurrency.second;
  stopoverSurcharges.travelSeg() = _travelSeg;
  stopoverSurcharges.isSegmentSpecific() = true;
}

void
DifferentialOverride::initialize(DifferentialData& differentialData, const FareMarket& fareMarket)
    const
{
  differentialData.amount() = _amount;
  differentialData.origin() = fareMarket.origin();
  differentialData.destination() = fareMarket.destination();
  differentialData.fareClassHigh() = _highFareClass;
}
}
