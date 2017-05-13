//----------------------------------------------------------------------------
//  File:        ACDiagCollector.h
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//  Created:     May 28, 2005
//
//  Description: Add-on construction diag. collector
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
//----------------------------------------------------------------------------

#pragma once

#include "AddonConstruction/ConstructionDefs.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareInfo;
class AddonFareInfo;
class AddonFareCortege;
class ConstructedFareInfo;

class ConstructionJob;
class DiagRequest;
class GatewayPair;

class ACDiagCollector : public DiagCollector
{
public:
  explicit ACDiagCollector(Diagnostic& root)
    : DiagCollector(root),
      _cJob(nullptr),
      _diagRequest(nullptr),
      _restrictPrivateFareAmt(false),
      _isJointVenture(false),
      _fcRequested(false),
      _crossRefType(DOMESTIC),
      _pricingTrx(nullptr)
  {
  }

  ACDiagCollector()
    : _cJob(nullptr),
      _diagRequest(nullptr),
      _restrictPrivateFareAmt(false),
      _isJointVenture(false),
      _fcRequested(false),
      _crossRefType(DOMESTIC),
      _pricingTrx(nullptr)
  {
  }

  void initialize(ConstructionJob* cj, DiagRequest* dr);
  void initialize(const Loc& orig, const Loc& dest);

  virtual void writeConstructedFare(const ConstructedFareInfo& cfi);
  virtual void writeSpecifiedFare(const FareInfo& fareInfo);
  void writeFiresHeader();

protected:
  ConstructionJob* _cJob;
  DiagRequest* _diagRequest;
  VendorCode _currentVendor;

  void writeCommonHeader(const VendorCode& vendor, const bool forceHeader = false);
  void gwQuote(const GatewayPair& gw);
  void writeFiresFooter();

  // add-on fares
  void writeAddonFare(bool isOriginAddon,
                      const LocCode& interiorCity,
                      const LocCode& gatewayCity,
                      const DateTime& effectiveDate,
                      const DateTime& discDate,
                      const Indicator owrt,
                      const CurrencyCode& curency,
                      const MoneyAmount amount,
                      const FareClassCode& fareClass,
                      const TariffNumber tariff);

  void writeAddonFare(const AddonFareCortege& afc, bool isOriginAddon);
  void writeAddonFare(const AddonFareInfo& af, bool isOriginAddon);

  // specified & constructed fares
  void writeSpecifiedOrConstructed(bool isSpecified,
                                   const Indicator inhibit,
                                   const LocCode& city1,
                                   const LocCode& city2,
                                   const DateTime& effectiveDate,
                                   const DateTime& discDate,
                                   const Indicator owrt,
                                   const Directionality& directionality,
                                   const CurrencyCode& curency,
                                   const MoneyAmount amount,
                                   const FareClassCode& fareClass,
                                   const TariffNumber tariff,
                                   bool skipFareAmt = false);

  void writeConstructedFare(const ConstructedFare& cf);
  void formatDateTime(const DateTime& dt);

  bool showFareAmount(const TariffNumber& tariff,
                      const VendorCode& vendor,
                      const CarrierCode& fareCarrier);

  bool
  isPrivateFare(const TariffNumber& tariff, const VendorCode& vendor, const CarrierCode& carrier);

  bool _restrictPrivateFareAmt;
  std::string _publishingCarrier;
  bool _isJointVenture;
  bool _fcRequested;
  RecordScope _crossRefType;
  DateTime _travelDate;
  PricingTrx* _pricingTrx;

  static const char* SEPARATOR;
};
} // namespace tse

