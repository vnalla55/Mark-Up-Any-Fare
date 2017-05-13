#pragma once
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <memory>
#include <utility>

namespace tse
{
class PricingRequest;
class PriceDeviator;

using PriceDeviatorPtr = std::unique_ptr<const PriceDeviator>;

class PriceDeviatorWrapper
{
  PricingRequest*& _request;
  PriceDeviatorPtr _deviator;

public:
  explicit PriceDeviatorWrapper(PricingRequest*& request) : _request(request) {}
  void assignDeviator(PriceDeviatorPtr deviator);
  void configureDeviation() const;
};

class PriceDeviator
{
public:
  virtual ~PriceDeviator() = default;
  virtual void configureDeviation(PricingRequest& request) const = 0;
  virtual void validateDeviation() const = 0;
};

class AmountPriceDeviator : public PriceDeviator
{
  void configureDeviation(PricingRequest& request) const override final;

protected:
  using Money = std::pair<MoneyAmount, CurrencyCode>;

  AmountPriceDeviator(const Money& money) : _deviation(money) {}

  const Money _deviation;
};

class DiscountAmountDeviator final : public AmountPriceDeviator
{
public:
  explicit DiscountAmountDeviator(const Money& money);

private:
  void validateDeviation() const override;
};

class MarkUpAmountDeviator final : public AmountPriceDeviator
{
public:
  explicit MarkUpAmountDeviator(const Money& money);

private:
  void validateDeviation() const override;
};

class PercentagePriceDeviator : public PriceDeviator
{
  void configureDeviation(PricingRequest& request) const override final;

protected:
  PercentagePriceDeviator(Percent deviation) noexcept : _deviationPercent(deviation) {}
  const Percent _deviationPercent;
};

class MarkUpPercentageDeviator final : public PercentagePriceDeviator
{
public:
  explicit MarkUpPercentageDeviator(Percent deviation);

private:
  void validateDeviation() const override;
};

class DiscountPercentageDeviator final : public PercentagePriceDeviator
{
public:
  explicit DiscountPercentageDeviator(Percent deviation);

private:
  void validateDeviation() const override;
  static constexpr Percent _lowerLimit = -100 - EPSILON; // allow to be 100
  static constexpr Percent _upperLimit = EPSILON; // allow to be 0
};
}
