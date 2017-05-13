// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/TseEnums.h"
#include "Common/ServiceFeeUtil.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/PassengerMapper.h"

namespace tse
{
class PricingTrx;

class PassengerMapperV2 : public tax::PassengerMapper
{
  friend class PassengerMapperV2Test;

public:
  PassengerMapperV2(PricingTrx& pricingTrx);
  virtual ~PassengerMapperV2() {}

  virtual bool matches(const tax::type::Vendor& vendor,
                       const tax::type::CarrierCode& validatingCarrier,
                       const tax::type::PassengerCode& ticketPtc,
                       const tax::type::PassengerCode& rulePtc) const override;

private:
  PricingTrx& _trx;
  ServiceFeeUtil _serviceFeeUtil;
};

} // namespace tse
