//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/TseEnums.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/IbfAvailabilityTools.h"

namespace tse
{

class IbfAvlTracker
{
public:
  void setStatus(const BrandCode& brandCode, IbfErrorMessage ibfErrorMessage);

  void setStatusForLeg(const BrandCode& brandCode, LegId legId, IbfErrorMessage ibfErrorMessage);

  IbfErrorMessage getStatusForLeg(const BrandCode& brandCode, LegId legId) const;

  IbfErrorMessage getStatusForBrand(const BrandCode& brandCode) const;

  bool atLeastOneLegValidForBrand(const BrandCode& brandCode) const;

private:
  typedef std::map<BrandCode, IbfErrorMessage> BrandErrors;
  IbfErrorMessageMapPerLeg _brandErrorsPerLeg;
  BrandErrors _brandErrors;
};

template <typename ToolkitType = IbfAvailabilityTools>
class TranslatingIbfAvailabilityTracker
{
public:
  void setStatus(const BrandCode& brandCode, IbfErrorMessage ibfErrorMessage)
  {
    _tracker.setStatus(brandCode, ibfErrorMessage);
  }

  void setStatusForLeg(const BrandCode& brandCode, LegId legId, IbfErrorMessage ibfErrorMessage)
  {
    _tracker.setStatusForLeg(brandCode, legId, ibfErrorMessage);
  }

  IbfErrorMessage getStatusForLegHardPassFix(const BrandCode& brandCode, LegId legId) const
  {
    return _tracker.getStatusForLeg(brandCode, legId);
  }

  IbfErrorMessage getStatusForLeg(const BrandCode& brandCode, LegId legId) const
  {
    IbfErrorMessage answer = _tracker.getStatusForLeg(brandCode, legId);
    return ToolkitType::translateForOutput(answer);
  }

  IbfErrorMessage getStatusForBrand(const BrandCode& brandCode) const
  {
    IbfErrorMessage answer = _tracker.getStatusForBrand(brandCode);
    return ToolkitType::translateForOutput(answer);
  }

  bool atLeastOneLegValidForBrand(const BrandCode& brandCode) const
  {
    return _tracker.atLeastOneLegValidForBrand(brandCode);
  }

private:
  IbfAvlTracker _tracker;
};

typedef TranslatingIbfAvailabilityTracker<> IbfAvailabilityTracker;

} // namespace tse

