//-------------------------------------------------------------------
//
//  File:        NonPublishedValues.h
//  Created:     October 6, 2005
//  Authors:     Doug Batchelor
//
//  Description: Addon items for constructed fares
//
//  Updates:
//          02/15/05 - Doug Batchelor - file created.
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingOptions.h"

namespace tse
{

class NonPublishedValues : public PricingOptions
{
private:
  NonPublishedValues(const NonPublishedValues&) = delete;
  NonPublishedValues& operator=(const NonPublishedValues&) = delete;

  // Non-published fare items

  VendorCode _vendorCode; // S37
  uint32_t _itemNo = 0; // Q41
  DateTime _createDate; // D12
  std::string _directionality; // S70
  bool _isNonPublishedFare = false;

public:
  NonPublishedValues() = default;

  bool& isNonPublishedFare() { return _isNonPublishedFare; }
  const bool& isNonPublishedFare() const { return _isNonPublishedFare; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

  uint32_t& itemNo() { return _itemNo; }
  const uint32_t& itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  std::string& directionality() { return _directionality; }
  const std::string& directionality() const { return _directionality; }
};
} // tse namespace
