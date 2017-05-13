//-------------------------------------------------------------------
//
//  File:        AtpcoConstructedFare.h
//  Created:     Feb 18, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents parts and extra fields for
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

#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/DateInterval.h"

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse
{

class AtpcoConstructedFare : public ConstructedFare
{
public:
  // construction/destruction & assigment
  // ======================== = =========

  AtpcoConstructedFare();
  virtual ~AtpcoConstructedFare() {};

  // main interface
  // ==== =========

  /**
   * This methods obtains a new AtpcoConstructedFare pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual ConstructedFare* clone(DataHandle& dataHandle) const override;

  /**
   * This methods populates a given AtpcoConstructedFare object to be
   * 'equal' to the current object
   *
   * @param Fare - object to populate
   */
  virtual void clone(AtpcoConstructedFare& cloneObj) const;

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
   * This methods populates a given ConstructedFareInfo object
   * to be 'equal' to the current object
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

  // accessors
  // =========

  AtpcoFareDateInterval& dateIntervals()
  {
    return _dateIntervals;
  };
  const AtpcoFareDateInterval& dateIntervals() const
  {
    return _dateIntervals;
  };

  TSEDateInterval& effInterval() override { return _dateIntervals.effInterval(); };
  const TSEDateInterval& effInterval() const override { return _dateIntervals.effInterval(); };

protected:
  AtpcoFareDateInterval _dateIntervals;
  unsigned int _fareClassPriority;

  virtual void
  setAddonInterval(const DateIntervalBase& addonInterval, const bool isOriginAddon) override;

  void defineFareClassPriority();

private:
  // Placed here so the clone methods must be used
  // ====== ==== == === ===== ======= ==== == ====

  AtpcoConstructedFare(AtpcoConstructedFare& rhs);
  AtpcoConstructedFare& operator=(const AtpcoConstructedFare& rhs);

}; // End of class AtpcoConstructedFare

} // End namespace tse

