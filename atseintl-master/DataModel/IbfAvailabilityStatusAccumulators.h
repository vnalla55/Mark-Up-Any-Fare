//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#pragma once

#include "Common/TseEnums.h"
#include "DataModel/IbfAvailabilityEnumPolicies.h"
#include "Pricing/Shopping/Utils/StatusAccumulator.h"

namespace tse
{

typedef utils::StatusAccumulator<BookingCodeValidationStatus, BcvsInitPolicy, NotAvailableWins>
BcvsNotAvailableWins;

typedef utils::StatusAccumulator<BookingCodeValidationStatus, BcvsInitPolicy, NotOfferedWins>
BcvsNotOfferedWins;

typedef utils::StatusAccumulator<IbfErrorMessage,
                                 IbfErrorMessageFmInitPolicy,
                                 IbfErrorMessageChoiceOrder> IbfErrorMessageFmAcc;

typedef utils::StatusAccumulator<IbfErrorMessage,
                                 IbfErrorMessageInitPolicy,
                                 IbfErrorMessageChoiceOrder> IbfErrorMessageChoiceAcc;

typedef utils::StatusAccumulator<IbfErrorMessage,
                                 IbfErrorMessageInitPolicy,
                                 IbfErrorMessageSequenceOrder> IbfErrorMessageSequenceAcc;

} // namespace tse

