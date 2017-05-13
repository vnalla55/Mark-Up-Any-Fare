//-------------------------------------------------------------------
//  Copyright Sabre 2010
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
#include "ServiceFees/OptionalFeeCollector.h"


namespace tse
{
class PricingTrx;
class ServiceFeesGroup;
class TravelSeg;
class OCFees;

class AncillaryPricingFeeCollector : public OptionalFeeCollector
{
  friend class AncillaryPricingFeeCollectorTest;

public:
  AncillaryPricingFeeCollector(PricingTrx& trx);
  virtual ~AncillaryPricingFeeCollector();

  void process() override;

  AncillaryPricingFeeCollector(const AncillaryPricingFeeCollector& rhs) : OptionalFeeCollector(rhs)
  {
  }

private:
  AncillaryPricingFeeCollector& operator=(const AncillaryPricingFeeCollector& rhs);

  void
  multiThreadSliceAndDice(std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                                std::vector<TravelSeg*>::const_iterator>>& routes,
                          int unitNo) override;
  bool isAgencyActive() override;

  bool checkAllSegsConfirmed(std::vector<TravelSeg*>::const_iterator begin,
                             std::vector<TravelSeg*>::const_iterator end,
                             bool doDiag = true) const override;
  void printActiveCxrGroupHeader() const override;
  bool processCarrierMMRecords() override;
  bool validMerchCxrGroup(const CarrierCode& carrier, const ServiceGroup& srvGroup) const override;
  void processSubCodes(ServiceFeesGroup& srvFeesGrp,
                       const CarrierCode& candCarrier,
                       int unitNo,
                       bool isOneCarrier) const override;
  void updateServiceFeesGroupState(ServiceFeesGroup* sfG) override;
  void updateServiceFeesDisplayOnlyState(OCFees* ocFee) override;
  void getGroupCodesNotProcessedForTicketingDate(
      std::vector<ServiceGroup>& grValid,
      std::vector<ServiceGroup>& grNotValidForTicketDate,
      std::vector<ServiceGroup>& grNotProcessedForTicketDate) override;
  bool shouldProcessAllGroups() const override;
  UserApplCode getUserApplCode() const override;
  void addInvalidGroups(Itin& itin,
                        const std::vector<ServiceGroup>& grNA,
                        const std::vector<ServiceGroup>& grNP,
                        const std::vector<ServiceGroup>& grNV) override;
  int getNumberOfSegments(std::vector<TravelSeg*>::const_iterator firstSeg,
                          std::vector<TravelSeg*>::const_iterator endSeg) const override;

  void getAllActiveGroupCodes(std::vector<ServiceGroup>& gValid) override; // Deprecated
  void getAllActiveGroupCodes(std::set<ServiceGroup>& gValid) override;
  ServiceFeesGroup::FindSolution getProcessingMethod(ServiceFeesGroup& srvGroup) override;
  bool isAncillaryPricing() const override { return true; };
};

} // tse namespace

