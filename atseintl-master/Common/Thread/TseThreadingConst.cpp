#include "Common/Thread/TseThreadingConst.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"

namespace tse
{

namespace TseThreadingConst
{

namespace
{

const char* TaskName[] =
{
  "TRANSACTION_TASK",
  "TRANSACTION_ASYNCHRONOUS_TASK",
  "PRICING_ITIN_TASK",
  "PRICING_ITIN_ASE_TASK",
  "FAREPATHPQ_TASK",
  "PRICINGUNITPQ_TASK",
  "FAREPATHCOLLECTOR_TASK",
  "INITFAREPATH_TASK",
  "NETFAREMARKETS_TASK",
  "PRICINGPQ_TASK",
  "PUPATHMATRIX_TASK",
  "SURCHARGES_TASK",
  "GATEWAY_TASK",
  "VENDOR_TASK",
  "FARESC_TASK",
  "FARESV_TASK",
  "FARE_CALC_TASK",
  "FARE_SELECTOR_TASK",
  "BAGGAGE_TASK",
  "REX_FARE_SELECTOR_TASK",
  "SVCFEESCOLLECTOR_TASK",// 20
  "SHOPPING_TASK",
  "SHOPPINGPQ_TASK",
  "TAX_TASK",
  "TO_TASK",
  "LDC_TASK",
  "LDC_FULL_VERIFY_TASK",
  "LDC_ACTION_QUEUE_TASK",
  "LDC_CACHE_EVENT_TASK",
  "CACHE_INITIALIZATION_TASK",
  "CACHE_UPDATE_TASK",// 30
  "SYNCHRONOUS_TASK",
  "SCOPED_EXECUTOR_TASK",
  "TASK_ID_ANY",
  "BOOSTER_TASK",
  "THROTTLING_TASK",
  "LOAD_ON_UPDATE_TASK",
  "Should not display this"
};

}// namespace

const char* getTaskName(TaskId taskId)
{
  return TaskName[taskId];
}

int getStatsPeriod()
{
  int period(0);
  if (Global::hasConfig())
  {
    Global::config().getValue("THREAD_STATS_PERIODICITY", period, "TSE_SERVER");
  }
  return period;
}

int getStatsThreshold()
{
  int threshold(0);
  if (Global::hasConfig())
  {
    Global::config().getValue("THREAD_STATS_THRESHOLD", threshold, "TSE_SERVER");
  }
  return threshold;
}

}// TseThreadingConst

}// tse
