//-------------------------------------------------------------------
//
//  File:        SitaConstructedFare.h
//  Created:     Feb 18, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents parts and extra fields for
//               one SITA constructed fare (result of an add-on
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

#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/DateInterval.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/SITAFareInfo.h"

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse
{

class FareInfo;

class ConstructedFareInfo;

class SitaConstructedFare : public ConstructedFare
{
public:
  // construction/destruction & assigment
  // ======================== = =========

  SitaConstructedFare();
  virtual ~SitaConstructedFare() {};

  // main interface
  // ==== =========

  /**
   * This methods obtains a new SitaConstructedFare pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual ConstructedFare* clone(DataHandle& dataHandle) const override;

  /**
   * This methods populates a given SitaConstructedFare object to be
   * 'equal' to the current object
   *
   * @param Fare - object to populate
   */
  virtual void clone(SitaConstructedFare& cloneObj) const;

  /**
   * This methods obtains a new ConstructedFareInfo pointer from
   * the Constructed Cache Manager and
   * populates it to be 'equal' to the current object
   *
   * @param ConstructionJob
   *
   * @return new object
   */
  virtual ConstructedFareInfo* cloneToConstructedFareInfo() override;

  /**
   * This methods populates a given ConstructedFareInfo object to be
   * 'equal' to the current object
   *
   * @param Fare - object to populate
   */
  virtual void cloneToConstructedFareInfo(ConstructedFareInfo& cfi) override;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  void initialize(ConstructionJob* cj,
                  const FareInfo& sf,
                  const GatewayPair& gw,
                  const bool oppositeSpecified) override;
#endif

  SitaFareDateInterval& dateIntervals()
  {
    return _dateIntervals;
  };
  const SitaFareDateInterval& dateIntervals() const
  {
    return _dateIntervals;
  };

  TSEDateInterval& effInterval() override { return _dateIntervals.effInterval(); }
  const TSEDateInterval& effInterval() const override { return _dateIntervals.effInterval(); }

  // accessors to SITA-specific fields in specified portion
  // ========= == ============= ====== == ========= =======

  virtual const RouteCode& routeCode() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->routeCode();
  }

  virtual const DBEClass& dbeClass() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->dbeClass();
  }

  virtual const Indicator fareQualCode() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->fareQualCode();
  }

  virtual const Indicator tariffFamily() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->tariffFamily();
  }

  virtual const Indicator cabotageInd() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->cabotageInd();
  }

  virtual const Indicator govtAppvlInd() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->govtAppvlInd();
  }

  virtual const Indicator constructionInd() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->constructionInd();
  }

  virtual const Indicator multiLateralInd() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->multiLateralInd();
  }

  virtual const LocCode& airport1() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->airport1();
  }

  virtual const LocCode& airport2() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->airport2();
  }

  virtual const Indicator viaCityInd() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->viaCityInd();
  }

  virtual const LocCode& viaCity() const
  {
    return static_cast<const SITAFareInfo*>(_specifiedFare)->viaCity();
  }

  // accessors to own SITA-specific fields
  // ========= == === ============= ======

  RoutingNumber& throughFareRouting() { return _throughFareRouting; }
  const RoutingNumber& throughFareRouting() const { return _throughFareRouting; }

  Indicator& throughMPMInd() { return _throughMPMInd; }
  const Indicator throughMPMInd() const { return _throughMPMInd; }

  RuleNumber& throughRule() { return _throughRule; }
  const RuleNumber& throughRule() const { return _throughRule; }

protected:
  SitaFareDateInterval _dateIntervals;

  RoutingNumber _throughFareRouting;
  Indicator _throughMPMInd;
  RuleNumber _throughRule;

  virtual void
  setAddonInterval(const DateIntervalBase& addonInterval, const bool isOriginAddon) override;

private:
  // Placed here so the clone methods must be used
  // ====== ==== == === ===== ======= ==== == ====

  SitaConstructedFare(SitaConstructedFare& rhs);
  SitaConstructedFare& operator=(const SitaConstructedFare& rhs);

}; // End of class SitaConstructedFare

} // End namespace tse

