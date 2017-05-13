//----------------------------------------------------------------------------
//  File: FDSuppressFare.h
//
//  Author: Partha Kumar Chakraborti
//    Description: This class acts as a data object which is passed to FDSuppressFareController
//                                for further processing.
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class FDSuppressFare
{
public:
  Indicator& pseudoCityType() { return _pseudoCityType; }
  const Indicator& pseudoCityType() const { return _pseudoCityType; }

  PseudoCityCode& pseudoCityCode() { return _pseudoCityCode; }
  const PseudoCityCode& pseudoCityCode() const { return _pseudoCityCode; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  Indicator& fareDisplayType() { return _fareDisplayType; }
  const Indicator& fareDisplayType() const { return _fareDisplayType; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  TJRGroup& ssgGroupNo() { return _ssgGroupNo; }
  const TJRGroup& ssgGroupNo() const { return _ssgGroupNo; }

  bool operator==(const FDSuppressFare& rhs) const
  {
    return ((_pseudoCityType == rhs._pseudoCityType) && (_pseudoCityCode == rhs._pseudoCityCode) &&
            (_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
            (_fareDisplayType == rhs._fareDisplayType) &&
            (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_ssgGroupNo == rhs._ssgGroupNo));
  }

  static void dummyData(FDSuppressFare& obj)
  {
    obj._pseudoCityType = 'A';
    obj._pseudoCityCode = "BCDEF";
    obj._carrier = ""; // DAO expects "", so don't change this one!
    obj._createDate = time(nullptr);
    obj._fareDisplayType = 'J';
    obj._directionality = BOTH;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._ssgGroupNo = 1;
  }

private:
  Indicator _pseudoCityType = ' '; // C1
  PseudoCityCode _pseudoCityCode; // C4
  CarrierCode _carrier; // C3
  DateTime _createDate;
  Indicator _fareDisplayType = ' '; // C1
  Directionality _directionality = Directionality::TERMINATE;
  LocKey _loc1;
  LocKey _loc2;
  TJRGroup _ssgGroupNo = 0;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCityCode);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _fareDisplayType);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _ssgGroupNo);
  }
};
} // End of namespace tse
