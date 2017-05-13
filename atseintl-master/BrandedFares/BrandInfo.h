//-------------------------------------------------------------------
//
//  File:        BrandInfo.h
//  Created:     March 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class BrandFeatureItem;
class PricingTrx;
class SvcFeesFareIdInfo;

class BrandInfo : boost::noncopyable
{

public:
  BrandInfo();
  // accessors

  BrandCode& brandCode() { return _brandCode; }
  const BrandCode& brandCode() const { return _brandCode; }

  BrandNameS8& brandName() { return _brandName; }
  const BrandNameS8& brandName() const { return _brandName; }

  TierNumber& tier() { return _tier; }
  TierNumber tier() const { return _tier; }

  long long& primaryFareIdTable() { return _primaryFareIdTable; }
  long long primaryFareIdTable() const { return _primaryFareIdTable; }

  long long& secondaryFareIdTable() { return _secondaryFareIdTable; }
  long long secondaryFareIdTable() const { return _secondaryFareIdTable; }

  void setCarrierFlightItemNum(int carrierFlightItemNum) { _carrierFlightItemNum = carrierFlightItemNum; }
  int getCarrierFlightItemNum() const { return _carrierFlightItemNum; }

  std::string& brandText() { return _brandText; }
  const std::string& brandText() const { return _brandText; }

  std::vector<FareBasisCode>& includedFareBasisCode() { return _includedFareBasisCode; }
  const std::vector<FareBasisCode>& includedFareBasisCode() const { return _includedFareBasisCode; }

  std::vector<FareBasisCode>& excludedFareBasisCode() { return _excludedFareBasisCode; }
  const std::vector<FareBasisCode>& excludedFareBasisCode() const { return _excludedFareBasisCode; }

  std::vector<BookingCode>& primaryBookingCode() { return _primaryBookingCode; }
  const std::vector<BookingCode>& primaryBookingCode() const { return _primaryBookingCode; }

  std::vector<BookingCode>& secondaryBookingCode() { return _secondaryBookingCode; }
  const std::vector<BookingCode>& secondaryBookingCode() const { return _secondaryBookingCode; }

  // placeholder for the future T189 DAO development
  std::vector<SvcFeesFareIdInfo*>& fareIDdataPrimaryT189() { return _fareIDdataPrimaryT189; }
  const std::vector<SvcFeesFareIdInfo*>& fareIDdataPrimaryT189() const
  {
    return _fareIDdataPrimaryT189;
  }

  std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189() { return _fareIDdataSecondaryT189; }
  const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189() const
  {
    return _fareIDdataSecondaryT189;
  }

  bool collectPrimaryFareIDdata(PricingTrx& trx, long long fareIDdataPrimaryT189, VendorCode vc);
  bool
  collectSecondaryFareIDdata(PricingTrx& trx, long long fareIDdataSecondaryT189, VendorCode vc);

  std::vector<BrandFeatureItem*>& getMutableFeatureItems() { return _featureItems; }
  const std::vector<BrandFeatureItem*>& getFeatureItems() const { return _featureItems; }

private:
  BrandCode _brandCode;
  BrandNameS8 _brandName;
  TierNumber _tier;
  long long _primaryFareIdTable; // T189
  long long _secondaryFareIdTable; // if present, suppose to be used by Shopping only
  int _carrierFlightItemNum;

  // placeholder for the future T189 DAO development
  std::vector<SvcFeesFareIdInfo*> _fareIDdataPrimaryT189; // vector of rows retrieved for primary
                                                          // T189
  std::vector<SvcFeesFareIdInfo*> _fareIDdataSecondaryT189; // vector of rows retrieved for primary
                                                            // T189
  std::string _brandText;
  std::vector<FareBasisCode> _includedFareBasisCode;
  std::vector<FareBasisCode> _excludedFareBasisCode;
  std::vector<BookingCode> _primaryBookingCode;
  std::vector<BookingCode> _secondaryBookingCode;

  std::vector<BrandFeatureItem*> _featureItems; // T166
};

} // tse

