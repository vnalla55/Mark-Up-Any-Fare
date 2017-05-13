#include "Pricing/PriceDeviator.h"

#include "Common/Code.h"
#include "Common/ErrorResponseException.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingRequest.h"

#include <limits>
#include <string>
#include <utility>

namespace tse
{
namespace
{
const std::string
reassignmentMsg("DISCOUNT - MARK UP MISMATCH");

const std::string
percentMsg(" PERCENT.");

const std::string
validDiscountRangeMsg("\nDISCOUNT PERCENTAGE SHOULD BE IN RANGE 0 - 100.");

const std::string
validMarkUpRangeMsg("\nMARK UP PERCENTAGE SHOULD BE GEATER THEN ZERO.");

const std::string
improperDiscountMsg("IMPROPER PRICE DISCOUNT REQUESTED: ");

const std::string
improperMarkUpMsg("IMPROPER PRICE MARK UP REQUESTED: ");

const std::string
currencyMissingMsg("DISCOUNT/MARK UP CURRENCY IS MISSING.");
}

void
PriceDeviatorWrapper::assignDeviator(std::unique_ptr<const PriceDeviator> deviator)
{
  if (_deviator)
    throw ErrorResponseException(ErrorResponseException::ErrorResponseCode::INVALID_INPUT,
                                 reassignmentMsg);
  deviator->validateDeviation();
  _deviator = std::move(deviator);
}

void
PriceDeviatorWrapper::configureDeviation() const
{
  if (!_request || !_deviator)
    return;

  _deviator->configureDeviation(*_request);
}

void
PercentagePriceDeviator::configureDeviation(PricingRequest& request) const
{
  constexpr int16_t dummySegment = 0;
  request.addDiscountPercentage(dummySegment, -_deviationPercent);
}

void
AmountPriceDeviator::configureDeviation(PricingRequest& request) const
{
  constexpr int16_t firstSeg = std::numeric_limits<int16_t>::min();
  constexpr int16_t lastSeg = std::numeric_limits<int16_t>::max();
  request.setDiscountAmountsNew(
      {{-std::get<0>(_deviation), std::get<1>(_deviation), firstSeg, lastSeg}});
}

void
MarkUpPercentageDeviator::validateDeviation() const
{
  if (_deviationPercent <= -EPSILON)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 improperMarkUpMsg + std::to_string(_deviationPercent) +
                                     percentMsg + validMarkUpRangeMsg);
}

void
DiscountPercentageDeviator::validateDeviation() const
{
  if (_deviationPercent <= DiscountPercentageDeviator::_lowerLimit ||
      _deviationPercent >= DiscountPercentageDeviator::_upperLimit)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 improperDiscountMsg + std::to_string(-_deviationPercent) +
                                     percentMsg + validDiscountRangeMsg);
}

void
MarkUpAmountDeviator::validateDeviation() const
{
  if (std::get<1>(_deviation).empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, currencyMissingMsg);

  if (std::get<0>(_deviation) <= -EPSILON)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 improperMarkUpMsg + std::to_string(std::get<0>(_deviation)) + " " +
                                     std::get<1>(_deviation));
}

void
DiscountAmountDeviator::validateDeviation() const
{
  if (std::get<1>(_deviation).empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, currencyMissingMsg);

  if (std::get<0>(_deviation) >= EPSILON)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 improperDiscountMsg + std::to_string(-std::get<0>(_deviation)) +
                                     " " + std::get<1>(_deviation));
}

MarkUpPercentageDeviator::MarkUpPercentageDeviator(Percent deviation)
  : PercentagePriceDeviator(deviation)
{
}

DiscountPercentageDeviator::DiscountPercentageDeviator(Percent deviation)
  : PercentagePriceDeviator(deviation)
{
}

MarkUpAmountDeviator::MarkUpAmountDeviator(const Money& money) : AmountPriceDeviator(money)
{
}

DiscountAmountDeviator::DiscountAmountDeviator(const Money& money) : AmountPriceDeviator(money)
{
}
}
