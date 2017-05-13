//-------------------------------------------------------------------
//
//  File:        AddonFareCortege.h
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Utility class to keep status of add-on fare
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
#include "AddonConstruction/DateInterval.h"

namespace tse
{

class AddonFareInfo;
class ConstructionJob;

class AddonFareCortege
{
public:
  // public types
  // ====== =====

  // enum "AddonFareClass" works in 2 different ways:
  // 1) it stores type of add-on fare class
  // 2) each type of add-on fare class has priority number which
  //    important for ATPCO fare class hierarchy processing.
  //
  // if for some reason you need to add/delete/modify definition of
  // this type please make sure you are not destroying ATPCO fare
  // class hierarchy processing!

  enum AddonFareClass
  {
    REGULAR = 3,
    ALPHA_FIVE_STAR = 2,
    SIX_STAR = 1
  };

  enum AddonFootnote
  {
    DIRECTIONAL_FOOTNOTE_NOT_FOUND = 0,
    FOOTNOTE_TO,
    FOOTNOTE_FROM
  };

  // construction/destruction
  // ========================

  AddonFareCortege()
    : _addonFare(nullptr),
      _seqNum(0),
      _gatewayFareCount(0),
      _addonFareClass(REGULAR),
      _addonFootNote(DIRECTIONAL_FOOTNOTE_NOT_FOUND),
      _fareDisplayOnly(false)
  {
  }

  virtual ~AddonFareCortege() {};

  // main interface
  // ==== =========

  void initialize(ConstructionJob& cj,
                  const AddonFareInfo* addonFare,
                  const TSEDateInterval& zoneInterval,
                  const Indicator splittedPart);

  // accessors
  // =========

  const AddonFareInfo*& addonFare() { return _addonFare; }
  const AddonFareInfo* addonFare() const { return _addonFare; }

  unsigned int& seqNum() { return _seqNum; }
  const unsigned int seqNum() const { return _seqNum; }

  unsigned int& gatewayFareCount() { return _gatewayFareCount; }
  const unsigned int gatewayFareCount() const { return _gatewayFareCount; }

  const AddonFareClass addonFareClass() const { return _addonFareClass; }
  const AddonFootnote addonFootNote() const { return _addonFootNote; }

  // value of _addonFareClass is using in this function to define
  // add-on score for ATPCO fare class hierarchy. other class will
  // summurize several scores so enum was converted to unsigned int

  const unsigned int atpcoFareClassPriority() const { return _addonFareClass; }

  TSEDateInterval& effInterval()
  {
    return _dateIntervals.effInterval();
  }
  const TSEDateInterval& effInterval() const
  {
    return _dateIntervals.effInterval();
  }

  AddonCortegeDateInterval& dateIntervals() { return _dateIntervals; }
  const AddonCortegeDateInterval& dateIntervals() const { return _dateIntervals; }

  bool& fareDisplayOnly() { return _fareDisplayOnly; }
  const bool fareDisplayOnly() const { return _fareDisplayOnly; }

protected:
  const AddonFareInfo* _addonFare;

  unsigned int _seqNum; // fare number (from the
  // begin of fare vector)
  unsigned int _gatewayFareCount; // number of fares for
  // this gateway
  AddonFareClass _addonFareClass;
  AddonFootnote _addonFootNote;

  AddonCortegeDateInterval _dateIntervals;

  bool _fareDisplayOnly;

  void parseFootnotes();

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  AddonFareCortege(const AddonFareCortege& rhs);
  AddonFareCortege operator=(const AddonFareCortege& rhs);

}; // End class AddonFareCorteges

} // End namespace tse

