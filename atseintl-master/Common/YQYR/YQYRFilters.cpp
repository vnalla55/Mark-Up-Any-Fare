#include "Common/YQYR/YQYRFilters.h"

#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/YQYR/YQYRTypes.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/Agent.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/YQYRFees.h"

#include <unordered_set>

#include <cassert>

namespace tse
{
namespace YQYR
{
bool
YQYRClassifier::classify(std::vector<YQYRBucket>& buckets,
                         const YQYRFees* fee,
                         DiagCollectorShopping* dc) const
{
  if (!classify(buckets, fee))
  {
    if (UNLIKELY(dc))
      dc->diagStream() << " - RECORD " << fee->seqNo() << " [" << fee->taxCode() << fee->subCode()
                       << "] FILTERED OUT BY " << getFilterName() << std::endl;
    return false;
  }

  return true;
}

void
YQYRClassifier::addToAllBuckets(std::vector<YQYRBucket>& buckets, const YQYRFees* fee)
{
  for (YQYRBucket& bucket : buckets)
    bucket.add(fee);
}

bool
YQYRFilters::append(YQYRFilter* filter)
{
  assert(_filterCount < MaxFilters);
  if (!filter->isNeeded())
    return false;

  _filters[_filterCount++] = filter;
  return true;
}

bool
YQYRFilters::prepend(YQYRFilter* filter)
{
  assert(_filterCount < MaxFilters);
  if (!filter->isNeeded())
    return false;

  std::move_backward(_filters.begin(),
                     _filters.begin() + _filterCount,
                     _filters.begin() + _filterCount + 1);
  _filters[0] = filter;
  ++_filterCount;
  return true;
}

bool
YQYRFilters::isFilteredOut(const YQYRFees* fee, DiagCollectorShopping* dc) const
{
  if (!fee)
    return true;

  for (size_t i = 0; i < _filterCount; ++i)
  {
    YQYRFilter& filter = *_filters[i];

    if (filter.isFilteredOut(fee))
    {
      if (UNLIKELY(dc))
        dc->diagStream() << " - RECORD " << fee->seqNo() << " [" << fee->taxCode() << fee->subCode()
                         << "] FILTERED OUT BY " << filter.getFilterName() << std::endl;
      return true;
    }
  }

  return false;
}

YQYRFilterReturnsToOrigin::YQYRFilterReturnsToOrigin(ShoppingTrx& trx, const bool returnsToOrigin)
  : YQYRFilter(trx, "RETURNS TO ORIGIN", Validations::RETURNS_TO_ORIGIN),
    _returnsToOrigin(returnsToOrigin)
{
}

bool
YQYRFilterReturnsToOrigin::isFilteredOut(const YQYRFees* fee) const
{
  switch (fee->returnToOrigin())
  {
  case 'Y':
    return !_returnsToOrigin;
  case 'N':
    return _returnsToOrigin;
  case ' ':
    return false;
  default:
    break;
  }
  return true;
}

YQYRFilterPointOfSale::YQYRFilterPointOfSale(ShoppingTrx& trx)
  : YQYRFilter(trx, "POINT OF SALE", Validations::POS),
    _posLoc(TrxUtil::saleLoc(trx)),
    _ticketingDate(_trx.getRequest()->ticketingDT())
{
}

bool
YQYRFilterPointOfSale::isFilteredOut(const YQYRFees* fee) const
{
  if (fee->posLoc().empty())
    return false;

  return !YQYRUtils::checkLocation(*_posLoc, fee, _ticketingDate);
}

bool
YQYRFilterPointOfSale::isNeeded() const
{
  return _posLoc;
}

YQYRFilterPointOfTicketing::YQYRFilterPointOfTicketing(ShoppingTrx& trx)
  : YQYRFilter(trx, "POINT OF TICKETING", Validations::POT),
    _ticketLoc(TrxUtil::ticketingLoc(trx)),
    _ticketingDate(_trx.getRequest()->ticketingDT())
{
}

bool
YQYRFilterPointOfTicketing::isFilteredOut(const YQYRFees* fee) const
{
  if (fee->potLoc().empty())
    return false;

  return !YQYRUtils::checkLocation(*_ticketLoc, fee, _ticketingDate);
}

bool
YQYRFilterPointOfTicketing::isNeeded() const
{
  return _ticketLoc;
}

YQYRFilterAgencyPCC::YQYRFilterAgencyPCC(ShoppingTrx& trx)
  : YQYRFilter(trx, "AGENCY PCC", Validations::AGENCY),
    _ticketingAgent(*_trx.getRequest()->ticketingAgent())
{
}

bool
YQYRFilterAgencyPCC::isFilteredOut(const YQYRFees* fee) const
{
  switch (fee->posLocaleType())
  {
  case 'T':
  {
    if ((fee->posAgencyPCC().size() == 5) && (!_ticketingAgent.officeDesignator().empty()))
    {
      return fee->posAgencyPCC() != _ticketingAgent.officeDesignator();
    }

    return fee->posAgencyPCC() != _ticketingAgent.tvlAgencyPCC();
  }
  case 'I':
  {
    const std::string& iIATA = _ticketingAgent.tvlAgencyIATA().empty()
                                   ? _ticketingAgent.airlineIATA()
                                   : _ticketingAgent.tvlAgencyIATA();
    if (iIATA.empty())
      return true;

    return fee->posIataTvlAgencyNo().compare(0, iIATA.size(), iIATA) != 0;
  }
  case ' ':
    return false;
  }

  return true;
}

YQYRFilterTicketingDate::YQYRFilterTicketingDate(ShoppingTrx& trx)
  : YQYRFilter(trx, "TICKETING DATE", Validations::TICKETING_DATE),
    _ticketingDate(_trx.getRequest()->ticketingDT())
{
}

bool
YQYRFilterTicketingDate::isFilteredOut(const YQYRFees* fee) const
{
  return (_ticketingDate < fee->firstTktDate() || _ticketingDate > fee->lastTktDate());
}

bool
YQYRFilterTicketingDate::isNeeded() const
{
  // YQYRFeesHistoricalDAO checks this in IsNotEffectiveYQHist
  return !_trx.dataHandle().isHistorical();
}

YQYRFilterFareBasisCode::YQYRFilterFareBasisCode(ShoppingTrx& trx,
                                                 const std::vector<PaxTypeFare*>& applicableFares)
  : YQYRFilter(trx, "FARE BASIS CODE")
{
  _applicableFareBasisCodes.reserve(applicableFares.size());
  for (const PaxTypeFare* fare : applicableFares)
    _applicableFareBasisCodes.push_back(fare->createFareBasis(trx));
}

bool
YQYRFilterFareBasisCode::isFilteredOut(const YQYRFees* fee) const
{
  const std::string& feeFareBasisCode(fee->fareBasis());
  if (feeFareBasisCode.empty())
    return false;

  const char patternFirstChar = feeFareBasisCode.front();
  const bool checkFirstChar(patternFirstChar != '-');

  for (const std::string& fareBasisCode : _applicableFareBasisCodes)
  {
    if (checkFirstChar && patternFirstChar != fareBasisCode.front())
      continue;

    if (YQYRUtils::matchFareBasisCode(feeFareBasisCode, fareBasisCode))
      return false;
  }

  return true;
}

YQYRFilterPassengerType::YQYRFilterPassengerType(ShoppingTrx& trx,
                                                 const std::vector<PaxTypeFare*>& applicableFares)
  : YQYRFilter(trx, "PASSENGER TYPE")
{
  std::unordered_set<PaxTypeCode> uniquePaxTypes;
  for (auto fare : applicableFares)
    uniquePaxTypes.insert(fare->actualPaxType()->paxType());

  _applicablePaxTypes.reserve(uniquePaxTypes.size());
  std::copy(uniquePaxTypes.begin(), uniquePaxTypes.end(), std::back_inserter(_applicablePaxTypes));
}

bool
YQYRFilterPassengerType::isFilteredOut(const YQYRFees* fee) const
{
  const PaxTypeCode& feePsgType(fee->psgType());
  if (feePsgType.empty() || feePsgType == " ")
    return false;

  for (const auto psgType : _applicablePaxTypes)
    if (YQYRUtils::validatePaxType(psgType, feePsgType, _trx))
      return false;

  return true;
}

YQYRFilterJourneyLocation::YQYRFilterJourneyLocation(ShoppingTrx& trx,
                                                     const Loc* journeyOrigin,
                                                     const Loc* furthestPoint)
  : YQYRFilter(trx, "JOURNEY LOCATION", Validations::JOURNEY),
    _ticketingDate(_trx.getRequest()->ticketingDT()),
    _journeyOrigin(journeyOrigin),
    _furthestPoint(furthestPoint)
{
}

bool
YQYRFilterJourneyLocation::isFilteredOut(const YQYRFees* fee) const
{
  if (fee->journeyLoc1().empty())
    return false;

  return !YQYRUtils::validateLocs(*_journeyOrigin, *_furthestPoint, fee, _ticketingDate);
}

YQYRClassifierFurthestPoint::YQYRClassifierFurthestPoint(ShoppingTrx& trx, const Loc* journeyOrigin)
  : YQYRClassifier(trx, "JOURNEY LOCATION", Validations::JOURNEY),
    _ticketingDate(_trx.getRequest()->ticketingDT()),
    _journeyOrigin(journeyOrigin)
{
}

bool
YQYRClassifierFurthestPoint::classify(std::vector<YQYRBucket>& buckets, const YQYRFees* fee) const
{
  if (fee->journeyLoc1().empty())
  {
    addToAllBuckets(buckets, fee);
    return true;
  }

  bool any = false;
  for (YQYRBucket& bucket : buckets)
  {
    const Loc* furthestPoint = bucket.getFurthestPoint();

    if (YQYRUtils::validateLocs(*_journeyOrigin, *furthestPoint, fee, _ticketingDate))
    {
      bucket.add(fee);
      any = true;
    }
  }

  return any;
}

YQYRClassifierPassengerType::YQYRClassifierPassengerType(ShoppingTrx& trx)
  : YQYRClassifier(trx, "PASSANGER TYPE", Validations::PTC)
{
}

bool
YQYRClassifierPassengerType::classify(std::vector<YQYRBucket>& buckets, const YQYRFees* fee) const
{
  const PaxTypeCode& feePsgType(fee->psgType());
  if (feePsgType.empty() || feePsgType == " ")
  {
    addToAllBuckets(buckets, fee);
    return true;
  }

  bool any = false;
  for (YQYRBucket& bucket : buckets)
  {
    const PaxTypeCode psgType = bucket.getPaxType()->paxType();

    if (YQYRUtils::validatePaxType(psgType, feePsgType, _trx))
    {
      bucket.add(fee);
      any = true;
    }
  }

  return any;
}
}
}
