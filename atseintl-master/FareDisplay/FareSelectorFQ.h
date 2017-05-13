//-------------------------------------------------------------------
//
//  File:        FareSelectorFQ.h
//  Created:     October 7, 2005
//  Authors:     Doug Batchelor
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

#include "Common/Thread/TseCallableTrxTask.h"
#include "FareDisplay/FareSelector.h"

namespace tse
{
class FareDisplayTrx;

/**
*   @class FareDisplayFQ
*
*   Description:
*   Retrieves The FQ Fares
*
*/
class FareSelectorFQ : public FareSelector
{
  friend class FareSelectorTest;

public:
  bool selectFares(FareDisplayTrx& trx) override;

  struct QualifyFareTask : public TseCallableTrxTask
  {
    QualifyFareTask() { desc("FS QUALIFY FARE TASK"); }

    void performTask() override;

    bool _fareSelected = false;
    CurrencyCode _alternateCurrency = EMPTY_STRING();
    FareMarket* _fareMarket = nullptr;
    FareSelector* _fareSelector = nullptr;
  };
};

} // namespace tse

