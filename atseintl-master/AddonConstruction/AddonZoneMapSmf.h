//-------------------------------------------------------------------
//
//  File:        AddonZoneMapSmf.h
//  Created:     May 24, 2006
//  Authors:     Vadim Nikushin
//
//  Description: Class to validate AddonZone for given SMF AddonFare
//
//  Copyright Sabre 2006
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

class AddonZoneMapSmf
{
public:
  // construction/destruction
  // ========================

  AddonZoneMapSmf() : _cJob(nullptr) {};
  virtual ~AddonZoneMapSmf() {};

  // main interface
  // ==== =========

  void initialize(ConstructionJob* cjob);

  bool validateZones(const ConstructionPoint cp,
                     const AddonZone zone,
                     const TariffNumber tariff,
                     const TSEDateInterval& afi,
                     TSEDateIntervalVec& zoneIntervals);

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

  typedef std::pair<bool, TSEDateInterval> AddonZoneStatus;
  typedef std::pair<AddonZoneKey, AddonZoneStatus> ZoneMapPair;

  typedef std::map<AddonZoneKey, AddonZoneStatus> ZoneMap;

  typedef ZoneMap::iterator ZoneMapI;
  typedef std::pair<ZoneMapI, ZoneMapI> ZoneMapIPair;

  typedef ZoneMap::const_iterator ZoneMapCI;
  typedef std::pair<ZoneMapCI, ZoneMapCI> ZoneMapCIPair;

  ConstructionJob* _cJob;
  ZoneMap _zoneMap;

  bool validateZones(const ConstructionPoint cp,
                     const AddonZone zone,
                     const TariffNumber tariff,
                     TSEDateInterval& zoneInterval);

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  AddonZoneMapSmf(const AddonZoneMapSmf& rhs);
  AddonZoneMapSmf operator=(const AddonZoneMapSmf& rhs);

}; // End class AddonZoneMapSmf

} // End namespace tse

