//----------------------------------------------------------------------------
//
//  File:           PaxTypeUtil.cpp
//  Created:        4/7/2004
//  Authors:
//
//  Description:    Common functions required for ATSE shopping/pricing.
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
//----------------------------------------------------------------------------

#include "Common/PaxTypeUtil.h"

#include "Common/AltPricingUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "Common/TSSCacheCommon.h"
#include "Common/Vendor.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/PaxTypeMatrix.h"

#include <algorithm>
#include <set>

namespace tse
{
FALLBACK_DECL(fallbackAB240SupportInvalidPaxType);

static Logger
logger("atseintl.Common.PaxTypeUtil");

const boost::regex PaxTypeUtil::PAX_WITH_SPECIFIED_AGE = boost::regex("^\\D\\d{2}");
const boost::regex PaxTypeUtil::PAX_WITH_UNSPECIFIED_AGE =
    boost::regex("^\\DNN", boost::regex_constants::icase);

bool
PaxTypeUtil::initialize(PricingTrx& trx,
                        PaxType& requestedPaxType,
                        PaxTypeCode& paxTypeCode,
                        uint16_t number,
                        uint16_t age,
                        StateCode& stateCode,
                        uint16_t inputOrder)
{
  requestedPaxType.paxType() = paxTypeCode;
  requestedPaxType.number() = number;
  requestedPaxType.age() = age;
  requestedPaxType.stateCode() = stateCode;
  requestedPaxType.inputOrder() = inputOrder;
  requestedPaxType.paxTypeInfo() = nullptr;

  // determine the vendor
  getVendorCode(trx.dataHandle(), requestedPaxType);

  const PaxTypeInfo* paxInfo = requestedPaxType.paxTypeInfo();
  if (paxInfo == nullptr)
  {
    if (!fallback::fallbackAB240SupportInvalidPaxType(&trx))
    {
      if (trx.activationFlags().isAB240())
        return false;
      else
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID PASSENGER TYPE");
    }
    else
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID PASSENGER TYPE");
    }
  }

  // Add the requested pax type itself for ANY_CARRIER
  addPaxTypeToMap(trx,
                  requestedPaxType.actualPaxType(),
                  requestedPaxType.paxType(),
                  requestedPaxType.number(),
                  requestedPaxType.age(),
                  requestedPaxType.stateCode(),
                  requestedPaxType.vendorCode(),
                  ANY_CARRIER,
                  requestedPaxType.maxPenaltyInfo());

  // get the vendor from the PaxTypeMatrix and set the paxTypeInfo pointer
  const std::vector<const PaxTypeMatrix*>& paxTypeMatrixList =
      trx.dataHandle().getPaxTypeMatrix(requestedPaxType.paxType());

  // because the vendor is Sabre create a new pax type and put it in the pax type vector
  for (const PaxTypeMatrix* const i : paxTypeMatrixList)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    i->atpPaxType(),
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), i->atpPaxType()),
                    i->carrier(),
                    requestedPaxType.maxPenaltyInfo());
  }

  // now we need to add paxTypes for adult, child, and senior based on the flags in the
  // requested passenger types paxInfo

  if ((paxInfo->adultInd() == 'Y') && (requestedPaxType.paxType() != ADULT))
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    ADULT,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), ADULT),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }

  if ((paxInfo->childInd() == 'Y') && (requestedPaxType.paxType() != CHILD))
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    CHILD,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), CHILD),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }

  if ((paxInfo->infantInd() == 'Y') && (requestedPaxType.paxType() != INFANT))
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    INFANT,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), INFANT),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }

  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

  if (fcConfig == nullptr)
  {
    LOG4CXX_FATAL(logger, "Unable to retrieve Fare Calc Config");
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED);
  }

  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  bool exchangeTrxAM = false;
  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX &&
      (static_cast<ExchangePricingTrx&>(trx)).reqType() == AGENT_PRICING_MASK)
    exchangeTrxAM = true;

  // TODO: In both cases brackets were added to remove warning -Wparentheses
  // using operatos predecense (&& goes befor ||), but it is not obvious if this
  // is what author had in mind...
  if (!noPNRPricingTrx && !exchangeTrxAM &&
      ((!trx.getOptions()->isFareFamilyType() && !trx.getOptions()->fareX() &&
           fcConfig->negPermitted() == NEG_FARES_PERMITTED_NO_MESSAGE) ||
       fcConfig->negPermitted() == NEG_FARES_PERMITTED_WITH_MESSAGE))
    createLowFareSearchNegPaxTypes(trx, requestedPaxType);

  if (noPNRPricingTrx)
  {
    noPNRPricingTrx->loadNoPNROptions();
    const NoPNROptions* wqcc = noPNRPricingTrx->noPNROptions();

    if (wqcc->negPassengerTypeMapping() == MAP_ADT_TO_NEG)
    {
      createLowFareSearchNegPaxTypes(trx, requestedPaxType);
    }
  }

  // now do sabre special transalations
  if (requestedPaxType.paxType() == CNE)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    NEG,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), NEG),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == INE)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    CNE,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), CNE),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());

    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    NEG,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), NEG),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == CBC)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    PFA,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), PFA),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == CBI)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    CBC,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), CBC),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());

    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    PFA,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), PFA),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == JNN)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    JCB,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), JCB),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == JNF)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    JCB,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), JCB),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());

    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    JNN,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), JCB),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == JNS)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    JCB,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), JCB),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());

    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    JNN,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), JCB),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == CSB)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    AST,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), AST),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == CHR)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    ADR,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), ADR),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == UNR)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    ADR,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), ADR),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  else if (requestedPaxType.paxType() == FNN)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    FLY,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), FLY),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
  // end of sabre special transalations

  return true;
}

void
PaxTypeUtil::initialize(FareDisplayTrx& trx)
{
  int16_t inputOrder = 1;

  // The request passenger list includes the sum total of both requested
  // passenger types and record 1 passenger types.
  for (const PaxTypeCode& ptCode : trx.getRequest()->passengerTypes())
  {
    PaxType* paxType;
    trx.dataHandle().get(paxType); // lint !e530

    if (PaxTypeUtil::isPaxWithSpecifiedAge(ptCode))
    {
      paxType->paxType() = PaxTypeUtil::getPaxWithUnspecifiedAge(ptCode.c_str()[0]);
      paxType->number() = 1;
      paxType->age() = uint16_t(atoi(&(ptCode.c_str()[1])));
    }
    else
    {
      paxType->paxType() = ptCode;
      paxType->number() = 1;
      if (ptCode == CHILD)
        paxType->age() = 11;
      else
        paxType->age() = 0;
    }

    paxType->stateCode() = "";
    paxType->inputOrder() = inputOrder++;
    getVendorCode(trx.dataHandle(), *paxType);

    if (paxType->paxTypeInfo() == nullptr)
    {
      continue; // skip this item
    }

    addPaxTypeToMap(trx,
                    paxType->actualPaxType(),
                    paxType->paxType(),
                    paxType->number(),
                    paxType->age(),
                    paxType->stateCode(),
                    paxType->vendorCode(),
                    ANY_CARRIER,
                    paxType->maxPenaltyInfo());
    trx.paxType().push_back(paxType);
  }

  if (trx.getRequest()->passengerTypes().empty())
    LOG4CXX_ERROR(logger, "!! NO FareDisplayTrx input passenger types");

  // Record 8 passenger types
  inputOrder = 1;

  for (const PaxTypeCode& ptCode : trx.getRequest()->rec8PassengerTypes())
  {
    PaxType* paxType;
    trx.dataHandle().get(paxType); // lint !e530

    paxType->paxType() = ptCode;
    paxType->number() = 1;
    if (ptCode == CHILD)
      paxType->age() = 11;
    else
      paxType->age() = 0;

    paxType->stateCode() = "";
    paxType->inputOrder() = inputOrder++;
    getVendorCode(trx.dataHandle(), *paxType);

    if (paxType->paxTypeInfo() == nullptr)
    {
      continue; // skip this item
    }

    addPaxTypeToMap(trx,
                    paxType->actualPaxType(),
                    paxType->paxType(),
                    paxType->number(),
                    paxType->age(),
                    paxType->stateCode(),
                    paxType->vendorCode(),
                    ANY_CARRIER,
                    paxType->maxPenaltyInfo());
    trx.paxTypeRec8().push_back(paxType);
  }

  if (trx.getRequest()->rec8PassengerTypes().empty())
    LOG4CXX_WARN(logger, "!! NO REC8 FareDisplayTrx input passenger types");
}

bool
PaxTypeUtil::addPaxTypeToMap(PricingTrx& trx,
                             std::map<CarrierCode, std::vector<PaxType*>*>& paxTypeMap,
                             const PaxTypeCode& paxTypeCode,
                             const uint16_t number,
                             const uint16_t age,
                             const StateCode& stateCode,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             MaxPenaltyInfo* maxPenaltyInfo)
{
  PaxType* paxType = nullptr;
  trx.dataHandle().get(paxType);
  std::vector<PaxType*>* paxTypeVec;

  // lint --e{413}
  paxType->paxType() = paxTypeCode;
  paxType->number() = number;
  paxType->age() = age;
  paxType->stateCode() = stateCode;
  paxType->vendorCode() = vendor;
  paxType->paxTypeInfo() = trx.dataHandle().getPaxType(paxTypeCode, vendor);
  paxType->maxPenaltyInfo() = maxPenaltyInfo;

  CarrierCode cxr = carrier;
  if (cxr.empty())
    cxr = ANY_CARRIER;

  paxTypeVec = paxTypeMap[cxr];

  if (paxTypeVec)
  {
    paxTypeVec->push_back(paxType);
  }
  else
  {
    std::vector<PaxType*>* paxTypeVecPtr = nullptr;
    trx.dataHandle().get(paxTypeVecPtr);
    paxTypeVecPtr->push_back(paxType);
    paxTypeMap[cxr] = paxTypeVecPtr;
  }
  return true;
}

const PaxType*
PaxTypeUtil::isAnActualPaxInTrxImpl(const PricingTrx& trx,
                                    const CarrierCode& carrier,
                                    const PaxTypeCode& paxTypeCode)

{
  for (const PaxType* const reqPaxType : trx.paxType())
  {
    const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypeMap =
        reqPaxType->actualPaxType();

    const auto& actualPaxTypeReq = actualPaxTypeMap.find(carrier);
    if (UNLIKELY(actualPaxTypeReq != actualPaxTypeMap.end()))
    {
      for (const PaxType* const actualPT : *actualPaxTypeReq->second)
      {
        if (actualPT->paxType() == paxTypeCode)
          return actualPT;
      }
    }

    // if no match for requested carrier look for ANY_CARRIER
    const auto& actualPaxTypeAny = actualPaxTypeMap.find(ANY_CARRIER);
    if (LIKELY(actualPaxTypeAny != actualPaxTypeMap.end()))
    {
      for (const auto& actualPT : *actualPaxTypeAny->second)
      {
        if (actualPT->paxType() == paxTypeCode)
          return actualPT;
      }
    }
  }

  return nullptr; // if not found return null
}

const PaxType*
PaxTypeUtil::isAnActualPaxInTrx(PricingTrx& trx,
                                const CarrierCode& carrier,
                                const PaxTypeCode& paxTypeCode)

{
  return tsscache::isAnActualPaxInTrx(trx, carrier, paxTypeCode, int(trx.getBaseIntId()));
}

bool
PaxTypeUtil::isPaxInTrx(const PricingTrx& trx, const std::vector<PaxTypeCode>& paxTypeCodes)
{
  for (const PaxType* const actualPT : trx.paxType())
  {
    for (const PaxTypeCode& paxCode : paxTypeCodes)
    {
      if (actualPT->paxType() == paxCode)
      {
        return true;
      }
    }
  }
  return false;
}

const PaxType*
PaxTypeUtil::isAnActualPaxInTrx(PricingTrx& trx,
                                const CarrierCode& carrier,
                                const PaxTypeCode& paxTypeCode,
                                const uint16_t minAge,
                                const uint16_t maxAge)
{
  for (const PaxType* const reqPaxType : trx.paxType())
  {
    const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypeMap =
        reqPaxType->actualPaxType();

    const auto& actualPaxTypeReq = actualPaxTypeMap.find(carrier);
    if (UNLIKELY(actualPaxTypeReq != actualPaxTypeMap.end()))
    {
      for (const PaxType* const actualPT : *actualPaxTypeReq->second)
      {
        if (actualPT->paxType() == paxTypeCode)
        {
          if (actualPT->age() > 0)
          {
            if (((minAge > 0) && (minAge > actualPT->age())) ||
                ((maxAge > 0) && (maxAge < actualPT->age())))
              continue;
          }

          return actualPT;
        }
      }
    }

    // if no match for requested carrier look for ANY_CARRIER
    const auto& actualPaxTypeAny = actualPaxTypeMap.find(ANY_CARRIER);
    if (LIKELY(actualPaxTypeAny != actualPaxTypeMap.end()))
    {
      for (const PaxType* const actualPT : *actualPaxTypeAny->second)
      {
        if (actualPT->paxType() == paxTypeCode)
        {
          if (actualPT->age() > 0)
          {
            if (((minAge > 0) && (minAge > actualPT->age())) ||
                ((maxAge > 0) && (maxAge < actualPT->age())))
              continue;
          }

          return actualPT;
        }
      }
    }
  }

  return nullptr; // if not found return null
}

// check if paxType is actually of paxTypeCode
bool
PaxTypeUtil::isAnActualPax(const PaxType& paxType,
                           const CarrierCode& carrier,
                           const PaxTypeCode& paxTypeCode,
                           const uint16_t minAge,
                           const uint16_t maxAge)
{
  bool matched = false;

  const uint16_t paxAge = paxType.age();
  if (paxAge != 0)
  {
    if ((minAge != 0) && (paxAge < minAge))
      return false;
    if ((maxAge != 0) && (paxAge > maxAge))
      return false;
  }

  // try fast match first
  if (paxType.paxType() == paxTypeCode)
    return true;

  const PaxTypeInfo& paxTypeInfo = paxType.paxTypeInfo();
  if (paxTypeCode == ADULT)
  {
    if (paxTypeInfo.isChild() || paxTypeInfo.isInfant())
      return false;
    else
      return true;
  }
  else if (paxTypeCode == CHILD)
  {
    return paxTypeInfo.isChild();
  }
  else if (paxTypeCode == INFANT)
  {
    return paxTypeInfo.isInfant();
  }

  // try actual PaxType Map
  const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypeMap = paxType.actualPaxType();

  std::map<CarrierCode, std::vector<PaxType*>*>::const_iterator actualPaxTypeMapI =
      actualPaxTypeMap.find(carrier);

  if (UNLIKELY(actualPaxTypeMapI != actualPaxTypeMap.end()))
  {
    matched = findMatch(*(actualPaxTypeMapI->second), paxTypeCode);
  }

  if (LIKELY(!matched))
  {
    // try ANY_CARRIER
    actualPaxTypeMapI = actualPaxTypeMap.find(ANY_CARRIER);
    if (LIKELY(actualPaxTypeMapI != actualPaxTypeMap.end()))
    {
      matched = findMatch(*(actualPaxTypeMapI->second), paxTypeCode);
    }
  }

  return matched;
}

const PaxType*
PaxTypeUtil::isAdultInTrx(PricingTrx& trx)
{
  for (const PaxType* const paxType : trx.paxType())
  {
    if (paxType->paxType() == tse::ADULT)
      return paxType;

    const PaxTypeInfo& psgTypeInfo = paxType->paxTypeInfo();
    if (!psgTypeInfo.isChild() && !psgTypeInfo.isInfant())
      return paxType;
  }

  return  nullptr;
}

std::vector<PaxTypeCode>
PaxTypeUtil::retrievePaxTypes(const PricingTrx& trx)
{
  std::vector<PaxTypeCode> paxTypes;
  paxTypes.reserve(trx.paxType().size());

  for (const auto paxType : trx.paxType())
    paxTypes.push_back(paxType->paxType());

  return paxTypes;
}

bool
PaxTypeUtil::isAdult(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  DataHandle dh;
  return isAdult(dh, paxTypeCode, vendor);
}

bool
PaxTypeUtil::isAdult(PricingTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  return isAdult(trx.dataHandle(), paxTypeCode, vendor);
}

bool
PaxTypeUtil::isAdult(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  if (isChild(dh, paxTypeCode, vendor) || isInfant(dh, paxTypeCode, vendor))
  {
    return false;
  }
  return true;
}

bool
PaxTypeUtil::isChild(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  DataHandle dh;
  return isChild(dh, paxTypeCode, vendor);
}

bool
PaxTypeUtil::isChildFD(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  DataHandle dh;
  return isChildFD(dh, paxTypeCode, vendor);
}

bool
PaxTypeUtil::isChild(PricingTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  return isChild(trx.dataHandle(), paxTypeCode, vendor);
}

bool
PaxTypeUtil::isChild(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  const PaxTypeInfo* paxInfo = dh.getPaxType(paxTypeCode, vendor);

  if (paxInfo != nullptr)
  {
    return paxInfo->isChild();
  }
  else if (PaxTypeUtil::isPaxWithSpecifiedAge(paxTypeCode))
  {
    paxInfo = dh.getPaxType(PaxTypeUtil::getPaxWithUnspecifiedAge(paxTypeCode[0]), vendor);
    return (paxInfo != nullptr && paxInfo->isChild());
  }

  return false;
}

bool
PaxTypeUtil::isChildFD(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  const PaxTypeInfo* paxInfo = dh.getPaxType(paxTypeCode, vendor);
  return (paxInfo != nullptr &&
          (paxInfo->isChild() || paxInfo->childInd() == 'Y' || paxInfo->psgGroupType() == "CH$"));
}

bool
PaxTypeUtil::isInfant(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  DataHandle dh;
  return isInfant(dh, paxTypeCode, vendor);
}

bool
PaxTypeUtil::isInfantFD(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  DataHandle dh;
  return isInfantFD(dh, paxTypeCode, vendor);
}

bool
PaxTypeUtil::isInfant(PricingTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  return isInfant(trx.dataHandle(), paxTypeCode, vendor);
}

bool
PaxTypeUtil::isInfant(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  const PaxTypeInfo* paxInfo = dh.getPaxType(paxTypeCode, vendor);
  return (paxInfo != nullptr && paxInfo->isInfant());
}

bool
PaxTypeUtil::isInfantFD(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  const PaxTypeInfo* paxInfo = dh.getPaxType(paxTypeCode, vendor);
  return (paxInfo != nullptr &&
          (paxInfo->isInfant() || paxInfo->infantInd() == 'Y' || paxInfo->psgGroupType() == "IN$"));
}

void
PaxTypeUtil::getVendorCode(DataHandle& dataHandle, PaxType& requestedPaxType)
{
  // determine the vendor

  if ((requestedPaxType.paxTypeInfo() =
           dataHandle.getPaxType(requestedPaxType.paxType(), Vendor::SABRE)))
  {
    requestedPaxType.vendorCode() = requestedPaxType.paxTypeInfo()->vendor();
  }
  else if ((requestedPaxType.paxTypeInfo() =
                dataHandle.getPaxType(requestedPaxType.paxType(), Vendor::ATPCO)))
  {
    requestedPaxType.vendorCode() = requestedPaxType.paxTypeInfo()->vendor();
  }
  else if ((requestedPaxType.paxTypeInfo() =
                dataHandle.getPaxType(requestedPaxType.paxType(), Vendor::SITA)))
  {
    requestedPaxType.vendorCode() = requestedPaxType.paxTypeInfo()->vendor();
  }
  else
  {
    LOG4CXX_ERROR(logger, "!! INVALID PASSENGER TYPE = " << requestedPaxType.paxType());
    requestedPaxType.vendorCode() = Vendor::SABRE;
  }
}

VendorCode
PaxTypeUtil::getVendorCode(DataHandle& dataHandle, const PaxTypeCode& paxTypeCode)
{
  // determine the vendor

  const PaxTypeInfo* paxTypeInfo = nullptr;
  if ((paxTypeInfo = dataHandle.getPaxType(paxTypeCode, Vendor::SABRE)))
  {
    return Vendor::SABRE;
  }
  else if ((paxTypeInfo = dataHandle.getPaxType(paxTypeCode, Vendor::ATPCO)))
  {
    return Vendor::ATPCO;
  }
  else if ((paxTypeInfo = dataHandle.getPaxType(paxTypeCode, Vendor::SITA)))
  {
    return Vendor::SITA;
  }
  else
  {
    LOG4CXX_ERROR(logger, "!! INVALID PASSENGER TYPE = " << paxTypeCode);
    return Vendor::SABRE;
  }
}

PaxTypeStatus
PaxTypeUtil::paxTypeStatus(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
{
  DataHandle dh;
  const PaxTypeInfo* paxInfo = dh.getPaxType(paxTypeCode, vendor);
  if (LIKELY(paxInfo != nullptr))
  {
    return static_cast<PaxTypeStatus>(paxInfo->paxTypeStatus());
  }
  return PAX_TYPE_STATUS_ADULT;
}

PaxTypeStatus
PaxTypeUtil::nextPaxTypeStatus(PaxTypeStatus paxTypeStatus)
{
  if (paxTypeStatus == PAX_TYPE_STATUS_UNKNOWN || paxTypeStatus == PAX_TYPE_STATUS_ADULT)
    return PAX_TYPE_STATUS_ADULT;

  if (paxTypeStatus == PAX_TYPE_STATUS_INFANT)
    return PAX_TYPE_STATUS_CHILD;

  if (paxTypeStatus == PAX_TYPE_STATUS_CHILD)
    return PAX_TYPE_STATUS_ADULT;

  return PAX_TYPE_STATUS_ADULT;
}

bool
PaxTypeUtil::findMatch(std::vector<PaxType*> const& paxTypeVec, const PaxTypeCode& paxTypeCode)
{
  return std::any_of(paxTypeVec.cbegin(),
                     paxTypeVec.cend(),
                     [paxTypeCode](const PaxType* const pt)
                     { return pt->paxType() == paxTypeCode; });
}

//-------------------------------------------------------------------
uint16_t
PaxTypeUtil::totalNumSeats(const PricingTrx& trx)
{
  uint32_t numSeats = 0;

  if (UNLIKELY(trx.paxType().front()->totalPaxNumber() && RexPricingTrx::isRexTrxAndNewItin(trx)))
    numSeats = trx.paxType().front()->paxTypeInfo()->numberSeatsReq() *
               trx.paxType().front()->totalPaxNumber();

  else
  {
    for (const PaxType* const paxType : trx.paxType())
    {
      if (UNLIKELY(paxType == nullptr))
        continue;
      numSeats += paxType->paxTypeInfo().numberSeatsReq() * paxType->number();
    }

    if (trx.isShopping())
    {
      const ShoppingTrx& shpTrx = static_cast<const ShoppingTrx&>(trx);
      numSeats += PaxTypeUtil::numSeatsForExcludedPaxTypes(shpTrx);
    }
  }

  return uint16_t(numSeats);
}

//-------------------------------------------------------------------
uint16_t
PaxTypeUtil::numSeatsForFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  // For a WPAS entry ALWAYS skip noMatchAvail from FCC
  if (UNLIKELY(trx.getRequest()->isWpas()))
    return 0;

  if (UNLIKELY(AltPricingUtil::ignoreAvail(trx) && !AltPricingUtil::isCat25SisterFare(paxTypeFare)))
    return 0;

  return totalNumSeats(trx);
}

//APO-43304:  return available seats for t999 restriction tags
uint16_t
PaxTypeUtil::numSeatsForFareWithoutIgnoreAvail(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  // For a WPAS entry ALWAYS skip noMatchAvail from FCC
  if (UNLIKELY(trx.getRequest()->isWpas()))
    return 0;

  return totalNumSeats(trx);
}
uint16_t
PaxTypeUtil::numSeatsForExcludedPaxTypes(const ShoppingTrx& trx)
{
  uint32_t numSeats = 0;

  for (const PaxType* paxType : trx.excludedPaxType())
  {
    numSeats += paxType->paxTypeInfo().numberSeatsReq() * paxType->number();
  }

  return uint16_t(numSeats);
}

bool
PaxTypeUtil::isATPCoPaxRollupMatch(PricingTrx& trx,
                                   const PaxTypeCode& paxChk,
                                   const PaxTypeCode& paxRef)
{
  const PaxTypeInfo* paxTypeChk = trx.dataHandle().getPaxType(paxChk, ATPCO_VENDOR_CODE);
  const PaxTypeInfo* paxTypeRef = trx.dataHandle().getPaxType(paxRef, ATPCO_VENDOR_CODE);

  if (UNLIKELY((paxTypeChk == nullptr) || (paxTypeRef == nullptr)))
    return false;

  // First the Infants ...
  if (paxTypeChk->infantInd() == 'Y')
  {
    if ((paxTypeRef->infantInd() == 'N') && (paxTypeRef->childInd() == 'N') &&
        (paxTypeRef->adultInd() == 'Y'))
      return false;
    else
      return true;
  }

  // ... Next, the Children ...
  if (paxTypeChk->childInd() == 'Y')
  {
    if ((paxTypeRef->childInd() == 'Y') || (paxTypeRef->adultInd() == 'Y'))
      return true;
    else
      return false;
  }

  // ... Finally, the Adults
  if (paxTypeChk->adultInd() == 'Y')
  {
    if (UNLIKELY((paxTypeRef->infantInd() == 'N') && (paxTypeRef->childInd() == 'N') &&
        (paxTypeRef->adultInd() == 'Y')))
      return true;
    else
      return false;
  }

  return false;
}

void
PaxTypeUtil::createLowFareSearchNegPaxTypes(PricingTrx& trx, PaxType& requestedPaxType)
{

  if (requestedPaxType.paxType() == ADULT || requestedPaxType.paxType() == CHILD ||
      requestedPaxType.paxType() == INFANT || requestedPaxType.paxType() == INS)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    NEG,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), NEG),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }

  if (requestedPaxType.paxType() == CHILD || requestedPaxType.paxType() == INFANT ||
      requestedPaxType.paxType() == INS)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    CNE,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), CNE),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }

  if (requestedPaxType.paxType() == INFANT)
  {
    addPaxTypeToMap(trx,
                    requestedPaxType.actualPaxType(),
                    INE,
                    requestedPaxType.number(),
                    requestedPaxType.age(),
                    requestedPaxType.stateCode(),
                    getVendorCode(trx.dataHandle(), INE),
                    ANY_CARRIER,
                    requestedPaxType.maxPenaltyInfo());
  }
}

void
PaxTypeUtil::createFareTypePricingPaxTypes(PricingTrx& trx, const std::set<PaxTypeCode>& psgTypes)
{
  for (PaxType* const requestedPaxType : trx.paxType())
  {
    for (const PaxTypeCode& ftPaxType : psgTypes)
    {
      if (requestedPaxType->paxType() == ftPaxType)
        continue;

      if (ftPaxType == ADULT || ftPaxType == CHILD || ftPaxType == INFANT)
        continue;

      PaxTypeStatus a, b;
      const VendorCode& vendor = requestedPaxType->paxTypeInfo()->vendor();
      if ((a = paxTypeStatus(requestedPaxType->paxType(), vendor)) >=
          (b = paxTypeStatus(ftPaxType, vendor)))
      {
        addPaxTypeToMap(trx,
                        requestedPaxType->actualPaxType(),
                        ftPaxType,
                        requestedPaxType->number(),
                        requestedPaxType->age(),
                        requestedPaxType->stateCode(),
                        getVendorCode(trx.dataHandle(), ftPaxType),
                        ANY_CARRIER,
                        requestedPaxType->maxPenaltyInfo());
      }
    }
  }
}

//-----------------------------------------------
bool
PaxTypeUtil::sabreVendorPaxType(PricingTrx& trx,
                                const PaxType& requestedPaxType,
                                const PaxTypeFare& paxTypeFare)
{
  PaxTypeCode farePaxType = paxTypeFare.fcasPaxType();
  if (farePaxType.empty())
    farePaxType = ADULT;

  const std::vector<const PaxTypeMatrix*>& paxTypeMatrixList =
      trx.dataHandle().getPaxTypeMatrix(requestedPaxType.paxType());

  for (const PaxTypeMatrix* const paxMatrix : paxTypeMatrixList)
  {
    if (requestedPaxType.paxType() == paxMatrix->sabrePaxType() &&
        farePaxType == paxMatrix->atpPaxType())
    {
      if (paxMatrix->carrier().empty() || paxMatrix->carrier() == ANY_CARRIER ||
          paxMatrix->carrier() == paxTypeFare.carrier())
        return true;
    }
  }

  return false;
}

bool
PaxTypeUtil::isAdultOrAssociatedType(const PaxTypeCode& paxTypeCode)
{
  if ((paxTypeCode.empty()) || (paxTypeCode == ADULT) || (paxTypeCode == CHILD) ||
      (paxTypeCode == INFANT) || (paxTypeCode == INS) || (paxTypeCode == UNN))
  {
    return true;
  }

  return false;
}

bool
PaxTypeUtil::isNegotiatedOrAssociatedType(const PaxTypeCode& paxTypeCode)
{
  if ((paxTypeCode == NEG) || (paxTypeCode == CNE) || (paxTypeCode == INE))
  {
    return true;
  }

  return false;
}

bool
PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType(const PaxTypeCode& paxType)
{
  return (isAdultOrAssociatedType(paxType) || isNegotiatedOrAssociatedType(paxType));
}

bool
PaxTypeUtil::getActualPaxInTrx(const PricingTrx& trx,
                               const CarrierCode& carrier,
                               const PaxTypeCode& paxTypeCode,
                               std::vector<PaxType*>& matchedPaxVec)
{
  for (PaxType* const reqPaxType : trx.paxType())
  {
    bool matched = false;

    const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypeMap =
        reqPaxType->actualPaxType();

    const auto& actualPaxTypeReq = actualPaxTypeMap.find(carrier);
    if (actualPaxTypeReq != actualPaxTypeMap.end())
    {
      for (const PaxType* const actualPT : *actualPaxTypeReq->second)
      {
        if (actualPT->paxType() == paxTypeCode)
        {
          matchedPaxVec.push_back(reqPaxType);
          matched = true;
          break;
        }
      }
    }

    if (matched)
      continue;

    // if no match for requested carrier look for ANY_CARRIER
    const auto& actualPaxTypeAny = actualPaxTypeMap.find(ANY_CARRIER);
    if (actualPaxTypeAny != actualPaxTypeMap.end())
    {
      for (const PaxType* const actualPT : *actualPaxTypeAny->second)
      {
        if (actualPT->paxType() == paxTypeCode)
        {
          matchedPaxVec.push_back(reqPaxType);
          break;
        }
      }
    }
  }

  return (!matchedPaxVec.empty());
}

bool
PaxTypeUtil::definedPaxAge(const PricingTrx& trx)
{
  return std::any_of(trx.paxType().cbegin(),
                     trx.paxType().cend(),
                     [](const PaxType* const pt){return pt->age() != 0;});
}

bool
PaxTypeUtil::hasNotAwardPaxType(const std::vector<PaxType*>& paxTypeVec)
{
  return std::any_of(paxTypeVec.cbegin(),
                     paxTypeVec.cend(),
                     [](const PaxType* const pt)
                     {
    const auto& ptCode = pt->paxType();
    return !ptCode.equalToConst("FFY") && !ptCode.equalToConst("FFP") && !ptCode.equalToConst("TNF") && !ptCode.equalToConst("TNN");
  });
}

bool
PaxTypeUtil::isPaxWithAge(const PaxTypeCode& paxType)
{
  boost::cmatch results;

  return boost::regex_match(paxType.c_str(), results, PaxTypeUtil::PAX_WITH_SPECIFIED_AGE) ||
         boost::regex_match(paxType.c_str(), results, PaxTypeUtil::PAX_WITH_UNSPECIFIED_AGE);
}

bool
PaxTypeUtil::isPaxWithSpecifiedAge(const PaxTypeCode& paxType)
{
  boost::cmatch results;
  return boost::regex_match(paxType.c_str(), results, PaxTypeUtil::PAX_WITH_SPECIFIED_AGE);
}

PaxTypeCode
PaxTypeUtil::getPaxWithUnspecifiedAge(char firstLetter)
{
  PaxTypeCode result;
  result[0] = firstLetter;
  result[1] = 'N';
  result[2] = 'N';

  return result;
}

bool
PaxTypeUtil::extractAgeFromPaxType(PaxTypeCode& ptc, uint16_t& age)
{
  if (!isPaxWithSpecifiedAge(ptc))
    return false;
  age = std::atoi(ptc.c_str() + 1);
  ptc = getPaxWithUnspecifiedAge(ptc[0]);
  return true;
}

bool
PaxTypeUtil::isSpanishPaxType(const PaxTypeCode& paxType)
{
  return (paxType == ADR || paxType == CHR || paxType == INR ||
          paxType == ISR || paxType == UNR);
}

bool PaxTypeUtil::isOnlyOnePassenger(const PricingTrx& trx)
{
  return (trx.paxType().size() == 1 && trx.paxType().front()->number() == 1);
}

uint16_t
PaxTypeUtil::getDifferenceInYears(const DateTime& from, const DateTime& to)
{
  if(!from.isValid() || !to.isValid())
  {
    return 0u;
  }

  if(from.year() >= to.year())
  {
    return 0u;
  }

  uint16_t years = to.year() - from.year();

  if(from.month() > to.month())
  {
    years -= 1;
  }
  else if((from.month() == to.month()) &&
          (from.day() > to.day()))
  {
    years -= 1;
  }

  return years;
}

void
PaxTypeUtil::parsePassengerWithAge(PaxType& paxType)
{
  if (containsAge(paxType.paxType()))
  {
    paxType.age() = static_cast<uint16_t>(std::atoi(paxType.paxType().begin() + 1));
    paxType.paxType() = removeAge(paxType.paxType());

    if (paxType.age() <= 1)
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID PASSENGER TYPE");
    }
  }
}

bool
PaxTypeUtil::containsAge(const PaxTypeCode& ptc)
{
  return ptc.length() == 3 && isalpha(ptc[0]) && isdigit(ptc[1]) && isdigit(ptc[2]);
}

PaxTypeCode
PaxTypeUtil::removeAge(const PaxTypeCode& ptc)
{
  return std::string(ptc, 0, 1) + "NN";
}

}
