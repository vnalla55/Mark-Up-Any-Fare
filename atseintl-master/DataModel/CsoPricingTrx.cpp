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
//---------------------------------------------------------------------------

#include "DataModel/CsoPricingTrx.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
static Logger
logger("atseintl.RexPricing.CsoPricingTrx");

bool
CsoPricingTrx::initialize(RexPricingTrx& trx, bool forDiagnostic)
{
  _parentTrx = &trx;
  _transactionStartTime = trx.transactionStartTime();
  TrxUtil::createTrxAborter(*this);

  Itin* itin = nullptr;
  _dataHandle.get(itin);
  _dataHandle.get(_request);
  _dataHandle.get(_options);

  if (!_request || !_options || !itin)
    return false;

  setPbbRequest(trx.isPbbRequest());

  _request->assign(*trx.getRequest());
  _options->assign(*trx.getOptions());

  _options->baseFareCurrencyOverride() = EMPTY_STRING();
  _options->mOverride() = 0;
  _options->currencyOverride() = EMPTY_STRING();
  _request->roeOverride() = 0.0;
  _request->lowFareRequested() = 'Y';
  _billing = trx.billing();
  _bookingDate = trx.currentTicketingDT(); // D07
  _request->ticketingDT() = trx.currentTicketingDT();
  _paxType = trx.paxType();
  _posPaxType = trx.posPaxType();
  _itin.push_back(itin);

  if (forDiagnostic)
    _redirectedDiagnostic = &trx.diagnostic();

  itin->calcCurrencyOverride() = EMPTY_STRING();

  cloneTravelSegments(trx.itin().front()->travelSeg());

  itin->setTravelDate(TseUtil::getTravelDate(itin->travelSeg()));
  _travelDate = itin->travelDate();

  _travelSeg = itin->travelSeg();

  clonePlusUpPricing(*trx.itin().front());
  _mcpCarrierSwap = trx.mcpCarrierSwap();
  _dynamicCfg = trx.dynamicCfg();
#ifdef CONFIG_HIERARCHY_REFACTOR
  _configBundle = trx.configBundle();
#endif
  _dynamicCfgOverriden = trx.isDynamicCfgOverriden();
  setValidatingCxrGsaApplicable(trx.isValidatingCxrGsaApplicable());
  countrySettlementPlanInfo() = trx.countrySettlementPlanInfo();
  setIataFareSelectionApplicable( trx.isIataFareSelectionApplicable() );

  return true;
}

namespace
{

struct IsValid
{
  bool operator()(const TravelSeg* seg) const
  {
    return seg->unflown() && seg->segmentType() != Arunk;
  }
};

struct SameFareMarket
{
  SameFareMarket(const FareMarket& fm) : _fm(fm) {}
  bool operator()(const FareMarket* fm) const { return fm->travelSeg() == _fm.travelSeg(); }

private:
  const FareMarket& _fm;
};
}

void
CsoPricingTrx::cloneTravelSegments(const std::vector<TravelSeg*>& rexTvlSegs)
{
  std::vector<TravelSeg*>::const_iterator first =
      std::find_if(rexTvlSegs.begin(), rexTvlSegs.end(), IsValid());

  std::copy(first, rexTvlSegs.end(), std::back_inserter(_itin.front()->travelSeg()));

  if (_itin.front()->travelSeg().empty())
  {
    LOG4CXX_FATAL(logger, "Invalid itinerary for cancel and start over pricing");
    throw ErrorResponseException(ErrorResponseException::NO_ITIN_SEGS_FOUND,
                                 "INVALID ITINERARY FOR CANCEL AND START OVER PRICING");
  }
}

void
CsoPricingTrx::cloneClassOfService()
{
  std::vector<FareMarket*>& csoFm = _itin.front()->fareMarket();
  const std::vector<FareMarket*>& rexFm =
      static_cast<const RexPricingTrx&>(*_parentTrx).newItin().front()->fareMarket();

  for (const auto elem : csoFm)
  {
    std::vector<FareMarket*>::const_iterator curr =
        std::find_if(rexFm.begin(), rexFm.end(), SameFareMarket(*elem));
    if (curr != rexFm.end())
    {
      elem->classOfServiceVec() = (*curr)->classOfServiceVec();
    }
    else
    {
      LOG4CXX_FATAL(logger, "Invalid fare market for cancel and start over pricing");
      throw ErrorResponseException(ErrorResponseException::NO_ITIN_SEGS_FOUND,
                                   "INVALID FARE MARKET FOR CANCEL AND START OVER PRICING");
    }
  }
}

void
CsoPricingTrx::clonePlusUpPricing(const Itin& itin)
{
  if (!itin.isPlusUpPricing())
    return;

  ConsolidatorPlusUp* plusUp;
  _dataHandle.get(plusUp);
  _itin.front()->consolidatorPlusUp() = plusUp;

  const ConsolidatorPlusUp& src = *itin.consolidatorPlusUp();

  plusUp->initialize(*this, src.amount(), src.currencyCode(), src.tktDesignator());
}

} // end of namespace tse
