//-------------------------------------------------------------------
//
//  File:        AddonZoneMap.h
//  Created:     Feb 25, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class to validate AddonZone for given AddonFare
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

#pragma once

#include "AddonConstruction/ConstructionDefs.h"


namespace tse
{

class ConstructionJob;
class AddonZoneInfo;

class AddonZoneMap
{
public:
  // construction/destruction
  // ========================

  AddonZoneMap() : _cJob(nullptr), _populated(false) {};
  virtual ~AddonZoneMap() {};

  // main interface
  // ==== =========

  const bool isPopulated() const
  {
    return _populated;
  };

  void populate(ConstructionJob& cJob, const ConstructionPoint constructionPoint);

  bool validateZones(const AddonZone zone,
                     const TariffNumber tariff,
                     const TSEDateInterval& afi,
                     TSEDateIntervalVec& zoneIntervals) const;

protected:
  // map to validate add-on zones

  class AddonZoneKey
  {
  public:
    // construction/destruction
    // ========================

    AddonZoneKey() : _zone(0), _tariff(0) {};

    AddonZoneKey(const AddonZone& zone, const TariffNumber tariff)
      : _zone(zone), _tariff(tariff) {};

    AddonZoneKey(const AddonZoneKey& rhs) : _zone(rhs._zone), _tariff(rhs._tariff) {};

    const AddonZone zone() const
    {
      return _zone;
    };
    const TariffNumber tariff() const
    {
      return _tariff;
    };

    // Assignment
    // ==========

    AddonZoneKey& operator=(const AddonZoneKey& rhs)
    {
      if (this != &rhs)
      {
        _zone = rhs._zone;
        _tariff = rhs._tariff;
      }
      return *this;
    }

    // Comparison
    // ==========

    friend bool operator<(const AddonZoneKey& x, const AddonZoneKey& y)
    {
      if (x._zone != y._zone)
        return (x._zone < y._zone);

      if (x._tariff != y._tariff)
        return (x._tariff < y._tariff);

      return false;
    }

    friend bool operator==(const AddonZoneKey& x, const AddonZoneKey& y)
    {
      return (x._zone == y._zone && x._tariff == y._tariff);
    }

    friend bool operator!=(const AddonZoneKey& x, const AddonZoneKey& y)
    {
      return (x._zone != y._zone || x._tariff != y._tariff);
    }

  protected:
    AddonZone _zone;
    TariffNumber _tariff;

  }; // End class AddonZoneKey

  typedef std::multimap<AddonZoneKey, TSEDateInterval> ZoneMap;

  typedef ZoneMap::iterator ZoneMapI;
  typedef std::pair<ZoneMapI, ZoneMapI> ZoneMapIPair;

  typedef ZoneMap::const_iterator ZoneMapCI;
  typedef std::pair<ZoneMapCI, ZoneMapCI> ZoneMapCIPair;

  ConstructionJob* _cJob;
  ZoneMap _zoneMap;
  bool _populated;

  void populate(const LocCode& location, bool collapseDuplicates);

  const AddonZoneInfoVec& getAddOnZones(const LocCode& location);

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  AddonZoneMap(const AddonZoneMap& rhs);
  AddonZoneMap operator=(const AddonZoneMap& rhs);

}; // End class AddonZoneMap

} // End namespace tse

