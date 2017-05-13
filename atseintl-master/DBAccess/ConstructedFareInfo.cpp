//-------------------------------------------------------------------
//
//  File:        ConstructedFareInfo.cpp
//  Created:     Feb 14, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents common data and members of
//               one ATPCO or SMF constructed fare
//
//  Copyright Sabre 2005
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

#include "DBAccess/ConstructedFareInfo.h"

#include "Common/ObjectComparison.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"

namespace tse
{

ConstructedFareInfo::ConstructedFareInfo()
  : _constructionType(DOUBLE_ENDED),
    _specifiedFareAmount(0),
    _constructedNucAmount(0),
    _constructedSecondNucAmount(0),
    _fareInfo(nullptr),
    _atpFareClassPriority(0),
    _origAddonZone(0),
    _origAddonTariff(0),
    _origAddonAmount(0),
    _destAddonZone(0),
    _destAddonTariff(0),
    _destAddonAmount(0),
    _origAddonOWRT(0),
    _destAddonOWRT(0)
{
  initialize();
}

ConstructedFareInfo::ConstructedFareInfo(bool init)
  : _constructionType(DOUBLE_ENDED),
    _specifiedFareAmount(0),
    _constructedNucAmount(0),
    _constructedSecondNucAmount(0),
    _fareInfo(nullptr),
    _atpFareClassPriority(0),
    _origAddonZone(0),
    _origAddonTariff(0),
    _origAddonAmount(0),
    _destAddonZone(0),
    _destAddonTariff(0),
    _destAddonAmount(0),
    _origAddonOWRT(0),
    _destAddonOWRT(0)
{
  if (UNLIKELY(init))
    initialize();
}

ConstructedFareInfo::~ConstructedFareInfo()
{
  if (LIKELY(_fareInfo))
  {
    delete _fareInfo;
    _fareInfo = nullptr;
  }
}

void
ConstructedFareInfo::initialize()
{
  _fareInfo = new FareInfo;
}

void
ConstructedFareInfo::clone(ConstructedFareInfo& cloneObj) const
{
  cloneObj._constructionType = _constructionType;

  cloneObj._gateway1 = _gateway1;
  cloneObj._gateway2 = _gateway2;

  cloneObj._specifiedFareAmount = _specifiedFareAmount;
  cloneObj._constructedNucAmount = _constructedNucAmount;
  cloneObj._constructedSecondNucAmount = _constructedSecondNucAmount;

  cloneObj._atpFareClassPriority = _atpFareClassPriority;

  cloneObj._origAddonZone = _origAddonZone;
  cloneObj._origAddonFootNote1 = _origAddonFootNote1;
  cloneObj._origAddonFootNote2 = _origAddonFootNote2;
  cloneObj._origAddonFareClass = _origAddonFareClass;
  cloneObj._origAddonTariff = _origAddonTariff;
  cloneObj._origAddonRouting = _origAddonRouting;

  cloneObj._origAddonAmount = _origAddonAmount;
  cloneObj._origAddonCurrency = _origAddonCurrency;

  cloneObj._destAddonZone = _destAddonZone;
  cloneObj._destAddonFootNote1 = _destAddonFootNote1;
  cloneObj._destAddonFootNote2 = _destAddonFootNote2;
  cloneObj._destAddonFareClass = _destAddonFareClass;
  cloneObj._destAddonTariff = _destAddonTariff;
  cloneObj._destAddonRouting = _destAddonRouting;

  cloneObj._destAddonAmount = _destAddonAmount;
  cloneObj._destAddonCurrency = _destAddonCurrency;

  cloneObj._origAddonOWRT = _origAddonOWRT;
  cloneObj._destAddonOWRT = _destAddonOWRT;

  if (LIKELY(_fareInfo && cloneObj._fareInfo))
    _fareInfo->clone(*(cloneObj._fareInfo));
}

ConstructedFareInfo*
ConstructedFareInfo::clone(DataHandle& dataHandle) const
{
  ConstructedFareInfo* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(*cloneObj); // lint !e413

  return cloneObj;
}

void
ConstructedFareInfo::dumpConstructedFareInfo(std::ostream& os) const
{
  os << "[" << _constructionType << "|" << _gateway1 << "|" << _gateway2 << "|"
     << _specifiedFareAmount << "|" << _constructedNucAmount << "|" << _constructedSecondNucAmount
     << "|" << _atpFareClassPriority << "|" << _origAddonZone << "|" << _origAddonFootNote1 << "|"
     << _origAddonFootNote2 << "|" << _origAddonFareClass << "|" << _origAddonTariff << "|"
     << _origAddonRouting << "|" << _origAddonAmount << "|" << _origAddonCurrency << "|"
     << _destAddonZone << "|" << _destAddonFootNote1 << "|" << _destAddonFootNote2 << "|"
     << _destAddonFareClass << "|" << _destAddonTariff << "|" << _destAddonRouting << "|"
     << _destAddonAmount << "|" << _destAddonCurrency << "|" << _origAddonOWRT << "|"
     << _destAddonOWRT;
  if (_fareInfo != nullptr)
    tse::dumpObject(os, *_fareInfo);
}

void
ConstructedFareInfo::dumpObject(std::ostream& os) const
{
  dumpConstructedFareInfo(os);
  os << "]";
}

void
ConstructedFareInfo::dummyData()
{
  _constructionType = SINGLE_ORIGIN;
  _gateway1 = "ABCDE";
  _gateway2 = "FTHIJ";
  _specifiedFareAmount = 1.11;
  _constructedNucAmount = 2.22;
  _constructedSecondNucAmount = 3.33;
  FareInfo::dummyData(*_fareInfo);
  _atpFareClassPriority = 3;
  _origAddonZone = 4;
  _origAddonFootNote1 = "KL";
  _origAddonFootNote2 = "MN";
  _origAddonFareClass = "OPQRSTUV";
  _origAddonTariff = 5;
  _origAddonRouting = "WXYZ";
  _origAddonAmount = 6.66;
  _origAddonCurrency = "abc";
  _destAddonZone = 7;
  _destAddonFootNote1 = "de";
  _destAddonFootNote2 = "fg";
  _destAddonFareClass = "hijklmno";
  _destAddonTariff = 8;
  _destAddonRouting = "pqrs";
  _destAddonAmount = 9.99;
  _destAddonCurrency = "tuv";
  _origAddonOWRT = 'w';
  _destAddonOWRT = 'x';
}

} // tse
