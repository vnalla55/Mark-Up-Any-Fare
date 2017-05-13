//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.cpp
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "BrandedFares/S8BrandedFaresSelector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "Common/FallbackUtil.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag889Collector.h"
#include "Rules/RuleUtil.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/format.hpp>

namespace tse
{

bool
BrandProgramComparator::
operator()(const BrandProgram* brandProgram1, const BrandProgram* brandProgram2) const
{
  if (!brandProgram1 || !brandProgram2)
    return false;

  return brandProgram1->programID() < brandProgram2->programID();
}

const ServiceRuleTariffInd S8BrandedFaresSelector::RULE_TARIFF_IND_PUBLIC = "PUB";
const ServiceRuleTariffInd S8BrandedFaresSelector::RULE_TARIFF_IND_PRIVATE = "PRI";

S8BrandedFaresSelector::S8BrandedFaresSelector(PricingTrx& trx)
  : _trx(trx)
{
}

PaxTypeFare::BrandStatus
S8BrandedFaresSelector::validateFare(const PaxTypeFare* paxTypeFare,
                                     const BrandProgram* brandPr,
                                     const BrandInfo* brand,
                                     bool& needBrandSeparator,
                                     BrandedFareDiagnostics& diagnostics,
                                     bool skipHardPassValidation) const
{
  const std::vector<SvcFeesFareIdInfo*>& fareIDDataT189 = brand->fareIDdataPrimaryT189();
  if (fareIDDataT189.empty())
  {
    diagnostics.printT189NotFound(brand);
    return PaxTypeFare::BS_FAIL;
  }

  diagnostics.printPT189Size(fareIDDataT189.size());
  bool hasPrintedBrandT189Item = false;
  std::vector<SecSvcFeesFareIdInfo*> secondarySvcFeesFareIdInfoVec;

  for (SvcFeesFareIdInfo* svcFeesFareIdInfo : brand->fareIDdataPrimaryT189())
  {
    if(!diagnostics.isForSeqNumber(svcFeesFareIdInfo->seqNo()))
      continue;

    StatusT189 rc = validateT189Data(paxTypeFare,
                                     svcFeesFareIdInfo,
                                     brand->fareIDdataSecondaryT189(),
                                     secondarySvcFeesFareIdInfoVec,
                                     brandPr,
                                     skipHardPassValidation);

    diagnostics.displayT189Validation(rc, svcFeesFareIdInfo, brand, needBrandSeparator, hasPrintedBrandT189Item, secondarySvcFeesFareIdInfoVec);

    secondarySvcFeesFareIdInfoVec.clear();

    if (rc == APPL_NEGATIVE)
      return PaxTypeFare::BS_FAIL;

    if (rc == PASS_T189 || rc == SOFTPASS_RBD)
    {
      return (rc == SOFTPASS_RBD) ? PaxTypeFare::BS_SOFT_PASS : PaxTypeFare::BS_HARD_PASS;
    }
  }
  return PaxTypeFare::BS_FAIL;
}

StatusT189
S8BrandedFaresSelector::validateT189Data(
    const PaxTypeFare* paxTypeFare,
    SvcFeesFareIdInfo* svcFeesFareIdInfo,
    const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189,
    std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec,
    const BrandProgram* brandPr,
    bool skipHardPassValidation) const
{
  if (!matchOneWayRoundTrip(svcFeesFareIdInfo->owrt(), paxTypeFare->owrt()))
  {
    return FAIL_OWRT;
  }

  if ((!svcFeesFareIdInfo->ruleTariffInd().empty()) &&
      (!matchRuleTariffInd(svcFeesFareIdInfo->ruleTariffInd(), paxTypeFare->tcrTariffCat())))
  {
    return FAIL_RULE_TARIFF_IND;
  }

  if (!matchRuleTariff(svcFeesFareIdInfo->ruleTariff(), *paxTypeFare))
  {
    return FAIL_RULE_TARIFF;
  }

  if (!matchRule(svcFeesFareIdInfo->rule(), paxTypeFare->ruleNumber()))
  {
    return FAIL_RULE;
  }

  if ((!svcFeesFareIdInfo->fareClass().empty()) && (svcFeesFareIdInfo->fareClass() != BLANK_CODE) &&
      (!RuleUtil::matchFareClass(svcFeesFareIdInfo->fareClass().c_str(),
                                 paxTypeFare->fareClass().c_str())))
  {
    return FAIL_FARECLASS;
  }

  if ((!svcFeesFareIdInfo->fareType().empty()) && (svcFeesFareIdInfo->fareType() != BLANK_CODE) &&
      (paxTypeFare->fareClassAppInfo() != nullptr) &&
      (!RuleUtil::matchFareType(svcFeesFareIdInfo->fareType(), paxTypeFare->fcaFareType())))
  {
    return FAIL_FARETYPE;
  }

  if (paxTypeFare->actualPaxType() != nullptr &&
      !matchPassengerType(
          svcFeesFareIdInfo->paxType(), paxTypeFare->actualPaxType()->paxType(), brandPr))
  {
    return FAIL_PAXTYPE;
  }

  std::string formattedRouting = (boost::format("%|05|") % svcFeesFareIdInfo->routing()).str();

  if (!matchRouting(formattedRouting, *paxTypeFare))
  {
    return FAIL_ROUTING;
  }

  bool matchSecondaryT189 = false;
  StatusT189 bookingCodeStatus = validateBookingCode(paxTypeFare,
                                                     svcFeesFareIdInfo,
                                                     fareIDdataSecondaryT189,
                                                     matchSecondaryT189,
                                                     secondarySvcFeesFareIdInfoVec,
                                                     skipHardPassValidation);

  if (bookingCodeStatus == FAIL_PRIME_RBD || bookingCodeStatus == FAIL_SEC_T189)
  {
    return bookingCodeStatus;
  }

  if (!matchSource(svcFeesFareIdInfo->source(), paxTypeFare->vendor()))
  {
    return FAIL_SOURCE;
  }

  StatusT189 range = isMinAndMaxFare(svcFeesFareIdInfo, *paxTypeFare, true);
  if (range != PASS_RANGE)
  {
    if ((!svcFeesFareIdInfo->cur2().empty()) && (svcFeesFareIdInfo->cur2() != BLANK_CODE) &&
        (svcFeesFareIdInfo->cur2() != svcFeesFareIdInfo->cur1()))
    {
      range = isMinAndMaxFare(svcFeesFareIdInfo, *paxTypeFare, false);
      if (range != PASS_RANGE)
      {
        return range; // FAIL_RANGE2;
      }
    }
    else
    {
      return range; // FAIL_RANGE1;
    }
  }

  if (svcFeesFareIdInfo->fareApplInd() == 'N')
    return APPL_NEGATIVE;

  if (matchSecondaryT189)
    return SOFTPASS_RBD;
  else
    return PASS_T189;
}

bool
S8BrandedFaresSelector::matchOneWayRoundTrip(const Indicator& owrtFromRule,
                                             const Indicator& owrtFromFare) const
{
  return owrtFromRule == CHAR_BLANK || owrtFromRule == owrtFromFare;
}

bool
S8BrandedFaresSelector::matchPassengerType(const PaxTypeCode& passengerTypeFromRule,
                                           const PaxTypeCode& passengerTypeFromFare,
                                           const BrandProgram* brandPr) const
{
  if (passengerTypeFromRule.empty() || passengerTypeFromRule == BLANK_CODE)
  {
    if (brandPr->passengerType().empty())
      return true;

    for (const PaxTypeCode& paxTypeCode : brandPr->passengerType())
    {
      if (paxTypeCode.empty() || paxTypeCode == BLANK_CODE || paxTypeCode == passengerTypeFromFare)
        return true;
    }
  }
  else
    return passengerTypeFromRule == passengerTypeFromFare;

  return false;
}

bool
S8BrandedFaresSelector::matchRouting(const std::string& routingFromRule, const PaxTypeFare& ptf) const
{

  if (routingFromRule == "99999")
  {
    return true;
  }

  if (ptf.isRoutingProcessed() && !ptf.isRouting())
  {
    if (routingFromRule == "00000")
    {
      return true;
    }
  }
  else
  {
    std::string lastFourRoutingFromRule =
        routingFromRule.length() >= 4 ? routingFromRule.substr(routingFromRule.length() - 4, 4)
                                      : routingFromRule;
    if (lastFourRoutingFromRule == ptf.routingNumber().c_str())
    {
      return true;
    }
  }

  return false;
}

bool
S8BrandedFaresSelector::matchBookingCode(const BookingCode& bookingCode,
                                         const std::vector<BookingCode>& primeBookingCodeVec) const
{

  if (bookingCode.empty() || bookingCode == BLANK_CODE)
  {
    return true;
  }

  BookingCode table189BookingCode = bookingCode;
  getOneCharBookingCode(table189BookingCode);

  for (auto fareBookingCode : primeBookingCodeVec)
  {
    getOneCharBookingCode(fareBookingCode);

    if (table189BookingCode == fareBookingCode)
    {
      return true;
    }
  }

  return false;
}

void
S8BrandedFaresSelector::getOneCharBookingCode(BookingCode& bookingCode) const
{
  if (bookingCode.length() > 1)
  {
    std::string tmpBookingCode(1, bookingCode[0]);
    bookingCode = tmpBookingCode;
  }
}

bool
S8BrandedFaresSelector::matchSource(const Indicator& sourceFromRule, const VendorCode& vendor) const
{
  switch (sourceFromRule)
  {

  case 'A':
    return true;

  case 'D':
    if (!(vendor == Vendor::ATPCO || vendor == Vendor::SITA))
      return true;

  case 'S':
    if (vendor == Vendor::SITA)
      return true;

  case ' ':
    if (vendor == Vendor::ATPCO)
      return true;
  }

  return false;
}

bool
S8BrandedFaresSelector::matchRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd,
                                           const TariffCategory& tariffCategory) const
{
  return (ruleTariffInd == RULE_TARIFF_IND_PUBLIC && tariffCategory != RuleConst::PRIVATE_TARIFF) ||
         (ruleTariffInd == RULE_TARIFF_IND_PRIVATE && tariffCategory == RuleConst::PRIVATE_TARIFF);
}

bool
S8BrandedFaresSelector::matchRuleTariff(const TariffNumber& ruleTariff, const PaxTypeFare& ptf) const
{

  return ruleTariff == -1 || ruleTariff == ptf.tcrRuleTariff() ||
         (ptf.fareClassAppInfo() != nullptr && ruleTariff == ptf.fareClassAppInfo()->_ruleTariff);
}

bool
S8BrandedFaresSelector::matchRule(const RuleNumber& ruleFromRule, const RuleNumber& ruleFromFare) const
{
  return ruleFromRule.empty() || ruleFromRule == BLANK_CODE || ruleFromRule == ruleFromFare;
}

StatusT189
S8BrandedFaresSelector::isMinAndMaxFare(SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                        const PaxTypeFare& ptf,
                                        bool fareRange1) const
{
  MoneyAmount minFareRange;
  MoneyAmount maxFareRange;
  CurrencyCode currency;
  CurrencyNoDec noDec;

  if (fareRange1)
  {
    minFareRange = svcFeesFareIdInfo->minFareAmt1();
    maxFareRange = svcFeesFareIdInfo->maxFareAmt1();
    currency = svcFeesFareIdInfo->cur1();
    noDec = svcFeesFareIdInfo->noDec1();
  }
  else
  {
    minFareRange = svcFeesFareIdInfo->minFareAmt2();
    maxFareRange = svcFeesFareIdInfo->maxFareAmt2();
    currency = svcFeesFareIdInfo->cur2();
    noDec = svcFeesFareIdInfo->noDec2();
  }

  if (currency.empty() || currency == BLANK_CODE)
  {
    return PASS_RANGE;
  }

  MoneyAmount curFareAmt = ptf.fareAmount();

  if (ptf.currency() != currency)
  {
    return fareRange1 ? FAIL_RANGE1_CURR : FAIL_RANGE2_CURR;
  }

  if (ptf.numDecimal() != noDec)
  {
    return fareRange1 ? FAIL_RANGE1_DECIMAL : FAIL_RANGE2_DECIMAL;
  }

  if (minFareRange != 0 && curFareAmt < minFareRange)
  {
    return fareRange1 ? FAIL_RANGE1_MIN : FAIL_RANGE2_MIN;
  }

  if (maxFareRange != 0 && curFareAmt > maxFareRange)
  {
    return fareRange1 ? FAIL_RANGE1_MAX : FAIL_RANGE2_MAX;
  }

  return PASS_RANGE;
}

StatusT189
S8BrandedFaresSelector::validateBookingCode(
    const PaxTypeFare* paxTypeFare,
    const SvcFeesFareIdInfo* svcFeesFareIdInfo,
    const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189,
    bool& matchSecondaryT189,
    std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec,
    bool skipHardPassValidation) const
{

  std::vector<BookingCode> primeBookingCodeVec;
  paxTypeFare->getPrimeBookingCode(primeBookingCodeVec);

  if (primeBookingCodeVec.empty() && paxTypeFare->isFareByRule())
    getFBRBookingCode(paxTypeFare, primeBookingCodeVec);

  if (_trx.getTnShoppingBrandingMode() != TnShoppingBrandingMode::SINGLE_BRAND)
  {
    for (const auto& elem : paxTypeFare->s8BFBookingCode())
    {
      std::string tmpBookingCode(1, elem);
      primeBookingCodeVec.push_back(tmpBookingCode);
      break;
    }
  }

  if (!skipHardPassValidation)
  {
    if (!matchBookingCode(svcFeesFareIdInfo->bookingCode1(), primeBookingCodeVec))
    {
      if (_trx.isSoftPassDisabled())
      {
        return FAIL_PRIME_RBD;
      }
    }
    else
    {
      return NO_STATUS;
    }
  }

  if (svcFeesFareIdInfo->bookingCode2().empty())
  {
    if (fareIDdataSecondaryT189.empty())
      return FAIL_PRIME_RBD;
    else
    {
      matchSecondaryT189 = processT189Secondary(
          primeBookingCodeVec, fareIDdataSecondaryT189, secondarySvcFeesFareIdInfoVec);
      if (!matchSecondaryT189)
        return FAIL_SEC_T189;
      else
        return NO_STATUS;
    }
  }

  if (!matchBookingCode(svcFeesFareIdInfo->bookingCode2(), primeBookingCodeVec))
  {
    if (fareIDdataSecondaryT189.empty())
    {
      return FAIL_PRIME_RBD;
    }
    else
    {
      matchSecondaryT189 = processT189Secondary(
          primeBookingCodeVec, fareIDdataSecondaryT189, secondarySvcFeesFareIdInfoVec);
      if (!matchSecondaryT189)
      {
        return FAIL_SEC_T189;
      }
    }
  }

  return NO_STATUS;
}

void
S8BrandedFaresSelector::getFBRBookingCode(const PaxTypeFare* paxTypeFare,
                                          std::vector<BookingCode>& primeBookingCodeVec) const
{
  uint16_t cat19 = 19;
  const PaxTypeFareRuleData* fbrPTFare =
      paxTypeFare->paxTypeFareRuleData(RuleConst::FARE_BY_RULE); // Cat 25 pointer

  const PaxTypeFareRuleData* diskPTfare = paxTypeFare->paxTypeFareRuleData(cat19); // Cat 19 pointer

  if (paxTypeFare->isDiscounted() && diskPTfare && diskPTfare->ruleItemInfo())
  {

    const DiscountInfo* discountInfo =
        dynamic_cast<const DiscountInfo*>(diskPTfare->ruleItemInfo());

    if (discountInfo && !discountInfo->bookingCode().empty())
      primeBookingCodeVec.push_back(discountInfo->bookingCode());
  }

  if (primeBookingCodeVec.empty() && fbrPTFare)
  {
    const FBRPaxTypeFareRuleData* fbrPTFBaseFare = PTFRuleData::toFBRPaxTypeFare(fbrPTFare);
    if (fbrPTFBaseFare && !fbrPTFBaseFare->isSpecifiedFare())
      fbrPTFBaseFare->getBaseFarePrimeBookingCode(
          primeBookingCodeVec); // gets Bkgs Codes from Base Fare
  }
}

bool
S8BrandedFaresSelector::processT189Secondary(
    const std::vector<BookingCode>& primeBookingCodeVec,
    const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189,
    std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec) const

{
  bool match = false;
  for (SvcFeesFareIdInfo* svcFeesFareIdInfo : fareIDdataSecondaryT189)
  {
    SecSvcFeesFareIdInfo* secSvcFeesFareIdInfo = nullptr;
    _trx.dataHandle().get(secSvcFeesFareIdInfo);
    secSvcFeesFareIdInfo->_svcFeesFareIdInfo = svcFeesFareIdInfo;
    if (matchBookingCode(svcFeesFareIdInfo->bookingCode1(), primeBookingCodeVec))
    {
      match = true;
      secSvcFeesFareIdInfo->_status = match;
      secondarySvcFeesFareIdInfoVec.push_back(secSvcFeesFareIdInfo);
      break;
    }
    else if (!svcFeesFareIdInfo->bookingCode2().empty() &&
             matchBookingCode(svcFeesFareIdInfo->bookingCode2(), primeBookingCodeVec))
    {
      match = true;
      secSvcFeesFareIdInfo->_status = match;
      secondarySvcFeesFareIdInfoVec.push_back(secSvcFeesFareIdInfo);
      break;
    }

    secSvcFeesFareIdInfo->_status = match;
    secondarySvcFeesFareIdInfoVec.push_back(secSvcFeesFareIdInfo);
  }
  return match;
}

} // namespace
