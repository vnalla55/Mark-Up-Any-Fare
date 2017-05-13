//-------------------------------------------------------------------
//
//  File:        TrfXrefMap.cpp
//  Created:     Jun 11, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class to match together AddonTarif1,AddonTarif2 and
//               FareTariff.
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

#include "AddonConstruction/TrfXrefMap.h"

#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DateInterval.h"
#include "Common/Logger.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/Diag253Collector.h"
#include "Util/BranchPrediction.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

static Logger
logger("atseintl.AddonConstruction.TrfXrefMap");

void
TrfXrefMap::init(ConstructionJob* cj)
{
  _cJob = cj;
}

bool
TrfXrefMap::populate()
{
  if (UNLIKELY(_isPopulated))
    return true;

  Diag253Collector* diag253{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag253 = _cJob->diagnostic<Diag253Collector>();
  }
  else
  {
    diag253 = _cJob->diag253();
  }
  if (UNLIKELY(diag253 != nullptr))
    diag253->writeTariffXRefHeader(_cJob->vendorCode());

  // get all TrfXref records for specified vendor/carrier

  const std::vector<TariffCrossRefInfo*>& tcrInfos =
      _cJob->dataHandle().getTariffXRef(_cJob->vendorCode(), _cJob->carrier(), INTERNATIONAL);

  std::vector<TariffCrossRefInfo*>::const_iterator i = tcrInfos.begin();
  std::vector<TariffCrossRefInfo*>::const_iterator ie = tcrInfos.end();
  for (; i != ie; ++i)
  {
    TariffNumber aTrf1 = (*i)->addonTariff1();
    TariffNumber aTrf2 = (*i)->addonTariff2();

    if ((*i)->fareTariff() > 0 && (aTrf1 > 0 || aTrf2 > 0))
    {
      Indicator inhibit = ' ';
      if (LIKELY(aTrf1 > 0))
        inhibit = addMapItem(**i, aTrf1);

      if (aTrf2 > 0)
        inhibit = addMapItem(**i, aTrf2);

      if (inhibit != INHIBIT_F && aTrf1 > 0 && aTrf2 > 0)
      {
        _addonTariffMap[aTrf1].insert(aTrf2);
        _addonTariffMap[aTrf2].insert(aTrf1);
      }

      if (UNLIKELY(diag253 != nullptr))
        diag253->writeTariffXRefRef(**i, (inhibit == INHIBIT_F));
    }
  }

  _isPopulated = true;

  if (UNLIKELY(diag253 != nullptr))
    diag253->writeTariffXRefFooter();

  return true;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

FareMatchCode
TrfXrefMap::matchFareAndAddonTariff(const TariffNumber fareTariff,
                                    const TariffNumber addonTariff,
                                    bool isHistorical,
                                    TSEDateInterval& validDI)
{
  if (UNLIKELY(!_isPopulated))
    populate();

  const TariffMapKey key(fareTariff, addonTariff);
  const TariffMapCIPair& xrefs = _fareTariffMap.equal_range(key);
  if (xrefs.first == _fareTariffMap.end() || xrefs.first->first != key)
    return FM_FARE_TARIFF;

  TSEDateInterval latestDI;
  for (TariffMapCI i = xrefs.first; i != xrefs.second; ++i)
  {
    if (LIKELY(i->second->inhibit() != INHIBIT_F))
    {
      TSEDateInterval di;
      if (di.defineIntersection(validDI, *i->second))
      {
        if (LIKELY(!isHistorical))
        {
          validDI = di;
          return FM_GOOD_MATCH;
        }
        else
        {
          if (di.expireDate() > latestDI.expireDate())
            latestDI = di;
        }
      }
    }
  }

  if (UNLIKELY(isHistorical))
  {
    if (!latestDI.expireDate().isEmptyDate())
    {
      validDI = latestDI;
      return FM_GOOD_MATCH;
    }
  }

  return FM_FARE_TARIFF;
}

#else

FareMatchCode
TrfXrefMap::matchFareAndAddonTariff(const TariffNumber fareTariff,
                                    const TariffNumber addonTariff,
                                    InhibitedDateIntervalVec& xrefRecords)
{
  if (UNLIKELY(!_isPopulated))
    populate();

  // find a range of records for specified fareTariff & addonTariff

  const TariffMapKey key(fareTariff, addonTariff);

  TariffMapCIPair xrefs = _fareTariffMap.equal_range(key);

  // equal_range returns position where an element can be inserted.
  // but it might not exist as well. so, two more checks...

  if (xrefs.first == _fareTariffMap.end())
    return FM_FARE_TARIFF;

  else if (xrefs.first->first != key)
    return FM_FARE_TARIFF;

  // send upstairs all found records

  for (TariffMapCI i = xrefs.first; i != xrefs.second; ++i)
    if (LIKELY(i->second->inhibit() != INHIBIT_F))
      xrefRecords.push_back(i->second);

  return (xrefRecords.empty() ? FM_FARE_TARIFF : FM_GOOD_MATCH);
}

#endif

bool
TrfXrefMap::matchAddonTariffs(const TariffNumber addonTariff1, const TariffNumber addonTariff2)
{
  if (addonTariff1 == addonTariff2)
    return true;

  if (UNLIKELY(!_isPopulated))
    populate();

  // find a set of add-on tariffs assotiated with addonTariff1

  AddonTarrifXrefMap::const_iterator i = _addonTariffMap.find(addonTariff1);

  if (i == _addonTariffMap.end())
    return false;

  // find addonTariff2 in the set

  const TariffNumberSet& tns = i->second;

  return (tns.find(addonTariff2) != tns.end());
}

const TrfXrefMap::TariffNumberSet*
TrfXrefMap::getAssociatedTariffSet(const TariffNumber addonTariff)
{
  if (!_isPopulated)
    populate();

  AddonTarrifXrefMap::const_iterator i = _addonTariffMap.find(addonTariff);

  if (i == _addonTariffMap.end())
    return nullptr;

  return &(i->second);
}

Indicator
TrfXrefMap::addMapItem(const TariffCrossRefInfo& tcrInfo, const TariffNumber addonTariff)
{
  InhibitedDateInterval* idi;
  // lint -e{530}
  _cJob->dataHandle().get(idi);

  tcrInfo.effInterval().cloneDateInterval(*idi);

  idi->inhibit() = _cJob->dataHandle().getTariffInhibit(
      tcrInfo.vendor(), 'I', tcrInfo.carrier(), tcrInfo.fareTariff(), tcrInfo.ruleTariffCode());

  _fareTariffMap.insert(std::pair<TariffMapKey, InhibitedDateInterval*>(
      TariffMapKey(tcrInfo.fareTariff(), addonTariff), idi));

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  _trfGlobals.emplace(tcrInfo.carrier(), addonTariff, tcrInfo.globalDirection());
#endif

  return idi->inhibit();
}


#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
bool
TrfXrefMap::isGlobalDirValid(const CarrierCode& cxr,
                             TariffNumber addonTrf,
                             GlobalDirection globalDir)
{
  if (!_isPopulated)
    populate();

  return _trfGlobals.find(TrfGlobal(cxr, addonTrf, globalDir)) != _trfGlobals.end();
}
#endif
}
