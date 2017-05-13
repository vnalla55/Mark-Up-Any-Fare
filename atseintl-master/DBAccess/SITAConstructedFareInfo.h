//-------------------------------------------------------------------
//
//  File:        SITAConstructedFareInfo.h
//  Created:     Feb 14, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents common data and members of
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/Flattenizable.h"
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

class SITAConstructedFareInfo : public ConstructedFareInfo
{
public:
  // construction/destruction & assignment
  // ======================== = ==========

  SITAConstructedFareInfo();
  const eConstructedFareInfoType objectType() const override { return eSITAConstructedFareInfo; }

  // main interface
  // ==== =========

  /**
   * This methods obtains a new SITAConstructedFareInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  ConstructedFareInfo* clone(DataHandle& dataHandle) const override;

  /**
   * This methods populates a given SITAConstructedFareInfo object to be
   * 'equal' to the current object
   *
   * @param cloneObj - object to populate
   */

  virtual void clone(SITAConstructedFareInfo& cloneObj) const;

  // accessors
  // =========

  RoutingNumber& throughFareRouting() { return _throughFareRouting; }
  const RoutingNumber& throughFareRouting() const { return _throughFareRouting; }

  Indicator& throughMPMInd() { return _throughMPMInd; }
  const Indicator throughMPMInd() const { return _throughMPMInd; }

  RuleNumber& throughRule() { return _throughRule; }
  const RuleNumber& throughRule() const { return _throughRule; }

  virtual bool operator==(const SITAConstructedFareInfo& rhs) const
  {
    return ((ConstructedFareInfo::operator==(rhs)) &&
            (_throughFareRouting == rhs._throughFareRouting) &&
            (_throughMPMInd == rhs._throughMPMInd) && (_throughRule == rhs._throughRule));
  }

  static void dummyData(SITAConstructedFareInfo& obj)
  {
    ConstructedFareInfo::dummyData(obj);

    obj._throughFareRouting = "ABCD";
    obj._throughMPMInd = 'E';
    obj._throughRule = "FGHI";
  }

protected:
  RoutingNumber _throughFareRouting;
  Indicator _throughMPMInd;
  RuleNumber _throughRule;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, ConstructedFareInfo);
    FLATTENIZE(archive, _throughFareRouting);
    FLATTENIZE(archive, _throughMPMInd);
    FLATTENIZE(archive, _throughRule);
  }

protected:
  void dumpObject(std::ostream& os) const override;
  void dummyData() override;

private:
  void initialize();

  // Placed here so the clone methods must be used
  // ====== ==== == === ===== ======= ==== == ====

  SITAConstructedFareInfo(SITAConstructedFareInfo& rhs);
  SITAConstructedFareInfo& operator=(const SITAConstructedFareInfo& rhs);

}; // End of class SITAConstructedFareInfo

} // End namespace tse

