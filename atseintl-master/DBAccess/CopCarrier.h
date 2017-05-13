//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef COPCARRIER_H
#define COPCARRIER_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class CopCarrier
{
public:
  CopCarrier() : _participationInd(' ') {}
  CarrierCode& copCarrier() { return _copCarrier; }
  const CarrierCode& copCarrier() const { return _copCarrier; }

  Indicator& participationInd() { return _participationInd; }
  const Indicator& participationInd() const { return _participationInd; }

  bool operator==(const CopCarrier& rhs) const
  {
    return ((_copCarrier == rhs._copCarrier) && (_participationInd == rhs._participationInd));
  }

  static void dummyData(CopCarrier& obj)
  {
    obj._copCarrier = "ABC";
    obj._participationInd = 'D';
  }

private:
  CarrierCode _copCarrier;
  Indicator _participationInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _copCarrier);
    FLATTENIZE(archive, _participationInd);
  }

};
}
#endif
