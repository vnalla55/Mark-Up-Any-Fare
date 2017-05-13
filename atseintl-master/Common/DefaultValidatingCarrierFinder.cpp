#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CountrySettlementPlanInfo.h"

namespace tse
{
FALLBACK_DECL(fallbackPreferredVC)

namespace
{
  struct IsSurfaceOrIndustrySegment
  {
    bool operator()(const TravelSeg* seg) const
    {
      return (seg->isAir() ? static_cast<const AirSeg&>(*seg).carrier() == INDUSTRY_CARRIER : true);
    }
  };

  template <typename T, const T& (Loc::*getter)() const>
  struct HasDifferent
  {
    bool operator()(const AirSeg* seg) const
    {
      return ((seg->origin()->*getter)() != (seg->destination()->*getter)());
    }
  };

  typedef HasDifferent<IATAAreaCode, &Loc::area> HasDifferentAreas;
  typedef HasDifferent<IATASubAreaCode, &Loc::subarea> HasDifferentSubAreas;
  typedef HasDifferent<NationCode, &Loc::nation> HasDifferentNations;

  struct IsInternational
  {
    bool operator()(const AirSeg* seg) const
    {
      if (LocUtil::isScandinavia(*seg->origin()) && LocUtil::isScandinavia(*seg->destination()))
        return false;

      return LocUtil::isInternational(*seg->origin(), *seg->destination());
    }
  };

  bool fromToArea(const AirSeg& seg, const IATAAreaCode& from, const IATAAreaCode& to)
  {
    return (seg.origin()->area() == from) && (seg.destination()->area() == to);
  }

  template <typename P>
  inline bool
  findCarrier(const std::vector<const AirSeg*>& segments, CarrierCode& carrier)
  {
    const std::vector<const AirSeg*>::const_iterator segment =
      std::find_if(segments.begin(), segments.end(), P());

    if (segment == segments.end())
      return false;

    carrier = (**segment).carrier();
    return true;
  }
} //namespace

typedef HasDifferent<IATAAreaCode, &Loc::area> HasDifferentAreas;
typedef HasDifferent<IATASubAreaCode, &Loc::subarea> HasDifferentSubAreas;
typedef HasDifferent<NationCode, &Loc::nation> HasDifferentNations;

DefaultValidatingCarrierFinder::DefaultValidatingCarrierFinder(
    PricingTrx& trx,
    Itin& itin,
    SettlementPlanType sp)
  : _trx(trx), _itin(itin), _sp(sp)
{
  if (_sp.empty())
  {
    if (itin.spValidatingCxrGsaDataMap() &&
        !itin.spValidatingCxrGsaDataMap()->empty())
      _sp = ValidatingCxrUtil::getSettlementPlanFromHierarchy(*itin.spValidatingCxrGsaDataMap());
    else
    {
      const std::vector<CountrySettlementPlanInfo*>& countrySettlementPlanInfos =
          _trx.isLockingNeededInShoppingPQ() ?
            _trx.getCopyOfCountrySettlementPlanInfos() :
            _trx.countrySettlementPlanInfos();

      if (!countrySettlementPlanInfos.empty())
      {
        CountrySettlementPlanInfo* cspi =
          ValidatingCxrUtil::determinePlanFromHierarchy(countrySettlementPlanInfos);
        if (cspi)
          _sp = cspi->getSettlementPlanTypeCode();
      }
    }
  }
}

bool
DefaultValidatingCarrierFinder::findDefValCxrFromPreferred(const std::vector<CarrierCode>& valCxrs,
                                          const std::vector<CarrierCode>& prefValCxrs,
                                          CarrierCode& defValCxr) const
{
  bool retVal = false;
  for(const auto& tempValCxr : prefValCxrs)
  {
    if(std::find(valCxrs.begin(), valCxrs.end(), tempValCxr) != valCxrs.end())
    {
      defValCxr = tempValCxr;
      retVal = true;
      break;
    }
  }
  return retVal;
}

bool
DefaultValidatingCarrierFinder::determineDefaultValidatingCarrier(const std::vector<CarrierCode>& valCxrs,
                                                                  CarrierCode& defValCxr,
                                                                  CarrierCode& defMktCxr) const
{
  if(!fallback::fallbackPreferredVC(&_trx)
       && !_trx.getRequest()->preferredVCs().empty()
       && findDefValCxrFromPreferred(valCxrs, _trx.getRequest()->preferredVCs(), defValCxr))
  {
    std::vector<CarrierCode> marketingCxrs;
    std::vector<CarrierCode> validatingCxrs;

    validatingCxrs.push_back(defValCxr);
    getMarketingCxrsForValidatingCxrs(validatingCxrs, marketingCxrs);
    if (!marketingCxrs.empty())
    {
      defMktCxr = marketingCxrs.front();
    }
    return true;
  }

  if(!_itin.isValidatingCxrGsaDataForMultiSp() || _sp.empty())
    return false;

  if (_itin.hasNeutralValidatingCarrier(_sp))
    return checkNeutralValCxr(valCxrs, defValCxr);

  if (isNoGsaSwapWithSingleValidatingCxr(valCxrs))
  {
    defValCxr = valCxrs[0];
    return true;
  }

  // We have more than one validatingCxrs. They all may be
  // 1. From Itin (no swaps)
  // 2. Or one or more may be swapped. We need to find corresponding
  // marketing carriers for swapped carriers to use IATA logic for
  // default validating carrier.
  std::vector<CarrierCode> marketingCxrs;
  getMarketingCxrsForValidatingCxrs(valCxrs, marketingCxrs);
  if (marketingCxrs.empty())
    return false;

  CarrierCode marketingCxr;
  if (1 == marketingCxrs.size())
  {
    marketingCxr = marketingCxrs.front();
    return setDefaultCarriers(valCxrs, marketingCxr, defValCxr, defMktCxr);
  }

  return findDefaultBasedOnIATA(marketingCxr,
                                defValCxr,
                                defMktCxr,
                                valCxrs,
                                marketingCxrs);
}

bool
DefaultValidatingCarrierFinder::checkNeutralValCxr(const std::vector<CarrierCode>& valCxrs,
                                                   CarrierCode& defValCxr) const
{
  bool res = true;
  if(1 == valCxrs.size())
    defValCxr = valCxrs[0];
  else
    res = false;
  return res;
}

// Finding marketing carriers that is either part of validatingCxr list or
// a validatingCxr is in marketingCxr swap list.
void
DefaultValidatingCarrierFinder::getMarketingCxrsForValidatingCxrs(const std::vector<CarrierCode>& valCxrs,
                                                                  std::vector<CarrierCode>& marketingCxrs) const
{
  std::vector<CarrierCode> itinMktCxrs, participatingCxrs;
  if (isValidatingCxrOverride())
  {
    itinMktCxrs.push_back(_trx.getRequest()->validatingCarrier());
    ValidatingCxrUtil::getParticipatingCxrs(_trx, _itin, participatingCxrs);
  }
  else
    ValidatingCxrUtil::getAllItinCarriers(_trx, _itin, itinMktCxrs, participatingCxrs);

  // Check whether itinMktCxrs exists in validatingCxr list
  std::set<CarrierCode> matchedItinMktCxrs;
  for (const CarrierCode& mCxr : itinMktCxrs)
  {
    if (std::find(valCxrs.begin(), valCxrs.end(), mCxr) != valCxrs.end())
      matchedItinMktCxrs.insert(mCxr); // marketing carrier already a VC
    else
    {
      // Check whether valCxr is in swap list of mCxr
      std::set<CarrierCode> swapCarriers;
      if (_itin.getSwapCarriers(mCxr, swapCarriers, _sp))
      {
        for (const CarrierCode& vCxr : valCxrs)
        {
          if (swapCarriers.find(vCxr) != swapCarriers.end())
          {
            matchedItinMktCxrs.insert(mCxr);
            break;
          }
        }
      }
    }
  }
  marketingCxrs.insert(marketingCxrs.end(), matchedItinMktCxrs.begin(), matchedItinMktCxrs.end());
}

// A mkt cxr can be either val-cxr or has one or multiple GSAs
void
DefaultValidatingCarrierFinder::getValidatingCxrsForMarketingCxr(const CarrierCode& mktCxr,
                                                                 std::set<CarrierCode>& valCxrs) const
{
  auto it = _itin.spValidatingCxrGsaDataMap()->find(_sp);
  if (it != _itin.spValidatingCxrGsaDataMap()->end() &&
      it->second->hasCarrier(mktCxr))
    valCxrs.insert(mktCxr);
  else
    _itin.getSwapCarriers(mktCxr, valCxrs, _sp);
}

bool
DefaultValidatingCarrierFinder::setDefaultCarriers(const std::vector<CarrierCode>& valCxrs,
                                                   CarrierCode& marketingCxr,
                                                   CarrierCode& defValCxr,
                                                   CarrierCode& defMktCxr) const
{
  std::set<CarrierCode> validatingCxrSet;
  getValidatingCxrsForMarketingCxr(marketingCxr, validatingCxrSet);

  for(const CarrierCode& vCxr : validatingCxrSet)
  {
    if (std::find(valCxrs.begin(), valCxrs.end(), vCxr) == valCxrs.end())
      validatingCxrSet.erase(vCxr);
  }

  if (validatingCxrSet.size() == 1)
  {
    defValCxr = *validatingCxrSet.begin();
    defMktCxr = marketingCxr;
    return true;
  }
  return false;
}

// This function goes through the air segments in an itin
// Check the segment is in the list of marketing carrier
// return only those segments that matches previous condition
void
DefaultValidatingCarrierFinder::getSegsFromCarrier(std::vector<CarrierCode>& marketingCxrs,
                                                   AirSegmentVec& segments) const
{
  for(TravelSeg * tSeg : _itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tSeg);
    if (airSeg)
    {
      if (std::find(marketingCxrs.begin(), marketingCxrs.end(),
                    airSeg->carrier()) != marketingCxrs.end())
        segments.push_back(airSeg);
    }
  }
}

bool
DefaultValidatingCarrierFinder::findDefaultBasedOnIATA(
    CarrierCode& marketingCxr,
    CarrierCode& defValCxr,
    CarrierCode& defMktCxr,
    const std::vector<CarrierCode>& valCxrs,
    std::vector<CarrierCode>& marketingCxrs) const
{
  std::vector<const AirSeg*> segments;
  getSegsFromCarrier(marketingCxrs, segments);

  do
  {
    marketingCxr = findDefaultValidatingCxr(segments);
    if(setDefaultCarriers(valCxrs, marketingCxr, defValCxr, defMktCxr))
      return true;
    else
    {
      std::vector<const AirSeg*>::iterator segIt = segments.begin();
      while(segIt != segments.end())
      {
        if((**segIt).carrier() == marketingCxr)
          segIt = segments.erase(segIt);
        else
          ++segIt;
      }
      marketingCxrs.erase(std::remove(marketingCxrs.begin(),
                                      marketingCxrs.end(),
                                      marketingCxr), marketingCxrs.end());
    }
  } while(!(marketingCxrs.empty() || segments.empty()));
  return false;
}


CarrierCode
DefaultValidatingCarrierFinder::findDefaultValidatingCxr(const AirSegmentVec& segments) const
{
  if (segments.empty())
    return CarrierCode();

  CarrierCode carrier;

  if (findCarrierBetweenAreas(segments, carrier) || findCarrierBetweenSubAreas(segments, carrier) ||
      findCarrierBetweenCountries(segments, carrier))
    return carrier;

  return segments.front()->carrier();
}

bool
DefaultValidatingCarrierFinder::findCarrierBetweenAreas(const AirSegmentVec& segments,
                                                  CarrierCode& carrier) const
{
  AirSegmentVec::const_iterator segment =
    std::find_if(segments.begin(), segments.end(), HasDifferentAreas());

  if (segment == segments.end())
    return false;

  if (fromToArea(**segment, IATA_AREA3, IATA_AREA2))
  {
    AirSegmentVec::const_iterator nextSegment =
      std::find_if(segment + 1, segments.end(), HasDifferentAreas());

    if ((nextSegment != segments.end()) && fromToArea(**nextSegment, IATA_AREA2, IATA_AREA1))
      segment = nextSegment;
  }

  carrier = (**segment).carrier();
  return true;
}

bool
DefaultValidatingCarrierFinder::findCarrierBetweenCountries(const AirSegmentVec& segments,
                                                      CarrierCode& carrier) const
{
  if (segments.empty())
    return false;

  return (findCarrier<IsInternational>(segments, carrier) ||
      (LocUtil::isScandinavia(*segments.front()->origin()) &&
       findCarrier<HasDifferentNations>(segments, carrier)));
}

bool
DefaultValidatingCarrierFinder::findCarrierBetweenSubAreas(const AirSegmentVec& segments,
                                           CarrierCode& carrier) const
{
  return findCarrier<HasDifferentSubAreas>(segments, carrier);
}
}
