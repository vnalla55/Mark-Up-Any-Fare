//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/AncillaryPricingResponseFormatter.h"

namespace tse
{
class AncRequest;
class PaxOCFees;

class AncillaryPricingResponseFormatterPostTkt : public AncillaryPricingResponseFormatter
{
  friend class AncillaryPricingResponseFormatterPostTktTest;

  class FindPointByPnrOrder
  {
    const TravelSeg* _tvl;

  public:
    FindPointByPnrOrder(const TravelSeg* tvl) : _tvl(tvl) {}
    bool operator()(const TravelSeg* seg) const { return _tvl->pnrSegment() == seg->pnrSegment(); }
  };

public:
  void formatAEFeesResponse(XMLConstruct& construct, AncillaryPricingTrx& ancTrx) override;

protected:
  bool collectFeesForAllItinsR7(std::vector<ServiceFeesGroup*>& ocFeesGroups,
                                AncillaryPricingTrx& ancTrx,
                                const std::vector<ServiceGroup>& groupCodes,
                                const std::vector<PaxType*>& allItinPaxTypes,
                                bool timeOutMax,
                                uint16_t maxNumberOfOCFees,
                                uint16_t& feesCount,
                                bool& maxNumOfFeesReached,
                                std::map<const Itin*, uint16_t>& dispOnlyFeesCount,
                                std::map<const Itin*, uint16_t>& dfeesCount);
  bool getOCFeesForR7(AncillaryPricingTrx& ancTrx, const bool timeoutMax = false);
  bool collectFeesForAllItinsR7U(std::vector<ServiceFeesGroup*>& ocFeesGroups,
                                 AncillaryPricingTrx& ancTrx,
                                 const std::vector<ServiceGroup>& groupCodes,
                                 const std::vector<PaxType*>& allItinPaxTypes,
                                 bool timeOutMax,
                                 uint16_t maxNumberOfOCFees,
                                 uint16_t& feesCount,
                                 bool& maxNumOfFeesReached,
                                 std::map<const Itin*, uint16_t>& dispOnlyFeesCount,
                                 std::map<const Itin*, uint16_t>& dfeesCount);
  bool getOCFeesForR7U(AncillaryPricingTrx& ancTrx, const bool timeoutMax = false);
  void builPNMData(XMLConstruct& construct) override;
  void
  formatOCFeesForR7(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const bool a = false) override;
  std::string getPaxType(const PaxOCFees& paxOcFees) override;
  std::string getPaxType(const PaxOCFeesUsages& paxOcFees) override;
  void buildGroupHeader(const ServiceFeesGroup* sfg, XMLConstruct& construct) override;
  void formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                             XMLConstruct& construct,
                             const PaxOCFees& paxOcFees,
                             const uint16_t index,
                             const char& indicator) override;
  void formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                             XMLConstruct& construct,
                             const PaxOCFeesUsages& paxOcFees,
                             const uint16_t index,
                             const char& indicator) override;
  bool ancillariesNonGuarantee() override;
  void buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                 XMLConstruct& construct,
                                 const PaxOCFees& paxOcFees) override;
  void buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                 XMLConstruct& construct,
                                 const PaxOCFeesUsages& paxOcFees) override;

  bool preformattedMsgEmpty() override { return _preformattedMsg.getXMLData().empty(); }

  void checkRequestedGroup(AncillaryPricingTrx& ancTrx, const ServiceGroup& sg);

  void createOcgSection(XMLConstruct& construct, std::vector<XMLConstruct*>& ocgParts);

  bool nothingShouldBeBack(AncillaryPricingTrx& ancTrx, bool& timeOutMax);

  void timeOutTrailersForSelectedGroup(AncillaryPricingTrx& ancTrx, XMLConstruct& construct);
  bool anyFeeForRequestedGroup(AncillaryPricingTrx& ancTrx,
                               const Itin* itin,
                               const ServiceGroup& sg,
                               ServiceFeesGroup* sfg);

private:
  void formatAeFeesResponseForGroup(ServiceFeesGroup* sfg,
                                    AncillaryPricingTrx& ancTrx,
                                    AncRequest* req,
                                    uint16_t& groupIndex);
  // placeholder for grouped MSGs
  XMLConstruct _preformattedMsg;
  // list of processed Fee group
  std::vector<ServiceGroup> _processedGroups;
  // map if itins per itin index
  struct PostTktResonseItinGrp
  {
    Itin* _itin = nullptr;
    Itin* _realItin = nullptr;
    XMLConstruct _xmlConstruct;
  };
  std::map<uint16_t, PostTktResonseItinGrp*> _itinXmls;
  // current itin xml construct data
  PostTktResonseItinGrp* _curXmlConstructData = nullptr;
  // pax type list used to create PNM section
  std::vector<const PaxType*> _PnmPaxTypes;
  // precreated full list of fees
  std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxR7OCFees>>> _groupFeesVector;
  // precreated full list of fees
  std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxR7OCFeesUsages>>> _groupFeesVectorU;
  // is fess for itin only for display
  std::map<const Itin*, bool> _allFeesDisplayOnly;
  // if fees are merged, then is any "ALL" returned?
  bool _isAnyAll = false;
  // real pax types per itin
  std::map<const Itin*, std::set<const PaxType*> > _paxTypes;
  // pre created map of is ancillary guaranted
  std::map<const Itin*, bool> _ancillariesNonGuaranteeMap;
  // currecnt pax Type
  const PaxType* _realPaxType = nullptr;
  bool _agcTagOpened = false;
  bool _trunkResp = false;
  bool _allRequestedProcessed = false;
  bool _useAllForSingelItin = false;
};
}
