// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/LocationUtil.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/SectorDetail.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxOnFareApplicator.h"
#include "Rules/TaxOnFareRule.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/RepricingService.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

#include <boost/optional.hpp>

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
namespace
{

static const type::TaxProcessingApplTag taxProcessingApplTag03_fareProration{"03"};
static const type::TaxProcessingApplTag taxProcessingApplTag06_likeFareForTaxKH{"06"};

static const std::string AD{"/AD"};
static const std::string ID("/ID");

struct FareComponent
{
  type::Index fareId;
  type::Index beginId;
  type::Index endId;
  std::vector<type::Index> flightIds;
  const std::vector<Map>* maps;
  bool fullyInRange;
};

boost::ptr_vector<FareComponent>
getFareComponentsBetween(type::Index begin,
                         type::Index end,
                         FarePath const& farePath,
                         GeoPathMapping const& geoPathMapping)
{
  boost::ptr_vector<FareComponent> fareComponents;
  type::Index flightIdOffset = 0;
  for (type::Index fareUsageId = 0; fareUsageId < farePath.fareUsages().size(); ++fareUsageId)
  {
    const std::vector<Map>& maps = geoPathMapping.mappings()[fareUsageId].maps();
    type::Index fareBegin = maps.front().index();
    type::Index fareEnd = maps.back().index();

    if (end < fareBegin || begin > fareEnd)
      continue;

    fareComponents.push_back(new FareComponent);
    fareComponents.back().fareId = farePath.fareUsages()[fareUsageId].index();
    fareComponents.back().beginId = fareBegin;
    fareComponents.back().endId = fareEnd;
    fareComponents.back().maps = &maps;
    fareComponents.back().fullyInRange = begin <= fareBegin && end >= fareEnd;
    type::Index flightsInComponent = maps.size() / 2;
    for (type::Index flightId = flightIdOffset; flightId < flightIdOffset + flightsInComponent;
         ++flightId)
    {
      fareComponents.back().flightIds.push_back(flightId);
    }
    flightIdOffset += flightsInComponent;
  }

  return fareComponents;
}

bool isLoc1ToLoc2Domestic(GeoPath const& geoPath,
                          type::Index loc1,
                          type::Index loc2)
{
  const auto& loc1Nation = geoPath.geos()[loc1].getNation();
  for (auto id = loc1 + 1 ; id <= loc2 ; ++id)
    if (!LocationUtil::isDomestic(loc1Nation, geoPath.geos()[id].getNation()))
      return false;

  return true;
}

Range
getPaymentRange(const PaymentDetail& paymentDetail)
{
  const Geo& taxPointBegin = paymentDetail.getTaxPointBegin();
  const Geo& taxPointEnd = paymentDetail.getTaxPointEnd();

  return ProperRange(taxPointBegin.id(), taxPointEnd.id());
}

bool
isInternational(FareComponent const& fareComponent, GeoPath const& geoPath)
{
  for (type::Index i = fareComponent.beginId + 1; i <= fareComponent.endId; ++i)
  {
    if (LocationUtil::isInternational(geoPath.geos()[fareComponent.beginId].getNation(),
                                      geoPath.geos()[i].getNation()))
      return true;
  }
  return false;
}

bool hasInternationalFareComponent(type::Index begin,
                                   type::Index end,
                                   FarePath const& farePath,
                                   GeoPath const& geoPath,
                                   GeoPathMapping const& geoPathMapping)
{
  auto fareComponents = getFareComponentsBetween(begin, end, farePath, geoPathMapping);
  for (const auto& fareComponent : fareComponents)
  {
    if (isInternational(fareComponent, geoPath))
      return true;
  }

  return false;
}
}

TaxOnFareApplicator::TaxOnFareApplicator(TaxOnFareRule const& parent,
                                         Services const& services,
                                         std::shared_ptr<SectorDetail const> sectorDetail,
                                         std::vector<Fare> const& fares,
                                         FarePath const& farePath,
                                         std::vector<Flight> const& flights,
                                         std::vector<FlightUsage> const& flightUsages,
                                         GeoPath const& geoPath,
                                         GeoPathMapping const& geoPathMapping,
                                         type::Index const& itinId,
                                         bool useRepricing,
                                         bool isRtw,
                                         MileageGetter const& mileageGetter)
  : BusinessRuleApplicator(&parent),
    _taxOnFareRule(parent),
    _services(services),
    _sectorDetail(sectorDetail),
    _fares(fares),
    _farePath(farePath),
    _flights(flights),
    _flightUsages(flightUsages),
    _geoPath(geoPath),
    _geoPathMapping(geoPathMapping),
    _itinId(itinId),
    _useRepricing(useRepricing),
    _isRtw(isRtw),
    _matcher(flightUsages, flights, fares, farePath.fareUsages(), geoPathMapping),
    _prorateCalculator(mileageGetter)
{
}

TaxOnFareApplicator::~TaxOnFareApplicator()
{
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::getFareAmount(PaymentDetail const& paymentDetail, bool withMarkup) const
{
  switch (_taxOnFareRule.taxAppliesToTagInd())
  {
  case type::TaxAppliesToTagInd::Blank:
    return boost::make_optional<type::MoneyAmount>(0);

  case type::TaxAppliesToTagInd::AllBaseFare:
    if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization) &&
        withMarkup)
    {
      return boost::make_optional<type::MoneyAmount>(_farePath.totalAmount() + _farePath.totalMarkupAmount());
    }
    else
    {
      return _farePath.totalAmount();
    }

  case type::TaxAppliesToTagInd::LowestFromFareList:
    return getFareAmountForLowestFromFareList(paymentDetail);
  case type::TaxAppliesToTagInd::BetweenFareBreaks:
    return getFareAmountForFareBreaks(paymentDetail);

  case type::TaxAppliesToTagInd::USDeduct:
    if (_taxOnFareRule.processingApplTag() == taxProcessingApplTag03_fareProration)
      return getFareAmountByProration(paymentDetail);
    else
      return getFareAmountWithUSDeductMethod(paymentDetail);

  case type::TaxAppliesToTagInd::FullyApplicableRoundTripFare:
    return getFareAmountForFullyApplicableRoundTrip(paymentDetail);
  case type::TaxAppliesToTagInd::BeforeDiscount:
    return _farePath.totalAmountBeforeDiscount();
  case type::TaxAppliesToTagInd::MatchingSectorDetail:
    return getFareAmountForSectorDetail();

  default:
    return _farePath.totalAmount();
  }
}

type::MoneyAmount
TaxOnFareApplicator::getFareAmountForFullyApplicableRoundTrip(PaymentDetail const& paymentDetail, bool withMarkup)
    const
{
  type::MoneyAmount totalAmount = _farePath.totalAmount();

  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization) &&
      withMarkup)
  {
    totalAmount += _farePath.totalMarkupAmount();
  }

  if (paymentDetail.roundTripOrOpenJaw())
    return totalAmount;
  else
    return totalAmount * 2;
}

type::MoneyAmount
TaxOnFareApplicator::getNetRemitFareAmount() const
{
  type::MoneyAmount totalNetRemitFareAmount = 0;
  for (const FareUsage& fareUsage: _farePath.fareUsages())
  {
    Fare const& fare = _fares[fareUsage.index()];
    if (fare.isNetRemitAvailable())
    {
      totalNetRemitFareAmount += fare.sellAmount();
    }
    else
    {
      totalNetRemitFareAmount += fare.amount();
    }
  }
  return totalNetRemitFareAmount;
}

type::MoneyAmount
TaxOnFareApplicator::collectFareBetweenFareBreaks(PaymentDetail const& paymentDetail) const
{
  type::MoneyAmount fareSum = 0;
  size_t mappingIndex = 0;
  for (const Mapping& mapping: _geoPathMapping.mappings())
  {
    if (paymentDetail.isRangeBetweenBeginAndEnd(mapping.maps().front().index(),
                                                mapping.maps().back().index()))
    {
      fareSum += _fares[_farePath.fareUsages()[mappingIndex].index()].amount();
    }
    mappingIndex++;
  }
  return fareSum;
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::getFareAmountForLowestFromFareList(PaymentDetail const& paymentDetail) const
{
  type::Index taxBeginId = paymentDetail.getTaxPointBegin().id();
  type::Index taxEndId = paymentDetail.getTaxPointEnd().id();
  if (taxBeginId > taxEndId)
    std::swap(taxBeginId, taxEndId); // to simplify conditions checks

  bool shouldApplySpecialKHLogic =
      paymentDetail.taxName().taxCode() == "KH" &&
      _taxOnFareRule.processingApplTag() == taxProcessingApplTag06_likeFareForTaxKH &&
      paymentDetail.taxableUnits().hasTag(type::TaxableUnit::Itinerary) &&
      !paymentDetail.bothBeginAndEndAreFareBreaks() &&
      isLoc1ToLoc2Domestic(_geoPath, taxBeginId, taxEndId) &&
      hasInternationalFareComponent(taxBeginId, taxEndId, _farePath, _geoPath, _geoPathMapping);

  bool shouldApplySpecialC9Logic =
      paymentDetail.taxName().taxCode() == "C9" &&
      paymentDetail.taxName().percentFlatTag() == type::PercentFlatTag::Percent &&
      paymentDetail.getLimitType() == type::TaxApplicationLimit::OnceForItin;

  if (shouldApplySpecialKHLogic)
  {
    bool fareFound = false;
    type::MoneyAmount fareAmount = _services.repricingService().getSimilarDomesticFare(
        taxBeginId, taxEndId, _itinId, fareFound);

    if (fareFound)
      return fareAmount;
    else
      return getFareAmountByProration(paymentDetail);
  }
  else if (shouldApplySpecialC9Logic)
  {
    return getFareAmountForLowestFromFareListBahamasLogic(paymentDetail);
  }
  else
  {
    return getFareAmountForLowestFromFareListStandardLogic(paymentDetail);
  }
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::getFareAmountForLowestFromFareListBahamasLogic(PaymentDetail const& paymentDetail) const
{
  type::Index taxBeginId = paymentDetail.getTaxPointBegin().id();
  type::Index taxEndId = paymentDetail.getTaxPointEnd().id();
  if (taxBeginId > taxEndId)
    std::swap(taxBeginId, taxEndId); // to simplify conditions checks

  boost::ptr_vector<FareComponent> fareComponents =
      getFareComponentsBetween(taxBeginId, taxEndId, _farePath, _geoPathMapping);

  bool matchTicketedOnly =
        (paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly);
  type::MoneyAmount fareSum = 0;
  for (const FareComponent& fareComponent: fareComponents)
  {
    const Fare& fare = _fares.at(fareComponent.fareId);
    bool discountFare =
        fare.amount() == 0 ||
        fare.sellAmount() > 0 ||
        (std::equal(AD.rbegin(), AD.rend(), fare.basis().rbegin()) ||
         std::equal(ID.rbegin(), ID.rend(), fare.basis().rbegin()));

    if ((fareComponent.fullyInRange && !discountFare) || !_useRepricing)
    {
      fareSum += fare.amount();
    }
    else
    {
      type::Index fareEnd = std::min(taxEndId, fareComponent.endId);
      type::Index start = std::max(taxBeginId, fareComponent.beginId);
      type::Index end = start + 1;

      while (end <= fareEnd)
      {
        while (matchTicketedOnly && _geoPath.geos()[end].isUnticketed())
          end += 2;
        if (end > fareEnd)
          break;

        fareSum +=
            _services.repricingService().getBahamasSDOMFare(start, end, _itinId);
        start = end + 1;
        end = start + 1;
      }
    }
  }

  return fareSum;
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::getFareAmountForLowestFromFareListStandardLogic(PaymentDetail const& paymentDetail) const
{
  if (paymentDetail.bothBeginAndEndAreFareBreaks())
    return collectFareBetweenFareBreaks(paymentDetail);

  type::Index taxBeginId = paymentDetail.getTaxPointBegin().id();
  type::Index taxEndId = paymentDetail.getTaxPointEnd().id();
  if (taxBeginId > taxEndId)
    std::swap(taxBeginId, taxEndId); // to simplify conditions checks

  boost::ptr_vector<FareComponent> fareComponents =
      getFareComponentsBetween(taxBeginId, taxEndId, _farePath, _geoPathMapping);

  if (LocationUtil::isDomestic(paymentDetail.getTaxPointBegin().getNation(),
                               paymentDetail.getTaxPointEnd().getNation()))
  {
    type::MoneyAmount fareSum = 0;
    for (const FareComponent& fareComponent: fareComponents)
    {
      if (fareComponent.fullyInRange || !_useRepricing)
      {
        fareSum += _fares[fareComponent.fareId].amount();
      }
      else if (isInternational(fareComponent, _geoPath))
      {
        fareSum += _services.repricingService().getFareFromFareList(
            std::max(taxBeginId, fareComponent.beginId),
            std::min(taxEndId, fareComponent.endId),
            _itinId);
      }
    }
    return fareSum;
  }
  return boost::none;
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::getFareAmountForFareBreaks(PaymentDetail const& paymentDetail) const
{
  if (paymentDetail.bothBeginAndEndAreFareBreaks())
  {
    return collectFareBetweenFareBreaks(paymentDetail);
  }
  return boost::none;
}

type::MoneyAmount
TaxOnFareApplicator::getFareAmountForSectorDetail() const
{
  type::MoneyAmount fareSum = 0;
  if (_taxOnFareRule.itemNo() != 0)
  {
    if (_sectorDetail == nullptr || _sectorDetail->entries.size() == 0)
    {
      // should it fail tax application?
      return 0;
    }

    boost::ptr_vector<FareComponent> fareComponents =
        getFareComponentsBetween(0, _geoPath.geos().size() - 1, _farePath, _geoPathMapping);
    for (const FareComponent& fareComponent : fareComponents)
    {
      bool addFare = true;
      for (const type::Index& flightId : fareComponent.flightIds)
      {
        bool matchedEntry = false;
        for (const SectorDetailEntry& entry : _sectorDetail->entries)
        {
          if (_matcher.matchSectorDetails(entry, flightId, fareComponent.fareId))
          {
            matchedEntry = true;
            addFare = (entry.applTag == type::SectorDetailAppl::Positive);
            break;
          }
        }

        if (!matchedEntry)
        {
          addFare = false;
        }
        if (!addFare)
        {
          break;
        }
      }

      if (addFare)
      {
        fareSum += _fares[fareComponent.fareId].amount();
      }
    }
  }
  else
  {
    for (const auto& elem : _farePath.fareUsages())
      fareSum += _fares[elem.index()].amount();
  }
  return fareSum;
}

type::MoneyAmount
TaxOnFareApplicator::getFareAmountByProration(const PaymentDetail& paymentDetail) const
{
  type::Index taxBeginId = paymentDetail.getTaxPointBegin().id();
  type::Index taxEndId = paymentDetail.getTaxPointEnd().id();
  if (taxBeginId > taxEndId)
    std::swap(taxBeginId, taxEndId); // to simplify conditions checks

  boost::ptr_vector<FareComponent> fareComponents =
      getFareComponentsBetween(taxBeginId, taxEndId, _farePath, _geoPathMapping);

  type::MoneyAmount fareSum = 0;
  for (const FareComponent& fareComponent : fareComponents)
  {
    if (fareComponent.fullyInRange || !_useRepricing)
    {
      fareSum += _fares[fareComponent.fareId].amount();
    }
    else
    {
      fareSum += _prorateCalculator.getProratedAmount(
          Range(fareComponent.beginId, fareComponent.endId),
          ProperRange(taxBeginId, taxEndId),
          _fares[fareComponent.fareId].amount(),
          _geoPath,
          paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly,
          fareComponent.maps);
    }
  }

  type::MoneyAmount roundingUnit{0};
  type::TaxRoundingDir roundingDir{type::TaxRoundingDir::NoRounding};
  _services.taxRoundingInfoService().getFareRoundingInfo(
      paymentDetail.taxEquivalentCurrency(), roundingUnit, roundingDir);
  _services.taxRoundingInfoService().doStandardRound(fareSum, roundingUnit, roundingDir);

  return fareSum;
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::getFareAmountWithUSDeductMethod(PaymentDetail const& paymentDetail, bool withMarkup) const
{
  if (paymentDetail.bothBeginAndEndAreFareBreaks())
  {
    return boost::none;
  }

  if (!_useRepricing)
  {
    if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization) &&
        withMarkup)
    {
      return boost::make_optional<type::MoneyAmount>(_farePath.totalAmount() + _farePath.totalMarkupAmount());
    }
    else
    {
      return _farePath.totalAmount();
    }
  }

  return _services.repricingService().getFareUsingUSDeductMethod(
      paymentDetail.getTaxPointBegin().id(), paymentDetail.getTaxPointEnd().id(), _itinId);
}

boost::optional<type::MoneyAmount>
TaxOnFareApplicator::calculateFareAmount(PaymentDetail const& paymentDetail, bool withMarkup) const
{
  return (_taxOnFareRule.netRemitApplTag() == type::NetRemitApplTag::Applies)
             ? getNetRemitFareAmount()
             : getFareAmount(paymentDetail, withMarkup);
}

bool
TaxOnFareApplicator::applyForRtw(PaymentDetail& paymentDetail) const
{
  if (_fares.empty() || paymentDetail.getItineraryDetail().isFailedRule())
    return true;

  boost::optional<type::MoneyAmount> calculatedFare = calculateFareAmount(paymentDetail);
  boost::optional<type::MoneyAmount> calculatedFareWithMarkup = calculateFareAmount(paymentDetail, true);
  if (!calculatedFare)
    return true;

  type::MoneyAmount fareAmount = *calculatedFare;
  type::MoneyAmount fareWithMarkupAmount = *calculatedFareWithMarkup;

  const Geo& taxPointBegin = paymentDetail.getTaxPointBegin();

  if (_fares.size() == 1 && fareAmount == _fares[0].amount() &&
      _taxOnFareRule.taxAppliesToTagInd() != type::TaxAppliesToTagInd::AllBaseFare &&
      _taxOnFareRule.taxAppliesToTagInd() != type::TaxAppliesToTagInd::BeforeDiscount &&
      taxPointBegin.loc().tag() != type::TaxPointTag::Delivery &&
      taxPointBegin.loc().tag() != type::TaxPointTag::Sale)
  {
    fareAmount = _prorateCalculator.getProratedAmount(
        Range(0, _geoPath.geos().size() - 1),
        getPaymentRange(paymentDetail),
        fareAmount,
        _geoPath,
        paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly,
        &_geoPathMapping.mappings()[0].maps());

    fareWithMarkupAmount = _prorateCalculator.getProratedAmount(
        Range(0, _geoPath.geos().size() - 1),
        getPaymentRange(paymentDetail),
        fareWithMarkupAmount,
        _geoPath,
        paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly,
        &_geoPathMapping.mappings()[0].maps());
  }

  type::MoneyAmount roundingUnit{0};
  type::TaxRoundingDir roundingDir{type::TaxRoundingDir::NoRounding};
  _services.taxRoundingInfoService().getFareRoundingInfo(
      paymentDetail.taxEquivalentCurrency(), roundingUnit, roundingDir);

  _services.taxRoundingInfoService().doStandardRound(fareAmount, roundingUnit, roundingDir);
  paymentDetail.setTotalFareAmount(fareAmount);

  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    _services.taxRoundingInfoService().doStandardRound(fareWithMarkupAmount, roundingUnit, roundingDir);
    paymentDetail.setTotalFareWithMarkupAmount(fareWithMarkupAmount);
  }

  return true;
}

bool
TaxOnFareApplicator::applyForNonRtw(PaymentDetail& paymentDetail) const
{
  if (_fares.empty() || paymentDetail.getItineraryDetail().isFailedRule())
    return true;

  boost::optional<type::MoneyAmount> calculatedFare = calculateFareAmount(paymentDetail);
  boost::optional<type::MoneyAmount> calculatedFareWithMarkup = calculateFareAmount(paymentDetail, true);
  if (!calculatedFare)
    return true;

  if (_taxOnFareRule.netRemitApplTag() != type::NetRemitApplTag::Applies &&
      _taxOnFareRule.taxAppliesToTagInd() != type::TaxAppliesToTagInd::AllBaseFare &&
      _taxOnFareRule.taxAppliesToTagInd() !=
          type::TaxAppliesToTagInd::DefinedByTaxProcessingApplTag &&
      _farePath.totalAmount() != *calculatedFare)
  {
    paymentDetail.exchangeDetails().isPartialTax = true;
  }

  paymentDetail.setTotalFareAmount(*calculatedFare);
  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    paymentDetail.setTotalFareWithMarkupAmount(*calculatedFareWithMarkup);
  }

  return true;
}

bool
TaxOnFareApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (_isRtw)
    return applyForRtw(paymentDetail);
  else
    return applyForNonRtw(paymentDetail);
}
}
