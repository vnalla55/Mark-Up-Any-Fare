#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareRetailerCalcDetailInfo
{
 public:

  FareRetailerCalcDetailInfo()
    : _fareRetailerCalcItemNo(0)
    , _orderNo(0)
    , _fareCalcInd(' ')
    , _percent1(0)
    , _percentNoDec1(0)
    , _percentMin1(0)
    , _percentMax1(0)
    , _amount1(0)
    , _amountNoDec1(0)
    , _amountMin1(0)
    , _amountMax1(0)
    , _percent2(0)
    , _percentNoDec2(0)
    , _percentMin2(0)
    , _percentMax2(0)
    , _amount2(0)
    , _amountNoDec2(0)
    , _amountMin2(0)
    , _amountMax2(0)
    , _hideOptnCd(' ')
  {
  }
  uint64_t& fareRetailerCalcItemNo() { return _fareRetailerCalcItemNo; }
  uint64_t fareRetailerCalcItemNo() const { return _fareRetailerCalcItemNo; }

  int& orderNo() { return _orderNo; }
  int orderNo() const { return _orderNo; }


  std::string&  calculationTypeCd() { return  _calculationTypeCd; }
  const std::string&  calculationTypeCd() const { return  _calculationTypeCd; }

  Indicator& fareCalcInd() { return _fareCalcInd; }
  Indicator fareCalcInd() const { return _fareCalcInd; }

  Percent& percent1() { return _percent1; }
  Percent percent1() const { return _percent1; }

  int& percentNoDec1() { return _percentNoDec1; }
  int percentNoDec1() const { return _percentNoDec1; }

  Percent& percentMin1() { return _percentMin1; }
  Percent percentMin1() const { return _percentMin1; }

  Percent& percentMax1() { return _percentMax1; }
  Percent percentMax1() const { return _percentMax1; }

  MoneyAmount& amount1() { return _amount1; }
  MoneyAmount amount1() const { return _amount1; }

  int& amountNoDec1() { return _amountNoDec1; }
  int amountNoDec1() const { return _amountNoDec1; }

  CurrencyCode& amountCurrency1() { return _amountCurrency1; }
  const CurrencyCode& amountCurrency1() const { return _amountCurrency1; }

  MoneyAmount& amountMin1() { return _amountMin1; }
  MoneyAmount amountMin1() const { return _amountMin1; }

  MoneyAmount& amountMax1() { return _amountMax1; }
  MoneyAmount amountMax1() const { return _amountMax1; }

  Percent& percent2() { return _percent2; }
  Percent percent2() const { return _percent2; }

  int&  percentNoDec2() { return _percentNoDec2; }
  int  percentNoDec2() const { return _percentNoDec2; }

  Percent& percentMin2() { return _percentMin2; }
  Percent percentMin2() const { return _percentMin2; }

  Percent& percentMax2() { return _percentMax2; }
  Percent percentMax2() const { return _percentMax2; }

  MoneyAmount& amount2() { return _amount2; }
  MoneyAmount amount2() const { return _amount2; }

  int&  amountNoDec2() { return _amountNoDec2; }
  int  amountNoDec2() const { return _amountNoDec2; }

  CurrencyCode& amountCurrency2() { return _amountCurrency2; }
  const CurrencyCode& amountCurrency2() const { return _amountCurrency2; }

  MoneyAmount& amountMin2() { return _amountMin2; }
  MoneyAmount amountMin2() const { return _amountMin2; }

  MoneyAmount& amountMax2() { return _amountMax2; }
  MoneyAmount amountMax2() const { return _amountMax2; }

  Indicator& hideOptnCd() { return _hideOptnCd; }
  Indicator hideOptnCd() const { return _hideOptnCd; }

  bool operator==(const FareRetailerCalcDetailInfo& rhs) const
  {
    return _fareRetailerCalcItemNo == rhs._fareRetailerCalcItemNo
           && _orderNo == rhs._orderNo
           && _calculationTypeCd    == rhs._calculationTypeCd
           && _fareCalcInd    == rhs._fareCalcInd
           && _percent1    == rhs._percent1
           && _percentNoDec1    == rhs._percentNoDec1
           && _percentMin1    == rhs._percentMin1
           && _percentMax1    == rhs._percentMax1
           && _amount1    == rhs._amount1
           && _amountNoDec1    == rhs._amountNoDec1
           && _amountCurrency1    == rhs._amountCurrency1
           && _amountMin1    == rhs._amountMin1
           && _amountMax1    == rhs._amountMax1
           && _percent2    == rhs._percent2
           && _percentNoDec2    == rhs._percentNoDec2
           && _percentMin2   == rhs._percentMin2
           && _percentMax2    == rhs._percentMax2
           && _amount2    == rhs._amount2
           && _amountNoDec2    == rhs._amountNoDec2
           && _amountCurrency2    == rhs._amountCurrency2
           && _amountMin2    == rhs._amountMin2
           && _amountMax2    == rhs._amountMax2
           && _hideOptnCd    == rhs._hideOptnCd;
  }

  static void dummyData(FareRetailerCalcDetailInfo& obj)
  {
    obj._fareRetailerCalcItemNo = 111111;
    obj._orderNo = 22;
    obj._calculationTypeCd = "ABC";
    obj._fareCalcInd = 'A';
    obj._percent1 = 2.22;
    obj._percentNoDec1 = 2;
    obj._percentMin1 = 2.33;
    obj._percentMax1 = 2.34;
    obj._amount1 = 3.23;
    obj._amountNoDec1 = 2;
    obj._amountCurrency1 = "USD";
    obj._amountMin1 = 3.33;
    obj._amountMax1 = 3.34;
    obj._percent2 = 5.55;
    obj._percentNoDec2 = 2;
    obj._percentMin2 = 5.01;
    obj._percentMax2 = 5.61;
    obj._amount2 = 600;
    obj._amountNoDec2 = 0;
    obj._amountCurrency2 = "XYZ";
    obj._amountMin2 = 605;
    obj._amountMax2 = 605;
    obj._hideOptnCd = 'C';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareRetailerCalcItemNo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _calculationTypeCd);
    FLATTENIZE(archive, _fareCalcInd);
    FLATTENIZE(archive, _percent1);
    FLATTENIZE(archive, _percentNoDec1);
    FLATTENIZE(archive, _percentMin1);
    FLATTENIZE(archive, _percentMax1);
    FLATTENIZE(archive, _amount1);
    FLATTENIZE(archive, _amountNoDec1);
    FLATTENIZE(archive, _amountCurrency1);
    FLATTENIZE(archive, _amountMin1);
    FLATTENIZE(archive, _amountMax1);
    FLATTENIZE(archive, _percent2);
    FLATTENIZE(archive, _percentNoDec2);
    FLATTENIZE(archive, _percentMin2);
    FLATTENIZE(archive, _percentMax2);
    FLATTENIZE(archive, _amount2);
    FLATTENIZE(archive, _amountNoDec2);
    FLATTENIZE(archive, _amountCurrency2);
    FLATTENIZE(archive, _amountMin2);
    FLATTENIZE(archive, _amountMax2);
    FLATTENIZE(archive, _hideOptnCd);
  }

 private:

  uint64_t _fareRetailerCalcItemNo;
  int _orderNo;
  std::string  _calculationTypeCd;
  Indicator _fareCalcInd;
  Percent _percent1;
  int _percentNoDec1;
  Percent _percentMin1;
  Percent _percentMax1;
  MoneyAmount _amount1;
  int _amountNoDec1;
  CurrencyCode _amountCurrency1;
  MoneyAmount _amountMin1;
  MoneyAmount _amountMax1;
  Percent _percent2;
  int  _percentNoDec2;
  Percent _percentMin2;
  Percent _percentMax2;
  MoneyAmount _amount2;
  int  _amountNoDec2;
  CurrencyCode _amountCurrency2;
  MoneyAmount _amountMin2;
  MoneyAmount _amountMax2;
  Indicator _hideOptnCd;

};

}// tse





