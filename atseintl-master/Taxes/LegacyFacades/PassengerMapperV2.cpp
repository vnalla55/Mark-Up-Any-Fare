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
#include "PassengerMapperV2.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/PricingTrx.h"
#include "LegacyFacades/ConvertCode.h"

namespace tse
{

PassengerMapperV2::PassengerMapperV2(PricingTrx& trx) : _trx(trx), _serviceFeeUtil(trx) {}

bool
PassengerMapperV2::matches(const tax::type::Vendor& vendor,
                           const tax::type::CarrierCode& validatingCarrier,
                           const tax::type::PassengerCode& ticketPtc,
                           const tax::type::PassengerCode& rulePtc) const
{
  const PaxTypeCode rulePaxCode = toTsePassengerCode(rulePtc);
  const PaxTypeCode ticketPaxCode = toTsePassengerCode(ticketPtc);
  const CarrierCode v2ValCarrier = toTseCarrierCode(validatingCarrier);

  // const PaxTypeInfo* ticketPaxTypeInfo = _trx.dataHandle().getPaxType(ticketPtc, vendor);
  const PaxType* ticketPaxType =
      PaxTypeUtil::isAnActualPaxInTrx(_trx, v2ValCarrier, ticketPaxCode);

  if (!ticketPaxType)
    return false;

  return _serviceFeeUtil.matchPaxType(v2ValCarrier, *ticketPaxType, rulePaxCode);
}
}
