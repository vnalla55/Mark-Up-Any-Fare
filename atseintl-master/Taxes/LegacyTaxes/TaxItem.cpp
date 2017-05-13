// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxItem.h"

#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeGenText.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxReissue.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagVisitor.h"
#include "PfcTaxesExemption/AutomaticPfcTaxExemption.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/Cat33TaxReissue.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"
#include "Taxes/LegacyTaxes/Reissue.h"
#include "Taxes/LegacyTaxes/ServiceFeeRec1ValidatorYQ.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxYQ.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace std;
namespace tse
{
FALLBACK_DECL(Cat33_Diag)
FALLBACK_DECL(fallbackFixForRTPricingInSplit);

log4cxx::LoggerPtr
TaxItem::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxItem"));

TaxItemInfo::TaxItemInfo()
  : seqNo(0),
    specialProcessNo(0),
    showseparateInd('\0'),
    multioccconvrndInd('\0'),
    spclTaxRounding('\0'),
    feeApplInd('\0'),
    bookingCode1(NULL_CODE),
    bookingCode2(NULL_CODE),
    bookingCode3(NULL_CODE),
    taxType('\0'),
    feeInd('\0'),
    taxOnTaxExcl('\0'),
    taxAmt(0.00),
    minTax(0.00),
    maxTax(0.00),
    minMaxTaxCurrency(),
    minMaxTaxCurrencyNoDec(0),
    taxNodec(0),
    taxcdRoundUnit(0.00),
    taxcdRoundUnitNodec(0),
    taxCurNodec(0),
    taxcdRoundRule(EMPTY),
    taxNation(),
    taxCode(),
    taxCur(),
    specConfigName() {};

void
TaxItemInfo::copyFrom(const TaxCodeReg& tcReg)
{
  taxAmt = tcReg.taxAmt();
  taxNodec = tcReg.taxNodec();
  taxCur = tcReg.taxCur();
  taxCode = tcReg.taxCode();
  taxNation = tcReg.nation();
  taxType = tcReg.taxType();
  showseparateInd = tcReg.showseparateInd();
  multioccconvrndInd = tcReg.multioccconvrndInd();
  specialProcessNo = tcReg.specialProcessNo();
  spclTaxRounding = tcReg.spclTaxRounding();
  taxcdRoundUnit = tcReg.taxcdRoundUnit();
  taxcdRoundUnitNodec = tcReg.taxcdRoundUnitNodec();
  taxcdRoundRule = tcReg.taxcdRoundRule();
  feeApplInd = tcReg.feeApplInd();
  bookingCode1 = tcReg.bookingCode1();
  bookingCode2 = tcReg.bookingCode2();
  bookingCode3 = tcReg.bookingCode3();
  seqNo = tcReg.seqNo();
  specConfigName = tcReg.specConfigName();
  taxOnTaxCode = tcReg.taxOnTaxCode();
  taxOnTaxExcl = tcReg.taxOnTaxExcl();
  feeInd = tcReg.feeInd();
  minTax = tcReg.minTax();
  maxTax = tcReg.maxTax();
  taxCurNodec = tcReg.taxCurNodec();
  minMaxTaxCurrency = tcReg.taxCur();
  minMaxTaxCurrencyNoDec = tcReg.taxCurNodec();
}

void
TaxItemInfo::copyTo(TaxCodeReg& tcReg) const
{
  tcReg.taxAmt() = taxAmt;
  tcReg.taxNodec() = taxNodec;
  tcReg.taxCur() = taxCur;
  tcReg.taxCode() = taxCode;
  tcReg.nation() = taxNation;
  tcReg.taxType() = taxType;
  tcReg.showseparateInd() = showseparateInd;
  tcReg.multioccconvrndInd() = multioccconvrndInd;
  tcReg.specialProcessNo() = specialProcessNo;
  tcReg.spclTaxRounding() = spclTaxRounding;
  tcReg.taxcdRoundUnit() = taxcdRoundUnit;
  tcReg.taxcdRoundUnitNodec() = taxcdRoundUnitNodec;
  tcReg.taxcdRoundRule() = taxcdRoundRule;
  tcReg.feeApplInd() = feeApplInd;
  tcReg.bookingCode1() = bookingCode1;
  tcReg.bookingCode2() = bookingCode2;
  tcReg.bookingCode3() = bookingCode3;
  tcReg.seqNo() = seqNo;
  tcReg.specConfigName() = specConfigName;
  tcReg.taxOnTaxCode() = taxOnTaxCode;
  tcReg.taxOnTaxExcl() = taxOnTaxExcl;
  tcReg.feeInd() = feeInd;
  tcReg.minTax() = minTax;
  tcReg.maxTax() = maxTax;
  tcReg.taxCurNodec() = taxCurNodec;
  tcReg.taxCur() = minMaxTaxCurrency;
  tcReg.taxCurNodec() = minMaxTaxCurrencyNoDec;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxItem::buildTaxItem
//
// Description:  This function builds the TaxItem object from the TaxCodeReg
// object.
//
// @param  TaxCodeReg - structure from the TaxCodeReg class for one taxCode
//
// @return - index of new taxOutItem
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxItem::buildTaxItem(PricingTrx& trx,
    Tax& tax,
    TaxResponse& taxResponse,
    TaxCodeReg& taxCodeReg,
    bool isTaxOnChangeFee)
{
  FarePath& farePath = *taxResponse.farePath();
  _taxItemInfo.copyFrom(taxCodeReg);
  _taxAmount = tax.taxAmount();
  _taxAmountAdjusted = tax.taxAmountAdjusted();
  _taxableFare = tax.taxableFare();
  _specialPercentage = tax.specialPercentage();
  _calculationDetails = tax.calculationDetails();
  _taxSplitDetails = tax.taxSplitDetails();
  _paymentCurrency = tax.paymentCurrency();
  _paymentCurrencyNoDec = tax.paymentCurrencyNoDec();
  _failCode = tax.failCode();
  _intermediateCurrency = tax.intermediateCurrency();
  _intermediateNoDec = tax.intermediateNoDec();
  _exchangeRate1 = tax.exchangeRate1();
  _exchangeRate1NoDec = tax.exchangeRate1NoDec();
  _exchangeRate2 = tax.exchangeRate2();
  _exchangeRate2NoDec = tax.exchangeRate2NoDec();
  _intermediateUnroundedAmount = tax.intermediateUnroundedAmount();
  _intermediateAmount = tax.intermediateAmount();
  _applyFeeOnTax = tax.applyFeeOnTax();

  _travelSegStartIndex = tax.travelSegStartIndex();
  _travelSegEndIndex = tax.travelSegEndIndex();

  _taxOnChangeFee = isTaxOnChangeFee;

  _skipTaxOnTaxIfNoFare = tax.skipTaxOnTaxIfNoFare();
  _requireTaxOnTaxSeqMatch = tax.requireTaxOnTaxSeqMatch();
  _requireTaxOnDomTaxMatch = tax.requireTaxOnDomTaxMatch();
  _taxOnTaxItems = tax.taxOnTaxItems();

  if (!_calculationDetails.taxableTaxItems.empty())
  {
    _taxOnTaxItems = _calculationDetails.taxableTaxItems;
  }

  if (tax.specialIndex())
  {
    _travelSegStartIndex = tax.travelSegSpecialTaxStartIndex();
    _travelSegEndIndex = tax.travelSegSpecialTaxEndIndex();
  }

  if ((_travelSegStartIndex >= farePath.itin()->travelSeg().size()) ||
      (_travelSegEndIndex >= farePath.itin()->travelSeg().size()))
  {
    LOG4CXX_WARN(_logger, "Invalid Index For Tax Item");

    _travelSegStartIndex = 0;
    _travelSegEndIndex = 0;
  }

  const Loc* originLocation = farePath.itin()->travelSeg()[_travelSegStartIndex]->origin();
  const Loc* destination =
      farePath.itin()->travelSeg()[_travelSegEndIndex]->destination(); // lint !e530

  _taxLocalBoard = originLocation->loc();
  _taxLocalOff = destination->loc();

  _legId = farePath.itin()->travelSeg()[_travelSegStartIndex]->legId();

  std::vector<TravelSeg*>::const_iterator travelSegI;

  if (tax.mixedTax())
    _mixedTax = true;

  if (UNLIKELY(tax.partialTax()))
  {
    _partialTax = true;
    _taxablePartialFare = tax.taxablePartialFare();
    _taxableFare = tax.thruTotalFare();
    _taxMilesLocal = tax.partialLocalMiles();
    _taxMilesThru = tax.partialThruMiles();
    _travelSegThruEndOrder = tax.travelSegThruEndOrder();

    travelSegI = farePath.itin()->travelSeg().begin();
    uint16_t index = 0;

    for (; travelSegI != farePath.itin()->travelSeg().end(); travelSegI++, index++)
    {
      if (farePath.itin()->segmentOrder(*travelSegI) == tax.travelSegPartialStartOrder())
      {
        _travelSegStartIndex = index;
        _taxLocalBoard = (*travelSegI)->origin()->loc();
      }

      if (farePath.itin()->segmentOrder(*travelSegI) == tax.travelSegPartialEndOrder())
      {
        _travelSegEndIndex = index;
        _taxLocalOff = (*travelSegI)->destination()->loc();
      }

      if (farePath.itin()->segmentOrder(*travelSegI) == tax.travelSegThruStartOrder())
        _taxThruBoard = (*travelSegI)->origin()->loc();

      if (farePath.itin()->segmentOrder(*travelSegI) == tax.travelSegThruEndOrder())
        _taxThruOff = (*travelSegI)->destination()->loc();
    }
  }

  if (LIKELY(farePath.itin()->travelSeg().size() > _travelSegStartIndex))
  {
    TravelSeg* travelSeg = farePath.itin()->travelSeg()[_travelSegStartIndex];
    _segmentOrderStart = farePath.itin()->segmentOrder(travelSeg);
  }

  if (LIKELY(farePath.itin()->travelSeg().size() > _travelSegEndIndex))
  {
    TravelSeg* travelSeg = farePath.itin()->travelSeg()[_travelSegEndIndex];
    _segmentOrderEnd = farePath.itin()->segmentOrder(travelSeg);
  }

  if (!tax.hiddenBrdAirport().empty())
    _taxLocalBoard = tax.hiddenBrdAirport();

  if (!tax.hiddenOffAirport().empty())
    _taxLocalOff = tax.hiddenOffAirport();

  _taxRecProcessed = false;
  _interline = !TravelSegUtil::isTravelSegVecOnline(farePath.itin()->travelSeg());

  travelSegI = farePath.itin()->travelSeg().begin() + _travelSegStartIndex;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (LIKELY(airSeg))
    _carrierCode = airSeg->marketingCarrierCode();

  _isGstTax = taxUtil::isGstTax(taxCodeReg.taxCode());

  std::vector<TaxCodeGenText*>::const_iterator taxCodeGenTextI;
  taxCodeGenTextI = taxCodeReg.taxCodeGenTexts().begin();

  if (taxCodeGenTextI != taxCodeReg.taxCodeGenTexts().end())
  {
    std::vector<string>::const_iterator taxTextI;
    taxTextI = (*taxCodeGenTextI)->txtMsgs().begin();

    if (LIKELY(taxTextI != (*taxCodeGenTextI)->txtMsgs().end()))
      _taxDescription = (*taxTextI);
  }

  if (UNLIKELY(trx.getRequest()->isTicketEntry()))
  {
    if (taxCodeReg.specialProcessNo() == 64)
    {
      _taxExemptAmount = _taxAmount;
      _taxAmount = 0.0;
      _failCode = EXEMPT_SPECIAL_FEE;
    }
  }

  // Change requested by Customers to Display all Exempted Taxes that Apply in the FQL

  if (UNLIKELY(trx.getRequest()->isExemptAllTaxes() && !tax.isSkipExempt()))
  {
    _taxExemptAmount = _taxAmount;
    _taxAmount = 0.0;
    _failCode = EXEMPT_ALL_TAXES;
  }

  if (UNLIKELY((trx.getRequest()->isExemptSpecificTaxes()) && (trx.getRequest()->taxIdExempted().empty()) &&
      (_failCode == 0) && (taxCodeReg.feeInd() != YES) && !tax.isSkipExempt()))
  {
    _taxExemptAmount = _taxAmount;
    _taxAmount = 0.0;
    _failCode = EXEMPT_ALL_TAXES;
  }

  std::vector<std::string>::iterator taxIdExemptedI = trx.getRequest()->taxIdExempted().begin();

  for (; taxIdExemptedI != trx.getRequest()->taxIdExempted().end(); taxIdExemptedI++)
  {
    if ((*taxIdExemptedI).size() > taxCodeReg.taxCode().size())
      continue;

    std::string baseTaxCode;

    baseTaxCode = utc::baseTaxCode(trx, tax.taxSpecConfig());

    if (!tax.isSkipExempt() && ((strncmp((*taxIdExemptedI).c_str(),
                                         taxCodeReg.taxCode().c_str(),
                                         (*taxIdExemptedI).size()) == 0) ||
                                (_taxOnChangeFee && baseTaxCode == *taxIdExemptedI)))
    {
      _taxExemptAmount = _taxAmount;
      _taxAmount = 0.0;
      _failCode = EXEMPT_SPECIFIED_TAXES;
    }
  }

  if (UNLIKELY(TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse)))
  {
    applyAutomaticPfcTaxExemption(trx, taxResponse);
  }

  if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
  {
    if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
        trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
        trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    {
      _reissueTaxInfo = ReissueTaxInfoBuilder().build(trx, taxCodeReg.taxCode());
    }

    TaxReissueSelector taxReissueSelector(
        trx.dataHandle().getTaxReissue(taxCode(),
            trx.getRequest()->ticketingDT()));
    Cat33TaxReissue cat33TaxReissue(
        taxReissueSelector.getTaxReissue(TaxReissueSelector::LEGACY_TAXES_TAX_TYPE,
                                         MCPCarrierUtil::swapToPseudo(&trx, _carrierCode)));

    if (!fallback::Cat33_Diag(&trx))
    {
      DiagCollector* diag = taxResponse.diagCollector();
      if (diag != nullptr)
      {
        PrintTaxReissueInfo visitor(cat33TaxReissue, taxCode(), _carrierCode);
        diag->accept(visitor);
      }
    }

    setRefundableTaxTag(cat33TaxReissue.getRefundableTaxTag());
  }
  else
  {
    if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX || trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    {
      _reissueTaxInfo = ReissueTaxInfoBuilder().build(trx, taxCodeReg.taxCode());
    }
  }

  if (!fallback::fallbackFixForRTPricingInSplit(&trx))
  {
    _travelSegStart = farePath.itin()->travelSeg()[_travelSegStartIndex];
    _travelSegEnd = farePath.itin()->travelSeg()[_travelSegEndIndex];
  }
}

void
TaxItem::applyAutomaticPfcTaxExemption(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (taxResponse.automaticPFCTaxExemptionData()->taxExemptionOption() ==
      AutomaticPfcTaxExemptionData::EXEMPT_INCLUDED)
  {
    for (TaxCode& exemptTaxCode : taxResponse.automaticPFCTaxExemptionData()->exemptionTaxCodes())
    {
      if (strncmp(exemptTaxCode.c_str(), _taxItemInfo.taxCode.c_str(), exemptTaxCode.size()) == 0)
      {
        _taxExemptAmount = _taxAmount;
        _taxAmount = 0.0;
        _failCode = EXEMPT_SPECIFIED_TAXES;

        DiagManager diagMgr(trx, Diagnostic807);
        if (diagMgr.isActive())
        {
          if (taxResponse.automaticPFCTaxExemptionData()->firstTaxExempted())
          {
            diagMgr.collector()
                << " \n*************** APPLIED TAX EXEMPTIONS *********************\n";
          }

          diagMgr.collector() << " \nTAX DB SEQ NBR: " << _taxItemInfo.seqNo;
          if (_exemptFromAtpco)
            diagMgr.collector() << " (EXEMPT BY ATPCO PROCESSING)";
          diagMgr.collector() << " \n" << _taxItemInfo.taxNation << "/" << _taxItemInfo.taxCode
                              << " " << _taxDescription << "\n";

          if (_taxItemInfo.taxType != 'P')
          {
            diagMgr.collector() << std::setw(14) << std::fixed
                                << std::setprecision(_taxItemInfo.taxNodec) << _taxItemInfo.taxAmt
                                << ":" << _taxItemInfo.taxCur;
          }
          else
          {
            diagMgr.collector() << std::setw(14) << std::fixed
                                << std::setprecision(_paymentCurrencyNoDec) << _taxExemptAmount
                                << ":" << _paymentCurrency;
          }

          diagMgr.collector() << std::setw(14) << std::fixed
                              << std::setprecision(_paymentCurrencyNoDec) << _taxExemptAmount
                              << " *" << _taxLocalBoard << " " << _taxLocalOff << "*"
                              << "\n";

          diagMgr.collector().flushMsg();
        }

        break;
      }
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxItem::buildTaxItem
//
// Description:  This function builds the TaxItem object from the TaxCodeReg
// object.
//
// @param  TaxCodeReg - structure from the TaxCodeReg class for one taxCode
//
// @return - index of new taxOutItem
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxItem::buildTaxItem(PricingTrx& trx,
                      YQYR::ServiceFee& serviceFee,
                      FarePath& farePath,
                      tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator)
{
  TaxCodeReg _taxCodeReg;
  _taxCodeReg.taxCode() = serviceFeeRec1Validator.taxCode();
  _taxCodeReg.seqNo() = serviceFeeRec1Validator.seqNo();
  _taxCodeReg.taxNodec() = serviceFeeRec1Validator.taxNodec();
  _taxCodeReg.taxAmt() = serviceFeeRec1Validator.taxAmt();
  _taxCodeReg.taxCur() = serviceFeeRec1Validator.taxCur();
  _taxCodeReg.taxCurNodec() = serviceFeeRec1Validator.taxCurNodec();
  _taxCodeReg.taxType() = serviceFeeRec1Validator.taxType();
  _taxItemInfo.copyFrom(_taxCodeReg);
  _taxAmount = serviceFee.taxAmount();
  _taxableFare = serviceFee.taxableFare();
  //_calculationDetails = serviceFee.calculationDetails();

  _paymentCurrency = serviceFee.paymentCurrency();
  _paymentCurrencyNoDec = serviceFee.paymentCurrencyNoDec();
  _failCode = 0;
  _intermediateCurrency = serviceFee.intermediateCurrency();
  _intermediateNoDec = serviceFee.intermediateNoDec();
  _exchangeRate1 = serviceFee.exchangeRate1();
  _exchangeRate1NoDec = serviceFee.exchangeRate1NoDec();
  _exchangeRate2 = serviceFee.exchangeRate2();
  _exchangeRate2NoDec = serviceFee.exchangeRate2NoDec();
  _intermediateUnroundedAmount = serviceFee.intermediateUnroundedAmount();
  _intermediateAmount = serviceFee.intermediateAmount();
  _taxIncluded = (serviceFeeRec1Validator.taxIncludedInd() == 'X');
  _applyFeeOnTax = true;

  _travelSegStartIndex = serviceFee.travelSegStartIndex();
  _travelSegEndIndex = serviceFee.travelSegEndIndex();

  if (UNLIKELY((_travelSegStartIndex >= farePath.itin()->travelSeg().size()) ||
      (_travelSegEndIndex >= farePath.itin()->travelSeg().size())))
  {
    LOG4CXX_WARN(_logger, "Invalid Index For Tax Item");

    _travelSegStartIndex = 0;
    _travelSegEndIndex = 0;
  }

  const Loc* originLocation = farePath.itin()->travelSeg()[_travelSegStartIndex]->origin();
  const Loc* destination =
      farePath.itin()->travelSeg()[_travelSegEndIndex]->destination(); // lint !e530

  _taxLocalBoard = originLocation->loc();
  _taxLocalOff = destination->loc();

  _legId = farePath.itin()->travelSeg()[_travelSegStartIndex]->legId();

  std::vector<TravelSeg*>::const_iterator travelSegI;

  if (LIKELY(farePath.itin()->travelSeg().size() > _travelSegStartIndex))
  {
    TravelSeg* travelSeg = farePath.itin()->travelSeg()[_travelSegStartIndex];
    _segmentOrderStart = farePath.itin()->segmentOrder(travelSeg);
  }

  if (LIKELY(farePath.itin()->travelSeg().size() > _travelSegEndIndex))
  {
    TravelSeg* travelSeg = farePath.itin()->travelSeg()[_travelSegEndIndex];
    _segmentOrderEnd = farePath.itin()->segmentOrder(travelSeg);
  }

  _taxRecProcessed = false;
  _interline = !TravelSegUtil::isTravelSegVecOnline(farePath.itin()->travelSeg());

  travelSegI = farePath.itin()->travelSeg().begin() + _travelSegStartIndex;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (LIKELY(airSeg))
    _carrierCode = airSeg->marketingCarrierCode();

  _isGstTax = false;

  // Change requested by Customers to Display all Exempted Taxes that Apply in the FQL

  if (UNLIKELY(trx.getRequest()->isExemptAllTaxes()))
  {
    _taxExemptAmount = _taxAmount;
    _taxAmount = 0.0;
    _failCode = EXEMPT_ALL_TAXES;
  }

  if (UNLIKELY((trx.getRequest()->isExemptSpecificTaxes()) && (trx.getRequest()->taxIdExempted().empty()) &&
      (_failCode == 0) /* && //Never filled for YQ
      (taxCode.feeInd() != YES) */
      ))
  {
    _taxExemptAmount = _taxAmount;
    _taxAmount = 0.0;
    _failCode = EXEMPT_ALL_TAXES;
  }

  std::vector<std::string>::iterator taxIdExemptedI = trx.getRequest()->taxIdExempted().begin();

  for (; taxIdExemptedI != trx.getRequest()->taxIdExempted().end(); taxIdExemptedI++)
  {
    if ((*taxIdExemptedI).size() > serviceFeeRec1Validator.taxCode().size())
      continue;

    std::string baseTaxCode;

    // baseTaxCode = utc::baseTaxCode(trx, tax.taxSpecConfig());

    if ((strncmp((*taxIdExemptedI).c_str(),
                 serviceFeeRec1Validator.taxCode().c_str(),
                 (*taxIdExemptedI).size()) == 0) /*||
        (_taxOnChangeFee && baseTaxCode ==  *taxIdExemptedI)*/
        )
    {
      _taxExemptAmount = _taxAmount;
      _taxAmount = 0.0;
      _failCode = EXEMPT_SPECIFIED_TAXES;
    }
  }

  if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
  {
    if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
        trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
        trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    {
      _reissueTaxInfo = ReissueTaxInfoBuilder().build(trx, serviceFeeRec1Validator.taxCode());
    }
  }
  else
  {
    if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
        trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    {
      _reissueTaxInfo = ReissueTaxInfoBuilder().build(trx, serviceFeeRec1Validator.taxCode());
    }
  }

  if (!fallback::fallbackFixForRTPricingInSplit(&trx))
  {
    _travelSegStart = farePath.itin()->travelSeg()[_travelSegStartIndex];
    _travelSegEnd = farePath.itin()->travelSeg()[_travelSegEndIndex];
  }
}

MoneyAmount
TaxItem::getFareSumAmount() const
{
  return taxSplitDetails().getFareSumAmount();
}

bool
TaxItem::useTaxableTaxSumAmount() const
{
  return taxSplitDetails().useTaxableTaxSumAmount();
}

bool
TaxItem::isCalculationDetailsSet() const
{
  return _taxSplitDetails.isSet();
}

} // namespace tse
