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
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "ItinAnalyzer/InclusionCodePaxType.h"

namespace tse
{
class FareDisplayTrx;
class FareDisplayInclCd;
class FareDisplayOptions;

class GenericInclusionCodePaxType : public InclusionCodePaxType
{
public:
  GenericInclusionCodePaxType() : _inclusionCode(nullptr) {}

  void getPaxType(FareDisplayTrx& trx) override;
  void inclusionCode(FareDisplayInclCd* inclCode) { _inclusionCode = inclCode; }

private:
  FareDisplayInclCd* _inclusionCode;

  bool isAllAdultNeeded(const FareDisplayInclCd& inclCd, const FareDisplayOptions& options) const;
  void collectPaxTypesForMultiInclusionCodesRequest(FareDisplayTrx& trx);
  void getDiscountPaxTypes(FareDisplayTrx& trx,
                           std::set<PaxTypeCode>& paxTypesForInclCode,
                           std::set<PaxTypeCode>& rec8PaxTypesForInclCode);

  static constexpr Indicator OR = 'O';
  static constexpr Indicator AND = 'A';
};
}

