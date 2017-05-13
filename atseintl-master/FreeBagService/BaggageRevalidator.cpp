//-------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "FreeBagService/BaggageRevalidator.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/BaggageAllowanceValidator.h"
#include "FreeBagService/RegularBtaSubvalidator.h"
#include "FreeBagService/RegularNonBtaFareSubvalidator.h"

namespace tse
{
inline static boost::logic::tribool
toTribool(const StatusS7Validation x)
{
  return (x == SOFT_PASS_S7) ? boost::indeterminate : boost::logic::tribool(x == PASS_S7);
}

BaggageRevalidator::BaggageRevalidator(const PricingTrx& trx, BaggageTravel& bt, const Ts2ss& ts2ss)
{
  // We need to instantiate BaggageAllowanceValidator only because we need an implementation
  // of IS7RecordFieldsValidator interface - it's required by BTA subvalidator.
  // Therefore, some of the arguments below can be just dummy. Maybe it's not the best design
  // and it would be better to extract that impl from BaggageAllowanceValidator, I agree.

  static const CheckedPoint dummyCheckedPoint;
  constexpr bool dummyIsIntl = false;
  const BagValidationOpt opt(bt, dummyCheckedPoint, ts2ss, dummyIsIntl, nullptr);

  _s7Validator = &trx.dataHandle().safe_create<BaggageAllowanceValidator>(opt);
  _btaSubvalidator =
      &trx.dataHandle().safe_create<RegularBtaSubvalidator>(bt, *_s7Validator, ts2ss, nullptr);
  _nonBtaSubvalidator =
      &trx.dataHandle().safe_create<RegularNonBtaFareSubvalidator>(bt, ts2ss, nullptr);
}

void
BaggageRevalidator::onFaresUpdated(FarePath* fp)
{
  _btaSubvalidator->onFaresUpdated();
  _nonBtaSubvalidator->onFaresUpdated(fp);
  _s7Validator->setFarePath(fp);
}

void
BaggageRevalidator::setDeferTargetCxr(const CarrierCode& cxr)
{
  _s7Validator->setDeferTargetCxr(cxr);
}

boost::logic::tribool
BaggageRevalidator::revalidateS7(const OCFees& savedItem, const bool fcLevel)
{
  const OptionalServicesInfo& s7 = *savedItem.optFee();
  boost::logic::tribool finalResult = true;

  if (savedItem.bagSoftPass().isAnySet(OCFees::BAG_SP_RULETARIFF | OCFees::BAG_SP_TOURCODE))
  {
    const boost::logic::tribool x = toTribool(_nonBtaSubvalidator->revalidate(s7));
    if (!x)
      return false;
    finalResult = finalResult && x;
  }

  if (savedItem.bagSoftPass().isSet(OCFees::BAG_SP_BTA_FARE_CHECKS))
  {
    const boost::logic::tribool x = toTribool(_btaSubvalidator->revalidate(s7, _dummyOc, fcLevel));
    if (!x)
      return false;
    finalResult = finalResult && x;
  }

  return finalResult;
}
}
