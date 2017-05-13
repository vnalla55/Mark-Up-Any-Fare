//-------------------------------------------------------------------
//
//  File:        ConstructedFare.h
//  Created:     Feb 18, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents parts and extra fields common for
//               one constructed fare (result of an add-on
//               construction process)
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
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse
{
class DataHandle;

class FareInfo;
class ConstructionJob;
class AddonFareCortege;
class ConstructedFareInfo;
class GatewayPair;
class DateIntervalBase;
class TSEDateInterval;

class ConstructedFare
{
public:
  // construction/destruction & assigment
  // ======================== = =========

  ConstructedFare();
  virtual ~ConstructedFare() {};

  // main interface
  // ==== =========

  bool& valid()
  {
    return _valid;
  };
  const bool valid() const
  {
    return _valid;
  };

  const bool isValid() const;

  /**
   * This methods obtains a new AtpcoConstructedFare pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual ConstructedFare* clone(DataHandle& dataHandle) const = 0;

  /**
   * This methods populates a given ConstructedFare object to be
   * 'equal' to the current object
   *
   * @param Fare - object to populate
   */
  virtual void clone(ConstructedFare& cloneObj) const;

  /**
   * This methods obtains a new ConstructedFareInfo pointer from
   * the Constructed Cache Manager and
   * populates it to be 'equal' to the current object
   *
   * @param ConstructionJob
   *
   * @return new object
   */
  virtual ConstructedFareInfo* cloneToConstructedFareInfo() = 0;

  /**
   * This methods populates a given ConstructedFareInfo object
   * to be 'equal' to the current object
   *
   * @param Fare - object to populate
   */
  virtual void cloneToConstructedFareInfo(ConstructedFareInfo& cfi);

  virtual void initialize(ConstructionJob* cj,
                          const FareInfo& sf,
                          const GatewayPair& gw,
                          const bool oppositeSpecified);

  virtual void setAddon(const AddonFareCortege* addon,
                        const bool isOriginAddon,
                        const DateIntervalBase& addonInterval);

  void defineOriginDestination();

  Directionality defineDirectionality() const;

  void adjustPrevValues();

  // accessors
  // =========

  virtual TSEDateInterval& effInterval() = 0;
  virtual const TSEDateInterval& effInterval() const = 0;

  const AddonFareCortege*& origAddon()
  {
    return _origAddon;
  };
  const AddonFareCortege* origAddon() const
  {
    return _origAddon;
  };

  const FareInfo*& specifiedFare()
  {
    return _specifiedFare;
  };
  const FareInfo* specifiedFare() const
  {
    return _specifiedFare;
  };

  const AddonFareCortege*& destAddon()
  {
    return _destAddon;
  };
  const AddonFareCortege* destAddon() const
  {
    return _destAddon;
  };

  const AddonFareCortege*& origOrDestAddon(bool originAddon)
  {
    return originAddon ? _origAddon : _destAddon;
  };

  const AddonFareCortege* origOrDestAddon(bool originAddon) const
  {
    return originAddon ? _origAddon : _destAddon;
  };

  bool& isDoubleEnded()
  {
    return _isDoubleEnded;
  };
  const bool isDoubleEnded() const
  {
    return _isDoubleEnded;
  };

  LocCode& market1() { return _market1; }
  const LocCode& market1() const { return _market1; }

  LocCode& market2() { return _market2; }
  const LocCode& market2() const { return _market2; }

  LocCode& gateway1() { return _gateway1; }
  const LocCode& gateway1() const { return _gateway1; }

  LocCode& gateway2() { return _gateway2; }
  const LocCode& gateway2() const { return _gateway2; }

  bool& fareDisplayOnly() { return _fareDisplayOnly; }
  const bool fareDisplayOnly() const { return _fareDisplayOnly; }

  bool& pricingOnly() { return _pricingOnly; }
  const bool pricingOnly() const { return _pricingOnly; }

  TSEDateInterval& prevEffInterval() { return _prevEffInterval; }
  const TSEDateInterval& prevEffInterval() const { return _prevEffInterval; }

protected:
  // constructed fare parts
  // =========== ==== =====

  ConstructionJob* _cJob;

  const AddonFareCortege* _origAddon;
  const FareInfo* _specifiedFare;
  const AddonFareCortege* _destAddon;

  // origin and destination
  // ====== === ===========

  bool _valid;
  bool _isDoubleEnded;
  bool _isOppositeSpecified;

  LocCode _market1;
  LocCode _market2;

  LocCode _gateway1;
  LocCode _gateway2;

  // extra fields
  // ===== ======

  bool _fareDisplayOnly;
  bool _pricingOnly; // but not Fare Display!

  bool _prevFareDisplayOnly;
  TSEDateInterval _prevEffInterval;

  virtual void
  setAddonInterval(const DateIntervalBase& addonInterval, const bool isOriginAddon) = 0;

private:
  // Placed here so the clone methods must be used
  // ====== ==== == === ===== ======= ==== == ====

  ConstructedFare(ConstructedFare& rhs);
  ConstructedFare& operator=(const ConstructedFare& rhs);

}; // End of class ConstructedFare

} // End namespace tse

