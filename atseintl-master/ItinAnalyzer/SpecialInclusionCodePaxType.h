//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "ItinAnalyzer/InclusionCodePaxType.h"

namespace tse
{
class PaxTypeInfo;

class SpecialInclusionCodePaxType : public InclusionCodePaxType
{
public:
  void getPaxType(FareDisplayTrx& trx) override;

private:
  virtual const PaxTypeInfo*
  getPaxType(FareDisplayTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  virtual const std::set<std::pair<PaxTypeCode, VendorCode> >&
  getFareDisplayWebPaxForCxr(FareDisplayTrx& trx, const CarrierCode& carrier);
};
}

