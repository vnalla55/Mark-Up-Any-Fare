//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "FreeBagService/AllowanceSoftPassProcessor.h"

#include "DataModel/PrecalcBaggageData.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BagValidationOpt.h"

#include <algorithm>
#include <vector>

namespace tse
{
AllowanceStepResult
AllowanceSoftPassProcessor::matchAllowance(const BagValidationOpt& opt)
{
  const BaggageTravel& bt = opt._bt;
  const Itin* const itin = bt.itin();
  TSE_ASSERT(bt._trx && itin);

  const PrecalcBaggage::CxrPair cxrPair(bt._allowanceCxr, opt._deferTargetCxr);
  PrecalcBaggage::AllowanceRecords& records = _bagTravelData.allowance[cxrPair];

  const SubCodeInfo* const s5 = AllowanceUtil::retrieveS5(*bt._trx, bt._allowanceCxr);
  if (!s5)
    return S5_FAIL;

  const uint32_t startSeqNo =
      records.s7s.empty() ? 0 : (records.s7s.back()->optFee()->seqNo() + 1);
  const std::vector<OCFees*> ocFees =
      BaggageOcValidationAdapter::matchS7AllowanceSkipFareChecks(opt, *s5, startSeqNo);

  records.s5Found = true;
  records.s7s.insert(records.s7s.end(), ocFees.begin(), ocFees.end());

  if (ocFees.empty())
    return S7_FAIL;

  // If defer is possible return S7_DEFER
  const auto isDefer = [](const OCFees* oc) { return AllowanceUtil::isDefer(*oc->optFee()); };

  if (std::any_of(ocFees.begin(), ocFees.end(), isDefer))
    return S7_DEFER;

  // If all records are soft passed then it's possible that we will end up with S7_FAIL
  // [Maybe it would be more clear to replace current AllowanceStepResult with bitset of possible
  // values. I'll rethink it in the future.]

  const auto isSoftPass = [](const OCFees* oc) { return !oc->bagSoftPass().isNull(); };

  if (std::all_of(ocFees.begin(), ocFees.end(), isSoftPass))
    return S7_FAIL;

  return S7_PASS;
}

void
AllowanceSoftPassProcessor::dotTableCheckFailed(const BagValidationOpt& opt)
{
  const BaggageTravel& bt = opt._bt;
  const PrecalcBaggage::CxrPair cxrPair(bt._allowanceCxr, opt._deferTargetCxr);
  PrecalcBaggage::AllowanceRecords& records = _bagTravelData.allowance[cxrPair];
  records.s5Found = false;
}
} // ns
