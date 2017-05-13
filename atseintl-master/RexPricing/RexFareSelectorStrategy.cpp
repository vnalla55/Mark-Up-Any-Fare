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

#include "RexPricing/RexFareSelectorStrategy.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "Diagnostic/DiagManager.h"
#include "RexPricing/RexFareSelectorStrategyAlgo.h"
#include "Rules/RuleConst.h"

namespace tse
{
FALLBACK_DECL(excDiscDiag23XImprovements);
FALLBACK_DECL(diag23XVarianceFix);
FALLBACK_DECL(sortMatchedExcFaresByDiff);

const MoneyAmount NOT_AMOUNT = -1.0;

const double RexFareSelectorStrategy::VARIANCE_SQ[3] = { 0.03, 0.06, 0.09 };

void
RexFareSelectorStrategy::setInitialFaresStatus(std::vector<PaxTypeFare*>& fares,
                                               unsigned category,
                                               bool catIsValid) const
{
  std::vector<PaxTypeFare*>::const_iterator faresIter = fares.begin();

  for (; faresIter != fares.end(); ++faresIter)
    (*faresIter)->setCategoryValid(category, catIsValid);
}

void
RexFareSelectorStrategy::process(FareCompInfo& fc) const
{
  std::vector<PaxTypeFareWrapper> preSelected;

  if (!getPreSelectedFares(fc, _trx.getCurrTktDateSeqStatus(), preSelected))
    preSelect(fc, preSelected);

  if (!processImpl(fc, preSelected))
    setPreSelectedFares(fc, _trx.getCurrTktDateSeqStatus(), preSelected);
}

bool
RexFareSelectorStrategy::processImpl(FareCompInfo& fc, std::vector<PaxTypeFareWrapper>& preSelected)
    const
{
  std::vector<PaxTypeFareWrapper> result;
  selection(fc, preSelected, result);

  if (!fallback::sortMatchedExcFaresByDiff(&_trx))
  {
    std::sort(result.begin(),
              result.end(),
              [fcAmt = fc.getTktBaseFareCalcAmt()](const PaxTypeFareWrapper& wrp1,
                                                   const PaxTypeFareWrapper& wrp2)
              { return std::abs(wrp1.getAmount() - fcAmt) < std::abs(wrp2.getAmount() - fcAmt); });
  }

  updateFareComp(fc, result);
  return !result.empty();
}

void
RexFareSelectorStrategy::updateFareComp(FareCompInfo& fc, std::vector<PaxTypeFareWrapper>& selected)
    const
{
  for (PaxTypeFareWrapper& wrp : selected)
  {
    wrp.get()->setCategoryValid(getCategory(), true);
    // Please remove PaxTypeFareWrapper::getVarianceOld() on fallback removal
    if (fallback::diag23XVarianceFix(&_trx))
      fc.matchedFares().push_back(
          FareCompInfo::MatchedFare(wrp.get(), wrp.getVarianceOld(fc.getTktBaseFareCalcAmt())));
    else
      fc.matchedFares().emplace_back(wrp.get(), wrp.getVariance(fc.getTktBaseFareCalcAmt()));
  }
  fc.updateFareMarket(_trx);
}

namespace
{

class CurrencyCheck : public std::unary_function<const PaxTypeFare*, bool>
{
public:
  CurrencyCheck(const CurrencyCode& curr) : _currency(curr) {}
  bool operator()(const PaxTypeFare* ptf) const { return ptf->currency() == _currency; }

protected:
  const CurrencyCode& _currency;
};

class SimpleFareBasisCodeCheck : public std::unary_function<const PaxTypeFare*, bool>
{
public:
  SimpleFareBasisCodeCheck(const RexBaseTrx& trx, const FareCompInfo& fc) : _trx(trx), _fc(fc) {}

  bool operator()(const PaxTypeFare* ptf) const
  {
    return _fc.fareBasisCode() == ptf->createFareBasis(const_cast<RexBaseTrx&>(_trx));
  }

protected:
  const RexBaseTrx& _trx;
  const FareCompInfo& _fc;
};

class TruncatedFareBasisCodeCheck : public std::unary_function<const PaxTypeFare*, bool>
{
public:
  TruncatedFareBasisCodeCheck(const RexBaseTrx& trx, const FareCompInfo& fc)
    : _trx(trx), _fbc((fc.fareBasisCode()).substr(0, 13).c_str())
  {
  }

  bool operator()(const PaxTypeFare* ptf) const
  {
    return _fbc == ptf->createFareBasis(const_cast<RexBaseTrx&>(_trx)).substr(0, 13);
  }

protected:
  const RexBaseTrx& _trx;
  const std::string _fbc;
};

class CheckForcedSideTrip : public std::unary_function<const TravelSeg*, bool>
{
public:
  bool operator()(const TravelSeg* seg) const { return (seg->forcedSideTrip() != 'T'); }
};

class IsNotIndustry : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  bool operator()(const PaxTypeFareWrapper& wrp) const { return !wrp.isIndustry(); }
};

class IsATPVendor : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  bool operator()(const PaxTypeFareWrapper& wrp) const { return wrp.get()->vendor().equalToConst("ATP"); }
};

class IsDirectionality : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  IsDirectionality(Directionality dir) : _dir(dir) {}

  bool operator()(const PaxTypeFareWrapper& wrp) const
  {
    return wrp.get()->directionality() == _dir;
  }

protected:
  Directionality _dir;
};

class SecurityCheck : public std::unary_function<const PaxTypeFare*, bool>
{
public:
  bool operator()(const PaxTypeFare* ptf) const
  {
    return ptf->isCategoryValid(1) && ptf->isCategoryValid(15) && ptf->isCategoryValid(35);
  }
};

class SecurityCheckWrp : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  bool operator()(const PaxTypeFareWrapper& wrp) const
  {
    return wrp.get()->isCategoryValid(1) && wrp.get()->isCategoryValid(15) &&
           wrp.get()->isCategoryValid(35);
  }
};

} // namespace

bool
RexFareSelectorStrategy::checkForcedSideTrip(const FareMarket& fm) const
{
  const std::vector<TravelSeg*>& travelSegs = fm.travelSeg();

  return std::find_if(travelSegs.begin(), travelSegs.end(), CheckForcedSideTrip()) ==
         travelSegs.end();
}

std::vector<PaxTypeFare*>::iterator
RexFareSelectorStrategy::selectByDirectionality(FareCompInfo& fc,
                                                std::vector<PaxTypeFare*>::iterator begin,
                                                std::vector<PaxTypeFare*>::iterator end) const
{
  const FareMarket& fm = *fc.fareMarket();

  if (fm.geoTravelType() != GeoTravelType::International)
    return end;

  if (checkForcedSideTrip(fm))
    return end;

  switch (fm.direction())
  {
  case FMDirection::OUTBOUND:
    return std::remove_if(begin, end, IsDirectionality(TO));

  case FMDirection::INBOUND:
  default:
    return end;
  }
}

void
RexFareSelectorStrategy::preSelect(FareCompInfo& fc, std::vector<PaxTypeFareWrapper>& preSelected)
    const
{
  PaxTypeBucket* ptc = fc.getPaxTypeBucket(_trx);
  if (ptc == nullptr)
    return;

  std::vector<PaxTypeFare*>& fares = ptc->paxTypeFare();

  if (_trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL,
                                       Diagnostic::MATCHING))
  {
    DiagManager diag(const_cast<RexBaseTrx&>(_trx));
    diag.activate(Diagnostic231);
    if (!fallback::excDiscDiag23XImprovements(&_trx))
    {
      diag.activate(Diagnostic233);
    }
    diag << "-----------------------------------------------------------------------\n";
    diag << "FARE COMPONENT " << fc.fareCompNumber() <<
            ", FARE MARKET: " <<  fc.fareMarket()->toString() << "\n";
    diag << "-----------------------------------------------------------------------\n\n";

    const std::string& fareClass = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

    for (PaxTypeFare* ptf : fares)
    {
      if (fareClass.empty() || ptf->fareClass() == fareClass ||
          fallback::excDiscDiag23XImprovements(&_trx))
        diag.collector().DiagCollector::operator<<(*ptf) << "\n";
    }
  }

  setInitialFaresStatus(fares, getCategory(), false);

  std::vector<PaxTypeFare*> preSelectedFares;

  if (fc.fareBasisCode().find("/") != std::string::npos)
    std::remove_copy_if(fares.begin(),
                        fares.end(),
                        back_inserter(preSelectedFares),
                        std::not1(TruncatedFareBasisCodeCheck(_trx, fc)));
  else
    std::remove_copy_if(fares.begin(),
                        fares.end(),
                        back_inserter(preSelectedFares),
                        std::not1(SimpleFareBasisCodeCheck(_trx, fc)));

  std::vector<PaxTypeFare*>::iterator newEnd =
      selectByDirectionality(fc, preSelectedFares.begin(), preSelectedFares.end());

  const CurrencyCode& currency = (_trx.exchangeItin().front()->calcCurrencyOverride().empty()
                                      ? _trx.exchangeItin().front()->calculationCurrency()
                                      : _trx.exchangeItin().front()->calcCurrencyOverride());

  bool usePublishedCurrency = false;

  if (currency != NUC)
  {
    usePublishedCurrency =
        find_if(fares.begin(), fares.end(), CurrencyCheck(currency)) != fares.end();
    if (usePublishedCurrency)
      newEnd = std::remove_if(preSelectedFares.begin(), newEnd, std::not1(CurrencyCheck(currency)));
  }

  if (fc.hasVCTR() || _trx.skipSecurityForExcItin())
    std::for_each(preSelectedFares.begin(),
                  newEnd,
                  std::bind2nd(std::mem_fun(&PaxTypeFare::setSkipCat35ForRex), true));
  else
    newEnd = std::remove_if(preSelectedFares.begin(), newEnd, std::not1(SecurityCheck()));

  preSelected.assign(preSelectedFares.begin(), newEnd);


  updateAmount(preSelected.begin(), preSelected.end(), usePublishedCurrency,
               _trx.getRexOptions().isNetSellingIndicator());
}

namespace
{

struct PublishedAmount_deprecated
{
  void operator()(PaxTypeFareWrapper& wrp) { wrp.setAmount(wrp.get()->fareAmount()); }
};

struct ConvertedAmount_deprecated
{
  ConvertedAmount_deprecated(const RexBaseTrx& trx)
    : _trx(trx),
      _target(_trx.exchangeItin().front()->calcCurrencyOverride().empty()
                  ? _trx.exchangeItin().front()->calculationCurrency()
                  : _trx.exchangeItin().front()->calcCurrencyOverride())
  {
  }

  void operator()(PaxTypeFareWrapper& wrp)
  {
    Money source(wrp.get()->fareAmount(), wrp.get()->currency());
    wrp.setAmount(_facade.convert(_target,
                                  source,
                                  const_cast<RexBaseTrx&>(_trx),
                                  _trx.exchangeItin().front()->useInternationalRounding())
                      ? _target.value()
                      : NOT_AMOUNT);
  }

protected:
  const RexBaseTrx& _trx;
  CurrencyConversionFacade _facade;
  Money _target;
};

struct SellLevelAmountGetter
{
  MoneyAmount operator()(const PaxTypeFareWrapper& wrp) const { return wrp.get()->fareAmount(); }
};

struct NetLevelAmountGetter
{
  MoneyAmount operator()(const PaxTypeFareWrapper& wrp) const
  {
    if (!wrp.get()->isNegotiated())
      return wrp.get()->fareAmount();

    typedef const NegPaxTypeFareRuleData* Data;
    Data rule = static_cast<Data>(wrp.get()->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE));
    return rule ? rule->netAmount() : wrp.get()->fareAmount();
  }
};

template<typename AmountGetter>
struct PublishedAmount
{
  void operator()(PaxTypeFareWrapper& wrp) { wrp.setAmount(AmountGetter()(wrp)); }
};

template<typename AmountGetter>
struct ConvertedAmount
{
  ConvertedAmount(const RexBaseTrx& trx)
    : _trx(trx),
      _target(_trx.exchangeItin().front()->calcCurrencyOverride().empty()
                  ? _trx.exchangeItin().front()->calculationCurrency()
                  : _trx.exchangeItin().front()->calcCurrencyOverride())
  {
  }

  void operator()(PaxTypeFareWrapper& wrp)
  {
    Money source(AmountGetter()(wrp), wrp.get()->currency());
    wrp.setAmount(_facade.convert(_target,
                                  source,
                                  const_cast<RexBaseTrx&>(_trx),
                                  _trx.exchangeItin().front()->useInternationalRounding())
                      ? _target.value()
                      : NOT_AMOUNT);
  }

protected:
  const RexBaseTrx& _trx;
  CurrencyConversionFacade _facade;
  Money _target;
};

template<typename AmountGetter>
void
amountUpdate(const RexBaseTrx& trx,
             RexFareSelectorStrategy::Iterator begin, RexFareSelectorStrategy::Iterator end,
             bool usePublishedCurrency)
{
  if (usePublishedCurrency)
    std::for_each(begin, end, PublishedAmount<AmountGetter>());
  else
    std::for_each(begin, end, ConvertedAmount<AmountGetter>(trx));
}

} // namespace

void
RexFareSelectorStrategy::updateAmount(Iterator begin, Iterator end, bool usePublishedCurrency) const
{
  if (usePublishedCurrency)
    std::for_each(begin, end, PublishedAmount_deprecated());
  else
    std::for_each(begin, end, ConvertedAmount_deprecated(_trx));
}

void
RexFareSelectorStrategy::updateAmount(Iterator begin, Iterator end,
                                      bool usePublishedCurrency, bool considerNetAmonut) const
{
  if (considerNetAmonut)
    amountUpdate<NetLevelAmountGetter>(_trx, begin, end, usePublishedCurrency);
  else
    amountUpdate<SellLevelAmountGetter>(_trx, begin, end, usePublishedCurrency);
}


uint16_t
RexFareSelectorStrategy::getCategory() const
{
  return (_trx.excTrxType() == PricingTrx::AF_EXC_TRX ? RuleConst::VOLUNTARY_REFUNDS_RULE
                                                      : RuleConst::VOLUNTARY_EXCHANGE_RULE);
}

void
RexFareSelectorStrategy::selection(FareCompInfo& fc,
                                   std::vector<PaxTypeFareWrapper>& preSelected,
                                   std::vector<PaxTypeFareWrapper>& result) const
{
  if (preSelected.empty())
    return;

  Iterator midpoint = stablePartition(preSelected.begin(), preSelected.end(), IsNotIndustry()),
           leftpoint = stablePartition(preSelected.begin(), midpoint, IsATPVendor());

  if (((preSelected.begin() != leftpoint) &&
       selectionNextLevel(fc, preSelected.begin(), leftpoint, result)) ||
      ((leftpoint != midpoint) && selectionNextLevel(fc, leftpoint, midpoint, result)))
    return;

  Iterator rightpoint = stablePartition(midpoint, preSelected.end(), IsATPVendor());

  if ((midpoint != rightpoint) && selectionNextLevel(fc, midpoint, rightpoint, result))
    return;

  if (rightpoint != preSelected.end())
    selectionNextLevel(fc, rightpoint, preSelected.end(), result);
}

inline bool
RexFareSelectorStrategy::selectionNextLevel(FareCompInfo& fc,
                                            Iterator begin,
                                            Iterator end,
                                            std::vector<PaxTypeFareWrapper>& result) const
{
  Iterator mid = stablePartition(begin, end, SecurityCheckWrp());
  return (((begin != mid) && select(fc, begin, mid, result)) ||
          ((mid != end) && select(fc, mid, end, result)));
}

double
RexFareSelectorStrategy::getTolerance() const
{
  static const double factor[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9 };
  return 1.0 / factor[_trx.exchangeItin().front()->calculationCurrencyNoDec()];
}

} // tse
