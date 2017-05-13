// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#pragma once

#include "DataModel/RefundPenalty.h"
#include "Xform/AbstractTaxBreakdownModel.h"

#include <memory>
#include <vector>

namespace tse
{
class CalcTotals;
class PricingUnit;
class RefundPermutation;
class ComparablePenaltyFee;

using PricingUnits = std::vector<PricingUnit*>;

class AbstractChangeFeeModel
{
private:
  PricingTrx& _trx;
  DateTime& getTicketingDate() const;

public:
  AbstractChangeFeeModel(PricingTrx& trx) : _trx(trx) {}
  virtual bool isSkipped() const = 0;
  virtual MoneyAmount getAmount() const = 0;
  virtual CurrencyNoDec getPrecision() const = 0;
  virtual MoneyAmount getAmountInPaymentCurrency() const;
  virtual CurrencyNoDec getPaymentPrecision() const;
  virtual bool isZeroFee() const = 0;
  virtual bool isNonZeroFee() const = 0;
  virtual bool isChangeFeeNotApplicable() const;
  virtual bool isChangeFeeWaived() const;
  virtual bool isHighestChangeFeeIndicator() const = 0;
  virtual CurrencyCode getChangeFeeCurrency() const;
  virtual CurrencyCode getPaymentCurrency() const;
  virtual bool isRefundable() const;
  virtual ~AbstractChangeFeeModel() {}
};

class ChangeFeeModel : public AbstractChangeFeeModel
{
  const ComparablePenaltyFee& _fee;
  RexPricingTrx& _trx;

  ChangeFeeModel(const ChangeFeeModel&) = delete;
  ChangeFeeModel& operator=(const ChangeFeeModel&) = delete;

public:
  ChangeFeeModel(const ComparablePenaltyFee& fee, RexPricingTrx& trx);
  bool isSkipped() const override;
  MoneyAmount getAmount() const override;
  CurrencyNoDec getPrecision() const override;
  bool isZeroFee() const override;
  bool isNonZeroFee() const override;
  bool isChangeFeeNotApplicable() const override;
  bool isChangeFeeWaived() const override;
  bool isHighestChangeFeeIndicator() const override;
  CurrencyCode getChangeFeeCurrency() const override;
};

class ZeroFeeChangeFeeModel : public AbstractChangeFeeModel
{
  const RefundPermutation& _permutation;

  ZeroFeeChangeFeeModel(const ZeroFeeChangeFeeModel&) = delete;
  ZeroFeeChangeFeeModel& operator=(const ZeroFeeChangeFeeModel&) = delete;

public:
  ZeroFeeChangeFeeModel(const RefundPermutation& winnerPerm, PricingTrx& trx);
  bool isSkipped() const override;
  MoneyAmount getAmount() const override;
  CurrencyNoDec getPrecision() const override;
  bool isZeroFee() const override;
  bool isNonZeroFee() const override;
  bool isChangeFeeNotApplicable() const override;
  bool isChangeFeeWaived() const override;
  bool isHighestChangeFeeIndicator() const override;
  CurrencyCode getChangeFeeCurrency() const override;
  bool isRefundable() const override;
};

class NonZeroFeeChangeFeeModel : public AbstractChangeFeeModel
{
  bool _highestChangeFee;
  bool _refundable;
  const Money& _amount;

  NonZeroFeeChangeFeeModel(const NonZeroFeeChangeFeeModel&) = delete;
  NonZeroFeeChangeFeeModel& operator=(const NonZeroFeeChangeFeeModel&) = delete;

public:
  NonZeroFeeChangeFeeModel(const RefundPenalty::Fee& fee, PricingTrx& trx);
  NonZeroFeeChangeFeeModel(const RefundPermutation& winnerPerm,
                           bool highestChangeFee,
                           bool refundable,
                           PricingTrx& trx);
  bool isSkipped() const override;
  MoneyAmount getAmount() const override;
  CurrencyNoDec getPrecision() const override;
  bool isZeroFee() const override;
  bool isNonZeroFee() const override;
  bool isHighestChangeFeeIndicator() const override;
  CurrencyCode getChangeFeeCurrency() const override;
  bool isRefundable() const override;
};

class ReissueExchangeModel
{
  const RefundPermutation& _winnerPerm;
  const PricingUnits& _pricingUnits;
  const PricingTrx& _trx;
  std::vector<std::unique_ptr<AbstractTaxBreakdownModel>> _taxesOnChangeFeeBreakdown;

  ReissueExchangeModel(const ReissueExchangeModel&) = delete;
  ReissueExchangeModel& operator=(const ReissueExchangeModel&) = delete;

public:
  ReissueExchangeModel(const RefundPermutation& perm,
                       const PricingUnits& pricingUnits,
                       const PricingTrx& trx);

  static std::unique_ptr<ReissueExchangeModel> create(const RefundPricingTrx& trx);
  static std::unique_ptr<ReissueExchangeModel>
  create(const RefundPricingTrx& trx, CalcTotals& calcTotals);

  Indicator getFormOfRefund() const;
  bool getReissueTaxRefundable() const;
  bool isRefundable() const;
  const RefundPermutation& getWinnerPermutation() const;
  const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>& getTaxesOnChangeFee() const;
};

} // end of tse namespace
