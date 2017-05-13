//----------------------------------------------------------------------------
//  File:        Diag888Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 888 - S8 Branded Fares programs, fares, services
//
//  Updates:
//          date - initials - description.
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/SvcFeesDiagCollector.h"

#include <map>

namespace tse
{
class BrandedFare;
class CarrierFlightSeg;
class FareMarket;
class PricingTrx;
class SvcFeesAccCodeInfo;
class SvcFeesFeatureInfo;

class Diag888Collector : public SvcFeesDiagCollector
{
  friend class Diag888CollectorTest;

public:
  explicit Diag888Collector(Diagnostic& root) : SvcFeesDiagCollector(root) {}
  Diag888Collector() = default;

  void printS8Banner();
  virtual void printS8CommonHeader();
  virtual void printS8BrandedFaresContent(const BrandedFare* info);
  virtual void printS8DetailContent(PricingTrx& trx, const BrandedFare* info);
  void printS8NotFound(const VendorCode& vendor, const CarrierCode& cxrCode);
  void printS8NotProcessed();

  //  Next three methods are reserved for the future T186 development.
  //  this table is not defined in the S8 yet

  //    void printCarrierFlightT186Header(const int itemNo);
  //    void printCarrierFlightApplT186Info(const CarrierFlightSeg& info, StatusT186 status);
  //    void printNoCxrFligtT186Data();
  void printS8FareMarket(const FareMarket& market);

private:
  void displayVendor(const VendorCode& vendor, bool isDetailDisp = false);
  bool isStartDateSpecified(const BrandedFare& S8Info);
  bool isStopDateSpecified(const BrandedFare& S8Info);

  virtual const std::vector<SvcFeesAccCodeInfo*>&
  getSvcFeesAccCodeInfo(PricingTrx& trx, int itemNo) const;
  virtual const std::vector<SvcFeesSecurityInfo*>&
  getSecurityInfo(PricingTrx& trx, VendorCode vc, int t183ItemNo);
  virtual const std::vector<SvcFeesFareIdInfo*>&
  getFareIdInfo(PricingTrx& trx, VendorCode vc, long long itemNo);

  virtual const std::vector<SvcFeesFeatureInfo*>&
  getT166Info(PricingTrx& trx, VendorCode vc, long long itemNo);
};
} // namespace tse

