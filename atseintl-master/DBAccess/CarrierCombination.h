//----------------------------------------------------------------------------
//
//      File:           CarrierCombination.h
//      Description:    TaxNation processing data
//
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//   ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class CarrierCombination : public RuleItemInfo
{
public:
  CarrierCombination()
    : _orderNo(0),
      _tvlSectSi(0),
      _carrierCombAppl(' '),
      _tvlSecOverwaterInd(' '),
      _tvlSecIntlInd(' '),
      _tvlSecInOutboundInd(' '),
      _tvlSecConstInd(' '),
      _loc1Type(' '),
      _loc2Type(' ')
  {
  }

  virtual bool operator==(const CarrierCombination& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_orderNo == rhs._orderNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_tvlSectSi == rhs._tvlSectSi) && (_carrierCombAppl == rhs._carrierCombAppl) &&
            (_carriers == rhs._carriers) && (_tvlSecOverwaterInd == rhs._tvlSecOverwaterInd) &&
            (_tvlSecIntlInd == rhs._tvlSecIntlInd) &&
            (_tvlSecInOutboundInd == rhs._tvlSecInOutboundInd) &&
            (_tvlSecConstInd == rhs._tvlSecConstInd) && (_loc1Type == rhs._loc1Type) &&
            (_loc1 == rhs._loc1) && (_loc2Type == rhs._loc2Type) && (_loc2 == rhs._loc2));
  }

  static void dummyData(CarrierCombination& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._orderNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._tvlSectSi = 2;
    obj._carrierCombAppl = 'A';

    obj._carriers.push_back("abc");
    obj._carriers.push_back("def");

    obj._tvlSecOverwaterInd = 'B';
    obj._tvlSecIntlInd = 'C';
    obj._tvlSecInOutboundInd = 'D';
    obj._tvlSecConstInd = 'E';
    obj._loc1Type = 'F';
    obj._loc1 = "ABCDEFGH";
    obj._loc2Type = 'I';
    obj._loc2 = "JKLMNOPQ";
  }

private:
  int _orderNo;
  DateTime _createDate;
  DateTime _expireDate;
  TvlSectSiCode _tvlSectSi;
  Indicator _carrierCombAppl;
  std::vector<CarrierCode> _carriers;
  Indicator _tvlSecOverwaterInd;
  Indicator _tvlSecIntlInd;
  Indicator _tvlSecInOutboundInd;
  Indicator _tvlSecConstInd;
  Indicator _loc1Type;
  LocCode _loc1;
  Indicator _loc2Type;
  LocCode _loc2;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _tvlSectSi);
    FLATTENIZE(archive, _carrierCombAppl);
    FLATTENIZE(archive, _carriers);
    FLATTENIZE(archive, _tvlSecOverwaterInd);
    FLATTENIZE(archive, _tvlSecIntlInd);
    FLATTENIZE(archive, _tvlSecInOutboundInd);
    FLATTENIZE(archive, _tvlSecConstInd);
    FLATTENIZE(archive, _loc1Type);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2Type);
    FLATTENIZE(archive, _loc2);
  }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  TvlSectSiCode& tvlSectSi() { return _tvlSectSi; }
  const TvlSectSiCode& tvlSectSi() const { return _tvlSectSi; }

  Indicator& carrierCombAppl() { return _carrierCombAppl; }
  const Indicator& carrierCombAppl() const { return _carrierCombAppl; }

  std::vector<CarrierCode>& carriers() { return _carriers; }
  const std::vector<CarrierCode>& carriers() const { return _carriers; }

  Indicator& tvlSecOverwaterInd() { return _tvlSecOverwaterInd; }
  const Indicator& tvlSecOverwaterInd() const { return _tvlSecOverwaterInd; }

  Indicator& tvlSecIntlInd() { return _tvlSecIntlInd; }
  const Indicator& tvlSecIntlInd() const { return _tvlSecIntlInd; }

  Indicator& tvlSecInOutboundInd() { return _tvlSecInOutboundInd; }
  const Indicator& tvlSecInOutboundInd() const { return _tvlSecInOutboundInd; }

  Indicator& tvlSecConstInd() { return _tvlSecConstInd; }
  const Indicator& tvlSecConstInd() const { return _tvlSecConstInd; }

  Indicator& loc1Type() { return _loc1Type; }
  const Indicator& loc1Type() const { return _loc1Type; }

  LocCode& loc1() { return _loc1; }
  const LocCode& loc1() const { return _loc1; }

  Indicator& loc2Type() { return _loc2Type; }
  const Indicator& loc2Type() const { return _loc2Type; }

  LocCode& loc2() { return _loc2; }
  const LocCode& loc2() const { return _loc2; }
};
}

