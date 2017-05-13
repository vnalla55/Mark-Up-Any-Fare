//----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#include "DBAccess/SITAAddonFareInfo.h"

#include "DBAccess/DataHandle.h"

namespace tse
{
AddonFareInfo*
SITAAddonFareInfo::clone(DataHandle& dataHandle) const
{
  SITAAddonFareInfo* cloneObj = nullptr;

  dataHandle.get(cloneObj);

  clone(*cloneObj); // lint !e413

  return cloneObj;
}

void
SITAAddonFareInfo::clone(AddonFareInfo& cloneObj) const
{
  SITAAddonFareInfo& afClone = dynamic_cast<SITAAddonFareInfo&>(cloneObj);

  afClone._baseFareRouting = _baseFareRouting;
  afClone._thruFareRouting = _thruFareRouting;
  afClone._baseMPMInd = _baseMPMInd;
  afClone._throughMPMInd = _throughMPMInd;
  afClone._ruleTariff = _ruleTariff;
  afClone._throughRule = _throughRule;
  afClone._ruleExcludeInd = _ruleExcludeInd;
  afClone._gatewayZone = _gatewayZone;
  afClone._interiorZone = _interiorZone;
  afClone._classFareBasisInd = _classFareBasisInd;
  afClone._globalClassFlag = _globalClassFlag;
  afClone._routeCode = _routeCode;
  afClone._tariffFamily = _tariffFamily;
  afClone._fareQualInd = _fareQualInd;
  afClone._fareQualCodes = _fareQualCodes;
  afClone._dbeClasses = _dbeClasses;
  afClone._rules = _rules;

  AddonFareInfo::clone(cloneObj);
}
}
