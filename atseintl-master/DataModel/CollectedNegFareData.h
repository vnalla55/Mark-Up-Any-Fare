//-------------------------------------------------------------------
//
//  File:        CollectedNegFareData.h
//  Created:     Oct 07, 2004
//  Authors:     Vladimir Koliasnikov
//
//  Description:
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{
class CollectedNegFareData
{
public:
  CollectedNegFareData();

  bool& indicatorCat35() { return _indicatorCat35; }
  const bool indicatorCat35() const { return _indicatorCat35; }

  Indicator& fareTypeCode() { return _fareTypeCode; }
  const Indicator fareTypeCode() const { return _fareTypeCode; }

  bool& cat35CSelling() { return _cat35CSelling; }
  const bool cat35CSelling() const { return _cat35CSelling; }

  MoneyAmount& totalSellingAmt() { return _totalSellingAmt; }
  const MoneyAmount totalSellingAmt() const { return _totalSellingAmt; }

  MoneyAmount& netTotalAmt() { return _netTotalAmt; }
  const MoneyAmount netTotalAmt() const { return _netTotalAmt; }

  MoneyAmount& netTotalAmtCharges() { return _netTotalAmtCharges; }
  const MoneyAmount netTotalAmtCharges() const { return _netTotalAmtCharges; }

  MoneyAmount getNetTotalBaseAmtCharges() const { return _netTotalBaseAmtCharges; }
  void setNetTotalBaseAmtCharges(MoneyAmount amt) { _netTotalBaseAmtCharges = amt; }

  MoneyAmount& totalMileageCharges() { return _mileageSurchargeTotalAmt; }
  const MoneyAmount totalMileageCharges() const { return _mileageSurchargeTotalAmt; }

  MoneyAmount& otherSurchargeTotalAmt() { return _otherSurchargeTotalAmt; }
  const MoneyAmount otherSurchargeTotalAmt() const { return _otherSurchargeTotalAmt; }

  MoneyAmount& cat8SurchargeTotalAmt() { return _cat8SurchargeTotalAmt; }
  const MoneyAmount cat8SurchargeTotalAmt() const { return _cat8SurchargeTotalAmt; }

  MoneyAmount& cat9SurchargeTotalAmt() { return _cat9SurchargeTotalAmt; }
  const MoneyAmount cat9SurchargeTotalAmt() const { return _cat9SurchargeTotalAmt; }

  MoneyAmount& cat12SurchargeTotalAmt() { return _cat12SurchargeTotalAmt; }
  const MoneyAmount cat12SurchargeTotalAmt() const { return _cat12SurchargeTotalAmt; }

  MoneyAmount& totalSellingMileageCharges() { return _mileageSurchargeTotalSellingAmt; }
  const MoneyAmount totalSellingMileageCharges() const { return _mileageSurchargeTotalSellingAmt; }

  Indicator& indNetGross() { return _indNetGross; }
  const Indicator indNetGross() const { return _indNetGross; }

  Indicator& bspMethod() { return _bspMethod; }
  const Indicator bspMethod() const { return _bspMethod; }

  Indicator& indTypeTour() { return _indTypeTour; }
  const Indicator indTypeTour() const { return _indTypeTour; }

  std::string& tourCode() { return _tourCode; }
  const std::string tourCode() const { return _tourCode; }

  std::string& fareBox() { return _fareBox; }
  const std::string fareBox() const { return _fareBox; }

  Percent& comPercent() { return _comPercent; }
  const Percent comPercent() const { return _comPercent; }

  int& noComPerDec() { return _noComPerDec; }
  const int noComPerDec() const { return _noComPerDec; }

  CurrencyCode& currency() { return _currency; }
  const CurrencyCode currency() const { return _currency; }

  int& noDec() { return _noDec; }
  const int noDec() const { return _noDec; }

  MoneyAmount& comAmount() { return _comAmount; }
  const MoneyAmount comAmount() const { return _comAmount; }

  MoneyAmount& netComAmount() { return _netComAmount; }
  const MoneyAmount netComAmount() const { return _netComAmount; }

  Percent& netComPercent() { return _netComPercent; }
  const Percent netComPercent() const { return _netComPercent; }

  TktDesignator& tDesignator() { return _tDesignator; }
  const TktDesignator tDesignator() const { return _tDesignator; }

  std::string& bagNo() { return _bagNo; }
  const std::string bagNo() const { return _bagNo; }

  std::string& trailerMsg() { return _trailerMsg; }
  const std::string trailerMsg() const { return _trailerMsg; }

  std::string& commissionMsg() { return _commissionMsg; }
  const std::string commissionMsg() const { return _commissionMsg; }

  int& numberWarningMsg() { return _numberWarningMsg; }
  const int numberWarningMsg() const { return _numberWarningMsg; }

  std::string& valueCode() { return _valueCode; }
  const std::string valueCode() const { return _valueCode; }

  bool& differentNetFare() { return _differentNetFare; }
  const bool differentNetFare() const { return _differentNetFare; }

  bool& differentCode() { return _differentCode; }
  const bool differentCode() const { return _differentCode; }

  bool& differentTktDesg() { return _differentTktDesg; }
  const bool differentTktDesg() const { return _differentTktDesg; }

  bool& differentComm() { return _differentComm; }
  const bool differentComm() const { return _differentComm; }

  bool& netRemitTicketInd() { return _netRemitTicketInd; }
  const bool netRemitTicketInd() const { return _netRemitTicketInd; }

  void setUseRedistributedWholeSale(bool b) { _shouldUseRedistributedWholeSale = b; }
  const bool shouldUseRedistributedWholeSale() const { return _shouldUseRedistributedWholeSale; }

  MoneyAmount& commissionBaseAmount() { return _commissionBaseAmount; }
  const MoneyAmount commissionBaseAmount() const { return _commissionBaseAmount; }

  MoneyAmount& markUpAmount() { return _markUpAmount; }
  const MoneyAmount markUpAmount() const { return _markUpAmount; }

private:
  MoneyAmount _totalSellingAmt;
  MoneyAmount _netTotalAmt;
  MoneyAmount _netTotalAmtCharges; // netTotal + charges
  MoneyAmount _netTotalBaseAmtCharges; // net total in equivalent currency
  MoneyAmount _mileageSurchargeTotalAmt; // total mileage surcharge from Net Amount
  MoneyAmount _otherSurchargeTotalAmt; // total Cat 8, 9 surcharges
  MoneyAmount _cat8SurchargeTotalAmt;
  MoneyAmount _cat9SurchargeTotalAmt;
  MoneyAmount _cat12SurchargeTotalAmt; // total 12 surcharges
  MoneyAmount _mileageSurchargeTotalSellingAmt; // total mileage surcharge from Selling Amount
  MoneyAmount _comAmount;
  MoneyAmount _netComAmount;
  Indicator _indNetGross; // N/G/B/blank
  Indicator _bspMethod; // 1/2/3/4/5/blank
  Indicator _fareTypeCode; // L/N/T/C
  Indicator _indTypeTour; // B/C/V/T/blank
  bool _netRemitTicketInd;
  bool _shouldUseRedistributedWholeSale = false;
  bool _indicatorCat35;
  bool _cat35CSelling; // C with 979
  std::string _tourCode;
  std::string _fareBox;
  Percent _netComPercent;
  Percent _comPercent;
  int _noComPerDec;
  int _noDec;
  CurrencyCode _currency;
  TktDesignator _tDesignator;
  std::string _bagNo; // 0-99/blank
  std::string _trailerMsg;
  std::string _commissionMsg;
  std::string _valueCode; // From Cat18  or from DAVC/static value code (NetRemit Optimus)
  int _numberWarningMsg;
  bool _differentNetFare;
  bool _differentCode;
  bool _differentTktDesg;
  bool _differentComm;
  MoneyAmount _commissionBaseAmount;
  MoneyAmount _markUpAmount;
};

inline CollectedNegFareData::CollectedNegFareData()
  : _totalSellingAmt(0),
    _netTotalAmt(0),
    _netTotalAmtCharges(0),
    _netTotalBaseAmtCharges(0),
    _mileageSurchargeTotalAmt(0),
    _otherSurchargeTotalAmt(0),
    _cat8SurchargeTotalAmt(0),
    _cat9SurchargeTotalAmt(0),
    _cat12SurchargeTotalAmt(0),
    _mileageSurchargeTotalSellingAmt(0),
    _comAmount(0),
    _netComAmount(0),
    _indNetGross(' '),
    _bspMethod(' '),
    _fareTypeCode(' '),
    _indTypeTour(' '),
    _netRemitTicketInd(false),
    _indicatorCat35(false),
    _cat35CSelling(false),
    _netComPercent(0),
    _comPercent(0),
    _noComPerDec(0),
    _noDec(0),
    _numberWarningMsg(0),
    _differentNetFare(false),
    _differentCode(false),
    _differentTktDesg(false),
    _differentComm(false),
    _commissionBaseAmount(0),
    _markUpAmount(0)
{
}
} // tse namespace
