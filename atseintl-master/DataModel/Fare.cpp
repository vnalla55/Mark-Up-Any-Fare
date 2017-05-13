//-------------------------------------------------------------------
//
//  File:        Fare.cpp
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: A generic class to represent common part of fare.
//               It includes FareInfo, TariffCrossRef and status
//               fields and accessors.
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

/*

                         Implementation notes
                         ============== =====

Fare directionality.
==== ===============

1) Fare directionality depends on FareMarket.
2) Market1 & Market2 are always from fare.
3) FS_ReversedDirection bit in fare status defines whether Market1
   is start point of FareMarket or isn't.
4) Directionality() returns value based on FS_ReversedDirection bit
   value
4) origin() and destination() return cities in relation to
   instant (not flipped) directionality

example:
--------------------------------------------------------------------
 FareInfo(instant)!   FareMarket DFW-PAR  !   FareMarket DFW-PAR
Mkt1 Mkt2     Dir ! Reversed Dir Orig Dest! Reversed Dir Orig Dest
------------------+-----------------------+-------------------------
DFW  PAR  F(DFW->)!     N     F  DFW PAR  !    Y      T  DFW  PAR
DFW  PAR  T(DFW<-)!     N     T  PAR DFW  !    Y      F  PAR  DFW
--------------------------------------------------------------------

*/
#include "DataModel/Fare.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/SITAConstructedFareInfo.h"
#include "Rules/RuleConst.h"


namespace tse
{

// static data
// ====== ====

const Fare Fare::_emptyFare;

namespace
{
Logger logger("atseintl.DataModel.Fare");
}

const Fare::RuleState Fare::_ruleStateMap[] = { RS_CatNotSupported, // cat  0
                                                RS_CatNotSupported, // cat  1
                                                RS_Cat02, // cat  2
                                                RS_Cat03, // cat  3
                                                RS_Cat04, // cat  4
                                                RS_Cat05, // cat  5
                                                RS_Cat06, // cat  6
                                                RS_Cat07, // cat  7
                                                RS_Cat08, // cat  8
                                                RS_Cat09, // cat  9
                                                RS_Cat10, // cat 10
                                                RS_Cat11, // cat 11
                                                RS_Cat12, // cat 12
                                                RS_CatNotSupported, // cat 13
                                                RS_Cat14, // cat 14
                                                RS_Cat15, // cat 15
                                                RS_Cat16, // cat 16
                                                RS_CatNotSupported, // cat 17
                                                RS_CatNotSupported, // cat 18
                                                RS_CatNotSupported, // cat 19
                                                RS_CatNotSupported, // cat 20
                                                RS_CatNotSupported, // cat 21
                                                RS_CatNotSupported, // cat 22
                                                RS_Cat23, // cat 23
                                                RS_CatNotSupported, // cat 24
                                                RS_CatNotSupported, // cat 25
                                                RS_CatNotSupported, // cat 26
                                                RS_Cat27, // cat 27
                                                RS_CatNotSupported, // cat 28
                                                RS_CatNotSupported, // cat 29
                                                RS_CatNotSupported, // cat 30
                                                RS_CatNotSupported, // cat 31
                                                RS_CatNotSupported, // cat 32
                                                RS_CatNotSupported, // cat 33
                                                RS_CatNotSupported, // cat 34
                                                RS_CatNotSupported // cat 35
};

bool
Fare::isValid() const
{
  return _status.isSet(FS_ScopeIsDefined) && !isFareInfoMissing() && !isTariffCrossRefMissing() &&
         isCat15SecurityValid() && isGlobalDirectionValid() && isCalcCurrForDomItinValid() &&
         isValidBySecondaryActionCode() && isAnyBrandedFareValid() && !isDirectionalityFail();
}

bool
Fare::isValidForFareDisplay() const
{
  return _status.isSet(FS_ScopeIsDefined) && !isFareInfoMissing() && !isTariffCrossRefMissing() &&
         isCat15SecurityValid() && isGlobalDirectionValid() && isCalcCurrForDomItinValid() &&
         isValidBySecondaryActionCode() && !isFareNotValidForDisplay();
}

bool
Fare::isValidForRouting() const
{
  return _status.isSet(FS_ScopeIsDefined) && !isFareInfoMissing() && !isTariffCrossRefMissing() &&
         isCat15SecurityValid() && isGlobalDirectionValid() && isCalcCurrForDomItinValid() &&
         isAnyBrandedFareValid() && !isDirectionalityFail();
}

void
Fare::setGeoTravelType(const GeoTravelType geoTravelType)
{
  switch (geoTravelType)
  {
  case GeoTravelType::Domestic:
    _status.set(FS_Domestic);
    break;

  case GeoTravelType::International:
    _status.set(FS_International);
    break;

  case GeoTravelType::Transborder:
    _status.set(FS_Transborder);
    break;

  case GeoTravelType::ForeignDomestic:
    _status.set(FS_ForeignDomestic);
    break;

  default: // satisfied compiler -> happy programmer!
    ;
  }
}

void
Fare::checkGlobalDirection(const FareMarket& fareMarket)
{
  const GlobalDirection fmgd = fareMarket.getGlobalDirection();
  GlobalDirection gd = globalDirection();

  // if global direction for the fare is not 'any' or 'universal' and
  // it's not equal to the fare market's global direction, we set it
  // as invalid. The fare market's global direction is only 'any' or
  // 'universal' in the case of shopping, in which case the global
  // direction of the fare is always valid.

  if (gd != GlobalDirection::XX && gd != GlobalDirection::ZZ && fmgd != GlobalDirection::XX &&
      fmgd != GlobalDirection::ZZ && gd != fmgd)
    setGlobalDirectionValid(false);
}

bool
Fare::initialize(const FareState initialStatus,
                 const FareInfo* fareInfo,
                 const FareMarket& fareMarket,
                 const TariffCrossRefInfo* tariffCrossRefInfo,
                 const ConstructedFareInfo* constructedFareInfo)
{
  _status.initialize(initialStatus);

  setGeoTravelType(fareMarket.geoTravelType());

  _fareInfo = fareInfo;
  _tariffCrossRefInfo = tariffCrossRefInfo;
  _constructedFareInfo = constructedFareInfo;

  checkGlobalDirection(fareMarket);

  return isValid();
}

Fare*
Fare::clone(DataHandle& dataHandle, bool resetStatus) const
{
  Fare* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(dataHandle, *cloneObj, resetStatus);

  return cloneObj;
}

void
Fare::clone(DataHandle& dataHandle, Fare& cloneObj, bool resetStatus) const
{
  cloneObj._status = _status;
  cloneObj._ruleStatus = _ruleStatus;
  cloneObj._ruleProcessStatus = _ruleProcessStatus;
  cloneObj._ruleSoftPassStatus = _ruleSoftPassStatus;
  cloneObj._footNoteStatus = _footNoteStatus;
  cloneObj._dontCheckFareRuleAltGenRule = _dontCheckFareRuleAltGenRule;
  cloneObj._footnoteRec2Status = _footnoteRec2Status;
  cloneObj._fareInfo = _fareInfo;
  cloneObj._tariffCrossRefInfo = _tariffCrossRefInfo;
  cloneObj._constructedFareInfo = _constructedFareInfo;
  cloneObj._nucFareAmount = _nucFareAmount;
  cloneObj._nucMarkupAmount = _nucMarkupAmount;
  cloneObj._rexSecondNucFareAmount = _rexSecondNucFareAmount;
  cloneObj._rexSecondNucOriginalFareAmount = _rexSecondNucOriginalFareAmount;
  cloneObj._nucOriginalFareAmount = _nucOriginalFareAmount;
  cloneObj._outboundFareForCarnivalInbound = _outboundFareForCarnivalInbound;
  cloneObj._mileageSurchargePctg = _mileageSurchargePctg;
  cloneObj._mileageSurchargeAmt = _mileageSurchargeAmt;
  cloneObj._rexSecondMileageSurchargeAmt = _rexSecondMileageSurchargeAmt;
  cloneObj._latestTktDT = _latestTktDT;
  cloneObj._latestTktDTFootN = _latestTktDTFootN;
  cloneObj._latestTktDTFareR = _latestTktDTFareR;
  cloneObj._latestTktDTGenR = _latestTktDTGenR;
  cloneObj._nFrC15Status = _nFrC15Status;
  cloneObj._wEtktC15Status = _wEtktC15Status;
  cloneObj._secC15Status = _secC15Status;
  cloneObj._domesticFootNote = _domesticFootNote;
  cloneObj._reProcessCat05NoMatch = _reProcessCat05NoMatch;
  cloneObj._warningMap = _warningMap;
  cloneObj._brandedFareStatus = _brandedFareStatus;
  cloneObj._forbiddenFop = _forbiddenFop;

  if (resetStatus)
    cloneObj.resetRuleStatus();
}

void
Fare::resetFnMissingStat(const uint16_t& numFootnotes)
{
  if (_footNoteStatus.isSet(InitializedFnStat))
    return; // already done reset once

  uint8_t missingFootNoteBits = 0;

  if (UNLIKELY(numFootnotes > MAX_NUMOF_FOOTNOTE))
    missingFootNoteBits = (1 << MAX_NUMOF_FOOTNOTE);
  else
    missingFootNoteBits = static_cast<uint8_t>(1 << numFootnotes);

  missingFootNoteBits--;

  _footNoteStatus.initialize((FootNoteState)missingFootNoteBits);
  _footNoteStatus.set(InitializedFnStat);
}

void
Fare::setFoundFootnote(uint16_t fnIndex)
{
  // fnIndex should range from 0 to MAX_NUMOF_FOOTNOTE-1
  if (UNLIKELY(fnIndex >= MAX_NUMOF_FOOTNOTE))
    return;

  uint8_t missingFootNoteBit = static_cast<uint8_t>(1 << fnIndex);

  _footNoteStatus.set((FootNoteState)missingFootNoteBit, false);
}

bool
Fare::missedFootnote(uint16_t& missingFnIndex) const
{
  if (!_footNoteStatus.isSet(MissingAnyFootNote))
    return false;

  if (_footNoteStatus.isSet(MissFootNote1))
    missingFnIndex = 0;
  else if (_footNoteStatus.isSet(MissFootNote2))
    missingFnIndex = 1;
  else if (_footNoteStatus.isSet(MissFootNote3))
    missingFnIndex = 2;
  else if (_footNoteStatus.isSet(MissFootNote4))
    missingFnIndex = 3;
  else if (_footNoteStatus.isSet(MissFootNote5))
    missingFnIndex = 4;
  else
    missingFnIndex = 5;

  return true;
}

WarningMap&
Fare::warningMap()
{
  return _warningMap;
}

const WarningMap&
Fare::warningMap() const
{
  return _warningMap;
}

WarningMap::WarningMap()
{
  _warningMap.initialize(0);
  _cat5Warnings.initialize(0);
}

bool
WarningMap::isSet(const WarningID warning) const
{
  uint16_t mask = 0x00000001;
  mask = static_cast<uint16_t>(mask << warning);

  return _warningMap.isSet(mask);
}

void
WarningMap::set(const WarningID warning, bool value) const
{
  uint16_t mask = 0x00000001;
  mask = static_cast<uint16_t>(mask << warning);

  _warningMap.set(mask, value);
}

bool
WarningMap::isCat5WqWarning(const int seg) const
{
  uint16_t mask = 0x00000001;
  mask = static_cast<uint16_t>(mask << seg);

  return _cat5Warnings.isSet(mask);
}

void
WarningMap::setCat5WqWarning(const int seg, bool value) const
{
  uint16_t mask = 0x00000001;
  mask = static_cast<uint16_t>(mask << seg);

  _cat5Warnings.set(mask, value);
}

WarningMap&
WarningMap::
operator=(const WarningMap& copy)
{
  if (LIKELY(this != &copy))
  {
    _warningMap = copy._warningMap;
    _cat5Warnings = copy._cat5Warnings;
  }

  return *this;
}

const Fare::BrandedFareStatus&
Fare::getBrandedFareStatus(const uint16_t index) const
{
  std::map<uint16_t, BrandedFareStatus>::const_iterator bfsi = _brandedFareStatus.find(index);
  if (UNLIKELY(bfsi != _brandedFareStatus.end()))
    return bfsi->second;

  static const BrandedFareStatus emptyBrandedFareStatus;
  return emptyBrandedFareStatus;
}

bool
Fare::isAnyBrandedFareValid() const
{
  if (LIKELY(_brandedFareStatus.empty()))
    return true;

  std::map<uint16_t, BrandedFareStatus>::const_iterator bfsi = _brandedFareStatus.begin();
  for (; bfsi != _brandedFareStatus.end(); ++bfsi)
  {
    if (!(bfsi->second.isSet(FS_InvBrand)) && !(bfsi->second.isSet(FS_InvBrandCorpID)))
      return true;
  }

  return false;
}

void
Fare::getBrandIndexes(std::set<uint16_t> &result) const
{
  std::map<uint16_t, BrandedFareStatus>::const_iterator bfsi = _brandedFareStatus.begin();
  for (; bfsi != _brandedFareStatus.end(); ++bfsi)
  {
    if (!(bfsi->second.isSet(FS_InvBrand)) && !(bfsi->second.isSet(FS_InvBrandCorpID)))
      result.insert(bfsi->first);
  }
}

void
Fare::resetRuleStatus()
{
  _ruleProcessStatus.setNull();
  _ruleStatus.setNull();
  _ruleSoftPassStatus.setNull();
  _dontCheckFareRuleAltGenRule.setNull();
  _footnoteRec2Status.setNull();
  setFootnotesPrevalidated(false);
}

void
Fare::addInvalidValidatingCxr(const CarrierCode& cxr)
{
  _invalidValidatingCarriers.push_back(cxr);
}

void
Fare::addInvalidValidatingCxr(const std::vector<CarrierCode>& cxrs)
{
  _invalidValidatingCarriers.insert(_invalidValidatingCarriers.end(), cxrs.begin(), cxrs.end());
}

} // tse
