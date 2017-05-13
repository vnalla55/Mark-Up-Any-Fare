    //----------------------------------------------------------------------------
//  File:        Diag198Collector.C
//  Authors:     Mike Carroll
//  Created:     July 5
//
//  Description: Diagnostic 198 formatter
//
//  Updates:
//          date - initials - description.
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
//----------------------------------------------------------------------------

#include "Diagnostic/Diag198Collector.h"

#include "Common/CabinType.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AmVatTaxRatesOnCharges.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PaxType.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Cat05OverrideCarrier.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "Diagnostic/DiagnosticAutoIndent.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <vector>
#include <boost/format.hpp>

namespace tse
{
FALLBACK_DECL(fallbackBrandedFaresPricing)
FALLBACK_DECL(fallbackBrandedFaresTNShopping)
FALLBACK_DECL(fallbackAB240)
FALLBACK_DECL(fallbackAMChargesTaxes)
FALLBACK_DECL(fallbackDiag198)
FALLBACK_DECL(fallbackSearchForBrandsPricing)
FALLBACK_DECL(fallbackNonPreferredVC)
FALLBACK_DECL(fallbackPriceByCabinActivation);
FALLBACK_DECL(fallbackPreferredVC)
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);
FALLBACK_DECL(azPlusUp);
FALLBACK_DECL(fallbackAAExcludedBookingCode);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(virtualFOPMaxOBCalculation);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);
FALLBACK_DECL(fallbackFlexFareGroupNewJumpCabinLogic);
FALLBACK_DECL(excDiscDiag23XImprovements);
FALLBACK_DECL(fallbackTraditionalValidatingCxr)
FALLBACK_DECL(dffOaFareCreation)
FALLBACK_DECL(fallbackFFGMaxPenaltyNewLogic);

namespace
{
template <typename T>
inline void
addBrandBkgVect(const std::vector<T>& bkgList, DiagCollector& dc)
{
  for (const auto& bkg : bkgList)
    dc << " " << bkg.c_str();

  dc << std::endl;
}

template <typename T>
inline void
addBrandBkgMap(const std::map<T, char>& bkgList, DiagCollector& dc)
{
  for (const auto& bkg : bkgList)
    dc << " " << bkg.first.c_str() << "-" << bkg.second;

  dc << std::endl;
}

void
printMarkUp(DiagCollector& dc, const Discounts& discounts, bool isMipTrx, const Trx& trx)
{
  if (!fallback::excDiscDiag23XImprovements(&trx))
  {
    dc << "DISCOUNT AMOUNT:";

    if (discounts.isDAEntry())
    {
      if (isMipTrx)
      {
        dc << " " << discounts.getAmounts().front().amount << " "
           << discounts.getAmounts().front().currencyCode;
      }
      else
      {
        for (const auto& elem : discounts.getAmounts())
          dc << "\n  SEGMENT: " << elem.startSegmentOrder << "-" << elem.endSegmentOrder
             << " AMOUNT: " << elem.amount << " " << elem.currencyCode;
      }
    }
    dc << '\n';

    dc << "DISCOUNT PERCENTAGE:";

    if (discounts.isDPEntry())
    {
      if (isMipTrx)
      {
        dc << " " << discounts.getPercentages().cbegin()->second;
      }
      else
      {
        for (const auto& elem : discounts.getPercentages())
          dc << "\n  SEGMENT: " << elem.first << " PERCENT: " << elem.second;
      }
    }
    dc << '\n';
  }

  dc << "MARK UP AMOUNT:";
  if (discounts.isPAEntry())
  {
    if (isMipTrx)
    {
      dc << " " << -discounts.getAmounts().front().amount << " "
         << discounts.getAmounts().front().currencyCode;
    }
    else
    {
      for (const auto& elem : discounts.getAmounts())
        dc << "\n  SEGMENT: " << elem.startSegmentOrder << "-" << elem.endSegmentOrder
           << " AMOUNT: " << fabs(elem.amount) << " " << elem.currencyCode;
    }
  }
  dc << '\n';

  dc << "MARK UP PERCENTAGE:";
  if (discounts.isPPEntry())
  {
    if (isMipTrx)
    {
      dc << " " << -discounts.getPercentages().cbegin()->second;
    }
    else
    {
      for (const auto& elem : discounts.getPercentages())
        dc << "\n  SEGMENT: " << elem.first << " PERCENT: " << fabs(elem.second);
    }
  }
  dc << '\n';
}
}

Diag198Collector&
Diag198Collector::operator << (const PricingTrx& trx)
{
  if (_active)
  {
    addAncillaryPricingFeeInfo(trx);
    addRequest(*trx.getRequest(), trx);
    addMultiTicketInd(trx);
    if (!fallback::fallbackBrandedFaresPricing(&trx))
      addPbbInd(trx);
    if (!fallback::fallbackBrandedFaresTNShopping(&trx))
      addBrandedFaresTNShoppingInd(trx);
    if (!fallback::fallbackSearchForBrandsPricing(&trx))
      addSearchForBrandsPricingInd(trx);
    endRequest();
    addValidatingCxrInfo(trx);
    (*this) << "Q6W AWARD REQUEST : " << (trx.awardRequest() ? "T" : "F") << "\n";
    addOptions(trx);
    addSnapInfo(trx);
    addConsolidatorPlusUp(*(trx.itin().front()));
    addExchangePricingDates(trx);
    addExchangeInfo(trx);
    if (const RexBaseTrx* rexBaseTrx = dynamic_cast<const RexBaseTrx*>(&trx))
    {
      addExchangePlusUpInfo(*rexBaseTrx);
    }
    addExchangeCat31Info(trx);
    addAncRequestDetails(trx);
    addONDInfo(trx);
    addItinInfo(trx);
    addFlexFareGroupsDataInfo(trx);
    addChargesTaxes(trx);
  }
  return *this;
}

Diag198Collector&
Diag198Collector::operator << (const CurrencyTrx& trx)
{
  if (_active)
    addRequest(*trx.getRequest(), trx);

  return *this;
}

Diag198Collector&
Diag198Collector::operator << (const MileageTrx& trx)
{
  if (_active)
    addRequest(*trx.getRequest(), trx);

  return *this;
}

void
Diag198Collector::addRequest(const PricingRequest& request, const Trx & trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "***************** START REQUEST **************************\n";

  addDefaultAgent(request);
  addTicketingAgent(request, trx);

  dc << "TICKETING DATE TIME: ";
  dc << request.ticketingDT().dateToString(DDMMMYY, "") << std::endl;
  dc << "CORPORATE ID: " << request.corporateID() << std::endl;
  dc << "ACCOUNT CODE: " << request.accountCode() << std::endl;
  dc << "DIAGNOSTIC NUMBER: " << request.diagnosticNumber() << std::endl;
  dc << "VALIDATING CARRIER: " << request.validatingCarrier() << std::endl;

  if(!fallback::fallbackNonPreferredVC(&trx) && !request.nonPreferredVCs().empty())
  {
    dc << "NON-PREFERRED VALIDATING CARRIERS: ";
    dc << DiagnosticUtil::containerToString(request.nonPreferredVCs()) << std::endl;
  }
  if(!fallback::fallbackPreferredVC(&trx) && !request.preferredVCs().empty())
  {
    dc << "PREFERRED VALIDATING CARRIERS: ";
    dc << DiagnosticUtil::containerToString(request.preferredVCs()) << std::endl;
  }
  if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    switch(request.spvInd())
    {
      case tse::spValidator::noSMV_noIEV:
        dc<<"SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    F"<< std::endl;
        break;
      case tse::spValidator::noSMV_IEV:
        dc<<"SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    T"<< std::endl;
        break;
      case tse::spValidator::SMV_IEV:
        dc<<"SETTLEMENT PLAN VALIDATION:    T, IET VALIDATION:    T"<< std::endl;
        break;
    }
    if(!request.spvCxrsCode().empty())
      dc<<"NSP CARRIERS:    "<<DiagnosticUtil::containerToString(request.spvCxrsCode()) << std::endl;
    if(!request.spvCntyCode().empty())
      dc<<"NSP COUNTRIES:    "<<DiagnosticUtil::containerToString(request.spvCntyCode()) << std::endl;
  }

  if (!fallback::fallbackTraditionalValidatingCxr(&trx))
  {
    if (request.ticketingAgent()->agentTJR())
    {
      dc << "TRADITIONAL VALIDATING CARRIER: " <<
         (request.ticketingAgent()->agentTJR()->pricingApplTag5() == 'Y' &&
          request.ticketingAgent()->agentTJR()->pricingApplTag7() == 'Y' ? "T" : "F") << std::endl;
    }
  }

  dc << "TICKET POINT OVERRIDE: " << request.ticketPointOverride() << std::endl;
  dc << "SALE POINT OVERRIDE: " << request.salePointOverride() << std::endl;

  if(!fallback::fallbackDiag198(&trx))
  {
    if(!request.salePointOverride().empty())
    {
      const Loc* loc = trx.dataHandle().getLoc(request.salePointOverride(), trx.ticketingDate());
      if(loc)
        dc << "SALE POINT OVERRIDE NATION: " << loc->nation() << std::endl;
    }
  }

  dc << "EQUIV AMOUNT OVERRIDE: " << request.equivAmountOverride() << std::endl;
  dc << "RATE AMOUNT OVERRIDE: " << request.rateAmountOverride() << std::endl;
  dc << "SECOND RATE AMOUNT OVERRIDE: " << request.secondRateAmountOverride() << std::endl;
  dc << "TICKET DATE OVERRIDE: " << request.ticketDateOverride().dateToString(DDMMMYY, "")
     << std::endl;
  dc << "GOVERNING CARRIER OVERRIDE: " << request.governingCarrierOverride(0) << std::endl;
  dc << "JAL/AXESS WPNETT CAT35: " << (request.isWpNettRequested() ? "T" : "F") << std::endl;
  dc << "JAL/AXESS WPSEL CAT35: " << (request.isWpSelRequested() ? "T" : "F") << std::endl;
  dc << "LOW FARE NO AVAILABILITY: " << request.lowFareNoAvailability() << std::endl;
  dc << "LOW FARE REQUESTED: " << request.lowFareRequested() << std::endl;
  dc << "EXEMPT SPECIFIC TAXES: " << request.exemptSpecificTaxes() << std::endl;
  dc << "EXEMPT ALL TAXES: " << request.exemptAllTaxes() << std::endl;
  dc << "PRICE NULL DATE: " << request.priceNullDate() << std::endl;
  dc << "TICKET ENTRY: " << request.ticketEntry() << std::endl;
  dc << "FORM OF PAYMENT CASH: " << request.formOfPaymentCash() << std::endl;
  dc << "FORM OF PAYMENT CHECK: " << request.formOfPaymentCheck() << std::endl;
  dc << "FORM OF PAYMENT GTR: " << request.formOfPaymentGTR() << std::endl;

  dc << "FORM OF PAYMENT CARD: " << request.formOfPaymentCard() << std::endl;
  dc << "CREDIT CARD BIN: " << request.formOfPayment() << std::endl;
  dc << "SECOND CREDIT CARD BIN: " << request.secondFormOfPayment() << std::endl;
  dc << "PAYMENT AMOUNT FOP: " << request.paymentAmountFop() << std::endl;
  dc << "CHARGE RESIDUAL: " << (request.chargeResidualInd() ? "T" : "F") << std::endl;
  dc << "CONTRACT NUMBER: " << request.contractNumber() << std::endl;
  dc << "LENGTH OF ATB FARE CALC: " << request.lengthATBFareCalc() << std::endl;
  dc << "NUMBER TAX BOXES: " << request.numberTaxBoxes() << std::endl;
  dc << "AGENCY TAX IS PERCENT: " << request.agencyTaxIsPercent() << std::endl;
  dc << "REFUND PENALTY: " << request.refundPenalty() << std::endl;
  dc << "PRICE NO AVAILABLITY: " << request.priceNoAvailability() << std::endl;
  dc << "FARE GROUP REQUESTED: " << request.fareGroupRequested() << std::endl;
  dc << "IGNORE TIME STAMP: " << request.ignoreTimeStamp() << std::endl;
  dc << "EXEMPT PFC: " << request.exemptPFC() << std::endl;
  dc << "ELECTRONIC TICKET: " << request.electronicTicket() << std::endl;
  dc << "DEFAULT ETKT IS OFF WITHOUT OVERRIDE: " << request.eTktOffAndNoOverride() << std::endl;
  dc << "CONSIDER MULTI AIRPORT: " << request.considerMultiAirport() << std::endl;
  dc << "FROM QREX/ARP: " << (request.rexEntry() ? "T" : "F") << std::endl;
  dc << "RATE OF EXCHANGE OVERRIDE: " << request.roeOverride() << std::endl;
  dc << "RATE OF EXCHANGE OVERRIDE NUM OF DEC PLACES: " << request.roeOverrideNoDec() << std::endl;
  if (request.ticketingAgent()->sabre1SUser() && request.isWpa50())
  {
    dc << "WPA 50 OPTIONS: "
       << "T" << std::endl;
  }

  if (request.isMultiAccCorpId())
  {
    dc << "***** START MULTI ACCOUNT CODE / CORPORATE ID *****\n";

    if (!request.corpIdVec().empty())
    {
      dc << "CORPORATE ID: ";
      std::vector<std::string>::const_iterator corpIdIter = request.corpIdVec().begin();
      std::vector<std::string>::const_iterator corpIdEnd = request.corpIdVec().end();

      while (corpIdIter != corpIdEnd)
      {
        dc << (*corpIdIter) << std::endl;

        ++corpIdIter;

        if (corpIdIter != corpIdEnd)
          dc << "              ";
      }
    }

    if (!request.accCodeVec().empty())
    {
      dc << "ACCOUNT CODE: ";
      std::vector<std::string>::const_iterator accCodeIter = request.accCodeVec().begin();
      std::vector<std::string>::const_iterator accCodeEnd = request.accCodeVec().end();

      while (accCodeIter != accCodeEnd)
      {
        dc << (*accCodeIter) << std::endl;

        ++accCodeIter;

        if (accCodeIter != accCodeEnd)
          dc << "              ";
      }
    }

    dc << "***** END MULTI ACCOUNT CODE / CORPORATE ID *****\n";
  }

  // Tax Override
  if (!request.taxOverride().empty())
  {
    dc << "***** START TAX OVERRIDE *****\n";
    for (const auto taxOverride : request.taxOverride())
      if (taxOverride != nullptr)
      {
        dc << " TAX CODE: " << taxOverride->taxCode() << std::endl;
        dc << " TAX AMT: " << taxOverride->taxAmt() << std::endl;
      }
    dc << "***** END TAX OVERRIDE *****\n";
  }

  // Tax ID Exempted
  if (!request.taxIdExempted().empty())
  {
    dc << "***** START ID EXEMPTED *****\n";
    for (const auto& taxId : request.taxIdExempted())
      dc << "    " << taxId << std::endl;

    dc << "***** END TAX ID EXEMPTED *****\n";
  }

  dc << "BOARD POINT: " << request.boardPoint() << std::endl;
  dc << "OFF POINT: " << request.offPoint() << std::endl;
  dc << "REQUESTED DEPARTURE DATE TIME: "
     << request.requestedDepartureDT().dateToString(DDMMMYY, "") << std::endl;
  dc << "FARE CLASS CODE: " << request.fareClassCode() << std::endl;
  std::string globalDirectionStr = "XX";
  if (globalDirectionToStr(globalDirectionStr, request.globalDirection()))
  {
    dc << "GLOBAL DIRECTION: " << (globalDirectionStr.empty() ? "ZZ" : globalDirectionStr)
       << std::endl;
  }
  else
  {
    dc << "GLOBAL DIRECTION: UNDEFINED" << std::endl;
  }

  addDiscountPlusUpInfo(request, trx);

  if (request.industryFareOverrides().size() > 0)
  {
    dc << "INDUSTRY FARE OVERRIDE SEGMENT: ";
    std::copy(request.industryFareOverrides().begin(),
              request.industryFareOverrides().end(),
              std::ostream_iterator<int16_t>(dc, " "));
    dc << '\n';
  }

  dc << "GOVERNING CARRIER: " << request.governingCarrier() << std::endl;

  if (request.governingCarrierOverrides().size() > 0)
  {
    dc << "CARRIER OVERRIDE:\n";
    for (const auto& elem : request.governingCarrierOverrides())
    {
      dc << "  SEGMENT: " << elem.first << " CARRIER: " << elem.second << '\n';
    }
  }

  // Tkt Designator
  if (request.tktDesignator().size() > 0)
  {
    dc << "***** START TKT DESIGNATOR *****\n";
    std::map<int16_t, TktDesignator>::const_iterator tktDesIter = request.tktDesignator().begin();
    std::map<int16_t, TktDesignator>::const_iterator tktDesEnd = request.tktDesignator().end();
    for (; tktDesIter != tktDesEnd; tktDesIter++)
      dc << "  SEGMENT: " << tktDesIter->first << " TKT DESIGNATOR: " << tktDesIter->second
         << std::endl;
    dc << "***** END TKT DESIGNATOR *****\n";
  }

  // Specified Tkt Designator
  if (request.specifiedTktDesignator().size() > 0)
  {
    dc << "***** START SPECIFIED TKT DESIGNATOR *****\n";
    std::map<int16_t, TktDesignator>::const_iterator tktDesIter =
        request.specifiedTktDesignator().begin();
    std::map<int16_t, TktDesignator>::const_iterator tktDesEnd =
        request.specifiedTktDesignator().end();
    for (; tktDesIter != tktDesEnd; tktDesIter++)
      dc << "  SEGMENT: " << tktDesIter->first
         << " SPECIFIED TKT DESIGNATOR: " << tktDesIter->second << std::endl;
    dc << "***** END SPECIFIED TKT DESIGNATOR *****\n";
  }
  dc << "ADD ON CONSTRUCTION: " << request.addOnConstruction() << std::endl;

  // Consider only carriers
  if (request.considerOnlyCarriers().size() > 0)
  {
    dc << "***** START CONSIDER ONLY CARRIERS *****\n";
    std::vector<CarrierCode>::const_iterator cocIter = request.considerOnlyCarriers().begin();
    std::vector<CarrierCode>::const_iterator cocEnd = request.considerOnlyCarriers().end();
    for (; cocIter != cocEnd; cocIter++)
      dc << "    " << (*cocIter) << std::endl;
    dc << "***** END CONSIDER ONLY CARRIERS *****\n";
  }

  // Diag Arg Type
  if (request.diagArgType().size() > 0)
  {
    dc << "***** START DIAG ARG TYPE *****\n";
    std::vector<std::string>::const_iterator diagArgTypeIter = request.diagArgType().begin();
    std::vector<std::string>::const_iterator diagArgTypeEnd = request.diagArgType().end();
    for (; diagArgTypeIter != diagArgTypeEnd; diagArgTypeIter++)
      dc << "    " << (*diagArgTypeIter) << std::endl;
    dc << "***** END DIAG ARG TYPE *****\n";
  }

  // Diag Arg Data
  if (request.diagArgData().size() > 0)
  {
    dc << "***** START DIAG ARG DATA *****\n";
    std::vector<std::string>::const_iterator diagArgDataIter = request.diagArgData().begin();
    std::vector<std::string>::const_iterator diagArgDataEnd = request.diagArgData().end();
    for (; diagArgDataIter != diagArgDataEnd; diagArgDataIter++)
      dc << "    " << (*diagArgDataIter) << std::endl;
    dc << "***** END DIAG ARG DATA *****\n";
  }

  dc << "FROM CURRENCY: " << request.fromCurrency() << std::endl;
  dc << "TO CURRENCY: " << request.toCurrency() << std::endl;
  dc << "AMOUNT TO BE CONVERTED: " << request.amountToBeConverted() << std::endl;
  dc << "COLLECT OB FEES: " << (TypeConvert::pssCharToBool(request.collectOBFee()) ? "T" : "F") << std::endl;
  dc << "COLLECT R Type OB FEES: " << (request.isCollectRTypeOBFee() ? "T" : "F") << std::endl;
  dc << "COLLECT T Type OB FEES: " << (request.isCollectTTypeOBFee() ? "T" : "F") << std::endl;
  dc << "COLLECT OC FEES: " << (request.isCollectOCFees() ? "T" : "F") << std::endl;

  if (!fallback::virtualFOPMaxOBCalculation(&trx))
    dc << "REQUESTED MAXIMUM OB FEE ONLY: " << (request.returnMaxOBFeeOnly() ? "T" : "F")
       << std::endl;

  dc << "SEV PROCESSVITADATAIND : " << (request.processVITAData() ? "T" : "F") << std::endl;
  dc << "OW PRICING RT TAX PROCESS: " << (request.owPricingRTTaxProcess() ? "T" : "F") << std::endl;
  dc << "EXPAND JUMP CABIN LOGIC: " << ((request.getJumpCabinLogic() != JumpCabinLogic::ENABLED) ? "T" : "F") << std::endl;
  dc << "VCX VALIDATING CARRIER REQUEST: " << (request.isValidatingCarrierRequest() ? "T":"F" ) << std::endl;
  dc << "SM0 SETTLEMENT METHOD OVERRIDE: " << request.getSettlementMethod() << std::endl;
  dc << "DVL ALTERNATE VALIDATING CARRIER REQUEST: " << (request.isAlternateValidatingCarrierRequest() ? "T":"F" ) << std::endl;
  dc << "S8 BRANDED FARE REQUEST: " << (request.isBrandedFaresRequest() ? "T" : "F") << std::endl;
  dc << "IBF CATCH ALL BUCKET: " << (request.isCatchAllBucketRequest() ? "T" : "F") << std::endl;
  dc << "CHEAPEST OPTION WITH BRAND PARITY PER LEG: " << (request.isCheapestWithLegParityPath() ? "T" : "F") << std::endl;
  dc << "IBF CHANGE BRANDS WHEN SOLDOUT: " << (request.isChangeSoldoutBrand() ? "T" : "F")
     << std::endl;
  dc << "IBF CHANGE BRANDS WHEN NO FARES FILED: " << (request.isUseCbsForNoFares() ? "T" : "F")
     << std::endl;
  dc << "IBF PROCESS PARITY BRANDS OVERRIDE: " << (request.isProcessParityBrandsOverride() ? "T" : "F")
     << std::endl;
  dc << "MIP BRANDING REQUEST: " << (isBrandingTrx() ? "T" : "F") << std::endl;
  dc << "DROP RESULTS ON TIMEOUT: " << (request.isDropResultsOnTimeout() ? "T" : "F") << std::endl;
  addBrandInfo(request);
}

void
Diag198Collector::addDiscountPlusUpInfo(const PricingRequest& request, const Trx& trx)
{
  DiagCollector& dc = *this;

  const PricingTrx* ptrx = dynamic_cast<const PricingTrx*>(&trx);
  const bool isMipTrx = ptrx && ptrx->isMip();

  if (TrxUtil::newDiscountLogic(request, trx))
  {
    if (fallback::excDiscDiag23XImprovements(&trx))
    {
      dc << "DISCOUNT AMOUNT:";

      if (request.isDAEntryNew())
      {
        if (isMipTrx)
        {
          dc << " " << request.getDiscountAmountsNew().front().amount << " "
             << request.getDiscountAmountsNew().front().currencyCode;
        }
        else
        {
          for (const auto& elem : request.getDiscountAmountsNew())
            dc << "\n  SEGMENT: " << elem.startSegmentOrder << "-" << elem.endSegmentOrder
               << " AMOUNT: " << elem.amount << " " << elem.currencyCode;
        }
      }
      dc << '\n';

      dc << "DISCOUNT PERCENTAGE:";

      if (request.isDPEntryNew())
      {
        if (isMipTrx)
        {
          dc << " " << request.getDiscountPercentagesNew().cbegin()->second;
        }
        else
        {
          for (const auto& elem : request.getDiscountPercentagesNew())
            dc << "\n  SEGMENT: " << elem.first << " PERCENT: " << elem.second;
        }
      }
      dc << '\n';
    }
  }
  else
  {
    dc << "DISCOUNT AMOUNT:" << std::endl;

    if (request.getDiscountAmounts().size() > 0)
    {
      for (const auto& elem : request.getDiscountAmounts())
      {
        dc << "  SEGMENT: " << elem.startSegmentOrder << "-" << elem.endSegmentOrder
           << " AMOUNT: " << elem.amount << " CURRENCY CODE: " << elem.currencyCode << "\n";
      }
    }

    dc << "DISCOUNT PERCENTAGE:" << std::endl;

    if (request.getDiscountPercentages().size() > 0)
    {
      for (const auto& elem : request.getDiscountPercentages())
      {
        dc << "  SEGMENT: " << elem.first << " PERCENT: " << elem.second << "\n";
      }
    }
  }

  if (TrxUtil::newDiscountLogic(request, trx))
  {
    printMarkUp(dc, request.discountsNew(), isMipTrx, trx);
  }
}

void
Diag198Collector::addValidatingCxrInfo(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "IS VALIDATING CARRIER APPLICABLE: " << (trx.isValidatingCxrGsaApplicable() ? "T":"F" ) << std::endl;
}

void
Diag198Collector::addMultiTicketInd(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  if (TrxUtil::isMultiTicketPricingEnabled(trx))
  {
    bool isMultiTicketRequest = trx.getRequest()->isMultiTicketRequest();

    dc << "MULTI TICKET REQUEST: " << (isMultiTicketRequest ? "T" : "F") << std::endl;
    dc << "MULTI TICKET ACTIVE : "
       << ( !isMultiTicketRequest ? "" : (trx.getRequest()->multiTicketActive() ? "YES" : "NO"))
       << std::endl;
  }
}

void
Diag198Collector::addPbbInd(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "PBB REQUEST: " << (trx.isPbbRequest() != NOT_PBB_RQ ? "T" : "F") << std::endl;
}

void
Diag198Collector::addBrandedFaresTNShoppingInd(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "***** START BRANDS FOR TN SHOPPING OPTIONS *****\n";
  dc << "BRTNS REQUEST: "
     << ((trx.getTnShoppingBrandingMode() == TnShoppingBrandingMode::SINGLE_BRAND) ? "T" : "F")
     << std::endl;
  dc << "BRALL REQUEST: "
     << ((trx.getTnShoppingBrandingMode() == TnShoppingBrandingMode::MULTIPLE_BRANDS) ? "T" : "F")
     << std::endl;

  const size_t brandsRequested = trx.getNumberOfBrands();
  dc << "NBR OF BRANDS REQUESTED: ";
   if (brandsRequested != 0)
     dc << brandsRequested << std::endl;
   else
     dc << "UNLIMITED\n";

  dc << "***** END BRANDS FOR TN SHOPPING OPTIONS *****\n";
}

void
Diag198Collector::addSearchForBrandsPricingInd(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "SEARCH FOR BRANDS PRICING REQUEST: " << (trx.activationFlags().isSearchForBrandsPricing() ? "T" : "F")  << std::endl;
}

void
Diag198Collector::endRequest()
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "***************** END REQUEST ****************************\n";
}

void
Diag198Collector::addOptions(const PricingTrx& trx)
{
  const PricingOptions& options = *trx.getOptions();

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "***************** START OPTIONS  **************************\n";
  dc << "FARE TYPE PRICING: ";
  if (options.specialFareType())
    dc << "EX" << std::endl;
  else if (options.incTourFareType())
    dc << "IT" << std::endl;
  else if (options.normalFareType())
    dc << "NL" << std::endl;
  else
    dc << " " << std::endl;
  dc << "PRIVATE FARES: " << options.privateFares() << std::endl;
  dc << "PUBLISHED FARES: " << options.publishedFares() << std::endl;
  dc << "XO FARES: " << options.xoFares() << std::endl;
  dc << "XC FARES: " << (options.forceCorpFares() ? "T" : "F") << std::endl;
  dc << "ONLINE FARES: " << options.onlineFares() << std::endl;
  dc << "IATA FARES: " << options.iataFares() << std::endl;
  dc << "FIRST FLIGHT REPEAT: " << options.firstFlightRepeat() << std::endl;
  dc << "CURRENCY OVERRIDE: " << options.currencyOverride() << std::endl;
  dc << "TICKET STOCK: " << options.ticketStock() << std::endl;
  dc << "TICKET TIME OVERRIDE: " << options.ticketTimeOverride() << std::endl;
  dc << "TICKET DATE OVERRIDE INDICATOR: " << (options.isTicketingDateOverrideEntry() ? "T" : "F")
     << std::endl;
  dc << "RETURN ALL DATA: " << options.returnAllData() << std::endl;
  dc << "NO PENALTIES: " << options.noPenalties() << std::endl;
  dc << "NO ADVANCED PURCH RESTR: " << options.noAdvPurchRestr() << std::endl;
  dc << "NO MIN MAX STAY RESTR: " << options.noMinMaxStayRestr() << std::endl;
  dc << "NORMAL FARE: " << options.normalFare() << std::endl;
  dc << "CAT35 NOT ALLOWED: " << options.cat35NotAllowed() << std::endl;
  dc << "CAT35 FARE SHOULD BE IGNORED: " << options.cat35FareShouldBeIgnored() << std::endl;
  dc << "FARE BY RULE ENTRY: " << options.fareByRuleEntry() << std::endl;
  dc << "THRU FARES: " << options.thruFares() << std::endl;
  dc << "WEB: " << options.web() << std::endl;
  dc << "FARE BY RULE SHIP REGISTRY: " << options.fareByRuleShipRegistry() << std::endl;

  if (options.excludeCarrier().size() > 0)
  {
    std::vector<std::string>::const_iterator excludeIter = options.excludeCarrier().begin();
    std::vector<std::string>::const_iterator excludeEnd = options.excludeCarrier().end();
    dc << "***** START EXCLUDE CARRIERS *****\n";
    for (; excludeIter != excludeEnd; excludeIter++)
      dc << "     " << *excludeIter << std::endl;
    dc << "***** END EXCLUDE CARRIERS *****\n";
  }

  dc << "ALTERNATE CURRENCY: " << options.alternateCurrency() << std::endl;
  dc << "FARE CALCULATION DISPLAY: "
     << (options.fareCalculationDisplay() == '\0' ? ' ' : options.fareCalculationDisplay())
     << std::endl;

  if (options.eprKeywords().size() > 0)
  {
    std::set<std::string>::const_iterator eprIter = options.eprKeywords().begin();
    std::set<std::string>::const_iterator eprEnd = options.eprKeywords().end();
    dc << "***** START EPR KEYWORDS *****\n";
    for (; eprIter != eprEnd; eprIter++)
      dc << "     " << *eprIter << std::endl;
    dc << "***** END EPR KEYWORDS *****\n";
  }

  dc << "M OVERRIDE: " << options.mOverride() << std::endl;
  dc << "CAT 35 SELL: " << options.cat35Sell() << std::endl;
  dc << "CAT 35 NET: " << options.cat35Net() << std::endl;
  dc << "CAT 35 NETSELL: " << (options.cat35NetSell() ? "T" : "F") << std::endl;
  dc << "RECORD QUOTE: " << options.recordQuote() << std::endl;
  dc << "NATIONALITY: " << options.nationality() << std::endl;
  dc << "RESIDENCY: " << options.residency() << std::endl;
  dc << "EMPLOYMENT: " << options.employment() << std::endl;
  dc << "AMOUNT: " << options.amount() << std::endl;
  dc << "BASE COUNTRY: " << options.baseCountry() << std::endl;

  dc << "COMMAND TEXT: " << options.commandText() << std::endl;
  dc << "BASE CURRENCY: " << options.baseCurrency() << std::endl;
  dc << "BASE DATE: " << options.baseDT().dateToString(DDMMMYY, "") << std::endl;
  dc << "RECIPROCAL: " << options.reciprocal() << std::endl;
  dc << "SORT TAX BY ORIG CITY: " << options.isSortTaxByOrigCity() << std::endl;
  dc << "FBC SELECTED: " << options.fbcSelected() << std::endl;
  dc << "JOURNEY ACTIVATED FOR PRICING: " << options.journeyActivatedForPricing() << std::endl;
  dc << "JPS ENTERED: " << options.jpsEntered() << std::endl;
  dc << "IGNORE FUEL SURCHARGE: " << (options.ignoreFuelSurcharge() ? "T" : " ") << std::endl;
  dc << "BOOKING CODE OVERRIDE: " << (options.bookingCodeOverride() ? "T" : " ") << std::endl;
  dc << "FREE BAGGAGE SUBSCRIBER: " << (options.freeBaggage() ? "T" : " ") << std::endl;
  dc << "PNR: " << options.pnr() << std::endl;
  dc << "LINE ENTRY: " << options.lineEntry().substr(0, options.lineEntry().size() - 1)
     << std::endl;
  dc << "ADVANCE PURCHASE OPTION: " << ((options.AdvancePurchaseOption() == 'T') ? "B" : "A")
     << std::endl;
  dc << "MWI MIPWITHOUTPREVIOUSIS: " << (options.MIPWithoutPreviousIS() ? "T" : "F") << std::endl;
  dc << "VTI VALIDATETICKETINGAGREEMENTIND: " << (options.validateTicketingAgreement() ? "T" : "F")
     << std::endl;

  dc << "OC FEES SAME TIME TICKETING: " << (options.isTicketingInd() ? "T" : "F") << std::endl;
  dc << "OC FEES SUMMARY REQUEST: " << (options.isSummaryRequest() ? "T" : "F") << std::endl;
  dc << "CARNIVAL SUM OF LOCALS: " << (options.isCarnivalSumOfLocal() ? "T" : "F") << std::endl;
  dc << "ENABLE ZERO FARE: " << (options.isZeroFareLogic() ? "T" : "F") << std::endl;

  if (options.isCarnivalSumOfLocal())
    dc << "SOL GROUP GENERATION CONFIGURATION: " << options.getSolGroupGenerationConfig()
       << std::endl;

  bool processAllGroups = trx.getOptions()->isProcessAllGroups() &&
                          !fallback::fallbackAB240(&trx);

  if (!options.serviceGroupsVec().empty() || processAllGroups)
  {
    dc << std::endl << "OC FEES GROUP CODES PARSED: " << std::endl;

    std::vector<RequestedOcFeeGroup>::const_iterator ocFeeGroupIter =
        options.serviceGroupsVec().begin();
    std::vector<RequestedOcFeeGroup>::const_iterator ocFeeGroupIterEnd =
        options.serviceGroupsVec().end();

    for (; ocFeeGroupIter != ocFeeGroupIterEnd; ++ocFeeGroupIter)
    {
      const RequestedOcFeeGroup& ocFeeGroup = (*ocFeeGroupIter);

      dc << "  GROUP CODE: " << ocFeeGroup.groupCode();

      if (options.isSummaryRequest())
      {
        dc << ", NO OF ITEMS: " << ocFeeGroup.numberOfItems();
      }

      if (ocFeeGroup.getRequestedInformation() != RequestedOcFeeGroup::NoData)
      {
        if (ocFeeGroup.getRequestedInformation() == RequestedOcFeeGroup::DisclosureData)
          dc << ",  DISCLOSURE";
        if (ocFeeGroup.getRequestedInformation() == RequestedOcFeeGroup::AncillaryData)
          dc << ",  ANCILLARY";
        if (ocFeeGroup.getRequestedInformation() == RequestedOcFeeGroup::CatalogData)
          dc << ",  BAG CATALOG";
      }
      dc << std::endl;
    }
    dc << std::endl;
  }

  dc << "MAX NUMBER OF OC FEES PER ITIN: " << options.maxNumberOfOcFeesForItin() << std::endl;

  dc << "OC FEE GROUPS SUMMARY CONFIGURATION: " << options.groupsSummaryConfig() << std::endl;

  if (!options.groupsSummaryConfigVec().empty())
  {
    dc << std::endl << "OC FEE GROUPS SUMMARY CONFIGURATION PARSED: " << std::endl;

    std::vector<OcFeeGroupConfig>::const_iterator feeGroupConfigIter =
        options.groupsSummaryConfigVec().begin();
    std::vector<OcFeeGroupConfig>::const_iterator feeGroupConfigIterEnd =
        options.groupsSummaryConfigVec().end();

    for (; feeGroupConfigIter != feeGroupConfigIterEnd; ++feeGroupConfigIter)
    {
      const OcFeeGroupConfig& feeGroupConfig = (*feeGroupConfigIter);

      dc << "  Group: " << feeGroupConfig.groupCode();

      if (feeGroupConfig.startRange() != -1)
      {
        dc << ", Quantity: " << feeGroupConfig.startRange() << " - " << feeGroupConfig.endRange();
      }
      else
      {
        dc << ", Quantity: N/A";
      }

      if (!feeGroupConfig.commandName().empty())
      {
        dc << ", Command: " << feeGroupConfig.commandName();
      }

      if (!feeGroupConfig.subTypeCodes().empty())
      {
        dc << ", Sub Codes: ";

        std::vector<ServiceSubTypeCode>::const_iterator subCodeIter =
            feeGroupConfig.subTypeCodes().begin();
        std::vector<ServiceSubTypeCode>::const_iterator subCodeIterEnd =
            feeGroupConfig.subTypeCodes().end();

        for (; subCodeIter != subCodeIterEnd; ++subCodeIter)
        {
          const ServiceSubTypeCode& subCode = (*subCodeIter);

          dc << subCode;

          if ((subCodeIterEnd - subCodeIter) > 1)
          {
            dc << "-";
          }
        }
      }

      dc << std::endl;
    }
  }

  if (!trx.paxType().empty())
  {
    std::vector<PaxType::FreqFlyerTierWithCarrier*>& ffsdVec =
        trx.paxType().at(0)->freqFlyerTierWithCarrier();

    if (ffsdVec.empty())
    {
      dc << "FREQUENT FLYER INFO: N/A" << std::endl;
    }
    else
    {
      dc << std::endl << "FREQUENT FLYER INFO:" << std::endl << std::endl;

      std::vector<PaxType::FreqFlyerTierWithCarrier*>::const_iterator ffsdIter = ffsdVec.begin();
      std::vector<PaxType::FreqFlyerTierWithCarrier*>::const_iterator ffsdIterEnd = ffsdVec.end();

      for (; ffsdIter != ffsdIterEnd; ++ffsdIter)
      {
        dc << "  FREQUENT FLYER - STATUS: " << (*ffsdIter)->freqFlyerTierLevel() << std::endl;

        if ((*ffsdIter)->cxr().empty())
        {
          dc << "  FREQUENT FLYER - CARRIER CODE: "
             << "NOT SPECIFIED" << std::endl;
        }
        else
        {
          dc << "  FREQUENT FLYER - CARRIER CODE: " << (*ffsdIter)->cxr() << std::endl;
        }
        dc << std::endl;
      }
    }
  }

  if (options.getSpanishLargeFamilyDiscountLevel() == SLFUtil::DiscountLevel::LEVEL_1)
    dc << "SPANISH LARGE FAMILY DISCOUNT: LEVEL 1 - 5 PERCENT" << std::endl;
  else if (options.getSpanishLargeFamilyDiscountLevel() == SLFUtil::DiscountLevel::LEVEL_2)
    dc << "SPANISH LARGE FAMILY DISCOUNT: LEVEL 2 - 10 PERCENT" << std::endl;
  else
    dc << "SPANISH LARGE FAMILY DISCOUNT: N/A" << std::endl;

  const FareDisplayOptions* fdOptions = dynamic_cast<const FareDisplayOptions*>(&options);
  if (fdOptions)
  {
    dc << "SELLING CURRENCY OVERRIDE: " << (fdOptions->isSellingCurrency() ? "T" : " ")
       << std::endl;

    dc << "FARES WITH SPECIFIED TARIFF NUM: " << fdOptions->frrTariffNumber() << std::endl;
    dc << "FARES WITH SPECIFIED RULE NUM: " << fdOptions->frrRuleNumber() << std::endl;
    dc << "FARES WITH SPECIFIED FARE TYPE CODE: " << fdOptions->frrFareTypeCode() << std::endl;
    dc << "FARES WITH SPECIFIED DISPLAY TYPE: " << fdOptions->frrDisplayType() << std::endl;
    dc << "FARES WITH SPECIFIED PRIVATE INDICATOR: " << fdOptions->frrPrivateIndicator() << std::endl;
    dc << "SELECT NEGOTIATED FARES: "   << (fdOptions->frrSelectCat35Fares() ? "T" : "F") << std::endl;
    dc << "SELECT FARE BY RULE FARES: " << (fdOptions->frrSelectCat25Fares() ? "T" : "F") << std::endl;
    dc << "SELECT SALES RESTRICTION FARES: "       << (fdOptions->frrSelectCat15Fares() ? "T" : "F") << std::endl;
  }

  const PricingRequest* request = trx.getRequest();
  if (!fallback::fallbackFRRProcessingRetailerCode(&trx) && request != nullptr)
  {
    bool frCodeExists = !request->rcqValues().empty();
    dc << "MATCH RETAILER CODE: " << (frCodeExists ? "T" : "F") << std::endl;
    if (frCodeExists)
    {
      dc << "FORCE RETAILER CODE: " << (request->prmValue() ? "T" : "F") << std::endl;
      dc << "***** START RETAILER CODE ***** " << std::endl;
      for (const auto& frCode : request->rcqValues())
        dc << frCode << std::endl;
      dc << "***** END RETAILER CODE *****" << std::endl;
    }
  }

  if (trx.getTrxType() != PricingTrx::PRICING_TRX &&
      trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
  {
    dc << std::endl << "CALLENDAR SHOPPING FOR INTERLINES:" << std::endl;
    dc << "  PA0 ENABLE_CALENDAR_FOR_INTERLINES             : "
       << (options.isEnableCalendarForInterlines() ? 'T' : 'F') << std::endl;
    if (options.isEnableCalendarForInterlines())
    {
      dc << "  PA1 SPLIT_TAXES_BY_LEG                         : "
         << (options.isSplitTaxesByLeg() ? 'T' : 'F') << std::endl;
      dc << "  PA2 SPLIT_TAXES_BY_FARE_COMPONENT              : "
         << (options.isSplitTaxesByFareComponent() ? 'T' : 'F') << std::endl;
    }
    dc << std::endl;
  }

  dc << "ROUND THE WORLD: " << (options.isRtw() ? 'T' : 'F') << std::endl;
  dc << "EXCLUDE FARE FOCUS RULES: " << (options.isExcludeFareFocusRule() ? 'T' : 'F') << std::endl;
  dc << "ASL ORIGINAL FARE: " << (options.isPDOForFRRule() ? 'T' : 'F') << std::endl;
  dc << "ASL AGENCY RETAIL DETAIL: " << (options.isPDRForFRRule() ? 'T' : 'F') << std::endl;
  dc << "ASL IGNORE SELLING LEVEL: " << (options.isXRSForFRRule() ? 'T' : 'F') << std::endl;


  const std::string fsActive = trx.isIataFareSelectionApplicable() ? "YES" : "NO";
  dc << "FARE SELECTION ACTIVE: " << fsActive << std::endl;
  dc << "RBD BY CABIN ANSWER TABLE ACTIVE: " << (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) ?
        "YES" : "NO" ) << std::endl;

  dc << "NUMBER OF FARES FOR ALTERNATE DATES PRECALCULATION: " << options.getNumFaresForCat12Estimation() << std::endl;

  dc << "RESERVATION/CHECK-IN PATH : " << boost::format("%|-8|") % ((AncRequestPath::AncCheckInPath == trx.getOptions()->getAncRequestPath()) ? "CHECK-IN" : "RES") << std::endl;

  if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
  {
    dc << "FQ - PRICE BY CABIN ACTIVE: " << (!fallback::fallbackFareDisplayByCabinActivation(&trx) ?
          "YES" : "NO" ) << std::endl;

    const FareDisplayRequest* fdRequest = dynamic_cast<const FareDisplayRequest*>((trx.getRequest()));
    if (!fallback::fallbackFareDisplayByCabinActivation(&trx) &&
         fdRequest && !fdRequest->inclusionCode().empty())
      displayCabinInclusionCode(*fdRequest);
  }

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    dc << "PRICE BY CABIN ACTIVE: " << (!fallback::fallbackPriceByCabinActivation(&trx) ?
          "YES" : "NO" ) << std::endl;
  }

  if (!fallback::fallbackPriceByCabinActivation(&trx) && !options.cabin().isUndefinedClass())
  {
    dc << "CABIN REQUESTED :";
    if (options.cabin().isPremiumFirstClass())
      dc << " PB - PREMIUM FIRST" << std::endl;
    else if (options.cabin().isFirstClass())
      dc << " FB - FIRST" << std::endl;
    else if (options.cabin().isPremiumBusinessClass())
      dc << " JB - PREMIUM BUSINESS" << std::endl;
    else if (options.cabin().isBusinessClass())
      dc << " BB - BUSINESS" << std::endl;
    else if (options.cabin().isPremiumEconomyClass())
      dc << " SB - PREMIUM ECONOMY" << std::endl;
    else if (options.cabin().isEconomyClass())
      dc << " YB - ECONOMY" << std::endl;
    else if (options.cabin().isAllCabin())
      dc << " AB - ALL CABINS" << std::endl;
  }

  if (!fallback::fallbackAAExcludedBookingCode(&trx))
  {
    if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    {
      dc << "PRICE BY EXCLUDED BOOKING CODE: " << options.aaBasicEBC() << std::endl;
    }
  }

  dc << "***************** END OPTIONS *****************************\n";
}

void
Diag198Collector::addAgent(const Agent& agent)
{
  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx* priTrx = dynamic_cast<PricingTrx*>(trx());
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "       ABACUS USER: " << (agent.abacusUser() ? "T" : " ") << std::endl;
  dc << "       JAL/AXESS USER: " << (agent.axessUser() ? "T" : " ") << std::endl;
  dc << "       CWT USER: " << (agent.cwtUser() ? "T" : " ") << std::endl;
  dc << "       VENDOR CRS CODE: " << agent.vendorCrsCode() << std::endl;
  dc << "       AGENT CITY: " << agent.agentCity() << std::endl;
  dc << "       AA CITY: " << ((agent.agentTJR() != nullptr) ? agent.agentTJR()->aaCity() : "")
     << std::endl;
  if (!fallback::dffOaFareCreation(trx()))
     dc << "       AS CLIENT: " << (TrxUtil::isRequestFromAS(*priTrx) ? "Y" : "N") << std::endl;
  dc << "       TRAVEL AGENCY PCC: " << agent.tvlAgencyPCC() << std::endl;

  if (!agent.officeDesignator().empty())
    dc << "       OFFICE DESIGNATOR: " << agent.officeDesignator() << std::endl;
  if (!agent.officeStationCode().empty())
    dc << "       OFFICE/STATION CODE: " << agent.officeStationCode() << std::endl;
  if (!agent.defaultTicketingCarrier().empty())
    dc << "       DEFAULT TICKETING CARRIER: " << agent.defaultTicketingCarrier() << std::endl;

  dc << "       MAIN TRAVEL AGENCY PCC: " << agent.mainTvlAgencyPCC() << std::endl;
  dc << "       TRAVEL AGENCY IATA: " << agent.tvlAgencyIATA() << std::endl;
  dc << "       HOME AGENCY IATA: " << agent.homeAgencyIATA() << std::endl;
  dc << "       AGENT FUNCTIONS: " << agent.agentFunctions() << std::endl;
  dc << "       AGENT DUTY: " << agent.agentDuty() << std::endl;
  dc << "       AIRLINE DEPT: " << agent.airlineDept() << std::endl;
  dc << "       CXR CODE: " << agent.cxrCode() << std::endl;
  dc << "       AGENT CURRENCY CODE: " << agent.currencyCodeAgent() << std::endl;
  dc << "       CO HOST ID: " << agent.coHostID() << std::endl;
  dc << "       AGENT COMMISSION TYPE: " << agent.agentCommissionType() << std::endl;
  dc << "       AGENT COMMISSION AMOUNT: " << agent.agentCommissionAmount() << std::endl;
  dc << "       COMMISSION AMOUNT: " << agent.commissionAmount() << std::endl;
  dc << "       COMMISSION PERCENT: " << agent.commissionPercent() << std::endl;
  dc << "       OPT IN AGENCY: " << ((agent.agentTJR() != nullptr) ? agent.agentTJR()->optInAgency()
                                                             : 'N') << std::endl;
  if (agent.agentTJR() != nullptr)
  {
    dc << "       DO NOT APPLY OB TKT FEE: " << agent.agentTJR()->doNotApplyObTktFees()
       << std::endl;
    dc << "       DEFAULT PAX TYPE: ";
    if (!agent.agentTJR()->defaultPassengerType().empty())
      dc << agent.agentTJR()->defaultPassengerType();
    else
      dc << ' ';
    dc << std::endl;
    // Added for Infini cat05 project
    DataHandle dataHandle(priTrx->getRequest()->ticketingDT());
    PseudoCityCode pcc = agent.tvlAgencyPCC();
    if (TrxUtil::isPricingInfiniCat05ValidationSkipActivated(*priTrx))
    {
      dc << "       CAT05 OVERRIDE BOOKING DATE VALIDATING CARRIER:";
      // read the cat05 override cxrs for infini pccs
      if (priTrx->getRequest()->ticketingAgent()->infiniUser())
      {
        Alpha3Char overrideCode = agent.agentTJR()->cat05OverrideCode();
        if ((overrideCode == tse::ANY_CARRIER) || (overrideCode == " "))
        {
          dc << std::setw(3) << overrideCode << std::endl;
        }
        else // read the list of override carriers from the overridecxr table
        {
          const std::vector<Cat05OverrideCarrier*>& ovCxrList =
              dataHandle.getCat05OverrideCarrierInfo(pcc);
          if (ovCxrList.size())
          {
            const std::vector<CarrierCode>& cxrList = ovCxrList[0]->carrierList();
            size_t numCxr = cxrList.size();
            for (size_t i = 0; i < numCxr; ++i)
            {
              if (i == 3)
                dc << std::endl << "             ";
              dc << std::setw(3) << cxrList[i];
            }
          }
          dc << std::endl;
        }
      }
      else
        dc << std::endl;
    }
  }
  const Loc& loc = *agent.agentLocation();
  dc << "       AGENT LOCATION: " << std::endl;
  dc << "             LOC CODE: " << loc.loc() << std::endl;
  dc << "             EXPIRE DATE: " << loc.expireDate().dateToString(DDMMMYY, "") << std::endl;
  dc << "             EFFECTIVE DATE: " << loc.effDate().dateToString(DDMMMYY, "") << std::endl;
  dc << "             SUBAREA: " << loc.subarea() << std::endl;
  dc << "             AREA: " << loc.area() << std::endl;
  dc << "             TRANSTYPE: " << loc.transtype() << std::endl;
  dc << "             CITY: " << loc.city() << std::endl;
  dc << "             NATION: " << loc.nation() << std::endl;
  dc << "             STATE: " << loc.state() << std::endl;
  dc << "             DST GRP: " << loc.dstgrp() << std::endl;
  dc << "             ALASKA ZONE: " << loc.alaskazone() << std::endl;
  dc << "             DESCRIPTION: " << loc.description() << std::endl;
  dc << "             LAT HEM: " << loc.lathem() << std::endl;
  dc << "             LAT DEG: " << loc.latdeg() << std::endl;
  dc << "             LAT MIN: " << loc.latmin() << std::endl;
  dc << "             LAT SEC: " << loc.latsec() << std::endl;
  dc << "             LONG HEM: " << loc.lnghem() << std::endl;
  dc << "             LONG DEG: " << loc.lngdeg() << std::endl;
  dc << "             LONG MIN: " << loc.lngmin() << std::endl;
  dc << "             LONG SEC: " << loc.lngsec() << std::endl;
  dc << "             CITY IND: " << loc.cityInd() << std::endl;
  dc << "             BUFFER ZONE IND: " << loc.bufferZoneInd() << std::endl;
  dc << "             RURAL ARP IND: " << loc.ruralarpind() << std::endl;
  dc << "             MULTI TRANS IND: " << loc.multitransind() << std::endl;
  dc << "             FARES IND: " << loc.faresind() << std::endl;
}

void
Diag198Collector::addDefaultAgent(const PricingRequest& request)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "****** START AGENT ******\n";
  addAgent(*(request.ticketingAgent()));
  dc << "****** END AGENT ********\n";
}

void
Diag198Collector::addTicketingAgent(const PricingRequest& request, const Trx& trx)
{
  const PricingRequest* prRq = &request;
  const AncRequest* ancRq = dynamic_cast<const AncRequest*>(prRq);
  const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(&trx);
  if (ancRq && pricingTrx)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    for (auto itin : pricingTrx->itin())
    {
      dc << "****** START TICKETING AGENT ******\n";

      if (!itin)
      {
        dc << "       ERROR: ITIN IS NULL\n";
        continue;
      }

      if (ancRq->isTicketNumberValid(itin->ticketNumber()))
      {
        dc << "       ITIN " << static_cast<unsigned>(itin->getItinOrderNum());
        dc << ", TKN " << static_cast<unsigned>(itin->ticketNumber()) << "\n";

        ancRq->setActiveAgent(AncRequest::TicketingAgent, itin->ticketNumber());

        if (request.ticketingAgent())
          addAgent(*(request.ticketingAgent()));
        else
          dc << "       TICKETING AGENT NOT SPECIFIED\n";

        ancRq->setActiveAgent(AncRequest::CheckInAgent);
      }
      else
      {
        dc << "       TICKETING AGENT NOT SPECIFIED\n";
      }

      dc << "****** END TICKETING AGENT ********\n";
    }
  }
}

void
Diag198Collector::addConsolidatorPlusUp(const Itin& itin)
{
  if (!itin.isPlusUpPricing())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "****** START CONSOLIDATOR PLUS UP ******\n";

  dc << "AMOUNT: " << itin.consolidatorPlusUp()->amount() << std::endl;
  dc << "CURRENCY: " << itin.consolidatorPlusUp()->currencyCode() << std::endl;
  dc << "TKT DESIGNATOR: " << itin.consolidatorPlusUp()->tktDesignator() << std::endl;

  dc << "****** END CONSOLIDATOR PLUS UP ********\n";
}

void
Diag198Collector::addExchangePricingDates(const PricingTrx& trx)
{
  if (trx.excTrxType() != PricingTrx::PORT_EXC_TRX &&
      (trx.getTrxType() != PricingTrx::MIP_TRX || trx.excTrxType() != PricingTrx::AR_EXC_TRX))
    return;

  const BaseExchangeTrx& excTrx = static_cast<const BaseExchangeTrx&>(trx);

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "************** START EXCHANGE PRICING DATES ***************\n";
  dc << "DATAHANDLE TICKET DATE  D93: "
     << excTrx.dataHandle().ticketDate().dateToString(YYYYMMDD, "-") << std::endl;
  dc << "TRX TICKET DATE         D93: " << excTrx.ticketingDate().dateToString(YYYYMMDD, "-")
     << std::endl;
  dc << "CURRENT DATE            D07: " << excTrx.currentTicketingDT().dateToString(YYYYMMDD, "-")
     << std::endl;
  dc << "PURCHASE DATE           D93: " << excTrx.purchaseDT().dateToString(YYYYMMDD, "-")
     << std::endl;
  dc << "ORIGINAL TKT ISSUE DATE D92: " << excTrx.originalTktIssueDT().dateToString(YYYYMMDD, "-")
     << std::endl;
  dc << "************* END EXCHANGE PRICING DATES ****************** \n";
}
void
Diag198Collector::addExchangeInfo(const PricingTrx& trx)
{
  // Display only for exchange
  if (trx.excTrxType() != PricingTrx::AR_EXC_TRX)
  {
    return;
  }

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  const RexPricingTrx* const rexPricingTrx = dynamic_cast<const RexPricingTrx*>(&trx);
  if (rexPricingTrx == nullptr)
    return;

  const RexPricingOptions& rexOptions =
      dynamic_cast<const RexPricingOptions&>(*rexPricingTrx->getOptions());
  dc << "*************** START EXCHANGE OPTIONS ********************\n";
  dc << "COUNTRYCODE NOM: " << rexOptions.excEmployment() << std::endl;
  dc << "NATIONALITY A40: " << rexOptions.excNationality() << std::endl;
  dc << "RESIDENCY   AHO: " << rexOptions.excResidency() << std::endl;

  const RexPricingRequest& rexRequest =
      dynamic_cast<const RexPricingRequest&>(*rexPricingTrx->getRequest());

  dc << "EXCVALIDATIONCARRIER B05: " << rexRequest.excValidatingCarrier() << "\n"
     << "EXCACCOUNTCODE       S11: " << rexRequest.excAccountCode() << "\n"
     << "WAIVERCODE           S94: " << rexPricingTrx->waiverCode() << "\n";

  ExcItin* const exchangeItin = rexPricingTrx->exchangeItin().back();

  if (exchangeItin != nullptr)
  {
    dc << "EXCITINCALCCURRENCY  C6Y: " << exchangeItin->calculationCurrency() << "\n"
       << "EXCITINCALCCUROVERR     : " << exchangeItin->calcCurrencyOverride() << "\n";
  }
  dc << "***************** END EXCHANGE OPTIONS ********************\n";

  dc << "************ START NEW ITIN OPTIONS ***********************\n";
  dc << "FAREAPPLDT           REX:  " << rexPricingTrx->fareApplicationDT() << std::endl;
  dc << "NEWITINCURRENCYOVERR C6Y: " << rexPricingTrx->newItin().back()->calcCurrencyOverride()
     << std::endl;
  dc << "BASEFARECURRENCY     C6P: " << rexOptions.baseFareCurrencyOverride() << std::endl;
  dc << "LASTTKTREISSUEDT     D94: " << rexPricingTrx->lastTktReIssueDT() << std::endl;
  dc << "**************** END NEW ITIN OPTIONS *********************\n";

  dc << "*************  START EXCHANGE PASSENGER TYPES *************\n";
  for (std::size_t i = 0; i < rexPricingTrx->excPaxType().size(); ++i)
  {
    addPassengerData(rexPricingTrx->excPaxType()[i], i+1, dc, trx);
  }
  dc << "*************  END EXCHANGE PASSENGER TYPES ***************\n";

  dc << "********* START EXCHANGE ACCOMPANY PASSENGER TYPES ********\n";
  for (std::size_t i = 0; i < rexPricingTrx->accompanyPaxType().size(); ++i)
  {
    addPassengerData(rexPricingTrx->accompanyPaxType()[i], i+1, dc, trx);
  }
  dc << "*********  END EXCHANGE ACCOMPANY PASSENGER TYPES *********\n";

  const Agent* rexAgent = rexRequest.prevTicketIssueAgent();
  dc << "************** START EXCHANGE TICKETING AGENT *************\n";
  if (rexAgent != nullptr)
  {
    dc << "AGENTCITY         A10: " << rexAgent->agentCity() << std::endl;
    dc << "TVLAGENCYPCC      A20: " << rexAgent->tvlAgencyPCC() << std::endl;
    dc << "MAINTVLAGENCYPCC  A21: " << rexAgent->mainTvlAgencyPCC() << std::endl;
    dc << "TVLAGENCYIATA     AB0: " << rexAgent->tvlAgencyIATA() << std::endl;
    dc << "HOMEAGENCYIATA    AB1: " << rexAgent->homeAgencyIATA() << std::endl;
    dc << "AGENTFUNCTIONS    A90: " << rexAgent->agentFunctions() << std::endl;
    dc << "AIRLINEDEPT       A80: " << rexAgent->airlineDept() << std::endl;
    dc << "AGENTDUTY         N0G: " << rexAgent->agentDuty() << std::endl;
    dc << "CXRCODE           B00: " << rexAgent->cxrCode() << std::endl;
    dc << "CURRENCYCODEAGENT C40: " << rexAgent->currencyCodeAgent() << std::endl;
    dc << "COHOSTID          Q01: " << rexAgent->coHostID() << std::endl;
    dc << "AGENTCOMMISSIONTYPE N0L: " << rexAgent->agentCommissionType() << std::endl;
    dc << "AGENTCOMMISIONAMT   C6C: " << rexAgent->agentCommissionAmount() << std::endl;
    if (rexRequest.getOriginalTicketAgentLocation())
      dc << "ORIGINAL AGENT LOCATION OAL: " << rexRequest.getOriginalTicketAgentLocation()->loc()
         << std::endl;
  }
  dc << "*********** END EXCHANGE TICKETING AGENT ******************\n";

  dc << "************* START EXCHANGE FARE COMPONENTS **************\n";
  for (const auto fareComponent : exchangeItin->fareComponent())
  {
    if (fareComponent != nullptr)
    {
      dc << "FARECOMPNUMBER Q6D: " << fareComponent->fareCompNumber() << std::endl;
      dc << "FAREBASISCODE  B50: " << fareComponent->fareBasisCode() << std::endl;
      dc << "VTCRVENDOR     S37: " << fareComponent->VCTR().vendor() << std::endl;
      dc << "VTCRCARRIER    B09: " << fareComponent->VCTR().carrier() << std::endl;
      dc << "VTCRTARIFF     S89: " << fareComponent->VCTR().tariff() << std::endl;
      dc << "VTCRRULE       S90: " << fareComponent->VCTR().rule() << std::endl;
      dc << "HASVTCR           : " << (fareComponent->hasVCTR() ? "TRUE" : "FALSE") << std::endl;
      dc << "MILEAGE DISPLAY CITY        AP3: " << fareComponent->mileageSurchargeCity()
         << std::endl;
      dc << "MILEAGE SURCHAGE PERCENTAGE Q48: " << fareComponent->mileageSurchargePctg()
         << std::endl;
      dc << std::endl;
    }
  }
  dc << "************** END EXCHANGE FARE COMPONENTS ***************\n";
}

void
Diag198Collector::addExchangePlusUpInfo(const RexBaseTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)(*this);

  dc << "****************** START EXCHANGE PLUS UP *****************\n";

  dc << "PROCESS TYPE INDICATOR FOR PRIMARY REQUEST TYPE N25: "
     << (trx.applyReissueExcPrimary() ? "true" : "false") << std::endl;
  dc << "PREVIOUS EXCHANGE DATE D95: " << trx.previousExchangeDT() << std::endl;
  dc << "BASE FARE AMOUNT C5A: " << trx.getRexOptions().excTotalFareAmt() << std::endl;

  if (!fallback::azPlusUp(&trx))
  {
    printMarkUp(dc, trx.getRexRequest().excDiscounts(), false, trx);
  }

  ExcItin* const exchangeItin = trx.exchangeItin().back();
  if (!exchangeItin->fareComponent().empty() && exchangeItin->fareComponent().back() != nullptr)
  {
    FareCompInfo* fareComponent = exchangeItin->fareComponent().back();
    const MinimumFareOverride* minFareOverride =
        dynamic_cast<const MinimumFareOverride*>(fareComponent->hip());
    if (minFareOverride != nullptr)
    {
      dc << "PLUSUPAMOUNT    C6L: " << minFareOverride->plusUpAmount << std::endl;
      dc << "BOARDPOINT      A11: " << minFareOverride->boardPoint << std::endl;
      dc << "OFFPOINT        A12: " << minFareOverride->offPoint << std::endl;
      dc << "FAREBOARDPOINT  A13: " << minFareOverride->fareBoardPoint << std::endl;
      dc << "FAREOFFPOINT    A14: " << minFareOverride->fareOffPoint << std::endl;
      dc << "CONSTRUCTPOINT  A18: " << minFareOverride->constructPoint << std::endl;
      dc << "CONSTRUCTPOINT2 A19: " << minFareOverride->constructPoint2 << std::endl;
    }
  }
  dc << "******************* END EXCHANGE PLUS UP ******************\n";
}

void
Diag198Collector::addPassengerData(const PaxType* curPT,
                                   std::size_t count,
                                   DiagCollector& dc,
                                   const PricingTrx& trx)
{
  if (curPT != nullptr)
  {
    dc << "PAXTYPE " << count << std::endl;
    dc << "PAXTYPE        B70: " << curPT->paxType() << std::endl;
    dc << "AGE            Q0T: " << curPT->age() << std::endl;
    dc << "NUMBER         Q0U: " << curPT->number() << std::endl;
    dc << "STATE CODE     A30: " << curPT->stateCode() << std::endl;

    dc << "MANUAL ADJ AMT C51: " << curPT->mslAmount() << std::endl;

    if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      dc << "TOTAL NUMBER OF PASSENGERS Q79: " << curPT->totalPaxNumber() << std::endl;
    }

    if (const MaxPenaltyInfo* maxPenaltyInfo = curPT->maxPenaltyInfo())
    {
      dc << "MAX PENALTY                   MPO: ";
      switch (maxPenaltyInfo->_mode)
      {
      case smp::INFO:
        dc << 'I' << std::endl;
        break;
      case smp::OR:
        dc << 'O' << std::endl;
        break;
      case smp::AND:
        dc << 'A' << std::endl;
        break;
      }

      if (maxPenaltyInfo->_mode != smp::INFO)
      {
        if (maxPenaltyInfo->_changeFilter._departure != smp::BOTH)
        {
          dc << "CHANGE AFTER/BEFORE DEPARTURE ABD: "
             << smp::toDepartureAppl(maxPenaltyInfo->_changeFilter._departure) << std::endl;
        }

        if (maxPenaltyInfo->_changeFilter._maxFee)
        {
          const Money& maxChangeFee = maxPenaltyInfo->_changeFilter._maxFee.get();
          dc << "MAX CHANGE AMOUNT             MPA: " << maxChangeFee.value() << std::endl;
          dc << "MAX CHANGE CURRENCY           MPC: " << maxChangeFee.code() << std::endl;
        }
        else if (maxPenaltyInfo->_changeFilter._query)
        {
          dc << "ANY(CHANGEABLE)/NONCHANGEABLE MPI: "
             << (maxPenaltyInfo->_changeFilter._query.get() == smp::CHANGEABLE ? 'A' : 'N')
             << std::endl;
        }

        if (maxPenaltyInfo->_refundFilter._departure != smp::BOTH)
        {
          dc << "CHANGE AFTER/BEFORE DEPARTURE ABD: " << maxPenaltyInfo->_refundFilter._departure
             << std::endl;
        }

        if (maxPenaltyInfo->_refundFilter._maxFee)
        {
          const Money& maxRefundFee = maxPenaltyInfo->_refundFilter._maxFee.get();
          dc << "MAX REFUND AMOUNT             MPA: " << maxRefundFee.value() << std::endl;
          dc << "MAX REFUND CURRENCY           MPC: " << maxRefundFee.code() << std::endl;
        }
        else if (maxPenaltyInfo->_refundFilter._query)
        {
          dc << "ANY(REFUNDABLE)/NONREFUNDABLE MPI: "
             << (maxPenaltyInfo->_refundFilter._query.get() == smp::CHANGEABLE ? 'A' : 'N')
             << std::endl;
        }
      }
    }
  }
}

void
Diag198Collector::addExchangeCat31Info(const PricingTrx& trx)
{
  // Display only for exchange mip
  if (trx.excTrxType() != PricingTrx::AR_EXC_TRX)
  {
    return;
  }

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  const RexPricingTrx* const rexPricingTrx = dynamic_cast<const RexPricingTrx*>(&trx);
  if (rexPricingTrx == nullptr)
    return;

  dc << "*************** START EXCHANGE CAT31 INFO ********************\n";
  std::vector<FareComponentInfo*>::const_iterator excCAT31InfoIter =
      rexPricingTrx->excFareCompInfo().begin();
  for (; excCAT31InfoIter != rexPricingTrx->excFareCompInfo().end(); ++excCAT31InfoIter)
  {
    if (*excCAT31InfoIter)
    {
      dc << "FARECOMPONENNTNUMBER     Q6D: " << (*excCAT31InfoIter)->fareCompNumber() << std::endl;
      if ((*excCAT31InfoIter)->vctrInfo())
      {
        dc << "VCTRVENDOR               S37: " << (*excCAT31InfoIter)->vctrInfo()->vendor()
           << std::endl;
        dc << "VCTRCARRIER              B09: " << (*excCAT31InfoIter)->vctrInfo()->carrier()
           << std::endl;
        dc << "VCTRFARETARIFF           S89: " << (*excCAT31InfoIter)->vctrInfo()->fareTariff()
           << std::endl;
        dc << "VCTRRULENUMBER           S90: " << (*excCAT31InfoIter)->vctrInfo()->ruleNumber()
           << std::endl;
        dc << "VCTRRETRIEVALTIMEDATE    RTD: "
           << (*excCAT31InfoIter)->vctrInfo()->retrievalDate().dateToString(DDMMMYY, "")
           << std::endl;
      }
      std::vector<Cat31Info*>::const_iterator r3InfoIter = (*excCAT31InfoIter)->cat31Info().begin();
      for (; r3InfoIter != (*excCAT31InfoIter)->cat31Info().end(); ++r3InfoIter)
      {
        if (*r3InfoIter)
        {
          dc << "CAT31RECORD3ITEMNO       Q5Y: " << (*r3InfoIter)->rec3ItemNo << std::endl;
          for (const int seqInfo : (*r3InfoIter)->tab988SeqNo)
          {
            dc << "CAT31RECORD3TAB988SEQNO  Q5Z: " << seqInfo << std::endl;
          }
        }
      }
    }
  }
  dc << "**************** END EXCHANGE CAT31 INFO *********************\n";
}

void
Diag198Collector::addSnapInfo(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "SNAP REQUEST: " << (trx.snapRequest() ? "T" : "F") << "\n";
}

void
Diag198Collector::addAncillaryPricingFeeInfo(const PricingTrx& trx)
{
  const AncillaryPricingTrx* const ancTrx = dynamic_cast<const AncillaryPricingTrx*>(&trx);
  if (!ancTrx)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "       ANCILLARY PRICING REQUEST " << std::endl;
}

void
Diag198Collector::addAncRequestDetails(const PricingTrx& trx)
{
  if (!dynamic_cast<const AncillaryPricingTrx*>(&trx) && !dynamic_cast<const BaggageTrx*>(&trx))
    return;

  const AncRequest& request = dynamic_cast<const AncRequest&>(*trx.getRequest());
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "  HOURS BEFORE DEPARTURE: " << request.noHoursBeforeDeparture() << "\n";
  dc << "  HARD MATCH INDICATOR: " << (request.hardMatchIndicator() ? "Y\n" : "N\n");
  dc << "********** ANCILLARY PRICING FLIGHT SEGMENT DETAILS **********\n";

  std::vector<Itin*>::const_iterator iit = trx.itin().begin();
  std::vector<Itin*>::const_iterator iie = trx.itin().end();

  for (; iit != iie; iit++)
  {
    addItinInfo(request, *iit);
    if ( trx.activationFlags().isMonetaryDiscount() )
      addAncillaryPriceModifiersToItinInfo(*iit);

    if (request.pricingItins().find(*iit) != request.pricingItins().end())
    {
      std::vector<Itin*>::const_iterator it = request.pricingItins().find(*iit)->second.begin();
      std::vector<Itin*>::const_iterator ie = request.pricingItins().find(*iit)->second.end();
      dc << "***** PRICING ITINS *****\n";
      bool firstItin(true);
      for (; it != ie; it++)
      {
        addItinInfo(request, *it);
        if (firstItin)
          addItinInfoAddOn(request, *it, *iit);
        else
          addItinInfoAddOn(request, *it, *it);
        firstItin = false;
      }
    }
    else
      addItinInfoAddOn(request, *iit, *iit);
  }
}

std::string
Diag198Collector::formatAmount(MoneyAmount amount, const CurrencyCode& currencyCode) const
{
  CurrencyNoDec noDec = 2;

  if (currencyCode != NUC)
  {
    DataHandle dataHandle;
    const Currency* currency = nullptr;
    currency = dataHandle.getCurrency( currencyCode );

    if (currency)
    {
      noDec = currency->noDec();
    }
  }
  std::ostringstream os;

  os.setf(std::ios::fixed, std::ios::floatfield);
  os.precision(noDec);

  os << amount;

  return os.str();
}

void
Diag198Collector::addONDInfo(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  for (const auto& elem : trx.orgDest)
  {
    dc << "BOARD MULTI CITY: " << elem.boardMultiCity << "\n";
    dc << "OFF MULTI CITY: " << elem.offMultiCity << "\n";
    dc << "TRAVEL DATE: " << elem.travelDate << "\n";
    dc << "TRAVEL END DATE: " << elem.travelEndDate << "\n";
    dc << "LEG ID: " << elem.legID << "\n";
    dc << "MIN BUDGET AMOUNT: " << formatAmount(elem.minBudgetAmount, elem.currencyCode) << "\n";
    dc << "MAX BUDGET AMOUNT: " << formatAmount(elem.maxBudgetAmount, elem.currencyCode) << "\n";
    dc << "CURRENCY CODE: " << elem.currencyCode << "\n";
  }
}

void
Diag198Collector::addBrandInfo(const PricingRequest& request)
{
  DiagCollector& dc = (DiagCollector&)*this;
  const uint16_t brandedFareSize = request.getBrandedFareSize();
  for (uint16_t brandIndex = 0; brandIndex < brandedFareSize; brandIndex++)
  {
    dc << "BRAND ID          SB2: " << request.brandId(brandIndex) << std::endl;
    dc << "PROGRAM ID        SC0: " << request.programId(brandIndex) << std::endl;
    dc << "BRANDED FARE BKG CODE:";
    addBrandBkgVect(request.brandedFareBookingCode(brandIndex), dc);
    dc << "BRANDED FARE SECONDARY BKG CODE:";
    addBrandBkgVect(request.brandedFareSecondaryBookingCode(brandIndex), dc);
    dc << "BRANDED FARE BKG CODE EXCLUDE:";
    addBrandBkgVect<BookingCode>(request.brandedFareBookingCodeExclude(brandIndex), dc);
    dc << "BRANDED FARE SECONDARY BKG CODE EXCLUDE:";
    addBrandBkgVect<BookingCode>(request.brandedFareSecondaryBookingCodeExclude(brandIndex), dc);
    dc << "BRANDED FARE BKG CODE DATA:";
    addBrandBkgMap<BookingCode>(request.brandedFareBookingCodeData(brandIndex), dc);
    dc << "BRANDED FARE SECONDARY BKG CODE DATA:";
    addBrandBkgMap<BookingCode>(request.brandedFareSecondaryBookingCodeData(brandIndex), dc);
    dc << "BRANDED FARE FAMILY:";
    addBrandBkgVect<FareClassCode>(request.brandedFareFamily(brandIndex), dc);
    dc << "BRANDED FARE FAMILY EXCLUDE:";
    addBrandBkgVect<FareClassCode>(request.brandedFareFamilyExclude(brandIndex), dc);
    dc << "BRANDED FARE FAMILY DATA:";
    addBrandBkgMap<FareClassCode>(request.brandedFareFamilyData(brandIndex), dc);
    dc << "BRANDED FARE BASIS CODE:";
    addBrandBkgVect<FareClassCode>(request.brandedFareBasisCode(brandIndex), dc);
    dc << "BRANDED FARE BASIS CODE EXCLUDE:";
    addBrandBkgVect<FareClassCode>(request.brandedFareBasisCodeExclude(brandIndex), dc);
    dc << "BRANDED FARE BASIS CODE DATA:";
    addBrandBkgMap<FareClassCode>(request.brandedFareBasisCodeData(brandIndex), dc);
  }
}

void
Diag198Collector::addItinInfo(const AncRequest& request, const Itin* itin)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "ITINERARY:\n  INDEX: " << itin->getItinIndex() << "\n";
  for (const auto travelSeg : itin->travelSeg())
    addTravelSegInfo(travelSeg);
}

void
Diag198Collector::addAncillaryPriceModifiersToItinInfo(const Itin* itin)
{
  Diag198Collector& dc = *this;
  if (itin->getAncillaryPriceModifiers().empty())
    return;
  DiagnosticAutoIndent indent;
  dc << ++indent + "PRICE MODIFIERS:\n";
  for (const auto& modifiersGroup : itin->getAncillaryPriceModifiers())
    addAncillaryPriceModifiersGroup(modifiersGroup, indent);
}

void
Diag198Collector::addAncillaryPriceModifiersGroup(const Itin::AncillaryPriceModifiersMap::value_type modifiersGroup,
                                                  DiagnosticAutoIndent indent)
{
  Diag198Collector& dc = *this;
  dc << indent + "ANCILLARY IDENTIFIER: " + modifiersGroup.first.getIdentifier() + "\n";
  for (const auto& priceModifier : modifiersGroup.second)
    addAncillaryPriceModifier(priceModifier, indent);
}

void
Diag198Collector::addAncillaryPriceModifier(const AncillaryPriceModifier& modifier,
                                            DiagnosticAutoIndent indent)
{
  Diag198Collector& dc = *this;
  dc << indent + "PRICE MODIFICATION:\n";
  addAncillaryPriceModifierContent(modifier, indent);
}

std::string
priceModificationTypeToString(AncillaryPriceModifier::Type type)
{
  switch(type)
  {
    case(AncillaryPriceModifier::Type::DISCOUNT):
      return "DISCOUNT";
    case(AncillaryPriceModifier::Type::RISE):
      return "RISE";
    default:
      return std::to_string((int)type);
  }
}

void
Diag198Collector::addAncillaryPriceModifierContent(const AncillaryPriceModifier &modifier,
                                                   DiagnosticAutoIndent indent)
{
  Diag198Collector& dc = *this;

  if (modifier._identifier)
    dc << indent + "IDENTIFIER: " << *modifier._identifier + "\n";

  dc << indent + "QUANTITY: " << std::to_string(modifier._quantity) << "\n";

  if (modifier._type)
    dc << indent + "TYPE: " << priceModificationTypeToString(modifier._type.get()) << "\n";

  if (modifier._money)
    dc << indent + "MONEY: " << std::to_string(modifier._money.get().value()) << " " << modifier._money.get().code() << "\n";

  if (modifier._percentage)
    dc << indent + "PERCENTAGE: " << std::to_string(modifier._percentage.get()) << "\n";
}

void
Diag198Collector::addItinInfoAddOn(const AncRequest& request, const Itin* itin, const Itin* trxItin)
{
  DiagCollector& dc = (DiagCollector&)*this;
  if (request.fareBreakAssociationPerItin().find(itin) !=
      request.fareBreakAssociationPerItin().end())
  {
    std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator ift =
        request.fareBreakAssociationPerItin().find(itin)->second.begin();
    std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator ife =
        request.fareBreakAssociationPerItin().find(itin)->second.end();
    for (; ift != ife; ift++)
    {
      dc << "  FARE BREAK ASSOCIATION:\n";
      dc << "    SEGMENT ID: " << (*ift)->segmentID() << "\n";
      dc << "    FARE COMPONENT ID: " << (*ift)->fareComponentID() << "\n";
      dc << "    SIDE TRIP ID: " << (*ift)->sideTripID() << "\n";
      dc << "    SIDE TRIP START: " << std::string((*ift)->sideTripStart() ? "T" : "F") << "\n";
      dc << "    SIDE TRIP END: " << std::string((*ift)->sideTripEnd() ? "T" : "F") << "\n";
    }
  }
  if (request.fareBreakPerItin().find(itin) != request.fareBreakPerItin().end())
  {
    std::vector<AncRequest::AncFareBreakInfo*>::const_iterator ibt =
        request.fareBreakPerItin().find(itin)->second.begin();
    std::vector<AncRequest::AncFareBreakInfo*>::const_iterator ibe =
        request.fareBreakPerItin().find(itin)->second.end();
    for (; ibt != ibe; ibt++)
    {
      dc << "  FARE BREAK INFO:\n";
      dc << "    FARE COMPONENT ID: " << (*ibt)->fareComponentID() << "\n";
      dc << "    GOVERNING CARRIER: " << (*ibt)->governingCarrier() << "\n";
      dc << "    MONEY AMOUNT: " << (*ibt)->fareAmount() << "\n";
      dc << "    FARE BASIS: " << (*ibt)->fareBasis() << "\n";
      dc << "    FARE TYPE: " << (*ibt)->fareType() << "\n";
      dc << "    TARIFF NUMBER: " << (*ibt)->fareTariff() << "\n";
      dc << "    RULE NUMBER: " << (*ibt)->fareRule() << "\n";
      dc << "    FARE INDICATOR: " << (*ibt)->fareIndicator() << "\n";
      dc << "    PRIVATE INDICATOR: " << std::string((*ibt)->privateIndicator() ? "T" : "F")
         << "\n";
    }
  }
  if (request.ancillNonGuaranteePerItin().find(itin) != request.ancillNonGuaranteePerItin().end())
    dc << "  FEE NOT GUARANTEE: "
       << std::string(request.ancillNonGuaranteePerItin().find(itin)->second ? "T" : "F") << "\n";

  if (request.tourCodePerItin().find(itin) != request.tourCodePerItin().end())
    dc << "  TOUR CODE: " << request.tourCodePerItin().find(itin)->second << "\n";

  if (request.paxTypesPerItin().find(itin) != request.paxTypesPerItin().end())
  {
    std::vector<PaxType*> tmpVec;
    if (request.paxTypesPerItin().find(trxItin) != request.paxTypesPerItin().end())
      tmpVec.insert(tmpVec.end(),
                    request.paxTypesPerItin().find(trxItin)->second.begin(),
                    request.paxTypesPerItin().find(trxItin)->second.end());
    if (itin != trxItin)
      tmpVec.insert(tmpVec.end(),
                    request.paxTypesPerItin().find(itin)->second.begin(),
                    request.paxTypesPerItin().find(itin)->second.end());

    std::vector<PaxType*>::const_iterator ipt = tmpVec.begin();
    std::vector<PaxType*>::const_iterator ipe = tmpVec.end();
    dc << "  PASSENGERS:\n";
    for (; ipt != ipe; ipt++)
    {
      dc << "    PAX TYPE: " << (*ipt)->paxType() << "  NUMBER: " << (*ipt)->number();
      if ((*ipt)->paxType() != (*ipt)->requestedPaxType())
      {
        if ((*ipt)->age())
          dc << "  AGE: " << (*ipt)->age();
      }
      dc << "\n";
      std::vector<PaxType::FreqFlyerTierWithCarrier*>::const_iterator ifft =
          (*ipt)->freqFlyerTierWithCarrier().begin();
      std::vector<PaxType::FreqFlyerTierWithCarrier*>::const_iterator iffe =
          (*ipt)->freqFlyerTierWithCarrier().end();
      for (; ifft != iffe; ifft++)
        dc << "      FREQUENT FLYER STATUS: " << (*ifft)->freqFlyerTierLevel()
           << " CARRIER: " << (*ifft)->cxr() << "\n";

      if (!(*ipt)->psgTktInfo().empty())
      {
        std::vector<PaxType::TktInfo*>::const_iterator ipts = (*ipt)->psgTktInfo().begin();
        std::vector<PaxType::TktInfo*>::const_iterator ipte = (*ipt)->psgTktInfo().end();
        for (; ipts != ipte; ipts++)
          dc << "        NAME NUMBER: " << (*ipts)->psgNameNumber()
             << "  TICKET REF NUMBER: " << (*ipts)->tktRefNumber() << "\n"
             << "      TICKET NUMBER: " << (*ipts)->tktNumber() << "\n";
      }
      dc << "\n";
    }
  }

  if (request.accountCodeIdPerItin().find(itin) != request.accountCodeIdPerItin().end())
  {
    std::vector<std::string>::const_iterator iai =
        request.accountCodeIdPerItin().find(itin)->second.begin();
    std::vector<std::string>::const_iterator iae =
        request.accountCodeIdPerItin().find(itin)->second.end();
    dc << "  ACCOUNT CODES:";
    for (; iai != iae; iai++)
      dc << " " << *iai;
    dc << "\n";
  }

  if (request.corpIdPerItin().find(itin) != request.corpIdPerItin().end())
  {
    std::vector<std::string>::const_iterator iai =
        request.corpIdPerItin().find(itin)->second.begin();
    std::vector<std::string>::const_iterator iae = request.corpIdPerItin().find(itin)->second.end();
    dc << "  VALID CORP IDS:";
    for (; iai != iae; iai++)
      dc << " " << *iai;
    dc << "\n";
  }

  if (request.invalidCorpIdPerItin().find(itin) != request.invalidCorpIdPerItin().end())
  {
    std::vector<std::string>::const_iterator iai =
        request.invalidCorpIdPerItin().find(itin)->second.begin();
    std::vector<std::string>::const_iterator iae =
        request.invalidCorpIdPerItin().find(itin)->second.end();
    dc << "  INVALID CORP IDS:";
    for (; iai != iae; iai++)
      dc << " " << *iai;
    dc << "\n";
  }

  // Tkt Designator
  if (request.tktDesignatorPerItin().find(itin) != request.tktDesignatorPerItin().end())
  {
    std::map<int16_t, TktDesignator>::const_iterator itt =
        request.tktDesignatorPerItin().find(itin)->second.begin();
    std::map<int16_t, TktDesignator>::const_iterator ite =
        request.tktDesignatorPerItin().find(itin)->second.end();
    dc << "  TKT DESIGNATORS:\n";
    for (; itt != ite; itt++)
      dc << "    SEGMENT: " << itt->first << " TKT DESIGNATOR: " << itt->second << "\n";
  }
}

void
Diag198Collector::addItinInfo(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;

  const AncillaryPricingTrx* const ancTrx = dynamic_cast<const AncillaryPricingTrx*>(&trx);
  if (ancTrx)
    return;

  for (const auto travelSeg : trx.travelSeg())
    addTravelSegInfo(travelSeg);

  dc << "*************  START PASSENGER TYPES *************\n";
  for (std::size_t i = 0; i < trx.paxType().size(); ++i)
    addPassengerData(trx.paxType()[i], i+1, dc, trx);

  dc << "*************  END PASSENGER TYPES ***************\n";
}

void
Diag198Collector::addTravelSegInfo(const TravelSeg* itt)
{
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "  TRAVEL SEGMENT:\n";
  dc << "    SEGMENT ID: " << itt->segmentOrder() << "\n";
  dc << "    BRAND CODE: " << itt->getBrandCode() << "\n";
  dc << "    ORIGIN: " << itt->origAirport() << "\n";
  dc << "    DESTINATION: " << itt->destAirport() << "\n";
  dc << "    DEPARTURE DATE: " << itt->departureDT().dateToString(DDMMYYYY, "-") + " TIME: "
     << itt->departureDT().timeToString(HHMM_AMPM, ":") << "\n";
  dc << "    ARRIVAL DATE: " << itt->arrivalDT().dateToString(DDMMYYYY, "-") + " TIME: "
     << itt->arrivalDT().timeToString(HHMM_AMPM, ":") << "\n";
  if (itt->segmentType() == Air)
  {
    const AirSeg* aseg = static_cast<const AirSeg*>(itt);
    dc << "    MARKETING CARRIER: " << aseg->marketingCarrierCode() << "\n";
    dc << "    OPERATING CARRIER: " << aseg->operatingCarrierCode() << "\n";
    dc << "    FLIGHT NUMBER: " << aseg->marketingFlightNumber() << "\n";
  }
  dc << "    CLASS OF SERVICE: " << itt->getBookingCode() << "\n";
  dc << "    CABIN: " << itt->bookedCabin().getCabinIndicator() << "\n";
  dc << "    EQUIPMENT TYPE: " << itt->equipmentType() << "\n";
  dc << "    RESERVATION STATUS: " << itt->resStatus() << "\n";
  dc << "    TICKET COUPON NUMBER: " << itt->ticketCouponNumber() << "\n";
}

bool
Diag198Collector::isBrandingTrx() const
{
  return dynamic_cast<const BrandingTrx*>(_trx);
}

void
Diag198Collector::addFlexFareGroupsDataInfo(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  flexFares::GroupsData ffgData = trx.getRequest()->getFlexFaresGroupsData();

  if (0 == ffgData.getSize())
  {
    return;
  }

  dc << "FLEX FARE GROUPS:\n";
  dc << "TOTAL GROUPS: " << ffgData.getSize() << "\n";

  for (flexFares::GroupsData::iterator ffgItr = ffgData.begin(); ffgItr != ffgData.end(); ++ffgItr)
  {
    uint16_t groupId = ffgItr->first;
    dc << "GROUP ID: " << groupId << "\n";
    dc << "  PASSENGER TYPE: " << ffgData.getPaxTypeCode(groupId) << "\n";
    dc << "  CABIN TYPE    : " << ffgData.getRequestedCabin(groupId) << "\n";
    dc << "  PUBLIC FARE   : " << (ffgData.arePublicFaresRequired(groupId) ? "TRUE" : "FALSE")
       << "\n";
    dc << "  PRIVATE FARE  : " << (ffgData.arePrivateFaresRequired(groupId) ? "TRUE" : "FALSE")
       << "\n";
    if (!fallback::fallbackFlexFareGroupNewJumpCabinLogic(&trx))
    {
      dc << "  FORCE CORP FARES  : " << (ffgData.isFlexFareXCIndicatorON(groupId) ? "TRUE" : "FALSE")
         << "\n";
      dc << "  FORCE XO FARES  : " << (TypeConvert::pssCharToBool(ffgData.getFlexFareXOFares(groupId)) ? "TRUE" : "FALSE")
         << "\n";
      dc << "  JUMP CABIN LOGIC  : ";
      switch (ffgData.getFFGJumpCabinLogic(groupId))
      {
        case flexFares::JumpCabinLogic::ENABLED:
          dc << "ENABLED";
          break;
        case flexFares::JumpCabinLogic::ONLY_MIXED:
          dc << "ONLY_MIXED";
          break;
        default:
          dc << "DISABLED";
          break;
      }
      dc << "\n";
    }
    dc << "  NO PENALTY    : " << (ffgData.isNoPenaltiesRequired(groupId) ? "TRUE" : "FALSE")
       << "\n";
    dc << "  NO ADVANCED PURCHASE : "
       << (ffgData.isNoAdvancePurchaseRequired(groupId) ? "TRUE" : "FALSE") << "\n";
    dc << "  NO MIN/MAX  STAY     : "
       << (ffgData.isNoMinMaxStayRequired(groupId) ? "TRUE" : "FALSE") << "\n";
    dc << "  NO RESTRICTIONS      : "
       << ((ffgData.isNoPenaltiesRequired(groupId) &&
            ffgData.isNoAdvancePurchaseRequired(groupId) && ffgData.isNoMinMaxStayRequired(groupId))
               ? "TRUE"
               : "FALSE") << "\n";

    std::set<std::string> strSet = ffgData.getCorpIds(groupId);
    dc << "  CORP IDS      : ";
    if (0 == strSet.size())
    {
      dc << "NONE\n";
    }
    else
    {
      for (const auto& elem : strSet)
      {
        dc << elem << " ";
      }
      dc << "\n";
    }

    strSet = ffgData.getAccCodes(groupId);
    dc << "  ACCOUNT CODES : ";
    if (0 == strSet.size())
    {
      dc << "NONE\n";
    }
    else
    {
      for (const auto& elem : strSet)
      {
        dc << elem << " ";
      }
      dc << "\n";
    }
    if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx))
      addFFGMaxPenaltyData(ffgData.getFFGMaxPenaltyInfo(groupId));
  }

  dc << "\n";
}

void
Diag198Collector::addChargesTaxes(const PricingTrx& trx)
{
  if(fallback::fallbackAMChargesTaxes(&trx)|| !trx.getAmVatTaxRatesOnCharges())
  {
    return;
  }

  DiagCollector& dc = (DiagCollector&)*this;

  const std::map<NationCode, AmVatTaxRatesOnCharges::AmVatTaxRate>& amVatRates = trx.getAmVatTaxRatesOnCharges()->getData();
  if(amVatRates.empty())
  {
    dc << "********** AM CHARGES TAXES CONFIGURATION DATA NOT FOUND **********\n";
    return;
  }

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "********** AM CHARGES TAXES CONFIGURATION DATA **********\n";
  dc << "POS  TAX CODE  TAX RATE\n";

  for (auto const &amVatRate : amVatRates)
    dc << std::setw(5) << amVatRate.first << std::setw(10) << amVatRate.second.getTaxCode() << std::setw(2) << static_cast<int> (amVatRate.second.getTaxRate()) << "\n";
}

void
Diag198Collector::displayCabinInclusionCode(const FareDisplayRequest& request)
{
  std::string sub_string = request.requestedInclusionCode().substr(0, 2);
  if(request.inclusionNumber(sub_string) == 0)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "CABIN INCLUSION"; 
  if(request.requestedInclusionCode().size() < 4 )
  {
    dc << " CODE REQUESTED : " << sub_string;
  }
  else if(request.requestedInclusionCode().size() > 10 )
  {
    dc << " CODE REQUESTED : AB";
  }
  else
  {
    dc << " CODES REQUESTED : ";
    uint8_t sizeIncl = request.requestedInclusionCode().size()/2;
    for (uint8_t number = 0; number < sizeIncl;  ++number)
    {
      sub_string = request.requestedInclusionCode().substr(number*2, 2);
      dc << sub_string << " ";
    }
  }
  dc << std::endl;
  return;
}

void
Diag198Collector::addFFGMaxPenaltyData(MaxPenaltyInfo* maxPenaltyInfo)
{
  if (maxPenaltyInfo)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "MAX PENALTY                   MPO: ";
    switch (maxPenaltyInfo->_mode)
    {
    case smp::INFO:
      dc << 'I' << std::endl;
      break;
    case smp::OR:
      dc << 'O' << std::endl;
      break;
    case smp::AND:
      dc << 'A' << std::endl;
      break;
    }

    if (maxPenaltyInfo->_mode != smp::INFO)
    {
      if (maxPenaltyInfo->_changeFilter._departure != smp::BOTH)
      {
        dc << "CHANGE AFTER/BEFORE DEPARTURE ABD: "
           << smp::toDepartureAppl(maxPenaltyInfo->_changeFilter._departure) << std::endl;
      }

      if (maxPenaltyInfo->_changeFilter._maxFee)
      {
        const Money& maxChangeFee = maxPenaltyInfo->_changeFilter._maxFee.get();
        dc << "MAX CHANGE AMOUNT             MPA: " << maxChangeFee.value() << std::endl;
        dc << "MAX CHANGE CURRENCY           MPC: " << maxChangeFee.code() << std::endl;
      }
      else if (maxPenaltyInfo->_changeFilter._query)
      {
        dc << "ANY(CHANGEABLE)/NONCHANGEABLE MPI: "
           << (maxPenaltyInfo->_changeFilter._query.get() == smp::CHANGEABLE ? 'A' : 'N')
           << std::endl;
      }

      if (maxPenaltyInfo->_refundFilter._departure != smp::BOTH)
      {
        dc << "CHANGE AFTER/BEFORE DEPARTURE ABD: " << maxPenaltyInfo->_refundFilter._departure
           << std::endl;
      }

      if (maxPenaltyInfo->_refundFilter._maxFee)
      {
        const Money& maxRefundFee = maxPenaltyInfo->_refundFilter._maxFee.get();
        dc << "MAX REFUND AMOUNT             MPA: " << maxRefundFee.value() << std::endl;
        dc << "MAX REFUND CURRENCY           MPC: " << maxRefundFee.code() << std::endl;
      }
      else if (maxPenaltyInfo->_refundFilter._query)
      {
        dc << "ANY(REFUNDABLE)/NONREFUNDABLE MPI: "
           << (maxPenaltyInfo->_refundFilter._query.get() == smp::CHANGEABLE ? 'A' : 'N')
           << std::endl;
      }
    }
  }
}

} // tse
