//----------------------------------------------------------------------------
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

#include "Pricing/Combinations.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/StructuredRuleData.h"
#include "DBAccess/CircleTripRuleItem.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/FareClassRestRule.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/TravelRestriction.h"
#include "DBAccess/OpenJawRestriction.h"
#include "DBAccess/OpenJawRule.h"
#include "DBAccess/RoundTripRuleItem.h"
#include "DBAccess/TariffRuleRest.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag625Collector.h"
#include "Diagnostic/DiagCollectorGuard.h"
#include "Pricing/CombinationsSubCat106.h"
#include "Pricing/CombinationsSubCat109.h"
#include "Pricing/PreserveRestoreRuleBasedFares.h"
#include "Pricing/PricingUtil.h"
#include "Rules/Eligibility.h"
#include "Rules/FlightApplication.h"
#include "Rules/MaximumStayApplication.h"
#include "Rules/MinimumStayApplication.h"
#include "Rules/PricingUnitDataAccess.h"
#include "Rules/RuleUtil.h"
#include "Rules/SalesRestrictionRule.h"
#include "Rules/TravelRestrictions.h"
#include "Rules/UpdaterObserver.h"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <string>
#include <vector>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackMipOptEOERules)
FALLBACK_DECL(fixAPO37202)
FALLBACK_DECL(structuredFareRulesMaxStayObserver)

static Logger
logger("atseintl.Pricing.Combinations");

static Logger
dataStringLogger("atseintl.Pricing.Combinations.DataString");

bool Combinations::_enablePULevelFailedFareUsageOptimization = true;
bool Combinations::_enableFPLevelFailedFareUsageOptimization = true;

const char Combinations::PASSCOMB;
const char Combinations::FAILCOMB;
const char Combinations::ABORT;
const char Combinations::IDLE;
const char Combinations::STOPCOMB;
//----------ATPCo Record 3 data ------------
const std::string Combinations::NO_RESTRICTION_S = "";
const std::string Combinations::NO_STOP_PERMITTED = "00";
const CarrierCode Combinations::MATCH_ANY_CARRIER = "   ";

//--- ATPCO Record 2 Combinations definition ---
const CarrierCode Combinations::JOINTCARRIER = "*J";

const char Combinations::ValidationElement::NOT_SET;
const char Combinations::ValidationElement::MATCHED;
const char Combinations::ValidationElement::NOT_MATCHED;

Combinations::FareTypeAvailabilty::FareTypeAvailabilty()
{
  reset();
}

void
Combinations::FareTypeAvailabilty::reset()
{
  _fareTypePresent.reset();
}

void
Combinations::FareTypeAvailabilty::setNormalFarePresent()
{
  _fareTypePresent[FT_NORMAL] = true;
}

void
Combinations::FareTypeAvailabilty::setSpecialFarePresent()
{
  _fareTypePresent[FT_SPECIAL] = true;
}

void
Combinations::FareTypeAvailabilty::setDomesticFarePresent()
{
  _fareTypePresent[FT_DOMESTIC] = true;
}

void
Combinations::FareTypeAvailabilty::setInternationalFarePresent()
{
  _fareTypePresent[FT_INTERNATIONAL] = true;
}

void
Combinations::FareTypeAvailabilty::setTransborderFarePresent()
{
  _fareTypePresent[FT_TRANSBORDER] = true;
}

bool
Combinations::FareTypeAvailabilty::isRequiredFareMissing(const EndOnEnd& endOnEndRule) const
{
  if (endOnEndRule.eoeNormalInd() == RESTRICTIONS && !_fareTypePresent[FT_NORMAL])
    return true;
  if (endOnEndRule.eoespecialInd() == RESTRICTIONS && !_fareTypePresent[FT_SPECIAL])
    return true;
  if (endOnEndRule.domInd() == RESTRICTIONS && !_fareTypePresent[FT_DOMESTIC])
    return true;
  if (endOnEndRule.intlInd() == RESTRICTIONS && !_fareTypePresent[FT_INTERNATIONAL])
    return true;
  if (endOnEndRule.uscatransborderInd() == RESTRICTIONS && !_fareTypePresent[FT_TRANSBORDER])
    return true;
  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool Combinations::ValidationFareComponents::evaluateMajor
//
// Description:  evaluate each Minor Subcategory per fare component
//
// @param  negative - any negative Minor Subcategory
//
// @return true if it pass
//
// </PRE>
// ----------------------------------------------------------------------------

bool
Combinations::ValidationFareComponents::evaluateMajor(FareUsage*& failedSourceFareUsage,
                                                      FareUsage*& failedTargetFareUsage)
{
  if (_forcePass)
    return true;

  bool result = true;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    if (!v._passMajor)
    {
      if (!this->_needAllPassSameMajorItem && !v._passMinor)
      {
        result = false;
        continue; // can wait for next major item
      }

      v.setPassMajor();

      if (!v._passMajor)
      {
        result = false;
        if ((v.getSubCat(M101) == FAILCOMB || v.getSubCat(M101) == ABORT ||
             v.getSubCat(M101) == MAJOR_NO_MATCH) ||
            (v.getSubCat(M103) == FAILCOMB || v.getSubCat(M103) == ABORT ||
             v.getSubCat(M103) == MAJOR_NO_MATCH) ||
            (v.getSubCat(M104) == FAILCOMB || v.getSubCat(M104) == ABORT))
        {
          failedTargetFareUsage = v._targetFareUsage;
          failedSourceFareUsage = v._currentFareUsage;
        }
      }
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool Combinations::ValidationFareComponents::evaluateMajorByPassedMinor
//
// Description:  evaluate each Minor Subcategory per fare component
//
// @return true if it pass
//
// </PRE>
// ----------------------------------------------------------------------------

bool
Combinations::ValidationFareComponents::evaluateMajorByPassedMinor_old()
{
  if (UNLIKELY(_forcePass))
    return true;

  bool result = true;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    if (LIKELY(!v._passMajor))
    {
      v._passMajor = v._passMinor;
      if (!v._passMajor)
      {
        result = false;
        continue; // can wait for next major item
      }
    }
  }
  return result;
}

bool
Combinations::ValidationFareComponents::evaluateMajorByPassedMinor()
{
  if (_forcePass)
    return true;

  bool result = true;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    v.setPassMajor();

    if (!v._passMajor)
    {
      v._passMajor = v._passMinor;
      if (!v._passMajor)
      {
        result = false;
        continue; // can wait for next major item
      }
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool Combinations::ValidationFareComponents::evaluateMinor
//
// Description:  evaluate each Minor Subcategory per fare component
//
// @param  negative - any negative Minor Subcategory
//
// @return true if it pass
//
// </PRE>
// ----------------------------------------------------------------------------

bool
Combinations::ValidationFareComponents::evaluateMinor()
{
  if (_forcePass)
    return true;

  if (!_needAllPassSameMajorItem)
  {
    _anyPassMinor = false;

    for (size_t i = 0; i < size(); ++i)
    {
      ValidationElement& v = (*this)[i];
      if (v._passMajor)
        continue;

      v.setPassMinor();

      if (v._passMinor)
      {
        _anyPassMinor = true;
      }
    }

    return _anyPassMinor;
  }

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    v.setPassMinor();

    if (UNLIKELY(!v._passMinor))
    {
      return false;
    }
  }

  return true;
}

bool
Combinations::ValidationFareComponents::evaluateMinorNegAppl(FareUsage*& failedFareUsage,
                                                             FareUsage*& failedEOETargetFareUsage)
{
  if (UNLIKELY(_forcePass))
    return true;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];
    v.setPassMinor();

    if (v._passMinor)
    {
      failedFareUsage = v._currentFareUsage;
      failedEOETargetFareUsage = v._targetFareUsage;
      return true;
    }
  }

  return false;
}

void
Combinations::ValidationFareComponents::getFailedFUInMinor(FareUsage*& failedSourceFareUsage,
                                                           FareUsage*& failedTargetFareUsage)
{
  if (UNLIKELY(_forcePass))
    return;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];
    v.setPassMinor();

    if (!v._passMinor && !v._passMajor)
    {
      failedSourceFareUsage = v._currentFareUsage;
      failedTargetFareUsage = v._targetFareUsage;
      return;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool Combinations::ValidationFareComponents::evaluate
//
// Description:  evaluate each fare component per chosen subcategory
//
// @return true if it pass
//
// </PRE>
// ----------------------------------------------------------------------------

bool
Combinations::ValidationFareComponents::evaluate(const MatchNumbers num)
{
  if (_forcePass)
    return true;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    if (v.getSubCat(num) == NO_MATCH)
    {
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Combinations::ValidationFareComponents::::reset
//
// Description:  reset Minor Subcategory indicator/Major pass indicator
//
// </PRE>
// ----------------------------------------------------------------------------
void
Combinations::ValidationFareComponents::reset()
{
  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    v.reset();
  }
  _anyPassMinor = false;
  _validatingCarrier.clear();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Combinations::ValidationFareComponents::::resetMinor
//
// Description:  reset Minor Subcategory indicator
//
// </PRE>
// ----------------------------------------------------------------------------
void
Combinations::ValidationFareComponents::resetMinor()
{
  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    v.resetAll();
  }
  _anyPassMinor = false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Combinations::ValidationFareComponents::::resetMajor
//
// Description:  reset Major Subcategory indicator
//
// </PRE>
// ----------------------------------------------------------------------------
void
Combinations::ValidationFareComponents::resetMajor()
{
  _anyPassMinor = false;

  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];

    if (!this->_needAllPassSameMajorItem)
    {
      if (!v._passMajor)
        v.reset();
      continue;
    }

    v.getSubCat(M104) = ValidationElement::NOT_SET;
  }
}

bool
Combinations::ValidationFareComponents::validateCarrierPreference(DataHandle& dataHandle,
                                                                  const FareUsage* sourceFareUsage,
                                                                  const PaxTypeFare* targetFare)
{
  bool ret = false;

  if (!_combPref)
  {
    const DateTime travelDate = (*sourceFareUsage->travelSeg().begin())->departureDT();

    const CarrierPreference* carrierPref =
        dataHandle.getCarrierPreference(sourceFareUsage->paxTypeFare()->carrier(), travelDate);
    if (carrierPref)
    {
      _combPref = &(carrierPref->combPrefs());
    }
  }

  // IF there is a table to check against
  if (_combPref)
  {
    VendorCode sourceVendor;
    VendorCode targetVendor;
    Indicator sourceIndicator;
    Indicator targetIndicator;

    // Normalize the values to be checked
    if (sourceFareUsage->paxTypeFare()->vendor() <= targetFare->vendor())
    {
      sourceVendor = sourceFareUsage->paxTypeFare()->vendor();
      targetVendor = targetFare->vendor();
      sourceIndicator =
          (sourceFareUsage->paxTypeFare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ? 'V' : 'P';
      targetIndicator = (targetFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ? 'V' : 'P';
    }
    else
    {
      sourceVendor = targetFare->vendor();
      targetVendor = sourceFareUsage->paxTypeFare()->vendor();
      targetIndicator =
          (sourceFareUsage->paxTypeFare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ? 'V' : 'P';
      sourceIndicator = (targetFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ? 'V' : 'P';
    }

    std::vector<CarrierPreference::CombPref>::const_iterator cpIt = _combPref->begin();

    // FOR EACH entry in the table
    for (; cpIt != _combPref->end(); ++cpIt)
    {
      // IF it is for the correct vendors
      if (cpIt->vendor1() == sourceVendor && cpIt->vendor2() == targetVendor)
      {
        // Check each entry starting at this one
        for (; cpIt != _combPref->end(); ++cpIt)
        {
          // IF the current entry is for this vendor pair
          if (cpIt->vendor1() == sourceVendor && cpIt->vendor2() == targetVendor)
          {
            // Check the public/private indicators to see if it is the correct entry
            if (cpIt->publicPrivate1() == sourceIndicator &&
                cpIt->publicPrivate2() == targetIndicator)
            {
              ret = true;
              break;
            }
          }
          else // (the current entry is for a different vendor pair)

          {
            // there are no more candidates so we should fail
            break;
          }
        }
        break;
      }
    }
  }

  return ret;
}

void
Combinations::ValidationFareComponents::setMinorPass()
{
  for (size_t i = 0; i < size(); ++i)
  {
    ValidationElement& v = (*this)[i];
    if (!v._passMajor)
      v._passMinor = true;
  }
}

bool
alignFailedFareUsage(const std::vector<FareUsage*>& fareUsages,
                     FareUsage*& failedSourceFareUsage,
                     FareUsage*& failedTargetFareUsage)
{
  std::vector<FareUsage*>::const_iterator fuIt = fareUsages.begin();
  std::vector<FareUsage*>::const_iterator fuItEnd = fareUsages.end();
  for (; fuIt != fuItEnd; ++fuIt)
  {
    if ((*fuIt) == failedSourceFareUsage)
    {
      return true;
    }
    else if ((*fuIt) == failedTargetFareUsage)
    {
      std::swap(failedSourceFareUsage, failedTargetFareUsage);
      return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool Combinations::process
//
// Description:  Combinability is ATPCo automated rule category 10
//               contains the restrictions of each fare has their own
//               Combinability rule to combine with the other fares.
//
//               Unlike the other categories in the Automated Rules System,
//               Combinability consists of subordinate category records.
//               Although the provisions for Combinability are sought by
//               interrogating Category Control records (Record 2's) identified
//               by a category number 010, there is no physical Category 10 record.
//               The data string in the Record 2 will refer to a series of pointers
//               to Sub-Categories Number 101-109 that will collectively define
//               the provisions to be applied.
//
//               The following categories are used to state the provisions under
//               which combinations in each case are permitted.
//                 Open Jaws - 101
//                 2 Component Circle Trips - 102
//                 More Than 2 Component Circle Trips - 103
//                 End on End - 104
//                 International Add-Ons - 105
//
//               The other Sub-Categories (Carrier Combinations - 106,
//               Tariff/Rule Combinations - 107, Fare Class/Type Combinations - 108,
//               and Open Jaw Sets - 109) are always used in IF statements to further
//               qualify the conditions under which any of Categories 101-105 apply.
//               It is possible that the only restriction that applies to a combination
//               is stated in a contingency category (106-109).
//               In that instance, the data table number of the major sub-category
//               (101-105) being qualified would contain zeroes.
//
//               The Category Control Record (Record 2) for Combinability differs
//               from the other Category Control records.  Preceding the variable
//               portion of the record (the data table string) is a series of tags
//               indicating whether combinations are permitted, not permitted or
//               permitted with restrictions for each combination type.
//               Additional informational is recorded in this portion of the record
//               indicating the presence of Carrier, Tariff/Rule and/or
//               Fare Class/Type restrictions in the data table string.
//
// @return bool      where:    true              // passed Combinability
//                             false             // failed Combinability
//         farePath  contains validated Pricing Unit
//                   Pricing Unit contains validated Fare Usage
//                   Fare Usage has reference to Fare
//                   Additonal info passed to Taxes and Fare Calculation Line
// </PRE>
// ----------------------------------------------------------------------------
CombinabilityValidationResult
Combinations::process(PricingUnit& pu,
                      FareUsage*& failedSourceFareUsage,
                      FareUsage*& failedTargetFareUsage,
                      DiagCollector& diag,
                      Itin* itin)
{
  LOG4CXX_INFO(logger, "In Combinations: validate PU");

  if (UNLIKELY(_trx->getOptions()->isRtw()))
    return CVR_PASSED;

  // this instrumentation is called so many times it slows the server
  // down, so it's currently disabled
  // TSELatencyData metrics(*_trx, "PO COMBINATIONS PROCESS");

  PreserveRestorePURuleBasedFares ruleBasedFarePreserver;
  // NOTE:  The Rulebased fares preserved in this call are restores in the destructor
  ruleBasedFarePreserver.Preserve(&pu, &diag);

  // If there is a scoreboard -> use it as a filter
  // Else continue processing without filtering

  failedSourceFareUsage = nullptr;
  failedTargetFareUsage = nullptr;
  CombinabilityValidationResult ret =
      (_comboScoreboard) ? _comboScoreboard->validate(pu, failedSourceFareUsage, diag) : CVR_PASSED;

  diag.enable(&pu, Diagnostic605);

  const PricingUnit::Type puType = pu.puType();

  if (puType == PricingUnit::Type::ROUNDTRIP && _comboScoreboard && _comboScoreboard->isMirrorImage(pu))
  {
    if (ret == CVR_PASSED)
    {
      LOG4CXX_INFO(logger, " PASSED COMBINATION - MIRROR IMAGE RT");
      if (UNLIKELY(diag.isActive()))
        diag << " PASSED PU-LEVEL COMBINATION - MIRROR IMAGE RT" << std::endl;
    }
    else
    {
      LOG4CXX_INFO(logger, " FAILED COMBINATION - MIRROR IMAGE RT");
      if (diag.isActive())
        diag << " FAILED PU-LEVEL COMBINATION - MIRROR IMAGE RT" << std::endl;
    }
  }
  else if (ret != CVR_PASSED)
  {
    if (UNLIKELY(diag.isActive()))
      diag << " FAILED PU-LEVEL COMBINATION - SCOREBOARD CHECK" << std::endl;
    LOG4CXX_INFO(logger, " FAILED PU-LEVEL COMBINATION - SCOREBOARD CHECK");
  }
  else if (pu.fareUsage().size() == 1)
  {
    if (pu.fareUsage().front()->rec2Cat10() == nullptr)
    {
      if (diag.isActive())
        diag << " PASSED PU-LEVEL COMBINATION - SYSTEM ASSUMPTION" << std::endl;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
        diag << " PASSED PU-LEVEL COMBINATION" << std::endl;
    }

    LOG4CXX_INFO(logger, " PASSED COMBINATION ");
  }
  else
  {
    if (puType == PricingUnit::Type::OPENJAW)
    {
      diag.enable(Diagnostic631, Diagnostic636, Diagnostic637, Diagnostic638, Diagnostic639);
    }
    else if (puType == PricingUnit::Type::ROUNDTRIP)
    {
      diag.enable(Diagnostic632, Diagnostic636, Diagnostic637, Diagnostic638);
    }
    else if (LIKELY(puType == PricingUnit::Type::CIRCLETRIP))
      diag.enable(Diagnostic633, Diagnostic636, Diagnostic637, Diagnostic638);
    else if (puType == PricingUnit::Type::ONEWAY)
      diag.enable(Diagnostic634, Diagnostic636, Diagnostic637, Diagnostic638);
    else
      diag.disable(Diagnostic631, Diagnostic632, Diagnostic633, Diagnostic634);

    if (UNLIKELY(diag.isActive()))
    {
      diag.printLine();
      diag << pu;
    }

    DirectionalityInfo directionalityInfo(
        PricingUnitLevel, "PU", FROM_LOC1_TO_LOC2, TO_LOC1_FROM_LOC2);

    ValidationFareComponents validationFareComponent;

    ret = validateCombination(diag,
                              directionalityInfo,
                              &validationFareComponent,
                              pu,
                              nullptr,
                              failedSourceFareUsage,
                              failedTargetFareUsage,
                              itin)
              ? CVR_PASSED
              : CVR_UNSPECIFIED_FAILURE;

    diag.enable(&pu, Diagnostic605);
    if (ret == CVR_PASSED)
    {
      if (UNLIKELY(diag.isActive()))
        diag << " PASSED PU-LEVEL COMBINATION - DATA STRING VALIDATION " << getSubCat(puType)
             << std::endl;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
        diag << " FAILED PU-LEVEL COMBINATION - DATA STRING VALIDATION " << getSubCat(puType)
             << std::endl;
      // Ensure the "Failed Fare" pair is in the right order
      if (failedTargetFareUsage)
      {
        if ((pu.fareUsage().size() <= 2) || (!_enablePULevelFailedFareUsageOptimization))
        {
          failedSourceFareUsage = nullptr;
          failedTargetFareUsage = nullptr;
        }
        else
        {
          alignFailedFareUsage(pu.fareUsage(), failedSourceFareUsage, failedTargetFareUsage);
        }
      }
    }
  }

  return ret;
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
Combinations::process(FarePath& farePath,
                      uint32_t farePathIndex,
                      FareUsage*& failedSourceFareUsage,
                      FareUsage*& failedEOETargetFareUsage,
                      DiagCollector& diag)
{
  LOG4CXX_INFO(logger, "In Combinations: validate Fare Path");

  if (UNLIKELY(_trx->getOptions()->isRtw()))
    return CVR_PASSED;

  // this instrumentation is called so many times it slows the server
  // down, so it's currently disabled
  TSELatencyData metrics(*_trx, "PO COMBINATIONS PROCESS");

  failedSourceFareUsage = nullptr;
  failedEOETargetFareUsage = nullptr;

  PreserveRestoreFPRuleBasedFares fpRuleBasedFarePreserver;
  // NOTE:  The Rulebased fares preserved in this call are restores in the destructor
  fpRuleBasedFarePreserver.Preserve(farePath, &diag);

  CombinabilityValidationResult ret = CVR_PASSED;

  diag.enable(Diagnostic634,
              Diagnostic636,
              Diagnostic637,
              Diagnostic638,
              Diagnostic639,
              Diagnostic640,
              Diagnostic654,
              Diagnostic614);
  diag.enableFilter(Diagnostic614, 1, farePathIndex);
  if (UNLIKELY(diag.isActive()))
  {
    diag.printLine();
    diag << farePath;
  }

  if (_comboScoreboard && (ret = _comboScoreboard->analyzeOneWay(farePath, diag)) != CVR_PASSED)
  {
    LOG4CXX_INFO(logger, " FAILED FARE_PATH-LEVEL ONEWAY SCOREBOARD CHECK");
  }
  else
  {
    std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();
    Combinations::EndOnEndList eoeL;

    //---------- Build EndOnEndList  ------------
    DirectionalityInfo directionalityInfo(
        FarePathLevel, "FarePath", ORIGIN_FROM_LOC1_TO_LOC2, ORIGIN_FROM_LOC2_TO_LOC1);
    directionalityInfo.farePathNumber = farePathIndex; // Used for diagnostics

    {
      DiagCollectorGuard dcg1(diag, Diagnostic614);

      if (UNLIKELY(diag.isActive()))
      {
        const std::map<std::string, std::string>& dgParamMap = _trx->diagnostic().diagParamMap();
        std::map<std::string, std::string>::const_iterator it = dgParamMap.find("FP");

        if (it != dgParamMap.end())
          directionalityInfo.diagFarePathNumber = atoi(it->second.c_str());

        it = dgParamMap.find("PU");
        if (it != dgParamMap.end())
          directionalityInfo.diagPricingUnitNumber = atoi(it->second.c_str());

        it = dgParamMap.find("FN");
        if (it != dgParamMap.end())
          directionalityInfo.diagFareUsageNumber = atoi(it->second.c_str());
      }
    }

    ValidationFareComponents validationFareComponent;

    //---------- Data String Validation  ------------
    puIt = farePath.pricingUnit().begin();
    for (directionalityInfo.pricingUnitNumber = 1; puIt != puItEnd;
         ++puIt, ++directionalityInfo.pricingUnitNumber)
    {
      bool retDSV = false;
      retDSV = validateCombination(diag,
                                   directionalityInfo,
                                   &validationFareComponent,
                                   *(*puIt),
                                   &farePath,
                                   failedSourceFareUsage,
                                   failedEOETargetFareUsage,
                                   farePath.itin());

      if (!retDSV)
      {
        LOG4CXX_INFO(logger, " FAILED EOE COMBINATION - DATA STRING VALIDATION");
        ret = CVR_UNSPECIFIED_FAILURE;
        if (failedEOETargetFareUsage)
        {
          std::vector<PricingUnit*>::const_iterator puCheckIt = farePath.pricingUnit().begin();

          for (; puCheckIt != puIt; ++puCheckIt)
          {
            const PricingUnit& pu = *(*puCheckIt);

            if (alignFailedFareUsage(
                    pu.fareUsage(), failedSourceFareUsage, failedEOETargetFareUsage))
            {
              break;
            }
          }
        }
        break;
      }
    }
  }

  if (UNLIKELY(!_enableFPLevelFailedFareUsageOptimization))
  {
    failedSourceFareUsage = nullptr;
    failedEOETargetFareUsage = nullptr;
  }

  return ret;
}

std::pair<bool, CombinabilityRuleItemInfo>
Combinations::findCategoryRuleItem(
    const std::vector<CombinabilityRuleItemInfoSet*>& catRuleInfoSetVec)
{
  for (const auto& setPtr : catRuleInfoSetVec)
  {
    for (const auto& seg : *setPtr)
    {
      if (seg.itemcat() == CARRIER)
        return std::make_pair(true, seg);
    }
  }
  return std::make_pair(false, CombinabilityRuleItemInfo());
}

bool
Combinations::validate106Only(PricingUnit& pu, DiagCollector& diag)
{
  DirectionalityInfo directionalityInfo(
      PricingUnitLevel, "PU", FROM_LOC1_TO_LOC2, TO_LOC1_FROM_LOC2);
  for (FareUsage* fu : pu.fareUsage())
  {
    const CombinabilityRuleInfo* pCat10(fu->rec2Cat10());

    if (!pCat10)
      return false;

    std::pair<bool, CombinabilityRuleItemInfo> fcri =
        findCategoryRuleItem(pCat10->categoryRuleItemInfoSet());

    if (!fcri.first)
      return false;

    ValidationFareComponents validationFareComponent;

    bool negative;

    CombinationsSubCat106 subCat106(*_trx,
                                    diag,
                                    pCat10->vendorCode(),
                                    fcri.second.itemNo(),
                                    pu,
                                    *fu,
                                    validationFareComponent,
                                    negative);

    if (!subCat106.match() || !validationFareComponent.evaluate(m106))
      return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
bool
Combinations::validateCombination(DiagCollector& diag,
                                  DirectionalityInfo& directionalityInfo,
                                  ValidationFareComponents* validationFareComponent,
                                  PricingUnit& curPu,
                                  FarePath* farePath,
                                  FareUsage*& failedFareUsage,
                                  FareUsage*& failedEOETargetFareUsage,
                                  Itin* itin)
{
  bool ret = true;
  uint16_t subCat;

  // get subCat
  if (directionalityInfo.validationLevel == FarePathLevel)
    subCat = END_ON_END;
  else
    subCat = getSubCat(curPu.puType());

  if (UNLIKELY(diag.isActive()))
    diag << " PU MAJOR CAT:" << subCat << "\n";

  std::vector<FareUsage*>::const_iterator fuIt = curPu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuItEnd = curPu.fareUsage().end();
  for (directionalityInfo.fareUsageNumber = 1; fuIt != fuItEnd;
       ++fuIt, ++directionalityInfo.fareUsageNumber)
  {
    if (UNLIKELY(_trx->getTrxType() == PricingTrx::MIP_TRX &&
                  (*fuIt)->paxTypeFare()->isDummyFare()))
      continue;

    FareUsage& fareUsage = **fuIt;
    CombinabilityRuleInfo* pCat10 = (*fuIt)->rec2Cat10();
    if (UNLIKELY(!pCat10))
    {
      LOG4CXX_INFO(logger, __FILE__ << ":" << __LINE__ << " : NO REC 2 CAT 10");
      diag << " NO REC2 CAT10 FOR " << fareUsage.paxTypeFare()->fareClass() << "\n";
      ret = false;
      break;
    }

    std::vector<CarrierCode> valCxrList;
    bool multiVCProcess = getMultipleValidatingCarriers(curPu, farePath, valCxrList);
    if (multiVCProcess)
      multiVCProcess = checkQualifyingCat15(pCat10);

    if (LIKELY(!multiVCProcess))
    {
      ret = dataStringValidation(diag,
                                 directionalityInfo,
                                 validationFareComponent,
                                 subCat,
                                 fareUsage,
                                 pCat10,
                                 curPu,
                                 farePath,
                                 failedFareUsage,
                                 failedEOETargetFareUsage,
                                 itin);
    }
    else
    {
      bool cxrRet = false;
      ret = false;
      FareUsage* failedFareUsageLocal = nullptr;
      failedFareUsage = nullptr;
      bool failsDifferentPTF = false;
      for (CarrierCode valCxr : valCxrList)
      {
        validationFareComponent->validatingCarrier() = valCxr;
        cxrRet = dataStringValidation(diag,
                                      directionalityInfo,
                                      validationFareComponent,
                                      subCat,
                                      fareUsage,
                                      pCat10,
                                      curPu,
                                      farePath,
                                      failedFareUsageLocal,
                                      failedEOETargetFareUsage,
                                      itin);

        if (!cxrRet)
        {
          removeValCxr(valCxr, curPu, farePath);
          diag << "FAILED FOR VAL-CXR " << valCxr << "\n";
          if (failedFareUsage && failedFareUsage != failedFareUsageLocal)
          {
            failsDifferentPTF = true;
          }
          else
            failedFareUsage = failedFareUsageLocal;
        }
        else
          ret = true;
      }
      if (ret || failsDifferentPTF)
        failedFareUsage = nullptr;
    }

    if (ret)
      continue;
    else
      return false;

  } // for each FareUsage

  LOG4CXX_INFO(logger, __FILE__ << ":" << __LINE__ << " exit " << (ret ? "TRUE" : "FALSE"));

  return ret;
}

// ----------------------------------------------------------------------------
bool
Combinations::checkQualifyingCat15(CombinabilityRuleInfo* pCat10) const
{
  for (const auto& setPtr : pCat10->categoryRuleItemInfoSet())
  {
    for (const auto& item : *setPtr)
    {
      if (item.itemcat() == 15)
      {
        // check record3 Cat-15
        const SalesRestriction* salesRest =
            _trx->dataHandle().getSalesRestriction(pCat10->vendorCode(), item.itemNo());
        if (salesRest && salesRest->validationInd() != RuleConst::BLANK)
          return true;
      }
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
bool
Combinations::getMultipleValidatingCarriers(const PricingUnit& pu,
                                            const FarePath* fp,
                                            std::vector<CarrierCode>& valCxrList)
{
  if (!_trx->isValidatingCxrGsaApplicable())
    return false;

  if (fp)
  {
    if (fp->validatingCarriers().empty())
      return false;
    else
    {
      valCxrList = fp->validatingCarriers();
      return true;
    }
  }
  if (pu.validatingCarriers().empty())
    return false;
  else
  {
    valCxrList = pu.validatingCarriers();
    return true;
  }
}

// ----------------------------------------------------------------------------
void
Combinations::removeValCxr(CarrierCode& valCxr, PricingUnit& pu, FarePath* fp)
{
  std::vector<CarrierCode>& valCxrList = fp ? fp->validatingCarriers() : pu.validatingCarriers();
  std::vector<CarrierCode>::iterator it = valCxrList.begin();
  std::vector<CarrierCode>::iterator itEnd = valCxrList.end();
  for (; it != itEnd; ++it)
  {
    if (*it == valCxr)
    {
      valCxrList.erase(it);
      break;
    }
  }
}

// ----------------------------------------------------------------------------
bool
Combinations::dataStringValidation(DiagCollector& diag,
                                   DirectionalityInfo& directionalityInfo,
                                   ValidationFareComponents* validationFareComponent,
                                   const uint16_t subCat,
                                   FareUsage& curFu,
                                   const CombinabilityRuleInfo* pCat10,
                                   const PricingUnit& curPu,
                                   FarePath* farePath,
                                   FareUsage*& failedFareUsage,
                                   FareUsage*& failedEOETargetFareUsage,
                                   Itin* itin)
{
  bool majorFound = false;
  bool passedMinor;
  bool passedMajor = false;
  bool failedDirectionality = false;
  bool haveMinor;

  bool diagMajorSubCat = diagInMajorSubCat(diag.diagnosticType());

  if (!buildValidationFareComponent(
          diag, directionalityInfo, curPu, curFu, pCat10, *farePath, *validationFareComponent))
  {
    LOG4CXX_INFO(dataStringLogger,
                 __FILE__ << ":" << __LINE__ << " : build ValidationFareComponent failed");
    return false;
  }

  directionalityInfo.fareMarket =
      (directionalityInfo.validationLevel == PricingUnitLevel)
          ? curFu.paxTypeFare()->fareMarket()
          : farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->fareMarket();

  std::vector<CombinabilityRuleItemInfoSet*>::const_iterator setIter =
      pCat10->categoryRuleItemInfoSet().begin();
  std::vector<CombinabilityRuleItemInfoSet*>::const_iterator setEnd =
      pCat10->categoryRuleItemInfoSet().end();

  int datasetNumber = 0;
  bool needThenDelimitedDataset = false;
  bool forceFailure = false;
  bool haveNoMatch = false;

  _eoePresentFareTypes.reset();

  //----- loop through all "THEN" or "ELSE" set(s) ----------------------
  for (; !forceFailure && setIter != setEnd; ++setIter)
  {
    passedMajor = false;
    failedDirectionality = false;
    passedMinor = false;
    haveMinor = false;
    haveNoMatch = false;

    datasetNumber++;

    LOG4CXX_INFO(dataStringLogger, __FILE__ << ":" << __LINE__ << " DATA SET:" << datasetNumber);

    std::vector<CombinabilityRuleItemInfo>::const_iterator itemIter = (*setIter)->begin();
    std::vector<CombinabilityRuleItemInfo>::const_iterator itemEnd = (*setIter)->end();

    for (; itemIter != itemEnd; ++itemIter)
    {
      const CombinabilityRuleItemInfo* pCat10Seg = &(*itemIter);

      if (!matchMajorSubCat(*pCat10Seg,
                            subCat,
                            needThenDelimitedDataset,
                            forceFailure,
                            diag,
                            diagMajorSubCat,
                            datasetNumber,
                            curFu))
      {
        continue;
      }

      // START of a Matching Major Sub Category

      majorFound = true;

      if (UNLIKELY(needThenDelimitedDataset &&
                    pCat10Seg->relationalInd() != CategoryRuleItemInfo::THEN))
      {
        LOG4CXX_INFO(dataStringLogger, " FAILED COMBINATION - 104 THEN required ELSE found");
        passedMajor = false;
        forceFailure = true;
        break;
      }
      else
      {
        needThenDelimitedDataset = false;
        forceFailure = false;
      }

      if (!validateDirectionality(*pCat10Seg, curFu, pCat10, directionalityInfo))
      {
        LOG4CXX_INFO(dataStringLogger, " FAILED COMBINATION - REC 2 DIRECTIONALITY");
        if (UNLIKELY(diagMajorSubCat && diag.isActive()))
          diag << " R2 DIRECTIONALITY NO MATCH - ITEM " << pCat10Seg->itemNo() << "\n";
        failedDirectionality = true;
        if (((itemIter + 1) != itemEnd) &&
            (*(itemIter + 1)).relationalInd() == CategoryRuleItemInfo::OR)
          continue;
        break;
      }

      passedMinor = true;
      haveMinor = false;
      ValidationElement* negMatchedVfc = nullptr;

      // Need to clear the ValidationFare Components
      if (datasetNumber > 1)
      {
        if (validationFareComponent->needAllPassSameMajorItem())
          validationFareComponent->reset();
        else
          validationFareComponent->resetMinor();
      }

      // Save info to get back here in case we need to recheck using it
      std::vector<CombinabilityRuleItemInfo>::const_iterator majorCatIter = itemIter;
      std::vector<CombinabilityRuleItemInfo>::const_iterator majorCatEndIter = itemEnd;

      // IF there is a next item
      for (++itemIter; itemIter != itemEnd; ++itemIter)
      {
        pCat10Seg = &(*itemIter);

        // IF it is part of this Major SubCategory block
        if (pCat10Seg->relationalInd() == CategoryRuleItemInfo::OR)
        {
          // IF it is the same Major Category
          if (pCat10Seg->itemcat() == subCat)
          {
            LOG4CXX_INFO(dataStringLogger,
                         __FILE__ << ":" << __LINE__
                                  << " OR of the same category:" << pCat10Seg->itemcat());
          }
          else
          {
            majorCatEndIter = itemIter;

            LOG4CXX_INFO(dataStringLogger,
                         __FILE__ << ":" << __LINE__
                                  << " OR of Different Category:" << pCat10Seg->itemcat());
          }
        }
        else if (pCat10Seg->relationalInd() == CategoryRuleItemInfo::IF)
        {
          haveMinor = true;

          // FLAG the end of the Major Subcategories
          if (majorCatEndIter == itemEnd)
            majorCatEndIter = itemIter;

          // PROCESS the Minor Subcategories
          // check if we have same MajorCat, overflown by subcat
          // Overflow is to help pass Minor cats, as multiple target FC may pass minor cats in
          // different set. So we only need overflow check if validationFareComponent size is
          // greater than 1
          bool minorCatOverflow = false;
          std::vector<CombinabilityRuleItemInfoSet*>::const_iterator overFlowSetIterEnd =
              setIter + 1;

          if (validationFareComponent->size() > 1)
          {
            for (; overFlowSetIterEnd != setEnd; overFlowSetIterEnd++)
            {
              if (!setsWithSameMajorCat(**setIter, **overFlowSetIterEnd))
                break;

              minorCatOverflow = true;
              datasetNumber++;
            }
          }

          failedFareUsage = nullptr;
          failedEOETargetFareUsage = nullptr;
          if (minorCatOverflow)
          {
            if (UNLIKELY(diagMajorSubCat && diag.isActive()))
            {
              diag << " OVERFLOWN TO DATA SET: " << datasetNumber << "\n";
            }
            passedMinor = processMinorCatOverflow(setIter,
                                                  overFlowSetIterEnd,
                                                  diagMajorSubCat,
                                                  diag,
                                                  farePath,
                                                  curPu,
                                                  curFu,
                                                  pCat10,
                                                  *validationFareComponent,
                                                  negMatchedVfc,
                                                  failedFareUsage,
                                                  failedEOETargetFareUsage,
                                                  itin);

            setIter = overFlowSetIterEnd - 1;
          }
          else
          {
            bool negApplFlag = false;
            passedMinor = processMinorCat(itemIter,
                                          itemEnd,
                                          diagMajorSubCat,
                                          diag,
                                          farePath,
                                          curPu,
                                          curFu,
                                          pCat10,
                                          *validationFareComponent,
                                          negApplFlag,
                                          failedFareUsage,
                                          failedEOETargetFareUsage,
                                          itin);

            if (passedMinor && negApplFlag)
            {
              if (!failedEOETargetFareUsage)
                negMatchedVfc = &((*validationFareComponent)[0]);
              else
              {
                const size_t numVfc = validationFareComponent->size();
                for (size_t i = 0; i < numVfc; i++)
                {
                  if ((*validationFareComponent)[i]._currentFareUsage == failedFareUsage &&
                      (*validationFareComponent)[i]._targetFareUsage == failedEOETargetFareUsage)
                  {
                    negMatchedVfc = &((*validationFareComponent)[i]);
                    break;
                  }
                }
              }
            }
          }

          break;
        }
      }

      if (passedMinor)
      {
        if (!haveMinor)
        {
          validationFareComponent->setMinorPass();
        }

        failedFareUsage = nullptr;
        failedEOETargetFareUsage = nullptr;
        LOG4CXX_INFO(dataStringLogger, __FILE__ << ":" << __LINE__ << " PASSED minor SubCats");

        // CHECK the appropriate major Categories
        // (Note this could be plural if a "MAJOR OR" condition exists)

        if (UNLIKELY(diagMajorSubCat && diag.isActive()))
        {
          diag << "   PASSED MINOR SUBCATS AT DATA SET " << datasetNumber;
          if (negMatchedVfc)
            diag << " WITH NEG APPL\n";
          else
            diag << "\n";
        }

        for (; majorCatIter != majorCatEndIter; ++majorCatIter)
        {
          const CombinabilityRuleItemInfo* pCat10MajorSegA = &(*majorCatIter);

          if (pCat10MajorSegA->itemcat() != subCat)
            continue;

          if (pCat10MajorSegA->itemNo() == 0 ||
              (pCat10MajorSegA->textonlyInd() == 'Y' && pCat10MajorSegA->itemcat() != OPEN_JAW))
          {
            if (UNLIKELY(diag.isActive()))
            {
              std::string fareBasis = curFu.paxTypeFare()->createFareBasis(nullptr);
              if (negMatchedVfc)
                diag << " FAILED COMBINATION NEG MATCHED - SUBCAT ITEM 0 OR TEXT ONLY ";
              else
                diag << " PASSED COMBINATION - SUBCAT ITEM 0 OR TEXT ONLY";
              diag << " - FARE: " << fareBasis << "\n";
            }
            if (negMatchedVfc)
            {
              passedMajor = false;
              failedFareUsage = negMatchedVfc->_currentFareUsage;
              failedEOETargetFareUsage = negMatchedVfc->_targetFareUsage;
              forceFailure = true;
            }
            else
            {
              if (!validationFareComponent->needAllPassSameMajorItem())
              {
                if (!fallback::fixAPO37202(_trx))
                  passedMajor = validationFareComponent->evaluateMajorByPassedMinor();
                else
                  passedMajor = validationFareComponent->evaluateMajorByPassedMinor_old();

                if (!passedMajor)
                {
                  validationFareComponent->diagNotPassedFC(diag);
                  haveNoMatch = true;
                }
              }
              else
              {
                passedMajor = true;
              }
              needThenDelimitedDataset = false;
            }

            break;
          }
          else
          {
            char majorResult = (directionalityInfo.validationLevel == PricingUnitLevel)
                                   ? processMajorSubCat(diag,
                                                        curPu,
                                                        curFu,
                                                        pCat10,
                                                        *pCat10MajorSegA,
                                                        *validationFareComponent,
                                                        negMatchedVfc)
                                   : processMajorSubCat(diag,
                                                        curPu,
                                                        curFu,
                                                        pCat10,
                                                        *pCat10MajorSegA,
                                                        *farePath,
                                                        *validationFareComponent,
                                                        negMatchedVfc,
                                                        directionalityInfo);

            if (!checkMajorResultAndContinue(majorResult,
                                             passedMajor,
                                             subCat,
                                             haveNoMatch,
                                             needThenDelimitedDataset,
                                             forceFailure,
                                             haveMinor,
                                             majorCatIter,
                                             majorCatEndIter,
                                             validationFareComponent,
                                             pCat10MajorSegA,
                                             datasetNumber,
                                             farePath,
                                             curFu,
                                             failedFareUsage,
                                             failedEOETargetFareUsage,
                                             diag))
            {
              break;
            }
          }
        } // major iter
      }
      else
      {
        if (UNLIKELY(diagMajorSubCat && diag.isActive()))
          diag << "   FAILED MINOR SUBCATS AT DATA SET " << datasetNumber << "\n";

        LOG4CXX_INFO(dataStringLogger, __FILE__ << ":" << __LINE__ << " FAILED minor SubCats");
      }

      break; // CHECK the next set (this one has been processed)

    } // for itemIter

    if (!continueNextSet(
            passedMajor, forceFailure, majorFound, passedMinor, haveMinor, haveNoMatch))
      break;

  } // for setIter

  if (passedCurrentFareComponent(validationFareComponent,
                                 subCat,
                                 farePath,
                                 curPu,
                                 curFu,
                                 majorFound,
                                 passedMajor,
                                 failedDirectionality,
                                 failedFareUsage,
                                 failedEOETargetFareUsage,
                                 diag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------------------
bool
Combinations::checkMajorResultAndContinue(
    char majorResult,
    bool& passedMajor,
    const uint16_t subCat,
    bool& haveNoMatch,
    bool& needThenDelimitedDataset,
    bool& forceFailure,
    bool haveMinor,
    const std::vector<CombinabilityRuleItemInfo>::const_iterator& majorCatIter,
    const std::vector<CombinabilityRuleItemInfo>::const_iterator& majorCatEndIter,
    ValidationFareComponents* validationFareComponent,
    const CombinabilityRuleItemInfo* pCat10MajorSeg,
    int datasetNumber,
    FarePath* farePath,
    FareUsage& curFu,
    FareUsage*& failedFareUsage,
    FareUsage*& failedEOETargetFareUsage,
    DiagCollector& diag)

{
  if ((majorResult != ABORT) && (majorResult != PASSCOMB) && (majorResult != STOPCOMB))
  {
    passedMajor = validationFareComponent->evaluateMajor(failedFareUsage, failedEOETargetFareUsage);
    if (majorResult == IDLE)
    {
      if (passedMajor)
        majorResult = PASSCOMB;
      else
        majorResult = NO_MATCH;
    }
  }

  if (majorResult == PASSCOMB)
  {
    passedMajor = true;

    // for NVA/NVB EndOnEnd process
    if (subCat == END_ON_END && farePath != nullptr)
    {
      FareUsage& fu = const_cast<FareUsage&>(curFu);
      fu.endOnEndRequired() = true;
      if (fallback::fixed::fallbackMipOptEOERules())
      {
        fu.eoeRules().push_back(pCat10MajorSeg);
      }
      else
      {
        if (std::find(fu.eoeRules().begin(), fu.eoeRules().end(), pCat10MajorSeg) ==
            fu.eoeRules().end())
        {
          fu.eoeRules().push_back(pCat10MajorSeg);
        }
      }
    }

    if (UNLIKELY(diag.isActive()))
      diag << "   PASSED MAJOR SUBCATS AT DATA SET " << datasetNumber << "\n";

    needThenDelimitedDataset = false;
    // break;  // No need to check further in this subcategory - it passed.
    return false;
  }
  else if (majorResult == FAILCOMB)
  {
    if (diag.isActive())
      diag << "   FAILED NEG MAJOR SUBCAT " << subCat << " AT DATA SET " << datasetNumber << "\n";
    forceFailure = true;
    // break;
    return false;
  }
  else if (majorResult == STOPCOMB)
  {
    validationFareComponent->evaluateMajor(failedFareUsage, failedEOETargetFareUsage);
    passedMajor = false;

    if (diag.isActive())
      diag << "   FATAL " << subCat << " AT DATA SET " << datasetNumber << "\n";
    forceFailure = true;
    // break;
    return false;
  }
  else // majorResult == NO_MATCH
  // majorResult == ABORT
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "   FAILED MAJOR SUBCAT " << subCat << " AT DATA SET " << datasetNumber << "\n";
      if (subCat == END_ON_END && !validationFareComponent->needAllPassSameMajorItem())
      {
        validationFareComponent->diagNotPassedFC(diag);
      }
    }

    if (subCat == 104)
    {
      if ((majorCatIter + 1) != majorCatEndIter)
      {
        // continue;
        return true;
      }
      if (LIKELY(majorResult == NO_MATCH))
        haveNoMatch = true;
      validationFareComponent->resetMajor();
    }
    if (haveMinor)
    {
      if (majorResult == NO_MATCH &&
          ((subCat == 101 && ((*validationFareComponent)[0].getSubCat(M101) == MAJOR_NO_MATCH)) ||
           (subCat == 102 && ((*validationFareComponent)[0].getSubCat(M102) == MAJOR_NO_MATCH)) ||
           (subCat == 103 && ((*validationFareComponent)[0].getSubCat(M103) == MAJOR_NO_MATCH))))
      {
        haveNoMatch = true;
      }
      needThenDelimitedDataset = true;
    }
  }
  return true;
}

//-------------------------------------------------------------------------------
bool
Combinations::continueNextSet(bool passedMajor,
                              bool forceFailure,
                              bool majorFound,
                              bool passedMinor,
                              bool haveMinor,
                              bool haveNoMatch)

{
  if (passedMajor)
  {
    LOG4CXX_INFO(dataStringLogger, __FILE__ << ":" << __LINE__ << " PASSED major SubCat");
    return false; // No need to check further in any set - this one passed.
  }
  else if (forceFailure)
  {
    LOG4CXX_INFO(dataStringLogger, __FILE__ << ":" << __LINE__ << " FAILED major SubCat");
    return false;
  }
  else if (majorFound && !passedMajor && passedMinor && haveMinor && !haveNoMatch)
  {
    LOG4CXX_INFO(dataStringLogger, __FILE__ << ":" << __LINE__ << " FAILED major SubCat");
    return false; // No need to check further in any set - this one Failed.
  }
  return true;
}

//-------------------------------------------------------------------------------
bool
Combinations::passedCurrentFareComponent(ValidationFareComponents* validationFareComponent,
                                         const uint16_t subCat,
                                         FarePath* farePath,
                                         const PricingUnit& curPu,
                                         FareUsage& curFu,
                                         bool majorFound,
                                         bool passedMajor,
                                         bool failedDirectionality,
                                         FareUsage*& failedFareUsage,
                                         FareUsage*& failedEOETargetFareUsage,
                                         DiagCollector& diag)
{
  if (!majorFound || passedMajor)
  {
    // continue;    // Check the next FareUsage
    return true;
  }
  else
  {
    LOG4CXX_INFO(dataStringLogger,
                 __FILE__ << ":" << __LINE__
                          << ": IN Combinations  process MajorSubCat Returned false");

    if (failedDirectionality)
    {
      displayDiag(diag, 70, curFu, 0);
    }

    if (UNLIKELY(curFu.isKeepFare()))
    {
      // RexPricing, do not kill
      if (diag.isActive())
      {
        diag << " CAT31 EXCHANGE KEEP FARE - DO NOT FAIL NOW\n";
      }
      curFu.combinationFailedButSoftPassForKeepFare() = true;
      (const_cast<PricingUnit&>(curPu)).combinationFailedButSoftPassForKeepFare() = true;
      // continue;    // Check the next Fare usage
      return true;
    }

    if (subCat == END_ON_END && !validationFareComponent->needAllPassSameMajorItem() &&
        !failedEOETargetFareUsage)
    {
      validationFareComponent->getNotPassedFC(failedFareUsage, failedEOETargetFareUsage);
    }
    // ret = false;
    // break;
    return false;
  }
}
//-------------------------------------------------------------------------------
bool
Combinations::matchMajorSubCat(const CombinabilityRuleItemInfo& pCat10Seg,
                               const uint16_t subCat,
                               bool& needThenDelimitedDataset,
                               bool& forceFailure,
                               DiagCollector& diag,
                               bool diagMajorSubCat,
                               int datasetNumber,
                               FareUsage& curFu)
{
  //------------------------------------------------------//
  //----- Find Major sub cat to match --------------------//
  //------------------------------------------------------//

  if (pCat10Seg.relationalInd() == CategoryRuleItemInfo::IF)
  {
    LOG4CXX_INFO(dataStringLogger,
                 __FILE__ << ":" << __LINE__ << " Skipping Minor Cat:" << pCat10Seg.itemcat()
                          << " Itemno: " << pCat10Seg.itemNo());
    return false;
  }
  else if (pCat10Seg.itemcat() != subCat)
  {
    LOG4CXX_INFO(dataStringLogger,
                 __FILE__ << ":" << __LINE__ << " Skipping Cat:" << pCat10Seg.itemcat()
                          << " Itemno: " << pCat10Seg.itemNo());
    if (needThenDelimitedDataset && pCat10Seg.relationalInd() == CategoryRuleItemInfo::THEN)
    {
      LOG4CXX_INFO(dataStringLogger, " Clearing 'THEN required' flag");
      needThenDelimitedDataset = false;
      forceFailure = false;
    }
    return false;
  }

  if (UNLIKELY(diagMajorSubCat))
  {
    if (diag.isActive())
    {
      std::string fareBasis = curFu.paxTypeFare()->createFareBasis(nullptr);
      diag << " PROCESS DATA SET: " << datasetNumber << " - FARE: " << fareBasis << "\n";
    }
  }

  LOG4CXX_INFO(dataStringLogger,
               __FILE__ << ":" << __LINE__ << " Major Cat Matched:" << pCat10Seg.itemcat()
                        << " Itemno: " << pCat10Seg.itemNo());
  return true;
}

//============================================================

bool
Combinations::setsWithSameMajorCat(const CombinabilityRuleItemInfoSet& set1,
                                   const CombinabilityRuleItemInfoSet& set2)
{
  auto major1CatIter = set1.begin();
  const auto major1CatIterEnd = set1.end();

  auto major2CatIter = set2.begin();
  const auto major2CatIterEnd = set2.end();

  for (; major1CatIter != major1CatIterEnd; major1CatIter++, major2CatIter++)
  {
    if (major2CatIter == major2CatIterEnd)
      return false;

    const auto& pMajorSegSet1 = (*major1CatIter);
    const auto& pMajorSegSet2 = (*major2CatIter);

    if (pMajorSegSet1.relationalInd() == CategoryRuleItemInfo::IF)
    {
      if (pMajorSegSet2.relationalInd() != CategoryRuleItemInfo::IF)
        return false; // set2 has more major item

      break;
    }

    if (pMajorSegSet1.itemcat() != pMajorSegSet2.itemcat())
      return false;
    if (pMajorSegSet1.itemNo() != pMajorSegSet2.itemNo())
      return false;
  }

  return true;
}

bool
Combinations::processMinorCatOverflow(
    const std::vector<CombinabilityRuleItemInfoSet*>::const_iterator& setIterBegin,
    const std::vector<CombinabilityRuleItemInfoSet*>::const_iterator& setIterEnd,
    bool diagMajorSubCat,
    DiagCollector& diag,
    FarePath* farePath,
    const PricingUnit& curPu,
    FareUsage& curFU,
    const CombinabilityRuleInfo* pCat10,
    ValidationFareComponents& validationFareComponent,
    ValidationElement*& negMatchedVfc,
    FareUsage*& failedFareUsage,
    FareUsage*& failedEOETargetFareUsage,
    Itin* itin)
{
  negMatchedVfc = nullptr;

  // There are multiple minor subcat set.
  // All individual FareUsage needs to pass a minor subcat set, not fail
  // any negative subcat set;
  // It is not required that all FareUsage needs to pass same subcat set
  std::vector<ValidationElement>::iterator vfcIt = validationFareComponent.begin();
  const std::vector<ValidationElement>::iterator vfcItEnd = validationFareComponent.end();

  ValidationFareComponents singleValidationFC;
  singleValidationFC.resize(1);
  singleValidationFC.setForcePass(false);

  for (; vfcIt != vfcItEnd; vfcIt++)
  {
    if (!validationFareComponent.needAllPassSameMajorItem() && (*vfcIt)._passMajor)
      continue;

    singleValidationFC[0].initialize((*vfcIt)._currentFareUsage, pCat10, (*vfcIt)._targetFareUsage);
    singleValidationFC.reset();
    if (UNLIKELY(diagMajorSubCat && diag.isActive()))
    {
      diag << "    COMBINED WITH - "
           << (*vfcIt)._targetFareUsage->paxTypeFare()->createFareBasis(nullptr) << "\n";
    }

    bool passedMinor = false;
    bool negApplFlag = false;

    std::vector<CombinabilityRuleItemInfoSet*>::const_iterator setIter = setIterBegin;
    for (; setIter != setIterEnd; setIter++)
    {
      std::vector<CombinabilityRuleItemInfo>::const_iterator minorCatIter = (*setIter)->begin();
      const std::vector<CombinabilityRuleItemInfo>::const_iterator minorCatIterEnd =
          (*setIter)->end();

      for (; minorCatIter != minorCatIterEnd; minorCatIter++)
      {
        const CombinabilityRuleItemInfo* pCat10Seg = &(*minorCatIter);
        if (pCat10Seg->relationalInd() == CategoryRuleItemInfo::IF)
          break;
      }

      negApplFlag = false;
      passedMinor = processMinorCat(minorCatIter,
                                    minorCatIterEnd,
                                    diagMajorSubCat,
                                    diag,
                                    farePath,
                                    curPu,
                                    curFU,
                                    pCat10,
                                    singleValidationFC,
                                    negApplFlag,
                                    failedFareUsage,
                                    failedEOETargetFareUsage,
                                    itin);

      if (passedMinor)
      {
        break;
      }

      if (UNLIKELY(diagMajorSubCat && diag.isActive()))
        diag << "    -----\n";

      singleValidationFC.reset();
    }
    if (!passedMinor)
    {
      if (!validationFareComponent.needAllPassSameMajorItem())
        continue;

      return false;
    }

    (*vfcIt)._passMinor = true;
    validationFareComponent.anyPassMinor() = true;

    if (negApplFlag)
    {
      negMatchedVfc = &(*vfcIt);
      break; // we have on FC matched with NegAppl, we either not match
      // Major, or have to fail the combination. So stop here
    }
  }

  if (!validationFareComponent.needAllPassSameMajorItem())
    return validationFareComponent.anyPassMinor();

  return true;
}

bool
Combinations::processMinorCat(
    const std::vector<CombinabilityRuleItemInfo>::const_iterator& minorCatIterBegin,
    const std::vector<CombinabilityRuleItemInfo>::const_iterator& minorCatIterEnd,
    bool diagMajorSubCat,
    DiagCollector& diag,
    FarePath* farePath,
    const PricingUnit& curPu,
    FareUsage& curFU,
    const CombinabilityRuleInfo* pCat10,
    ValidationFareComponents& validationFareComponent,
    bool& negApplFlag,
    FareUsage*& failedFareUsage,
    FareUsage*& failedEOETargetFareUsage,
    Itin* itin)
{
  bool passedMinor = false;

  std::vector<CombinabilityRuleItemInfo>::const_iterator minorCatIter = minorCatIterBegin;
  while (minorCatIter != minorCatIterEnd)
  {
    const CombinabilityRuleItemInfo* pCat10Seg = &(*minorCatIter);

    if (pCat10Seg->relationalInd() == CategoryRuleItemInfo::OR)
    {
      if (passedMinor && !negApplFlag)
      {
        // Evaluate all Minor sub-cat together
        passedMinor = validationFareComponent.evaluateMinor();
      }
      if (passedMinor)
      {
        if (UNLIKELY(diagMajorSubCat && diag.isActive()))
          diag << "    PASSED MINOR \n";
        return true;
      }
      else
      {
        if (UNLIKELY(diagMajorSubCat && diag.isActive()))
        {
          diag << "    FAILED MINOR\n"
               << "    PROCESS MINOR OR:\n";
        }
        negApplFlag = false;
        validationFareComponent.reset();
      }
    }

    passedMinor = processMinorSubCat(diag,
                                     farePath,
                                     curPu,
                                     curFU,
                                     pCat10,
                                     *pCat10Seg,
                                     negApplFlag,
                                     validationFareComponent,
                                     itin);

    if (!passedMinor && !negApplFlag && !validationFareComponent.needAllPassSameMajorItem())
    {
      passedMinor = validationFareComponent.evaluateMinor();
    }

    if (passedMinor)
    {
      if (UNLIKELY(diagMajorSubCat && diag.isActive()))
        diag << "    PASSED MINOR SUBCAT: " << pCat10Seg->itemcat() << "   ITEM "
             << pCat10Seg->itemNo() << "\n";
    }
    else
    {
      if (UNLIKELY(diagMajorSubCat && diag.isActive()))
        diag << "    FAILED MINOR SUBCAT: " << pCat10Seg->itemcat() << "   ITEM "
             << pCat10Seg->itemNo() << "\n";
    }

    if (!passedMinor && negApplFlag && validationFareComponent.size() > 1)
    {
      passedMinor =
          validationFareComponent.evaluateMinorNegAppl(failedFareUsage, failedEOETargetFareUsage);
    }

    if (!passedMinor)
    {
      ++minorCatIter;
      while (minorCatIter != minorCatIterEnd)
      {
        // look for OR cluase
        if (minorCatIter->relationalInd() == CategoryRuleItemInfo::OR)
          break;
        else
          ++minorCatIter;
      }
    }
    else
    {
      ++minorCatIter;
    }
  }

  if (passedMinor && !negApplFlag)
  {
    // Evaluate all Minor sub-cat together
    passedMinor = validationFareComponent.evaluateMinor();
  }

  if (!passedMinor && !negApplFlag)
  {
    if (farePath != nullptr)
    {
      // FarePath Scope
      validationFareComponent.getFailedFUInMinor(failedFareUsage, failedEOETargetFareUsage);
    }
  }

  return passedMinor;
}

// ----------------------------------------------------------------------------
bool
Combinations::buildValidationFareComponent(DiagCollector& diag,
                                           DirectionalityInfo& directionalityInfo,
                                           const PricingUnit& curPu,
                                           FareUsage& curFareUsage,
                                           const CombinabilityRuleInfo* pCat10,
                                           const FarePath& farePath,
                                           ValidationFareComponents& validationFareComponent)
{
  bool ret = true;

  // Check that the current fare usage has a Cat10 record
  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "build ValidationFareComponent: NO REC 2 CAT 10");
    ret = false;
  }
  else
  {
    validationFareComponent.resetCarrierPref();

    // IF the list id is being built for a PricingUnit level test
    if (directionalityInfo.validationLevel == PricingUnitLevel)
    {
      ret = populateValidationFareComponent(
          diag, curPu, curFareUsage, pCat10, validationFareComponent);
    }
    else // Fare Path level validation
    {
      ret = populateValidationFareComponent(
          diag, curPu, curFareUsage, pCat10, farePath, validationFareComponent);
    }
  }
  if (!ret)
  {
    if (diag.isActive())
    {
      if (directionalityInfo.validationLevel == PricingUnitLevel)
        diag << " ERROR IN BUILD VALIDATION FC AT PU LEVEL\n";
      else
        diag << " ERROR IN BUILD VALIDATION FC AT FP LEVEL\n";
    }
  }
  else if (UNLIKELY(_trx->excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    // do not validate DummyFare combinability
    TargetIsDummyFare targetIsDummyFare;

    validationFareComponent.erase(std::remove_if(validationFareComponent.begin(),
                                                 validationFareComponent.end(),
                                                 targetIsDummyFare),
                                  validationFareComponent.end());
  }

  return ret;
}

Combinations::EOEAllSegmentIndicator
Combinations::determineAllSegmentIndicatorValue(const CombinabilityRuleInfo* pCat10) const
{
  EOEAllSegmentIndicator allSegmentIndicator = Adjacent;

  std::vector<CombinabilityRuleItemInfoSet*>::const_iterator setIter =
      pCat10->categoryRuleItemInfoSet().begin();
  std::vector<CombinabilityRuleItemInfoSet*>::const_iterator setEnd =
      pCat10->categoryRuleItemInfoSet().end();

  for (; setIter != setEnd; ++setIter)
  {
    std::vector<CombinabilityRuleItemInfo>::const_iterator itemIter = (*setIter)->begin();
    std::vector<CombinabilityRuleItemInfo>::const_iterator itemEnd = (*setIter)->end();

    for (; itemIter != itemEnd; ++itemIter)
    {
      const CombinabilityRuleItemInfo* pCat10SegA = &(*itemIter);

      if (pCat10SegA->itemcat() == END_ON_END)
      {
        if (pCat10SegA->itemNo() == 0 || pCat10SegA->textonlyInd() == 'Y' ||
            pCat10SegA->eoeallsegInd() == 'A')
        {
          allSegmentIndicator = AllSegment;
          setIter = setEnd - 1; // We don't have to look at any more sets
          break; // We don't have to look at any more items for this set
        }
        else if (UNLIKELY(pCat10SegA->eoeallsegInd() == 'C'))
        {
          allSegmentIndicator = CommonPoint;
        }
        // default pCat10SegA->eoeallsegInd() == 'J'
      }
    } // for itemIter
  } // for setIter

  return allSegmentIndicator;
}

const std::vector<PricingUnit*>*
Combinations::determineSideTripPUVector(const PricingUnit& curPu, const FarePath& farePath) const
{
  const std::vector<PricingUnit*>* sideTripPUVector = nullptr;

  // find the appropriate vector of sidetrip PUs
  std::vector<PricingUnit*>::const_iterator puWholePathIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puWholePathItEnd = farePath.pricingUnit().end();

  // LOOP through each PricingUnit
  for (; puWholePathIt != puWholePathItEnd && !sideTripPUVector; ++puWholePathIt)
  {
    // IF the PricingUnit is a sidtrip
    if ((*puWholePathIt)->hasSideTrip())
    {
      const std::vector<PricingUnit*>& currST = (*puWholePathIt)->sideTripPUs();
      std::vector<PricingUnit*>::const_iterator puSideTripIt = currST.begin();
      std::vector<PricingUnit*>::const_iterator puSideTripItEnd = currST.end();

      // LOOP through the PUs in this sidetrip
      for (; puSideTripIt != puSideTripItEnd; ++puSideTripIt)
      {
        // IF the source PU is in this sidetrip
        if (&curPu == (*puSideTripIt))
        {
          // This is the sidetrip we want.
          sideTripPUVector = &currST;
          break;
        }
      }
    }
  }
  return sideTripPUVector;
}

bool
Combinations::populateValidationFareComponent(DiagCollector& diag,
                                              const PricingUnit& curPu,
                                              FareUsage& curFareUsage,
                                              const CombinabilityRuleInfo* pCat10,
                                              const FarePath& farePath,
                                              ValidationFareComponents& validationFareComponent)
{
  int usedElements = 0;
  uint16_t numCarriers = 0; // These counters are not meant to be accurate.  If they are not
  uint16_t numVendors = 0; // zero at the end of processing, we have multiple carriers or vendors.
  bool allPublicFares = true;

  // ---------------------------------------------------------
  // Find the type of validation necessary for this Fare Usage
  // ---------------------------------------------------------

  EOEAllSegmentIndicator allSegmentIndicator = determineAllSegmentIndicatorValue(pCat10);

  // -----------------------------------------------------------------
  // Now that the type of validation is known, validation can proceed
  // -----------------------------------------------------------------

  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();
  bool wantMainTrip = true;

  // if this PU is in the Main trip, things are alreadt set up to process thos PUs.
  // IF it is a sidetrip PU, we have to set things up to process the correct SideTrip PUs.
  if (curPu.isSideTripPU())
  {
    // find the appropriate vector of sidetrip PUs
    const std::vector<PricingUnit*>* sideTripPUVector = determineSideTripPUVector(curPu, farePath);

    // IF there was no side trip vector found for this PU
    if (UNLIKELY(!sideTripPUVector))
    {
      // Make sure we have things set so diagnostic output will work
      // (Need to add the source FU to the validation list)
      validationFareComponent.resize(1);
      validationFareComponent[0].initialize(&curFareUsage, pCat10, &curFareUsage);

      validationFareComponent.setHasOneCarrier(numCarriers == 0);
      validationFareComponent.setHasOneVendor(numVendors == 0);
      validationFareComponent.setallPublicFares(allPublicFares);

      return true;
    }
    else
    {
      // Otherwise, set things up to process the indicated sidetrip vector
      puIt = sideTripPUVector->begin();
      puItEnd = sideTripPUVector->end();

      wantMainTrip = false;
    }
  }

  // -----------------------------------------------------------------------
  // Allocate space for a validation item for each fare usage
  // in each Pricing Unit in main trip with the exception of this one
  // (We want to make sure we allocate enough space for the worst
  //  case scenario (allsegments), but there is no point in
  //  validating within the pricing unit since we have already done that)
  //------------------------------------------------------------------------

  size_t numElements = 0;
  std::vector<PricingUnit*>::const_iterator puSizeIt = puIt;

  for (; puSizeIt != puItEnd; ++puSizeIt)
  {
    // if we are processing MT and hit a side trip PU,
    if (wantMainTrip && (*puSizeIt)->isSideTripPU())
    {
      // we have finished with all the main trip PUs.  So leave!
      break;
    }
    // ELSE IF the current is not the same as the Source PU
    else if (&curPu != (*puSizeIt))
    {
      // Reserve space for each of the FareUsages in the current FU.
      numElements += (*puSizeIt)->fareUsage().size();
    }
  }

  // Add the Target Fare Usages to the list of validation candidates
  if (numElements > 0)
  {
    validationFareComponent.resize(numElements);

    // Initialize the elements based on the EOE requirements.

    std::vector<FareUsage*>::const_iterator fuIt;
    std::vector<FareUsage*>::const_iterator fuItEnd;
    std::vector<ValidationElement>::iterator vfcIt = validationFareComponent.begin();

    // Loop through to add the appropriate FareUasges.
    for (; puIt != puItEnd; ++puIt)
    {
      // if we hit a side trip PU, we have finished with all the main trip PUs.
      if (wantMainTrip && (*puIt)->isSideTripPU())
      {
        // So leave!
        break;
      }
      // ELSE IF the current (target) PU is not the same as the Source PU
      else if (&curPu != (*puIt))
      {
        fuIt = (*puIt)->fareUsage().begin();
        fuItEnd = (*puIt)->fareUsage().end();

        // FOR EACH FareUsage in this PricingUnit
        for (; fuIt != fuItEnd; ++fuIt)
        {
          // ADD the appropriate ones to the list to be validated.
          if (LIKELY((*fuIt) != &curFareUsage))
          {
            if ((*fuIt)->rec2Cat10() == nullptr)
            {
              LOG4CXX_INFO(logger, " NO REC 2 CAT 10");
              return false;
            }
            else
            {
              const tse::LocCode& cOrig =
                  curFareUsage.paxTypeFare()->fareMarket()->boardMultiCity();
              const tse::LocCode& cDest = curFareUsage.paxTypeFare()->fareMarket()->offMultiCity();
              const tse::LocCode& tOrig = (*fuIt)->paxTypeFare()->fareMarket()->boardMultiCity();
              const tse::LocCode& tDest = (*fuIt)->paxTypeFare()->fareMarket()->offMultiCity();

              const Itin* itin = farePath.itin();

              if (allSegmentIndicator == AllSegment ||
                  (allSegmentIndicator == CommonPoint && ((cOrig == tOrig) || (cOrig == tDest) ||
                                                          (cDest == tOrig) || (cDest == tDest))) ||
                  (allSegmentIndicator == Adjacent &&
                   adjacentLineOfFlight(curFareUsage, **fuIt, *itin)))
              {
                // Set the entry in the list to reflect the need to validate
                //     the current "source FU"-"target FU" pair
                (*vfcIt).initialize(&curFareUsage, pCat10, *fuIt);
                vfcIt++;

                usedElements++;

                // Set the counters for Carriers, and Vendors, and
                // the PublicFares indicator based on the comparisons
                // between those for the Source and Target FUs
                const PaxTypeFare& targetPTF = *((*fuIt)->paxTypeFare());
                if (curFareUsage.paxTypeFare()->carrier() != targetPTF.carrier())
                {
                  if (!targetPTF.fare()->isIndustry() ||
                      (curFareUsage.paxTypeFare()->carrier() !=
                       targetPTF.fareMarket()->governingCarrier()))
                  {
                    numCarriers++;
                  }
                }

                if (curFareUsage.paxTypeFare()->vendor() != (*fuIt)->paxTypeFare()->vendor())
                {
                  numVendors++;
                }
                if (LIKELY(!curFareUsage.paxTypeFare()->publicFareCheck()))
                {
                  allPublicFares = false;
                }
              }
            }
          }
        } // END   for( ; fuIt != fuItEnd; ++fuIt)
      } //  END    else if (&curPu != (*puIt) )
    } //  END  for( ; puIt != puItEnd; ++puIt)

    // Insert the target FareUsages into the list.
  }

  // IF no items were put in the list, add the current one.  (There has to be an entry in
  //   the list because the first entry is accessed later in the code)
  if (usedElements == 0)
  {
    validationFareComponent.resize(1);
    validationFareComponent[0].initialize(&curFareUsage, pCat10, &curFareUsage);
    validationFareComponent.setForcePass(true);

    usedElements = 1;
  }
  else
  {
    validationFareComponent.resize(usedElements);
    validationFareComponent.setForcePass(false);
  }

  validationFareComponent.setHasOneCarrier(numCarriers == 0);
  validationFareComponent.setHasOneVendor(numVendors == 0);
  validationFareComponent.setallPublicFares(allPublicFares);
  validationFareComponent.anyPassMinor() = false;

  if (usedElements > 1)
  {
    validationFareComponent.needAllPassSameMajorItem() = false;
  }

  return usedElements > 0;
}

bool
Combinations::adjacentLineOfFlight(const FareUsage& curFareUsage,
                                   const FareUsage& targetFareUsage,
                                   const Itin& itin)
{
  const TravelSeg* cTvlSegOrig = nullptr;
  const TravelSeg* cTvlSegDest = nullptr;

  const TravelSeg* tTvlSegOrig = nullptr;
  const TravelSeg* tTvlSegDest = nullptr;

  cTvlSegOrig = curFareUsage.paxTypeFare()->fareMarket()->travelSeg().front();
  cTvlSegDest = curFareUsage.paxTypeFare()->fareMarket()->travelSeg().back();

  tTvlSegOrig = targetFareUsage.paxTypeFare()->fareMarket()->travelSeg().front();
  tTvlSegDest = targetFareUsage.paxTypeFare()->fareMarket()->travelSeg().back();

  if (itin.segmentFollows(cTvlSegDest, tTvlSegOrig) ||
      itin.segmentFollows(tTvlSegDest, cTvlSegOrig) ||
      itin.segmentFollowsAfterArunk(cTvlSegDest, tTvlSegOrig) ||
      itin.segmentFollowsAfterArunk(tTvlSegDest, cTvlSegOrig))
  {
    return true;
  }
  return false;
}

bool
Combinations::populateValidationFareComponent(DiagCollector& diag,
                                              const PricingUnit& curPu,
                                              FareUsage& curFareUsage,
                                              const CombinabilityRuleInfo* pCat10,
                                              ValidationFareComponents& validationFareComponent)
{
  int usedElements = 0;
  uint16_t numCarriers = 0; // These counters are not meant to be accurate.  If they are not
  uint16_t numVendors = 0; // zero at the end of processing, we have multiple carriers or vendors.
  bool allPublicFares = true;

  //    validationFareComponent.resetCarrierPref();

  // Allocate space for a validation item for each fare usage
  // in the Pricing Unit with the exception of this one
  // (There is no point in validating it with itself )
  size_t numElements = curPu.fareUsage().size() - 1;

  // If there are no other FUs in this PU things are easy.
  // IF there are other FUs, we need to insert them in the list.
  if (LIKELY(numElements >= 1))
  {
    // MAKE sure we have space for the FUs we will be adding.
    validationFareComponent.resize(numElements);

    // Initialize each entry so we can validate against all
    // other Fare Usages in this Pricing Unit
    std::vector<FareUsage*>::const_iterator fuIt = curPu.fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuItEnd = curPu.fareUsage().end();
    std::vector<ValidationElement>::iterator vfcIt = validationFareComponent.begin();

    // FOR EACH FareUsage in this PricingUnit
    for (; fuIt != fuItEnd; ++fuIt)
    {
      // IF it is not the currrent (source) FareUsage
      if (*fuIt != &curFareUsage)
      {
        // Verify that the current "target" fare usage has a valid Cat10RuleInfo
        // (If we check this here we won't have to check it anywhere else)
        if (UNLIKELY((*fuIt)->rec2Cat10() == nullptr))
        {
          LOG4CXX_INFO(logger, " NO REC 2 CAT 10");
          return false;
        }
        else
        {
          // Set the entry in the list to reflect the need to validate
          //     the current "source FU"-"target FU" pair
          (*vfcIt).initialize(&curFareUsage, pCat10, *fuIt);
          vfcIt++;
          usedElements++;

          // Set the counters for Carriers, and Vendors, and the PublicFares indicator
          //     based on the comparisons between those for the Source and Target FUs
          if (curFareUsage.paxTypeFare()->carrier() != (*fuIt)->paxTypeFare()->carrier())
          {
            numCarriers++;
          }
          if (curFareUsage.paxTypeFare()->vendor() != (*fuIt)->paxTypeFare()->vendor())
          {
            numVendors++;
          }
          if (LIKELY(!curFareUsage.paxTypeFare()->publicFareCheck()))
          {
            allPublicFares = false;
          }
        }
      }
    }
  }
  // If no items were put in the list
  if (UNLIKELY(usedElements == 0))
  {
    // Add an entry for the current FareUsage
    // (We need at least on entry since the first item
    //  in the list is referenced elsewhere for diagnostic output)
    validationFareComponent.setForcePass(true);
    validationFareComponent.resize(1);
    validationFareComponent[0].initialize(&curFareUsage, pCat10, &curFareUsage);
    usedElements = 1;
  }
  else
  {
    validationFareComponent.setForcePass(false);
    validationFareComponent.resize(usedElements);
  }

  // Set the "special" indicators used for optimizations within the validations
  validationFareComponent.setHasOneCarrier(numCarriers == 0);
  validationFareComponent.setHasOneVendor(numVendors == 0);
  validationFareComponent.setallPublicFares(allPublicFares);

  return usedElements > 0;
}

// ----------------------------------------------------------------------------
bool
Combinations::validateDirectionality(const CombinabilityRuleItemInfo& cat10Seg,
                                     const FareUsage& fusage,
                                     const CombinabilityRuleInfo* pCat10,
                                     const DirectionalityInfo& directionalityInfo)
{
  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "validate Directionality: NO REC 2 CAT 10");
    return false;
  }

  return (FAIL !=
          CategoryRuleItem::isDirectionPass(
              *_trx, fusage, *pCat10, &cat10Seg, fusage.paxTypeFare()->r2Cat10LocSwapped()));
}

// ----------------------------------------------------------------------------
// return :
//             NO_MATCH    no match major cat
//             PASSCOMB    passed major subcat
//             FAILCOMB    failed major subcat, need forced stop
//                         This is either on neg minor matched
//             ABORT       data error
// ----------------------------------------------------------------------------
char
Combinations::processMajorSubCat(DiagCollector& diag,
                                 const PricingUnit& pu,
                                 FareUsage& curFu,
                                 const CombinabilityRuleInfo* pCat10,
                                 const CombinabilityRuleItemInfo& pCat10Seg,
                                 ValidationFareComponents& validationFareComponent,
                                 ValidationElement* negMatchedVfc)
{
  LOG4CXX_INFO(logger, "in process MajorSubCat - PU");

  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "process MajorSubCat: NO REC 2 CAT 10");
    return ABORT;
  }

  char ret = NO_MATCH;

  switch (pCat10Seg.itemcat())
  {
  //-------------------------------//
  //  101 - Open Jaw Restrictions  //
  //-------------------------------//
  case OPEN_JAW:
  {
    const PricingUnit::PUSubType puSubType = pu.puSubType();

    if (UNLIKELY(
            ((puSubType == PricingUnit::ORIG_OPENJAW || puSubType == PricingUnit::DEST_OPENJAW) &&
             pCat10->sojInd() == PERMITTED) ||
            (puSubType == PricingUnit::DOUBLE_OPENJAW && pCat10->dojInd() == PERMITTED)))
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M101) = FAILCOMB;
        ret = FAILCOMB;
      }
      else
      {
        // overwrite any FAILCOMB result by previous set
        validationFareComponent[0].getSubCat(M101) = PASSCOMB;

        ret = PASSCOMB;
      }

      break;
    }

    const char result = processOpenJawRestriction(
        diag, pu, curFu, pCat10, pCat10Seg.itemNo(), validationFareComponent);
    if (UNLIKELY(result == ABORT))
    {
      ret = ABORT;
    }
    else if (result == PASSCOMB)
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M101) = FAILCOMB;
        ret = FAILCOMB;
      }
      else
        ret = PASSCOMB;
    }
    else
    {
      ret = NO_MATCH;
    }

    if (ret != PASSCOMB)
    {
      LOG4CXX_INFO(logger,
                   __LINE__ << ", : IN Combinations,  process MajorSubCat FAILED, PU-Level");
    }
    break;
  }

  //---------------------------------//
  //  102 - Round Trip Restrictions  //
  //---------------------------------//
  case ROUND_TRIP:
  {
    if (UNLIKELY(pCat10->ct2Ind() == PERMITTED || pCat10->ct2Ind() == 'V'))
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M102) = FAILCOMB;
        ret = FAILCOMB;
      }
      else
      {
        // overwrite any FAILCOMB result by previous set
        validationFareComponent[0].getSubCat(M102) = PASSCOMB;
        ret = PASSCOMB;
      }

      break;
    }

    const char result = processRoundTripRestriction(
        diag, pu, curFu, pCat10, pCat10Seg.itemNo(), validationFareComponent);
    if (UNLIKELY(result == ABORT))
    {
      ret = ABORT;
    }
    else if (result == PASSCOMB)
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M102) = FAILCOMB;
        ret = FAILCOMB;
      }
      else
        ret = PASSCOMB;
    }

    if (ret != PASSCOMB)
    {
      LOG4CXX_INFO(logger,
                   __LINE__ << ", : IN Combinations,  process MajorSubCat FAILED, PU-Level");
    }
    break;
  }

  //----------------------------------//
  //  103 - Circle Trip Restrictions  //
  //----------------------------------//
  case CIRCLE_TRIP:
  {
    if (UNLIKELY(pCat10->ct2plusInd() == PERMITTED))
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M103) = FAILCOMB;
        ret = FAILCOMB;
      }
      else
      {
        // overwrite any FAILCOMB result by previous set
        validationFareComponent[0].getSubCat(M103) = PASSCOMB;
        ret = PASSCOMB;
      }

      break;
    }

    const char result = processCircleTripRestriction(
        diag, pu, curFu, pCat10, pCat10Seg.itemNo(), validationFareComponent);
    if (UNLIKELY(result == ABORT))
    {
      ret = ABORT;
    }
    else if (result == PASSCOMB)
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M103) = FAILCOMB;
        ret = FAILCOMB;
      }
      else
        ret = PASSCOMB;
    }
    else
      ret = NO_MATCH;

    if (UNLIKELY(!ret))
    {
      LOG4CXX_INFO(logger,
                   __LINE__ << ", : IN Combinations,  process MajorSubCat FAILED, PU-Level");
    }
    break;
  }

  //---------------------------------//
  //  105 - Add-on                   //
  //---------------------------------//
  case ADD_ON:
  {
    ret = PASSCOMB;

    break;
  }

  //-----------//
  //  Default  //
  //-----------//
  default:
  {
    ret = NO_MATCH;
    LOG4CXX_INFO(logger, "ret = ABORT");
    LOG4CXX_INFO(logger, __LINE__ << ", : IN Combinations,  process MajorSubCat FAILED, PU-Level");
    break;
  }
  } // switch

  return ret;
}

// ----------------------------------------------------------------------------
// return :
//             NO_MATCH    no match major cat
//             PASSCOMB    passed major subcat
//             FAILCOMB    failed major subcat, need forced stop
//                         This is either on neg minor matched
//             ABORT       data error
//             IDLE        need evaluateMajor to know if we passed or not
// ----------------------------------------------------------------------------
char
Combinations::processMajorSubCat(DiagCollector& diag,
                                 const PricingUnit& pu,
                                 const FareUsage& curFu,
                                 const CombinabilityRuleInfo* pCat10,
                                 const CombinabilityRuleItemInfo& pCat10Seg,
                                 const FarePath& farePath,
                                 ValidationFareComponents& validationFareComponent,
                                 ValidationElement* negMatchedVfc,
                                 const DirectionalityInfo& directionalityInfo)
{
  LOG4CXX_INFO(logger, "in process MajorSubCat - FP");

  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "process MajorSubCat: NO REC 2 CAT 10");
    return ABORT;
  }

  char ret = NO_MATCH;
  switch (pCat10Seg.itemcat())
  {
  //-------------------------------//
  //  101 - Open Jaw Restrictions  //
  //-------------------------------//
  case OPEN_JAW:
  {
    ret = PASSCOMB;
    break;
  }

  //---------------------------------//
  //  102 - Round Trip Restrictions  //
  //---------------------------------//
  case ROUND_TRIP:
  {
    ret = PASSCOMB;
    break;
  }

  //----------------------------------//
  //  103 - Circle Trip Restrictions  //
  //----------------------------------//
  case CIRCLE_TRIP:
  {
    ret = PASSCOMB;
    break;
  }

  //---------------------------------//
  //  104 - End on End Restrictions  //
  //---------------------------------//
  case END_ON_END:
  {
    if (UNLIKELY(pCat10->eoeInd() == PERMITTED))
    {
      if (negMatchedVfc)
      {
        negMatchedVfc->getSubCat(M104) = FAILCOMB;
        ret = STOPCOMB; // no try next set
      }
      else
      {
        ret = PASSCOMB;
      }
      break;
    }

    // if we have negMatchedVfc, we only need to make sure this
    // negative matched validation FareComponent does not pass,
    // then we have NO_MATCH for the set;
    // otherwise we will have FAILCOMB
    if (!negMatchedVfc)
    {
      ret = processEndOnEndRestriction(
          diag, farePath, pu, pCat10Seg.itemNo(), validationFareComponent, directionalityInfo);
      if (UNLIKELY(ret == FAILCOMB))
        ret = NO_MATCH;
      // else ABORT, PASSCOMB, IDLE same meaning
    }
    else
    {
      ValidationFareComponents singleValidationFC;
      singleValidationFC.resize(1);
      singleValidationFC[0].initialize(
          negMatchedVfc->_currentFareUsage, pCat10, negMatchedVfc->_targetFareUsage);

      ret = processEndOnEndRestriction(
          diag, farePath, pu, pCat10Seg.itemNo(), singleValidationFC, directionalityInfo);
      if (ret != ABORT)
      {
        if (singleValidationFC[0].getSubCat(M104) == PASSCOMB)
        {
          negMatchedVfc->getSubCat(M104) = FAILCOMB;
          ret = FAILCOMB;
        }
        else
        {
          if (ret != STOPCOMB)
            ret = NO_MATCH;
          else
            negMatchedVfc->getSubCat(M104) = FAILCOMB;
        }
      }
    }

    break;
  }

  //---------------------------------//
  //  105 - Add-on                   //
  //---------------------------------//
  case ADD_ON:
  {
    ret = PASSCOMB;
    break;
  }

  //-----------//
  //  Default  //
  //-----------//
  default:
  {
    ret = NO_MATCH;
    LOG4CXX_INFO(logger,
                 __LINE__ << ", : IN Combinations,  process MajorSubCat FAILED, FarePath-Level");
    break;
  }
  } // switch

  return (ret);
}

// ----------------------------------------------------------------------------
char
Combinations::processOpenJawRestriction(DiagCollector& diag,
                                        const PricingUnit& pu,
                                        FareUsage& curFu,
                                        const CombinabilityRuleInfo* pCat10,
                                        const uint32_t itemNo,
                                        ValidationFareComponents& validationFareComponent)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processOpenJawRestriction ()");
  char ret = PASSCOMB;
  const OpenJawRule* pOpenJaws;

  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "processOpenJawRestriction: NO REC 2 CAT 10");
    return ABORT;
  }
  if (UNLIKELY(validationFareComponent[0]._passMajor))
  {
    if (diag.isActive())
      diag << "   PASSED - PRIOR RULE PASSED - " << std::endl;
    validationFareComponent[0].getSubCat(M101) = PASSCOMB;
    return ret;
  }
  else
  {
    // Assume it will pass until it fails some test
    validationFareComponent[0].getSubCat(M101) = PASSCOMB;
  }

  const Fare* pFare = curFu.paxTypeFare()->fare();

  pOpenJaws = _trx->dataHandle().getOpenJawRule(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(pOpenJaws == nullptr))
  {
    displayDiag(diag, 61, curFu, itemNo);

    validationFareComponent[0].getSubCat(M101) = FAILCOMB;
    return FAILCOMB;
  }

  DiagCollectorGuard dcg1(diag, Diagnostic631);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << " ------" << std::endl << " VENDOR " << pCat10->vendorCode()
         << " - 101 ITEM NO " << itemNo << " - FARE "
         << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;

    diag << "  UNAVAILABLE DATA TAG - " << pOpenJaws->unavailtag();
  }

  if (UNLIKELY(pOpenJaws->unavailtag() == UNAVAILABLE_DATA))
  {
    if (diag.isActive())
      diag << " - FAILED" << std::endl;
    displayDiag(diag, 60, curFu, itemNo);
    validationFareComponent[0].getSubCat(M101) = FAILCOMB;
    return FAILCOMB;
  }
  else if (pOpenJaws->unavailtag() == TEXT_DATA_ONLY)
  {
    size_t numStop = 0;
    const std::vector<FareUsage*>& puFareUsage = pu.fareUsage();

    if (puFareUsage.size() > 2)
    {
      numStop = std::count_if(puFareUsage.begin(),
                              puFareUsage.end(),
                              [](const FareUsage* fu)
                              { return fu->isFareBreakHasStopOver(); });

      if (numStop > 1)
      {
        if (diag.isActive())
          diag << " - FAILED " << numStop << " STOPS" << std::endl;
        displayDiag(diag, 71, curFu, itemNo);
        validationFareComponent[0].getSubCat(M101) = FAILCOMB;
        return FAILCOMB;
      }
    }

    diag << std::endl;
    validationFareComponent[0].getSubCat(M101) = PASSCOMB;
    return ret;
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  APPLICATION TAG - " << pOpenJaws->ojApplInd();
  }

  const PricingUnit::PUSubType puSubType = pu.puSubType();

  if ((pOpenJaws->ojApplInd() == DOUBLE_OPEN_JAW &&
       (puSubType == PricingUnit::ORIG_OPENJAW || puSubType == PricingUnit::DEST_OPENJAW)) ||
      (pOpenJaws->ojApplInd() == SINGLE_OPEN_JAW && puSubType == PricingUnit::DOUBLE_OPENJAW))
  {
    if (diag.isActive())
      diag << " - FAILED" << std::endl;
    displayDiag(diag, 71, curFu, itemNo);
    validationFareComponent[0].getSubCat(M101) = MAJOR_NO_MATCH;
    return FAILCOMB;
  }

  if (UNLIKELY(diag.isActive()))
    diag << std::endl;

  if (pOpenJaws->overrideDateTblItemNo() != 0)
  {
    LOG4CXX_INFO(logger, " pOpenJaws->overrideDateTblItemNo() != 0 ");

    DateTime travelDate = DateTime::openDate();

    RuleUtil::getTvlDateForTbl994Validation(
        travelDate, RuleConst::COMBINABILITY_RULE, *curFu.paxTypeFare(), nullptr, &pu);
    DateTime bookingDate;
    RuleUtil::getLatestBookingDate(*_trx, bookingDate, *curFu.paxTypeFare());

    DiagCollectorGuard dcg2(diag, Diagnostic654);
    if (diag.isActive())
    {
      diag << " ------ " << std::endl << " VENDOR " << pFare->vendor() << " - 994 ITEM NO "
           << pOpenJaws->overrideDateTblItemNo() << " - SUBCAT 101 - FARE " << pFare->fareClass()
           << std::endl << "  DEPARTURE DATE: " << travelDate.dateToString(DDMMM, "") << std::endl
           << "  TICKETING DATE: " << _trx->getRequest()->ticketingDT().dateToString(DDMMM, "")
           << std::endl << "  BOOKING DATE  : " << bookingDate.dateToString(DDMMM, "") << std::endl;
    }
    if (RuleUtil::validateDateOverrideRuleItem(*_trx,
                                               pOpenJaws->overrideDateTblItemNo(),
                                               pFare->vendor(),
                                               travelDate,
                                               _trx->getRequest()->ticketingDT(),
                                               bookingDate,
                                               &diag,
                                               Diagnostic654))
    {
      diag.enable(Diagnostic654);
      if (diag.isActive())
        diag << "   MATCH - OVERRIDE DATE TABLE" << std::endl;

      diag.enable(Diagnostic631);
      if (diag.isActive())
        diag << "   PASSED - OVERRIDE DATE TABLE - " << pOpenJaws->overrideDateTblItemNo()
             << std::endl;
    }
    else
    {
      diag.enable(Diagnostic654);
      if (diag.isActive())
        diag << "   NO MATCH" << std::endl;

      diag.enable(Diagnostic631);
      if (diag.isActive())
        diag << "   FAILED - OVERRIDE DATE TABLE - " << pOpenJaws->overrideDateTblItemNo()
             << std::endl;

      displayDiag(diag, 71, curFu, itemNo);
      validationFareComponent[0].getSubCat(M101) = MAJOR_NO_MATCH;
      return FAILCOMB;
    }
  }

  diag.enable(Diagnostic631);
  if (UNLIKELY(diag.isActive()))
    diag << "  NO. FARE COMPONENTS - " << pOpenJaws->farecompInd();

  if (pOpenJaws->farecompInd() != UNLIMITED)
  {
    // More than 2 comp Intl OJ-PU Template is never built
    // therefore no need to check for Intl-PU

    const size_t fuCnt = pu.fareUsage().size();
    if (static_cast<int>(fuCnt) - (pOpenJaws->farecompInd() - '0') > 0)
    {
      if (diag.isActive())
        diag << " - FAILED - " << fuCnt << " FC" << std::endl;
      displayDiag(diag, 71, curFu, itemNo);
      validationFareComponent[0].getSubCat(M101) = FAILCOMB;
      return FAILCOMB;
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  NUMBER OF STOPS - " << pOpenJaws->stopoverCnt();
  }

  if (pOpenJaws->stopoverCnt() != NO_RESTRICTION_S)
  {
    const std::vector<FareUsage*>& puFareUsage = pu.fareUsage();
    size_t numStop = std::count_if(puFareUsage.begin(),
                                   puFareUsage.end(),
                                   [](const FareUsage* fu)
                                   { return fu->isFareBreakHasStopOver(); });

    if (UNLIKELY(pOpenJaws->stopoverCnt() == NO_STOP_PERMITTED && numStop > 0))
    {
      if (diag.isActive())
        diag << " - FAILED - " << numStop << " STOPS" << std::endl;
      displayDiag(diag, 71, curFu, itemNo);
      validationFareComponent[0].getSubCat(M101) = FAILCOMB;
      return FAILCOMB;
    }
    else if (static_cast<int>(numStop) - atoi(pOpenJaws->stopoverCnt().c_str()) > 0)
    {
      if (diag.isActive())
        diag << " - FAILED - " << numStop << " STOPS" << std::endl;
      displayDiag(diag, 71, curFu, itemNo);
      validationFareComponent[0].getSubCat(M101) = FAILCOMB;
      return FAILCOMB;
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  SAME CARRIER TAG - " << pOpenJaws->sameCarrierInd();
  }

  if (pOpenJaws->sameCarrierInd() == RESTRICTION_APPLIES)
    if (!sameCarrierCheck(diag, pu, curFu))
    {
      displayDiag(diag, 71, curFu, itemNo);
      validationFareComponent[0].getSubCat(M101) = FAILCOMB;
      return FAILCOMB;
    }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  OW/RT TAG - " << pOpenJaws->owrt();
  }

  if (UNLIKELY(pOpenJaws->owrt() != NO_APPLICATION))
  {
    const std::vector<FareUsage*>& puFareUsage = pu.fareUsage();
    size_t fuCnt = puFareUsage.size();
    for (size_t i = 0; i < fuCnt; ++i)
    {
      if (puFareUsage[i]->paxTypeFare()->owrt() != pOpenJaws->owrt())
      {
        if (diag.isActive())
        {
          diag << " - FAILED FARE OW/RT - " << puFareUsage[i]->paxTypeFare()->owrt() << std::endl;
        }
        displayDiag(diag, 71, curFu, itemNo);
        validationFareComponent[0].getSubCat(M101) = FAILCOMB;
        return FAILCOMB;
      }
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  VALIDITY CHECK TAG - " << pOpenJaws->sojvalidityInd();
  }

  if (pOpenJaws->sojvalidityInd() != NO_APPLICATION) // ' '
  {
    /******
    // per Carrie, to fix PL 13405
    if (   pOpenJaws->sojvalidityInd() == ANY_SOJ
    && puSubType == PricingUnit::DOUBLE_OPENJAW )
    {
    diag << " - FAILED DOJ" << std::endl;
    displayDiag(diag, 71, curFu, itemNo);
    validationFareComponent[0].getSubCat(M101) = FAILCOMB;
    return FAILCOMB;
    }
     ****/

    if (pOpenJaws->sojvalidityInd() == MILEAGE_LONGEST // 'K'
        ||
        pOpenJaws->sojvalidityInd() == MILEAGE_LONGEST_INTL_ONLY) // 'Y'
    {
      bool restrictDomesticSurf = (pOpenJaws->sojvalidityInd() == MILEAGE_LONGEST);

      if (!checkOJMileage(diag, pu, true, restrictDomesticSurf))
      {
        if (diag.isActive())
        {
          diag << " - FAILED MILEAGE CHECK" << std::endl;
          displayDiag(diag, 71, curFu, itemNo);
        }
        validationFareComponent[0].getSubCat(M101) = FAILCOMB;
        return FAILCOMB;
      }
    }
    else if (LIKELY(pOpenJaws->sojvalidityInd() == MILEAGE_SHORTEST // 'M'
                     ||
                     pOpenJaws->sojvalidityInd() == MILEAGE_SHORTEST_INTL_ONLY)) // 'N'
    {
      // If it is shopping IS process, the journey (direct route) itinerary is used.
      // The longerleg miles should be used to cover most of itineraries to be processed in MIP
      // later.
      bool longestMile = (_trx->getTrxType() == PricingTrx::IS_TRX) ? true : false;
      bool restrictDomesticSurf = (pOpenJaws->sojvalidityInd() == MILEAGE_SHORTEST);

      if (!checkOJMileage(diag, pu, longestMile, restrictDomesticSurf))
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " - FAILED MILEAGE CHECK" << std::endl;
          displayDiag(diag, 71, curFu, itemNo);
        }
        validationFareComponent[0].getSubCat(M101) = FAILCOMB;
        return FAILCOMB;
      }
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  HIGHEST RT TAG - " << pOpenJaws->highrtInd();
  }

  if (pOpenJaws->highrtInd() == MUST_BE_CHARGE)
    curFu.highRT() = true;

  if (UNLIKELY(diag.isActive()))
    diag << std::endl;

  if (UNLIKELY(pOpenJaws->halftransportInd() != NO_APPLICATION && diag.isActive()))
    diag << std::endl << "  HALF TRANSPORTATION TAG - " << pOpenJaws->halftransportInd()
         << " - APPL UNKNOWN AT THIS TIME" << std::endl;

  if (UNLIKELY(pOpenJaws->ojbackhaulInd() != NO_APPLICATION && diag.isActive()))
    diag << std::endl << "  OPEN JAW BACKHAUL TAG - " << pOpenJaws->ojbackhaulInd()
         << " - APPL UNKNOWN AT THIS TIME" << std::endl;

  return ret;
}

// ----------------------------------------------------------------------------
char
Combinations::processRoundTripRestriction(DiagCollector& diag,
                                          const PricingUnit& pu,
                                          FareUsage& curFu,
                                          const CombinabilityRuleInfo* pCat10,
                                          const uint32_t itemNo,
                                          ValidationFareComponents& validationFareComponent)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processRoundTripRestriction ()");
  char ret = PASSCOMB;
  const RoundTripRuleItem* pRoundTrip;

  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "processRoundTripRestriction: NO REC 2 CAT 10");
    return ABORT;
  }
  if (UNLIKELY(validationFareComponent[0]._passMajor))
  {
    if (diag.isActive())
      diag << "   PASSED - PRIOR RULE PASSED - " << std::endl;
    validationFareComponent[0].getSubCat(M102) = PASSCOMB;
    return ret;
  }
  else
  {
    // Assume it will pass until it fails some test
    validationFareComponent[0].getSubCat(M102) = PASSCOMB;
  }

  const Fare* pFare = curFu.paxTypeFare()->fare();

  pRoundTrip = _trx->dataHandle().getRoundTripRuleItem(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(pRoundTrip == nullptr))
  {
    displayDiag(diag, 62, curFu, itemNo);
    validationFareComponent[0].getSubCat(M102) = FAILCOMB;
    return FAILCOMB;
  }

  DiagCollectorGuard dcg1(diag, Diagnostic632);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << " ------" << std::endl << " VENDOR " << pCat10->vendorCode()
         << " - 102 ITEM NO " << itemNo << " - FARE "
         << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;

    diag << "  UNAVAILABLE DATA TAG - " << pRoundTrip->unavailTag();
  }
  if (UNLIKELY(pRoundTrip->unavailTag() == UNAVAILABLE_DATA))
  {
    if (diag.isActive())
      diag << " - FAILED" << std::endl;
    displayDiag(diag, 60, curFu, itemNo);
    validationFareComponent[0].getSubCat(M102) = FAILCOMB;
    return FAILCOMB;
  }
  else if (UNLIKELY(pRoundTrip->unavailTag() == TEXT_DATA_ONLY))
  {
    if (diag.isActive())
      diag << " - PASSED" << std::endl;
    validationFareComponent[0].getSubCat(M102) = PASSCOMB;
    return ret;
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  APPLICATION TAG - " << pRoundTrip->applInd();
  }

  if (pRoundTrip->applInd() == APPLICATION_NOT_PERMITTED)
  {
    if (diag.isActive())
      diag << " - FAILED" << std::endl;
    displayDiag(diag, 72, curFu, itemNo);
    validationFareComponent[0].getSubCat(M102) = FAILCOMB;
    return FAILCOMB;
  }

  if (UNLIKELY(diag.isActive()))
    diag << std::endl;

  if (pRoundTrip->overrideDateTblItemNo() != 0)
  {
    DateTime travelDate = DateTime::openDate();
    RuleUtil::getTvlDateForTbl994Validation(
        travelDate, RuleConst::COMBINABILITY_RULE, *curFu.paxTypeFare(), nullptr, &pu);

    DateTime bookingDate;
    RuleUtil::getLatestBookingDate(*_trx, bookingDate, *curFu.paxTypeFare());

    DiagCollectorGuard dcg2(diag, Diagnostic654);
    if (diag.isActive())
    {
      diag << " ------ " << std::endl << " VENDOR " << pFare->vendor() << " - 994 ITEM NO "
           << pRoundTrip->overrideDateTblItemNo() << " - SUBCAT 102 - FARE " << pFare->fareClass()
           << std::endl << "  DEPARTURE DATE: " << travelDate.dateToString(DDMMM, "") << std::endl
           << "  TICKETING DATE: " << _trx->getRequest()->ticketingDT().dateToString(DDMMM, "")
           << std::endl << "  BOOKING DATE  : " << bookingDate.dateToString(DDMMM, "") << std::endl;
    }
    if (RuleUtil::validateDateOverrideRuleItem(*_trx,
                                               pRoundTrip->overrideDateTblItemNo(),
                                               pFare->vendor(),
                                               travelDate,
                                               _trx->getRequest()->ticketingDT(),
                                               bookingDate,
                                               &diag,
                                               Diagnostic654))
    {
      diag.enable(Diagnostic654);
      if (diag.isActive())
        diag << "   MATCH - OVERRIDE DATE TABLE" << std::endl;

      diag.enable(Diagnostic632);
      if (diag.isActive())
      {
        diag << "   PASSED - OVERRIDE DATE TABLE - " << pRoundTrip->overrideDateTblItemNo()
             << std::endl;
      }
    }
    else
    {
      diag.enable(Diagnostic654);
      if (diag.isActive())
        diag << "   NO MATCH" << std::endl;

      diag.enable(Diagnostic632);
      if (diag.isActive())
      {
        diag << "   FAILED - OVERRIDE DATE TABLE - " << pRoundTrip->overrideDateTblItemNo()
             << std::endl;
      }
      displayDiag(diag, 72, curFu, itemNo);
      validationFareComponent[0].getSubCat(M102) = MAJOR_NO_MATCH;
      return FAILCOMB;
    }
  }

  diag.enable(Diagnostic632);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  SAME CARRIER TAG - " << pRoundTrip->sameCarrierInd();
  }

  if (pRoundTrip->sameCarrierInd() == RESTRICTION_APPLIES)
  {
    if (!sameCarrierCheck(diag, pu, curFu))
    {
      displayDiag(diag, 72, curFu, itemNo);
      validationFareComponent[0].getSubCat(M102) = FAILCOMB;
      return FAILCOMB;
    }
  }

  if (UNLIKELY(diag.isActive()))
    diag << std::endl;

  if (pRoundTrip->atwInd() != NO_APPLICATION)
  {
    bool failOnRTW = false;
    if (pRoundTrip->atwInd() == APPLICATION_NOT_PERMITTED)
    {
      failOnRTW = LocUtil::isRtw(pu);
    }

    if (failOnRTW)
    {
      if (diag.isActive())
      {
        diag << "  ATW INDICATOR - " << pRoundTrip->atwInd() << " - FAILED" << std::endl;
      }
      validationFareComponent[0].getSubCat(M102) = FAILCOMB;
      return FAILCOMB;
    }
    else
    {
      if (diag.isActive())
      {
        diag << "  ATW INDICATOR - " << pRoundTrip->atwInd() << " - PASSED" << std::endl;
      }
    }
  }

  if (LIKELY(PricingUtil::allowHRTCForVendor(*_trx, curFu.paxTypeFare()) ||
              TrxUtil::isAtpcoTTProjectHighRTEnabled(*_trx)))
  {
    if (UNLIKELY(pRoundTrip->highrtInd() == MUST_BE_CHARGE))
    {
      curFu.highRT() = true;
    }
  }

  return ret;
}

// ----------------------------------------------------------------------------
char
Combinations::processCircleTripRestriction(DiagCollector& diag,
                                           const PricingUnit& pu,
                                           const FareUsage& curFu,
                                           const CombinabilityRuleInfo* pCat10,
                                           const uint32_t itemNo,
                                           ValidationFareComponents& validationFareComponent)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processCircleTripRestriction ()");
  char ret = PASSCOMB;
  const CircleTripRuleItem* pCircleTrip;

  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "processCircleTripRestriction: NO REC 2 CAT 10");
    return ABORT;
  }
  if (UNLIKELY(validationFareComponent[0]._passMajor))
  {
    if (diag.isActive())
      diag << "   PASSED - PRIOR RULE PASSED - " << std::endl;
    validationFareComponent[0].getSubCat(M103) = PASSCOMB;
    return ret;
  }
  else
  {
    // Assume it will pass until it fails some test
    validationFareComponent[0].getSubCat(M103) = PASSCOMB;
  }
  const Fare* pFare = curFu.paxTypeFare()->fare();

  pCircleTrip = _trx->dataHandle().getCircleTripRuleItem(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(pCircleTrip == nullptr))
  {
    displayDiag(diag, 63, curFu, itemNo);
    validationFareComponent[0].getSubCat(M103) = FAILCOMB;
    return FAILCOMB;
  }

  DiagCollectorGuard dcg1(diag, Diagnostic633);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << " ------" << std::endl << " VENDOR " << pCat10->vendorCode()
         << " - 103 ITEM NO " << itemNo << " - FARE "
         << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;

    diag << "  UNAVAILABLE DATA TAG - " << pCircleTrip->unavailtag();
  }
  if (UNLIKELY(pCircleTrip->unavailtag() == UNAVAILABLE_DATA))
  {
    if (diag.isActive())
      diag << " - FAILED" << std::endl;
    displayDiag(diag, 60, curFu, itemNo);
    validationFareComponent[0].getSubCat(M103) = FAILCOMB;
    return FAILCOMB;
  }
  else if (UNLIKELY(pCircleTrip->unavailtag() == TEXT_DATA_ONLY))
  {
    if (diag.isActive())
      diag << " - PASSED" << std::endl;
    validationFareComponent[0].getSubCat(M103) = PASSCOMB;
    return ret;
  }

  if (UNLIKELY(diag.isActive()))
    diag << std::endl;

  if (pCircleTrip->overrideDateTblItemNo() != 0)
  {
    DateTime travelDate = DateTime::openDate();
    RuleUtil::getTvlDateForTbl994Validation(
        travelDate, RuleConst::COMBINABILITY_RULE, *curFu.paxTypeFare(), nullptr, &pu);
    //        DateTime travelDate =
    // curFu.paxTypeFare()->fareMarket()->travelSeg().front()->departureDT();
    DateTime bookingDate;
    RuleUtil::getLatestBookingDate(*_trx, bookingDate, *curFu.paxTypeFare());

    DiagCollectorGuard dcg2(diag, Diagnostic654);
    if (diag.isActive())
    {
      diag << " ------ " << std::endl << " VENDOR " << pFare->vendor() << " - 994 ITEM NO "
           << pCircleTrip->overrideDateTblItemNo() << " - SUBCAT 103 - FARE " << pFare->fareClass()
           << std::endl << "  DEPARTURE DATE: " << travelDate.dateToString(DDMMM, "") << std::endl
           << "  TICKETING DATE: " << _trx->getRequest()->ticketingDT().dateToString(DDMMM, "")
           << std::endl << "  BOOKING DATE  : " << bookingDate.dateToString(DDMMM, "") << std::endl;
    }
    if (RuleUtil::validateDateOverrideRuleItem(*_trx,
                                               pCircleTrip->overrideDateTblItemNo(),
                                               pFare->vendor(),
                                               travelDate,
                                               _trx->getRequest()->ticketingDT(),
                                               bookingDate,
                                               &diag,
                                               Diagnostic654))
    {
      diag.enable(Diagnostic654);
      if (diag.isActive())
        diag << "   MATCH - OVERRIDE DATE TABLE" << std::endl;

      diag.enable(Diagnostic633);
      if (diag.isActive())
      {
        diag << "   PASSED - OVERRIDE DATE TABLE - " << pCircleTrip->overrideDateTblItemNo()
             << std::endl;
      }
    }
    else
    {
      diag.enable(Diagnostic654);
      if (diag.isActive())
        diag << "   NO MATCH" << std::endl;

      diag.enable(Diagnostic633);
      if (diag.isActive())
      {
        diag << "   FAILED - OVERRIDE DATE TABLE - " << pCircleTrip->overrideDateTblItemNo()
             << std::endl;
      }
      displayDiag(diag, 73, curFu, itemNo);
      validationFareComponent[0].getSubCat(M103) = MAJOR_NO_MATCH;
      return FAILCOMB;
    }
  }

  diag.enable(Diagnostic633);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  SAME CARRIER TAG - " << pCircleTrip->sameCarrierInd();
  }

  if (pCircleTrip->sameCarrierInd() == RESTRICTION_APPLIES)
  {
    if (!sameCarrierCheck(diag, pu, curFu))
    {
      displayDiag(diag, 73, curFu, itemNo);
      validationFareComponent[0].getSubCat(M103) = FAILCOMB;
      return FAILCOMB;
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  NUMBER OF BREAK POINTS - " << pCircleTrip->breakPoints();
  }

  if (pCircleTrip->breakPoints() != NO_APPLICATION)
  {
    int16_t numBreakIntl = 0;
    const std::vector<FareUsage*>& puFareUsage = pu.fareUsage();
    size_t fuCnt = puFareUsage.size();
    size_t numBreak = fuCnt - 1;

    for (size_t i = 0; i < fuCnt; ++i)
    {
      if (puFareUsage[i]->paxTypeFare()->isInternational())
        ++numBreakIntl;
    }

    if (pCircleTrip->breakPoints() == '1' && (numBreakIntl > 2 || numBreak > 1))
    {
      if (diag.isActive())
      {
        diag << " - FAILED - " << numBreak << " FARE BREAKS" << std::endl
             << "                                        " << numBreakIntl
             << " INTL FARE COMPONENTS" << std::endl;
      }
      displayDiag(diag, 73, curFu, itemNo);
      validationFareComponent[0].getSubCat(M103) = FAILCOMB;
      return FAILCOMB;
    }
    else if (pCircleTrip->breakPoints() - '0' - static_cast<int>(numBreak) < 0)
    {
      if (diag.isActive())
        diag << " - FAILED - " << numBreak << " FARE BREAKS" << std::endl;
      displayDiag(diag, 73, curFu, itemNo);
      validationFareComponent[0].getSubCat(M103) = FAILCOMB;
      return FAILCOMB;
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  NUMBER OF STOPS - " << pCircleTrip->stopoverCnt();
  }

  if (pCircleTrip->stopoverCnt() != NO_RESTRICTION_S)
  {
    const std::vector<FareUsage*>& puFareUsage = pu.fareUsage();
    size_t numStop = std::count_if(puFareUsage.begin(),
                                   puFareUsage.end(),
                                   [](const FareUsage* fu)
                                   { return fu->isFareBreakHasStopOver(); });

    if (UNLIKELY(pCircleTrip->stopoverCnt() == NO_STOP_PERMITTED && numStop > 0))
    {
      if (diag.isActive())
        diag << " - FAILED - " << numStop << " STOPS" << std::endl;
      displayDiag(diag, 73, curFu, itemNo);
      validationFareComponent[0].getSubCat(M103) = FAILCOMB;
      return FAILCOMB;
    }
    else if (static_cast<int>(numStop) - atoi(pCircleTrip->stopoverCnt().c_str()) > 0)
    {
      if (diag.isActive())
        diag << " - FAILED - " << numStop << " STOPS" << std::endl;
      displayDiag(diag, 73, curFu, itemNo);
      validationFareComponent[0].getSubCat(M103) = FAILCOMB;
      return FAILCOMB;
    }
  }

  diag << std::endl;

  if (pCircleTrip->atwInd() != NO_APPLICATION)
  {
    bool failOnRTW = false;
    if (LIKELY(pCircleTrip->atwInd() == APPLICATION_NOT_PERMITTED))
    {
      failOnRTW = LocUtil::isRtw(pu);
    }

    if (failOnRTW)
    {
      if (diag.isActive())
      {
        diag << "  ATW INDICATOR - " << pCircleTrip->atwInd() << " - FAILED" << std::endl;
      }
      validationFareComponent[0].getSubCat(M103) = FAILCOMB;
      return FAILCOMB;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "  ATW INDICATOR - " << pCircleTrip->atwInd() << " - PASSED" << std::endl;
      }
    }
  }

  return ret;
}

// ----------------------------------------------------------------------------
bool
Combinations::validateInternationalTravel(DiagCollector& diag,
                                          const FarePath& farePath,
                                          ValidationFareComponents& validationFareComponent,
                                          const EndOnEnd* pEndOnEnd)
{
  //--- eoe list ----
  size_t numOfFU = validationFareComponent.size();

  // IF there is no validation to perform -> pass it!
  if (numOfFU == 1 &&
      (validationFareComponent[0]._currentFareUsage == validationFareComponent[0]._targetFareUsage))
  {
    return true;
  }
  // else if there is no international travel in the entire itnerary -> fail it!
  else if (farePath.itin()->geoTravelType() != GeoTravelType::International)
  {
    diag.enable(Diagnostic614, Diagnostic634);
    if (diag.isActive())
    {
      diag << "   INTERNATIONAL IND - " << pEndOnEnd->intlInd() << "  ITIN IS NOT - FAIL EOE\n";
    }
    validationFareComponent[0].getSubCat(M104) = FAILCOMB;
    return false;
  }

  return true;
}

char
Combinations::processEndOnEndRestriction(EndOnEndDataAccess& eoeDA)
{
  diagEndOnEndFareInfo(eoeDA);

  char rtn = processUnavailTag(eoeDA);
  if (UNLIKELY(rtn != PASSCOMB))
    return rtn;

  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  if (eoeDA._endOnEndRule.intlInd() == RESTRICTIONS &&
      !validateInternationalTravel(
          diag, eoeDA._farePath, validationFareComponent, &eoeDA._endOnEndRule))
  {
    return STOPCOMB;
  }

  //--- eoe list ----
  size_t numOfFU = validationFareComponent.size();

  for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)
  {
    if (validationFareComponent[fuCount]._currentFareUsage ==
        validationFareComponent[fuCount]._targetFareUsage)
      continue;

    if (!validationFareComponent.needAllPassSameMajorItem())
    {
      if (!validationFareComponent[fuCount]._passMinor)
        continue;
    }

    diagEndOnEndTargetFare(eoeDA, fuCount);

    if (validationFareComponent[fuCount]._passMajor)
    {
      diagEndOnEndTargetFUPriorPassed(eoeDA);
      validationFareComponent[fuCount].getSubCat(M104) = PASSCOMB;
      continue;
    }
    else
    {
      // Assume it will pass until it fails some test
      validationFareComponent[fuCount].getSubCat(M104) = PASSCOMB;
    }

    //----- check override date table -------------
    if (NO_MATCH == processEndOnEndTbl994(eoeDA, fuCount))
      continue;

    //-----  validate A-B-A -----
    rtn = processEndOnEndABA(eoeDA, fuCount);
    if (UNLIKELY(rtn == ABORT))
      break;
    else if (rtn == NO_MATCH)
      continue;

    diag.enable(Diagnostic634);
    //----- if same carrier required --------------
    // When same carrier tag is set in 101-104, it excludes YY fares.
    // only the published carrier fare is considered.

    rtn = processEndOnEndSameCxr(eoeDA, fuCount);
    if (rtn == FAILCOMB)
      continue;
    else if (UNLIKELY(rtn != PASSCOMB))
      return rtn;

    //----- check restriction tag -----------------
    rtn = processEndOnEndRestInd(eoeDA, fuCount);
    if (UNLIKELY(rtn == FAILCOMB))
      continue;
    else if (UNLIKELY(rtn != PASSCOMB))
      return rtn;

    //----- move check GEO SPEC and others in front of
    // eoeNormalInd, etc Ind check, so that we can STOP process
    // 104 when we have hit a NOT_PERMITTED condition
    if (eoeDA._endOnEndRule.fareTypeLocAppl() != NO_APPLICATION)
    {
      char rtn = checkGeoSpec(eoeDA, fuCount);
      if (rtn == FAILCOMB)
        continue;
      else if (UNLIKELY(rtn != PASSCOMB))
        return rtn;
    }

    //----- check CONST POINT ---------------------
    if (eoeDA._endOnEndRule.constLocAppl() != NO_APPLICATION)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "         - CONST LOC APPL - " << eoeDA._endOnEndRule.constLocAppl() << std::endl;
      }
    }

    if (UNLIKELY(PASSCOMB != processEndOnEndCxrPreference(eoeDA, fuCount)))
    {
      continue;
    }

    //----- check normal/special fare ---------------------
    if (eoeDA._endOnEndRule.eoeNormalInd() == NO_APPLICATION &&
        eoeDA._endOnEndRule.eoespecialInd() == NO_APPLICATION)
    {
      diagEndOnEndPassBlankNmlSpclInd(
          eoeDA, *validationFareComponent[fuCount]._targetFareUsage->paxTypeFare());
    }
    else
    {
      char rtn = PASSCOMB;
      //----- if normal fare -----
      if (validationFareComponent[fuCount]._targetFareUsage->isPaxTypeFareNormal())
      {
        rtn = processEndOnEndNormalFare(eoeDA, fuCount);
      }
      //----- if special fare -----
      else if (LIKELY(
                   validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->isSpecial()))
      {
        rtn = processEndOnEndSpecialFare(eoeDA, fuCount);
      }
      if (rtn == FAILCOMB)
        continue;
      else if (rtn != PASSCOMB)
        return rtn;
    }

    //----- check US/CAN, US Domestic, International-----------
    rtn = processEndOnEndDomIntlTransbFare(eoeDA, fuCount);
    if (rtn == FAILCOMB)
      continue;
    else if (rtn != PASSCOMB)
      return rtn;

    //----- check ticket tag ----------------------
    rtn = processEndOnEndTktInd(eoeDA, fuCount);
    if (UNLIKELY(rtn != PASSCOMB))
      return rtn;

    // final result for this target FU
    if (UNLIKELY(eoeDA._diag614IsActive))
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  PASSED" << std::endl;
      }
    }
  } // fuCount

  if (_eoePresentFareTypes.isRequiredFareMissing(eoeDA._endOnEndRule))
  {
    if (diag.isActive())
      diag << "     MISS EOE REQUIRED FARE\n";

    return STOPCOMB;
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processEndOnEndRestriction(DiagCollector& diag,
                                         const FarePath& farePath,
                                         const PricingUnit& pu,
                                         const uint32_t itemNo,
                                         ValidationFareComponents& validationFareComponent,
                                         const DirectionalityInfo& directionalityInfo)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processEndOnEndRestriction ()");

  const CombinabilityRuleInfo* pCat10 = validationFareComponent[0]._pCat10;
  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "process EndOnEndRestriction: NO REC 2 CAT 10");
    return ABORT;
  }

  const EndOnEnd* pEndOnEnd = _trx->dataHandle().getEndOnEnd(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(pEndOnEnd == nullptr))
  {
    displayDiag(diag, 64, *(validationFareComponent[0]._currentFareUsage), itemNo);
    validationFareComponent[0].getSubCat(M104) = FAILCOMB;
    return FAILCOMB;
  }

  // with fix and refactorred this ~1200 lines function
  EndOnEndDataAccess eoeDA(
      farePath, pu, *pCat10, itemNo, validationFareComponent, *pEndOnEnd, diag, directionalityInfo);

  return processEndOnEndRestriction(eoeDA);
}

// ----------------------------------------------------------------------------
bool
Combinations::sameCarrierCheck(DiagCollector& diag, const PricingUnit& pu, const FareUsage& curFu)
{
  const CombinabilityRuleInfo* pCat10 = curFu.rec2Cat10();
  if (UNLIKELY(!pCat10))
  {
    LOG4CXX_INFO(logger, "sameCarrier Check: NO REC 2 CAT 10");
    return false;
  }

  std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuItEnd = pu.fareUsage().end();
  for (; fuIt != fuItEnd; ++fuIt)
  {
    if ((*fuIt) == &curFu)
      continue;

    const FareUsage& targetFu = **fuIt;
    if (!sameCarrierCheck(diag, curFu, targetFu, pCat10))
      return false;
  }

  return true;
}
// ----------------------------------------------------------------------------
bool
Combinations::sameCarrierCheck(DiagCollector& diag,
                               const FareUsage& curFu,
                               const FareUsage& targetFu,
                               const CombinabilityRuleInfo* pCat10)
{
  const CombinabilityRuleInfo* pCat10target = targetFu.rec2Cat10();
  if (UNLIKELY(!pCat10target))
  {
    LOG4CXX_INFO(logger, "sameCarrier Check: NO REC 2 CAT 10");
    return false;
  }

  if (pCat10->carrierCode() != JOINT_CARRIER && pCat10target->carrierCode() != JOINT_CARRIER &&
      pCat10target->carrierCode() != pCat10->carrierCode())
  {
    if (UNLIKELY(diag.isActive()))
      diag << " - FAILED CARRIER : " << pCat10target->carrierCode() << std::endl;
    return false;
  }
  else if (UNLIKELY(pCat10->carrierCode() == JOINT_CARRIER ||
                     pCat10target->carrierCode() == JOINT_CARRIER))
  {
    // Currently out of ATSEI scope
    //@TODO for Alaska, Hawaii, PR and VI use overwater carrier

    if (curFu.paxTypeFare()->fareMarket()->governingCarrier() !=
        targetFu.paxTypeFare()->fareMarket()->governingCarrier())
    {
      if (diag.isActive())
      {
        diag << " - FAILED CARRIER : " << targetFu.paxTypeFare()->fareMarket()->governingCarrier()
             << std::endl;
      }
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
bool
Combinations::processMinorSubCat(DiagCollector& diag,
                                 FarePath* farePath,
                                 const PricingUnit& currentPU,
                                 FareUsage& currentFU,
                                 const CombinabilityRuleInfo* pCat10RuleInfo,
                                 const CombinabilityRuleItemInfo& pCat10Seg,
                                 bool& negative,
                                 ValidationFareComponents& validationFareComponent,
                                 Itin* itin)
{
  bool ret = false;

  if (UNLIKELY(!pCat10RuleInfo))
  {
    LOG4CXX_INFO(logger, "process MinorSubCat: NO REC 2 CAT 10");
    return false;
  }

  switch (pCat10Seg.itemcat())
  {
  //------------------------------//
  //  106 - Carrier Restrictions  //
  //------------------------------//
  case CARRIER:
  {
    CombinationsSubCat106 subCat106(*_trx,
                                    diag,
                                    pCat10RuleInfo->vendorCode(),
                                    pCat10Seg.itemNo(),
                                    currentPU,
                                    currentFU,
                                    validationFareComponent,
                                    negative);
    ret = subCat106.match();

    if (LIKELY(ret))
      ret = validationFareComponent.evaluate(m106);
    break;
  }

  //----------------------------------//
  //  107 - Rule/Tariff Restrictions  //
  //----------------------------------//
  case RULE_TARIFF:
  {
    ret = (processTariffRuleRestriction(diag,
                                        currentFU,
                                        pCat10RuleInfo,
                                        pCat10Seg.itemNo(),
                                        negative,
                                        validationFareComponent,
                                        currentPU) != ABORT);
    if (LIKELY(ret))
      ret = validationFareComponent.evaluate(m107);
    break;
  }

  //--------------------------------------//
  //  108 - Fare Class/Type Restrictions  //
  //--------------------------------------//
  case FARE_CLASS:
  {
    ret = (processFareClassTypeRestriction(diag,
                                           currentFU,
                                           pCat10RuleInfo,
                                           pCat10Seg.itemNo(),
                                           negative,
                                           validationFareComponent,
                                           currentPU) != ABORT);
    if (LIKELY(ret))
      ret = validationFareComponent.evaluate(m108);
    break;
  }

  //-----------------------------------//
  //  109 - Open Jaw Set Restrictions  //
  //-----------------------------------//
  case OPEN_JAW_SET:
  {
    CombinationsSubCat109 subCat109(*_trx,
                                    diag,
                                    pCat10RuleInfo->vendorCode(),
                                    pCat10Seg.itemNo(),
                                    currentPU,
                                    currentFU,
                                    validationFareComponent,
                                    negative);
    ret = subCat109.match();

    if (LIKELY(ret))
      ret = validationFareComponent.evaluate(m109);
    break;
  }

  case FLIGHT_APPL_RULE:
  {
    ret = (processFlightRestriction(
               diag, currentFU, pCat10RuleInfo, pCat10Seg.itemNo(), validationFareComponent) !=
           ABORT);
    if (LIKELY(ret))
      ret = validationFareComponent.evaluate(m004);
    break;
  }

  case ELIGIBILITY_RULE:
  {
    ret = (processEligibility(diag,
                              farePath,
                              currentPU,
                              currentFU,
                              *pCat10RuleInfo,
                              pCat10Seg,
                              pCat10Seg.itemNo(),
                              validationFareComponent) != ABORT);
    if (ret)
      ret = validationFareComponent.evaluate(m001);
    break;
  }

  case MINIMUM_STAY_RULE:
  {
    ret = (processMinimumStay(diag,
                              farePath,
                              currentPU,
                              currentFU,
                              *pCat10RuleInfo,
                              pCat10Seg,
                              pCat10Seg.itemNo(),
                              validationFareComponent,
                              itin) != ABORT);
    if (ret)
      ret = validationFareComponent.evaluate(m006);
    break;
  }

  case MAXIMUM_STAY_RULE:
  {
    ret = (processMaximumStay(diag,
                              farePath,
                              currentPU,
                              currentFU,
                              *pCat10RuleInfo,
                              pCat10Seg,
                              pCat10Seg.itemNo(),
                              validationFareComponent,
                              itin) != ABORT);
    if (ret)
      ret = validationFareComponent.evaluate(m007);
    break;
  }

  case TRAVEL_RESTRICTION_RULE:
  {
    // This is new code to handle CAT14. The else would return as same as going to DEFAULT case
    ret = (processTravelRestriction(diag,
                                    farePath,
                                    currentPU,
                                    currentFU,
                                    *pCat10RuleInfo,
                                    pCat10Seg,
                                    pCat10Seg.itemNo(),
                                    validationFareComponent,
                                    itin) != ABORT);
    if (ret)
      ret = validationFareComponent.evaluate(m014);
    break;
  }

  case SALES_RESTRICTIONS_RULE:
  {
    ret = (processSalesRestrictions(diag,
                                    farePath,
                                    currentPU,
                                    currentFU,
                                    *pCat10RuleInfo,
                                    pCat10Seg,
                                    pCat10Seg.itemNo(),
                                    validationFareComponent,
                                    itin) != ABORT);
    if (LIKELY(ret))
      ret = validationFareComponent.evaluate(m015);
    break;
  }

  //-----------//
  //  Default  //
  //-----------//
  default:
  {
    //----- data error -----
    // displayDiag(diag, farePath, 50);
    return false;
  }
  } // switch

  return ret;
}

char
Combinations::processTariffRuleRestriction(DiagCollector& diag,
                                           const FareUsage& curFu,
                                           const CombinabilityRuleInfo* pCat10,
                                           const uint32_t itemNo,
                                           bool& negative,
                                           ValidationFareComponents& validationFareComponent,
                                           const PricingUnit& pu)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processTariffRuleRestriction ()");

  std::vector<TariffRuleRest*>::const_iterator iter;
  std::vector<TariffRuleRest*>::const_iterator iterEnd;

  const std::vector<TariffRuleRest*>& tariffRuleRestVector =
      _trx->dataHandle().getTariffRuleRest(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(tariffRuleRestVector.empty()))
  {
    LOG4CXX_INFO(logger, " tariffRuleRestList.empty().......");
    displayDiag(diag, 67, curFu, itemNo);
    return ABORT;
  }

  DiagCollectorGuard dcg1(diag, Diagnostic637);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  ------" << std::endl << "  VENDOR " << pCat10->vendorCode()
         << " - 107 ITEM NO " << itemNo << " - FARE "
         << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
  }

  // IF there is only one vendor or only one carrier there is no need to validate carrier
  //    preference for vendors;
  bool mustVerifyCarrierPreference =
      (!validationFareComponent.hasOneCarrier() || !validationFareComponent.hasOneVendor());

  size_t numOfFU = validationFareComponent.size();

  for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)
  {
    if (!validationFareComponent.needAllPassSameMajorItem())
    {
      validationFareComponent[fuCount]._passMinor = false;
      if (validationFareComponent[fuCount]._passMajor)
        continue;
    }
    else if (UNLIKELY(validationFareComponent[fuCount]._passMinor))
    {
      if (diag.isActive())
      {
        diag << " PASSED MINOR ";
        if (validationFareComponent[fuCount].getSubCat(m107) == ValidationElement::MATCHED)
          diag << "MATCH107\n";
        else
          diag << "NOT MATCH107\n";
      }

      continue;
    }

    if (validationFareComponent[fuCount]._targetFareUsage ==
        validationFareComponent[fuCount]._currentFareUsage)
    {
      continue;
    }

    if (UNLIKELY(!validationFareComponent[fuCount]._targetFareUsage->rec2Cat10()))
    {
      LOG4CXX_INFO(logger, " ** NO Rec 2 Cat 10 **");
      return ABORT;
    }

    PaxTypeFare* currentFare = validationFareComponent[fuCount]._currentFareUsage->paxTypeFare();
    PaxTypeFare* targetFare = validationFareComponent[fuCount]._targetFareUsage->paxTypeFare();

    targetFare = validateFareCat10overrideCat25(currentFare, targetFare, &pu);

    if (UNLIKELY(diag.isActive()))
    {
      diag << "  COMBINING WITH FARE " << targetFare->createFareBasis(nullptr) << std::endl;
    }

    iter = tariffRuleRestVector.begin();
    iterEnd = tariffRuleRestVector.end();

    int numDefaulted = 0;
    bool tariffRuleInList = false;

    for (; iter != iterEnd; ++iter)
    {
      TariffRuleRest* pTariffRuleRestrictions = *iter;
      if (UNLIKELY(diag.isActive()))
      {
        diag << "   ---------------" << std::endl << "   APPLICATION TAG - "
             << pTariffRuleRestrictions->trfRuleApplInd() << std::endl;
      }

      if (pTariffRuleRestrictions->trfRuleApplInd() == NOT_ALLOWED)
      {
        negative = true;
      }

      if (UNLIKELY(diag.isActive()))
      {
        std::string gd;
        globalDirectionToStr(gd, pTariffRuleRestrictions->globalDirection());
        diag << "   SAME TARIFF/RULE IND - " << pTariffRuleRestrictions->sametrfRuleInd()
             << std::endl << "   TARIFF NUMBER - " << pTariffRuleRestrictions->ruleTariff()
             << std::endl << "   RULE NUMBER - " << pTariffRuleRestrictions->rule() << std::endl
             << "   GLOBAL DIRECTION - " << gd << std::endl;
      }

      if ((pTariffRuleRestrictions->rule() == targetFare->ruleNumber()) &&
          (pTariffRuleRestrictions->ruleTariff() == targetFare->tcrRuleTariff()))
      {
        tariffRuleInList = true;
      }

      if (pTariffRuleRestrictions->sametrfRuleInd() != NO_APPLICATION)
      {
        // Note: Same Rule requires same Tariff
        if (pTariffRuleRestrictions->sametrfRuleInd() == SAME_TARIFF ||
            pTariffRuleRestrictions->sametrfRuleInd() == SAME_RULE)
        {
          if (currentFare->vendor() != targetFare->vendor())
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          }
          else if (currentFare->tcrRuleTariff() != targetFare->tcrRuleTariff())
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          }
          else if (pTariffRuleRestrictions->sametrfRuleInd() == SAME_RULE)
          {
            if (currentFare->ruleNumber() == targetFare->ruleNumber())
            {
              validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
            }
            else
            {
              validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
            }
          }
          else
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
          }
        }
        else if (pTariffRuleRestrictions->sametrfRuleInd() == ANY_PUBLIC_TARIFF)
        {
          bool targetPrivate = (targetFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF);
          if (targetPrivate)
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          }
          else
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
          }
        }
        else if (LIKELY(pTariffRuleRestrictions->sametrfRuleInd() == ANY_PRIVATE_TARIFF))
        {
          bool targetPrivate = (targetFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF);
          if (!targetPrivate)
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          }
          else
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
          }
        }
        //--- data error ---
        else
        {
          displayDiag(diag, 50);
          return ABORT;
          break;
        }
      }
      else if (pTariffRuleRestrictions->ruleTariff() == ANY_TARIFF ||
               targetFare->tcrRuleTariff() == pTariffRuleRestrictions->ruleTariff())
      {
        if (pTariffRuleRestrictions->rule() == ANY_RULE ||
            targetFare->ruleNumber() == pTariffRuleRestrictions->rule())
        {
          if (pTariffRuleRestrictions->globalDirection() == ' ' ||
              pTariffRuleRestrictions->globalDirection() == GlobalDirection::ZZ ||
              targetFare->globalDirection() == pTariffRuleRestrictions->globalDirection())
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
          }
          else
          {
            validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          }
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
        }
      }
      else if (pTariffRuleRestrictions->ruleTariff() == NO_TARIFF_APPLICATION ||
               pTariffRuleRestrictions->rule() == RULENUM_BLANK)
      {
        if (pTariffRuleRestrictions->globalDirection() == ' ' ||
            pTariffRuleRestrictions->globalDirection() == GlobalDirection::ZZ ||
            targetFare->globalDirection() == pTariffRuleRestrictions->globalDirection())
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
        }
      }
      else
      {
        numDefaulted++;
      }

      if (UNLIKELY(diag.isActive()))
      {
        std::string gd;
        globalDirectionToStr(gd, targetFare->globalDirection());
        if (validationFareComponent[fuCount].getSubCat(m107) == ValidationElement::MATCHED)
          diag << "     MATCH TARIFF: ";
        else // if ( validationFareComponent[fuCount].getSubCat(m107) != ValidationElement::NOT_SET
             // )
          diag << "     NO MATCH TARIFF: ";

        diag << targetFare->tcrRuleTariff() << "/" << targetFare->tcrRuleTariffCode()
             << " RULE: " << targetFare->ruleNumber() << " GLOBAL DI: " << gd << std::endl;
      }

      if (validationFareComponent[fuCount].getSubCat(m107) == ValidationElement::NOT_MATCHED)
        continue;

      if (UNLIKELY(diag.isActive()))
      {
        diag << "   PRIME RULE TAG - " << pTariffRuleRestrictions->primeRuleInd() << std::endl;
      }

      if (UNLIKELY(pTariffRuleRestrictions->primeRuleInd() == 'X'))
      {
        if (targetFare->tcrRuleTariff() == pTariffRuleRestrictions->ruleTariff() &&
            targetFare->ruleNumber() == pTariffRuleRestrictions->rule())
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "     MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "     NO MATCHED" << std::endl;
          continue;
        }
      }

      if (UNLIKELY(diag.isActive()))
        diag << "   DEFAULT RULE TAG - " << pTariffRuleRestrictions->defaultRuleInd() << std::endl;

      if (UNLIKELY(pTariffRuleRestrictions->defaultRuleInd() == 'X'))
      {
        if (!_trx->dataHandle().isRuleInFareMarket(targetFare->fareMarket()->boardMultiCity(),
                                                   targetFare->fareMarket()->offMultiCity(),
                                                   targetFare->fareMarket()->governingCarrier(),
                                                   pTariffRuleRestrictions->rule()))
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "     MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "     NO MATCHED" << std::endl;
          continue;
        }
      }

      if (mustVerifyCarrierPreference &&
          validationFareComponent[fuCount].getSubCat(m107) != ValidationElement::NOT_MATCHED &&
          pTariffRuleRestrictions->ruleTariff() != ANY_TARIFF &&
          pTariffRuleRestrictions->sametrfRuleInd() != NO_APPLICATION &&
          currentFare->carrier() != targetFare->carrier() &&
          currentFare->vendor() != targetFare->vendor())
      {
        if (!validationFareComponent.validateCarrierPreference(
                _trx->dataHandle(), &curFu, targetFare))
        {
          validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "     INVALID VENDOR COMBINATION IN CXR PREF" << std::endl;
        }
      }

      if (validationFareComponent[fuCount].getSubCat(m107) == ValidationElement::MATCHED)
        break;
    }

    if (validationFareComponent[fuCount].getSubCat(m107) == ValidationElement::NOT_SET &&
        numDefaulted > 0 && !tariffRuleInList)
    {
      validationFareComponent[fuCount].getSubCat(m107) = ValidationElement::NOT_MATCHED;

      if (UNLIKELY(diag.isActive()))
      {
        diag << "FAILED-NO MATCHING TARIFF RULE IN LIST" << std::endl;
      }
    }
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processFareClassTypeRestriction(DiagCollector& diag,
                                              const FareUsage& curFu,
                                              const CombinabilityRuleInfo* pCat10,
                                              const uint32_t itemNo,
                                              bool& negative,
                                              ValidationFareComponents& validationFareComponent,
                                              const PricingUnit& pu)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::process FareClassTypeRestriction ()");

  std::vector<FareClassRestRule*>::const_iterator iter;
  std::vector<FareClassRestRule*>::const_iterator iterEnd;

  const std::vector<FareClassRestRule*>& fareClassRestRuleVector =
      _trx->dataHandle().getFareClassRestRule(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(fareClassRestRuleVector.empty()))
  {
    LOG4CXX_INFO(logger, " fareClassRestRuleList.empty().......");
    displayDiag(diag, 68, curFu, itemNo);
    return ABORT;
  }

  DiagCollectorGuard dcg1(diag, Diagnostic638);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "  ------" << std::endl << "  VENDOR " << pCat10->vendorCode()
         << " - 108 ITEM NO " << itemNo << " - FARE "
         << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
  }

  size_t numOfFU = validationFareComponent.size();

  for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)
  {
    if (!validationFareComponent.needAllPassSameMajorItem())
    {
      validationFareComponent[fuCount]._passMinor = false;
      if (validationFareComponent[fuCount]._passMajor)
        continue;
    }
    else if (UNLIKELY(validationFareComponent[fuCount]._passMinor))
      continue;
    if (validationFareComponent[fuCount]._targetFareUsage ==
        validationFareComponent[fuCount]._currentFareUsage)
      continue;

    if (UNLIKELY(!validationFareComponent[fuCount]._targetFareUsage->rec2Cat10()))
    {
      LOG4CXX_INFO(logger, " ** NO Rec 2 Cat 10 **");
      return ABORT;
    }

    PaxTypeFare* targetFare = validationFareComponent[fuCount]._targetFareUsage->paxTypeFare();
    targetFare = validateFareCat10overrideCat25(curFu.paxTypeFare(), targetFare, &pu);

    if (UNLIKELY(diag.isActive()))
    {
      diag << "  COMBINING WITH FARE " << targetFare->createFareBasis(nullptr) << std::endl;
    }

    iter = fareClassRestRuleVector.begin();
    iterEnd = fareClassRestRuleVector.end();

    for (; iter != iterEnd; ++iter)
    {
      FareClassRestRule* pFareClassRestrictions = *iter;

      if (UNLIKELY(diag.isActive()))
      {
        diag << "   ---------------" << std::endl
             << "   APPLICATION TAG: " << pFareClassRestrictions->fareClassTypeApplInd()
             << std::endl;
      }

      //------------------------------------------------------------
      //----- determine if negative 108 ---------
      //------------------------------------------------------------
      //----- pFareClassRestrictions->fareClassTypeApplInd contains
      //----- blank = allowed to combine with restriction
      //-----   N   = not allowed to combine with restriction
      //------------------------------------------------------------
      if (pFareClassRestrictions->fareClassTypeApplInd() == NOT_ALLOWED)
      {
        negative = true;
      }

      if (UNLIKELY(diag.isActive()))
      {
        diag << "   NORMAL FARE TAG 1: " << pFareClassRestrictions->normalFaresInd() << std::endl
             << "   OW/RT IND: " << pFareClassRestrictions->owrt() << std::endl;
      }

      //-------------------------------------------------------
      //----- pFareClassRestrictions->normalFaresInd contains
      //----- blank = no restriction
      //-----   1   = tag 1 / tag 3 fares
      //-----   2   = tag 2 fares
      //-----   3   = tag 1 / tag 2 / tag 3 normal fares
      //-----   4   = tag 1 / tag 2 / tag 3 special fares
      //-----   5   = tag 1 / tag 3 normal fares
      //-----   6   = tag 1 / tag 3 special fares
      //-----   7   = tag 2 normal fares
      //-----   8   = tag 2 special fares
      //-------------------------------------------------------
      //----- check if ow allowed -----------
      if (targetFare->owrt() == ONE_WAY_MAY_BE_DOUBLED)
      {
        if (pFareClassRestrictions->normalFaresInd() == ' ' &&
            (pFareClassRestrictions->owrt() == '2' || pFareClassRestrictions->owrt() == '7' ||
             pFareClassRestrictions->owrt() == '8'))
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED" << std::endl;
          continue;
        }
        else if (pFareClassRestrictions->normalFaresInd() == 'X')
        {
          if (!_trx->dataHandle().isRuleInFareMarket(
                  targetFare->fareMarket()->boardMultiCity(),
                  targetFare->fareMarket()->offMultiCity(),
                  targetFare->fareMarket()->governingCarrier(),
                  validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->ruleNumber()))
          {
            validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
            if (diag.isActive())
              diag << "     MATCHED" << std::endl;
          }
          else
          {
            validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
            if (diag.isActive())
              diag << "     NO MATCHED" << std::endl;
            continue;
          }
        }
      }

      //----- check if rt not allowed -----------
      else if (targetFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      {
        if (pFareClassRestrictions->owrt() == '1' || pFareClassRestrictions->owrt() == '5' ||
            pFareClassRestrictions->owrt() == '6')
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED" << std::endl;
          continue;
        }
      }
      else if (LIKELY(targetFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)) // tag3
      {
        if (pFareClassRestrictions->owrt() == '2')
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED" << std::endl;
          continue;
        }
      }

      //---------------------------------------------------
      //----- check same/different tag ---
      //---------------------------------------------------
      //----- pFareClassRestrictions->sameDiffInd contains
      //----- blank = no application
      //-----   1   = same fare class
      //-----   2   = same fare type
      //-----   3   = same season type
      //-----   4   = same day of week type
      //-----   5   = different fare class
      //-----   6   = different fare type
      //-----   7   = different season type
      //-----   8   = different day of week type
      //---------------------------------------------------

      if (UNLIKELY(diag.isActive()))
      {
        diag << "   SAME/DIFFERENT TAG: " << pFareClassRestrictions->samediffInd() << std::endl;
      }

      switch (pFareClassRestrictions->samediffInd())
      {
      //--- no application ---//
      case ' ':
      {
        validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
        break;
      }
      //--- same fare class ---//
      case '1':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fareClass() !=
            targetFare->fareClass())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - FARE CLASS" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "    MATCHED - FARE CLASS" << std::endl;
        }
        break;
      }
      //--- same fare type ---//
      case '2':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fcaFareType() !=
            targetFare->fcaFareType())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - FARE TYPE" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "    MATCHED - FARE TYPE" << std::endl;
        }
        break;
      }
      //--- same season type ---//
      case '3':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fcaSeasonType() !=
            targetFare->fcaSeasonType())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - SEASON TYPE" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "    MATCHED - SEASON TYPE" << std::endl;
        }
        break;
      }
      //--- same day of week type ---//
      case '4':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fcaDowType() !=
            targetFare->fcaDowType())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - DAY OF WEEK TYPE" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "    MATCHED - DAY OF WEEK TYPE" << std::endl;
        }
        break;
      }
      //--- different fare class ---//
      case '5':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fareClass() ==
            targetFare->fareClass())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
          {
            diag << "    NO MATCHED - REQUIRED DIFFERENT FARE CLASS" << std::endl;
          }
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
        }
        break;
      }
      //--- different fare type ---//
      case '6':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fcaFareType() ==
            targetFare->fcaFareType())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - REQUIRED DIFFERENT FARE TYPE" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
        }
        break;
      }
      //--- different season type ---//
      case '7':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fcaSeasonType() ==
            targetFare->fcaSeasonType())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - REQUIRED DIFFERENT SEASON TYPE" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
        }
        break;
      }
      //--- different day of week type ---//
      case '8':
      {
        if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fcaDowType() ==
            targetFare->fcaDowType())
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
          if (diag.isActive())
            diag << "    NO MATCHED - REQUIRED DIFFERENT DAY OF WEEK TYPE" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
        }
        break;
      }
      //--- none of the above ---//
      default:
      {
        displayDiag(diag, 50);
        return ABORT;
        break;
      }
      }

      if (validationFareComponent[fuCount].getSubCat(m108) == ValidationElement::MATCHED)
      {
        validationFareComponent[fuCount]._currentFareUsage->sameMinMaxInd() =
            pFareClassRestrictions->sameminMaxInd();

        if (UNLIKELY(diag.isActive()))
        {
          diag << "   SAME MIN/MAX STAY: " << pFareClassRestrictions->sameminMaxInd() << std::endl;
          diag << "   TYPE: " << pFareClassRestrictions->typeInd()
               << " TYPE CODE: " << pFareClassRestrictions->typeCode() << std::endl;
        }

        switch (pFareClassRestrictions->typeInd())
        {
        //--- no application ---//
        case ' ':
        {
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          break;
        }
        //--- fare class ---//
        case 'F':
        case 'A':
        {
          if (!RuleUtil::matchFareClass(pFareClassRestrictions->typeCode().c_str(),
                                        targetFare->fareClass().c_str()))
          {
            validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
            if (UNLIKELY(diag.isActive()))
            {
              diag << "    NO MATCHED - FARE CLASS " << targetFare->fareClass() << std::endl;
            }
            break;
          }
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (UNLIKELY(diag.isActive()))
            diag << "    MATCHED - FARE CLASS" << std::endl;
          break;
        }
        //--- fare type ---//
        case 'T':
        {
          if (!RuleUtil::matchFareType(pFareClassRestrictions->typeCode(),
                                       targetFare->fcaFareType()))
          {
            validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
            if (UNLIKELY(diag.isActive()))
            {
              diag << "    NO MATCHED - FARE TYPE " << targetFare->fcaFareType() << std::endl;
            }
            break;
          }
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (UNLIKELY(diag.isActive()))
            diag << "    MATCHED - FARE TYPE" << std::endl;
          break;
        }
        //--- season type ---//
        case 'S':
        {
          if (!RuleUtil::matchSeasons(pFareClassRestrictions->typeCode()[0],
                                      targetFare->fcaSeasonType()))
          {
            validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
            if (diag.isActive())
            {
              diag << "    NO MATCHED - SEASON TYPE " << targetFare->fcaSeasonType() << std::endl;
            }
            break;
          }
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "    MATCHED - SEASON TYPE" << std::endl;
          break;
        }
        //--- day of week type ---//
        case 'D':
        {
          if (!RuleUtil::matchDayOfWeek(pFareClassRestrictions->typeCode()[0],
                                        targetFare->fcaDowType()))
          {
            validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::NOT_MATCHED;
            if (diag.isActive())
            {
              diag << "    NO MATCHED - DAY OF WEEK TYPE " << targetFare->fcaDowType() << std::endl;
            }
            break;
          }
          validationFareComponent[fuCount].getSubCat(m108) = ValidationElement::MATCHED;
          if (diag.isActive())
            diag << "    MATCHED - DAY OF WEEK TYPE" << std::endl;
          break;
        }

        //--- none of the above ---//
        default:
        {
          displayDiag(diag, 50);
          return ABORT;
          break;
        }
        }

        if (UNLIKELY(diag.isActive()))
        {
          diag << "   PENALTY/SERVICE CHARGE APPLICATION: "
               << pFareClassRestrictions->penaltysvcchrgApplInd() << std::endl;
        }
        if (pFareClassRestrictions->penaltysvcchrgApplInd() == RESTRICTION_APPLIES)
        {
          if (diag.isActive())
          {
            diag << "   MOST RESTRICTIVE PENALTY HIERARCHY: "
                 << pFareClassRestrictions->penaltyRestInd()
                 << " APPEND: " << pFareClassRestrictions->appendageCode() << std::endl;
          }

          // if highest penaltyRestInd() does not have AppendageCode  ignore
          //  NR ->nonRefundable

          validationFareComponent[fuCount]._currentFareUsage->penaltyRestInd() =
              pFareClassRestrictions->penaltyRestInd();
          validationFareComponent[fuCount]._currentFareUsage->appendageCode() =
              pFareClassRestrictions->appendageCode();
        }
      }
      // BREAK out of the loop when we encounter a rule that matches (Restrictions only require one
      // match)
      if (validationFareComponent[fuCount].getSubCat(m108) == ValidationElement::MATCHED)
      {
        break;
      }
    } //  for ( ; iter != iterEnd; ++iter )
    if (UNLIKELY(diag.isActive()))
    {
      if (validationFareComponent[fuCount].getSubCat(m108) == ValidationElement::MATCHED)
        diag << "  PASSED COMBINATION: ";
      else
        diag << "  FAILED COMBINATION: ";

      diag << curFu.paxTypeFare()->createFareBasis(nullptr) << " - "
           << targetFare->createFareBasis(nullptr) << "\n  ------\n";
    }
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processFlightRestriction(DiagCollector& diag,
                                       const FareUsage& curFu,
                                       const CombinabilityRuleInfo* pCat10,
                                       const uint32_t itemNo,
                                       ValidationFareComponents& validationFareComponent)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processFlightRestriction ()");

  const FlightAppRule* pFltApplRule;

  pFltApplRule = _trx->dataHandle().getFlightAppRule(pCat10->vendorCode(), itemNo);

  if (UNLIKELY(pFltApplRule == nullptr))
  {
    LOG4CXX_INFO(logger, " pFltApplRule.empty().......");
    displayDiag(diag, 75, curFu, itemNo);
    return ABORT;
  }
  else
  {
    FlightApplication flightApplication;
    flightApplication.initialize(
        pFltApplRule, false, pCat10->vendorCode(), _trx, _trx->ticketingDate());

    DiagCollectorGuard dcg(diag, Diagnostic640);
    if (UNLIKELY(diag.isActive()))
    {
      diag << std::endl << "  ------" << std::endl << "  VENDOR " << pCat10->vendorCode()
           << " - CAT 4 ITEM NO " << itemNo << " - FARE "
           << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
    }
    size_t numOfFU = validationFareComponent.size();

    for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)

    {
      if ((!validationFareComponent[fuCount]._passMinor) &&
          validationFareComponent[fuCount]._targetFareUsage !=
              validationFareComponent[fuCount]._currentFareUsage)
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << "  COMBINING WITH FARE "
               << validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->createFareBasis(
                      nullptr) << std::endl;
        }

        {
          DiagCollectorGuard dcg(diag, Diagnostic304);
          if (UNLIKELY(diag.isActive()))
          {
            diag << std::endl << "  ------ COMBINATION PHASE -----" << std::endl << " - FARE "
                 << curFu.paxTypeFare()->createFareBasis(nullptr);
            diag << "  COMBINING WITH FARE "
                 << validationFareComponent[fuCount]
                        ._targetFareUsage->paxTypeFare()
                        ->createFareBasis(nullptr) << std::endl;
          }
        }
        Record3ReturnTypes ret;

        ret = flightApplication.process(
            *(validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()), *_trx);

        if (ret == PASS)
        {
          validationFareComponent[fuCount].getSubCat(m004) = MATCH;
          if (UNLIKELY(diag.isActive()))
            diag << "   MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m004) = NO_MATCH;
          if (diag.isActive())
            diag << "   NO MATCHED" << std::endl;
        }
      }
    }
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processSalesRestrictions(DiagCollector& diag,
                                       FarePath* farePath,
                                       const PricingUnit& curPu,
                                       FareUsage& curFu,
                                       const CombinabilityRuleInfo& ruleInfo,
                                       const CombinabilityRuleItemInfo& ruleItemInfo,
                                       const uint32_t itemNo,
                                       ValidationFareComponents& validationFareComponent,
                                       Itin* itin)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processSalesRestrictions()");

  const SalesRestriction* pSalesRestRule;

  pSalesRestRule = _trx->dataHandle().getSalesRestriction(ruleInfo.vendorCode(), itemNo);

  if (!pSalesRestRule)
  {
    LOG4CXX_INFO(logger, " pSalesRestRule.empty().......");
    displayDiag(diag, 76, curFu, itemNo);
    return ABORT;
  }
  else
  {
    SalesRestrictionRuleWrapper salesRest;
    PricingUnitDataAccess da(*_trx, farePath, const_cast<PricingUnit&>(curPu), curFu, itin);
    da.setValidatingCxr(validationFareComponent.validatingCarrier());
    salesRest.setRuleDataAccess((RuleControllerDataAccess*)&da);

    DiagCollectorGuard dcg(diag, Diagnostic640);
    if (diag.isActive())
    {
      diag << std::endl << "  ------" << std::endl << "  VENDOR " << ruleInfo.vendorCode()
           << " - CAT 15 ITEM NO " << itemNo << " - FARE "
           << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
    }
    size_t numOfFU = validationFareComponent.size();

    for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)

    {
      if ((!validationFareComponent[fuCount]._passMinor) &&
          validationFareComponent[fuCount]._targetFareUsage !=
              validationFareComponent[fuCount]._currentFareUsage)
      {
        if (diag.isActive())
        {
          diag << "  COMBINING WITH FARE "
               << validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->createFareBasis(
                      nullptr) << std::endl;
        }

        {
          DiagCollectorGuard dcg(diag, Diagnostic315);
          if (diag.isActive())
          {
            diag << std::endl << "  ------ COMBINATION PHASE -----" << std::endl << " - FARE "
                 << curFu.paxTypeFare()->createFareBasis(nullptr);
            diag << "  COMBINING WITH FARE "
                 << validationFareComponent[fuCount]
                        ._targetFareUsage->paxTypeFare()
                        ->createFareBasis(nullptr) << std::endl;
          }
        }
        bool isCat15Security = true;
        FareUsage* fu = validationFareComponent[fuCount]._targetFareUsage;

        Record3ReturnTypes ret = SKIP;

        if (farePath)
        {
          ret = salesRest.validate(*_trx,
                                   *(farePath->itin()),
                                   fu,
                                   *(fu->paxTypeFare()),
                                   ruleInfo,
                                   &ruleItemInfo,
                                   pSalesRestRule,
                                   true,
                                   isCat15Security,
                                   false);
        }
        else
        {
          Itin* curItin = _trx->itin().front();
          if (_trx->getOptions()->isCarnivalSumOfLocal())
          {
            curItin = itin;
          }

          if (!curItin)
          {
            throw ErrorResponseException(ErrorResponseException::NO_ERROR, "NULL itin pointer");
          }

          ret = salesRest.validate(*_trx,
                                   *curItin,
                                   fu,
                                   *(fu->paxTypeFare()),
                                   ruleInfo,
                                   &ruleItemInfo,
                                   pSalesRestRule,
                                   true,
                                   isCat15Security,
                                   false);
        }

        if (ret == PASS)
        {
          validationFareComponent[fuCount].getSubCat(m015) = MATCH;
          if (diag.isActive())
            diag << "   MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m015) = NO_MATCH;
          if (diag.isActive())
            diag << "   NO MATCHED" << std::endl;
        }
      }
    }
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processEligibility(DiagCollector& diag,
                                 FarePath* farePath,
                                 const PricingUnit& curPu,
                                 FareUsage& curFu,
                                 const CombinabilityRuleInfo& ruleInfo,
                                 const CombinabilityRuleItemInfo& ruleItemInfo,
                                 const uint32_t itemNo,
                                 ValidationFareComponents& validationFareComponent)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processEligibility()");

  const EligibilityInfo* pEligibilityInfo;

  pEligibilityInfo = _trx->dataHandle().getEligibility(ruleInfo.vendorCode(), itemNo);

  if (!pEligibilityInfo)
  {
    LOG4CXX_INFO(logger, " pEligibilityInfo.empty().......");
    displayDiag(diag, 77, curFu, itemNo);
    return ABORT;
  }
  else
  {
    Eligibility eligibility;

    DiagCollectorGuard dcg(diag, Diagnostic640);
    if (diag.isActive())
    {
      diag << std::endl << "  ------" << std::endl << "  VENDOR " << ruleInfo.vendorCode()
           << " - CAT 1 ITEM NO " << itemNo << " - FARE "
           << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
    }
    size_t numOfFU = validationFareComponent.size();

    for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)

    {
      if ((!validationFareComponent[fuCount]._passMinor) &&
          validationFareComponent[fuCount]._targetFareUsage !=
              validationFareComponent[fuCount]._currentFareUsage)
      {
        if (diag.isActive())
        {
          diag << "  COMBINING WITH FARE "
               << validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->createFareBasis(
                      nullptr) << std::endl;
        }

        {
          DiagCollectorGuard dcg(diag, Diagnostic301);
          if (diag.isActive())
          {
            diag << std::endl << "  ------ COMBINATION PHASE -----" << std::endl << " - FARE "
                 << curFu.paxTypeFare()->createFareBasis(nullptr);
            diag << "  COMBINING WITH FARE "
                 << validationFareComponent[fuCount]
                        ._targetFareUsage->paxTypeFare()
                        ->createFareBasis(nullptr) << std::endl;
          }
        }
        FareUsage* fu = validationFareComponent[fuCount]._targetFareUsage;

        Record3ReturnTypes ret = SKIP;

        if (farePath)
        {
          ret = eligibility.validate(*_trx, pEligibilityInfo, *farePath, curPu, *fu, true);
        }
        else
        {
          ret = eligibility.validate(*_trx, pEligibilityInfo, curPu, *fu, true);
        }

        if (ret == PASS)
        {
          validationFareComponent[fuCount].getSubCat(m001) = MATCH;
          if (diag.isActive())
            diag << "   MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m001) = NO_MATCH;
          if (diag.isActive())
            diag << "   NO MATCHED" << std::endl;
        }
      }
    }
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processMinimumStay(DiagCollector& diag,
                                 FarePath* farePath,
                                 const PricingUnit& curPu,
                                 FareUsage& curFu,
                                 const CombinabilityRuleInfo& ruleInfo,
                                 const CombinabilityRuleItemInfo& ruleItemInfo,
                                 const uint32_t itemNo,
                                 ValidationFareComponents& validationFareComponent,
                                 Itin* itin)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processMinimumStay()");

  const MinStayRestriction* pMinStayInfo;

  pMinStayInfo = _trx->dataHandle().getMinStayRestriction(ruleInfo.vendorCode(), itemNo);

  if (!pMinStayInfo)
  {
    LOG4CXX_INFO(logger, " pMinStayInfo.empty().......");
    displayDiag(diag, 78, curFu, itemNo);
    return ABORT;
  }
  else
  {
    MinimumStayApplication minStay;

    DiagCollectorGuard dcg(diag, Diagnostic640);
    if (diag.isActive())
    {
      diag << std::endl << "  ------" << std::endl << "  VENDOR " << ruleInfo.vendorCode()
           << " - CAT 6 ITEM NO " << itemNo << " - FARE "
           << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
    }
    size_t numOfFU = validationFareComponent.size();

    for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)

    {
      std::unique_ptr<MinStayApplicationObserverType> minStayObserver =
          MinStayApplicationObserverType::create(
              _trx->getRequest()->isSFR() ? (ObserverType::MIN_STAY_SFR) : (ObserverType::MIN_STAY),
              _trx->dataHandle(),
              &minStay);

      if ((!validationFareComponent[fuCount]._passMinor) &&
          validationFareComponent[fuCount]._targetFareUsage !=
              validationFareComponent[fuCount]._currentFareUsage)
      {
        if (diag.isActive())
        {
          diag << "  COMBINING WITH FARE "
               << validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->createFareBasis(
                      nullptr) << std::endl;
        }

        {
          DiagCollectorGuard dcg(diag, Diagnostic306);
          if (diag.isActive())
          {
            diag << std::endl << "  ------ COMBINATION PHASE -----" << std::endl << " - FARE "
                 << curFu.paxTypeFare()->createFareBasis(nullptr);
            diag << "  COMBINING WITH FARE "
                 << validationFareComponent[fuCount]
                        ._targetFareUsage->paxTypeFare()
                        ->createFareBasis(nullptr) << std::endl;
          }
        }
        FareUsage* fu = validationFareComponent[fuCount]._targetFareUsage;

        Record3ReturnTypes ret = SKIP;

        if (farePath)
        {
          ret = minStay.validate(*_trx, pMinStayInfo, *farePath, curPu, *fu);
        }
        else
        {
          Itin* curItin = _trx->itin().front();

          if (_trx->getOptions()->isCarnivalSumOfLocal())
          {
            curItin = itin;
          }

          if (!curItin)
          {
            throw ErrorResponseException(ErrorResponseException::NO_ERROR, "NULL itin pointer");
          }

          ret = minStay.validate(*_trx, pMinStayInfo, *curItin, curPu, *fu);
        }

        minStayObserver->updateIfNotified(*fu);
        if (ret == PASS)
        {
          validationFareComponent[fuCount].getSubCat(m006) = MATCH;
          if (diag.isActive())
            diag << "   MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m006) = NO_MATCH;
          if (diag.isActive())
            diag << "   NO MATCHED" << std::endl;
        }
      }
    }
  }

  return IDLE;
}

// ----------------------------------------------------------------------------
char
Combinations::processMaximumStay(DiagCollector& diag,
                                 FarePath* farePath,
                                 const PricingUnit& curPu,
                                 FareUsage& curFu,
                                 const CombinabilityRuleInfo& ruleInfo,
                                 const CombinabilityRuleItemInfo& ruleItemInfo,
                                 const uint32_t itemNo,
                                 ValidationFareComponents& validationFareComponent,
                                 Itin* itin)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processMaximumStay()");

  const MaxStayRestriction* pMaxStayInfo;

  pMaxStayInfo = _trx->dataHandle().getMaxStayRestriction(ruleInfo.vendorCode(), itemNo);

  if (!pMaxStayInfo)
  {
    LOG4CXX_INFO(logger, " pMaxStayInfo.empty().......");
    displayDiag(diag, 79, curFu, itemNo);
    return ABORT;
  }
  else
  {
    MaximumStayApplication maxStay;

    DiagCollectorGuard dcg(diag, Diagnostic640);
    if (diag.isActive())
    {
      diag << std::endl << "  ------" << std::endl << "  VENDOR " << ruleInfo.vendorCode()
           << " - CAT 7 ITEM NO " << itemNo << " - FARE "
           << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
    }
    size_t numOfFU = validationFareComponent.size();

    for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)

    {
      std::unique_ptr<MaxStayApplicationObserverType> maxStayObserver =
          MaxStayApplicationObserverType::create(
              _trx->getRequest()->isSFR() ? (ObserverType::MAX_STAY_SFR) : (ObserverType::MAX_STAY),
              _trx->dataHandle(),
              &maxStay);

      if ((!validationFareComponent[fuCount]._passMinor) &&
          validationFareComponent[fuCount]._targetFareUsage !=
              validationFareComponent[fuCount]._currentFareUsage)
      {
        if (diag.isActive())
        {
          diag << "  COMBINING WITH FARE "
               << validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->createFareBasis(
                      nullptr) << std::endl;
        }

        {
          DiagCollectorGuard dcg(diag, Diagnostic307);
          if (diag.isActive())
          {
            diag << std::endl << "  ------ COMBINATION PHASE -----" << std::endl << " - FARE "
                 << curFu.paxTypeFare()->createFareBasis(nullptr);
            diag << "  COMBINING WITH FARE "
                 << validationFareComponent[fuCount]
                        ._targetFareUsage->paxTypeFare()
                        ->createFareBasis(nullptr) << std::endl;
          }
        }
        FareUsage* fu = validationFareComponent[fuCount]._targetFareUsage;

        Record3ReturnTypes ret = SKIP;

        if (farePath)
        {
          ret = maxStay.validate(*_trx, pMaxStayInfo, *farePath, curPu, *fu);
        }
        else
        {
          Itin* curItin = _trx->itin().front();

          if (_trx->getOptions()->isCarnivalSumOfLocal())
          {
            curItin = itin;
          }

          if (!curItin)
          {
            throw ErrorResponseException(ErrorResponseException::NO_ERROR, "NULL itin pointer");
          }

          ret = maxStay.validate(*_trx, pMaxStayInfo, *curItin, curPu, *fu);
        }
        maxStayObserver->updateIfNotified(*fu);

        if (ret == PASS)
        {
          validationFareComponent[fuCount].getSubCat(m007) = MATCH;
          if (diag.isActive())
            diag << "   MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m007) = NO_MATCH;
          if (diag.isActive())
            diag << "   NO MATCHED" << std::endl;
        }
      }
    }
  }

  return IDLE;
}
// ----------------------------------------------------------------------------
char
Combinations::processTravelRestriction(DiagCollector& diag,
                                       FarePath* farePath,
                                       const PricingUnit& curPu,
                                       FareUsage& curFu,
                                       const CombinabilityRuleInfo& ruleInfo,
                                       const CombinabilityRuleItemInfo& ruleItemInfo,
                                       const uint32_t itemNo,
                                       ValidationFareComponents& validationFareComponent,
                                       Itin* itin)
{
  LOG4CXX_INFO(logger, "Entered: Combinations::processTravelRestriction()");

  const TravelRestriction* pTravelRestrictionInfo;

  pTravelRestrictionInfo = _trx->dataHandle().getTravelRestriction(ruleInfo.vendorCode(), itemNo);

  if (!pTravelRestrictionInfo)
  {
    LOG4CXX_INFO(logger, " pTravelRestrictionInfo.empty().......");
    displayDiag(diag, 79, curFu, itemNo);
    return ABORT;
  }
  else
  {
    TravelRestrictionsObserverWrapper tvlRest;
    DiagCollectorGuard dcg(diag, Diagnostic640);
    if (diag.isActive())
    {
      diag << std::endl << "  ------" << std::endl << "  VENDOR " << ruleInfo.vendorCode()
           << " - CAT 14 ITEM NO " << itemNo << " - FARE "
           << curFu.paxTypeFare()->createFareBasis(nullptr) << std::endl;
    }
    size_t numOfFU = validationFareComponent.size();

    for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)

    {
      if ((!validationFareComponent[fuCount]._passMinor) &&
          validationFareComponent[fuCount]._targetFareUsage !=
              validationFareComponent[fuCount]._currentFareUsage)
      {
        if (diag.isActive())
        {
          diag << "  COMBINING WITH FARE "
               << validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->createFareBasis(
                      nullptr) << std::endl;
        }

        {
          DiagCollectorGuard dcg(diag, Diagnostic314);
          if (diag.isActive())
          {
            diag << std::endl << "  ------ COMBINATION PHASE -----" << std::endl << " - FARE "
                 << curFu.paxTypeFare()->createFareBasis(nullptr);
            diag << "  COMBINING WITH FARE "
                 << validationFareComponent[fuCount]
                        ._targetFareUsage->paxTypeFare()
                        ->createFareBasis(nullptr) << std::endl;
          }
        }
        FareUsage* fu = validationFareComponent[fuCount]._targetFareUsage;

        Record3ReturnTypes ret = SKIP;

        if (farePath)
        {
          ret = tvlRest.validate(*_trx, pTravelRestrictionInfo, *farePath, curPu, *fu);
        }
        else
        {
          Itin* curItin = _trx->itin().front();

          if (_trx->getOptions()->isCarnivalSumOfLocal())
          {
            curItin = itin;
          }

          if (!curItin)
          {
            throw ErrorResponseException(ErrorResponseException::NO_ERROR, "NULL itin pointer");
          }

          ret = tvlRest.validate(*_trx, pTravelRestrictionInfo, *curItin, curPu, *fu);
        }

        if (ret == PASS)
        {
          validationFareComponent[fuCount].getSubCat(m014) = MATCH;
          if (diag.isActive())
            diag << "   MATCHED" << std::endl;
        }
        else
        {
          validationFareComponent[fuCount].getSubCat(m014) = NO_MATCH;
          if (diag.isActive())
            diag << "   NO MATCHED" << std::endl;
        }
      }
    }
  }

  return IDLE;
}
// ----------------------------------------------------------------------------
void
Combinations::displayDiag(DiagCollector& diag, const int16_t errNum)
{
  DiagCollectorGuard dcg(diag, Diagnostic610);

  if (diag.isActive())
  {
    if (errNum == 0)
      diag << " PASSED COMBINATION" << std::endl;
    else if (errNum == 1)
      diag << " FAILED COMBINATION" << std::endl;
    else if (errNum == 2)
      diag << " FAILED SUBCATEGORY VALIDATION" << std::endl;
    else if (errNum == 50)
      diag << " FAILED TO BUILD END ON END LIST" << std::endl;
  }
}

// ----------------------------------------------------------------------------
void
Combinations::displayDiag(DiagCollector& diag,
                          const int16_t errNum,
                          const FareUsage& fUsage,
                          const uint32_t itemNo)
{
  DiagCollectorGuard dcg(diag);

  switch (errNum)
  {
  case 60:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " FARE - " << fUsage.paxTypeFare()->createFareBasis(nullptr)
           << " HAS UNAVAILABLE DATA" << std::endl;
    break;
  case 61:
    diag.enable(&fUsage, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND 101 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 62:
    diag.enable(&fUsage, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND 102 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 63:
    diag.enable(&fUsage, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND 103 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 64:
    diag.enable(&fUsage, Diagnostic610);
    if (diag.isActive())
      diag << " NOT FOUND 104 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 65:
    diag.enable(&fUsage, Diagnostic610);
    if (diag.isActive())
      diag << " FARE - " << fUsage.paxTypeFare()->createFareBasis(nullptr)
           << " HAS END-ON-END DATA ERROR" << std::endl;
    break;
  case 67:
    diag.enable(&fUsage, Diagnostic637);
    if (diag.isActive())
      diag << " NOT FOUND 107 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 68:
    diag.enable(&fUsage, Diagnostic638);
    if (diag.isActive())
      diag << " NOT FOUND 108 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 70:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " FAILED DIRECTIONALITY CHECK - " << fUsage.paxTypeFare()->createFareBasis(nullptr)
           << " FARE" << std::endl;
    break;
  case 71:
    diag.enable(&fUsage, Diagnostic605);
    if (diag.isActive())
      diag << " FAILED SUBCAT 101 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 72:
    diag.enable(&fUsage, Diagnostic605);
    if (UNLIKELY(diag.isActive()))
      diag << " FAILED SUBCAT 102 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 73:
    diag.enable(&fUsage, Diagnostic605);
    if (UNLIKELY(diag.isActive()))
      diag << " FAILED SUBCAT 103 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 74:
    diag.enable(&fUsage, Diagnostic610);
    if (UNLIKELY(diag.isActive()))
      diag << " FAILED SUBCAT 104 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 75:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND CAT 4 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 76:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND CAT 15 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 77:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND CAT 1 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 78:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND CAT 6 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  case 79:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " NOT FOUND CAT 7 - ITEM NO " << itemNo << " - "
           << fUsage.paxTypeFare()->createFareBasis(nullptr) << " FARE" << std::endl;
    break;
  default:
    diag.enable(&fUsage, Diagnostic610, Diagnostic605);
    if (diag.isActive())
      diag << " FAILED COMBINATION" << std::endl;
  }
}

// ----------------------------------------------------------------------------
bool
Combinations::checkOJMileage(DiagCollector& diag,
                             const PricingUnit& pu,
                             bool UseLongerLeg,
                             bool restrictDomesticSurf)
{
  bool resultBasedOnPreCheckedSurfStatus = false;
  const std::vector<FareUsage*>& puFareUsage = pu.fareUsage();
  PricingUnit::OJSurfaceStatus ojSurfaceStatus = pu.ojSurfaceStatus();

  if (LIKELY(ojSurfaceStatus != PricingUnit::NOT_CHECKED))
  {
    if (UNLIKELY(puFareUsage.size() > 2 && diag.isActive()))
    {
      diag << "\n      MORE THAN 2 FARE COMP OJ\n";
    }

    if (ojSurfaceStatus == PricingUnit::SURFACE_SHORTEST)
    {
      // Surface is the Shortest
      resultBasedOnPreCheckedSurfStatus = true;
    }

    if (UseLongerLeg)
    {
      if (ojSurfaceStatus == PricingUnit::SURFACE_NOT_SHORTEST)
      {
        // Surface is not the Shortest but not the largest either
        resultBasedOnPreCheckedSurfStatus = true;
      }
    }

    if (LIKELY(!diag.isActive())) // return early or go farther to show diagnostic
    {
      if (resultBasedOnPreCheckedSurfStatus) // surface shortest - ready to return, no need to check
        // same nation
        return true;

      if (restrictDomesticSurf) // otherwise don't return, need to check if surface is same nation
        return resultBasedOnPreCheckedSurfStatus;
    }
  }

  if (UNLIKELY(puFareUsage.size() > 2))
  {
    if (diag.isActive())
      diag << "\n      MORE THAN 2 FARE COMP OJ\n";
    return true;
  }

  const PricingUnit::PUSubType puSubType = pu.puSubType();
  const GeoTravelType geoTvlType = pu.geoTravelType();

  const FareMarket& fm1 = *puFareUsage[0]->paxTypeFare()->fareMarket();
  const FareMarket& fm2 = *puFareUsage[1]->paxTypeFare()->fareMarket();

  uint32_t origSurfMiles = 0;
  uint32_t destSurfMiles = 0;

  //---------------  Get TPM Milage of flown outbound leg ------------------
  //

  const uint32_t leg1Miles = getTPM(fm1, geoTvlType);

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "    TPM MILEAGE OF " << fm1.origin()->loc() << " - "
         << fm1.destination()->loc() << " - " << leg1Miles << std::endl;
  }

  //----------  Get Milage of flown inbound leg ------------------

  const uint32_t leg2Miles = getTPM(fm2, geoTvlType);

  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "    TPM MILEAGE OF " << fm2.origin()->loc() << " - "
         << fm2.destination()->loc() << " - " << leg2Miles << std::endl;
  }

  //----------  Get Milage of Orig Surface segment ------------------

  if (puSubType == PricingUnit::ORIG_OPENJAW || puSubType == PricingUnit::DOUBLE_OPENJAW)
  {
    const Loc& loc1 = *fm1.origin();
    const Loc& loc2 = *fm2.destination();

    bool sameCountrySurfAlwaysShorter =
        (!restrictDomesticSurf && LocUtil::isWithinSameCountryOJ(geoTvlType,
                                                                 pu.itinWithinScandinavia(),
                                                                 loc1,
                                                                 loc2,
                                                                 *fm1.destination(),
                                                                 *fm2.origin()));

    if (!sameCountrySurfAlwaysShorter)
      origSurfMiles = getTPM(loc1, loc2, fm1.travelDate());

    if (UNLIKELY(diag.isActive()))
    {
      std::string text = " ** OJ **";
      if (sameCountrySurfAlwaysShorter)
        text = " *SAME COUNTRY*";
      diag << std::endl << "    TPM MILEAGE OF " << loc1.loc() << " - " << loc2.loc() << " - "
           << getTPM(loc1, loc2, fm1.travelDate()) << text << std::endl;
    }

    if (LIKELY(ojSurfaceStatus != PricingUnit::NOT_CHECKED))
    {
      if (UNLIKELY(resultBasedOnPreCheckedSurfStatus)) // possible only if diagnostic is active
        return true;
      if (UNLIKELY(restrictDomesticSurf)) // possible only if diagnostic is active
        return resultBasedOnPreCheckedSurfStatus;
    }

    if (puSubType == PricingUnit::ORIG_OPENJAW)
    {
      return compareSurface(leg1Miles, leg2Miles, origSurfMiles, 0, UseLongerLeg);
    }
  }

  //----------  Get Milage of Dest Surface segment ------------------

  if ((puSubType == PricingUnit::DEST_OPENJAW || puSubType == PricingUnit::DOUBLE_OPENJAW))
  {
    const Loc& loc1 = *fm1.destination();
    const Loc& loc2 = *fm2.origin();

    bool sameCountrySurfAlwaysShorter =
        (!restrictDomesticSurf && LocUtil::isWithinSameCountryOJ(geoTvlType,
                                                                 pu.itinWithinScandinavia(),
                                                                 loc1,
                                                                 loc2,
                                                                 *fm1.origin(),
                                                                 *fm2.destination()));

    if (!sameCountrySurfAlwaysShorter)
      destSurfMiles = getTPM(loc1, loc2, fm1.travelDate());

    if (diag.isActive())
    {
      std::string text = " ** OJ **";
      if (sameCountrySurfAlwaysShorter)
        text = " *SAME COUNTRY*";
      diag << std::endl << "    TPM MILEAGE OF " << loc1.loc() << " - " << loc2.loc() << " - "
           << getTPM(loc1, loc2, fm1.travelDate()) << text << std::endl;
    }

    if (puSubType == PricingUnit::DEST_OPENJAW)
    {
      return compareSurface(leg1Miles, leg2Miles, 0, destSurfMiles, UseLongerLeg);
    }
  }

  if (puSubType == PricingUnit::DOUBLE_OPENJAW)
  {
    return compareSurface(leg1Miles, leg2Miles, origSurfMiles, destSurfMiles, UseLongerLeg);
  }

  return false;
}

// ----------------------------------------------------------------------------
bool
Combinations::compareSurface(uint32_t leg1Miles,
                             uint32_t leg2Miles,
                             uint32_t origSurfMiles,
                             uint32_t destSurfMiles,
                             bool UseLongerLeg)
{
  if (UseLongerLeg)
  {
    if ((leg1Miles >= origSurfMiles || leg2Miles >= origSurfMiles) &&
        (leg1Miles >= destSurfMiles || leg2Miles >= destSurfMiles))
      return true;
    else
      return false;
  }
  else
  {
    if (leg1Miles >= origSurfMiles && leg1Miles >= destSurfMiles && leg2Miles >= origSurfMiles &&
        leg2Miles >= destSurfMiles)
      return true;
    else
      return false;
  }
}

// ----------------------------------------------------------------------------
uint32_t
Combinations::getTPM(const FareMarket& fm, const GeoTravelType& geoTvlType)
{
  const DateTime& travelDate = fm.travelDate();

  if (UNLIKELY(geoTvlType != GeoTravelType::International))
  {
    const Loc& orig = *fm.origin();
    const Loc& dest = *fm.destination();
    return LocUtil::getTPM(orig, dest, fm.getGlobalDirection(), travelDate, _trx->dataHandle());
  }
  else
  {
    GlobalDirection gd;
    uint32_t mileage = 0;

    for (TravelSeg* travelSeg : fm.travelSeg())
    {
      const Loc& orig = *travelSeg->origin();
      const Loc& dest = *travelSeg->destination();
      GlobalDirectionFinderV2Adapter::getGlobalDirection(_trx, travelDate, *travelSeg, gd);

      mileage += LocUtil::getTPM(orig, dest, gd, travelDate, _trx->dataHandle());
    }
    return mileage;
  }
}
// ----------------------------------------------------------------------------
uint32_t
Combinations::getTPM(const Loc& loc1, const Loc& loc2, DateTime travelDate)
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg tvlSeg;
  tvlSeg.origin() = &loc1;
  tvlSeg.destination() = &loc2;
  tvlSegs.push_back(&tvlSeg);

  GlobalDirection gd;
  GlobalDirectionFinderV2Adapter::getGlobalDirection(_trx, travelDate, tvlSegs, gd);

  return LocUtil::getTPM(loc1, loc2, gd, travelDate, _trx->dataHandle());
}

uint16_t
Combinations::getSubCat(const PricingUnit::Type curPuType)
{
  if (curPuType == PricingUnit::Type::ROUNDTRIP)
    return ROUND_TRIP;
  else if (UNLIKELY(curPuType == PricingUnit::Type::ONEWAY))
    return END_ON_END;
  else if (curPuType == PricingUnit::Type::OPENJAW)
    return OPEN_JAW;
  else if (LIKELY(curPuType == PricingUnit::Type::CIRCLETRIP))
    return CIRCLE_TRIP;
  else
    return 0;
}

bool
Combinations::diagInMajorSubCat(const DiagnosticTypes& diagType)
{
  if (LIKELY(diagType == DiagnosticNone))
    return false;

  if (diagType == Diagnostic636 || diagType == Diagnostic637 || diagType == Diagnostic638 ||
      diagType == Diagnostic639)
  {
    return false;
  }
  return true;
}

char
Combinations::checkGeoSpec(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;
  DiagCollector& diag = eoeDA._diag;
  const EndOnEnd& eoeRule = eoeDA._endOnEndRule;
  const VendorCode& vendorCode = eoeDA._cat10Rule.vendorCode();

  const tse::LocCode& origin = validationFareComponent[fuCount]
                                   ._targetFareUsage->paxTypeFare()
                                   ->fareMarket()
                                   ->boardMultiCity();
  const tse::LocCode& destination = validationFareComponent[fuCount]
                                        ._targetFareUsage->paxTypeFare()
                                        ->fareMarket()
                                        ->offMultiCity();
  const tse::DateTime& ticketingDate = _trx->getRequest()->ticketingDT();

  //--- check GEO LOC1 ------------------
  bool loc1 = LocUtil::isInLoc(origin,
                               eoeRule.fareTypeLoc1Type(),
                               eoeRule.fareTypeLoc1(),
                               vendorCode,
                               RESERVED,
                               GeoTravelType::International,
                               LocUtil::OTHER,
                               ticketingDate);

  //--- check GEO LOC2 ------------------
  bool loc2 = LocUtil::isInLoc(destination,
                               eoeRule.fareTypeLoc2Type(),
                               eoeRule.fareTypeLoc2(),
                               vendorCode,
                               RESERVED,
                               GeoTravelType::International,
                               LocUtil::OTHER,
                               ticketingDate);

  //--- check GEO LOC3 ------------------
  bool loc3 = LocUtil::isInLoc(destination,
                               eoeRule.fareTypeLoc1Type(),
                               eoeRule.fareTypeLoc1(),
                               vendorCode,
                               RESERVED,
                               GeoTravelType::International,
                               LocUtil::OTHER,
                               ticketingDate);

  //--- check GEO LOC4 ------------------
  bool loc4 = LocUtil::isInLoc(origin,
                               eoeRule.fareTypeLoc2Type(),
                               eoeRule.fareTypeLoc2(),
                               vendorCode,
                               RESERVED,
                               GeoTravelType::International,
                               LocUtil::OTHER,
                               ticketingDate);

  //--- Application Indicator ---
  switch (eoeRule.fareTypeLocAppl())
  {
  //--- between/And ---//
  case 'B':
  {
    if ((loc1 && loc2) || (loc3 && loc4))
    {
      diag << "    MATCH - FARE TYPE LOC APPL - ";
    }
    else if ((loc1 || loc3) && eoeRule.fareTypeLoc2().empty())
    {
      diag << "    MATCH - FARE TYPE LOC APPL - ";
    }
    else
    {
      diag << "    FAILED - FARE TYPE LOC APPL - ";
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
    }
    break;
  }
  //--- via ---//
  case 'V':
  {
    if (loc1 || loc2)
    {
      diag << "    MATCH - FARE TYPE LOC APPL - ";
    }
    else
    {
      diag << "    FAILED - FARE TYPE LOC APPL - ";
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
    }
    break;
  }
  default:
  {
    //--- data error ---
    diag << "    DATA ERROR - FARE TYPE LOC APPL - " << eoeRule.fareTypeLocAppl() << std::endl;
    //--- data error ---
    displayDiag(diag, 65, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
    if (eoeDA._diag614IsActive)
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  ABORTED" << std::endl;
      }
    }
    return ABORT;
  }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << eoeRule.fareTypeLocAppl() << std::endl << "                    "
         << "  LOC 1: " << eoeRule.fareTypeLoc1Type() << " - " << eoeRule.fareTypeLoc1()
         << "  LOC 2 : " << eoeRule.fareTypeLoc2Type() << " - " << eoeRule.fareTypeLoc2()
         << std::endl << "                    "
         << "  ORG: " << origin << "  DES: " << destination << std::endl;
  }

  if (validationFareComponent[fuCount].getSubCat(M104) == FAILCOMB)
  {
    displayDiag(diag, 74, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
    if (UNLIKELY(eoeDA._diag614IsActive))
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  FAILED" << std::endl;
      }
    }

    return FAILCOMB;
  }
  return PASSCOMB;
}

char
Combinations::processEndOnEndNormalFare(EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  _eoePresentFareTypes.setNormalFarePresent();

  const Indicator eoeNormalInd = eoeDA._endOnEndRule.eoeNormalInd();
  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  switch (eoeNormalInd)
  {
  case RESTRICTIONS:
  case PERMITTED:
    if (diag.isActive())
    {
      diag << "    MATCH - EOE NORMAL IND - " << eoeNormalInd << std::endl;
    }
    return PASSCOMB;

  case NO_APPLICATION:
  case NOT_PERMITTED:
    if (diag.isActive())
    {
      diag << "    FAILED - EOE NORMAL IND - " << eoeNormalInd << std::endl;
    }
    displayDiag(diag, 74, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
    if (eoeDA._diag614IsActive)
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  FAILED" << std::endl;
      }
    }
    validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
    if (eoeNormalInd == NOT_PERMITTED)
      return STOPCOMB;
    else
      return FAILCOMB;
  default:
  {
    if (diag.isActive())
    {
      diag << "    DATA ERROR - EOE NORMAL IND - " << eoeNormalInd << std::endl;
    }
    displayDiag(diag, 65, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
    if (eoeDA._diag614IsActive)
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  ABORTED" << std::endl;
      }
    }
    return ABORT;
  }
  }
}

void
Combinations::diagEndOnEndFareInfo(EndOnEndDataAccess& eoeDA)
{
  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  DiagCollectorGuard dcg1(diag, Diagnostic634);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << "   ------" << std::endl << "   VENDOR " << eoeDA._cat10Rule.vendorCode()
         << " - 104 ITEM NO " << eoeDA._itemNo << " - FARE "
         << validationFareComponent[0]._currentFareUsage->paxTypeFare()->createFareBasis(nullptr)
         << std::endl;

    diag << "   ALL SEGMENTS IND - " << eoeDA._endOnEndRule.allsegsInd() << std::endl;
  }

  const DirectionalityInfo& directionalityInfo = eoeDA._directionalityInfo;

  if (LIKELY(
          (!directionalityInfo.diagFarePathNumber ||
           (directionalityInfo.farePathNumber == directionalityInfo.diagFarePathNumber)) &&
          (!directionalityInfo.diagPricingUnitNumber ||
           (directionalityInfo.pricingUnitNumber == directionalityInfo.diagPricingUnitNumber)) &&
          (!directionalityInfo.diagFareUsageNumber ||
           (directionalityInfo.fareUsageNumber == directionalityInfo.diagFareUsageNumber))))
  {
    DiagCollectorGuard dcg2(diag, Diagnostic614);
    if (UNLIKELY(diag.enableFilter(Diagnostic614, 1, directionalityInfo.farePathNumber)))
    {
      eoeDA._diag614IsActive = true;

      const PaxTypeFare& paxTypeFare =
          *((validationFareComponent[0]._currentFareUsage)->paxTypeFare());

      diag << "    ----------------------------------------------------------\n"
           << "      FP " << directionalityInfo.farePathNumber << "   PU "
           << directionalityInfo.pricingUnitNumber << "   FN " << directionalityInfo.fareUsageNumber
           << "\n\n"
           << "         " << std::setw(3)
           << (std::string)(validationFareComponent[0]._currentFareUsage)
                  ->paxTypeFare()
                  ->fareMarket()
                  ->boardMultiCity() << "-" << std::setw(2)
           << (validationFareComponent[0]._currentFareUsage)
                  ->paxTypeFare()
                  ->fareMarket()
                  ->governingCarrier() << "-" << std::setw(3)
           << (std::string)(validationFareComponent[0]._currentFareUsage)
                  ->paxTypeFare()
                  ->fareMarket()
                  ->offMultiCity() << "  " << std::setw(13)
           << DiagnosticUtil::getFareBasis(paxTypeFare) << "  END ON END WITH" << std::endl;
    }
  }
}

char
Combinations::processUnavailTag(const EndOnEndDataAccess& eoeDA)
{
  DiagCollector& diag = eoeDA._diag;
  const Indicator unavailTag = eoeDA._endOnEndRule.unavailTag();

  if (UNLIKELY(unavailTag == UNAVAILABLE_DATA))
  {
    if (diag.isActive())
    {
      diag << "   UNAVAILABLE DATA TAG - " << unavailTag << std::endl;
      diag << " - FAILED" << std::endl;
    }
    displayDiag(diag, 60, *(eoeDA._validationFareComponent[0]._currentFareUsage), eoeDA._itemNo);
    eoeDA._validationFareComponent[0].getSubCat(M104) = FAILCOMB;
    return FAILCOMB;
  }
  else if (UNLIKELY(unavailTag == TEXT_DATA_ONLY))
  {
    if (diag.isActive())
    {
      diag << "   UNAVAILABLE DATA TAG - " << unavailTag << std::endl;
      diag << " - SKIPPED" << std::endl;
    }
    eoeDA._validationFareComponent[0].getSubCat(M104) = NO_MATCH;
    return NO_MATCH;
  }

  return PASSCOMB;
}

char
Combinations::processEndOnEndABA(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  const Indicator abacombInd = eoeDA._endOnEndRule.abacombInd();
  if (abacombInd == NO_RESTRICTION)
    return PASSCOMB;

  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  if (abacombInd != MUST_NOT_BE_A_B_A && abacombInd != MUST_BE_A_B_A &&
      abacombInd != IF_EOE_MUST_BE_A_B_A)
  {
    if (diag.isActive())
    {
      diag << "   DATA ERROR - ABA COMB IND - " << abacombInd << " - ITEM NO " << eoeDA._itemNo
           << std::endl;
    }
    //--- data error ---
    displayDiag(diag, 65, *(validationFareComponent[fuCount]._currentFareUsage), eoeDA._itemNo);
    validationFareComponent[fuCount].getSubCat(M104) = ABORT;
  }
  else
  {
    const tse::LocCode& cOrig = validationFareComponent[fuCount]
                                    ._currentFareUsage->paxTypeFare()
                                    ->fareMarket()
                                    ->boardMultiCity();
    const tse::LocCode& cDest = validationFareComponent[fuCount]
                                    ._currentFareUsage->paxTypeFare()
                                    ->fareMarket()
                                    ->offMultiCity();
    const tse::LocCode& tOrig = validationFareComponent[fuCount]
                                    ._targetFareUsage->paxTypeFare()
                                    ->fareMarket()
                                    ->boardMultiCity();
    const tse::LocCode& tDest = validationFareComponent[fuCount]
                                    ._targetFareUsage->paxTypeFare()
                                    ->fareMarket()
                                    ->offMultiCity();

    bool isABA = false;
    // same locations opposite direction
    isABA = (cOrig == tDest && cDest == tOrig);

    if ((isABA && abacombInd == MUST_NOT_BE_A_B_A) || (!isABA && abacombInd == MUST_BE_A_B_A) ||
        (!isABA && !eoeDA._pu.noPUToEOE() && abacombInd == IF_EOE_MUST_BE_A_B_A))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "   FAILED - ABA COMB IND - " << abacombInd << std::endl;
      }
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
    }
    else
    {
      if (diag.isActive())
      {
        diag << "   MATCH - ABA COMB IND - " << abacombInd << std::endl;
      }
    }
  }

  if ((validationFareComponent[fuCount].getSubCat(M104) == ABORT) ||
      (validationFareComponent[fuCount].getSubCat(M104) == FAILCOMB))
  {
    if (UNLIKELY(eoeDA._diag614IsActive))
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        if (validationFareComponent[fuCount].getSubCat(M104) == FAILCOMB)
        {
          diag << "  FAILED" << std::endl;
        }
        else
        {
          diag << "  ABORTED" << std::endl;
        }
      }
    }
    if (UNLIKELY(validationFareComponent[fuCount].getSubCat(M104) == ABORT))
      return ABORT;
    else
      return NO_MATCH;
  }

  return PASSCOMB;
}

char
Combinations::processEndOnEndTbl994(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  const uint32_t overrideDateTblItemNo = eoeDA._endOnEndRule.overrideDateTblItemNo();

  if (overrideDateTblItemNo == 0)
    return MATCH;

  DiagCollector& diag = eoeDA._diag;

  LOG4CXX_INFO(logger, " pEndOnEnd->overrideDateTblItemNo() != 0 ");

  const PaxTypeFare& paxTypeFare =
      *(eoeDA._validationFareComponent[fuCount]._currentFareUsage->paxTypeFare());
  const Fare* pFare = paxTypeFare.fare();

  DateTime travelDate = DateTime::openDate();
  RuleUtil::getTvlDateForTbl994Validation(
      travelDate, RuleConst::COMBINABILITY_RULE, paxTypeFare, nullptr, &eoeDA._pu);
  DateTime bookingDate;
  RuleUtil::getLatestBookingDate(*_trx, bookingDate, paxTypeFare);

  DiagCollectorGuard dcg2(diag, Diagnostic654);
  if (UNLIKELY(diag.isActive()))
  {
    diag << " ------ " << std::endl << " VENDOR " << pFare->vendor() << " - 994 ITEM NO "
         << overrideDateTblItemNo << " - SUBCAT 104 - FARE " << pFare->fareClass() << std::endl
         << "  DEPARTURE DATE: " << travelDate.dateToString(DDMMM, "") << std::endl
         << "  TICKETING DATE: " << _trx->getRequest()->ticketingDT().dateToString(DDMMM, "")
         << std::endl << "  BOOKING DATE  : " << bookingDate.dateToString(DDMMM, "") << std::endl;
  }
  if (RuleUtil::validateDateOverrideRuleItem(*_trx,
                                             overrideDateTblItemNo,
                                             pFare->vendor(),
                                             travelDate,
                                             _trx->getRequest()->ticketingDT(),
                                             bookingDate,
                                             &diag,
                                             Diagnostic654))
  {
    diag.enable(Diagnostic654);
    if (diag.isActive())
      diag << "   MATCH - OVERRIDE DATE TABLE" << std::endl;

    diag.enable(Diagnostic634);
    if (diag.isActive())
    {
      diag << "   PASSED - OVERRIDE DATE TABLE - " << overrideDateTblItemNo << std::endl;
    }
    return MATCH;
  }

  diag.enable(Diagnostic654);
  if (UNLIKELY(diag.isActive()))
    diag << "   NO MATCH" << std::endl;

  diag.enable(Diagnostic634);
  if (UNLIKELY(diag.isActive()))
  {
    diag << "   FAILED - OVERRIDE DATE TABLE - " << overrideDateTblItemNo << std::endl;
  }
  displayDiag(diag, 74, *eoeDA._validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
  if (UNLIKELY(eoeDA._diag614IsActive))
  {
    DiagCollectorGuard dcg10(diag, Diagnostic614);
    if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
    {
      diag << "  FAILED" << std::endl;
    }
  }
  eoeDA._validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
  return NO_MATCH;
}

char
Combinations::processEndOnEndSpecialFare(EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  _eoePresentFareTypes.setSpecialFarePresent();

  const Indicator eoespecialInd = eoeDA._endOnEndRule.eoespecialInd();
  const Indicator eoespecialApplInd = eoeDA._endOnEndRule.eoespecialApplInd();
  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  if (eoespecialInd == NO_APPLICATION || eoespecialInd == NOT_PERMITTED)
  {
    //----- portion of travel -------------------
    switch (eoespecialApplInd)
    {
    case IGNORE_SPECIAL_FARE:
      if (diag.isActive())
      {
        diag << "    MATCH - EOE SPECIAL IND - " << eoespecialInd << " AND EOE SPECIAL APPL IND - "
             << eoespecialApplInd << std::endl;
      }
      break;

    case APPLY_ONLY_SPECIAL_FARE:
      if (diag.isActive())
      {
        diag << "    FAILED - EOE SPECIAL IND - " << eoespecialInd << " AND EOE SPECIAL APPL IND - "
             << eoespecialApplInd << std::endl;
      }
      displayDiag(diag, 74, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
      if (eoeDA._diag614IsActive)
      {
        DiagCollectorGuard dcg10(diag, Diagnostic614);
        if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
        {
          diag << "  FAILED" << std::endl;
        }
      }
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
      if (eoespecialInd == NOT_PERMITTED)
        return STOPCOMB;
      else
        return FAILCOMB;

    case APPLY_TO_ENTIRE_TRIP:
    {
      if (diag.isActive())
      {
        diag << "    MATCH - EOE SPECIAL IND - " << eoespecialInd << " AND EOE SPECIAL APPL IND - "
             << eoespecialApplInd << std::endl;
      }
      break;
    }
    default:
    {
      if (diag.isActive())
      {
        diag << "    DATA ERROR - EOE SPECIAL APPL IND - " << eoespecialApplInd << std::endl;
      }
      displayDiag(diag, 65, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
      if (eoeDA._diag614IsActive)
      {
        DiagCollectorGuard dcg10(diag, Diagnostic614);
        if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
        {
          diag << "  ABORTED" << std::endl;
        }
      }
      return ABORT;
    }
    } // switch

    return PASSCOMB;
  }

  if (LIKELY(eoespecialInd == RESTRICTIONS || eoespecialInd == PERMITTED))
  {
    switch (eoespecialApplInd)
    {
    case IGNORE_SPECIAL_FARE:
    case APPLY_ONLY_SPECIAL_FARE:
      if (diag.isActive())
      {
        diag << "    MATCH - EOE SPECIAL IND - " << eoespecialInd << " AND EOE SPECIAL APPL IND - "
             << eoespecialApplInd << std::endl;
      }
      break;

    case APPLY_TO_ENTIRE_TRIP:
    {
      if (diag.isActive())
      {
        diag << "    MATCH - EOE SPECIAL IND - " << eoespecialInd << " AND EOE SPECIAL APPL IND - "
             << eoespecialApplInd << std::endl;
      }
      break;
    }
    default:
    {
      if (diag.isActive())
      {
        diag << "    DATA ERROR - EOE SPECIAL APPL IND - " << eoespecialApplInd << std::endl;
      }
      displayDiag(diag, 65, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
      if (eoeDA._diag614IsActive)
      {
        DiagCollectorGuard dcg10(diag, Diagnostic614);
        if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
        {
          diag << "  ABORTED" << std::endl;
        }
      }
      return ABORT;
    }
    }
    return PASSCOMB;
  }

  //--- data error ---
  if (diag.isActive())
  {
    diag << "    DATA ERROR - EOE SPECIAL IND - " << eoespecialInd << std::endl;
  }
  displayDiag(diag, 65, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
  if (eoeDA._diag614IsActive)
  {
    DiagCollectorGuard dcg10(diag, Diagnostic614);
    if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
    {
      diag << "  ABORTED" << std::endl;
    }
  }
  return ABORT;
}

char
Combinations::processEndOnEndDomIntlTransbFare(EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  const PaxTypeFare& targetPTF =
      *eoeDA._validationFareComponent[fuCount]._targetFareUsage->paxTypeFare();
  const FareMarket* fareMarket = targetPTF.fareMarket();

  const EndOnEnd& eoeRule = eoeDA._endOnEndRule;

  if (eoeRule.uscatransborderInd() == NO_APPLICATION && eoeRule.domInd() == NO_APPLICATION &&
      eoeRule.intlInd() == NO_APPLICATION)
  {
    diagEndOnEndPassBlankDomIntlTransInd(eoeDA, *targetPTF.fare());
    return PASSCOMB;
  }

  char rtn = PASSCOMB;

  // Determine type of fare.

  //  ATPCO Cat10_dapp_C.pdf, page E.03.10.160:
  //
  //    US/Canada Fares (byte 18)
  //        Transborder Fares. Fares between the United States (including the US 50 states,
  //        the District of Columbia, Puerto Rico, and US Virgin Islands) and Canada.
  //
  //    Domestic Fares (byte 19)
  //        Fares where the origin and destination are in the same ATPCO Country Code.
  //        Exception: Fares between countries XU and RU are domestic.
  //
  //        Notes for clarification:
  //          - Fares between US and Canada are not domestic. These are US/CA Transborder fares and
  //            validate against byte 18.
  //          - Fares between countries in Scandinavia are not domestic.
  //          - Fares between a nation and its territory possessing its own ATPCO Country Code are
  //          not
  //            domestic fares (cabotage fares is not domestic).
  //          - Fares between a nation and its territory not possessing its own ATPCO Country Code
  //          are
  //            considered part of the sovereign state and are domestic.
  //
  //    International Fares (byte 20)
  //        Fares where the origin and destination are not in the same ATPCO Country Code.
  //        Exception: Fares between countries XU and RU are not international.
  //
  //        Notes for clarification:
  //          - Fares between US and Canada are not International. These are US/CA Transborder fares
  //          and
  //            validate against byte 18.
  //          - Fares between countries in Scandinavia are international.
  //          - Fares between a nation and its territory possessing its own ATPCO Country Code are
  //            international fares (cabotage fares is international).
  //          - Fares between a nation and its territory not possessing its own ATPCO Country Code
  //          are
  //            considered part of the sovereign state and are not international.

  bool isDomesticFare =
      fareMarket->isDomestic() || fareMarket->isForeignDomestic() ||
      LocUtil::isDomesticRussia(*fareMarket->origin(), *fareMarket->destination());

  if (isDomesticFare)
  {
    rtn = processEndOnEndFareGeo(eoeDA, fuCount, eoeRule.domInd(), "DOMESTIC");

    if (rtn == PASSCOMB)
      _eoePresentFareTypes.setDomesticFarePresent();
  }
  else if (fareMarket->isTransBorder())
  {
    rtn = processEndOnEndFareGeo(eoeDA, fuCount, eoeRule.uscatransborderInd(), "US/CA TRANSBORDER");

    if (rtn == PASSCOMB) // msd
      _eoePresentFareTypes.setTransborderFarePresent();
  }
  else if (LIKELY(fareMarket->isInternational()))
  {
    rtn = processEndOnEndFareGeo(eoeDA, fuCount, eoeRule.intlInd(), "INTERNATIONAL");

    if (rtn == PASSCOMB)
      _eoePresentFareTypes.setInternationalFarePresent();
  }
  return rtn;
}

char
Combinations::processEndOnEndFareGeo(EndOnEndDataAccess& eoeDA,
                                     const size_t fuCount,
                                     const Indicator geoInd,
                                     const std::string& diagGeoStr)
{
  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  if (geoInd == RESTRICTIONS || geoInd == PERMITTED)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "    MATCH - " << diagGeoStr << " IND - " << geoInd << std::endl;
    }
    return PASSCOMB;
  }

  if (LIKELY(geoInd == NO_APPLICATION || geoInd == NOT_PERMITTED))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "    FAILED - " << diagGeoStr << " IND - " << geoInd << std::endl;
    }
    displayDiag(diag, 74, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
    if (UNLIKELY(eoeDA._diag614IsActive))
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  FAILED" << std::endl;
      }
    }
    validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
    if (geoInd == NOT_PERMITTED)
      return STOPCOMB;
    else
      return FAILCOMB;
  }

  //--- data error ---
  displayDiag(diag, 65, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
  if (eoeDA._diag614IsActive)
  {
    DiagCollectorGuard dcg10(diag, Diagnostic614);
    if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
    {
      diag << "  ABORTED" << std::endl;
    }
  }
  return ABORT;
}

char
Combinations::processEndOnEndTktInd(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  //----- don't know yet, How we handle this cases -----
  const Indicator tktInd = eoeDA._endOnEndRule.tktInd();

  if (tktInd == NO_APPLICATION)
    return PASSCOMB;

  DiagCollector& diag = eoeDA._diag;

  if (UNLIKELY(diag.isActive()))
  {
    diag << "    MATCH - EOE TICKET TAG - " << tktInd << std::endl;
  }

  switch (tktInd)
  {
  //-- fares used in combination must be issued on separate tickets --//
  case '1':
  {
    break;
  }
  //-- fares must be shown separately on the ticket --//
  case '2':
  {
    break;
  }
  //-- fares must be shown as a through fare on the ticket --//
  case '3':
  {
    break;
  }
  default:
  {
    if (diag.isActive())
    {
      diag << "    DATA ERROR - EOE TICKET TAG - " << tktInd << std::endl;
    }
    displayDiag(
        diag, 65, *eoeDA._validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
    if (eoeDA._diag614IsActive)
    {
      DiagCollectorGuard dcg10(diag, Diagnostic614);
      if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
      {
        diag << "  ABORTED" << std::endl;
      }
    }
    return ABORT;
  }
  }
  return PASSCOMB;
}

void
Combinations::diagEndOnEndTargetFare(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  if (UNLIKELY(eoeDA._diag614IsActive))
  {
    DiagCollectorGuard dcg2(diag, Diagnostic614);
    if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
    {
      diag << "                           " << std::setw(3)
           << (std::string)(validationFareComponent[fuCount]._targetFareUsage)
                  ->paxTypeFare()
                  ->fareMarket()
                  ->boardMultiCity() << "-" << std::setw(2)
           << (validationFareComponent[fuCount]._targetFareUsage)
                  ->paxTypeFare()
                  ->fareMarket()
                  ->governingCarrier() << "-" << std::setw(3)
           << (std::string)(validationFareComponent[fuCount]._targetFareUsage)
                  ->paxTypeFare()
                  ->fareMarket()
                  ->offMultiCity() << "  " << std::setw(13)
           << (validationFareComponent[fuCount]._targetFareUsage)->paxTypeFare()->createFareBasis(
                  nullptr);
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << "   EOE WITH FARE "
         << (validationFareComponent[fuCount]._targetFareUsage)->paxTypeFare()->createFareBasis(
                nullptr) << std::endl;
  }
}

void
Combinations::diagEndOnEndTargetFUPriorPassed(const EndOnEndDataAccess& eoeDA)
{
  DiagCollector& diag = eoeDA._diag;

  if (eoeDA._diag614IsActive)
  {
    DiagCollectorGuard dcg2(diag, Diagnostic614);
    if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
    {
      diag << "   PASSED - PRIOR RULE PASSED - " << std::endl;
    }
  }
  else
  {
    if (diag.isActive())
      diag << "   PASSED - PRIOR RULE PASSED - " << std::endl;
  }
}

char
Combinations::processEndOnEndSameCxr(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  if (eoeDA._endOnEndRule.sameCarrierInd() != RESTRICTION_APPLIES)
    return PASSCOMB;

  const Fare* pFare =
      eoeDA._validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->fare();
  const Fare* pFareT =
      eoeDA._validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->fare();

  if (pFare->carrier() != pFareT->carrier())
  //&& pFareT->carrier() != INDUSTRY_CARRIER )
  {
    diagEndOnEndFailedSameCxr(eoeDA, fuCount);
    eoeDA._validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
    return FAILCOMB;
  }
  return PASSCOMB;
}

void
Combinations::diagEndOnEndFailedSameCxr(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  DiagCollector& diag = eoeDA._diag;
  if (UNLIKELY(diag.isActive()))
  {
    diag << "    FAILED - SAME CARRIER IND - " << eoeDA._endOnEndRule.sameCarrierInd() << std::endl;
  }
  displayDiag(diag, 74, *eoeDA._validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
  if (UNLIKELY(eoeDA._diag614IsActive))
  {
    DiagCollectorGuard dcg10(diag, Diagnostic614);
    if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
    {
      diag << "  FAILED" << std::endl;
    }
  }
}

char
Combinations::processEndOnEndRestInd(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  if (eoeDA._endOnEndRule.eoeRestInd() == NO_APPLICATION)
    return PASSCOMB;

  DiagCollector& diag = eoeDA._diag;
  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;
  const Indicator eoeRestInd = eoeDA._endOnEndRule.eoeRestInd();

  const tse::LocCode& cOrig = validationFareComponent[fuCount]
                                  ._currentFareUsage->paxTypeFare()
                                  ->fareMarket()
                                  ->boardMultiCity();
  const tse::LocCode& tOrig = validationFareComponent[fuCount]
                                  ._targetFareUsage->paxTypeFare()
                                  ->fareMarket()
                                  ->boardMultiCity();
  const tse::LocCode& tDest = validationFareComponent[fuCount]
                                  ._targetFareUsage->paxTypeFare()
                                  ->fareMarket()
                                  ->offMultiCity();

  if (UNLIKELY(cOrig == tOrig && cOrig == tDest))
  {
    if (eoeRestInd == MAY_NOT_COMBINE_OW_RT)
    {
      if (diag.isActive())
      {
        diag << "    FAILED - EOE REST IND - " << eoeRestInd << std::endl;
      }
      displayDiag(diag, 74, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
      if (eoeDA._diag614IsActive)
      {
        DiagCollectorGuard dcg10(diag, Diagnostic614);
        if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
        {
          diag << "  FAILED" << std::endl;
        }
      }
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;

      return FAILCOMB;
    }

    const Fare* pFareT = validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->fare();
    if (pFareT->owrt() == ROUND_TRIP_FARE)
    {
      if (diag.isActive())
      {
        diag << "    FAILED - EOE REST IND - " << eoeRestInd << " FARE TAG: " << pFareT->owrt()
             << std::endl;
      }
      displayDiag(diag, 74, *validationFareComponent[fuCount]._currentFareUsage, eoeDA._itemNo);
      if (eoeDA._diag614IsActive)
      {
        DiagCollectorGuard dcg10(diag, Diagnostic614);
        if (diag.enableFilter(Diagnostic614, 1, eoeDA._directionalityInfo.farePathNumber))
        {
          diag << "  FAILED" << std::endl;
        }
      }
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;

      return FAILCOMB;
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << "    MATCH - EOE REST IND - " << eoeRestInd << std::endl;
  }
  return PASSCOMB;
}

char
Combinations::processEndOnEndCxrPreference(const EndOnEndDataAccess& eoeDA, const size_t fuCount)
{
  if (UNLIKELY(!eoeDA._mustVerifyCxrPreference))
    return PASSCOMB;

  ValidationFareComponents& validationFareComponent = eoeDA._validationFareComponent;

  if (validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->carrier() !=
          validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->carrier() &&
      validationFareComponent[fuCount]._currentFareUsage->paxTypeFare()->vendor() !=
          validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()->vendor())
  {
    if (!validationFareComponent.validateCarrierPreference(
            _trx->dataHandle(),
            validationFareComponent[fuCount]._currentFareUsage,
            validationFareComponent[fuCount]._targetFareUsage->paxTypeFare()))
    {
      validationFareComponent[fuCount].getSubCat(M104) = FAILCOMB;
      if (eoeDA._diag.isActive())
        eoeDA._diag << "     INVALID VENDOR COMBINATION IN CXR PREF" << std::endl;
      return FAILCOMB;
    }
  }
  return PASSCOMB;
}

void
Combinations::diagEndOnEndPassBlankNmlSpclInd(const EndOnEndDataAccess& eoeDA,
                                              const PaxTypeFare& targetPTF)
{
  DiagCollector& diag = eoeDA._diag;
  if (LIKELY(!diag.isActive()))
    return;

  if (targetPTF.isNormal())
  {
    diag << "    MATCH - EOE NORMAL IND - " << eoeDA._endOnEndRule.eoeNormalInd() << std::endl;
  }
  else
  {
    diag << "    MATCH - EOE SPECIAL IND - " << eoeDA._endOnEndRule.eoespecialInd() << std::endl;
  }
}

void
Combinations::diagEndOnEndPassBlankDomIntlTransInd(const EndOnEndDataAccess& eoeDA,
                                                   const Fare& targetFare)
{
  DiagCollector& diag = eoeDA._diag;

  if (LIKELY(!diag.isActive()))
    return;

  if (targetFare.isDomestic() || targetFare.isForeignDomestic())
    diag << "    MATCH - DOMESTIC IND - " << eoeDA._endOnEndRule.domInd() << std::endl;
  else if (targetFare.isTransborder())
    diag << "    MATCH - US/CA TRANSBORDER IND - " << eoeDA._endOnEndRule.uscatransborderInd()
         << std::endl;
  else if (targetFare.isInternational())
    diag << "    MATCH - INTERNATIONAL IND - " << eoeDA._endOnEndRule.intlInd() << std::endl;
}

void
Combinations::ValidationFareComponents::getNotPassedFC(FareUsage*& failedFareUsage,
                                                       FareUsage*& failedEOETargetFareUsage)
{
  std::vector<ValidationElement>::reverse_iterator vfcItR = this->rbegin();
  const std::vector<ValidationElement>::reverse_iterator vfcItEndR = this->rend();

  for (; vfcItR != vfcItEndR; vfcItR++)
  {
    if (!(*vfcItR)._passMajor)
    {
      failedFareUsage = (*vfcItR)._currentFareUsage;
      failedEOETargetFareUsage = (*vfcItR)._targetFareUsage;
      return;
    }
  }
}

void
Combinations::ValidationFareComponents::diagNotPassedFC(DiagCollector& diag)
{
  if (LIKELY(!diag.isActive()))
    return;

  diag << "   NOT PASSED - ";

  for (const ValidationElement& element : *this)
  {
    if (!element._passMajor)
    {
      diag << element._targetFareUsage->paxTypeFare()->createFareBasis(nullptr) << " ";
    }
  }
  diag << "\n";
}

PaxTypeFare*
Combinations::fareChangedCat10overrideCat25(PaxTypeFare* fare, const PricingUnit* pu)
{
  if (pu->isFareChangedCat10overrideCat25(fare))
  {
    return pu->getOldFareCat10overrideCat25(fare);
  }
  else
  {
    return fare;
  }
}

PaxTypeFare*
Combinations::validateFareCat10overrideCat25(const PaxTypeFare* fare1,
                                             PaxTypeFare* fare2,
                                             const PricingUnit* pu)
{
  if (fare1->isFareByRule())
  {
    return fareChangedCat10overrideCat25(fare2, pu);
  }
  else
  {
    if (pu->isFareChangedCat10overrideCat25(fare1))
    {
      if (fare2->isFareByRule())
      {
        PaxTypeFare* fare = fare2->getBaseFare();
        if (fare)
          return fare;
        return fare2;
      }
      else
      {
        return fare2;
      }
    }
    else
    {
      return fareChangedCat10overrideCat25(fare2, pu);
    }
  }
}
}
