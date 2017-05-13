//-------------------------------------------------------------------
//
//  File:        NegotiatedFareRuleUtil.cpp
//  Created:     Oct 07, 2004
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

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/PrintOption.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag535Collector.h"
#include "Rules/NetRemitFareSelection.h"
#include "Rules/NetRemitPscMatchUtil.h"
#include "Rules/RuleUtil.h"
#include "Rules/TourCodeUtil.h"
#include "Rules/ValueCodeUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(fallbackProcessEmptyTktEndorsements)
FALLBACK_DECL(fallbackBSPMt3For1S)
FALLBACK_DECL(fallbackAMCFixMissingMileageNetAmtCat35L);
FALLBACK_DECL(fallbackEndorsementsRefactoring)
FALLBACK_DECL(fallbackFixPQNRedistributedWholesale)

const std::string NegotiatedFareRuleUtil::IT_TICKET = "IT";
const std::string NegotiatedFareRuleUtil::BT_TICKET = "BT";
const std::string NegotiatedFareRuleUtil::BLANK_CURRENCY = "***";

bool
NegotiatedFareRuleUtil::processNegFareITBT(PricingTrx& trx, FarePath& farePath)

{
  _indicatorTkt = TrxUtil::isExchangeOrTicketing(trx);
  _abacusUser = trx.getRequest()->ticketingAgent()->abacusUser();
  _infiniUser = trx.getRequest()->ticketingAgent()->infiniUser();
  _axessUser = trx.getRequest()->ticketingAgent()->axessUser();

  _isGnrUser = trx.isGNRAllowed();

  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  _baseFareCurrency = farePath.itin()->originationCurrency();

  if (LIKELY(!trx.getOptions()->currencyOverride().empty()))
    _paymentCurrency = trx.getOptions()->currencyOverride();

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic865))
  {
    std::map<std::string, std::string>::const_iterator e = trx.diagnostic().diagParamMap().end();
    std::map<std::string, std::string>::const_iterator i;

    // check a qualifiers
    //    i = trx.diagnostic().diagParamMap().find( ENTRY_TYPE );  // need to change TseConst.h !!!!
    i = trx.diagnostic().diagParamMap().find("ET");
    if (i != e)
    {
      if (i->second.substr(0, 1) == "T")
        _indicatorTkt = true;
    }
  }

  CollectedNegFareData* cNegFareData;
  // lint -e{530}
  trx.dataHandle().get(cNegFareData);

  if (trx.getRequest()->ticketingAgent()->axessUser() &&
      (trx.getRequest()->isWpNettRequested() || trx.getRequest()->isWpSelRequested()))
  {
    processAxessCat35(trx, farePath, cNegFareData);
    return true;
  }

  bool retCode = isNegotiatedFareCombo(trx, farePath, cNegFareData);
  farePath.collectedNegFareData() = cNegFareData;

  if (LIKELY(_warningMsg == NO_WARNING))
  {
    // check low priority warning/error
    if (_invalidNetRemComm)
      _warningMsg = INVALID_NET_REMIT_COMM;
  }

  if (retCode) // was cat35
  {
    if (TrxUtil::optimusNetRemitEnabled(trx) && _cat35NetRemit && farePath.collectedNegFareData())
    {
      setNetRemitTicketInd(trx, farePath);
      if (_warningMsg != MULTIPLE_VALUE_CODE && _warningMsg != MULTIPLE_TOUR_CODE)
      {
        if (!ValueCodeUtil::decodeValueCode(trx, farePath))
          ValueCodeUtil::saveStaticValueCode(trx, farePath);
        // otherwise, Cat18 ValueCode was saved in collectedNegFareData()->valueCode()

        TourCodeUtil::saveTourCodeForNetRemitSMF(trx, farePath);
        prepareTourCodeValueCodeByPO(trx, farePath);
      }
    }
    saveTourCodeForPQ(trx, farePath);
    return handleCat35Processing(cNegFareData, trx, farePath);
  }
  // else was not cat35
  cNegFareData->indicatorCat35() = false; //  indicate non cat35
  if (trx.getOptions()->isCat35Net())
  {
    cNegFareData->trailerMsg() = "NO NET FARE AMOUNT";

    AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(&trx);
    if (altPricingTrx == nullptr || altPricingTrx->altTrxType() != AltPricingTrx::WPA)
      throw tse::ErrorResponseException(ErrorResponseException::NO_NET_FARE_AMOUNT);

    if (_abacusUser || _infiniUser)
    {
      farePath.tfrRestricted() = true;
      return false;
    }
  }
  return true;
}

bool
NegotiatedFareRuleUtil::checkWarningMsgForTicketing(FarePath& farePath, const PricingTrx& trx)
{
  if (_netAmtWasFlipped) // was flipped?
    farePath.setTotalNUCAmount(_netTotalAmt);

  switch (_warningMsg)
  {
  case NO_WARNING: // Correct cat35 ticketing data
    break;
  case MULTIPLE_TOUR_CODE:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_MULTIPLE_TOUR_CODES);
  case TOUR_CODE_NOT_FOUND:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_TOUR_CODE_NOT_FOUND);
  case INVALID_ITBT_PSG_COUPON:
  case EMPTY_ITBT_PSG_COUPON:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_INVALID_TEXT_BOX_COMBO);
  case ITBT_BSP_NOT_BLANK:
    throw tse::ErrorResponseException(ErrorResponseException::NET_REMIT_FARE_PHASE_FOUR);
  case NOT_ITBT:
    throw tse::ErrorResponseException(ErrorResponseException::UNABLE_TO_PROCESS_NEG_FARE_DATA);
  case MULTIPLE_FARE_BOX:
  case MULTIPLE_BSP:
  case MULTIPLE_NET_GROSS:
  case MIX_FARES:
  case ISSUE_SEPARATE_TKT:
  case MIXED_FARES_COMBINATION:
    throw tse::ErrorResponseException(ErrorResponseException::ISSUE_SEPARATE_TICKET);
  case FARES_NOT_COMBINABLE:
  case NET_SELLING_CONFLICT:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_NET_SELLING_AMOUNTS_CONFLICT);
  case MULTIPLE_COMMISSION:
  case MIX_COMMISSION:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_COMMISSIONS_NOT_COMBINABLE);
  case MIXED_FARE_BOX_AMT:
  case INVALID_NET_REMIT_FARE:
  case CONFLICTING_TFD_BYTE101:
    throw tse::ErrorResponseException(ErrorResponseException::UNABLE_AUTO_TKT_INV_NET_REMIT_FARE);
  case INVALID_NET_REMIT_COMM:
    throw tse::ErrorResponseException(ErrorResponseException::FARE_REQUIRE_COMM_PERCENT);
  case MULTIPLE_VALUE_CODE:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_MULTIPLE_VALUE_CODES);
  case MULTIPLE_PRINT_OPTION:
    throw tse::ErrorResponseException(ErrorResponseException::UTAT_MULTIPLE_PRINT_OPTIONS);
  default:
    throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
  return true;
}

// Return true for "net-remit fare but agent is not part of GNR or method type is not 2"
bool
NegotiatedFareRuleUtil::isTktRestrictedForGnrUsers(const FarePath& farePath)
{
  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  for(PricingUnit * pu : farePath.pricingUnit())
  {
    for(FareUsage * fu : pu->fareUsage())
    {
      if (!fu->paxTypeFare())
        continue;

      const NegFareRest* negFareRest = getCat35Record3(fu->paxTypeFare(), negPaxTypeFare);
      if (negFareRest)
      {
        if (isNetRemitFare(fu->paxTypeFare()->fcaDisplayCatType(), negFareRest) &&
            (!_isGnrUser || negFareRest->netRemitMethod() != RuleConst::NRR_METHOD_2))
          return true;
      }
    }
  }
  return false;
}

// TFR restricted for GNR users with Cat35 (negotiated fare) method type 2
bool
NegotiatedFareRuleUtil::isTfrRestrictedForGnrUsers(const FarePath& farePath)
{
  if (!_isGnrUser)
    return false;

  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  for(PricingUnit * pu : farePath.pricingUnit())
  {
    for(FareUsage * fu : pu->fareUsage())
    {
      if (!fu->paxTypeFare())
        continue;

      const NegFareRest* negFareRest = getCat35Record3(fu->paxTypeFare(), negPaxTypeFare);
      if (negFareRest && negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2)
        return true;
    }
  }
  return false;
}

bool
NegotiatedFareRuleUtil::checkWarningMsgForPricing(CollectedNegFareData* cNegFareData,
                                                  PricingTrx& trx,
                                                  FarePath& farePath)
{
  if (_warningMsg == NO_WARNING && trx.getOptions()->isCat35Net()) // if WPNET entry was O'K
  {
    cNegFareData->trailerMsg() = "NET FARE AMOUNT FOR INFORMATIONAL PURPOSES ONLY";
  }

  if (_netAmtWasFlipped) // was flipped?
  {
    farePath.setTotalNUCAmount(_netTotalAmt);
  }

  if (_warningMsg == NO_WARNING)
  {
    return true;
  }

  if (_warningMsg == INVALID_NET_REMIT_COMM)
  {
    cNegFareData->trailerMsg() = "FARE REQUIRES COMMISSION PERCENT";
    return true;
  }

  if (_warningMsg == AUTO_TKT_NOT_PERMITTED)
  {
    cNegFareData->trailerMsg() = "AUTO TICKETING NOT PERMITTED";
    farePath.tktRestricted() = true;
    return true;
  }

  // Set up "tfrRestricted" indicator for the Abacus User
  // when there is any other warning
  if (_abacusUser || _infiniUser)
    farePath.tfrRestricted() = true;

  if (_warningMsg == MIX_COMMISSION || _warningMsg == MULTIPLE_COMMISSION ||
      _warningMsg == MULTIPLE_NET_GROSS)
  {
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - COMMISSIONS NOT COMBINABLE";
    if (isTfrRestrictedForGnrUsers(farePath))
      farePath.tfrRestricted() = true;
    return true;
  }

  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(&trx);

  if (_warningMsg == MULTIPLE_TOUR_CODE)
  {
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - MULTIPLE TOUR CODES";
  }
  else if (_warningMsg == TOUR_CODE_NOT_FOUND)
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - TOUR CODE NOT FOUND";
  else if (_warningMsg == NOT_ITBT || _warningMsg == INVALID_ITBT_PSG_COUPON ||
           _warningMsg == EMPTY_ITBT_PSG_COUPON || _warningMsg == MULTIPLE_FARE_BOX)
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - INVALID TEXT BOX COMBINATION";
  else if (_warningMsg == ITBT_BSP_NOT_BLANK)
    cNegFareData->trailerMsg() = "NET REMIT FARE - PHASE 4 AND USE NET/ FOR TKT ISSUANCE";
  else if (_warningMsg == MULTIPLE_BSP || _warningMsg == ISSUE_SEPARATE_TKT ||
           _warningMsg == MIX_FARES || _warningMsg == MIXED_FARES_COMBINATION)
  {
    cNegFareData->trailerMsg() = "ISSUE SEPARATE TICKETS";
    if (isTfrRestrictedForGnrUsers(farePath))
      farePath.tfrRestricted() = true;
  }
  else if (_warningMsg == NO_NET_FARE_AMOUNT)
  {
    cNegFareData->trailerMsg() = "NO NET FARE AMOUNT";
    if (altPricingTrx == nullptr || altPricingTrx->altTrxType() != AltPricingTrx::WPA)
      throw tse::ErrorResponseException(ErrorResponseException::NO_NET_FARE_AMOUNT);

    //@todo: should we check for isGnrUser here?
    if (_abacusUser || _infiniUser)
    {
      return false;
    }
  }
  else if (_warningMsg == FARES_NOT_COMBINABLE_NO_NET)
  {
    cNegFareData->trailerMsg() = "FARES NOT COMBINABLE";
    if (altPricingTrx == nullptr || altPricingTrx->altTrxType() != AltPricingTrx::WPA)
      throw tse::ErrorResponseException(ErrorResponseException::NO_NET_FARE_AMOUNT);
  }
  else if (_warningMsg == FARES_NOT_COMBINABLE || _warningMsg == NET_SELLING_CONFLICT)
  {
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - NET/SELLING AMOUNTS CONFLICT";
  }
  else if (_warningMsg == SYSTEM_ERROR)
  {
    cNegFareData->trailerMsg() = "SYSTEM ERROR";
    if (altPricingTrx == nullptr || altPricingTrx->altTrxType() != AltPricingTrx::WPA)
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
  else if (_warningMsg == INVALID_NET_REMIT_FARE || _warningMsg == MIXED_FARE_BOX_AMT ||
           ((TrxUtil::tfdNetRemitFareCombEnabled(trx) &&
             (_abacusUser || _infiniUser || _isGnrUser)) &&
            (_isCmdPricing && _warningMsg == CONFLICTING_TFD_BYTE101)))
  {
    cNegFareData->trailerMsg() = "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
    if (isTfrRestrictedForGnrUsers(farePath))
      farePath.tfrRestricted() = true;
  }
  else if (_warningMsg == NET_FARE_AMOUNT_EXCEEDS_FARE)
  {
    cNegFareData->trailerMsg() = "NET AMOUNT EXCEEDS FARE - VERIFY NET AMOUNT";
  }
  else if (_warningMsg == MULTIPLE_VALUE_CODE)
  {
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - MULTIPLE VALUE CODES";
  }
  else if (_warningMsg == MULTIPLE_PRINT_OPTION)
  {
    cNegFareData->trailerMsg() = "UNABLE TO AUTO TICKET - MULTIPLE PRINT OPTIONS";
  }
  else
  {
    cNegFareData->trailerMsg() = "SYSTEM ERROR WARNING ";
  }

  if (_warningMsg == MULTIPLE_PRINT_OPTION || _warningMsg == MULTIPLE_VALUE_CODE ||
      _warningMsg == MULTIPLE_TOUR_CODE)
  {
    if (isTfrRestrictedForGnrUsers(farePath))
      farePath.tfrRestricted() = true;
  }

  if ((_warningMsg == NOT_ITBT || _warningMsg == INVALID_ITBT_PSG_COUPON ||
       _warningMsg == EMPTY_ITBT_PSG_COUPON || _warningMsg == MULTIPLE_FARE_BOX ||
       _warningMsg == MULTIPLE_NET_GROSS || _warningMsg == MULTIPLE_BSP ||
       _warningMsg == ISSUE_SEPARATE_TKT || _warningMsg == MIX_FARES) &&
      (!_abacusUser && !_infiniUser))
  {
    farePath.tktRestricted() = true; // CAT35 TFSF
  }

  if (!farePath.tktRestricted() && isTktRestrictedForGnrUsers(farePath))
    farePath.tktRestricted() = true;

  return true;
}

bool
NegotiatedFareRuleUtil::handleCat35Processing(CollectedNegFareData* cNegFareData,
                                              PricingTrx& trx,
                                              FarePath& farePath)
{
  cNegFareData->indicatorCat35() = true; //  indicate cat35

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic535 && _warningMsg != NO_WARNING))
  {
    Diag535Collector* diag = nullptr;
    DCFactory* factory = DCFactory::instance();
    diag = dynamic_cast<Diag535Collector*>(factory->create(trx));
    diag->enable(Diagnostic535);
    diag->diag535Message(trx, _warningMsg);
    diag->flushMsg();
  }
  if (_indicatorTkt && checkWarningMsgForTicketing(farePath, trx))
    return true;
  else // pricing...
    return checkWarningMsgForPricing(cNegFareData, trx, farePath);
}

// -------------------------------------------------------------------------
//  isNegotiatedFareCombo()
//
//  Cat 35 Fare Combinations:
//  1. Cat 35 IT/BT fare can only combine with Cat 35 IT/BT.
//  2. Cat 35 non-IT/BT fare can combine with fares with the same tour code
//     (or any blanks). The blank can be coming from any type of fare
//     (Cat 15, 35, 25 or public).
//  3. Cat 35 fare that is using Cat 35 for security only (ignore Cat 35
//     ticketing data) will be treated as regular private fare. The Cat 35
//     indicator will not be set in CollectedNegFareData for ticketing.
//  4. For multiple carriers,
//     4.1 Allow single ticket for Cat 35 IT/BT combinations.
//     4.2 Allow single ticket for Cat 35 non IT/BT combinations
//         if all segments have same tour codes.
//         Otherwise, send trailer message to issue separate ticket.
//     4.3 Allow single ticket for Cat 35 using Cat 35 for security only
//         if all segments have no ticketing information.
//         Otherwise, send trailer message to issue separate ticket.
// -------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::isNegotiatedFareCombo(PricingTrx& trx,
                                              FarePath& fPath,
                                              CollectedNegFareData* cNegFareData)
{
  Diag535Collector* diag = nullptr;
  bool dg = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic535))
  {
    DCFactory* factory = DCFactory::instance();
    diag = dynamic_cast<Diag535Collector*>(factory->create(trx));
    diag->enable(Diagnostic535);
    dg = true;
    diag->diag535Request(trx, fPath);
  }

  cNegFareData->comPercent() = RuleConst::PERCENT_NO_APPL; // Not applicable
  bool firstFareProcessed = false;
  bool firstCat35FareProcessed = false;
  CarrierCode carrierCode;

  bool cat35Found = false;
  bool cat35NetRemitFound = false;
  bool cat35NetTKTFound = false;

  _tktCarrier.clear();
  _endorsementTxt.clear();

  _isCmdPricing = checkCmdPricingFare(trx, fPath);


  std::vector<const PaxTypeFare*> paxTypeFares;

  for (const PricingUnit* pricingUnit : fPath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (UNLIKELY(diag && diag->isActive()))
      {
        diag->diag535Collector(trx, *fPath.itin(), *fareUsage);
      }
      _cat35NetRemit = false;
      _cat35NetTicketing = false;

      if (!firstFareProcessed) // initialize processing for 1st fare
      {
        carrierCode = fareUsage->paxTypeFare()->carrier();
        if (fareUsage->paxTypeFare()->isNegotiated())
        {
          cat35Found = true;
          bool rc =
              checkTktDataCat35(trx, fPath, *fareUsage, cNegFareData, firstCat35FareProcessed);
          if (rc)
          {
            if (_warningMsg != NO_WARNING)
            {
              return true; //  Negfare data problem
            }
          }
          else
          {
            return true; // some hard error prblem in cat35
          }
          if (_cat35NetRemit)
          {
            cat35NetRemitFound = true;
          }
          else if (_cat35NetTicketing)
          {
            cat35NetTKTFound = true;
          }
        }
        else
        {
          // accumulate totalSellingAmt for the commissions.
          // (may be mix publish fare and C35 non_IT/BT)
          cNegFareData->totalSellingAmt() += fareUsage->paxTypeFare()->nucFareAmount();
          if (trx.getOptions()->isCat35Net()) // if WPNET entry
          {
            _warningMsg = NO_NET_FARE_AMOUNT;
            return true;
          }
          cNegFareData->netTotalAmt() += fareUsage->paxTypeFare()->nucFareAmount();
        }
        firstFareProcessed = true;
      }
      else // 2nd, 3rd.. etc.. fares
      {
        if (fareUsage->paxTypeFare()->isNegotiated())
        {
          if (!cat35Found)
          {
            cat35Found = true;
          }
          bool rc =
              checkTktDataCat35(trx, fPath, *fareUsage, cNegFareData, firstCat35FareProcessed);
          if (!rc || // some problem in cat35
              _warningMsg != NO_WARNING)
            return true;
          if (_isCmdPricing)
          {
            if ((_cat35NetRemit && !cat35NetRemitFound) || (!_cat35NetRemit && cat35NetRemitFound))
            {
              _warningMsg = MIXED_FARES_COMBINATION; // Net Remit combines with non-Net Remit
              return true;
            }
            if (((_indNetGross != BLANK || _comPercent != RuleConst::PERCENT_NO_APPL ||
                  _comAmount > 0) &&
                 (!cat35NetTKTFound && !cat35NetRemitFound)) ||
                ((!_cat35NetTicketing && !_cat35NetRemit) &&
                 (cNegFareData->indNetGross() != BLANK ||
                  cNegFareData->comPercent() != RuleConst::PERCENT_NO_APPL ||
                  cNegFareData->comAmount() > 0)))
            {
              _warningMsg = MIX_COMMISSION;
              return true;
            }
          }
          if (_cat35NetTicketing && !cat35NetTKTFound)
          {
            cat35NetTKTFound = true;
          }
        }
        else // non-Cat 35 fare
        {
          // accumulate totalSellingAmt for the commissions
          // (mix C35 non_IT/BT and publish fare)
          cNegFareData->totalSellingAmt() += fareUsage->paxTypeFare()->nucFareAmount();
          if (trx.getOptions()->isCat35Net()) // if WPNET entry
          {
            _warningMsg = NO_NET_FARE_AMOUNT;
            return true;
          }
          cNegFareData->netTotalAmt() += fareUsage->paxTypeFare()->nucFareAmount();
          if (_isCmdPricing) // Net Remit mixes with non-Net Remit
          {
            if (cat35NetRemitFound)
            {
              _warningMsg = MIXED_FARES_COMBINATION; // Net Remit combines with non-Net Remit
              return true;
            }
            if (cNegFareData->indNetGross() != BLANK ||
                cNegFareData->comPercent() != RuleConst::PERCENT_NO_APPL ||
                cNegFareData->comAmount() > 0)
            {
              _warningMsg = MIX_COMMISSION;
              return true;
            }
          }
        }
      }

      if (UNLIKELY(_isCmdPricing))
      {
        paxTypeFares.push_back(fareUsage->paxTypeFare());
      }
    }

    if (pricingUnit->hrtojNetPlusUp())
    {
      _netTotalAmt += pricingUnit->hrtojNetPlusUp()->plusUpAmount;
      cNegFareData->netTotalAmt() += pricingUnit->hrtojNetPlusUp()->plusUpAmount;
    }

    if (pricingUnit->hrtcNetPlusUp())
    {
      _netTotalAmt += pricingUnit->hrtcNetPlusUp()->plusUpAmount;
      cNegFareData->netTotalAmt() += pricingUnit->hrtcNetPlusUp()->plusUpAmount;
    }
  }

  if (_isCmdPricing && (cat35NetTKTFound || cat35NetRemitFound))
  {
    if (!validateCat35Combination(trx, fPath, paxTypeFares, cat35NetRemitFound))
      return true;
  }

  if (UNLIKELY(dg && diag->isActive()))
  {
    diag->flushMsg();
  }

  if (!fallback::fallbackBSPMt3For1S(&trx) ? !_axessUser : (_abacusUser || _infiniUser || _isGnrUser))
  {
    if (cat35NetRemitFound)
    {
      processGlobalNetRemit(trx, fPath, cNegFareData, dg, diag);
      return true;
    }
    else if (cat35NetTKTFound)
    {
      findTktRestrictedFare(fPath);
      return true;
    }
  }

  if (trx.getOptions()->isCat35Net() && cat35Found)
  {
    return true;
  }

  if (LIKELY(!_indicatorTkt)) // not TKT
    findTktRestrictedFare(fPath); // Cat35 TFSF

  if (!cat35NetRemitFound && !cat35NetTKTFound)
  {
    return false; // there is no Cat35 processing or
    // Cat 35 fares are treated as regular private fares
  }

  return true;
}

// ------------------------------------------------------------------------------
//  validateCat35Combination()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::validateCat35Combination(
    PricingTrx& trx,
    FarePath& fPath,
    const std::vector<const PaxTypeFare*>& paxTypeFares,
    bool cat35NetRemitFound)
{
  if (TrxUtil::optimusNetRemitEnabled(trx) && !validatePrintOptionCombination(paxTypeFares))
  {
    _warningMsg = MULTIPLE_PRINT_OPTION;
    return false;
  }

  if (!checkForTourCodesConflict(trx, paxTypeFares))
  {
    _warningMsg = MULTIPLE_TOUR_CODE;
    return false;
  }

  if (!validateTourCodeCombination(trx, paxTypeFares))
  {
    if (TrxUtil::cat35LtypeEnabled(trx))
      _warningMsg = MULTIPLE_TOUR_CODE;
    else
      _warningMsg = cat35NetRemitFound ? MIX_FARES : MULTIPLE_TOUR_CODE;
    return false;
  }

  if (TrxUtil::optimusNetRemitEnabled(trx))
  {
    bool dynamic = ValueCodeUtil::hasDynamicValueCodeFare(fPath);

    if ((dynamic && !ValueCodeUtil::validateDynamicValueCodeCombination(trx, fPath)) ||
        (!dynamic && !ValueCodeUtil::validateStaticValueCodeCombination(trx, fPath)))
    {
      _warningMsg = MULTIPLE_VALUE_CODE;
      return false;
    }
  }

  if (TrxUtil::tfdNetRemitFareCombEnabled(trx) && !validateTFDCombination(trx, paxTypeFares))
  {
    _warningMsg = CONFLICTING_TFD_BYTE101;
    return false;
  }

  if (((_abacusUser && TrxUtil::optimusNetRemitEnabled(trx)) || _infiniUser || _isGnrUser) &&
      cat35NetRemitFound && !paxTypeFares.empty() && !validateFareBoxCombination(paxTypeFares))
  {
    return false;
  }

  return true;
}

// ------------------------------------------------------------------------------
//  processGlobalNetRemit()
// ------------------------------------------------------------------------------
void
NegotiatedFareRuleUtil::processGlobalNetRemit(PricingTrx& trx,
                                              FarePath& fPath,
                                              CollectedNegFareData* cNegFareData,
                                              bool dg,
                                              Diag535Collector* diag)
{
  if (_bspMethod == RuleConst::NRR_METHOD_3 && !_indicatorTkt &&
      !LocUtil::isKorea(*(trx.getRequest()->ticketingAgent()->agentLocation())))
  {
    _warningMsg = AUTO_TKT_NOT_PERMITTED;
    return;
  }

  if (_bspMethod == RuleConst::NRR_METHOD_3 && !_indicatorTkt &&
      (_tktFareDataInd == ' ' || _tktFareDataInd == RuleConst::NR_VALUE_F))
  {
    MoneyAmount cat35NetTotalAmt = cNegFareData->netTotalAmt();
    cat35NetTotalAmt += cNegFareData->totalMileageCharges();
    cat35NetTotalAmt += cNegFareData->otherSurchargeTotalAmt();
    cat35NetTotalAmt += cNegFareData->cat12SurchargeTotalAmt();

    if (cat35NetTotalAmt - fPath.getTotalNUCAmount() > EPSILON)
    {
      _warningMsg = NET_FARE_AMOUNT_EXCEEDS_FARE;
      return;
    }
  }
  if (!fallback::fallbackBSPMt3For1S(&trx) ? (!_axessUser && _processFareBox) :
      (_abacusUser || _infiniUser || _isGnrUser) && _processFareBox)
  {
    if (_tktFareDataInd == RuleConst::NR_VALUE_N || _tktFareDataInd == RuleConst::NR_VALUE_F ||
        _tktFareDataInd == RuleConst::NR_VALUE_N || _tktFareDataInd == RuleConst::BLANK)
    {
      fPath.selectedNetRemitFareCombo() = true;
    }
  }
  else
  {
    if (_tktFareDataInd == RuleConst::NR_VALUE_N)
      fPath.selectedNetRemitFareCombo() = true;
  }

  if (TrxUtil::optimusNetRemitEnabled(trx))
  {
    if ((_tktFareDataInd == RuleConst::BLANK || _tktFareDataInd == RuleConst::NR_VALUE_N) &&
        !_isCmdPricing)
    {
      findTktRestrictedFare(fPath);
      return;
    }

    if (processNetRemit(trx, fPath))
    {
      if (UNLIKELY(dg && diag->isActive() && (_tktFareDataInd == RuleConst::NR_VALUE_A ||
                                               _tktFareDataInd == RuleConst::NR_VALUE_F ||
                                               _tktFareDataInd == RuleConst::NR_VALUE_B)))
      {
        *diag << " NET REMIT FARE SELECTED\n";
        *diag << "************************************************************\n";
        diag->flushMsg();
      }
      findTktRestrictedFare(fPath);
    }
  }
  else // This portion can be removed after Optimus Net Remit project is implemented.
  {
    if (_tktFareDataInd == RuleConst::BLANK || _tktFareDataInd == RuleConst::NR_VALUE_N)
    {
      findTktRestrictedFare(fPath);
      return;
    }

    if (processNetRemit(trx, fPath))
    {
      if (UNLIKELY(dg && diag->isActive()))
      {
        *diag << " NET REMIT FARE SELECTED\n";
        *diag << "************************************************************\n";
        diag->flushMsg();
      }
      findTktRestrictedFare(fPath);
    }
  }

  return;
}

// ------------------------------------------------------------------------------
//  checkTktDataCat35()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::checkTktDataCat35(PricingTrx& trx,
                                          FarePath& fPath,
                                          FareUsage& fu,
                                          CollectedNegFareData* cNegFareData,
                                          bool& firstCat35FareProcessed)
{
  bool rc = true;
  _warningMsg = NO_WARNING;

  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  const NegFareRest* negFareRest = nullptr;

  PaxTypeFare* ptf = fu.paxTypeFare();

  negFareRest = getCat35Record3(ptf, negPaxTypeFare);
  if (!negFareRest)
  {
    _warningMsg = SYSTEM_ERROR; // hard error
    return false;
  }

  if (trx.getOptions()->isCat35Net() && negPaxTypeFare->cat35Level() == NET_LEVEL_NOT_ALLOWED)
  {
    _warningMsg = NO_NET_FARE_AMOUNT;
    return true;
  }

  if (trx.getOptions()->isCat35Net()) // WPNET
  {
    if (fu.paxTypeFare()->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
        negFareRest->negFareCalcTblItemNo() == 0) // Tyle L with no NET
    {
      _warningMsg = NO_NET_FARE_AMOUNT;
      return true;
    }
  }

  rc = checkTktDataCat35Fare(trx, fPath, fu, ptf, negFareRest, cNegFareData, negPaxTypeFare);

  accumulateCommissionAmt(trx, cNegFareData);

  if (!firstCat35FareProcessed &&
      (trx.getOptions()->isCat35Net() || _cat35NetRemit || _cat35NetTicketing))
  {
    processFirstCat35Fare(trx, *fPath.itin(), fu, ptf, negFareRest, cNegFareData, negPaxTypeFare);
    firstCat35FareProcessed = true;
    return true;
  }

  // compare cat35 data for each fare in the final combo
  if (rc) // The cat35 rule data is o'K for the current rule
  { // check combo
    // check multi cat35 fare conditions
    // compare Cat 35 tkt data with another Cat 35 fare
    if (_cat35NetTicketing)
    {
      processCat35NetTicketingFare(cNegFareData);
    }
    // compare Cat 35 Net Remit tkt data with another Cat 35 fare
    else if (_cat35NetRemit)
    {
      processCat35NetRemitFare(trx, *fPath.itin(), fu, negFareRest, cNegFareData);
    }
  }

  return true;
}

void
NegotiatedFareRuleUtil::accumulateCommissionAmt(PricingTrx& trx, CollectedNegFareData* cNegFareData)
{
  if (_comAmount != 0)
  {
    if (_currency != _paymentCurrency)
    {
      Money targetMoney(_paymentCurrency);

      CurrencyConversionFacade ccFacade;

      // convert commission amount to the BaseFare currency
      targetMoney.value() = 0;
      Money sourceMoneyCalculation(_comAmount, _currency);

      if (ccFacade.convert(
              targetMoney, sourceMoneyCalculation, trx, false, CurrencyConversionRequest::FARES))
      {
        _comAmount = targetMoney.value();
      }
      else
      {
        _comAmount = 0; // ??
      }
      _noComPerDec = targetMoney.noDec();
      _currency = _paymentCurrency;
    }

    cNegFareData->comAmount() += _comAmount; // accumulate a flat commissions in BaseFare currency.
    cNegFareData->currency() = _paymentCurrency;

    if (_cat35NetRemit)
    {
      _invalidNetRemComm = true; // Low priority in Warnings/Error
    }
  }
}

// ------------------------------------------------------------------------------
//  checkTktDataCat35Fare()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::checkTktDataCat35Fare(PricingTrx& trx,
                                              FarePath& fPath,
                                              FareUsage& fu,
                                              PaxTypeFare* ptf,
                                              const NegFareRest* negFareRest,
                                              CollectedNegFareData* cNegFareData,
                                              NegPaxTypeFareRuleData* negPaxTypeFare)

{
  // set up defaults :
  _indTypeTour = BLANK;
  _tourCode.clear();
  _fareBox.clear();
  _indNetGross = BLANK;
  _comPercent = RuleConst::PERCENT_NO_APPL; // Not applicable
  _noComPerDec = 0;
  _comAmount = 0;
  _noDec = 0;
  _currency.clear(); // Not applicable
  _bspMethod = BLANK;
  _tDesignator.clear();

  bool useRedistributedWholeSale =
    cNegFareData->shouldUseRedistributedWholeSale() ||
    (!fallback::fallbackFixPQNRedistributedWholesale(&trx) &&
     shouldUseRedistributedWholeSale(&trx, *ptf) &&
     (negPaxTypeFare->tktIndicator() == 'N'));

  cNegFareData->setUseRedistributedWholeSale(useRedistributedWholeSale);

  MoneyAmount baseNetAmount = useRedistributedWholeSale ?
                                negPaxTypeFare->wholeSaleNucNetAmount() :
                                negPaxTypeFare->nucNetAmount();

  // Calculate a total net & selling fare amounts per PAX
  if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
      negFareRest->negFareCalcTblItemNo() == 0)
    cNegFareData->netTotalAmt() += ptf->nucFareAmount();
  else
    cNegFareData->netTotalAmt() += baseNetAmount;

  cNegFareData->totalSellingAmt() += ptf->nucFareAmount();

  // Collect mileage surcharges in the cNegFareData object for CR#1
  // Later on these charges will be accumulated with other 8,9,12 charges if needed
  if (ptf->mileageSurchargeAmt() > 0 && ptf->mileageSurchargePctg() > 0)
  {
    MoneyAmount mileageSurchargeAmt;
    if (!fallback::fallbackAMCFixMissingMileageNetAmtCat35L(&trx))
    {
      if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
          negFareRest->negFareCalcTblItemNo() == 0)
        mileageSurchargeAmt =
          (ptf->nucFareAmount() * ptf->mileageSurchargePctg()) / RuleConst::HUNDRED_PERCENTS;
      else
        mileageSurchargeAmt =
          (baseNetAmount * ptf->mileageSurchargePctg()) / RuleConst::HUNDRED_PERCENTS;
    }
    else
    {
      mileageSurchargeAmt =
          (negPaxTypeFare->nucNetAmount() * ptf->mileageSurchargePctg()) /
          RuleConst::HUNDRED_PERCENTS;
    }
    CurrencyUtil::truncateNUCAmount(mileageSurchargeAmt);
    cNegFareData->totalMileageCharges() += mileageSurchargeAmt;
  }

  // Collect Mileage, Cat 8, 9, 12 surcharges for Cat 35 Commission calculation

  cNegFareData->totalSellingMileageCharges() += ptf->mileageSurchargeAmt();
  cNegFareData->otherSurchargeTotalAmt() += fu.stopOverAmt();
  cNegFareData->otherSurchargeTotalAmt() += fu.transferAmt();
  cNegFareData->cat12SurchargeTotalAmt() += fu.surchargeAmt();
  cNegFareData->cat8SurchargeTotalAmt() += fu.stopOverAmt();
  cNegFareData->cat9SurchargeTotalAmt() += fu.transferAmt();

  if (!trx.getOptions()->isCat35Sell() ||
      (trx.getOptions()->isCat35Sell() && trx.getOptions()->isCat35Net()))
  // if not tkt selling fares or tkt selling fare and WPNET entry
  {

    if (trx.getOptions()->isCat35Net())
    {
      fu.netCat35NucUsed(); // For TAX's..
      if (!NegotiatedFareRuleUtil::shouldUseRedistributedWholeSale(&trx, *ptf))
      {
        fu.netCat35NucAmount() =
          negPaxTypeFare->nucNetAmount(); // all amt's are in calculate currency..
      }
      else
      {
        fu.netCat35NucAmount() =
          negPaxTypeFare->wholeSaleNucNetAmount();
      }
      ptf->nucFareAmount() =
          fu.netCat35NucAmount(); // swap net/selling fare amounts ( for FareCalc)
      if (ptf->mileageSurchargeAmt() > 0 && ptf->mileageSurchargePctg() > 0)
      {
        ptf->mileageSurchargeAmt() =
            (ptf->nucFareAmount() * ptf->mileageSurchargePctg()) / RuleConst::HUNDRED_PERCENTS;
        CurrencyUtil::truncateNUCAmount(ptf->mileageSurchargeAmt());
      }
      _netAmtWasFlipped = true;
      _netTotalAmt += fu.totalFareAmount(); // Cat 35 Net Amount and all plus up amounts
    }
  }

  // save commission amount/percentage in payment currency
  saveCommissions(trx, negFareRest, negPaxTypeFare, ptf);

  // -------------------
  // Net Remit Process
  // -------------------
  bool allowNetRemit =
      ((negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2 &&
        TrxUtil::isNetRemitEnabled(trx)) ||
      (!fallback::fallbackBSPMt3For1S(&trx) ? negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3 && !_axessUser
        :
       negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3 && (_abacusUser || _infiniUser)) ||
       (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_1 && _infiniUser));

  if (allowNetRemit)
  {
    _cat35NetRemit = true;
    // Save Cat 35 Net Remit data
    _bspMethod = negFareRest->netRemitMethod();
    _indNetGross = negFareRest->netGrossInd();
    _comPercent = negFareRest->commPercent();
    _indTypeTour = negFareRest->tourBoxCodeType1();
    _tourCode = negFareRest->tourBoxCode1();

    if (checkFareBox(negFareRest->fareBoxText1()))
    {
      _fareBox = negFareRest->fareBoxText1();
    }

    if (_isCmdPricing && _infiniUser && !validateNetRemitMethod1(negFareRest))
    {
      _warningMsg = INVALID_NET_REMIT_FARE;
    }

    return true;
  }
  else
  { // CAT 35 TFSF
    if (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2)
    {
      _bspMethod = negFareRest->netRemitMethod();
      _indNetGross = negFareRest->netGrossInd();
      _comPercent = negFareRest->commPercent();
      _indTypeTour = negFareRest->tourBoxCodeType1();
      _tourCode = negFareRest->tourBoxCode1();
      _fareBox = negFareRest->fareBoxText1();
    }
    else
    {
      _bspMethod = negFareRest->netRemitMethod();
    }
  }

  if (trx.getOptions()->isCat35Net())
  {
    return true;
  }

  // save Method Type ind. = 1/2/3/4/5/blank
  // vk _bspMethod = negFareRest->netRemitMethod();

  // save NetGross ind.   = N/G/B/blank
  _indNetGross = negFareRest->netGrossInd();

  // check & save fareBoxText
  if (!validateFareBoxText(negFareRest))
  {
    if (_warningMsg == INVALID_ITBT_PSG_COUPON)
      return false;
    _warningMsg = NO_WARNING;
  }
  // check & save method type
  if (!validateBSPMethod(trx, negFareRest))
  {
    return false;
  }
  // check & save Tour Type  (B/C/V/T/blank)  and Tour Code value
  if (!validateTourCode(trx, negFareRest, cNegFareData))
  {
    if (_warningMsg == MULTIPLE_TOUR_CODE)
      return false;
    _warningMsg = NO_WARNING;
  }

  // Cat 35 display type L without Net amounts
  // If the tour code, fare box text and commission fields are not populated,
  // it will be treated as regular private fare (Use cat35 for security only)
  if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
      negFareRest->negFareCalcTblItemNo() == 0 && _tourCode.empty() &&
      negFareRest->fareBoxText1().empty() && negFareRest->fareBoxText2().empty() &&
      negFareRest->commAmt1() == 0 && negFareRest->commAmt2() == 0 &&
      negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL)
  {
    return true;
  }
  _cat35NetTicketing = true;
  return true;
}

// ------------------------------------------------------------------------------
//  validateBSPMethod()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::validateBSPMethod(PricingTrx& trx, const NegFareRest* negFareRest)
{
  _bspMethod = negFareRest->netRemitMethod();

  if (_bspMethod != BLANK)
  {
    _warningMsg = ITBT_BSP_NOT_BLANK;
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------------
//  validateTourCode()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::validateTourCode(PricingTrx& trx,
                                         const NegFareRest* negFareRest,
                                         CollectedNegFareData* cNegFareData)
{
  if (negFareRest->noSegs() == ONE_SEGMENT)
  {
    // save tour code
    _indTypeTour = negFareRest->tourBoxCodeType1();
    _tourCode = negFareRest->tourBoxCode1();
  }
  // the following logic for 2 segments (Audit/Passenger coupons)
  else if (negFareRest->noSegs() == TWO_SEGMENTS)
  {
    // check/save tour code
    if (negFareRest->tourBoxCodeType1() != negFareRest->tourBoxCodeType2() ||
        negFareRest->tourBoxCode1() != negFareRest->tourBoxCode2())
    {
      _warningMsg = MULTIPLE_TOUR_CODE;
      return false;
    }
    _indTypeTour = negFareRest->tourBoxCodeType1();
    _tourCode = negFareRest->tourBoxCode1();
  }
  if (_tourCode.empty())
  {
    _warningMsg = TOUR_CODE_NOT_FOUND;
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------------
//  validateFareBoxText()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::validateFareBoxText(const NegFareRest* negFareRest)
{
  if (negFareRest->noSegs() == ONE_SEGMENT)
  {
    const std::string& fbt = negFareRest->fareBoxText1();
    bool fareBoxValid = checkFareBox(fbt);

    if (negFareRest->couponInd1() == AUDIT_COUPON || !fareBoxValid)
    {
      _warningMsg = NOT_ITBT;
      return false;
    }
    _fareBox = negFareRest->fareBoxText1();
  }
  else if (negFareRest->noSegs() == TWO_SEGMENTS)
  {
    const std::string& fbt1 = negFareRest->fareBoxText1();
    const std::string& fbt2 = negFareRest->fareBoxText2();
    if (negFareRest->couponInd1() == PSG_COUPON)
    {
      if (!checkFareBoxTextITBT(fbt1, fbt2))
        return false;

      _fareBox = negFareRest->fareBoxText1();
    }
    else if (negFareRest->couponInd2() == PSG_COUPON)
    {
      if (!checkFareBoxTextITBT(fbt2, fbt1))
        return false;

      _fareBox = negFareRest->fareBoxText2();
    }

    bool fareBoxValid = checkFareBox(_fareBox);

    if (!fareBoxValid)
    {
      _warningMsg = NOT_ITBT;
      return false;
    }
  }
  else
  {
    _warningMsg = NOT_ITBT;
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------------
//  checkFareBoxTextITBT()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::checkFareBoxTextITBT(const std::string& firstFBTWithPsgrCoupon,
                                             const std::string& secondFBT)
{
  if ((firstFBTWithPsgrCoupon.compare(0, IT_TICKET.size(), IT_TICKET) == 0 &&
       secondFBT.compare(0, BT_TICKET.size(), BT_TICKET) == 0) ||
      (firstFBTWithPsgrCoupon.compare(0, BT_TICKET.size(), BT_TICKET) == 0 &&
       secondFBT.compare(0, IT_TICKET.size(), IT_TICKET) == 0) ||
      (firstFBTWithPsgrCoupon.empty() && secondFBT.compare(0, IT_TICKET.size(), IT_TICKET) == 0) ||
      (firstFBTWithPsgrCoupon.empty() && secondFBT.compare(0, BT_TICKET.size(), BT_TICKET) == 0))
  {
    _warningMsg = INVALID_ITBT_PSG_COUPON;
    return false;
  }
  else
    return true;
}

// ------------------------------------------------------------------------------
//  checkFareBox()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::checkFareBox(const std::string& fareBoxText)
{
  return (fareBoxText == IT_TICKET || fareBoxText == BT_TICKET);
}

// ------------------------------------------------------------------------------
//  saveCommissions()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::saveCommissions(PricingTrx& trx,
                                        const NegFareRest* negFareRest,
                                        const NegPaxTypeFareRuleData* negPaxTypeFare,
                                        const PaxTypeFare* ptf)
{
  _comPercent = RuleConst::PERCENT_NO_APPL;
  _noComPerDec = 0;
  _comAmount = 0; // commission amount
  _currency.clear(); // selected commission currency
  _noDec = 0;

  if (negFareRest->commAmt1() == 0 && negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL &&
      (negFareRest->cur1() == BLANK_CURRENCY || negFareRest->cur1().empty()))
  {
    if (_isGnrUser && negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2 &&
        trx.getRequest() && trx.getRequest()->ticketingAgent() &&
        trx.getRequest()->ticketingAgent()->commissionAmount() != 0)
    {
      _warningMsg = INVALID_NET_REMIT_COMM;
    }
    return true;
  }

  CurrencyCode cc = negPaxTypeFare->calculatedNegCurrency();
  // the selected currency for the commission should be the same as
  // a calculated currency for cat35 amount.

  if (negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL)
  {
    if (cc == negFareRest->cur1() || (!negFareRest->cur1().empty() && negFareRest->cur2().empty()))
    {
      _comAmount = negFareRest->commAmt1();
      _currency = negFareRest->cur1();
      _noDec = negFareRest->noDec1();
    }
    else if (cc == negFareRest->cur2() ||
             (!negFareRest->cur2().empty() && negFareRest->cur1().empty()))
    {
      _comAmount = negFareRest->commAmt2();
      _currency = negFareRest->cur2();
      _noDec = negFareRest->noDec2();
    }
    else if (!negFareRest->cur1().empty() && !negFareRest->cur2().empty())
    {
      MoneyAmount commAmountNuc1 = 0;
      MoneyAmount commAmountNuc2 = 0;

      Money commAmt1(negFareRest->commAmt1(), negFareRest->cur1());
      Money nuc1("NUC");
      CurrencyConversionRequest request(nuc1,
                                        commAmt1,
                                        trx.getRequest()->ticketingDT(),
                                        *(trx.getRequest()),
                                        trx.dataHandle(),
                                        ptf->fare()->isInternational());

      NUCCurrencyConverter ncc;
      if (ncc.convert(request, nullptr))
      {
        commAmountNuc1 = nuc1.value();
      }

      Money commAmt2(negFareRest->commAmt2(), negFareRest->cur2());
      Money nuc2("NUC");

      CurrencyConversionRequest request2(nuc2,
                                         commAmt2,
                                         trx.getRequest()->ticketingDT(),
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         ptf->fare()->isInternational());
      NUCCurrencyConverter ncc2;
      if (ncc2.convert(request2, nullptr))
      {
        commAmountNuc2 = nuc2.value();
      }

      if (commAmountNuc1 < commAmountNuc2)
      {
        _comAmount = negFareRest->commAmt1();
        _currency = negFareRest->cur1();
        _noDec = negFareRest->noDec1();
      }
      else if (negFareRest->commAmt2() != 0)
      {
        _comAmount = negFareRest->commAmt2();
        _currency = negFareRest->cur2();
        _noDec = negFareRest->noDec2();
      }
    }
  }
  else
  {
    _comPercent = negFareRest->commPercent();
  }

  if (_comAmount != 0)
  {
    if (ptf->isRoundTrip())
      _comAmount = _comAmount / 2;
  }

  return true;
}

// ------------------------------------------------------------------------------
//  processNetRemit()
// ------------------------------------------------------------------------------
bool
NegotiatedFareRuleUtil::processNetRemit(PricingTrx& trx, FarePath& fPath)
{
  std::vector<PricingUnit*>::const_iterator puIt = fPath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = fPath.pricingUnit().end();

  bool hasCmdPricingFare = checkCmdPricingFare(trx, fPath);

  for (; puIt != puItEnd; ++puIt)
  {
    // lint -e{578}
    PricingUnit& pricingUnit = **puIt;
    if (TrxUtil::optimusNetRemitEnabled(trx) && hasCmdPricingFare)
    {
      NetRemitPscMatchUtil nrMatch(pricingUnit);
      if (!nrMatch.process())
      {
        _warningMsg = INVALID_NET_REMIT_FARE;
        return false;
      }

      if (_tktFareDataInd == RuleConst::BLANK || _tktFareDataInd == RuleConst::NR_VALUE_N)
        continue;
    }

    std::vector<FareUsage*>::iterator fareUsageI = pricingUnit.fareUsage().begin();
    std::vector<FareUsage*>::iterator fareUsageEnd = pricingUnit.fareUsage().end();

    for (; fareUsageI != fareUsageEnd; ++fareUsageI)
    {
      FareUsage& fu = **fareUsageI;

      NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
      const NegFareRest* negFareRest = nullptr;

      PaxTypeFare* ptf = fu.paxTypeFare();

      negFareRest = getCat35Record3(ptf, negPaxTypeFare);
      if (!negFareRest)
      {
        _warningMsg = SYSTEM_ERROR; // hard error
        return false;
      }

      bool ret = fu.tktNetRemitFare() || !fu.netRemitPscResults().empty();

      if ((!_abacusUser && !_infiniUser) || !TrxUtil::tfdNetRemitFareCombEnabled(trx) ||
          (hasCmdPricingFare))
      {
        ret = processNetRemitFareSelection(trx, fPath, pricingUnit, fu, *negFareRest);
      }

      if (ret)
      {
        fPath.selectedNetRemitFareCombo() = true; // all Fares should be selected.
      }
      else
      {
        fPath.selectedNetRemitFareCombo() = false;
        _warningMsg = INVALID_NET_REMIT_FARE;
        return false;
      }
    }
  }
  return true;
}

bool
NegotiatedFareRuleUtil::processNetRemitFareSelection(PricingTrx& trx,
                                                     const FarePath& farePath,
                                                     PricingUnit& pricingUnit,
                                                     FareUsage& fareUsage,
                                                     const NegFareRest& negFareRest) const
{
  return NetRemitFareSelection::processNetRemitFareSelection(
      trx, farePath, pricingUnit, fareUsage, negFareRest);
}

// ------------------------------------------------------------------------------
//  findTktRestrictedFare()
// ------------------------------------------------------------------------------
void
NegotiatedFareRuleUtil::findTktRestrictedFare(FarePath& fPath)
{
  if (_indicatorTkt)
    return;

  for (const auto pricingUnit : fPath.pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      const auto* ptf = fareUsage->paxTypeFare();

      if (!(ptf->isNegotiated()))
        continue;

      const NegPaxTypeFareRuleData* negPaxTypeFare = ptf->getNegRuleData();
      if (negPaxTypeFare == nullptr)
      {
        _warningMsg = SYSTEM_ERROR; // hard error
        return;
      }

      if (negPaxTypeFare->tktIndicator() == 'N')
      {
        _warningMsg = AUTO_TKT_NOT_PERMITTED;

        if (_infiniUser || _abacusUser)
          fPath.tktRestricted() = true;

        return;
      }
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareRuleUtil::getCat18Endorsement
//
// Description:  To access Cat 18 Endorsement from fare usage
//
// </PRE>
// ----------------------------------------------------------------------------
void
NegotiatedFareRuleUtil::getCat18Endorsement(const PricingTrx& trx,
                                            const Itin& itin,
                                            const FareUsage& fu,
                                            std::string& endorsementTxt,
                                            std::string& valueCode)
{
  if (fu.tktEndorsement().size() >= 1)
  {
    std::vector<TicketEndorseItem>& tktEndorsement = const_cast<FareUsage&>(fu).tktEndorsement();

    Indicator tktLocInd = tktEndorsement.front().tktLocInd;
    if (tktLocInd == '1' || tktLocInd == '3' || tktLocInd == '5')
    {
      valueCode = tktEndorsement.front().endorsementTxt;
    }

    // Make sure Value Codes are not in endorsement anymore

    TicketingEndorsement tktEndo;
    TicketEndorseLine* line;

    if (!fallback::fallbackEndorsementsRefactoring(&trx))
    {
      line = tktEndo.sortAndGlue(trx, itin, const_cast<FareUsage&>(fu), EndorseCutter());
    }
    else
    {
      line = tktEndo.sortAndGlue(trx, itin, const_cast<FareUsage&>(fu), EndorseCutterUnlimited());
    }

    if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
    {
      if (line && !line->endorseMessage.empty())
        endorsementTxt = line->endorseMessage;
    }
    else
    {
      if (line->endorseMessage.size() > 0)
        endorsementTxt = line->endorseMessage;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void NegotiatedFareRuleUtil::processAxessCat35
//
// Description:  To process Cat 35 for JAL/AXESS
//
// </PRE>
// ----------------------------------------------------------------------------
void
NegotiatedFareRuleUtil::processAxessCat35(PricingTrx& trx,
                                          FarePath& fPath,
                                          CollectedNegFareData* cNegFareData)
{
  Diag535Collector* diag = nullptr;
  bool dg = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic535))
  {
    DCFactory* factory = DCFactory::instance();
    diag = dynamic_cast<Diag535Collector*>(factory->create(trx));
    diag->enable(Diagnostic535);
    dg = true;
    diag->diag535Request(trx, fPath);
  }

  bool firstFareProcessed = false;
  bool rc = true;
  _warningMsg = NO_WARNING;

  _isCmdPricing = checkCmdPricingFare(trx, fPath);

  std::vector<const PaxTypeFare*> paxTypeFares;

  for (const PricingUnit* pricingUnit : fPath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (UNLIKELY(dg && diag->isActive()))
      {
        diag->diag535Collector(trx, *fPath.itin(), *fareUsage);
      }

      rc = checkTktDataAxessCat35(trx, *fareUsage, cNegFareData, firstFareProcessed);

      if (!rc)
      {
        break;
      }
      if (_isCmdPricing)
      {
        paxTypeFares.push_back(fareUsage->paxTypeFare());
      }
    }

    if (!rc)
    {
      break;
    }

    if (pricingUnit->hrtojNetPlusUp())
    {
      _netTotalAmt += pricingUnit->hrtojNetPlusUp()->plusUpAmount;
      cNegFareData->netTotalAmt() += pricingUnit->hrtojNetPlusUp()->plusUpAmount;
    }

    if (pricingUnit->hrtcNetPlusUp())
    {
      _netTotalAmt += pricingUnit->hrtcNetPlusUp()->plusUpAmount;
      cNegFareData->netTotalAmt() += pricingUnit->hrtcNetPlusUp()->plusUpAmount;
    }
  }

  if (_isCmdPricing && (!checkForTourCodesConflict(trx, paxTypeFares) ||
                        !validateTourCodeCombination(trx, paxTypeFares)))
  {
    cNegFareData->differentCode() = true;
    _warningMsg = ISSUE_SEPARATE_TKT;
  }

  if (UNLIKELY(dg && diag->isActive()))
  {
    if (_warningMsg != NO_WARNING)
    {
      diag->diag535Message(trx, _warningMsg);
    }
    diag->flushMsg();
  }

  fPath.collectedNegFareData() = cNegFareData;

  if (_warningMsg != NO_WARNING)
  {
    if (_warningMsg == ISSUE_SEPARATE_TKT)
    {
      cNegFareData->trailerMsg() = "ISSUE SEPARATE TICKETS";
    }
    else if (_warningMsg == FARES_NOT_COMBINABLE)
    {
      throw tse::ErrorResponseException(ErrorResponseException::NO_FARES_RBD_CARRIER);
    }
    else if (_warningMsg == INVALID_NET_REMIT_FARE)
    {
      cNegFareData->trailerMsg() = "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
    }
    else
    {
      cNegFareData->trailerMsg() = "SYSTEM ERROR";
      return;
    }
  }

  // To select the published (ticketed) fares for wpnett
  if (trx.getRequest()->isWpNettRequested())
  {
    if (_tktFareDataInd == RuleConst::NR_VALUE_A)
    {
      if (processNetRemit(trx, fPath))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << " PUBLISHED FARE SELECTED FOR JAL AXESS USER\n";
          *diag << "************************************************************\n";
          diag->flushMsg();
        }
      }
    }
  }
  // WPSEL entry does not select the published (ticketing) fares
  else
  {
    if (UNLIKELY(dg && diag->isActive()))
    {
      *diag << " WPSEL IS EXEMPT FOR TFD RETRIEVAL\n";
      *diag << "************************************************************\n";
      diag->flushMsg();
    }
  }

  if (_netAmtWasFlipped) // was flipped?
  {
    fPath.setTotalNUCAmount(_netTotalAmt);
  }
}

bool
NegotiatedFareRuleUtil::checkTktDataAxessCat35(PricingTrx& trx,
                                               FareUsage& fu,
                                               CollectedNegFareData* cNegFareData,
                                               bool& firstFareProcessed)
{
  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  const NegFareRest* negFareRest = nullptr;

  PaxTypeFare* ptf = fu.paxTypeFare();

  negFareRest = getCat35Record3(ptf, negPaxTypeFare);
  if (!negFareRest)
  {
    _warningMsg = SYSTEM_ERROR; // hard error
    return false;
  }

  // Calculate a total net & selling fare amounts per PAX
  cNegFareData->netTotalAmt() += negPaxTypeFare->nucNetAmount();
  cNegFareData->totalSellingAmt() += ptf->nucFareAmount();

  // Collect mileage surcharges in the cNegFareData object for CR#1
  // Later on these charges will be accumulated with other 8,9,12 charges if needed
  if (ptf->mileageSurchargeAmt() > 0 && ptf->mileageSurchargePctg() > 0)
  {
    MoneyAmount mileageSurchargeAmt =
        (negPaxTypeFare->nucNetAmount() * ptf->mileageSurchargePctg()) /
        RuleConst::HUNDRED_PERCENTS;
    CurrencyUtil::truncateNUCAmount(mileageSurchargeAmt);
    cNegFareData->totalMileageCharges() += mileageSurchargeAmt;
  }

  if (trx.getRequest()->isWpNettRequested())
  {
    fu.netCat35NucUsed(); // For TAX's..
    fu.netCat35NucAmount() =
        negPaxTypeFare->nucNetAmount(); // all amt's are in calculate currency..
    ptf->nucFareAmount() = fu.netCat35NucAmount(); // swap net/selling fare amounts ( for FareCalc)
    if (ptf->mileageSurchargeAmt() > 0 && ptf->mileageSurchargePctg() > 0)
    {
      ptf->mileageSurchargeAmt() =
          (ptf->nucFareAmount() * ptf->mileageSurchargePctg()) / RuleConst::HUNDRED_PERCENTS;
      CurrencyUtil::truncateNUCAmount(ptf->mileageSurchargeAmt());
    }
    _netAmtWasFlipped = true;
    _netTotalAmt += fu.totalFareAmount(); // Cat 35 Net Amount and all plus up amounts
  }

  if (!firstFareProcessed)
  {
    // Save Cat 35 Net Remit data
    _tktAppl = negFareRest->tktAppl();
    _tktCarrier = negFareRest->carrier();
    _tktFareDataInd = negFareRest->tktFareDataInd1();
    _indTypeTour = negFareRest->tourBoxCodeType1();
    _tourCode = negFareRest->tourBoxCode1();
    _indNetGross = negFareRest->netGrossInd();
    _comPercent = negFareRest->commPercent();
    _tDesignator = negFareRest->tktDesignator1();
    _owrt = negFareRest->owrt1();
    _globalDir = negFareRest->globalDir1();
    _ruleTariff = negFareRest->ruleTariff1();
    _carrier = negFareRest->carrier11();
    _rule = negFareRest->rule1();
    _bspMethod = negFareRest->netRemitMethod();
    _firstCat35Fare = ptf;

    // save final cat35 info.
    cNegFareData->fareTypeCode() = ptf->fcaDisplayCatType();
    cNegFareData->indTypeTour() = _indTypeTour;
    cNegFareData->tourCode() = _tourCode;
    cNegFareData->indNetGross() = _indNetGross;
    cNegFareData->comPercent() = _comPercent;
    cNegFareData->noComPerDec() = _noComPerDec;
    cNegFareData->tDesignator() = _tDesignator;
    cNegFareData->fareBox() = negFareRest->fareBoxText1();
    cNegFareData->currency() = negPaxTypeFare->calculatedNegCurrency();

    firstFareProcessed = true;
    return true;
  }
  else // compare cat35 data for each fare in the final combo
  {
    if (negFareRest->tktFareDataInd1() != _tktFareDataInd ||
        _tktFareDataInd != RuleConst::NR_VALUE_A)
    {
      _warningMsg = INVALID_NET_REMIT_FARE;
      _tktFareDataInd = BLANK;
    }
    else if (negFareRest->owrt1() != _owrt || negFareRest->globalDir1() != _globalDir ||
             negFareRest->ruleTariff1() != _ruleTariff || negFareRest->carrier11() != _carrier ||
             negFareRest->rule1() != _rule)
    {
      _warningMsg = ISSUE_SEPARATE_TKT;
    }

    if (_isCmdPricing && _warningMsg != ISSUE_SEPARATE_TKT &&
        (negFareRest->netRemitMethod() != _bspMethod ||
         negFareRest->netGrossInd() != _indNetGross || negFareRest->commPercent() != _comPercent))
    {
      _warningMsg = ISSUE_SEPARATE_TKT;
    }

    if (trx.getRequest()->isWpSelRequested())
    {
      if (ptf->fareClass() != _firstCat35Fare->fareClass() ||
          ptf->fareTariff() != _firstCat35Fare->fareTariff() ||
          ptf->ruleNumber() != _firstCat35Fare->ruleNumber())
      {
        cNegFareData->differentNetFare() = true;
      }
      else
      {
        const NegPaxTypeFareRuleData* firstNegPtfrData = _firstCat35Fare->getNegRuleData();
        if (firstNegPtfrData != nullptr && negPaxTypeFare->netAmount() != firstNegPtfrData->netAmount())
        {
          cNegFareData->differentNetFare() = true;
        }
      }
    }

    if (cNegFareData->tourCode().empty() && !negFareRest->tourBoxCode1().empty())
    {
      cNegFareData->indTypeTour() = negFareRest->tourBoxCodeType1();
      cNegFareData->tourCode() = negFareRest->tourBoxCode1();
    }

    if (cNegFareData->tDesignator() != negFareRest->tktDesignator1())
    {
      cNegFareData->differentTktDesg() = true;
    }

    if (_isCmdPricing && (cNegFareData->indNetGross() != negFareRest->netGrossInd() ||
                          cNegFareData->comPercent() != negFareRest->commPercent()))
    {
      cNegFareData->differentComm() = true;
    }
  } // combo

  return true;
}

void
NegotiatedFareRuleUtil::processFirstCat35Fare(PricingTrx& trx,
                                              const Itin& itin,
                                              FareUsage& fu,
                                              PaxTypeFare* ptf,
                                              const NegFareRest* negFareRest,
                                              CollectedNegFareData* cNegFareData,
                                              NegPaxTypeFareRuleData* negPaxTypeFare)
{
  // save final cat35 info.
  cNegFareData->tourCode() = _tourCode;
  cNegFareData->indTypeTour() = _indTypeTour;
  cNegFareData->fareBox() = _fareBox;
  cNegFareData->indNetGross() = _indNetGross;
  cNegFareData->comPercent() = _comPercent;
  cNegFareData->noComPerDec() = _noComPerDec;
  cNegFareData->noDec() = _noDec;
  cNegFareData->bspMethod() = _bspMethod;
  cNegFareData->tDesignator() = _tDesignator;

  if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
      negFareRest->negFareCalcTblItemNo() == 0)
    cNegFareData->fareTypeCode() = RuleConst::SELLING_FARE; // set up 'L'

  else if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
           negFareRest->negFareCalcTblItemNo() != 0)
    cNegFareData->fareTypeCode() =
        RuleConst::SELLING_FARE_NOT_FOR_SEC; // set up 'N' (internal logic)

  else
  {
    cNegFareData->fareTypeCode() = ptf->fcaDisplayCatType();
    if (ptf->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD &&
        negFareRest->negFareCalcTblItemNo() != 0 && negPaxTypeFare &&
        !negPaxTypeFare->isT979FareIndInRange())
      cNegFareData->cat35CSelling() = true;
  }

  if (_cat35NetRemit)
  {
    _tktAppl = negFareRest->tktAppl();
    _tktCarrier = negFareRest->carrier();
    _tktFareDataInd = negFareRest->tktFareDataInd1();

    if (!fallback::fallbackBSPMt3For1S(&trx) ? !_axessUser : (_abacusUser || _infiniUser))
    {
      _processFareBox = hasFareBoxAmount(negFareRest) && hasFareBoxNetRemitMethod(negFareRest);
    }

    if (_tktFareDataInd == RuleConst::BLANK)
    {
      if (negPaxTypeFare && negPaxTypeFare->negFareRestExt())
      {
        _tktFareDataInd = negPaxTypeFare->negFareRestExt()->fareBasisAmtInd();
      }
    }

    std::string endorsement;
    if (TrxUtil::optimusNetRemitEnabled(trx))
    {
      DataHandle dataHandle;
      FareUsage* dummyFU = fu.clone(dataHandle); // To avoid endorsement being erased in sortAndGlue
      getCat18Endorsement(trx, itin, *dummyFU, endorsement, cNegFareData->valueCode());
    }
    else
      getCat18Endorsement(trx, itin, fu, endorsement, cNegFareData->valueCode());
    _endorsementTxt = endorsement;
  }
}

void
NegotiatedFareRuleUtil::processCat35NetTicketingFare(CollectedNegFareData* cNegFareData)
{
  if (_isCmdPricing && cNegFareData->bspMethod() != RuleConst::NRR_METHOD_BLANK)
  {
    if (_bspMethod == RuleConst::NRR_METHOD_BLANK)
    {
      _warningMsg = MIXED_FARES_COMBINATION;
      return;
    }
    else if (cNegFareData->bspMethod() != _bspMethod)
    {
      _warningMsg = MULTIPLE_BSP;
      return;
    }
  }
  if (_isCmdPricing && cNegFareData->indNetGross() != _indNetGross)
  {
    _warningMsg = MULTIPLE_NET_GROSS;
    return;
  }
  if (_indicatorTkt)
  {
    if ((cNegFareData->comPercent() != RuleConst::PERCENT_NO_APPL && !_currency.empty()) ||
        (_comPercent != RuleConst::PERCENT_NO_APPL && !cNegFareData->currency().empty()))
    {
      _warningMsg = MIX_COMMISSION;
      return;
    }
    else if (cNegFareData->comPercent() != _comPercent)
    {
      _warningMsg = MULTIPLE_COMMISSION;
      return;
    }

  } // if Ticketing entry
  else
  {
    if (_isCmdPricing && cNegFareData->comPercent() != _comPercent)
    {
      _warningMsg = MULTIPLE_COMMISSION;
      return;
    }
  } // if Pricing entry
  if (cNegFareData->tourCode().empty() && !_tourCode.empty())
  {
    cNegFareData->indTypeTour() = _indTypeTour;
    cNegFareData->tourCode() = _tourCode;
  }
  if (cNegFareData->fareBox().empty() && !_fareBox.empty())
  {
    cNegFareData->fareBox() = _fareBox;
  }
}

void
NegotiatedFareRuleUtil::processCat35NetRemitFare(PricingTrx& trx,
                                                 const Itin& itin,
                                                 FareUsage& fu,
                                                 const NegFareRest* negFareRest,
                                                 CollectedNegFareData* cNegFareData)
{
  if (_isCmdPricing)
  {
    if (cNegFareData->bspMethod() == RuleConst::NRR_METHOD_BLANK)
    {
      _warningMsg = MIXED_FARES_COMBINATION;
      return;
    }
    else
    {
      if (cNegFareData->bspMethod() != _bspMethod)
      {
        _warningMsg = MIX_FARES;
        return;
      }
      else if (cNegFareData->indNetGross() != _indNetGross ||
               cNegFareData->comPercent() != _comPercent)
      {
        _warningMsg = TrxUtil::cat35LtypeEnabled(trx) ? MULTIPLE_COMMISSION : MIX_FARES;
        return;
      }
    }
  }

  if (!TrxUtil::tfdNetRemitFareCombEnabled(trx) &&
      _tktFareDataInd != negFareRest->tktFareDataInd1())
  {
    _warningMsg = MIX_FARES;
    return;
  }

  std::string endorsement;
  std::string valueCode;
  if (TrxUtil::optimusNetRemitEnabled(trx))
  {
    DataHandle dataHandle;
    FareUsage* dummyFU =
        fu.clone(dataHandle); // To avoid endorsement being erased in sortAndGlueGlue
    getCat18Endorsement(trx, itin, *dummyFU, endorsement, valueCode);
  }
  else
  {
    getCat18Endorsement(trx, itin, fu, endorsement, valueCode);
  }

  if (validateEndorsements(trx, endorsement))
  {
    return;
  }

  if (cNegFareData->tourCode().empty() && !_tourCode.empty())
  {
    cNegFareData->indTypeTour() = _indTypeTour;
    cNegFareData->tourCode() = _tourCode;
  }

  if (cNegFareData->fareBox().empty() && !_fareBox.empty())
  {
    cNegFareData->fareBox() = _fareBox;
  }
}

bool
NegotiatedFareRuleUtil::validateEndorsements(PricingTrx& trx, const std::string& endorsementTxt)
{
  if (!TrxUtil::cat35LtypeEnabled(trx) && _endorsementTxt != endorsementTxt)
  {
    _warningMsg = MIX_FARES;
    return true;
  }
  return false;
}

bool
NegotiatedFareRuleUtil::checkForTourCodesConflict(
    PricingTrx& trx, const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                  ptfItEnd = paxTypeFares.end();

  for (; ptfIt != ptfItEnd; ptfIt++)
  {
    const PaxTypeFare& currentPTF = **ptfIt;
    if (UNLIKELY(currentPTF.isNegotiated() && areTourCodesInConflict(trx, currentPTF)))
    {
      return false;
    }
  }
  return true;
}

bool
NegotiatedFareRuleUtil::areTourCodesInConflict(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
    const
{
  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  const NegFareRest* negFareRest = nullptr;
  negFareRest = getCat35Record3(&paxTypeFare, negPaxTypeFare);

  if (LIKELY(negFareRest))
  {
    if (UNLIKELY(negFareRest->noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS &&
        (negFareRest->tourBoxCodeType1() != negFareRest->tourBoxCodeType2() ||
         negFareRest->tourBoxCode1() != negFareRest->tourBoxCode2())))
    {
      return true;
    }
  }
  return false;
}

bool
NegotiatedFareRuleUtil::validatePrintOptionCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (paxTypeFares.size() > 1)
  {
    const PaxTypeFare& firstPtf = *paxTypeFares.front();
    if (!firstPtf.isNegotiated())
      return true;

    NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;

    if (!isNetRemitFare(firstPtf.fcaDisplayCatType(), getCat35Record3(&firstPtf, negPaxTypeFare)))
      return true;

    Indicator firstPO = getPrintOption(*paxTypeFares.front());
    std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                    ptfItEnd = paxTypeFares.end();
    for (ptfIt++; ptfIt != ptfItEnd; ptfIt++)
    {
      const PaxTypeFare& ptf = *(*ptfIt);
      if (firstPO != getPrintOption(ptf))
        return false;
    }
  }
  return true;
}

Indicator
NegotiatedFareRuleUtil::getPrintOption(const PaxTypeFare& ptf)
{
  Indicator po = '3'; // default
  const NegPaxTypeFareRuleData* nfrData = ptf.getNegRuleData();
  if (nfrData && nfrData->printOption())
    po = nfrData->printOption()->printOption();
  return po;
}

bool
NegotiatedFareRuleUtil::validateTourCodeCombination(
    PricingTrx& trx, const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  TourCodeUtil tcUtil(paxTypeFares);
  if (UNLIKELY(tcUtil.useOptimusNetRemit(trx)))
    return tcUtil.validate(trx);

  std::multimap<CarrierCode, std::string> cxrTourMap;
  std::string tourCode;

  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  const NegFareRest* negFareRest = nullptr;
  Indicator firstTourCodeType = BLANK;
  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                  ptfItEnd = paxTypeFares.end();
  for (; ptfIt != ptfItEnd; ptfIt++)
  {
    const PaxTypeFare& ptf = *(*ptfIt);
    if (ptf.isNegotiated())
    {
      negFareRest = getCat35Record3((*ptfIt), negPaxTypeFare);
      if (negFareRest && !negFareRest->tourBoxCode1().empty())
      {
        tourCode = negFareRest->tourBoxCode1();

        if (firstTourCodeType == BLANK)
        {
          firstTourCodeType = negFareRest->tourBoxCodeType1();
        }
        else if (UNLIKELY(negFareRest->tourBoxCodeType1() != firstTourCodeType))
        {
          return false;
        }
      }
      else
      {
        RuleUtil::getCat27TourCode(&ptf, tourCode);
      }
    }
    else
    {
      RuleUtil::getCat27TourCode(&ptf, tourCode);
    }

    CarrierCode carrier;
    if (ptf.carrier().equalToConst("YY"))
    {
      carrier = ptf.fareMarket()->governingCarrier();
    }
    else
    {
      carrier = ptf.carrier();
    }
    cxrTourMap.insert(std::pair<CarrierCode, std::string>(carrier, tourCode));
    tourCode.clear();
  }

  return validateCxrTourCodeMap(cxrTourMap);
}

bool
NegotiatedFareRuleUtil::validateCxrTourCodeMap(
    const std::multimap<CarrierCode, std::string>& cxrTourMap) const
{
  std::multimap<CarrierCode, std::string>::const_iterator mapIt1;
  std::multimap<CarrierCode, std::string>::const_iterator mapIt2;

  for (mapIt1 = cxrTourMap.begin(); mapIt1 != cxrTourMap.end(); ++mapIt1)
  {
    for (mapIt2 = cxrTourMap.begin(); mapIt2 != cxrTourMap.end(); ++mapIt2)
    {
      if (mapIt1->second != mapIt2->second && !mapIt1->second.empty() && !mapIt2->second.empty())
        return false;
    }
  }

  return true;
}

bool
NegotiatedFareRuleUtil::validateTFDCombination(PricingTrx& trx,
                                               const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (LIKELY(paxTypeFares.size() > 0))
  {
    std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                    ptfItEnd = paxTypeFares.end();

    const PaxTypeFare& ptf = *(*ptfIt);
    const NegFareRest* negFareRest1 = nullptr;
    NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;

    if (ptf.isNegotiated())
      negFareRest1 = getCat35Record3(*ptfIt, negPaxTypeFare);

    if (negFareRest1 && isNetRemitFare(ptf.fcaDisplayCatType(), negFareRest1))
    {
      Indicator tktFareDataInd1 = negFareRest1->tktFareDataInd1();

      if (negFareRest1->tktFareDataInd1() == RuleConst::BLANK && negPaxTypeFare->negFareRestExt())
        tktFareDataInd1 = negPaxTypeFare->negFareRestExt()->fareBasisAmtInd();

      for (ptfIt++; ptfIt != ptfItEnd; ptfIt++)
      {
        const NegFareRest* negFareRest2 = nullptr;
        const PaxTypeFare& ptf2 = *(*ptfIt);

        if (!ptf2.isNegotiated())
          return false;
        negFareRest2 = getCat35Record3(*ptfIt, negPaxTypeFare);

        if (negFareRest2 && negFareRest1 != negFareRest2)
        {
          Indicator tktFareDataInd2 = negFareRest2->tktFareDataInd1();

          if (negFareRest2->tktFareDataInd1() == RuleConst::BLANK &&
              negPaxTypeFare->negFareRestExt())
            tktFareDataInd2 = negPaxTypeFare->negFareRestExt()->fareBasisAmtInd();

          if (tktFareDataInd1 != tktFareDataInd2) // check if conflict byte 101 of TFD
            return false;
        }
      }
    }
  }
  return true;
}

bool
NegotiatedFareRuleUtil::isNetRemitFare(const Indicator& displayType, const NegFareRest* negFareRest)
{
  return negFareRest
             ? (negFareRest->netRemitMethod() != RuleConst::NRR_METHOD_BLANK &&
                (displayType == RuleConst::NET_SUBMIT_FARE ||
                 displayType == RuleConst::NET_SUBMIT_FARE_UPD ||
                 (displayType == RuleConst::SELLING_FARE && negFareRest->negFareCalcTblItemNo())))
             : false;
}

const NegFareRest*
NegotiatedFareRuleUtil::getCat35Record3(const PaxTypeFare* paxTypeFare,
                                        NegPaxTypeFareRuleData*& negPaxTypeFare)
{
  negPaxTypeFare = paxTypeFare->getNegRuleData();

  if (LIKELY(negPaxTypeFare))
    return dynamic_cast<const NegFareRest*>(negPaxTypeFare->ruleItemInfo());

  return nullptr;
}

bool
NegotiatedFareRuleUtil::checkCmdPricingFare(PricingTrx& trx, FarePath& farePath)
{
  for(const PricingUnit * pu : farePath.pricingUnit())
  {
    for(const FareUsage * fu : pu->fareUsage())
    {
      if (UNLIKELY(fu->paxTypeFare()->isCmdPricing()))
        return true;
    }
  }
  return false;
}

bool
NegotiatedFareRuleUtil::checkVendor(PricingTrx& trx, const VendorCode& vendor) const
{
  if (trx.dataHandle().getVendorType(vendor) == RuleConst::SMF_VENDOR)
    return true;
  return false;
}

const PrintOption*
NegotiatedFareRuleUtil::getPrintOptionInfo(PricingTrx& trx, FarePath& farePath) const
{
  std::vector<const FareUsage*> fareUsages;
  ValueCodeUtil::getFareUsages(farePath, fareUsages);

  for(const FareUsage * fareUsage : fareUsages)
  {
    const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
    if (!checkVendor(trx, paxTypeFare->vendor()))
      continue;

    if (paxTypeFare->isNegotiated())
    {
      NegPaxTypeFareRuleData* negPTFRule =
          ValueCodeUtil::getRuleDataForNetRemitCat35Fare(*paxTypeFare);
      if (negPTFRule && negPTFRule->printOption())
        return negPTFRule->printOption();
    }
  }

  return nullptr;
}

void
NegotiatedFareRuleUtil::prepareTourCodeValueCodeByPO(PricingTrx& trx, FarePath& farePath) const
{
  CollectedNegFareData* cNegFareData = farePath.collectedNegFareData();

  const PrintOption* printOption = getPrintOptionInfo(trx, farePath);

  if (printOption)
  {
    switch (printOption->printOption())
    {
    case '1':
      cNegFareData->tourCode() =
          cNegFareData->valueCode() +
          (!cNegFareData->valueCode().empty() && !cNegFareData->tourCode().empty() ? "/" : "") +
          cNegFareData->tourCode();
      break;
    case '2':
      cNegFareData->tourCode() =
          cNegFareData->tourCode() +
          (!cNegFareData->valueCode().empty() && !cNegFareData->tourCode().empty() ? "/" : "") +
          cNegFareData->valueCode();
      break;
    case '4':
    {
      std::string tourCode = cNegFareData->tourCode();
      cNegFareData->tourCode() = cNegFareData->valueCode();
      cNegFareData->valueCode() = tourCode;
    }
    break;
    default:
      break;
    }
  }

  if (cNegFareData->valueCode().size() > 14)
    cNegFareData->valueCode().resize(14);
  if (cNegFareData->tourCode().size() > 15)
    cNegFareData->tourCode().resize(15);
}

void
NegotiatedFareRuleUtil::setNetRemitTicketInd(PricingTrx& trx, FarePath& farePath) const
{
  CollectedNegFareData* cNegFareData = farePath.collectedNegFareData();

  if (_tktFareDataInd == RuleConst::NR_VALUE_F || _tktFareDataInd == RuleConst::NR_VALUE_A)
  {
    cNegFareData->netRemitTicketInd() = true;
    return;
  }

  for(const PricingUnit * pu : farePath.pricingUnit())
  {
    for(const FareUsage * fu : pu->fareUsage())
    {
      if (!fu->netRemitPscResults().empty() && fu->netRemitPscResults().front()._tfdpscSeqNumber &&
          !fu->netRemitPscResults().front()._tfdpscSeqNumber->uniqueFareBasis().empty())
      {
        cNegFareData->netRemitTicketInd() = true;
        return;
      }
    }
  }
}

void
NegotiatedFareRuleUtil::saveTourCodeForPQ(PricingTrx& trx, FarePath& farePath) const
{
  if (_indicatorTkt || _warningMsg == NO_WARNING)
    return;
  CollectedNegFareData* cNegFareData = farePath.collectedNegFareData();
  if (cNegFareData && cNegFareData->tourCode().empty())
  {
    for(const PricingUnit * pu : farePath.pricingUnit())
    {
      for(const FareUsage * fu : pu->fareUsage())
      {
        const PaxTypeFare* paxTypeFare = fu->paxTypeFare();
        if (paxTypeFare->isNegotiated())
        {
          NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
          const NegFareRest* negFareRest = getCat35Record3(paxTypeFare, negPaxTypeFare);
          if (negFareRest && !negFareRest->tourBoxCode1().empty())
          {
            cNegFareData->indTypeTour() = negFareRest->tourBoxCodeType1();
            cNegFareData->tourCode() = negFareRest->tourBoxCode1();
            return;
          }
        }
      }
    }
  }
}

bool
NegotiatedFareRuleUtil::validateFareBoxCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  NegPaxTypeFareRuleData* negPaxTypeFareRuleData(nullptr);

  if (!hasFareBoxNetRemitMethod(getCat35Record3(paxTypeFares.front(), negPaxTypeFareRuleData)))
    return true;

  bool foundFareAmount(false);
  bool allHaveValidData(true);
  const NegFareRest* negFareRest(nullptr);
  for(const PaxTypeFare * ptf : paxTypeFares)
  {
    negFareRest = getCat35Record3(ptf, negPaxTypeFareRuleData);

    if (hasFareBoxAmount(negFareRest))
    {
      foundFareAmount = true;
    }
    else
    {
      allHaveValidData = false;
    }

    if (foundFareAmount && !allHaveValidData)
    {
      _warningMsg = MIXED_FARE_BOX_AMT;
      return false;
    }
  }
  return true;
}

bool
NegotiatedFareRuleUtil::hasFareBoxAmount(const NegFareRest* negFareRest)
{
  return negFareRest && !negFareRest->cur11().empty();
}

bool
NegotiatedFareRuleUtil::hasFareBoxNetRemitMethod(const NegFareRest* negFareRest)
{
  // FARE BOX PROBLEM LOG
  return negFareRest && ((negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_1) ||
                         (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2) ||
                         (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3));
}

bool
NegotiatedFareRuleUtil::validateNetRemitMethod1(std::vector<const PaxTypeFare*>& paxTypeFares)
{
  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  const NegFareRest* negFareRest = nullptr;
  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                  ptfItEnd = paxTypeFares.end();
  for (; ptfIt != ptfItEnd; ptfIt++)
  {
    const PaxTypeFare& ptf = *(*ptfIt);
    if (UNLIKELY(ptf.isNegotiated()))
    {
      negFareRest = getCat35Record3(*ptfIt, negPaxTypeFare);
      if (!negFareRest)
        return false;
      if (!validateNetRemitMethod1(negFareRest))
        return false;
    }
  }
  return true;
}

bool
NegotiatedFareRuleUtil::validateNetRemitMethod1(const NegFareRest* negFareRest)
{
  if (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_1)
  {
    if (!(negFareRest->netGrossInd() == 'G' &&
          (negFareRest->tourBoxCodeType1() == 'B' || negFareRest->tourBoxCodeType1() == 'V' ||
           negFareRest->tourBoxCodeType2() == 'B' || negFareRest->tourBoxCodeType2() == 'V')))
      return false;

    if (!negFareRest->cur1().empty() || !negFareRest->cur2().empty())
      return false;
  }
  return true;
}

bool
NegotiatedFareRuleUtil::isItBtTicketingData(FarePath& fp)
{
  CollectedNegFareData* cnfd = fp.collectedNegFareData();
  if (cnfd && (RuleConst::SELLING_FARE_NOT_FOR_SEC == cnfd->fareTypeCode() // type L with T979
               ||
               RuleConst::NET_SUBMIT_FARE == cnfd->fareTypeCode() // type T
               ||
               RuleConst::NET_SUBMIT_FARE_UPD == cnfd->fareTypeCode())) // type C
  {
    if (checkFareBox(cnfd->fareBox())) // IT, BT
      return true;
  }
  return false;
}

bool
NegotiatedFareRuleUtil::isNetTicketingWithItBtData(const PricingTrx& trx, FarePath& fp)
{
  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  for(const PricingUnit * pu : fp.pricingUnit())
  {
    for(const FareUsage * fu : pu->fareUsage())
    {
      const PaxTypeFare* paxTypeFare = fu->paxTypeFare();
      if (paxTypeFare && paxTypeFare->isNegotiated())
      {
        const NegFareRest* negFareRest = getCat35Record3(paxTypeFare, negPaxTypeFare);
        if (negFareRest &&
            ((paxTypeFare->fcaDisplayCatType() == RuleConst::SELLING_FARE &&         // Type L with T979
              negFareRest && negFareRest->negFareCalcTblItemNo() != 0) ||
              paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||      // Type T
              paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) && // Type C
            negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_BLANK &&          // Non-Net Remit
            validateFareBoxText(negFareRest)) // IT, BT
        {
          return true;
        }
      }
    }
  }
  return false;
}
bool
NegotiatedFareRuleUtil::isRegularNet(FarePath& fp)
{
  bool ret = true;
  for(const PricingUnit * pu : fp.pricingUnit())
  {
    for(const FareUsage * fu : pu->fareUsage())
    {
      const PaxTypeFare* paxTypeFare = fu->paxTypeFare();
      if (paxTypeFare && paxTypeFare->isNegotiated())
      {
        NegPaxTypeFareRuleData* ruleData =
            paxTypeFare->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE)
                ->toNegPaxTypeFareRuleData();
        if (ruleData)
        {
          const NegFareRest* negFareRest = getCat35Record3(paxTypeFare, ruleData);
          if (negFareRest &&
             ((paxTypeFare->fcaDisplayCatType() == RuleConst::SELLING_FARE &&          // Type L with T979
               negFareRest && negFareRest->negFareCalcTblItemNo() != 0) ||
               paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||       // Type T
               paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))    // Type C
          {
            if (negFareRest->netRemitMethod() != RuleConst::NRR_METHOD_BLANK)          // Net Remit
              return false;
          }
          else
           return false;
        }
        else
          return false;
      }
      else
        return false;
    }
  }
  return ret;
}
Indicator
NegotiatedFareRuleUtil::getFareAmountIndicator(const PricingTrx& trx, const FareUsage* fu)
{
  if (!fu || !fu->paxTypeFare())
    return ' ';

  NegPaxTypeFareRuleData* ruleData = nullptr;
  const PaxTypeFare* ptf = fu->paxTypeFare();
  const NegFareRest* negFareRest = getCat35Record3(ptf, ruleData);
  if (!negFareRest)
    return ' ';

  Indicator tktFareDataInd1 = negFareRest->tktFareDataInd1();
  if (tktFareDataInd1 == RuleConst::BLANK &&
      trx.dataHandle().getVendorType(ptf->vendor()) == RuleConst::SMF_VENDOR)
  {
    ruleData = ptf->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE)->toNegPaxTypeFareRuleData();
    if (ruleData)
    {
      const NegFareRestExt* negFareRestExt = ruleData->negFareRestExt();
      if (negFareRestExt)
        tktFareDataInd1 = negFareRestExt->fareBasisAmtInd();
    }
  }
  return tktFareDataInd1;
}

bool
NegotiatedFareRuleUtil::isTFDPSC(const std::vector<FareUsage*>& fuCol)
{
  return std::any_of(fuCol.begin(),
                     fuCol.end(),
                     [](FareUsage* fu)
                     { return !fu->netRemitPscResults().empty(); });
}

bool
NegotiatedFareRuleUtil::shouldUseRedistributedWholeSale(PricingTrx* trx, const PaxTypeFare& ptFare)
{
  FareDisplayTrx *fdTrx = dynamic_cast<FareDisplayTrx*>(trx);
  if (fdTrx)
    return false;        // It must not be fare display transaction

  Agent*& agent = trx->getRequest()->ticketingAgent();
  if (!agent)
    return false;

  PseudoCityCode& tvlAgencyPCC_A20 = agent->tvlAgencyPCC();
  if (tvlAgencyPCC_A20.empty()) // travel agency must not be empty
    return false;

  if (ptFare.fcaDisplayCatType()==RuleConst::NET_SUBMIT_FARE_UPD)
  {
    const NegPaxTypeFareRuleData* negPtfRule = ptFare.getNegRuleData();
    if (negPtfRule && negPtfRule->fareRetailerRuleId())
    {
      if (tvlAgencyPCC_A20 != negPtfRule->sourcePseudoCity())
        return true;
    }
  }

  return false;
}

} // tse
