//------------------------------------------------------------------
//
//  File: MockPenalties.h
//
//  Copyright Sabre 2014
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------

#ifndef MOCKPENALTIES_H_
#define MOCKPENALTIES_H_

#include <gmock/gmock.h>
#include "Rules/Penalties.h"
#include "DBAccess/PenaltyInfo.h"
#include "Diagnostic/DiagManager.h"

namespace tse
{

class MockPenalties : public Penalties
{
public:
  MOCK_CONST_METHOD2(validateOptions, Record3ReturnTypes(const PenaltyInfo*, DiagManager&));
  virtual ~MockPenalties() {}
};

} // namespace tse
#endif /* MOCKPENALTIES_H_ */
