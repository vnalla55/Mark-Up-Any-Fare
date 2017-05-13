//----------------------------------------------------------------------------
//  File:        Diag877Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 877 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2009
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

#include "Common/OcTypes.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/SvcFeesDiagCollector.h"

#include <map>

namespace tse
{
class BaggageTravel;
class CarrierFlightSeg;
class FarePath;
class OCFees;
class OptionalServicesInfo;
class PaxTypeFare;
class PricingTrx;
class SubCodeInfo;
class SvcFeesCurrencyInfo;
class SvcFeesCxrResultingFCLInfo;
class SvcFeesResBkgDesigInfo;

class Diag877Collector : public SvcFeesDiagCollector
{
  friend class Diag877CollectorTest;

public:
  enum ProcessingContext
  {
    PROCESSING_RBD,
    PROCESSING_T171,
    PROCESSING_OUTPUT_TICKET_DESIG,
    PROCESSING_T186,
    PROCESSING_ACCOUNT_CODES,
    PROCESSING_INPUT_TICKET_DESIG,
    PROCESSING_T170,
    PROCESSING_T183
  };

  class S7DetailsDiagBuilder
  {
  public:
    S7DetailsDiagBuilder(Diag877Collector& diag, const OptionalServicesInfo* s7)
      : _diag(diag), _dc(static_cast<DiagCollector&>(diag)), _s7(s7)
    {
    }

    void buildFullInfo(const char* headerContext, bool addAsterixIfFieldPresent);

    S7DetailsDiagBuilder& addHeader(const char* headerContext);
    S7DetailsDiagBuilder& addUpToStopoverTime();
    S7DetailsDiagBuilder& addCabin(bool allignRight = true);
    S7DetailsDiagBuilder& addRBD198(bool addAsterixIfFieldPresent, bool allignRight = true);
    S7DetailsDiagBuilder& addUpgradeCabin();
    S7DetailsDiagBuilder& addUpgradeRBD198();
    S7DetailsDiagBuilder& addT171(bool addAsterixIfFieldPresent);
    S7DetailsDiagBuilder& addT173(bool addAsterixIfFieldPresent);
    S7DetailsDiagBuilder& addRuleTariffIndicator();
    S7DetailsDiagBuilder& addRuleTariff(bool allignRight = true);
    S7DetailsDiagBuilder& addRule();
    S7DetailsDiagBuilder& add1stCommonPart();
    S7DetailsDiagBuilder& addT186(bool addAsterixIfFieldPresent);
    S7DetailsDiagBuilder& add2ndCommonPart();

  private:
    Diag877Collector& _diag;
    DiagCollector& _dc;
    const OptionalServicesInfo* _s7;
  };

  explicit Diag877Collector(Diagnostic& root) : SvcFeesDiagCollector(root) {}
  Diag877Collector() = default;

  void printS7Banner();
  void printS7PortionOfTravel(const TravelSeg& seg1,
                              const TravelSeg& seg2,
                              const FarePath& farePath,
                              const PricingTrx& trx);
  void printS7GroupCxrService_old(const FarePath& farePath,
                              const ServiceGroup group,
                              const CarrierCode cxr,
                              bool cxrInd);
  void printS7GroupCxrService(const FarePath& farePath,
                              const ServiceGroup group,
                              const CarrierCode cxr,
                              const std::string& carrierStrategyType);
  virtual void printS7CommonHeader();
  virtual void printS7OptionalFeeInfo(const OptionalServicesInfo* info,
                                      const OCFees& ocFees,
                                      const StatusS7Validation& rc,
                                      const bool markAsSelected = false);

  virtual void printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx);

  void printS7DetailInfoPadis(const OptionalServicesInfo* info,
                              const OCFees& ocFees,
                              const std::vector<SvcFeesResBkgDesigInfo*>& padisData,
                              const DateTime& travelDate);
  virtual void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) {}
  virtual void printS7OptionalServiceStatus(StatusS7Validation rc);
  virtual void
  printS7RecordValidationFooter(const OptionalServicesInfo& info, const PricingTrx& trx);

  void printS7NotFound(const SubCodeInfo& subcode);
  void printSvcFeeCurTable170Header(const int itemNo);
  void printStartStopTimeHeader(bool isErrorFromDOW);
  void printSvcFeeCurInfoDetail(const SvcFeesCurrencyInfo& svcInfo, bool status);
  void printSvcFeeCurInfo170Label(bool isNullHeader);

  void printRBDTable198Header(const int itemNo);
  void printRBDTable198Info(const BookingCode& bookingCode,
                            const LocCode& origAirport,
                            const LocCode& destAirport,
                            const SvcFeesResBkgDesigInfo* info,
                            StatusT198 status);
  void printResultingFareClassTable171Header(const int itemNo);
  void printResultingFareClassTable171Info(const PaxTypeFare& ptf,
                                           const SvcFeesCxrResultingFCLInfo& info,
                                           StatusT171 status,
                                           bool useFareCarrier = false);
  void printCarrierFlightT186Header(const int itemNo);
  void printCarrierFlightApplT186Info(const CarrierFlightSeg& info, StatusT186 status);
  void printNoCxrFligtT186Data();
  void printS7SoftMatchedFields(const OptionalServicesInfo* info, const OCFees& ocFees);
  void printS7GroupService(const FarePath& farePath, const ServiceGroup group);
  void printTaxBanner();
  Diag877Collector& operator<<(const TaxResponse& taxResponse) override;

  virtual bool shouldCollectInRequestedContext(ProcessingContext context) const;
  void formatResultingFareClassTable171Info(const SvcFeesCxrResultingFCLInfo& info,
                                            const std::string& fareBasis,
                                            const FareType& fcaFareType,
                                            const CarrierCode& carrier,
                                            bool useFareCarrier);
  CarrierCode getCarrierCode(const PaxTypeFare& ptf, bool useFareCarrier);
  std::string
  getFareBasis(bool isAncillaryRequest, const TravelSeg* ptfTravelSeg, const PaxTypeFare& ptf);

protected:
  void displayStatus(StatusS7Validation rc);
  void displayStopCnxDestInd(Indicator ind);
  void displaySourceForFareCreated(Indicator ind);
  void displayFormOfTheRefund(Indicator ind);
  void displayNotAvailNoCharge(Indicator ind);
  void displayRefundReissue(Indicator ind);
  virtual void displayFFMileageAppl(Indicator ind);
  void displaySoftMatchField(const std::string&, int& i);
  void displayVendor(const VendorCode& vendor, bool isDetailDisp = false);
  virtual void displayAmount(const OCFees& ocFees);
  bool isTaxExempted(const std::string& taxCode,
                     const bool isExemptAllTaxes,
                     const bool isExemptSpecificTaxes,
                     const std::vector<std::string>& taxIdExempted);
};
} // namespace tse

