#pragma once
//----------------------------------------------------------------------------
// IndustryPricingAppl.h
//
// Copyright Sabre 2004
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class IndustryPricingAppl
{
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Indicator& primePricingAppl() { return _primePricingAppl; }
  const Indicator& primePricingAppl() const { return _primePricingAppl; }

  Indicator& minimumFareAppl() { return _minimumFareAppl; }
  const Indicator& minimumFareAppl() const { return _minimumFareAppl; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  bool operator==(const IndustryPricingAppl& rhs) const
  {
    return ((_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_expireDate == rhs._expireDate) && (_discDate == rhs._discDate) &&
            (_carrier == rhs._carrier) && (_globalDir == rhs._globalDir) &&
            (_directionality == rhs._directionality) &&
            (_primePricingAppl == rhs._primePricingAppl) &&
            (_minimumFareAppl == rhs._minimumFareAppl) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_orderNo == rhs._orderNo));
  }

  static void dummyData(IndustryPricingAppl& obj)
  {
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._carrier = "ABC";
    obj._globalDir = GlobalDirection::US;
    obj._directionality = TO;
    obj._primePricingAppl = 'D';
    obj._minimumFareAppl = 'E';

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._orderNo = 1;
  }

private:
  DateTime _createDate;
  DateTime _effDate;
  DateTime _expireDate;
  DateTime _discDate;
  CarrierCode _carrier;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  Directionality _directionality = Directionality::TERMINATE;
  Indicator _primePricingAppl = ' ';
  Indicator _minimumFareAppl = ' ';
  LocKey _loc1;
  LocKey _loc2;
  int _orderNo = 0;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _primePricingAppl);
    FLATTENIZE(archive, _minimumFareAppl);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _orderNo);
  }
};

} // namespace tse

