//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/BaggageSectorCarrierApp.h"

namespace tse
{

bool
BaggageSectorCarrierApp::
operator==(const BaggageSectorCarrierApp& rhs) const
{
  return (_marketingCarrier == rhs._marketingCarrier) && (_seqNo == rhs._seqNo) &&
         (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
         (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
         (_operatingCarrier == rhs._operatingCarrier) && (_inclExclInd == rhs._inclExclInd) &&
         (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_flt1 == rhs._flt1) &&
         (_flt2 == rhs._flt2);
}

void
BaggageSectorCarrierApp::dummyData(BaggageSectorCarrierApp& obj)
{
  obj._marketingCarrier = "AA";
  obj._seqNo = 1;
  obj._createDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._effDate = time(nullptr);
  obj._discDate = time(nullptr);
  obj._operatingCarrier = "BB";
  obj._inclExclInd = 'C';
  LocKey::dummyData(obj._loc1);
  LocKey::dummyData(obj._loc2);
  obj._flt1 = 1111;
  obj._flt2 = 2222;
}

void
BaggageSectorCarrierApp::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _marketingCarrier);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _operatingCarrier);
  FLATTENIZE(archive, _inclExclInd);
  FLATTENIZE(archive, _loc1);
  FLATTENIZE(archive, _loc2);
  FLATTENIZE(archive, _flt1);
  FLATTENIZE(archive, _flt2);
}

} // tse
