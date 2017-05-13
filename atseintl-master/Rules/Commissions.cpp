//-------------------------------------------------------------------
//
//  File:        Commissions.cpp
//  Created:     Nov 05, 2004
//  Authors:     Vladimir Koliasnikov
//
//  Description:
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Rules/Commissions.h"

#include "Common/CurrencyRoundingUtil.h"
#include "Common/SecurityHandshakeValidator.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CommissionCap.h"
#include "DBAccess/NegFareRest.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag865Collector.h"
#include "Diagnostic/Diag866Collector.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/AgencyCommissions.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{

using namespace amc;

FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(fallbackCommissionManagement);
FALLBACK_DECL(fallbackCommissionCapRemoval);
FALLBACK_DECL(fallbackJira1908NoMarkup);
FALLBACK_DECL(fallbackBSPMt3For1S)

Commissions::Commissions(PricingTrx& trx) : _trx(trx)
{
  Diagnostic& trxDiag = _trx.diagnostic();

  if (trxDiag.diagnosticType() == Diagnostic865)
  {
    _diag865 = dynamic_cast<Diag865Collector*>(DCFactory::instance()->create(_trx));
    _diag865->enable(Diagnostic865);

    std::map<std::string, std::string>::const_iterator i = trxDiag.diagParamMap().find("ET");

    if (i != trxDiag.diagParamMap().end())
    {
      if (i->second.substr(0, 1) == "T")
      {
        _ticketingEntry = true;
      }
    }
  }
  else if (trxDiag.diagnosticType() == Diagnostic866)
  {
    _diag866 = dynamic_cast<Diag866Collector*>(DCFactory::instance()->create(_trx));
    _diag866->enable(Diagnostic866);
    _ticketingEntry = true;
  }
  else if (trxDiag.diagnosticType() == Diagnostic867)
  {
    _diag867 = dynamic_cast<Diag867Collector*>(DCFactory::instance()->create(_trx));
    _diag867->enable(Diagnostic867);
    _diag867->initTrx(_trx);
    _ticketingEntry = true;
  }

  _ticketingEntry = _trx.getRequest()->isTicketEntry();
  _paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  _commType = _trx.getRequest()->ticketingAgent()->agentCommissionType();
  _inputCommAmt = _trx.getRequest()->ticketingAgent()->commissionAmount();
  _inputCommPrc = _trx.getRequest()->ticketingAgent()->commissionPercent();
}

Commissions::~Commissions()
{
  if (UNLIKELY(_diag865))
  {
    _diag865->flushMsg();
  }
  if (UNLIKELY(_diag867))
    _diag867->flushMsg();
}

void
Commissions::getCommissions(FarePath& fpath,
                            const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
                            VCFMCommissionPrograms& vcfmCommProgs,
                            VCFMPTFCommissionRules& vcfmptfCommRules)
{
  if (!agencyCommissionsApplicable(fpath, csHsInfoCol))
  {
    getCommissions(fpath);
    return;
  }

  AgencyCommissions agentCommission(_trx, _diag867);
  fpath.isAgencyCommissionQualifies() = true;
  if(agentCommission.getAgencyCommissions(fpath, csHsInfoCol, vcfmCommProgs, vcfmptfCommRules))
    return;

  if(isTicketingAgentSpecifiedCommission())
    getCommissions(fpath);
}

bool
Commissions::agencyCommissionsApplicable(
    FarePath& fpath,
    const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol) const
{
  if(_trx.excTrxType() != PricingTrx::NOT_EXC_TRX ||
      _trx.noPNRPricing())
  {
    if (_diag867)
      _diag867->printAMCCommandNotSupported(fpath);
    return false;
  }

  if (!isRequestFromTravelAgent(fpath) ||
      isCat35Solution(fpath) ||
      isTicketingAgentSpecCommissionWithEPR(fpath) ||
      !isTrxApplicableForAgencyCommission(fpath) ||
      (!fallback::fallbackAMCPhase2(&_trx) ?
       csHsInfoCol.empty() : !securityHandShakeValid(fpath)))
    return false;

  return true;
}

void
Commissions::getCommissions(FarePath& fPath)
{
  // temporary code for the INOV iteration
  _baseFareCurrency = fPath.baseFareCurrency();
  _calculationCurrency = fPath.calculationCurrency();
  _useInternationalRounding = fPath.itin()->useInternationalRounding();
  _applyNonIATARounding = fPath.applyNonIATARounding(_trx);

  if (LIKELY(!_trx.getOptions()->currencyOverride().empty()))
    _paymentCurrency = _trx.getOptions()->currencyOverride();

  CollectedNegFareData* cNegFareData = fPath.collectedNegFareData();
  if (cNegFareData == nullptr || !cNegFareData->indicatorCat35())
  {
    // do regular commission
    if (!_ticketingEntry && !_trx.getOptions()->isCat35Net() &&
        !_trx.getRequest()->ticketingAgent()->abacusUser())
    {
      if (!_trx.getRequest()->ticketingAgent()->infiniUser())
        return;
    }

    // Calculate a commissions only for the ticketing entry and for Abacus user.
    getRegularCommissions(fPath);
  }
  else // do cat35 commission
  {
    getCat35Commissions(fPath, *cNegFareData);
  }
}

void
Commissions::calcNetRemitCommission()
{
  Diag865Collector::CommissionCalcMethod method = Diag865Collector::AMOUNT;

  if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
  {
    if (UNLIKELY(_diag865))
    {
      if (_indNetGross == RuleConst::NGI_NET_AMOUNT)
      {
        method = Diag865Collector::NET_TIMES_COMM_PCT;
      }
      else
      {
        method = Diag865Collector::SELL_TIMES_COMM_PCT;
      }
    }
    _commAmount = (_rulePercent * _amtForCalcComm) / 100.0;
  }
  else if (_ruleCommAmt == 0 && _commType.empty()) // comm. amount is 0 but no agent i/p
  {
    _commAmount = _ruleCommAmt;
  }
  else if (_ruleCommAmt != 0) // comm. amount is not 0
  {
    _commAmount = _ruleCommAmt;
  }
  else // or agent input
  {
    if (!_commType.empty())
    {
      if (_commType[0] == PERCENT_COMM_TYPE)
      {
        if (UNLIKELY(_diag865))
        {
          if (_indNetGross == RuleConst::NGI_NET_AMOUNT)
          {
            method = Diag865Collector::NET_TIMES_COMM_PCT;
          }
          else
          {
            method = Diag865Collector::SELL_TIMES_COMM_PCT;
          }
        }
        _commAmount = (_inputCommPrc * _amtForCalcComm) / 100.0;
      }
      else
      {
        _commAmount = _inputCommAmt;
      }
    }
  }

  if (_commAmount == 0)
    _commPercent = 0;
  else
    _commPercent = _rulePercent;

  if (UNLIKELY(_diag865))
  {
    _diag865->displayCommissionApplication(*this, _commAmount, method);
  }
}

void
Commissions::getRegularCommissions(FarePath& fPath)
{
  CarrierCode validCarrier = CarrierCode();

  if(_trx.isValidatingCxrGsaApplicable())
  {
    if(fPath.defaultValidatingCarrier().empty())
      return;
    else
      validCarrier = fPath.defaultValidatingCarrier();
  }
  else
    validCarrier = fPath.itin()->validatingCarrier();

  _totalSellAmt = fPath.getTotalNUCAmount(); // sum of FareAmounts, surcharges,stopovers,transfers
  _commAmount = 0;
  _commPercent = 0;

  _allOnValidCarrier = true; // all itinerary on validating carrier
  _anyOnValidCarrier = false; // at least one segment on Validating carrier
  _noValidCarrier = false; // there are no segments on Validating crx

  bool withinCA = false; // wholly within Canada

  SmallBitSet<uint16_t, Itin::TripCharacteristics>& tripChar = fPath.itin()->tripCharacteristics();

  std::vector<TravelSeg*>::iterator tsB = fPath.itin()->travelSeg().begin();
  std::vector<TravelSeg*>::iterator tsE = fPath.itin()->travelSeg().end();

  for (; tsB != tsE; ++tsB)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*tsB);
    if (airSeg != nullptr)
    {
      if (airSeg->carrier() == validCarrier)
        _anyOnValidCarrier = true;
      else
        _allOnValidCarrier = false;
    }
  }
  if (!_allOnValidCarrier && !_anyOnValidCarrier)
    _noValidCarrier = true;

  const DateTime& travelDate = fPath.itin()->travelSeg().front()->departureDT();

  if (UNLIKELY(_diag866))
    _diag866->diag866Request(_trx, fPath, *this);

  const std::string& cType = _trx.getRequest()->ticketingAgent()->agentCommissionType();
  Percent inputPrc = _trx.getRequest()->ticketingAgent()->commissionPercent();

  if (!cType.empty()) // input agent commission
  {
    if (cType[0] == PERCENT_COMM_TYPE)
    {
      CurrencyConversionFacade ccFacade;
      if (fPath.calculationCurrency() != _baseFareCurrency)
      {
        Money targetMoney(_baseFareCurrency);
        targetMoney.value() = 0;
        Money sourceMoneyCalculation(_totalSellAmt, fPath.calculationCurrency());
        if (ccFacade.convert(targetMoney,
                             sourceMoneyCalculation,
                             _trx,
                             fPath.itin()->useInternationalRounding()))
        {
          _totalSellAmt = targetMoney.value();
        }
        else
          _totalSellAmt = 0;
      }
      if (_baseFareCurrency != _paymentCurrency)
      {
        Money targetMoney(_paymentCurrency);
        targetMoney.value() = 0;
        Money sourceMoneyCalculation(_totalSellAmt, _baseFareCurrency);
        if (ccFacade.convert(targetMoney,
                             sourceMoneyCalculation,
                             _trx,
                             fPath.itin()->useInternationalRounding()))
        {
          _totalSellAmt = targetMoney.value();
        }
        else
          _totalSellAmt = 0;
      }
      _commAmount = (inputPrc * _totalSellAmt) / 100.0;
    }
    else
      _commAmount = _trx.getRequest()->ticketingAgent()->commissionAmount();

    fPath.commissionAmount() = _commAmount;
    fPath.commissionPercent() = _commPercent;

    if (UNLIKELY(_diag866))
    {
      _diag866->diag866Commission(_trx, fPath, *this);
      _diag866->flushMsg();
    }
    return;
  }

  if (fallback::fallbackCommissionCapRemoval(&_trx))
  {
  // retrieve a vector of CommissionCap records
  const std::vector<CommissionCap*>& ccList =
      _trx.dataHandle().getCommissionCap(validCarrier, _paymentCurrency, travelDate);

  std::vector<CommissionCap*>::const_iterator bIt = ccList.begin();
  std::vector<CommissionCap*>::const_iterator eIt = ccList.end();

  bool commissionCap = false;

  for (; bIt != eIt; ++bIt)
  {

    if ((*bIt)->valCarrierTvl() == ALL_SEG_ON_VAL_CARRIER && !allOnValidCarrier())
      continue;
    if ((*bIt)->valCarrierTvl() == AT_LEAST_ONE_SEG_ON_VAL_CARRIER && !anyOnValidCarrier())
      continue;
    if ((*bIt)->valCarrierTvl() == NO_SEG_ON_VAL_CARRIER && !noValidCarrier())
      continue;
    if ((*bIt)->transBorder() == NO && fPath.itin()->geoTravelType() == GeoTravelType::Transborder)
      continue;
    if ((*bIt)->canada() == NO && withinCA)
      continue;
    if ((*bIt)->domestic() == NO && fPath.itin()->geoTravelType() == GeoTravelType::Domestic)
      continue;
    if ((*bIt)->international() == NO && fPath.itin()->geoTravelType() == GeoTravelType::International)
      continue;

    if ((*bIt)->owRt() == ONE_WAY && !tripChar.isSet(Itin::OneWay))
      continue;
    if ((*bIt)->owRt() == ROUND_TRIP && !tripChar.isSet(Itin::RoundTrip))
      continue;

    commissionCap = true;
    calculateCommissions(fPath, (**bIt));

    if (UNLIKELY(_diag866))
    {
       _diag866->diag866DisplayComCap(_trx, (**bIt));
    }
    break;
  }

  if (UNLIKELY(_diag866 && !commissionCap))
  {
    *_diag866 << " COMMISSION CAP DATA             : NOT FOUND" << std::endl;
  }

  fPath.commissionAmount() = _commAmount;
  fPath.commissionPercent() = _commPercent;

  if (UNLIKELY(_diag866))
  {
     _diag866->diag866Commission(_trx, fPath, *this);
     _diag866->flushMsg();
  }
  }
}

void
Commissions::calculateCommissions(const FarePath& fPath, const CommissionCap& cCap)
{
  if (cCap.amtType() == PERCENT_COMM_TYPE)
  {
    _commPercent = cCap.amt();
    _commAmount = (_commPercent * _totalSellAmt) / 100.0;

    CurrencyConversionFacade ccFacade;

    if (_calculationCurrency != _paymentCurrency)
    {
      _commAmount = convertCurrency(_totalSellAmt, _calculationCurrency, _paymentCurrency);
    }
    if (_commAmount != 0)
    {
      CurrencyRoundingUtil cru;
      Money target(_commAmount, _paymentCurrency);
      if (cru.round(target, _trx))
        _commAmount = target.value();
    }
  }
  else
    _commAmount = cCap.amt();

  // check a commission amount for MAX
  MoneyAmount amt = 0.0;
  if (cCap.maxAmtType() == PERCENT_COMM_TYPE)
  {
    amt = (cCap.maxAmt() * _totalSellAmt) / 100.0;
    if (_commAmount > amt)
      _commAmount = amt;
  }
  else if (cCap.maxAmtType() == FLAT_AMOUNT_TYPE)
  {
    if (_commAmount > cCap.maxAmt())
      _commAmount = cCap.maxAmt();
  }
  // check a commission amount for MIN
  if (cCap.minAmtType() == PERCENT_COMM_TYPE)
  {
    amt = (cCap.minAmt() * _totalSellAmt) / 100.0;
    if (_commAmount > amt)
      _commAmount = amt;
  }
  else if (cCap.minAmtType() == FLAT_AMOUNT_TYPE)
  {
    if (_commAmount < cCap.minAmt())
      _commAmount = cCap.minAmt();
  }
}

void
Commissions::calculateWPCat35Commission(CollectedNegFareData& cNegFareData)
{
  Diag865Collector::CommissionCalcMethod method = Diag865Collector::AMOUNT;

  // default Allow PSS( ticketing) to use input commissions
  // if pct commissions = blank (999.9999) and amt Commission =0
  _commPercent = _rulePercent;
  _commAmount = _ruleCommAmt;

  // ONLY T or L with Net or C with Carrier Specified that have an exception of Commission
  // Calculation
  if (cNegFareData.fareTypeCode() == RuleConst::NET_SUBMIT_FARE || // Type T
      cNegFareData.fareTypeCode() ==
          RuleConst::SELLING_FARE_NOT_FOR_SEC || // Type L with Net in 979 - (N)
      (cNegFareData.fareTypeCode() == RuleConst::NET_SUBMIT_FARE_UPD &&
       cNegFareData.cat35CSelling())) // Type C with 979 Fare Ind A/C/M/S
  {
    if (_markUpCommission) // Selling - Net != 0
    {
      if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
      {
        if (UNLIKELY(_diag865))
        {
          method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
        }
        _commAmount = _markUpCommission + (_rulePercent * _totalSellAmt) / 100.0;
      }
      else if ((_ruleCommAmt != 0) || // comm. amount is not 0
               (_commType.empty() && _ruleCommAmt == 0 &&
                _rulePercent == RuleConst::PERCENT_NO_APPL))
      {
        if (UNLIKELY(_diag865))
        {
          method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
        }
        _commAmount = _markUpCommission + _ruleCommAmt;
      }
      else // or agent input
      {
        if (!_commType.empty())
        {
          if (_commType[0] == PERCENT_COMM_TYPE) // agent i/p is %
          {
            if (UNLIKELY(_diag865))
            {
              method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
            }
            _commAmount = _markUpCommission + (_inputCommPrc * _totalSellAmt) / 100.0;
          }
          else // agent i/p is amt
          {
            if (UNLIKELY(_diag865))
            {
              method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
            }
            _commAmount = _markUpCommission + _inputCommAmt;
          }
        }
      }
    }
    else // Selling = Net
    {
      if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
      {
        _commPercent = _rulePercent;
      }
      else if ((_ruleCommAmt != 0) || // comm. amount is not 0
               (_commType.empty() && _ruleCommAmt == 0 &&
                _rulePercent == RuleConst::PERCENT_NO_APPL))
      {
        if (UNLIKELY(_diag865))
        {
          method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
        }
        _commAmount = _markUpCommission + _ruleCommAmt;
      }
      else if (!_commType.empty() && _commType[0] != PERCENT_COMM_TYPE) // agent i/p is amt
      {
        if (UNLIKELY(_diag865))
        {
          method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
        }
        _commAmount = _markUpCommission + _inputCommAmt;
      }
    }
  } // Type T/L with Net/C with Net

  if (_rulePercent == RuleConst::PERCENT_NO_APPL &&
      _ruleCommAmt == 0 &&
      _commType.empty())
  {
    _commBaseAmount += _totalSellAmt; // collect commission base amount
  }

  if (UNLIKELY(_diag865))
  {
    _diag865->displayCommissionApplication(*this, _commAmount, method);
  }
}

void
Commissions::getCat35Commissions(FarePath& fPath, CollectedNegFareData& cNegFareData)
{
  _ruleCommAmt = cNegFareData.comAmount(); // cat35 selected currency was converted to BaseFare curr
  _rulePercent = cNegFareData.comPercent();
  _totalNetAmt = cNegFareData.netTotalAmt(); // in calculation currency
  _totalSellAmt = cNegFareData.totalSellingAmt(); // in calculation currency
  _markUpCommission = (_totalSellAmt - _totalNetAmt); // in calculation currency
  cNegFareData.netTotalAmtCharges() =
      cNegFareData.netTotalAmt(); // populate total in NUC before convertion
  _commAmount = 0;
  _commPercent = 0;
  _commBaseAmount = 0.0;
  _markUpAmount = 0.0;

  if (UNLIKELY(_diag865))
  {
    _diag865->displayRequestData(_trx, fPath, *this, cNegFareData);
  }

  if (((cNegFareData.totalSellingAmt() - cNegFareData.netTotalAmt()) < 0) &&
      !cNegFareData.shouldUseRedistributedWholeSale())
  {
    if (_rulePercent == RuleConst::PERCENT_NO_APPL &&
        _trx.getRequest()->ticketingAgent()->infiniUser())
      fPath.commissionPercent() = _rulePercent;
    else
      fPath.commissionPercent() = 0;

    fPath.commissionAmount() = 0;

    if (UNLIKELY(_diag865))
    {
      _diag865->displayFarePathHeader(fPath);
      _diag865->displayFailInfo();
    }
    return;
  }

  checkAgentInputCommission(cNegFareData);

  NegotiatedFareRuleUtil nfru;
  if (TrxUtil::isCat35TFSFEnabled(_trx) && nfru.isNetTicketingWithItBtData(_trx, fPath))
  {
    _isNetTktingWithItBtFare = true;
    getNetTicketingCommission(fPath);
  }
  else if (_trx.getRequest()->ticketingAgent()->abacusUser() ||
           _trx.getRequest()->ticketingAgent()->infiniUser() ||
           (cNegFareData.bspMethod() == RuleConst::NRR_METHOD_2 &&
            TrxUtil::isNetRemitEnabled(_trx)) ||
           (!fallback::fallbackBSPMt3For1S(&_trx) &&
            cNegFareData.bspMethod() == RuleConst::NRR_METHOD_3 &&
            !_trx.getRequest()->ticketingAgent()->axessUser()))
  {

    if (!_ticketingEntry && _ruleCommAmt == 0 && _rulePercent == RuleConst::PERCENT_NO_APPL)
    {
      // do not calculate commissions, just send back to PSS all commissions info.
      // (per PSS_TKT_TFR_group request)
      _commAmount = _ruleCommAmt;
      _commPercent = _rulePercent;

      if (UNLIKELY(_diag865))
      {
        if ((cNegFareData.bspMethod() == RuleConst::NRR_METHOD_1 &&
             _trx.getRequest()->ticketingAgent()->infiniUser()) ||
            cNegFareData.bspMethod() == RuleConst::NRR_METHOD_2 ||
            cNegFareData.bspMethod() == RuleConst::NRR_METHOD_3)
        {
          getNetRemitCommission(fPath, cNegFareData, false);
        }
        else
        {
          getNetTicketingCommission(fPath, false);
        }
      }
      else
      {
        // Fix for SSDMLT-1908
        // Keep Abacus logic to not calculate commission but populate C57 for now.
        // The actual commission calculation may be impacted by the
        // 'Standardize CAT35 Commission Requirement' project
        if (!fallback::fallbackJira1908NoMarkup(&_trx) &&
            cNegFareData.bspMethod() == RuleConst::NRR_METHOD_BLANK)
        {
          getCat35TFSFCommission(fPath, cNegFareData, false);
        }
      }
    }
    else if ((cNegFareData.bspMethod() == RuleConst::NRR_METHOD_1 &&
              _trx.getRequest()->ticketingAgent()->infiniUser()) ||
             cNegFareData.bspMethod() == RuleConst::NRR_METHOD_2 ||
             cNegFareData.bspMethod() == RuleConst::NRR_METHOD_3)
    {
      getNetRemitCommission(fPath, cNegFareData);
    }
    else
    {
      getNetTicketingCommission(fPath);
    }
  }
  else
  {
    if (!_ticketingEntry)
    {
      getCat35TFSFCommission(fPath, cNegFareData);
    }
    else
    {
      getNetTicketingCommission(fPath);
    }
  }

  if (_calculationCurrency != _baseFareCurrency)
  {
    cNegFareData.netTotalAmt() = convertCurrency(
        cNegFareData.netTotalAmt(), _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
  }

  if (_baseFareCurrency != _paymentCurrency)
  {
    cNegFareData.netTotalAmt() =
        convertCurrency(cNegFareData.netTotalAmt(), _baseFareCurrency, _paymentCurrency);
  }

  if (_errorCommission)
  {
    cNegFareData.numberWarningMsg() = 174;
  }

  if (_commAmount != 0)
  {
    CurrencyConverter cc;
    Money target(_commAmount, _paymentCurrency);
    cc.roundNone(target, 0);
    _commAmount = target.value();
  }

  if (_netCommAmount != 0)
  {
    CurrencyConverter cc;
    Money target(_netCommAmount, _paymentCurrency);
    cc.roundNone(target, 0);
    _netCommAmount = target.value();
  }

  // in paymentCurrency
  fPath.commissionAmount() = _commAmount;
  fPath.commissionPercent() = _commPercent;
  cNegFareData.netComAmount() = _netCommAmount;
  cNegFareData.netComPercent() = _netCommPercent;
  cNegFareData.commissionBaseAmount() = _commBaseAmount;
  if (!fallback::fallbackCommissionManagement(&_trx))
  {
    cNegFareData.markUpAmount() = _markUpAmount;
  }

  if (UNLIKELY(_diag865))
  {
    _diag865->displayFinalCommission(*this);
    _diag865->flushMsg();
  }
}

void
Commissions::checkAgentInputCommission(CollectedNegFareData& cNegFareData)
{
  if (!_commType.empty())
  {
    if (isNetRemitAllowed(cNegFareData))
    {
      if (_rulePercent != RuleConst::PERCENT_NO_APPL || _ruleCommAmt != 0)
      {
        // there is an error when the agent input commissions
        // and cat35 contains commissions for Net Remit.
        _errorCommission = true; // send warning MSG

        // send a traler MSG for any WP/W' entry type
        cNegFareData.trailerMsg() = "UNABLE TO OVERRIDE NEGOTIATED DATA FOR COMMISSION";
      }
    }
    else
    {
      if (_commType[0] == PERCENT_COMM_TYPE) // agent input a commission %
      {
        if (_rulePercent != RuleConst::PERCENT_NO_APPL)
        {
          if (_inputCommPrc != _rulePercent)
          {
            _errorCommission = true;
          }
        }
        if (_ruleCommAmt != 0)
        {
          _errorCommission = true;
        }
      }
      else // agent input a commission amount
      {
        if (_ruleCommAmt != 0)
        {
          if (_inputCommAmt != _ruleCommAmt)
          {
            _errorCommission = true;
          }
        }
        if (_rulePercent != RuleConst::PERCENT_NO_APPL)
        {
          _errorCommission = true;
        }
      }
    }
  }
}

void
Commissions::getNetRemitCommission(FarePath& fPath,
                                   CollectedNegFareData& cNegFareData,
                                   bool calculate)
{
  MoneyAmount surchargeTotalAmt = cNegFareData.cat8SurchargeTotalAmt() // Cat 8 charges
                                  + cNegFareData.cat9SurchargeTotalAmt() // Cat 9 charges
                                  + cNegFareData.cat12SurchargeTotalAmt(); // Cat 12 charges
  _totalNetAmt += cNegFareData.totalMileageCharges(); // Mileage surcharge
  _totalNetAmt += surchargeTotalAmt;
  _totalSellAmt += cNegFareData.totalSellingMileageCharges(); // Mileage surcharge
  _totalSellAmt += surchargeTotalAmt;

  _indNetGross = cNegFareData.indNetGross();

  if (_indNetGross == RuleConst::NGI_NET_AMOUNT)
    _amtForCalcComm = _totalNetAmt; // Total net amount with charges
  else
  {
    bool grossAmountUsed = false;

    if (_indNetGross == RuleConst::NGI_GROSS_AMOUNT || isCalculateCommFromTFD(fPath, cNegFareData))
    {
      if (fPath.selectedNetRemitFareCombo())
      {
        NetRemitFarePath* netRemitFp = fPath.netRemitFarePath();

        if (netRemitFp != nullptr)
        {
          _amtForCalcComm = netRemitFp->getTotalNUCAmount();
          grossAmountUsed = true;
        }
      }
    }

    if (!grossAmountUsed)
      _amtForCalcComm = _totalSellAmt; // Total sell amount with charges
  }

  if (_calculationCurrency != _baseFareCurrency)
  {
    _amtForCalcComm = convertCurrency(
        _amtForCalcComm, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
    _totalSellAmt = convertCurrency(
        _totalSellAmt, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
    _totalNetAmt = convertCurrency(
        _totalNetAmt, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
    _markUpCommission = (_totalSellAmt - _totalNetAmt);
  }

  if (_baseFareCurrency != _paymentCurrency)
  {
    _amtForCalcComm = convertCurrency(_amtForCalcComm, _baseFareCurrency, _paymentCurrency);
    _totalSellAmt = convertCurrency(_totalSellAmt, _baseFareCurrency, _paymentCurrency);
    _totalNetAmt = convertCurrency(_totalNetAmt, _baseFareCurrency, _paymentCurrency);
    _markUpCommission = (_totalSellAmt - _totalNetAmt);
  }

  if (UNLIKELY(_diag865))
  {
    _diag865->displayFarePathHeader(fPath);
    _diag865->displayCommissionData(*this, fPath);
  }

  if (calculate)
  {
    calcNetRemitCommission();
  }
  else if (UNLIKELY(_diag865))
  {
    _diag865->displayCommissionApplication(*this);
  }
}

void
Commissions::getCat35TFSFCommission(FarePath& fPath,
                                    CollectedNegFareData& cNegFareData,
                                    bool calculate)
{
  MoneyAmount surchargeTotalAmt = cNegFareData.cat8SurchargeTotalAmt() // Cat 8 charges
                                  + cNegFareData.cat9SurchargeTotalAmt() // Cat 9 charges
                                  + cNegFareData.cat12SurchargeTotalAmt(); // Cat 12 charges
  _totalNetAmt += cNegFareData.totalMileageCharges(); // Mileage surcharge
  _totalNetAmt += surchargeTotalAmt;
  _totalSellAmt += cNegFareData.totalSellingMileageCharges(); // Mileage surcharge
  _totalSellAmt += surchargeTotalAmt;

  if (_calculationCurrency != _baseFareCurrency)
  {
    _totalSellAmt = convertCurrency(
        _totalSellAmt, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
    _totalNetAmt = convertCurrency(
        _totalNetAmt, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
    _markUpCommission = (_totalSellAmt - _totalNetAmt);
  }

  if (_baseFareCurrency != _paymentCurrency)
  {
    _totalSellAmt = convertCurrency(_totalSellAmt, _baseFareCurrency, _paymentCurrency);
    _totalNetAmt = convertCurrency(_totalNetAmt, _baseFareCurrency, _paymentCurrency);
    _markUpCommission = (_totalSellAmt - _totalNetAmt);
  }

  if (!fallback::fallbackCommissionManagement(&_trx))
  {
    _markUpAmount = _markUpCommission;
  }

  if (UNLIKELY(_diag865))
  {
    _diag865->displayFarePathHeader(fPath);
    _diag865->displayCommissionData(*this, fPath);
  }

  if (calculate)
    calculateWPCat35Commission(cNegFareData); // Cat35TFSF Commission
}

void
Commissions::getNetTicketingCommission(FarePath& fPath,
                                       bool calculate)
{
  bool netAmtWasFlipped = false;
  MoneyAmount netTotalAmt = 0.0;
  MoneyAmount sellingComm = 0.0;
  MoneyAmount netComm = 0.0;

  NegotiatedFareRuleUtil nfru;
  std::vector<PricingUnit*>::const_iterator puIt = fPath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = fPath.pricingUnit().end();

  if (UNLIKELY(_diag865))
  {
    _diag865->displayFareComponentHeader();
  }

  for (; puIt != puItEnd; ++puIt)
  {
    PricingUnit& pricingUnit = **puIt;
    std::vector<FareUsage*>::iterator fareUsageI = pricingUnit.fareUsage().begin();
    std::vector<FareUsage*>::iterator fareUsageEnd = pricingUnit.fareUsage().end();

    for (; fareUsageI != fareUsageEnd; ++fareUsageI)
    {
      FareUsage* fu = *fareUsageI;
      PaxTypeFare* ptf = fu->paxTypeFare();

      if (!ptf->isNegotiated())
      {
        netTotalAmt += fu->totalFareAmount();
        continue;
      }

      NegotiatedFareRuleUtil nfru;
      NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;

      const NegFareRest* negFareRest = nfru.getCat35Record3(ptf, negPaxTypeFare);

      if (!negFareRest || !negPaxTypeFare)
      {
        netTotalAmt += fu->totalFareAmount();
        continue;
      }

      if (calculate)
      {
        getRuleCommission(ptf, negFareRest, negPaxTypeFare);
      }

      if (ptf->mileageSurchargePctg() > 0)
      {
        _netMileageSurchargeAmt = (negPaxTypeFare->nucNetAmount() * ptf->mileageSurchargePctg()) /
                                  RuleConst::HUNDRED_PERCENTS;
        CurrencyUtil::truncateNUCAmount(_netMileageSurchargeAmt);
      }
      else
      {
        _netMileageSurchargeAmt = 0;
      }

      _totalSellAmt = ptf->nucFareAmount(); // in calculation currency
      _totalSellAmt += ptf->mileageSurchargeAmt();

      if (fPath.isRollBackSurcharges())
      {
        _totalSellAmt += fu->stopOverAmtUnconverted();
        _totalSellAmt += fu->transferAmtUnconverted();
        _totalSellAmt += fu->surchargeAmtUnconverted();
      }
      else
      {
        _totalSellAmt += fu->stopOverAmt();
        _totalSellAmt += fu->transferAmt();
        _totalSellAmt += fu->surchargeAmt();
      }

      if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
          negFareRest->negFareCalcTblItemNo() == 0) // L w/o 979
      {
        _totalNetAmt = _totalSellAmt; // Or no need to build _totalNetAmt ???
      }
      else
      {
        _totalNetAmt = negPaxTypeFare->nucNetAmount(); // in calculation currency
        _totalNetAmt += _netMileageSurchargeAmt;
        if (fPath.isRollBackSurcharges())
        {
          _totalNetAmt += fu->stopOverAmtUnconverted();
          _totalNetAmt += fu->transferAmtUnconverted();
          _totalNetAmt += fu->surchargeAmtUnconverted();
        }
        else
        {
          _totalNetAmt += fu->stopOverAmt();
          _totalNetAmt += fu->transferAmt();
          _totalNetAmt += fu->surchargeAmt();
        }
      }

      if (UNLIKELY(_diag865))
      {
        _diag865->totalNetAmtCalCurr() = _totalNetAmt;
        _diag865->totalSellAmtCalCurr() = _totalSellAmt;
      }

      _markUpCommission = (_totalSellAmt - _totalNetAmt); // in calculation currency
      if (_calculationCurrency != _baseFareCurrency)
      {
        _totalSellAmt = convertCurrency(
            _totalSellAmt, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
        _totalNetAmt = convertCurrency(
            _totalNetAmt, _calculationCurrency, _baseFareCurrency, _applyNonIATARounding);
        _markUpCommission = (_totalSellAmt - _totalNetAmt);
      }

      if (_baseFareCurrency != _paymentCurrency)
      {
        _totalSellAmt = convertCurrency(_totalSellAmt, _baseFareCurrency, _paymentCurrency);
        _totalNetAmt = convertCurrency(_totalNetAmt, _baseFareCurrency, _paymentCurrency);
        _markUpCommission = (_totalSellAmt - _totalNetAmt);
      }

      if (!fallback::fallbackCommissionManagement(&_trx))
      {
        _markUpAmount += _markUpCommission;
      }

      if (UNLIKELY(_diag865))
      {
        _diag865->displayCommissionData(*this, fu, fPath, negFareRest, negPaxTypeFare);
      }

      if (calculate)
      {
        if (!_isNetTktingWithItBtFare)
        {
          _commAmount += calculateNetTicketingCommission(ptf, negFareRest);
        }
        else
        {
          sellingComm = 0.0;
          netComm = 0.0;
          calculateNetTktCommissions(ptf, negFareRest, sellingComm, netComm, *negPaxTypeFare);

          _commAmount += sellingComm;
          _netCommAmount += netComm;
        }
      }
      else if (UNLIKELY(_diag865))
      {
        _diag865->displayCommissionApplication(*this);
      }

      if (!(ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
            negFareRest->negFareCalcTblItemNo() == 0) &&
          _ticketingEntry && !_trx.getRequest()->isFormOfPaymentCard() &&
          !_trx.getOptions()->isCat35Sell() && isITBTFare(negFareRest) &&
          fPath.netFarePath() == nullptr) // don't do it for W' Cat 35 TFSF (it is done in PricingUtil)
      {
        fu->netCat35NucUsed(); // For TAX's..
        fu->netCat35NucAmount() =
            negPaxTypeFare->nucNetAmount(); // all amt's are in calculate currency..
        ptf->nucFareAmount() =
            fu->netCat35NucAmount(); // swap net/selling fare amounts ( for FareCalc)

        if (_netMileageSurchargeAmt > 0)
        {
          ptf->mileageSurchargeAmt() = _netMileageSurchargeAmt;
        }

        netAmtWasFlipped = true;
      }
      netTotalAmt += fu->totalFareAmount();
    }
  }

  if (netAmtWasFlipped) // was flipped?
  {
    fPath.setTotalNUCAmount(netTotalAmt);
  }

  if (calculate)
  {
    if (_ticketingEntry && _ruleCommAmt == 0 && _rulePercent == RuleConst::PERCENT_NO_APPL &&
        !_commType.empty() && _commType[0] != PERCENT_COMM_TYPE)
    {
      _commAmount += _inputCommAmt;
    }

    if (_isNetTktingWithItBtFare && !_ticketingEntry && _ruleCommAmt == 0 &&
        _rulePercent == RuleConst::PERCENT_NO_APPL && !_commType.empty() &&
        _commType[0] != PERCENT_COMM_TYPE)
    {
      _commAmount += _inputCommAmt;
      _netCommAmount += _inputCommAmt;
    }
    if (_commAmount == 0)
      _commPercent = 0;
    else
      _commPercent = _rulePercent;

    if (_isNetTktingWithItBtFare)
    {
      _netCommPercent = _rulePercent;
    }
  }
}

MoneyAmount
Commissions::calculateNetTicketingCommission(PaxTypeFare* ptf, const NegFareRest* negFareRest)
{
  MoneyAmount commAmount = 0;
  Diag865Collector::CommissionCalcMethod method = Diag865Collector::AMOUNT;

  if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
      negFareRest->negFareCalcTblItemNo() == 0) // Type L without Net in 979 - any FOP
  {
    if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
    {
      // method SELL * COMM PCT
      method = Diag865Collector::SELL_TIMES_COMM_PCT;
      commAmount = (_rulePercent * (_totalSellAmt)) / 100.0;
    }
    else if (_ruleCommAmt == 0 && _commType.empty()) // comm. amount is 0 but no agent i/p
    {
      commAmount = _ruleCommAmt;
      _commBaseAmount += _totalSellAmt; // collect commission base amount
    }
    else if (_ruleCommAmt != 0) // comm. amount is not 0
    {
      commAmount = _ruleCommAmt;
    }
    else // or agent input
    {
      if (!_commType.empty())
      {
        if (_commType[0] == PERCENT_COMM_TYPE)
        {
          method = Diag865Collector::SELL_TIMES_COMM_PCT;
          commAmount = (_inputCommPrc * (_totalSellAmt)) / 100.0;
        }
        else
        {
          method = Diag865Collector::MARKUP;
          commAmount = 0;
        }
      }
    }
  }
  else
  {
    const bool noITBTfare = !isITBTFare(negFareRest);

    if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
    {
      if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
          noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
      {
        // method SELL - NET + SELL * COMM PCT
        method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
        commAmount = _markUpCommission + (_rulePercent * _totalSellAmt) / 100.0;
      }
      else
      {
        // method NET * COMM PCT
        method = Diag865Collector::NET_TIMES_COMM_PCT;
        commAmount = (_rulePercent * _totalNetAmt) / 100.0;
      }
    }
    else if (_ruleCommAmt == 0 && _commType.empty()) // comm. amount is 0 but no agent i/p
    {
      if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
          noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
      {
        commAmount = _markUpCommission;
        _commBaseAmount += _totalSellAmt; // collect commission base amount
      }
      else
      {
        commAmount = _ruleCommAmt;
        _commBaseAmount += _totalNetAmt; // collect commission base amount
      }
    }
    else if (_ruleCommAmt != 0) // comm. amount is not 0
    {
      if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
          noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT)
      {
        // method SELL - NET + COMM AMT
        method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
        commAmount = _markUpCommission + _ruleCommAmt;
      }
      else
      {
        commAmount = _ruleCommAmt;
      }
    }
    else // or agent input
    {
      if (!_commType.empty())
      {
        if (_commType[0] == PERCENT_COMM_TYPE) // agent i/p is %
        {
          if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
              noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
          {
            method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
            commAmount = _markUpCommission + (_inputCommPrc * _totalSellAmt) / 100.0;
          }
          else
          {
            method = Diag865Collector::NET_TIMES_COMM_PCT;
            commAmount = (_inputCommPrc * _totalNetAmt) / 100.0;
          }
        }
        else // agent i/p is amt
        {
          method = Diag865Collector::MARKUP;
          if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
              noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
          {
            commAmount = _markUpCommission;
          }
          else
          {
            commAmount = 0;
          }
        }
      }
    }
  }

  if (UNLIKELY(_diag865))
  {
    _diag865->displayCommissionApplication(*this, commAmount, method);
  }
  return commAmount;
}

void
Commissions::calculateNetTktCommissions(PaxTypeFare* ptf,
                                        const NegFareRest* negFareRest,
                                        MoneyAmount& sellingCommission,
                                        MoneyAmount& netCommission,
                                        const NegPaxTypeFareRuleData& negPaxTypeFare)
{
  MoneyAmount commAmount = 0;
  Diag865Collector::CommissionCalcMethod method = Diag865Collector::AMOUNT;
  Diag865Collector::CommissionCalcMethod method2 =
      Diag865Collector::AMOUNT; // method for net fare comm. calc.

  if ( (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
        negFareRest->negFareCalcTblItemNo() == 0) ) // Type L without Net in 979 - any FOP
  {
    if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
    {
      // method SELL * COMM PCT
      method = Diag865Collector::SELL_TIMES_COMM_PCT;
      commAmount = (_rulePercent * (_totalSellAmt)) / 100.0;
    }
    else if (_ruleCommAmt == 0 && _commType.empty()) // comm. amount is 0 but no agent i/p
    {
      commAmount = _ruleCommAmt;
      _commBaseAmount += _totalSellAmt; // collect commission base amount
    }
    else if (_ruleCommAmt != 0) // comm. amount is not 0
    {
      commAmount = _ruleCommAmt;
    }
    else // or agent input
    {
      if (!_commType.empty())
      {
        if (_commType[0] == PERCENT_COMM_TYPE)
        {
          method = Diag865Collector::SELL_TIMES_COMM_PCT;
          commAmount = (_inputCommPrc * (_totalSellAmt)) / 100.0;
        }
        else
        {
          commAmount = 0;
        }
      }
    }
    sellingCommission = commAmount;
    if (!_ticketingEntry)
    {
      netCommission = sellingCommission;
      method2 = method;
    }
  }
  else
  {
    const bool noITBTfare = !isITBTFare(negFareRest);

    if (_rulePercent != RuleConst::PERCENT_NO_APPL) // comm. %
    {
      if (_ticketingEntry)
      {
        if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
            noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
        {
          // method SELL - NET + SELL * COMM PCT
          method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
          commAmount = _markUpCommission + (_rulePercent * _totalSellAmt) / 100.0;
        }
        else
        {
          // method NET * COMM PCT
          method = Diag865Collector::NET_TIMES_COMM_PCT;
          commAmount = (_rulePercent * _totalNetAmt) / 100.0;
        }
      }
      else // pricing entry
      {
        // method SELL - NET + SELL * COMM PCT
        method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
        commAmount = _markUpCommission + (_rulePercent * _totalSellAmt) / 100.0;
        // method NET * COMM PCT
        method2 = Diag865Collector::NET_TIMES_COMM_PCT;
        netCommission = (_rulePercent * _totalNetAmt) / 100.0;
      }
    }
    else if (_ruleCommAmt == 0 && _commType.empty()) // comm. amount is 0 but no agent i/p
    {
      if (_ticketingEntry)
      {
        if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
            noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
        {
          // method SELL - NET DIFERENCE
          method = Diag865Collector::MARKUP;
          commAmount = _markUpCommission;
          _commBaseAmount += _totalSellAmt; // collect commission base amount
        }
        else
        {
          commAmount = _ruleCommAmt;
          _commBaseAmount += _totalNetAmt; // collect commission base amount
        }
      }
      else
      {
        // method SELL - NET DIFERENCE
        method = Diag865Collector::MARKUP;
        commAmount = _markUpCommission;
        netCommission = _ruleCommAmt;
        _commBaseAmount += _totalSellAmt; // collect commission base amount
      }
    }
    else if (_ruleCommAmt != 0) // comm. amount is not 0
    {
      if (_ticketingEntry)
      {
        if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
            noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT)
        {
          // method SELL - NET + COMM AMT
          method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
          commAmount = _markUpCommission + _ruleCommAmt;
        }
        else
        {
          commAmount = _ruleCommAmt;
        }
      }
      else
      {
        // method SELL - NET + COMM AMT
        method = Diag865Collector::MARKUP_PLUS_COMM_AMT;
        commAmount = _markUpCommission + _ruleCommAmt;
        netCommission = _ruleCommAmt;
      }
    }
    else // or agent input
    {
      if (!_commType.empty())
      {
        if (_commType[0] == PERCENT_COMM_TYPE) // agent i/p is %
        {
          if (_ticketingEntry)
          {
            if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
                noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
            {
              method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
              commAmount = _markUpCommission + (_inputCommPrc * _totalSellAmt) / 100.0;
            }
            else
            {
              method = Diag865Collector::NET_TIMES_COMM_PCT;
              commAmount = (_inputCommPrc * _totalNetAmt) / 100.0;
            }
          }
          else
          {
            method = Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT;
            commAmount = _markUpCommission + (_inputCommPrc * _totalSellAmt) / 100.0;
            method2 = Diag865Collector::NET_TIMES_COMM_PCT;
            netCommission = (_inputCommPrc * _totalNetAmt) / 100.0;
          }
        }
        else // agent i/p is amt
        {
          if (_ticketingEntry)
          {
            if (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell() ||
                noITBTfare) // CC or use selling fare ind or fare filed w/o IT/BT
            {
              method = Diag865Collector::MARKUP;
              commAmount = _markUpCommission;
            }
            else
            {
              commAmount = 0;
            }
          }
          else
          {
            method = Diag865Collector::MARKUP;
            commAmount = _markUpCommission;
            netCommission = 0;
          }
        }
      }
    }
    sellingCommission = commAmount;
  }

  if (UNLIKELY(_diag865))
  {
    _diag865->displayCommissionApplication(*this, commAmount, method);
    // display net commission info only for pricing request
    if (!_ticketingEntry)
      _diag865->displayCommissionApplication(*this, netCommission, method2, true);
  }
}

void
Commissions::getRuleCommission(const PaxTypeFare* ptf,
                               const NegFareRest* negFareRest,
                               const NegPaxTypeFareRuleData* negPaxTypeFare)
{
  _ruleCommAmt = 0;
  _rulePercent = RuleConst::PERCENT_NO_APPL;
  CurrencyCode currency = negPaxTypeFare->calculatedNegCurrency();

  if (negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL)
  {
    if (negFareRest->cur1() == currency ||
        (!negFareRest->cur1().empty() && negFareRest->cur2().empty()))
    {
      _ruleCommAmt = negFareRest->commAmt1();
      currency = negFareRest->cur1();
    }
    else if (negFareRest->cur2() == currency ||
             (!negFareRest->cur2().empty() && negFareRest->cur1().empty()))
    {
      _ruleCommAmt = negFareRest->commAmt2();
      currency = negFareRest->cur2();
    }
    else if (!negFareRest->cur1().empty() && !negFareRest->cur2().empty())
    {
      MoneyAmount commAmountInNUC1 = 0, commAmountInNUC2 = 0;

      convertToNUC(negFareRest->commAmt1(), negFareRest->cur1(), commAmountInNUC1);
      convertToNUC(negFareRest->commAmt2(), negFareRest->cur2(), commAmountInNUC2);

      if (commAmountInNUC1 < (commAmountInNUC2 + EPSILON))
      {
        _ruleCommAmt = negFareRest->commAmt1();
        currency = negFareRest->cur1();
      }
      else if (fabs(negFareRest->commAmt2()) > EPSILON)
      {
        _ruleCommAmt = negFareRest->commAmt2();
        currency = negFareRest->cur2();
      }
    }
  }
  else
  {
    _rulePercent = negFareRest->commPercent();
  }

  if (_ruleCommAmt && ptf->isRoundTrip())
  {
    _ruleCommAmt = _ruleCommAmt * 0.5;
  }
  if (currency != _paymentCurrency)
  {
    _ruleCommAmt = convertCurrency(_ruleCommAmt, currency, _paymentCurrency);
  }
}

MoneyAmount
Commissions::convertCurrency(MoneyAmount sourceAmount,
                             const CurrencyCode& sourceCurrency,
                             const CurrencyCode& targetCurrency,
                             bool doNonIataRounding)
{
  Money sourceMoney(sourceAmount, sourceCurrency);
  Money targetMoney(targetCurrency);
  MoneyAmount targetAmount = 0;

  _ccFacade.setRoundFare(true);

  if (doNonIataRounding)
    targetMoney.setApplyNonIATARounding();

  if (_ccFacade.convert(targetMoney, sourceMoney, _trx, _useInternationalRounding))
  {
    targetAmount = targetMoney.value();
  }
  return targetAmount;
}

bool
Commissions::convertToNUC(MoneyAmount amount, const CurrencyCode& currency, MoneyAmount& result)
{
  Money amountInCurrency(amount, currency);
  Money amountInNUC("NUC");
  CurrencyConversionRequest request(amountInNUC,
                                    amountInCurrency,
                                    _trx.getRequest()->ticketingDT(),
                                    *(_trx.getRequest()),
                                    _trx.dataHandle(),
                                    _useInternationalRounding);

  NUCCurrencyConverter ncc;

  if (ncc.convert(request, nullptr))
  {
    result = amountInNUC.value();
    return true;
  }
  return false;
}

bool
Commissions::isITBTFare(const NegFareRest* negFareRest) const
{
  NegotiatedFareRuleUtil negFareRuleUtil;
  std::string fareBox;

  if (negFareRest->noSegs() == NegotiatedFareRuleUtil::ONE_SEGMENT)
  {
    fareBox = negFareRest->fareBoxText1();
  }
  else if (negFareRest->noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS)
  {
    if (negFareRest->couponInd1() == NegotiatedFareRuleUtil::PSG_COUPON)
    {
      fareBox = negFareRest->fareBoxText1();
    }
    else if (negFareRest->couponInd2() == NegotiatedFareRuleUtil::PSG_COUPON)
    {
      fareBox = negFareRest->fareBoxText2();
    }
  }

  return negFareRuleUtil.checkFareBox(fareBox);
}

//@todo should we move this to RuleUtil and rename it?
bool
Commissions::isNetRemitAllowed(const CollectedNegFareData& cNegFareData) const
{
  bool abacusUser = _trx.getRequest()->ticketingAgent()->abacusUser();
  bool infiniUser = _trx.getRequest()->ticketingAgent()->infiniUser();
  bool axessUser = _trx.getRequest()->ticketingAgent()->axessUser();

  return (
     (cNegFareData.bspMethod() == RuleConst::NRR_METHOD_1 && infiniUser) ||
     (cNegFareData.bspMethod() == RuleConst::NRR_METHOD_2 && TrxUtil::isNetRemitEnabled(_trx)) ||
     (!fallback::fallbackBSPMt3For1S(&_trx) ? cNegFareData.bspMethod() == RuleConst::NRR_METHOD_3 && !axessUser :
      cNegFareData.bspMethod() == RuleConst::NRR_METHOD_3 && (abacusUser || infiniUser)));
}

bool
Commissions::isCalculateCommFromTFD(const FarePath& fp, const CollectedNegFareData& cNegFareData)
    const
{
  if (cNegFareData.bspMethod() == RuleConst::NRR_METHOD_2 &&
      _ticketingEntry && /* is direct ticketing? */
      _commType.size() &&
      _commType[0] == PERCENT_COMM_TYPE && /* is command W#PKN? */
      _ruleCommAmt == 0 &&
      _rulePercent == RuleConst::PERCENT_NO_APPL && /* Cat35 no amount or no percent filing */
      _trx.isGNRAllowed() &&
      fp.pricingUnit().size() && fp.pricingUnit()[0]->fareUsage().size())
  {
    Indicator tktFareInd =
        NegotiatedFareRuleUtil::getFareAmountIndicator(_trx, fp.pricingUnit()[0]->fareUsage()[0]);
    return (tktFareInd == RuleConst::NR_VALUE_A || tktFareInd == RuleConst::NR_VALUE_B);
  }
  return false;
}

// @todo Remove this method with fallbackAMCPhase2 and SecurityHandshakeValidator
// Return true if valid but also fetch all source PCCs who has granted
// permissions to target PCC (request PCC) to use commission rules
bool
Commissions::securityHandShakeValid(FarePath& fpath) const
{
  PseudoCityCode tvlAgencyPCC;
  if (_trx.getRequest() && _trx.getRequest()->ticketingAgent())
    tvlAgencyPCC = _trx.getRequest()->ticketingAgent()->tvlAgencyPCC();

  SecurityHandshakeProductCode prodCode("CM");
  if (tvlAgencyPCC.empty() ||
      !SecurityHandshakeValidator::validateSecurityHandshake(_trx, prodCode, tvlAgencyPCC))
  {
    if (!tvlAgencyPCC.empty() && _diag867)
      _diag867->printNoSecurityHandShakeForPCC(fpath, tvlAgencyPCC);
    return false;
  }
  return true;
}

bool Commissions::isTrxApplicableForAgencyCommission(FarePath& fpath) const
{
  if (_trx.getTrxType() == PricingTrx::PRICING_TRX ||
      _trx.getTrxType() == PricingTrx::MIP_TRX ||
      _trx.getTrxType() == PricingTrx::IS_TRX)
  {
    return true;
  }

  if (_diag867)
    _diag867->printAMCCommandNotSupported(fpath);

  return false;
}

bool Commissions::isRequestFromTravelAgent(FarePath& fpath) const
{
  if (_trx.getRequest() &&
          _trx.getRequest()->ticketingAgent() &&
          !_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    return true;

  if(_diag867)
    _diag867->printAMCforASNotApplicable(fpath);

  return false;
}

bool Commissions::isCat35Solution(const FarePath& fpath) const
{
  if (fpath.collectedNegFareData() &&
      fpath.collectedNegFareData()->indicatorCat35())
  {
    if(_diag867)
      _diag867->printAMCforCat35NotApplicable(fpath);
    return true;
  }
  return false;
}

bool Commissions::isTicketingAgentSpecCommissionWithEPR(FarePath& fpath) const
{
  if(isTicketingAgentSpecifiedCommission() &&
     _trx.getOptions()->isKeywordPresent(EPR_COMMOVER))
  {
    if(_diag867)
      _diag867->printAMCforKPandEPRNotApplicable(fpath);
    return true;
  }
  return false;
}

}
