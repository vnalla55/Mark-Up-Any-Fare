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
#include "Rules/NetRemitPscMatchUtil.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"


namespace tse
{
const std::string NetRemitPscMatchUtil::ANY_CITY = "***";

NetRemitPscMatchUtil::NetRemitPscMatchUtil(const PricingUnit& pricingUnit)
  : _pricingUnit(pricingUnit), _negFareRuleData(nullptr), _fu(nullptr)
{
}

NetRemitPscMatchUtil::~NetRemitPscMatchUtil() {}

bool
NetRemitPscMatchUtil::process()
{
  bool hasFailMatchTravelSegments = false;
  for (FareUsage* fu : _pricingUnit.fareUsage())
  {
    _fu = fu;
    MatchPaxTypeFareStatus status = matchPaxTypeFare(fu->paxTypeFare());

    if (status == FAIL)
      return false;

    if (status == PASS && !matchTravelSegments())
      hasFailMatchTravelSegments = true;
  }

  if (hasFailMatchTravelSegments)
  {
    Indicator fareBasisAmtInd = _negFareRuleData->negFareRestExt()->fareBasisAmtInd();
    if (fareBasisAmtInd == RuleConst::NR_VALUE_A || fareBasisAmtInd == RuleConst::NR_VALUE_F ||
        fareBasisAmtInd == RuleConst::NR_VALUE_B)
      return false;
  }
  return true;
}

NetRemitPscMatchUtil::MatchPaxTypeFareStatus
NetRemitPscMatchUtil::matchPaxTypeFare(const PaxTypeFare* ptf)
{
  NegPaxTypeFareRuleData* ruleData = nullptr;
  const NegFareRest* negFareRest = NegotiatedFareRuleUtil::getCat35Record3(ptf, ruleData);

  if (!negFareRest)
    return FAIL;

  const NegFareRestExt* negFareRestExt = ruleData->negFareRestExt();

  if (negFareRest->tktFareDataInd1() != RuleConst::BLANK || !negFareRestExt ||
      negFareRestExt->tktFareDataSegExistInd() == 'N')
  {
    return SKIP;
  }

  _negFareRuleData = ruleData;
  return PASS;
}

bool
NetRemitPscMatchUtil::matchTravelSegments()
{
  bool hasFailMatchedSeq = false;
  std::vector<const NegFareRestExtSeq*> matched;
  std::vector<const TravelSeg*> begins;
  std::vector<const TravelSeg*> ends;

  std::vector<TravelSeg*>::const_iterator startOfFc = _fu->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator i = _fu->travelSeg().begin();

  for (; i != _fu->travelSeg().end(); i++)
  {
    if (i < startOfFc)
      continue; // SKIP segments validated before by fare component

    std::vector<NegFareRestExtSeq*>::const_iterator matchedSeq = findMatchedSeq(i, startOfFc);
    if (matchedSeq == _negFareRuleData->negFareRestExtSeq().end())
    {
      hasFailMatchedSeq = true;
    }
    else
    {
      matched.push_back(*matchedSeq);
      begins.push_back(*i);
      ends.push_back(*startOfFc);
    }
    startOfFc++;
  }

  storeMatchedInFareUsage(matched, begins, ends);
  if (hasFailMatchedSeq)
    return false;
  return true;
}

std::vector<NegFareRestExtSeq*>::const_iterator
NetRemitPscMatchUtil::findMatchedSeq(std::vector<TravelSeg*>::const_iterator beginOfFc,
                                     std::vector<TravelSeg*>::const_iterator& endOfFc) const
{
  std::vector<NegFareRestExtSeq*>::const_iterator i = _negFareRuleData->negFareRestExtSeq().begin();
  for (; i != _negFareRuleData->negFareRestExtSeq().end(); i++)
  {
    if (matchTravelSeg(*beginOfFc, **i) || matchFareComp(beginOfFc, endOfFc, **i))
    {
      return i;
    }
  }

  return _negFareRuleData->negFareRestExtSeq().end();
}

bool
NetRemitPscMatchUtil::matchTravelSeg(const TravelSeg* seg, const NegFareRestExtSeq& nfrExtSeq) const
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(seg);

  return airSeg && // skip ARUNK
         nfrExtSeq.viaCity1().empty() && // if VIA city match FareComp
         matchToFrom(*seg, nfrExtSeq) && matchCarrier(nfrExtSeq.carrier(), airSeg->carrier());
}

bool
NetRemitPscMatchUtil::matchFareComp(std::vector<TravelSeg*>::const_iterator beginOfFc,
                                    std::vector<TravelSeg*>::const_iterator& endOfFc,
                                    const NegFareRestExtSeq& nfrExtSeq) const
{
  const LocCode& from = _fu->isInbound() ? nfrExtSeq.cityTo() : nfrExtSeq.cityFrom();
  const LocCode& to = _fu->isInbound() ? nfrExtSeq.cityFrom() : nfrExtSeq.cityTo();

  if (nfrExtSeq.viaCity1().empty() || // match by sector not by fare comp
      (from != ANY_CITY && from != (*beginOfFc)->boardMultiCity() &&
       from != (*beginOfFc)->origAirport()))
  {
    return false;
  }

  // look for end of Fare comp
  std::vector<TravelSeg*>::const_iterator newEnd = _fu->travelSeg().end() - 1;

  for (; newEnd > beginOfFc; newEnd--)
  {
    if ((to == ANY_CITY || to == (*newEnd)->offMultiCity() || to == (*newEnd)->destAirport()) &&
        matchVia(nfrExtSeq, beginOfFc, newEnd) &&
        matchCarrier(beginOfFc, newEnd, nfrExtSeq.carrier()))
    {
      endOfFc = newEnd;
      return true;
    }
  }

  return false;
}

bool
NetRemitPscMatchUtil::matchToFrom(const TravelSeg& seg, const NegFareRestExtSeq& nfrExtSeq) const
{
  const LocCode& from = _fu->isInbound() ? nfrExtSeq.cityTo() : nfrExtSeq.cityFrom();
  const LocCode& to = _fu->isInbound() ? nfrExtSeq.cityFrom() : nfrExtSeq.cityTo();

  return (from == ANY_CITY || from == seg.boardMultiCity() || from == seg.origAirport()) &&
         (to == ANY_CITY || to == seg.offMultiCity() || to == seg.destAirport());
}

bool
NetRemitPscMatchUtil::matchVia(const NegFareRestExtSeq& nfrExtSeq,
                               std::vector<TravelSeg*>::const_iterator start,
                               std::vector<TravelSeg*>::const_iterator end) const
{
  for (std::vector<TravelSeg*>::const_iterator i = start; i < end; i++)
  {
    if (!matchVia(nfrExtSeq.viaCity1(), **i) && !matchVia(nfrExtSeq.viaCity2(), **i) &&
        !matchVia(nfrExtSeq.viaCity3(), **i) && !matchVia(nfrExtSeq.viaCity4(), **i))
    {
      return false;
    }
  }
  return true;
}

bool
NetRemitPscMatchUtil::matchVia(const LocCode& via, const TravelSeg& seg) const
{
  return via == ANY_CITY || via == seg.offMultiCity() || via == seg.destAirport();
}

bool
NetRemitPscMatchUtil::matchCarrier(const CarrierCode& cxr, const CarrierCode& segCxr) const
{
  return cxr == ANY_CARRIER || cxr == segCxr ||
         (cxr.empty() && _fu->paxTypeFare()->carrier() == segCxr);
}

bool
NetRemitPscMatchUtil::matchCarrier(std::vector<TravelSeg*>::const_iterator start,
                                   std::vector<TravelSeg*>::const_iterator end,
                                   const CarrierCode& cxr) const
{
  const CarrierCode& matchCxr = cxr.empty() ? _fu->paxTypeFare()->carrier() : cxr;

  for (std::vector<TravelSeg*>::const_iterator i = start; i <= end; i++)
  {
    const AirSeg* seg = dynamic_cast<const AirSeg*>(*i);

    if (seg && matchCxr == seg->carrier())
    {
      return true;
    }
  }
  return false;
}

void
NetRemitPscMatchUtil::storeMatchedInFareUsage(const std::vector<const NegFareRestExtSeq*>& matched,
                                              const std::vector<const TravelSeg*>& begins,
                                              const std::vector<const TravelSeg*>& ends)
{
  std::vector<const NegFareRestExtSeq*>::const_iterator mi = matched.begin();
  std::vector<const TravelSeg*>::const_iterator bi = begins.begin();
  std::vector<const TravelSeg*>::const_iterator ei = ends.begin();

  for (; mi != matched.end() && bi != begins.end() && ei != ends.end(); mi++, bi++, ei++)
  {
    _fu->netRemitPscResults().push_back(FareUsage::TktNetRemitPscResult(*bi, *ei, *mi, nullptr));
  }
}
}
