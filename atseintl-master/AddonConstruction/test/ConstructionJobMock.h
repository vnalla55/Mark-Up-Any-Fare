#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "DBAccess/DataHandle.h"
#include "Common/DateTime.h"
#include "AddonConstruction/ConstructionJob.h"

#define MINUS_ONE_HOUR_TIME_SHFT -1
namespace tse
{
class ConstructionJobMock : public ConstructionJob
{
public:
  ConstructionJobMock(PricingTrx& trx, bool isHistorical = false) : ConstructionJob(trx)
  {
    DateTime date;
    date = (isHistorical ? DateTime::localTime() - Hours(24) : DateTime::localTime());
    Global::_allowHistorical = isHistorical;
    _dataHandle.setTicketDate(date);
    _dataHandle.refreshHist(date);
  }

  void set_singleOverDouble(bool val) { _singleOverDouble = val; }
};
}
