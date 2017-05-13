//-------------------------------------------------------------------
//
//  File:     SellingFareData.h
//  Author:   Iswarya Arunan
//  Date:     Aug 2016
//
//  Description: Selling Fare Data object. (SFD)
//
//  Copyright Sabre 2016
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "DataModel/TaxDetail.h"

namespace tse
{
class TaxDetail;
class TaxBreakdown;

class SellingFareData
{
  public:

  SellingFareData();

  std::string& layerTypeName() { return _layerTypeName; }
  const std::string& layerTypeName() const { return _layerTypeName; }

  MoneyAmount& baseFareAmount() { return _baseFareAmount; }
  const MoneyAmount& baseFareAmount() const { return _baseFareAmount; }

  MoneyAmount& constructedTotalAmount() { return _constructedTotalAmount; }
  const MoneyAmount& constructedTotalAmount() const { return _constructedTotalAmount; }

  MoneyAmount& equivalentAmount() { return _equivalentAmount; }
  const MoneyAmount& equivalentAmount() const { return _equivalentAmount; }

  MoneyAmount& totalTaxes() { return _totalTaxes; }
  const MoneyAmount& totalTaxes() const { return _totalTaxes; }

  MoneyAmount& totalPerPassenger() { return _totalPerPassenger; }
  const MoneyAmount& totalPerPassenger() const { return _totalPerPassenger; }

  std::string& fareCalculation() { return _fareCalcLine; }
  const std::string& fareCalculation() const { return _fareCalcLine; }

  std::vector<TaxDetail*>& taxDetails() { return _taxDetails; }
  const std::vector<TaxDetail*>& taxDetails() const { return _taxDetails; }

  std::vector<TaxBreakdown*>& taxBreakdowns() { return _taxBreakdowns; }
  const std::vector<TaxBreakdown*>& taxBreakdowns() const { return _taxBreakdowns; }

  private:

  std::string _layerTypeName; // TYP
  MoneyAmount _baseFareAmount = 0; // C5A
  MoneyAmount _constructedTotalAmount = 0; // C5E
  MoneyAmount _equivalentAmount = 0; // C5F
  MoneyAmount _totalTaxes = 0; // C65
  MoneyAmount _totalPerPassenger = 0; // C66
  std::string _fareCalcLine; // S66

  std::vector<TaxDetail*> _taxDetails; // TAX
  std::vector<TaxBreakdown*> _taxBreakdowns; // TBD
};

inline SellingFareData::SellingFareData()
{}

}

