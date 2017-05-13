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
#include "DataModel/ExchangePricingTrx.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PaxType.h"
#include "DataModel/RexBaseTrx.h"
#include "Rules/RuleConst.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

namespace tse
{
using namespace std;

static Logger
logger("atseintl.DataModel.ExchangePricingTrx");

static const std::string ME_RESPONSE_OPEN_TAG = "<RexPricingResponse S96=\"ME\">";
static const std::string ME_RESPONSE_CLOSE_TAG = "</RexPricingResponse>";

ExchangePricingTrx::ExchangePricingTrx()
{
  _excTrxType = PricingTrx::PORT_EXC_TRX;
  _dataHandle.isPortExchange() = true;
}

bool
ExchangePricingTrx::initialize(RexBaseTrx& trx, bool forDiagnostic)
{
  _parentTrx = &trx;
  _transactionStartTime = trx.transactionStartTime();
  TrxUtil::createTrxAborter(*this);

  _dataHandle.get(_request);
  _dataHandle.get(_options);

  if (!_request || !_options)
    return false;

  _reqType = trx.secondaryExcReqType();

  // PrincingTrx
  _request->assign(*trx.getRequest());
  _options->assign(*trx.getOptions());
  _billing = trx.billing();
  _request->lowFareRequested() = 'N';
  _request->ticketingDT() = trx.currentTicketingDT();
  _itin = trx.newItin();
  trx.initalizeForRedirect(_fareMarket, _travelSeg);
  _paxType = trx.PricingTrx::paxType();
  _itin.front()->calculationCurrency() = "";
  _dynamicCfgOverriden = trx.isDynamicCfgOverriden();
  _dynamicCfg = trx.dynamicCfg();
#ifdef CONFIG_HIERARCHY_REFACTOR
  _configBundle = trx.configBundle();
#endif
  setValidatingCxrGsaApplicable(trx.isValidatingCxrGsaApplicable());
  countrySettlementPlanInfo() = trx.countrySettlementPlanInfo();
  setIataFareSelectionApplicable( trx.isIataFareSelectionApplicable() );

  // BaseExchangeTrx
  _originalTktIssueDT = trx.originalTktIssueDT();
  _lastTktReIssueDT = DateTime::emptyDate();
  _currentTicketingDT = trx.currentTicketingDT();
  _previousExchangeDT = trx.previousExchangeDT();
  _exchangeItin = trx.exchangeItin();
  _accompanyPaxType = trx.accompanyPaxType();
  _reissueLocation = trx.reissueLocation();
  _exchangeItin.front()->calculationCurrency() = _exchangeItin.front()->calcCurrencyOverride();
  _rexPrimaryProcessType = trx.rexSecondaryProcessType();
  _rexSecondaryProcessType = RuleConst::BLANK;

  initializeOverrides(trx);

  // EchangePricingTrx
  if (!trx.purchaseDT().isValid())
    _purchaseDT = trx.currentTicketingDT();
  else
    _purchaseDT = trx.purchaseDT();

  _request->ticketingDT() = _purchaseDT;
  _dataHandle.setTicketDate(_purchaseDT);

  if (forDiagnostic)
    _redirectedDiagnostic = &trx.diagnostic();
  _mcpCarrierSwap = trx.mcpCarrierSwap();

  return true;
}

void
ExchangePricingTrx::initializeOverrides(RexBaseTrx& trx)
{
  _exchangeOverrides.differentialOverride() = trx.exchangeOverrides().differentialOverride();
  _exchangeOverrides.dummyFareMileTktCity() = trx.exchangeOverrides().dummyFareMileTktCity();
  _exchangeOverrides.surchargeOverride() = trx.exchangeOverrides().surchargeOverride();
  _exchangeOverrides.stopoverOverride() = trx.exchangeOverrides().stopoverOverride();
  _exchangeOverrides.journeySTOOverrideCnt() = trx.exchangeOverrides().journeySTOOverrideCnt();
  _exchangeOverrides.plusUpOverride() = trx.exchangeOverrides().plusUpOverride();
  _exchangeOverrides.mileageTypeData() = trx.exchangeOverrides().mileageTypeData();
  _exchangeOverrides.dummyFCSegs() = trx.exchangeOverrides().dummyFCSegs();
  _exchangeOverrides.dummyFareMiles() = trx.exchangeOverrides().dummyFareMiles();
  _exchangeOverrides.dummyFareMileCity() = trx.exchangeOverrides().dummyFareMileCity();
  _exchangeOverrides.forcedSideTrip() = trx.exchangeOverrides().forcedSideTrip();
}

const DateTime&
ExchangePricingTrx::adjustedTravelDate(const DateTime& travelDate) const
{
  return std::max(_dataHandle.ticketDate(), travelDate);
}

const DateTime&
ExchangePricingTrx::travelDate() const
{
  return adjustedTravelDate(_travelDate);
}

void
ExchangePricingTrx::applyCurrentBsrRoeDate()
{
  _request->ticketingDT() = _currentTicketingDT;
  _dataHandle.setTicketDate(_currentTicketingDT);
}

void
ExchangePricingTrx::applyHistoricalBsrRoeDate()
{
  _request->ticketingDT() = _historicalBsrRoeDate;
  _dataHandle.setTicketDate(_historicalBsrRoeDate);
}

void
ExchangePricingTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string tmpResponse(ere.message());
  if (ere.code() > 0 && ere.message().empty())
  {
    tmpResponse = "UNKNOWN EXCEPTION";
  }
  bool isMERsp = false;
  if (excTrxType() == PricingTrx::NEW_WITHIN_ME)
  {
    isMERsp = true;
    tmpResponse += " NEW";
  }
  else if (excTrxType() == PricingTrx::EXC1_WITHIN_ME)
  {
    isMERsp = true;
    tmpResponse += " EX1";
  }
  else if (excTrxType() == PricingTrx::EXC2_WITHIN_ME)
  {
    isMERsp = true;
    tmpResponse += " EX2";
  }
  if (isMERsp)
    response = ME_RESPONSE_OPEN_TAG;

  PricingResponseFormatter formatter;
  response += formatter.formatResponse(tmpResponse, false, *this, nullptr, ere.code());
  if (isMERsp)
    response += ME_RESPONSE_CLOSE_TAG;
}

bool
ExchangePricingTrx::convert(std::string& response)
{
  std::string tmpResponse;
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  char tmpBuf[512];
  FareCalcCollector* fareCalcCollector = nullptr;
  Diagnostic& diag = diagnostic();
  tmpResponse = diag.toString();
  if (diag.diagnosticType() != DiagnosticNone && tmpResponse.length() == 0)
  {
    sprintf(tmpBuf, "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
    tmpResponse.insert(0, tmpBuf);
  }

  if ((diag.diagnosticType() == DiagnosticNone || diag.diagnosticType() == Diagnostic855) &&
      !_fareCalcCollector.empty())
  {
    fareCalcCollector = _fareCalcCollector.front();
  }
  PricingResponseFormatter formatter;
  response = formatter.formatResponse(tmpResponse, displayOnly(), *this, fareCalcCollector);
  LOG4CXX_DEBUG(logger, "response: " << response);

  return true;
}
}
