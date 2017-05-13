//----------------------------------------------------------------------------
//  File:           FareTypeMatcher.cpp
//  Description:    Fare type pricing - fare matcher logic
//  Created:        03/07/2007
//  Authors:        Quan Ta
//
//  Updates:
//
// Copyright 2009, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system or publication,
// is strictly prohibited
//----------------------------------------------------------------------------
#include "Fares/FareTypeMatcher.h"

#include "Common/Assert.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareTypeQualifier.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag203Collector.h"
#include "FareCalc/FcUtil.h"

namespace tse
{
static Logger
logger("atseintl.Fares.FareTypeMatcher");

bool
FareTypeMatcher::
operator()(const FareType& fareType) const
{
  // Reguardless of the trip type, if there is only one matched FareType
  // item, then we can use it to match.
  if (_ftqList.size() == 1)
  {
    return matchFareTypeQualifier(_ftqList.front(), fareType);
  }

  if (_journeyType != FareTypeMatcher::Unknown)
  {
    for (const auto ftq : _ftqList)
    {
      if (matchFareTypeQualifier(ftq, fareType))
        return true;
    }
    return false;
  }
  else
  {
    // cannot pass or fail yet.  we need to check again after
    // combinability, when PUs are constructed.
    return true;
  }
}

bool
FareTypeMatcher::
operator()(const PaxTypeFare* ptf) const
{
  return (*this)(ptf->fcaFareType(), ptf);
}

bool
FareTypeMatcher::
operator()(const FareType& fareType, const PaxTypeFare* ptf) const
{
  LOG4CXX_DEBUG(logger, "Check Ft: " << fareType << ", Psg: " << ptf->fcasPaxType());

  // Reguardless of the trip type, if there is only one matched FareType
  // item, then we can use it to match.
  if (_ftqList.size() == 1)
  {
    return (matchFareTypeQualifier(_ftqList.front(), fareType, ptf));
  }

  if (_journeyType != FareTypeMatcher::Unknown)
  {
    for (const auto ftq : _ftqList)
    {
      if (matchFareTypeQualifier(ftq, fareType, ptf))
        return true;
    }
    return false;
  }
  else
  {
    if (ptf->fareMarket()->geoTravelType() == GeoTravelType::International)
    {
      MatchPuType mPuType(FareTypeMatcher::International);
      for (const auto ftq : _ftqList)
      {
        if (!mPuType(ftq))
          continue;

        if (matchFareTypeQualifier(ftq, fareType, ptf))
          return true;
      }
      return false;
    }
    // cannot pass or fail yet.  we need to check again after
    // combinability, when PUs are constructed.
    return true;
  }
}

bool
FareTypeMatcher::
operator()(const FarePath* fp) const
{
  if (_journeyType == FareTypeMatcher::Unknown)
  {
    // If you get here - it means there is a coding error.
    throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION);
  }

  // Reset the message setting
  _groupTrailerMsg = false;
  _itTrailerMsg = false;

  // FareType_Match_Diagnostic
  if (_trx.diagnostic().diagnosticType() == Diagnostic607)
  {
    DCFactory* factory = DCFactory::instance();
    _diag = static_cast<Diag203Collector*>(factory->create(_trx));
    _diag->enable(Diagnostic607);
    _diag->displayHeading(_trx);
  }
  msg() << "FAILED: FARE TYPE VALIDATION" << std::endl;

  if (!matchFareTypePermittedFP(fp))
    return false;

  if (hasFareTypeRequired())
  {
    if (_journeyType == FareTypeMatcher::EndOnEnd)
    {
      if (!matchFareTypeRequired(fp, FareTypeMatcher::Domestic) ||
          !matchFareTypeRequired(fp, FareTypeMatcher::International))
      {
        if (UNLIKELY(_diag))
        {
          *_diag << "FAILED FT VALIDATION\n";
        }
        return false;
      }
    }
    else
    {
      // the journey and pu trip type are the same in this case.
      if (!matchFareTypeRequired(fp, _journeyType))
      {
        if (UNLIKELY(_diag))
        {
          *_diag << "FAILED FT VALIDATION\n";
        }
        return false;
      }
    }
  }
  else
  {
    if (_journeyType == FareTypeMatcher::EndOnEnd)
    {
      if (!matchFareTypeRequired(fp, FareTypeMatcher::Domestic) ||
          !matchFareTypeRequired(fp, FareTypeMatcher::International))
      {
        if (UNLIKELY(_diag))
        {
          *_diag << "FAILED FT VALIDATION\n";
        }
        return false;
      }
    }
  }

  if (UNLIKELY(_diag))
  {
    *_diag << "PASSED FT VALIDATION\n";
  }

  // FareType_Match_Diagnostic
  msg().clear();

  if (UNLIKELY(_diag))
    _diag->flushMsg();

  return true;
}

/**
 * Return true if this PU has at least one fare component that match the
 *             fare type required field,
 *        false otherwise.
 **/
bool
FareTypeMatcher::
operator()(const PricingUnit* pu) const
{
  TripType puType = puTripType(*pu);

  std::set<PaxTypeCode> ftReqs;
  bool ftrStatus = !hasFareTypeRequired(ftReqs, puType);

  // FareType_Match_Diagnostic
  if (UNLIKELY(_diag))
  {
    DiagCollector& diag = *_diag;
    diag << *pu << "\n  \n";
  }

  const FareTypeQualifier* ftq = getFareTypeQualifier(puType);
  unsigned matchCount = 0;

  if (ftq != nullptr)
  {
    for (const auto fu : pu->fareUsage())
    {
      const PaxTypeFare* ptf = fu->paxTypeFare();
      if (matchFareTypeQualifier(ftq, ptf->fcaFareType(), ptf, ftrStatus))
      {
        matchCount++;
      }
      else
        break;
    }
  }

  // FareType_Match_Diagnostic
  if (UNLIKELY(_diag))
    *_diag << "\n  \n";

  if (_farePath && !ftrStatus)
  {
    msg() << "  FARE TYPE REQUIRED:";
    for (const auto& ftReq : ftReqs)
    {
      msg() << " " << ftReq;
      msg() << std::endl;
    }
  }
  return (ftrStatus && pu->fareUsage().size() == matchCount);
}

bool
FareTypeMatcher::matchFareTypeQualifier(const FareTypeQualifier* ftq, const FareType& fareType)
    const
{
  const std::map<PaxTypeCode, FareTypeQualMsg>& fareTypeMap = ftq->qualifierMgs();
  if (fareTypeMap.find(fareType.c_str()) != fareTypeMap.end())
  {
    LOG4CXX_INFO(logger, "Ft: " << fareType << " matched");
    return true;
  }
  return false;
}

bool
FareTypeMatcher::matchFareTypeQualifier(const FareTypeQualifier* ftq,
                                        const FareType& fareType,
                                        const PaxTypeFare* ptf) const
{
  const std::map<PaxTypeCode, FareTypeQualMsg>& fareTypeMap = ftq->qualifierMgs();
  std::map<PaxTypeCode, FareTypeQualMsg>::const_iterator iter = fareTypeMap.find(fareType.c_str());
  if (iter != fareTypeMap.end())
  {
    const std::set<PaxTypeCode>& psgTypeSet = ftq->psgType();

    bool psgTypeMatched = true;

    if (!psgTypeSet.empty() && !ptf->fcasPaxType().empty())
    {
      psgTypeMatched = psgTypeSet.find(ptf->fcasPaxType()) != psgTypeSet.end();
      if (!psgTypeMatched && ptf->isDiscounted() && ptf->discountInfo().category() == 19)
      {
        PaxTypeFare* baseFare = ptf->baseFare(19);
        if (baseFare)
        {
          psgTypeMatched = baseFare->fcasPaxType().empty() ||
                           psgTypeSet.find(baseFare->fcasPaxType()) != psgTypeSet.end();
        }
      }
    }

    if (psgTypeMatched)
    {
      LOG4CXX_INFO(logger, "Ft: " << fareType << ", Psg: " << ptf->fcasPaxType() << " matched");

      // FareType_Match_Diagnostic
      if (UNLIKELY(_diag))
      {
        *_diag << std::setw(10) << std::left << ptf->fareClass() << " FT /" << fareType
               << "/ MATCH.  "
               << "PSG TYPE /" << ptf->fcasPaxType() << "/ MATCH.\n";
      }

      if (iter->second.groupTrailerMsgInd() == 'Y')
        _groupTrailerMsg = true;
      if (iter->second.itTrailerMsgInd() == 'Y')
        _itTrailerMsg = true;
      return true;
    }
  }
  else
  {
    // FareType_Match_Diagnostic
    if (UNLIKELY(_diag))
    {
      *_diag << std::setw(10) << std::left << ptf->fareClass() << " FT /" << fareType
             << "/ NOT MATCH.\n";
    }
  }

  // FareType_Match_Diagnostic
  if (UNLIKELY(_diag))
  {
    *_diag << std::setw(10) << std::left << ptf->fareClass() << " PSG TYPE /" << ptf->fcasPaxType()
           << "/ NOT MATCH.\n";
  }
  if (_farePath)
  {
    msg() << "  " << ptf->fareClass() << "  FARE TYPE NOT MATCH: " << fareType << std::endl;
  }

  return false;
}

bool
FareTypeMatcher::matchFareTypeQualifier(const FareTypeQualifier* ftq,
                                        const FareType& fareType,
                                        const PaxTypeFare* ptf,
                                        bool& ftrMatched) const
{
  bool ret = true;
  bool matchedFtReq = false;

  // If there is only one qualifier, we already did the match check.
  // The same is true for domestic or international journey
  // Note the non ftr-matched already have this check performed before
  // invocation.
  if (_ftqList.size() > 1 || _journeyType == FareTypeMatcher::EndOnEnd ||
      _journeyType == FareTypeMatcher::Unknown)
  {
    ret = matchFareTypeQualifier(ftq, fareType, ptf);
  }

  if (ret)
  {
    const std::set<PaxTypeCode>& fareTypeRequired = ftq->fareTypeRequired();
    if (fareTypeRequired.find(fareType.c_str()) != fareTypeRequired.end())
    {
      matchedFtReq = true;

      LOG4CXX_INFO(logger, "Ft: " << fareType << " matched FtReq: " << fareType);

      // FareType_Match_Diagnostic
      if (UNLIKELY(_diag))
      {
        *_diag << std::setw(10) << std::left << ptf->fareClass() << " FT /" << fareType
               << "/ MATCH FT-REQ\n";
      }
      if (_farePath)
      {
        msg() << ptf->fareClass() << "  FT /" << fareType << "/MATCHED FT REQ" << std::endl;
      }
    }
  }

  // FareType_Match_Diagnostic
  if (UNLIKELY(_diag && !ftq->fareTypeRequired().empty() && !matchedFtReq))
  {
    *_diag << "FT /" << fareType << "/ FAIL FT-REQ:";
    for (const auto& ftr : ftq->fareTypeRequired())
    {
      *_diag << " " << ftr;
    }
    *_diag << '\n';
  }

  if (matchedFtReq)
    ftrMatched = true;

  return ret;
}

std::string
FareTypeMatcher::getFtPricingQualifier(const PricingOptions& options)
{
  std::string qualifier;

  switch (options.fareFamilyType())
  {
  case 'T':
    qualifier = "1"; // Inclusive tour - T/IT
    break;
  case 'N':
    qualifier = "2"; // Normal - T/NL
    break;
  case 'S':
    qualifier = "3"; // Special - T/EX
    break;
  default:
    break;
  }

  return qualifier;
}

void
FareTypeMatcher::getFareTypeQualifier()
{
  if (LIKELY(!_trx.getOptions()->isFareFamilyType()))
    return;

  Indicator userApplType;
  std::string userAppl;
  FareCalcUtil::getUserAppl(_trx, *_trx.getRequest()->ticketingAgent(), userApplType, userAppl);

  FareType qualifier(getFtPricingQualifier(*_trx.getOptions()).c_str());

  const std::vector<FareTypeQualifier*>& ftqList =
      _trx.dataHandle().getFareTypeQualifier(userApplType, userAppl, qualifier);

  if (!ftqList.empty())
  {
    if (_journeyType != FareTypeMatcher::Unknown)
    {
      MatchJourneyType match(_journeyType);
      for (const auto ftq : ftqList)
      {
        if (match(ftq))
        {
          _ftqList.push_back(ftq);
          _psgTypes.insert(ftq->psgType().begin(), ftq->psgType().end());
        }
      }
    }
    else
    {
      MatchJourneyType matchIntl(FareTypeMatcher::International);
      MatchJourneyType matchEoE(FareTypeMatcher::EndOnEnd);
      for (const auto ftq : ftqList)
      {
        if (matchIntl(ftq) || matchEoE(ftq))
        {
          _ftqList.push_back(ftq);
          _psgTypes.insert(ftq->psgType().begin(), ftq->psgType().end());
        }
      }
    }
  }
}

const FareTypeQualifier*
FareTypeMatcher::getFareTypeQualifier(TripType puType) const
{
  MatchJourneyType mJourneyType(_journeyType);
  MatchPuType mPuType(puType);

  for (const auto ftq : _ftqList)
  {
    if (!mJourneyType(ftq))
      continue;
    if (!mPuType(ftq))
      continue;

    return ftq;
  }

  return nullptr;
}

FareTypeMatcher::TripType
FareTypeMatcher::journeyType() const
{
  TSE_ASSERT(!_trx.itin().empty());
  TSE_ASSERT(_trx.itin().front() != nullptr);

  const Itin& itin = *_trx.itin().front();
  // If the itin domestic, or foreign domestic, we can process
  // as domestic journey type
  if (itin.geoTravelType() != GeoTravelType::International)
  {
    return FareTypeMatcher::Domestic;
  }

  // If the itin is internation, check if all fare markets is
  // internation. If so, we can process as international journey type,
  // otherwise, EndOnEnd journey type
  else
  {
    for (const auto fm : itin.fareMarket())
    {
      if (fm->geoTravelType() != GeoTravelType::International)
      {
        if (UNLIKELY(_farePath != nullptr))
        {
          for (const auto pu : _farePath->pricingUnit())
          {
            if (pu->geoTravelType() != GeoTravelType::International)
            {
              return FareTypeMatcher::EndOnEnd;
            }
          }
          return FareTypeMatcher::International;
        }
        else
        {
          // NOTE: here we return Unknown instead of EndOnEnd! Only
          // Combinability can make the final determination.
          return FareTypeMatcher::Unknown;
        }
      }
    }

    return FareTypeMatcher::International;
  }
}

FareTypeMatcher::TripType
FareTypeMatcher::puTripType(const PricingUnit& pu) const
{
  if (pu.geoTravelType() == GeoTravelType::International)
    return FareTypeMatcher::International;
  else
    return FareTypeMatcher::Domestic;
}

/**
 * Check if FareTypeRequired field processing is required
 **/
bool
FareTypeMatcher::hasFareTypeRequired() const
{
  if (_ftqList.empty())
    return false;

  for (const auto ftq : _ftqList)
  {
    if (ftq->fareTypeRequired().empty() == false)
      return true;
  }

  return false;
}

bool
FareTypeMatcher::hasFareTypeRequired(std::set<PaxTypeCode>& ftr, TripType puType) const
{
  if (_ftqList.empty())
    return false;

  bool ret = false;
  MatchPuType mPuType(puType);

  for (const auto ftq : _ftqList)
  {
    if (!mPuType(ftq))
      continue;

    if (ftq->fareTypeRequired().empty() == false)
    {
      ret = true;
      ftr.insert(ftq->fareTypeRequired().begin(), ftq->fareTypeRequired().end());
    }
  }

  return ret;
}

bool
FareTypeMatcher::matchFareTypePermittedFP(const FarePath* fp) const
{
  if (_journeyType == FareTypeMatcher::EndOnEnd)
  {
    if (!matchFareTypePermitted(fp, FareTypeMatcher::Domestic) ||
        !matchFareTypePermitted(fp, FareTypeMatcher::International))
    {
      if (UNLIKELY(_diag))
      {
        *_diag << "FAILED PERMITTED VALIDATION\n";
        *_diag << "---------------------------------------------------------------\n";
      }
      return false;
    }
  }
  else
  {
    // the journey and pu trip type are the same in this case.
    if (!matchFareTypePermitted(fp, _journeyType))
    {
      if (UNLIKELY(_diag))
      {
        *_diag << "FAILED PERMITTED VALIDATION\n";
        *_diag << "---------------------------------------------------------------\n";
      }
      return false;
    }
  }
  if (UNLIKELY(_diag))
  {
    *_diag << "PASS PERMITTED VALIDATION\n";
    *_diag << "---------------------------------------------------------------\n";
  }
  return true;
}

bool
FareTypeMatcher::matchFareTypePermitted(const FarePath* fp, TripType puType) const
{
  for (const auto pu : fp->pricingUnit())
  {
    if (puTripType(*pu) == puType && !matchFareTypePermittedPU(pu, puType))
      return false;
  }
  return true;
}

bool
FareTypeMatcher::matchFareTypePermittedPU(const PricingUnit* pu, TripType puType) const
{
  TripType pricingUnitType = puTripType(*pu);

  std::set<PaxTypeCode> ftReqs;

  // FareType_Match_Diagnostic
  if (UNLIKELY(_diag))
  {
    *_diag << "JRNY TYPE: " << (char)_journeyType << "  PU TYPE: " << (char)pricingUnitType << '\n';
    DiagCollector& diag = *_diag;
    diag << *pu << "\n  \n";
  }

  const FareTypeQualifier* ftq = getFareTypeQualifier(puType);
  unsigned matchCount = 0;

  if (ftq != nullptr)
  {
    if (UNLIKELY(_diag))
    {
      *_diag << *ftq << "\n  \n";
    }

    for (const auto fu : pu->fareUsage())
    {
      const PaxTypeFare* ptf = fu->paxTypeFare();
      if (matchFareTypeQualifier(ftq, ptf->fcaFareType(), ptf))
      {
        matchCount++;
      }
      else
        break;
    }
  }

  // FareType_Match_Diagnostic
  if (UNLIKELY(_diag))
    *_diag << "\n  \n";

  return (pu->fareUsage().size() == matchCount);
}

bool
FareTypeMatcher::matchFareTypeRequired(const FarePath* fp, FareTypeMatcher::TripType puType) const
{
  // FareType_Match_Diagnostic
  msg() << " PUTYPE: " << (char)puType << std::endl;

  for (std::vector<PricingUnit*>::const_iterator puI = fp->pricingUnit().begin(),
                                                 puEnd = fp->pricingUnit().end();
       puI != puEnd;
       ++puI)
  {
    if (puTripType(**puI) != puType)
      continue;

    if ((*this)(*puI))
    {
      // FareType_Match_Diagnostic
      msg() << " MATCHES" << std::endl;
      if (UNLIKELY(_diag && std::distance(puI, puEnd) > 1))
        *_diag << "---------------------------------------------------------------\n";
      return true;
    }
  }

  // FareType_Match_Diagnostic
  msg() << " DO NOT MATCH" << std::endl;
  if (UNLIKELY(_diag))
    *_diag << "\n  \n";

  return false;
}

bool
MatchJourneyType::
operator()(const FareTypeQualifier* ftq) const
{
  if (match)
  {
    return ((tripType == FareTypeMatcher::Domestic && ftq->journeyTypeDom() == 'Y') ||
            (tripType == FareTypeMatcher::International && ftq->journeyTypeIntl() == 'Y') ||
            (tripType == FareTypeMatcher::EndOnEnd && ftq->journeyTypeEoe() == 'Y'));
  }
  else
  {
    return ((tripType == FareTypeMatcher::Domestic && ftq->journeyTypeDom() == 'N') ||
            (tripType == FareTypeMatcher::International && ftq->journeyTypeIntl() == 'N') ||
            (tripType == FareTypeMatcher::EndOnEnd && ftq->journeyTypeEoe() == 'N'));
  }
}

bool
MatchPuType::
operator()(const FareTypeQualifier* ftq) const
{
  if (match)
  {
    return ((tripType == FareTypeMatcher::Domestic && ftq->pricingUnitDom() == 'Y') ||
            (tripType == FareTypeMatcher::International && ftq->pricingUnitIntl() == 'Y'));
  }
  else
  {
    return ((tripType == FareTypeMatcher::Domestic && ftq->pricingUnitDom() == 'N') ||
            (tripType == FareTypeMatcher::International && ftq->pricingUnitIntl() == 'N'));
  }
}
}
