//----------------------------------------------------------------------------
// Copyright Sabre 2015
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//----------------------------------------------------------------------------
#include "Decode/FrequentFlyerStatusGenerator.h"

#include "DataModel/FrequentFlyerTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FreqFlyerStatus.h"

namespace tse
{
void FrequentFlyerStatusGenerator::generateStatusList()
{
  for (const auto& cxrData : _trx.getCxrs())
  {
    FrequentFlyerTrx::StatusList statusList;
    for (const FreqFlyerStatus* status : _trx.dataHandle().getFreqFlyerStatuses(cxrData.first))
    {
      statusList.emplace_back(status);
      statusList.back()._maxPassengersTotal =
          status->_maxPassengersSamePNR + status->_maxPassengersDiffPNR;
    }

    _trx.setCxrData(cxrData.first, std::move(statusList));
  }
}
}
