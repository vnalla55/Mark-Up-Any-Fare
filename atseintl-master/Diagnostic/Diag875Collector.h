//----------------------------------------------------------------------------
//  File:        Diag875Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 875 formatter
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ItinHelperStructs.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class AncRequest;
class FarePath;
class MerchCarrierPreferenceInfo;
class PricingTrx;
class SubCodeInfo;

class Diag875Collector : public DiagCollector
{
  friend class Diag875CollectorTest;

public:
  explicit Diag875Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag875Collector() = default;

  void printNoGroupCodeProvided();
  void printNoOptionsRequested();
  void printGroupCodesInTheRequest(PricingTrx& trx,
                                   const std::vector<ServiceGroup>& input,
                                   const std::vector<ServiceGroup>& vecInv,
                                   const std::vector<ServiceGroup>& vecInvTkt,
                                   ItinBoolMap& roundTrip,
                                   ItinBoolMap& international,
                                   bool noGroupCode) const;

  void printPccIsDeactivated(const PseudoCityCode& pcc) const;
  void printS5NotFound_old(const ServiceGroup group, const CarrierCode cxr, bool cxrInd) const;
  void printS5NotFound(const ServiceGroup group, const CarrierCode cxr, char carrierStrategyType) const;
  void printS5SubCodeInfo_old(const SubCodeInfo* info, bool cxrInd, const StatusS5Validation& rc);
  void printS5SubCodeInfo(const SubCodeInfo* info, char carrierStrategyType, const StatusS5Validation& rc);
  void printTravelPortionHeader(TravelSeg& seg1,
                                TravelSeg& seg2,
                                std::set<tse::CarrierCode> marketingCxr,
                                std::set<tse::CarrierCode> operatingCxr,
                                const FarePath& farePath,
                                const PricingTrx& trx) const;
  void printS5CommonHeader();
  void printJourneyDestination(const LocCode& point) const;

  void printCanNotCollect(const StatusS5Validation rc);
  void printDetailS5Info_old(const SubCodeInfo* info, bool cxrInd);
  void printDetailS5Info(const SubCodeInfo* info, char carrierStrategyType);
  void printS5SubCodeStatus(StatusS5Validation rc);


  void printDetailInterlineEmdProcessingS5Info(const tse::NationCode& nation,
                                               const tse::CrsCode& gds,
                                               const tse::CarrierCode& validatingCarrier,
                                               const std::set<tse::CarrierCode>& marketingCarriers,
                                               const std::set<tse::CarrierCode>& operatingCarriers);
  void printDetailInterlineEmdProcessingStatusS5Info(bool isValidationPassed,
                                                     const std::set<tse::CarrierCode>& failedCarriers
                                                     = std::set<tse::CarrierCode>());
  void printNoInterlineDataFoundInfo();
  void printDetailInterlineEmdAgreementInfo(const std::vector<EmdInterlineAgreementInfo*>& eiaList) const;

  void displayVendor(const VendorCode& vendor, bool ind = false);
  void displayActiveCxrGroupHeader();
  void displayMrktDrivenCxrHeader();
  void displayMrktDrivenCxrData(std::vector<MerchCarrierPreferenceInfo*>& mCxrPrefVec);
  void displayMrktDrivenCxrNoData();
  void displayCxrNoMAdata(const CarrierCode cxr);
  void displayActiveCxrGroup(const CarrierCode cxr,
                             bool active,
                             const std::string& group = EMPTY_STRING());
  void displayNoActiveCxrGroupForOC();
  void displayTicketDetail(const PricingTrx* ptrx);
  void displayTicketDetail(const AncRequest* req,
                           const Itin* itin,
                           const std::string& segStr,
                           Indicator TktInd);

  void displayGroupDescriptionEmpty(ServiceGroup sg) const;

private:
  void displayStatus(StatusS5Validation rc);
  virtual void displayConcurInd(Indicator ind);
  virtual void displayBookingInd(ServiceBookingInd ind);
  std::string getCarrierListAsString(const std::set<tse::CarrierCode>& carriers,
                                     const std::string& separator) const;
};
} // namespace tse

