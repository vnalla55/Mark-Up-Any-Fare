//-------------------------------------------------------------------
//
//  File:        CheckTrxError.h
//  Created:     June 26, 2007
//  Authors:     Simon Li
//
//  Updates:
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

#include "Common/ErrorResponseException.h"

#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class RexBaseTrx;
class FareCompInfo;
class PaxTypeFare;

class CheckTrxError
{
  friend class CheckTrxErrorTest;

public:
  CheckTrxError(RexBaseTrx& trx);

  void process() const;
  void checkPermutations() const;
  void multiItinProcess() const;
  void multiItinCheckPermutations() const;

protected:
  void processRepriceExcPhase() const;
  void processMatchExcRulePhase() const;
  void processPaxTypeFareForRefund(const PaxTypeFare* ptf) const;
  void processPaxTypeFareForReissue(const PaxTypeFare* ptf) const;

  bool fareCompMatchedFare(const FareCompInfo& fc) const;
  bool fareCompMatchedReissueRules(const FareCompInfo& fc) const;

  void getBaseFare(const PaxTypeFare*& ptf) const;

private:
  RexBaseTrx& _trx;
  const uint16_t _subjectCategory;
  ErrorResponseException::ErrorResponseCode _missingR3Error;
};
} // namespace tse

