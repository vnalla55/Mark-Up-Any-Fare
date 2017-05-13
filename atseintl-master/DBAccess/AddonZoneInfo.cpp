//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AddonZoneInfo.h"

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

bool
AddonZoneInfo::
operator==(const AddonZoneInfo& rhs) const
{
  return ((_effInterval == rhs._effInterval) && (_vendor == rhs._vendor) &&
          (_fareTariff == rhs._fareTariff) && (_carrier == rhs._carrier) &&
          (_inclExclInd == rhs._inclExclInd) && (_market == rhs._market) && (_zone == rhs._zone));
}

WBuffer&
AddonZoneInfo::write(WBuffer& os) const
{
  return convert(os, this);
}

RBuffer&
AddonZoneInfo::read(RBuffer& is)
{
  return convert(is, this);
}

void
AddonZoneInfo::dummyData(AddonZoneInfo& obj)
{
  obj.effInterval().createDate() = time(nullptr);
  obj.effInterval().effDate() = time(nullptr);
  obj.effInterval().expireDate() = time(nullptr);
  obj.effInterval().discDate() = time(nullptr);
  obj.vendor() = "ABCD";
  obj.fareTariff() = 1;
  obj.carrier() = "EFG";
  obj.inclExclInd() = 'H';
  obj.market().loc() = "IJKLMNOP";
  obj.market().locType() = 'Q';
  obj.zone() = 2;
}

void
AddonZoneInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _effInterval);
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _carrier);
  FLATTENIZE(archive, _inclExclInd);
  FLATTENIZE(archive, _market);
  FLATTENIZE(archive, _zone);
}

} // tse
