//-------------------------------------------------------------------
//
//  File:        CombFareClassMap.cpp
//  Created:     Jun 17, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class to map together add-on and fare FareClass
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

#include "AddonConstruction/CombFareClassMap.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DiagRequest.h"
#include "Common/Logger.h"
#include "Common/FallbackUtil.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/AddonFareInfo.h"
#include "Diagnostic/Diag253Collector.h"
#include "Util/BranchPrediction.h"

#include <boost/range/algorithm.hpp>

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

static Logger
logger("atseintl.AddonConstruction.CombFareClassMap");

CombFareClassMap::CombFareClassMap()
  : _cJob(nullptr),
    _date(DateTime::localTime()),
    _fareTariff(-1),
    _diagHeaderPrinted(false),
    _fareClassCombVector(nullptr),
    _fareClassCombMultiMap(nullptr)
{
}

CombFareClassMap::~CombFareClassMap() {}

void
CombFareClassMap::init(ConstructionJob* cJob)
{
  _cJob = cJob;
  _tariffCombFareClassMap.clear();
  _fareClassCombMultiMap = nullptr;
  _fareClassCombVector = nullptr;
}

void
CombFareClassMap::setTariff(TariffNumber fareTariff)
{
  if (_fareTariff != fareTariff)
  {
    _fareTariff = fareTariff;
    _fareClassCombMultiMap = nullptr;
    _fareClassCombVector = nullptr;
    if (LIKELY(!_cJob->dataHandle().isHistorical()))
    {
      TariffAddonFareClassCombMultiMap::const_iterator it(_tariffCombFareClassMap.find(fareTariff));
      if (it != _tariffCombFareClassMap.end())
      {
        _fareClassCombMultiMap = it->second;
      }
    }
    else // new code for historical
    {
      TariffCombFareClassVecMap::const_iterator it(_tariffCombFareClassVecMap.find(fareTariff));
      if (it != _tariffCombFareClassVecMap.end())
      {
        _fareClassCombVector = it->second;
      }
    }
  }
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

FALLBACK_DECL(fallbackConstructedFareEffectiveDate)

void
CombFareClassMap::writeDiagnostics()
{
  Diag253Collector* diag253{ nullptr };
  if (_cJob)
  {
    if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
    {
      diag253 = _cJob->diagnostic<Diag253Collector>();
    }
    else
    {
      diag253 = _cJob->diag253();
    }
  }

  if (UNLIKELY(diag253 != nullptr))
  {
    if (!_diagHeaderPrinted)
    {
      diag253->writeCombFareClassHeader(_cJob->vendorCode());
      _diagHeaderPrinted = true;
    }
    if (const DiagRequest* diagReq = _cJob->diagRequest())
    {
      if (!diagReq->isValidForDiag(_cJob->vendorCode(), _fareTariff))
        return;
    }
    if (_fareClassCombMultiMap)
    {
      AddonFareClassCombMultiMap::const_iterator itm(_fareClassCombMultiMap->begin()),
          itmEnd(_fareClassCombMultiMap->end());
      for (; itm != itmEnd; ++itm)
      {
        for (const AddonCombFareClassInfo* afc: itm->second)
          diag253->writeCombFareClass(*afc, _fareTariff);
      }
    }
    if (_fareClassCombVector)
    {
      AddonCombFareClassInfoVec::const_iterator itv(_fareClassCombVector->begin()),
          itvEnd(_fareClassCombVector->end());
      for (; itv != itvEnd; ++itv)
      {
        diag253->writeCombFareClass(**itv, _fareTariff);
      }
    }
  }
}

bool
CombFareClassMap::populate()
{
  if (LIKELY(!_fareClassCombMultiMap))
  {

    // get all AddOnCombFareClass records for specified vendor/tariff/carrier

    // AddonCombFareClass objects are to be shared across addon construction threads -
    // that's why transaction-scoped dataHandle has to be used here.
    const AddonFareClassCombMultiMap& acfMap = _cJob->trxDataHandle().getAddOnCombFareClass(
        _cJob->vendorCode(), _fareTariff, _cJob->carrier(), _cJob->ticketingDate());

    _fareClassCombMultiMap = &acfMap;
    _tariffCombFareClassMap.insert(
        TariffAddonFareClassCombMultiMap::value_type(_fareTariff, _fareClassCombMultiMap));

    writeDiagnostics();
  }
  return true;
}

FareMatchCode
CombFareClassMap::matchFareClassesHistorical(const FareInfo& specFare,
                                             const AddonFareCortege& addonFC,
                                             const Indicator geoAppl,
                                             const Indicator owrt,
                                             TSEDateInterval& validDI)
{
  FareMatchCode matchResult = FM_COMB_FARE_CLASS;

  const FareClassCode& sfClass(specFare.fareClass());

  if (nullptr == _fareClassCombVector)
  {
    _fareClassCombVector = &_cJob->dataHandle().getAddOnCombFareClassHistorical(
        _cJob->vendorCode(), _fareTariff, _cJob->carrier());
    _tariffCombFareClassVecMap.insert(std::make_pair(_fareTariff, _fareClassCombVector));
  }

  typedef std::pair<AddonCombFareClassInfoVec::const_iterator,
                    AddonCombFareClassInfoVec::const_iterator> SelectionRange;

  SortACFCKeyLess pred;
  AddonCombFareClassInfo ref;
  ref.addonFareClass() = addonFC.addonFare()->fareClass()[0];
  ref.geoAppl() = geoAppl;
  ref.owrt() = owrt;
  ref.fareClass() = sfClass.c_str();
  SelectionRange range(boost::equal_range(*_fareClassCombVector, &ref, pred));

  AddonCombFareClassInfo const* latest(nullptr);
  for (auto i = range.first; i != range.second; ++i)
  {
    if (latest == nullptr || (*i)->expireDate() > latest->expireDate())
      latest = *i;
  }
  if (latest != nullptr)
  {
    if (latest->createDate() > validDI.createDate())
      validDI.createDate() = latest->createDate();
    if (LIKELY(!_cJob->fallbackConstructedFareEffectiveDate()))
    {
      if (latest->effDate() > validDI.effDate())
        validDI.effDate() = latest->effDate();
      if (latest->expireDate() < validDI.expireDate())
        validDI.expireDate() = latest->expireDate();
    }
    matchResult = FM_GOOD_MATCH;
  }

  return matchResult;
}


const AddonFareClasses*
CombFareClassMap::matchSpecifiedFare(const FareInfo& specFare)
{
  if (!_fareClassCombMultiMap)
    populate();

  Indicator owrt(specFare.owrt());

  if (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    owrt = ONE_WAY_MAY_BE_DOUBLED;

  AddonFareClassCombMultiMap::const_iterator i =
    _fareClassCombMultiMap->find(AddonCombFareClassSpecifiedKey(specFare.fareClass().c_str(), owrt));
  if (i != _fareClassCombMultiMap->end())
    return &i->second;

  return nullptr;
}

#else

void
CombFareClassMap::writeDiagnostics()
{
  Diag253Collector* diag253{ nullptr };
  if (_cJob)
  {
    if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
    {
      diag253 = _cJob->diagnostic<Diag253Collector>();
    }
    else
    {
      diag253 = _cJob->diag253();
    }
  }

  if (UNLIKELY(diag253 != nullptr))
  {
    if (!_diagHeaderPrinted)
    {
      diag253->writeCombFareClassHeader(_cJob->vendorCode());
      _diagHeaderPrinted = true;
    }
    if (_fareClassCombMultiMap)
    {
      AddonFareClassCombMultiMap::const_iterator itm(_fareClassCombMultiMap->begin()),
          itmEnd(_fareClassCombMultiMap->end());
      for (; itm != itmEnd; ++itm)
      {
        diag253->writeCombFareClass(*itm->second);
      }
    }
    if (_fareClassCombVector)
    {
      AddonCombFareClassInfoVec::const_iterator itv(_fareClassCombVector->begin()),
          itvEnd(_fareClassCombVector->end());
      for (; itv != itvEnd; ++itv)
      {
        diag253->writeCombFareClass(**itv);
      }
    }
  }
}

bool
CombFareClassMap::populate()
{
  if (LIKELY(!_fareClassCombMultiMap))
  {

    // get all AddOnCombFareClass records for specified vendor/tariff/carrier

    const AddonFareClassCombMultiMap& acfMap = _cJob->dataHandle().getAddOnCombFareClass(
        _cJob->vendorCode(), _fareTariff, _cJob->carrier());

    _fareClassCombMultiMap = &acfMap;
    _tariffCombFareClassMap.insert(
        TariffAddonFareClassCombMultiMap::value_type(_fareTariff, _fareClassCombMultiMap));
    writeDiagnostics();
  }
  return true;
}

FareMatchCode
CombFareClassMap::matchFareClasses(ConstructedFare& specFare,
                                   const AddonFareCortege& addonFC,
                                   const Indicator geoAppl,
                                   const Indicator owrt,
                                   AddonCombFareClassInfoVec& fClassCombRecords)
{
  FareMatchCode matchResult = FM_COMB_FARE_CLASS;

  const FareClassCode& sfClass(specFare.specifiedFare()->fareClass());

  switch (addonFC.addonFareClass())
  {
  case AddonFareCortege::REGULAR:

    // exact match only

    if (sfClass == addonFC.addonFare()->fareClass())
      matchResult = FM_COMB_FARE_EXACT_MATCH;

    break;

  case AddonFareCortege::ALPHA_FIVE_STAR:
  case AddonFareCortege::SIX_STAR:
  {
    // Alpha Five Asterisk (A*****) or Six Asterisk (******)
    // generic add-on.
    //
    // For these types of add-ons we use POSITIVE logic, i.e.
    // if fareClass is found then the answer is YES
    if (LIKELY(!_cJob->dataHandle().isHistorical()))
    {
      if (!_fareClassCombMultiMap)
        populate();

      AddonCombFareClassInfoKey fareClassInfoKey(
          addonFC.addonFare()->fareClass().c_str(), geoAppl, owrt, sfClass.c_str());

      std::pair<AddonFareClassCombMultiMap::const_iterator,
                AddonFareClassCombMultiMap::const_iterator> range =
          _fareClassCombMultiMap->equal_range(fareClassInfoKey);

      AddonCombFareClassInfo* combFareClassInfo(nullptr);
      while (range.first != range.second)
      {
        combFareClassInfo = range.first->second;

        if (UNLIKELY(_cJob->dataHandle().isHistorical()))
        {
          fClassCombRecords.push_back(combFareClassInfo);
        }
        else
        {
          if (_date <= combFareClassInfo->expireDate())
          {
            fClassCombRecords.push_back(combFareClassInfo);
          }
        }
        ++range.first;
      }
    }
    else // new code for historical
    {
      if (nullptr == _fareClassCombVector)
      {
        _fareClassCombVector = &_cJob->dataHandle().getAddOnCombFareClassHistorical(
            _cJob->vendorCode(), _fareTariff, _cJob->carrier());
        _tariffCombFareClassVecMap.insert(std::make_pair(_fareTariff, _fareClassCombVector));
      }
      typedef std::pair<AddonCombFareClassInfoVec::const_iterator,
                        AddonCombFareClassInfoVec::const_iterator> SelectionRange;
      SortACFCKeyLess pred;
      AddonCombFareClassInfo ref;
      ref.addonFareClass() = addonFC.addonFare()->fareClass().c_str();
      ref.geoAppl() = geoAppl;
      ref.owrt() = owrt;
      ref.fareClass() = sfClass.c_str();
      SelectionRange range(boost::equal_range(*_fareClassCombVector, &ref, pred));
      fClassCombRecords.insert(fClassCombRecords.end(), range.first, range.second);
    }
    if (!fClassCombRecords.empty())
      matchResult = FM_GOOD_MATCH;
  }
  break;
  }
  return matchResult;
}

#endif
}
