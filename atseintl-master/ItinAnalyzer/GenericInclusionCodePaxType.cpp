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


#include "ItinAnalyzer/GenericInclusionCodePaxType.h"

#include "Common/PaxTypeFilter.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDispCldInfPsgType.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/FareDispRec8PsgType.h"
#include "FareDisplay/InclusionCodeConsts.h"

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.ItinAnalyzerService.InclusionCodePaxType");

void
GenericInclusionCodePaxType::getPaxType(FareDisplayTrx& trx)
{
  if (trx.getRequest()->inclusionCode() == NET_FARES)
  {
    LOG4CXX_DEBUG(logger, " Retrieving ALL Pax Types for NET Inclusion Code");

    if (trx.isShortRD())
    {
      PaxTypeFilter::getAllPaxType(trx, trx.getRequest()->passengerTypes());
      PaxTypeFilter::getAllPaxType(trx, trx.getRequest()->rec8PassengerTypes());
    }
    else
    {
      PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->passengerTypes());
      PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->rec8PassengerTypes());
    }
  }
  else if (isAllAdultNeeded(*_inclusionCode, *trx.getOptions()))
  {
    LOG4CXX_DEBUG(logger,
                  " Retrieving ALL ADULT Pax Type for " << trx.getRequest()->inclusionCode() << " "
                                                        << "Inclusion Code");
    PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->passengerTypes());
    PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->rec8PassengerTypes());
  }
  else if (!fallback::fallbackFareDisplayByCabinActivation(&trx) &&
           trx.getRequest()->multiInclusionCodes())
  {
    return collectPaxTypesForMultiInclusionCodesRequest(trx);
  }
  else
  {
    LOG4CXX_DEBUG(logger,
                  " Retrieving Record1 Pax Types for " << trx.getRequest()->inclusionCode()
                                                       << "Inclusion Code");
    // Load record 1 passenger types
    const std::vector<FareDispRec1PsgType*>& rec1PsgTypes =
        trx.dataHandle().getFareDispRec1PsgType(_inclusionCode->userApplType(),
                                                _inclusionCode->userAppl(),
                                                _inclusionCode->pseudoCityType(),
                                                _inclusionCode->pseudoCity(),
                                                _inclusionCode->inclusionCode());
    if (!rec1PsgTypes.empty())
    {
      LOG4CXX_DEBUG(logger, "REC1 types: " << rec1PsgTypes.size());
      std::vector<FareDispRec1PsgType*>::const_iterator iter = rec1PsgTypes.begin();
      std::vector<FareDispRec1PsgType*>::const_iterator iterEnd = rec1PsgTypes.end();
      for (; iter != iterEnd; iter++)
      {
        LOG4CXX_DEBUG(logger, "Inserting    : " << (*iter)->psgType());
        trx.getRequest()->passengerTypes().insert((*iter)->psgType());
      }
    }

    // Load record 8 passenger types
    const std::vector<FareDispRec8PsgType*>& rec8PsgTypes =
        trx.dataHandle().getFareDispRec8PsgType(_inclusionCode->userApplType(),
                                                _inclusionCode->userAppl(),
                                                _inclusionCode->pseudoCityType(),
                                                _inclusionCode->pseudoCity(),
                                                _inclusionCode->inclusionCode());
    if (!rec8PsgTypes.empty())
    {
      LOG4CXX_DEBUG(logger, "REC8 types: " << rec8PsgTypes.size());
      std::vector<FareDispRec8PsgType*>::const_iterator iter = rec8PsgTypes.begin();
      std::vector<FareDispRec8PsgType*>::const_iterator iterEnd = rec8PsgTypes.end();
      for (; iter != iterEnd; iter++)
      {
        trx.getRequest()->rec8PassengerTypes().insert((*iter)->psgType());
        trx.getRequest()->passengerTypes().insert((*iter)->psgType());
      }
    }
  }

  getAdditionalPaxType(trx);

  if (trx.getRequest()->passengerTypes().empty())
  {
    LOG4CXX_ERROR(logger, "No Pax Type was Populated.--Adding ADT as default : ")
    getDefaultPaxCode(trx);
  }
}

bool
GenericInclusionCodePaxType::isAllAdultNeeded(const FareDisplayInclCd& inclCd,
                                              const FareDisplayOptions& options) const
{
  if (inclCd.fareTypeAndOrPsgType() == GenericInclusionCodePaxType::OR ||
      inclCd.displTypeAndOrPsgType() == GenericInclusionCodePaxType::OR)
  {
    if (options.isAdultFares())
      return true;
    else if (options.isChildFares() || options.isInfantFares())
      return false;
    else
      return true;
  }
  return false;
}

void
GenericInclusionCodePaxType::collectPaxTypesForMultiInclusionCodesRequest(FareDisplayTrx& trx)
{
   std::set<PaxTypeCode> paxTypesForInclCode;
   std::set<PaxTypeCode> rec8PaxTypesForInclCode;

   LOG4CXX_DEBUG(logger,
   " Retrieving Record1 Pax Types for MultiInclCode " << trx.getRequest()->inclusionCode()
                                                       << "Inclusion Code");
   // Load record 1 passenger types
   const std::vector<FareDispRec1PsgType*>& rec1PsgTypes =
        trx.dataHandle().getFareDispRec1PsgType(_inclusionCode->userApplType(),
                                                _inclusionCode->userAppl(),
                                                _inclusionCode->pseudoCityType(),
                                                _inclusionCode->pseudoCity(),
                                                _inclusionCode->inclusionCode());
   if (!rec1PsgTypes.empty())
   {
     LOG4CXX_DEBUG(logger, "REC1 types: " << rec1PsgTypes.size());
     std::vector<FareDispRec1PsgType*>::const_iterator iter = rec1PsgTypes.begin();
     std::vector<FareDispRec1PsgType*>::const_iterator iterEnd = rec1PsgTypes.end();
     for (; iter != iterEnd; iter++)
     {
       LOG4CXX_DEBUG(logger, "Inserting    : " << (*iter)->psgType());
       paxTypesForInclCode.insert((*iter)->psgType());
     }
   }

   const std::vector<FareDispRec8PsgType*>& rec8PsgTypes =
        trx.dataHandle().getFareDispRec8PsgType(_inclusionCode->userApplType(),
                                                _inclusionCode->userAppl(),
                                                _inclusionCode->pseudoCityType(),
                                                _inclusionCode->pseudoCity(),
                                                _inclusionCode->inclusionCode());
   if (!rec8PsgTypes.empty())
   {
     LOG4CXX_DEBUG(logger, "REC8 types: " << rec8PsgTypes.size());
     std::vector<FareDispRec8PsgType*>::const_iterator iter = rec8PsgTypes.begin();
     std::vector<FareDispRec8PsgType*>::const_iterator iterEnd = rec8PsgTypes.end();
     for (; iter != iterEnd; iter++)
     {
       paxTypesForInclCode.insert((*iter)->psgType());
       rec8PaxTypesForInclCode.insert((*iter)->psgType());
     }
   }

   LOG4CXX_DEBUG(logger, "Now Adding Additional Pax Type: ");
   if (trx.getOptions() && trx.getOptions()->isRtw())
   {
     if (!trx.getRequest()->displayPassengerTypes().empty())
       paxTypesForInclCode.insert(
               trx.getRequest()->displayPassengerTypes().begin(),
               trx.getRequest()->displayPassengerTypes().end());
     else
     {
       paxTypesForInclCode.insert(
                      trx.getRequest()->inputPassengerTypes().begin(),
                      trx.getRequest()->inputPassengerTypes().end());
       getDiscountPaxTypes(trx, paxTypesForInclCode, rec8PaxTypesForInclCode);
     }
   }
   else
   {
     if (trx.getRequest()->displayPassengerTypes().empty())
     {
       getDiscountPaxTypes(trx, paxTypesForInclCode, rec8PaxTypesForInclCode);
     }
     else
       if (!trx.getRequest()->displayPassengerTypes().empty())
       {
         std::copy(trx.getRequest()->displayPassengerTypes().begin(),
                   trx.getRequest()->displayPassengerTypes().end(),
                   std::inserter(paxTypesForInclCode,
                                 paxTypesForInclCode.begin()));
       }
   }

   if (paxTypesForInclCode.empty())
   {
     LOG4CXX_ERROR(logger, "No Pax Type was Populated.--Adding ADT as default : ")
     paxTypesForInclCode.insert(ADULT);
   }
   // Now populate Request()->passengerTypes() AND Request()->rec8PassengerTypes()
   if (!paxTypesForInclCode.empty())
   {
      std::copy(paxTypesForInclCode.begin(),
                paxTypesForInclCode.end(),
                std::inserter(trx.getRequest()->passengerTypes(),
                              trx.getRequest()->passengerTypes().begin()));
   }
   if (!rec8PaxTypesForInclCode.empty())
   {
      std::copy(rec8PaxTypesForInclCode.begin(),
                rec8PaxTypesForInclCode.end(),
                std::inserter(trx.getRequest()->rec8PassengerTypes(),
                              trx.getRequest()->rec8PassengerTypes().begin()));
   }
   trx.getRequest()->paxTypesPerInclCodeMap().insert(std::make_pair(
        trx.getRequest()->inclusionNumber(trx.getRequest()->inclusionCode()),
              std::make_pair(paxTypesForInclCode, rec8PaxTypesForInclCode)));
}

void
GenericInclusionCodePaxType::getDiscountPaxTypes(FareDisplayTrx& trx,
                                          std::set<PaxTypeCode>& paxTypesForInclCode,
                                          std::set<PaxTypeCode>& rec8PaxTypesForInclCode)
{
  if (trx.getOptions()->isChildFares())
  {
    LOG4CXX_DEBUG(logger, " Getting All Child PaxTypes ");
    PaxTypeFilter::getAllChildPaxType(trx, paxTypesForInclCode);
    PaxTypeFilter::getAllChildPaxType(trx, rec8PaxTypesForInclCode);
  }

  if (trx.getOptions()->isInfantFares())
  {
    LOG4CXX_DEBUG(logger, " Getting All Infant PaxTypes ");
    PaxTypeFilter::getAllInfantPaxType(trx, paxTypesForInclCode);
    PaxTypeFilter::getAllInfantPaxType(trx, rec8PaxTypesForInclCode);
  }
}

}
