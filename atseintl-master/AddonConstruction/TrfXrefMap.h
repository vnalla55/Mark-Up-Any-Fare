//-------------------------------------------------------------------
//
//  File:        TrfXrefMap.h
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

#pragma once

#include "AddonConstruction/ConstructionDefs.h"


#include <set>
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include <unordered_set>
#include <boost/functional/hash.hpp>
#endif

namespace tse
{

class ConstructionJob;
class TariffCrossRefInfo;
class InhibitedDateInterval;

class TrfXrefMap
{
public:
  // map to search add-on tariffs combinable with targeted
  // add-on tariff.

  typedef std::set<TariffNumber> TariffNumberSet;
  typedef std::map<TariffNumber, TariffNumberSet> AddonTarrifXrefMap;

  // construction/destruction
  // ========================

  TrfXrefMap() : _cJob(nullptr), _isPopulated(false) {};
  virtual ~TrfXrefMap() {};

  // main interface
  // ==== =========

  void init(ConstructionJob* cj);

  bool populate();

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  FareMatchCode matchFareAndAddonTariff(const TariffNumber fareTariff,
                                        const TariffNumber addonTariff,
                                        bool isHistorical,
                                        TSEDateInterval& validDI);
#else
  FareMatchCode matchFareAndAddonTariff(const TariffNumber fareTariff,
                                        const TariffNumber addonTariff,
                                        InhibitedDateIntervalVec& xrefRecords);
#endif

  bool matchAddonTariffs(const TariffNumber addonTariff1, const TariffNumber addonTariff2);

  const TariffNumberSet* getAssociatedTariffSet(const TariffNumber addonTariff);
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  bool isGlobalDirValid(const CarrierCode& cxr,
                        TariffNumber addonTrf,
                        GlobalDirection globalDir);
#endif

protected:
  // map to search applicable add-on tariffs by specified fare tariff

  class TariffMapKey
  {
  public:
    TariffMapKey() : _fareTariff(0), _addonTariff(0) {};

    TariffMapKey(const TariffNumber fareTariff, const TariffNumber addonTariff)
      : _fareTariff(fareTariff), _addonTariff(addonTariff) {};

    TariffMapKey& operator=(const TariffMapKey& rhs)
    {
      if (this != &rhs)
      {
        _fareTariff = rhs._fareTariff;
        _addonTariff = rhs._addonTariff;
      }

      return *this;
    }

    friend bool operator<(const TariffMapKey& x, const TariffMapKey& y)
    {
      if (x._fareTariff != y._fareTariff)
        return (x._fareTariff < y._fareTariff);

      if (x._addonTariff != y._addonTariff)
        return (x._addonTariff < y._addonTariff);

      return false;
    }

    friend bool operator==(const TariffMapKey& x, const TariffMapKey& y)
    {
      return (x._fareTariff == y._fareTariff && x._addonTariff == y._addonTariff);
    }

    friend bool operator!=(const TariffMapKey& x, const TariffMapKey& y)
    {
      return (x._fareTariff != y._fareTariff || x._addonTariff != y._addonTariff);
    }

  protected:
    TariffNumber _fareTariff;
    TariffNumber _addonTariff;
  };

  typedef std::multimap<TariffMapKey, InhibitedDateInterval*> TariffMap;

  typedef TariffMap::const_iterator TariffMapCI;
  typedef std::pair<TariffMapCI, TariffMapCI> TariffMapCIPair;

  TariffMap _fareTariffMap;
  AddonTarrifXrefMap _addonTariffMap;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  typedef std::tuple<CarrierCode, TariffNumber, GlobalDirection> TrfGlobal;
  typedef std::unordered_set<TrfGlobal, boost::hash<TrfGlobal> > TrfGlobals;
  TrfGlobals _trfGlobals;
#endif

  ConstructionJob* _cJob;

  bool _isPopulated;

  Indicator addMapItem(const TariffCrossRefInfo& tcrInfo, const TariffNumber addonTariff);

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  TrfXrefMap(const TrfXrefMap& rhs);
  TrfXrefMap operator=(const TrfXrefMap& rhs);

}; // End class TrfXrefMap

} // End namespace tse

