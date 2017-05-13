//----------------------------------------------------------------------------
//  Copyright Sabre 2010
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"

#include <vector>

namespace tse
{
class PricingUnit;
class FareUsage;
class NegFareRestExtSeq;
class NegPaxTypeFareRuleData;
class PaxTypeFare;
class TravelSeg;

class NetRemitPscMatchUtil
{
  friend class NetRemitPscMatchUtilTest;

  enum MatchPaxTypeFareStatus
  {
    PASS,
    FAIL,
    SKIP
  };

public:
  NetRemitPscMatchUtil(const PricingUnit& pricingUnit);
  virtual ~NetRemitPscMatchUtil();

  bool process();

private:
  MatchPaxTypeFareStatus matchPaxTypeFare(const PaxTypeFare* ptf);
  bool matchTravelSegments();
  std::vector<NegFareRestExtSeq*>::const_iterator
  findMatchedSeq(std::vector<TravelSeg*>::const_iterator beginOfFc,
                 std::vector<TravelSeg*>::const_iterator& endOfFc) const;
  bool matchTravelSeg(const TravelSeg* seg, const NegFareRestExtSeq& nfrExtSeq) const;
  bool matchFareComp(std::vector<TravelSeg*>::const_iterator beginOfFc,
                     std::vector<TravelSeg*>::const_iterator& endOfFc,
                     const NegFareRestExtSeq& nfrExtSeq) const;
  bool matchToFrom(const TravelSeg& seg, const NegFareRestExtSeq& nfrExtSeq) const;
  bool matchVia(const NegFareRestExtSeq& nfrExtSeq,
                std::vector<TravelSeg*>::const_iterator start,
                std::vector<TravelSeg*>::const_iterator end) const;
  bool matchVia(const LocCode& via, const TravelSeg& seg) const;
  bool matchCarrier(const CarrierCode& cxr, const CarrierCode& segCxr) const;
  bool matchCarrier(std::vector<TravelSeg*>::const_iterator start,
                    std::vector<TravelSeg*>::const_iterator end,
                    const CarrierCode& cxr) const;
  void storeMatchedInFareUsage(const std::vector<const NegFareRestExtSeq*>& matched,
                               const std::vector<const TravelSeg*>& begins,
                               const std::vector<const TravelSeg*>& ends);
  // Data
private:
  const PricingUnit& _pricingUnit;
  const NegPaxTypeFareRuleData* _negFareRuleData;
  FareUsage* _fu;

  static const std::string ANY_CITY;
};
}

