//----------------------------------------------------------------------------
//
//  File:InclusionCodeQualifier.cpp
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/InclusionCodeQualifier.h"

#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/MiscFareTag.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/ValidatorBuilder.h"
#include "FareDisplay/WebICValidator.h"
#include "Rules/RuleConst.h"

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

InclusionCodeQualifier::InclusionCodeQualifier() {}

InclusionCodeQualifier::~InclusionCodeQualifier() {}

const tse::PaxTypeFare::FareDisplayState
InclusionCodeQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL INC CODE");

  if (!qualifyInclusionCode(trx, ptFare))
  {
    LOG4CXX_DEBUG(_logger, "!!!! invalidating: " << ptFare.fareClass());
    return PaxTypeFare::FD_Inclusion_Code;
  }

  return retProc(trx, ptFare);
}

bool
InclusionCodeQualifier::qualifyInclusionCode(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  LOG4CXX_INFO(_logger, "Entered FareSelector::qualifyInclusionCode()");

  // Pass all ADDONS
  // TODO: IF it is AD inclusion code, we should not even create an Inclsuion Code Qualifier
  if (trx.getRequest()->inclusionCode() == ADDON_FARES)
    return true;

  if (ptFare.miscFareTag() && ptFare.miscFareTag()->proportionalInd() == RuleConst::PROPORTIONAL)
  {
    return (trx.getRequest()->inclusionCode() == ASEAN_ADDON_FARES);
  }

  if (ptFare.isValidForPricing())
  {
    if(!_inclusionCodeValidators.empty())
      return qualifyFareForMultiInclCodes(trx, ptFare);

    return qualifyFare(trx, ptFare);
  }
  return false;
}

bool
InclusionCodeQualifier::qualifyFare(const FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  uint16_t noOfValidators = _validators.size();
  uint16_t failedCount(0);

  bool invalidDT = false;
  bool invalidFT = false;
  bool invalidPT = false;

  std::vector<Validator*>::const_iterator iter(_validators.begin());
  std::vector<Validator*>::const_iterator iterE(_validators.end());

  if(!fallback::fallbackFareDisplayByCabinActivation(&trx))
  {
    if(!validatorPassed(_validators, ptFare, failedCount, invalidDT, invalidFT, invalidPT))
      return false;
  }
  else
  {
    for (; iter != iterE; ++iter)
    {
      if ((*iter)->validate(ptFare))
      {
        if ((*iter)->restriction(ptFare) == Validator::OR)
          return true;
      }
      else
      {
        ++failedCount;
        if ((*iter)->restriction(ptFare) == Validator::AND)
          return false;

        (*iter)->failValidator(invalidDT, invalidFT, invalidPT);
      }
    }
  }

  if (failedCount == 0)
    return true;

  if (failedAll(noOfValidators, failedCount))
    return false;

  iter = _validators.begin();
  if (!(*iter)->validateMixedRestriction(invalidDT, invalidFT, invalidPT))
    return false;

  return true;
}

bool
InclusionCodeQualifier::validatorPassed(const std::vector<Validator*>& validators,
                                        const PaxTypeFare& ptFare,
                                        uint16_t& failedCount, bool& invalidDT,
                                        bool& invalidFT, bool& invalidPT) const
{
  for (auto validator : validators)
  {
    if (validator->validate(ptFare))
    {
      if (validator->restriction(ptFare) == Validator::OR)
        return true;
    }
    else
    {
      ++failedCount;
      if (validator->restriction(ptFare) == Validator::AND)
        return false;

      validator->failValidator(invalidDT, invalidFT, invalidPT);
    }
  }
  return true;
}

bool
InclusionCodeQualifier::qualifyFareForMultiInclCodes(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  std::map<uint8_t, std::vector<Validator*> >::const_iterator iterVec =
                                  _inclusionCodeValidators.begin();
  std::map<uint8_t, std::vector<Validator*> >::const_iterator iterE =
                                  _inclusionCodeValidators.end();

  uint16_t noOfValidators(0);

  for(; iterVec != iterE; ++iterVec)
  {
    if(iterVec->second.empty())
      continue;

    noOfValidators = iterVec->second.size();
    uint16_t failedCount(0);

    bool invalidDT = false;
    bool invalidFT = false;
    bool invalidPT = false;

    if(!validatorPassed(iterVec->second, ptFare, failedCount, invalidDT, invalidFT, invalidPT))
    {
      setStatusIntoMap(ptFare, iterVec->first, false);
      continue;
    }

    if (failedCount == 0)
    {
      // populate InclCode map in the PaxType fare its a new map < uint_8, bool> with PASS value
      setStatusIntoMap(ptFare, iterVec->first, true);
      continue;
    }

    if (failedAll(noOfValidators, failedCount))
    {
      //populate InclCode map in the PaxType fare its a new map < uint_8, bool> with FAIL value
      setStatusIntoMap(ptFare, iterVec->first, false);
      continue;
    }

    auto iter = iterVec->second.begin();
    if (!(*iter)->validateMixedRestriction(invalidDT, invalidFT, invalidPT))
    {
      //populate InclCode map in the PaxType fare its a new map < uint_8, bool> with FAIL value
      setStatusIntoMap(ptFare, iterVec->first, false);
      continue;
    }
    else
    {
      // populate InclCode map in the PaxType fare its a new map < uint_8, bool> with PASS value
      setStatusIntoMap(ptFare, iterVec->first, true);
      continue;
    }
  }

  //  analyze the new map in the PaxTypeFare for the content of the map
  //  - return false - if all Incl has value FAIL
  //  - return true - if at least one incl code has PASS
  //                - or, map is empty
  if(ptFare.fareStatusPerInclCode().empty())
    return true;

  std::map<uint8_t, bool> inclCodeFareStatus = ptFare.fareStatusPerInclCode();
  std::map<uint8_t, bool>::iterator ptfIter = inclCodeFareStatus.begin();
  std::map<uint8_t, bool>::iterator ptfIterEnd = inclCodeFareStatus.end();
  for (; ptfIter != ptfIterEnd; ++ptfIter)
  {
    if(ptfIter->second)
     return true;
  }
  return false;
}

void
InclusionCodeQualifier::setStatusIntoMap(const PaxTypeFare& ptFare, const uint8_t& number, bool status) const
{
   const_cast<PaxTypeFare&>(ptFare).setFareStatusForInclCode(number, status);
}

bool
InclusionCodeQualifier::failedAll(const uint16_t& noOfValidators, const uint16_t count) const
{
  return (noOfValidators == count);
}

void
InclusionCodeQualifier::build(FareDisplayTrx& trx)
{
  ValidatorBuilder builder(trx, _validators, _inclusionCodeValidators);
  builder.build();
}

bool
InclusionCodeQualifier::setup(FareDisplayTrx& trx)
{
  if (trx.getRequest()->inclusionCode() == ALL_FARES &&
      trx.getRequest()->displayPassengerTypes().empty() && !trx.getOptions()->isChildFares() &&
      !trx.getOptions()->isInfantFares() && !trx.getOptions()->isAdultFares())
  {
    return false; // NOTHING TO VALIDATE
  }
  else
  {
    build(trx);
    if(!fallback::fallbackFareDisplayByCabinActivation(&trx) &&
       trx.getRequest()->multiInclusionCodes())
      return !_inclusionCodeValidators.empty();

    return !_validators.empty();
  }
}
}
