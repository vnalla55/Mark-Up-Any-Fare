// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------


#pragma once

#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class TaxSplitDetails
{
  bool _isSet;
  bool _totShopRestEnabled;
  bool _totShopRestIncludeBaseFare;
  MoneyAmount _fareSumAmount;
  bool _useTaxableTaxSumAmount;

public:
  TaxSplitDetails() :
      _isSet(false),
      _totShopRestEnabled(false),
      _totShopRestIncludeBaseFare(true),
      _fareSumAmount(-1),
      _useTaxableTaxSumAmount(false) {}

  void
  setIsSet(bool isSet)
  {
    _isSet = isSet;
  }

  bool
  isSet() const
  {
    return _isSet;
  }

  void
  setTotShopRestEnabled(bool isTotShopRestEnabled)
  {
    _totShopRestEnabled = isTotShopRestEnabled;
  }

  bool
  isTotShopRestEnabled()
  {
    return _totShopRestEnabled;
  }

  bool
  checkShoppingRestrictions() const
  {
    return _totShopRestEnabled && !_totShopRestIncludeBaseFare;
  }

  void
  setTotShopRestIncludeBaseFare(bool isTotShopRestIncludeBaseFare)
  {
    _totShopRestIncludeBaseFare = isTotShopRestIncludeBaseFare;
  }

  bool
  isTotShopRestIncludeBaseFare() const
  {
    return _totShopRestIncludeBaseFare;
  }

  void
  setFareSumAmount(const MoneyAmount& fareSumAmount)
  {
    _fareSumAmount = fareSumAmount;
  }

  MoneyAmount
  getFareSumAmount() const
  {
    return _fareSumAmount;
  }

  void
  setUseTaxableTaxSumAmount(bool useTaxableTaxSumAmount)
  {
    _useTaxableTaxSumAmount = useTaxableTaxSumAmount;
  }

  bool
  useTaxableTaxSumAmount() const
  {
    return _useTaxableTaxSumAmount;
  }
};

}
