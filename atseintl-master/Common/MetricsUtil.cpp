//----------------------------------------------------------------------------
//
//  Description: Common Metrics formatting functions for ATSE shopping/pricing.
//
//  Updates:
//
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
#include "Common/MetricsUtil.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Gauss.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/Monitor.h"
#include "Common/MemoryUsage.h"
#include "Common/MetricsMan.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/Trx.h"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <tr1/unordered_map>

namespace tse
{
namespace
{
ConfigurableValue<bool>
showSystem("METRICS", "SHOW_SYSTEM_TIME", false);
ConfigurableValue<bool>
useHiResTimer("METRICS", "USE_HIRES_TIMER", false);
ConfigurableValue<bool>
collectCpuTopLevelOnly("METRICS", "COLLECT_CPU_TOP_LEVEL_ONLY", true);
ConfigurableValue<bool>
forceTopLevelOnly("METRICS", "FORCE_TOP_LEVEL_ONLY", false);
ConfigurableValue<bool>
forceLowLevel("METRICS", "FORCE_LOW_LEVEL", false);

const std::tr1::unordered_map<int, std::string>
createDescs()
{
  std::tr1::unordered_map<int, std::string> factorDescs;

  //
  // TO Metrics
  //
  factorDescs[MetricsUtil::TO_PROCESS_CURRENCY] = "TO PROCESS CURRENCY";
  factorDescs[MetricsUtil::TO_PROCESS_METRICS] = "TO PROCESS METRICS";
  factorDescs[MetricsUtil::TO_PROCESS_MILEAGE] = "TO PROCESS MILEAGE";
  factorDescs[MetricsUtil::TO_PROCESS_PRICING] = "TO PROCESS PRICING";
  factorDescs[MetricsUtil::TO_PROCESS_SHOPPING] = "TO PROCESS SHOPPING";
  factorDescs[MetricsUtil::TO_PROCESS_STATUS] = "TO PROCESS STATUS";

  factorDescs[MetricsUtil::TO_SVC_FARES_C] = "TO FARES C SERVICE";
  factorDescs[MetricsUtil::TO_SVC_FARES_V] = "TO FARES V SERVICE";
  factorDescs[MetricsUtil::TO_SVC_PRICING] = "TO PRICING SERVICE";
  factorDescs[MetricsUtil::TO_SVC_SHOPPING] = "TO SHOPPING SERVICE";
  factorDescs[MetricsUtil::TO_SVC_ITIN] = "TO ITIN SERVICE";
  factorDescs[MetricsUtil::TO_SVC_TAX] = "TO TAX SERVICE";
  factorDescs[MetricsUtil::TO_SVC_FARE_CALC] = "TO FARE CALC SERVICE";
  factorDescs[MetricsUtil::TO_SVC_CURRENCY] = "TO CURRENCY SERVICE";
  factorDescs[MetricsUtil::TO_SVC_MILEAGE] = "TO MILEAGE SERVICE";
  factorDescs[MetricsUtil::TO_SVC_INTERNAL] = "TO INTERNAL SERVICE";
  factorDescs[MetricsUtil::TO_SVC_SERVICE_FEES] = "TO SERVICE FEES SERVICE";
  factorDescs[MetricsUtil::TO_SVC_TICKETING_FEES] = "TO TICKETING FEES SERVICE";
  factorDescs[MetricsUtil::TO_SVC_S8_BRAND] = "TO S8 BRAND SERVICE";
  factorDescs[MetricsUtil::TO_SVC_DECODE] = "TO DECODE SERVICE";

  //
  // FCO Metrics
  //
  factorDescs[MetricsUtil::FCO_PROCESS] = "FCO PROCESS";
  factorDescs[MetricsUtil::FCO_GOV_CXR] = "FCO GOV CXR";
  factorDescs[MetricsUtil::FCO_GLB_DIR] = "FCO GLOBAL DIR";
  factorDescs[MetricsUtil::FCO_FLT_TRACKER] = "FCO FLT TRACKER";
  factorDescs[MetricsUtil::FCO_FIND_FARES] = "FCO FINDFARES";
  factorDescs[MetricsUtil::FCO_DIAG] = "FCO DIAGNOSTICS";
  factorDescs[MetricsUtil::FCO_FARE_CS] = "FCO CURRENCY SELECT";
  factorDescs[MetricsUtil::FCO_CFC_PROCESS] = "FCO CXRFARECNTRLR";
  factorDescs[MetricsUtil::FCO_AFC_PROCESS] = "FCO ADDONFARECNTRLR";
  factorDescs[MetricsUtil::FCO_FBRC_PROCESS] = "FCO FBRCNTRLR";
  factorDescs[MetricsUtil::FCO_DFC_PROCESS] = "FCO DISCFARECNTRLR";
  factorDescs[MetricsUtil::FCO_DFC_VALIDATION] = "FCO DISCFARE VALIDATION";
  factorDescs[MetricsUtil::FCO_DFC_CALCAMT] = "FCO DISCFARE CALCAMT";
  factorDescs[MetricsUtil::FCO_IFC_PROCESS] = "FCO INDFARECNTRLR";
  factorDescs[MetricsUtil::FCO_NFC_PROCESS] = "FCO NEGFARECNTRLR";
  factorDescs[MetricsUtil::FCO_FARE_SORT] = "FCO FARE SORT";
  factorDescs[MetricsUtil::FCO_FC_TVLDATE] = "FCO FC TVLDATE";
  factorDescs[MetricsUtil::FCO_FC_TXREF] = "FCO FC TXREF";
  factorDescs[MetricsUtil::FCO_FC_FCA] = "FCO FC FCAPP";
  factorDescs[MetricsUtil::FCO_FC_FCASEG] = "FCO FC FCAPPSEG";
  factorDescs[MetricsUtil::FCO_FC_FTMATRIX] = "FCO FC FTMATRIX";
  factorDescs[MetricsUtil::FCO_FC_MATCHLOC] = "FCO FC MATCHLOC";
  factorDescs[MetricsUtil::FCO_FC_CREATEFARES] = "FCO FC CREATEFARES";
  factorDescs[MetricsUtil::FCO_FC_CREATEPTFARES] = "FCO FC CREATEPTFARES";
  factorDescs[MetricsUtil::FCO_FC_CURRENCY] = "FCO FC CURRENCY";
  factorDescs[MetricsUtil::FCO_FC_INITFARE] = "FCO FC INITFARE";
  factorDescs[MetricsUtil::FCO_FC_GETFARE] = "FCO FC GETFARE";

  //
  // FVO Metrics
  //
  factorDescs[MetricsUtil::FVO_PROCESS] = "FVO PROCESS";
  factorDescs[MetricsUtil::FVO_ROUTING] = "FVO ROUTING";
  factorDescs[MetricsUtil::FVO_RTG_MAP_VALIDATION] = "FVO ROUTING MAP";
  factorDescs[MetricsUtil::FVO_RTG_MAP] = "FVO MAP SPECIFIED";
  factorDescs[MetricsUtil::FVO_RTG_DRV] = "FVO MAP DRV";
  factorDescs[MetricsUtil::FVO_RTG_MILEAGE_VALIDATION] = "FVO ROUTING MILEAGE";
  factorDescs[MetricsUtil::FVO_RTG_TPM] = "FVO MILEAGE TPM";
  factorDescs[MetricsUtil::FVO_RTG_MPM] = "FVO MILEAGE MPM";
  factorDescs[MetricsUtil::FVO_RTG_TPD] = "FVO MILEAGE TPD";
  factorDescs[MetricsUtil::FVO_RTG_PSR] = "FVO MILEAGE PSR";
  factorDescs[MetricsUtil::FVO_RTG_RESTRICTION_VALIDATION] = "FVO ROUTING RESTRICTION";
  factorDescs[MetricsUtil::FVO_RTG_SURCH_EXCEPT] = "FVO ROUTING MILEAGE SURCHARGE";
  factorDescs[MetricsUtil::FVO_BKGCODE_VALIDATOR] = "FVO BKGCODE VALIDATOR";
  factorDescs[MetricsUtil::FVO_RULE_CONTROLLER] = "FVO RULE CONTROLLER";
  factorDescs[MetricsUtil::FVO_MC_CONTROLLER] = "FVO MIXED CLASS CTRL";

  //
  // PO Metrics
  //
  factorDescs[MetricsUtil::PO_PROCESS] = "PO PROCESS";
  factorDescs[MetricsUtil::PO_FMKT_MERGER] = "PO FMKT MERGER";
  factorDescs[MetricsUtil::PO_FMKT_PATH_MATRIX] = "PO FMKT PATH MATRIX";
  factorDescs[MetricsUtil::PO_PU_PATH_MATRIX] = "PO PU PATH MATRIX";
  factorDescs[MetricsUtil::PO_INIT_PU_PQ] = "PO INIT PU PQ";
  factorDescs[MetricsUtil::PO_GET_PU] = "PO GET PU";
  factorDescs[MetricsUtil::PO_BUILDING_PU] = "PO BUILDING PU";
  factorDescs[MetricsUtil::PO_PU_PQ_OPERATION] = "PO PU PQ OPERATION";
  factorDescs[MetricsUtil::PO_INIT_FP_PQ] = "PO INIT FP PQ";
  factorDescs[MetricsUtil::PO_GET_FP] = "PO GET FAREPATH";
  factorDescs[MetricsUtil::PO_FP_PQ_OPERATION] = "PO FP PQ OPERATION";
  factorDescs[MetricsUtil::PO_INIT_GFP_PQ] = "PO INIT GROUPFP PQ";
  factorDescs[MetricsUtil::PO_GET_GFP] = "PO GET GROUP FAREPATH";
  factorDescs[MetricsUtil::PO_GFP_PQ_OPERATION] = "PO GFP PQ OPERATION";
  factorDescs[MetricsUtil::PO_COMB_PROCESS] = "PO COMBINATIONS PROCESS";
  factorDescs[MetricsUtil::PO_CXR_PREF_CHECK] = "PO CXR PREF CHK";
  factorDescs[MetricsUtil::PO_OJ_DEF_CHECK] = "PO OJ DEF CHK";
  factorDescs[MetricsUtil::PO_RT_DEF_CHECK] = "PO RT DEF CHK";
  factorDescs[MetricsUtil::PO_LIMITATION] = "PO LIMITATION";
  factorDescs[MetricsUtil::PO_RT_TO_CT_CONVERSION] = "PO RT TO CT CONVERSION";
  factorDescs[MetricsUtil::PO_MINFARE_PROCESS] = "PO MINFARE PROCESS";
  factorDescs[MetricsUtil::PO_RULE_VALIDATE] = "PO RULE VALIDATION ";
  factorDescs[MetricsUtil::PO_BKGCODE_REVALIDATE] = "PO BKGCODE REVALIDATE";
  factorDescs[MetricsUtil::PO_BKGCODE_MIXED_CLASS] = "PO BKGCODE MIXED CLASS";
  factorDescs[MetricsUtil::PO_COLLECT_ENDORSEMENT] = "PO COLLECT ENDORSEMENT";
  factorDescs[MetricsUtil::PO_DETERMINE_MOST_REST_TKT_DT] = "PO DETERMINE MOST REST TKT DT";

  //
  // SO Metrics
  //
  factorDescs[MetricsUtil::SO_PROCESS] = "SO PROCESS";

  //
  // ITIN Metrics
  //
  factorDescs[MetricsUtil::ITIN_PROCESS] = "ITIN PROCESS";
  factorDescs[MetricsUtil::ITIN_ATAE] = "ITIN ATAE";

  //
  // TAX Metrics
  //
  factorDescs[MetricsUtil::TAX_PROCESS] = "TAX PROCESS";

  //
  // SERVICE FEES Metrics
  //
  factorDescs[MetricsUtil::SERVICE_FEES_PROCESS] = "SERVICE FEES PROCESS";

  //
  // TICKETING FEES Metrics
  //
  factorDescs[MetricsUtil::TICKETING_FEES_PROCESS] = "TICKETING FEES PROCESS";

  //
  // S8 BRAND Metrics
  //
  factorDescs[MetricsUtil::S8_BRAND_PROCESS] = "S8 BRAND PROCESS";
  factorDescs[MetricsUtil::S8_BRAND_QUERY] = "S8 BRAND QUERY";

  //
  // CBAS BRAND Metrics
  //
  factorDescs[MetricsUtil::CBAS_BRAND_QUERY] = "CBAS BRAND QUERY";

  //
  // CorbaRequestManager Metrics
  //
  factorDescs[MetricsUtil::CORBAMGR_PROCESS] = "CORBAMGR PROCESS";
  factorDescs[MetricsUtil::CORBAMGR_REQ_XFORM] = "CORBAMGR REQXFORM";
  factorDescs[MetricsUtil::CORBAMGR_SERVICE] = "CORBAMGR SERVICE";
  factorDescs[MetricsUtil::CORBAMGR_RSP_XFORM] = "CORBAMGR RSPXFORM";

  //
  // Rules Metrics
  //
  factorDescs[MetricsUtil::RC_VALIDATE_PAXFARETYPE] = "RULE VALIDATION PAX FARE TYPE";
  factorDescs[MetricsUtil::RC_VALIDATE_PRICINGUNIT] = "RULE VALIDATION PRICING UNIT";

  //
  // Rule validation phases
  //
  factorDescs[MetricsUtil::RC_FC_PRE_VALIDATION] = "RULE PRE-VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_NORMAL_VALIDATION] = "RULE NORMAL VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_PRECOMBINABILITY_VALIDATION] = "RULE PRE-COMBINABILITY FARE";
  factorDescs[MetricsUtil::RC_FC_RE_VALIDATION] = "RULE RE-VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_VALIDATION] = "RULE SHOPPING VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_ASO_VALIDATION] = "RULE SHOPPING ASO VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_WITH_FLIGHTS_VALIDATION] =
      "RULE SHOPPING FLIGHTS VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_ASO_WITH_FLIGHTS_VALIDATION] =
      "RULE SHOPPING ASO FLIGHTS VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_DYNAMIC_VALIDATION] = "RULE DYNAMIC VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_VALIDATE_IF_CAT4] = "RULE SHOPPING VALIDATE IF CAT4";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_ITIN_BASED_VALIDATION] =
      "RULE SHOPPING ITIN BASED VALIDATION";
  factorDescs[MetricsUtil::RC_FC_SHOPPING_ITIN_BASED_VALIDATION_ALT] =
      "RULE SHOPPING ALTDATE ITIN BASED VALIDATION";
  factorDescs[MetricsUtil::RC_FC_FARE_DISPLAY_VALIDATION] = "FARE DISPLAY NORMAL VALIDATION FARE";
  factorDescs[MetricsUtil::RC_FC_RULE_DISPLAY_VALIDATION] = "RULE DISPLAY NORMAL VALIDATION FARE";

  factorDescs[MetricsUtil::RC_PU_PRE_VALIDATION] = "RULE PRE-VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_NORMAL_VALIDATION] = "RULE NORMAL VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_PRECOMBINABILITY_VALIDATION] = "RULE PRE-COMBINABILITY PU";
  factorDescs[MetricsUtil::RC_PU_RE_VALIDATION] = "RULE RE-VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_VALIDATION] = "RULE SHOPPING VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_ASO_VALIDATION] = "RULE SHOPPING ASO VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_WITH_FLIGHTS_VALIDATION] =
      "RULE SHOPPING FLIGHTS VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_ASO_WITH_FLIGHTS_VALIDATION] =
      "RULE SHOPPING ASO FLIGHTS VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_DYNAMIC_VALIDATION] = "RULE DYNAMIC VALIDATION PU";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_VALIDATE_IF_CAT4] = "RULE SHOPPING VALIDATE IF CAT4";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_ITIN_BASED_VALIDATION] =
      "RULE SHOPPING ITIN BASED VALIDATION";
  factorDescs[MetricsUtil::RC_PU_SHOPPING_ITIN_BASED_VALIDATION_ALT] =
      "RULE SHOPPING ALTDATE ITIN BASED VALIDATION";

  //
  // Rules - Record 3 Category Rules
  //
  factorDescs[MetricsUtil::RULE3CAT_ALL] = "RULES ALL RECORD 3 CATEGORIES";
  factorDescs[MetricsUtil::RULE3CAT_1] = "RULE CAT  1";
  factorDescs[MetricsUtil::RULE3CAT_1_PU] = "CAT  1 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_1_FC] = "CAT  1 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_2] = "RULE CAT  2";
  factorDescs[MetricsUtil::RULE3CAT_2_PU] = "CAT  2 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_2_FC] = "CAT  2 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_3] = "RULE CAT  3";
  factorDescs[MetricsUtil::RULE3CAT_3_PU] = "CAT  3 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_3_FC] = "CAT  3 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_4] = "RULE CAT  4";
  factorDescs[MetricsUtil::RULE3CAT_4_PU] = "CAT  4 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_4_FC] = "CAT  4 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_5] = "RULE CAT  5";
  factorDescs[MetricsUtil::RULE3CAT_5_PU] = "CAT  5 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_5_FC] = "CAT  5 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_6] = "RULE CAT  6";
  factorDescs[MetricsUtil::RULE3CAT_6_PU] = "CAT  6 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_6_FC] = "CAT  6 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_7] = "RULE CAT  7";
  factorDescs[MetricsUtil::RULE3CAT_7_PU] = "CAT  7 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_7_FC] = "CAT  7 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_8] = "RULE CAT  8";
  factorDescs[MetricsUtil::RULE3CAT_8_PU] = "CAT  8 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_8_FC] = "CAT  8 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_9] = "RULE CAT  9";
  factorDescs[MetricsUtil::RULE3CAT_9_PU] = "CAT  9 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_9_FC] = "CAT  9 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_11] = "RULE CAT 11";
  factorDescs[MetricsUtil::RULE3CAT_11_PU] = "CAT 11 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_11_FC] = "CAT 11 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_12] = "RULE CAT 12";
  factorDescs[MetricsUtil::RULE3CAT_12_PU] = "CAT 12 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_12_FC] = "CAT 12 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_13] = "RULE CAT 13";
  factorDescs[MetricsUtil::RULE3CAT_13_PU] = "CAT 13 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_13_FC] = "CAT 13 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_14] = "RULE CAT 14";
  factorDescs[MetricsUtil::RULE3CAT_14_PU] = "CAT 14 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_14_FC] = "CAT 14 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_15] = "RULE CAT 15";
  factorDescs[MetricsUtil::RULE3CAT_15_PU] = "CAT 15 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_15_FC] = "CAT 15 FC SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_16] = "RULE CAT 16";
  factorDescs[MetricsUtil::RULE3CAT_16_PU] = "CAT 16 PU SCOPE";
  factorDescs[MetricsUtil::RULE3CAT_16_FC] = "CAT 16 FC SCOPE";
  factorDescs[MetricsUtil::TO_PROCESS_FARE_DISPLAY] = " TO FARE DISPLAY PROCESS ";
  factorDescs[MetricsUtil::FARE_DISPLAY_SVC] = " FARE DISPLAY SERVICE ";
  factorDescs[MetricsUtil::FARE_DISPLAY_DSS] = " FARE DISPLAY SCHEDULES ";

  return factorDescs;
}

static const std::tr1::unordered_map<int, std::string>
_factorDescs(createDescs());
static const std::string
_emptyString("");
} // unnamed namespace

const std::string MetricsUtil::ITEM_SEPARATOR = " ";

boost::mutex MetricsUtil::_loadTimerImplsMutex;

bool MetricsUtil::_timerImplsLoaded = false;
bool MetricsUtil::_forceTopLevelOnly = false;
bool MetricsUtil::_forceLowLevel = false;

const MetricsTimerImpl* MetricsUtil::_doNothingTimerImpl;
const MetricsTimerImpl* MetricsUtil::_defaultTopLevelTimerImpl;
const MetricsTimerImpl* MetricsUtil::_defaultTimerImpl;

const double MetricsUtil::LORES_ELAPSED_TIME_RESOLUTION = 1.0e-2;
const double MetricsUtil::LORES_CPU_TIME_RESOLUTION = 1.0e-2;
const double MetricsUtil::HIRES_ELAPSED_TIME_RESOLUTION = 1.0e-6;
const double MetricsUtil::HIRES_CPU_TIME_RESOLUTION = 1.0e-6;

double MetricsUtil::ELAPSED_TIME_RESOLUTION = MetricsUtil::LORES_ELAPSED_TIME_RESOLUTION;

double MetricsUtil::CPU_TIME_RESOLUTION = MetricsUtil::LORES_CPU_TIME_RESOLUTION;

std::size_t MetricsUtil::ELAPSED_TIME_DECIMAL_PLACES =
    MetricsUtil::LORES_ELAPSED_TIME_DECIMAL_PLACES;
std::size_t MetricsUtil::CPU_TIME_DECIMAL_PLACES = MetricsUtil::LORES_CPU_TIME_DECIMAL_PLACES;

MetricsUtil::getThreadCPUTimeFunc* MetricsUtil::_getThreadCPUTimeFunc;

namespace
{
bool
getThreadCPUTimeFromProc(long& utime, long& stime)
{
  int tid = (int)syscall(SYS_gettid);
  char fname[100];
  sprintf(fname, "/proc/self/task/%d/stat", tid);

  int fd = open(fname, O_RDONLY);
  if (UNLIKELY(fd == -1))
  {
    return false;
  }

  char buf[1024];
  ssize_t nbytes = read(fd, buf, sizeof(buf));
  close(fd);

  if (nbytes == -1)
  {
    return false;
  }

  const char* ptr = buf;
  int spaces = 0;
  while (spaces != 13 && ptr != buf + nbytes)
  {
    if (*ptr++ == ' ')
    {
      ++spaces;
    }
  }

  if (LIKELY(ptr != buf + nbytes))
  {
    utime = atol(ptr);
    while (ptr != buf + nbytes && *ptr != ' ')
    {
      ++ptr;
    }

    if (LIKELY(ptr != buf + nbytes))
    {
      ++ptr;
      stime = atol(ptr);
      return true;
    }
  }
  return false;
}

bool
getThreadCPUTimeFromHRT(long& utime, long& stime)
{
  // Use the HRT (High Resolution Timer) API to
  //  get CPU time down to 1 nanosecond resolution.

  timespec tscpu;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tscpu) != 0)
  {
    return false;
  }
  // Return the time in microseconds
  //
  utime = (tscpu.tv_sec * 1000000L) + (tscpu.tv_nsec / 1000L);
  stime = 0;
  return true;
}

bool
getThreadCPUTimeFromStdLib(long& utime, long& stime)
{
  struct tms tmStruct;
  tmStruct.tms_utime = 0;
  tmStruct.tms_stime = 0;
  tmStruct.tms_cutime = 0;
  tmStruct.tms_cstime = 0;

  times(&tmStruct);
  utime = tmStruct.tms_utime;
  stime = tmStruct.tms_stime;
  return true;
}

} // namespace

bool
MetricsUtil::getThreadCPUTime(long& utime, long& stime)
{
  if (!_timerImplsLoaded)
    loadTimerImpls();
  return (*_getThreadCPUTimeFunc)(utime, stime);
}

// Base metrics timer implementation class.
// An instance of this class is a "Do Nothing Timer" and is used for
//  disabled metrics. Other timer classes inherit from this class.
//
class MetricsTimerImpl
{
public:
  MetricsTimerImpl(bool enabled = false) : _enabled(enabled) {}
  virtual ~MetricsTimerImpl() {}

  bool enabled() const { return _enabled; }

  virtual void start(MetricsTimer::Data& data) const { /*Do nothing*/}
  virtual void stop(MetricsTimer::Data& data) const { /*Do nothing*/}

private:
  MetricsTimerImpl(const MetricsTimerImpl&);
  MetricsTimerImpl& operator=(const MetricsTimerImpl&);
  bool _enabled;
};

class CommonTimerImplBase : public MetricsTimerImpl
{
public:
  virtual void start(MetricsTimer::Data& data) const override;
  virtual void stop(MetricsTimer::Data& data) const override;

protected:
  CommonTimerImplBase() : MetricsTimerImpl(true) {}
  virtual bool getThreadCPUTime(long& utime, long& stime) const = 0;

private:
  bool getWallClockTime(long& tsec, long& tusec) const;
};

// Timer which collects only elapsed times. This timer does not
//  collect CPU times. This is the default timer when running in
//  low-res mode, for all but the top-level metrics.
//
class ElapsedOnlyMetricsTimerImpl : public CommonTimerImplBase
{
protected:
  virtual bool getThreadCPUTime(long& utime, long& stime) const override { return true; }
};

// Low resolution metrics timer which uses the standard
//  library function times().
// This does not work properly on RedHat4 and above.
//
class StdLibMetricsTimerImpl : public CommonTimerImplBase
{
protected:
  virtual bool getThreadCPUTime(long& utime, long& stime) const override
  {
    return getThreadCPUTimeFromStdLib(utime, stime);
  }
};

// Low resolution (10ms) metrics timer which uses
//  the /proc filesystem to collect CPU utilization times.
//
class ProcMetricsTimerImpl : public CommonTimerImplBase
{
protected:
  virtual bool getThreadCPUTime(long& utime, long& stime) const override
  {
    return getThreadCPUTimeFromProc(utime, stime);
  }
};

// High resolution metrics timer which uses the HRT API (clock_*)
class HRTMetricsTimerImpl : public CommonTimerImplBase
{
protected:
  virtual bool getThreadCPUTime(long& utime, long& stime) const override
  {
    return getThreadCPUTimeFromHRT(utime, stime);
  }
};

double
MetricsTimer::Data::getElapsedTime() const
{
  long endUsec = _elapsedTimeEndUsec;
  long endSec = _elapsedTimeEndSec;
  if (endUsec < _elapsedTimeStartUsec)
  {
    endSec -= 1;
    endUsec += 1000000;
  }

  return (double(endSec - _elapsedTimeStartSec) +
          (double(endUsec - _elapsedTimeStartUsec) / double(1000000)));
}

double
MetricsTimer::Data::getCpuUserTime() const
{
  double result(0);
  if (LIKELY(_cpuValid))
  {
    result = double(_cpuUserTimeEnd - _cpuUserTimeStart) * MetricsUtil::getCPUTimeResolution();
  }
  return result;
}

double
MetricsTimer::Data::getCpuSystemTime() const
{
  double result(0);
  if (LIKELY(_cpuValid))
  {
    result = double(_cpuSystemTimeEnd - _cpuSystemTimeStart) * MetricsUtil::getCPUTimeResolution();
  }
  return result;
}

void
CommonTimerImplBase::start(MetricsTimer::Data& data) const
{
  getWallClockTime(data._elapsedTimeStartSec, data._elapsedTimeStartUsec);
  bool valid(getThreadCPUTime(data._cpuUserTimeStart, data._cpuSystemTimeStart));
  data.setCpuValid(valid);
}

void
CommonTimerImplBase::stop(MetricsTimer::Data& data) const
{
  getWallClockTime(data._elapsedTimeEndSec, data._elapsedTimeEndUsec);
  if (data.isCpuValid())
  {
    bool valid(getThreadCPUTime(data._cpuUserTimeEnd, data._cpuSystemTimeEnd));
    valid = valid && data._cpuUserTimeEnd >= data._cpuUserTimeStart &&
            data._cpuSystemTimeEnd >= data._cpuSystemTimeStart;
    data.setCpuValid(valid);
  }
}

bool
CommonTimerImplBase::getWallClockTime(long& tsec, long& tusec) const
{
  struct timeval tvData;
  gettimeofday(&tvData, nullptr);

  tsec = tvData.tv_sec;
  tusec = tvData.tv_usec;
  return true;
}

MetricsTimer::MetricsTimer() : _timerImpl(MetricsUtil::getTimerImpl())
{
}

MetricsTimer::MetricsTimer(bool isTopLevel, bool recordMetrics, bool recordTopLevelOnly)
  : _timerImpl(MetricsUtil::getTimerImpl(isTopLevel, recordMetrics, recordTopLevelOnly))
{
}

double
MetricsTimer::getElapsedTime()
{
  if (UNLIKELY(!_endDataIsValid))
    stop();
  return _data.getElapsedTime();
}

double
MetricsTimer::getCpuUserTime()
{
  double result(0);
  if (_data.isCpuValid() && _endDataIsValid)
  {
    result = _data.getCpuUserTime();
  }
  return result;
}

double
MetricsTimer::getCpuSystemTime()
{
  double result(0);
  if (_data.isCpuValid() && _endDataIsValid)
  {
    result = _data.getCpuSystemTime();
  }
  return result;
}

void
MetricsTimer::start()
{
  _timerImpl.start(_data);
}

void
MetricsTimer::stop()
{
  _timerImpl.stop(_data);
  _endDataIsValid = true;
}

bool
MetricsTimer::enabled() const
{
  return _timerImpl.enabled();
}

bool
MetricsUtil::header(std::ostream& os, const std::string& title)
{
  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::endl;
  os << "**************************************************" << std::endl;
  os << "* " << std::setw(46) << title << " *" << std::endl;
  os << "**************************************************" << std::endl;
  return true;
}

bool
MetricsUtil::lineItemHeader(std::ostream& os)
{
  os.setf(std::ios::right, std::ios::adjustfield);

  os << std::setw(int(NUM_WIDTH)) << "#" << ITEM_SEPARATOR << std::setw(int(DESC_WIDTH)) << "DESC"
     << ITEM_SEPARATOR << std::setw(int(ITEM_WIDTH)) << "MIN" << ITEM_SEPARATOR
     << std::setw(int(ITEM_WIDTH)) << "MAX" << ITEM_SEPARATOR << std::setw(int(ITEM_WIDTH))
     << "MEAN" << std::endl;

  return true;
}

bool
MetricsUtil::lineItem(std::ostream& os, const int factor)
{
  if (factor < 0 || factor >= MAX_METRICS_ENUM)
    return false;

  Gauss<double> rsp;
  Global::metrics().select(factor, rsp);

  os.precision(ITEM_DECIMALS);
  os.setf(std::ios::right, std::ios::adjustfield);
  os.setf(std::ios::fixed, std::ios::floatfield);

  os << std::setw(int(NUM_WIDTH)) << factor << ITEM_SEPARATOR << std::setw(int(DESC_WIDTH))
     << MetricsUtil::factorDesc(factor) << ITEM_SEPARATOR << std::setw(int(ITEM_WIDTH)) << rsp.min()
     << ITEM_SEPARATOR << std::setw(int(ITEM_WIDTH)) << rsp.max() << ITEM_SEPARATOR
     << std::setw(int(ITEM_WIDTH)) << rsp.mean() << std::endl;

  return true;
}

namespace
{
// class to represent a call tree, which holds metrics information for
// each node in the tree.
class CallTree
{
public:
  CallTree(const char* call = nullptr,
           double wall = 0.0,
           double user = 0.0,
           double system = 0.0,
           double childUser = 0.0,
           double childSystem = 0.0,
           size_t ncalls = 0,
           size_t startMem = 0,
           size_t endMem = 0,
           size_t thread = 0)
    : _call(call),
      _wall(wall),
      _user(user),
      _system(system),
      _childUser(childUser),
      _childSystem(childSystem),
      _ncalls(ncalls),
      _startMem(startMem),
      _endMem(endMem),
      _thread(thread)
  {
  }

  // main function to parse accumulated data. Recursively divides the
  // range into sub-ranges. The item at the front of the range is
  // the first item in this range. Its 'nItems' element specifies
  // the number of child items it has, and its next sibling
  // can be found after its children. Thus each call to this function
  // will add a child, and then spawn two sub-calls: one to add
  // the child's siblings, and another to add the child's children.
  template <typename RanIt>
  void addChild(RanIt beg, RanIt end)
  {
    while (beg != end)
    {
      const Trx::Latency& latency = *beg;
      std::vector<CallTree>::iterator i;
      for (i = _children.begin(); i != _children.end(); ++i)
      {
        if (i->_call == latency.description)
        {
          break;
        }
      }

      if (i == _children.end())
      {
        _children.push_back(CallTree(latency.description,
                                     latency.wallTime,
                                     latency.userTime,
                                     latency.systemTime,
                                     latency.childUserTime,
                                     latency.childSystemTime,
                                     latency.nRepeats,
                                     latency.startMem,
                                     latency.endMem,
                                     latency.thread));
        i = _children.end() - 1;
      }
      else
      {
        i->_wall += latency.wallTime;
        i->_user += latency.userTime;
        i->_system += latency.systemTime;
        i->_childUser += latency.childUserTime;
        i->_childSystem += latency.childSystemTime;
        i->_ncalls += latency.nRepeats;
        i->_startMem = std::min(i->_startMem, latency.startMem);
        i->_endMem = std::max(i->_endMem, latency.endMem);
      }

      ++beg;
      TSE_ASSERT(end - beg >= 0);
      if (UNLIKELY(size_t(end - beg) < latency.nItems))
      {
        break;
      }

      i->addChild(beg, beg + latency.nItems); // parse children
      beg += latency.nItems;
    }
  }

  void output(std::ostream& oss, bool showSystem, uint32_t indent = 0) const;
  void outputMemory(std::ostream& oss, uint32_t indent = 0) const;
  void outputItem(std::ostream& oss, bool showSystem, uint32_t indent = 0) const;
  void outputMemoryItem(std::ostream& oss, uint32_t indent = 0) const;

  void addToCallMap(std::map<std::string, CallTree>& m) const;

  void recordMetrics(MetricsMan& metricsMan, std::map<const char*, int>& factors) const;

  bool operator<(const CallTree& o) const { return _wall > o._wall; }

  const char* call() const { return _call; }
  std::size_t thread() const { return _thread; }
  const std::vector<CallTree>& children() const { return _children; }
  void addToChildRootUserTime(double delta) { _children.back()._user += delta; }
  void addToChildRootSystemTime(double delta) { _children.back()._system += delta; }

  size_t memoryUsage() const { return _endMem / (1024 * 1024) - _startMem / (1024 * 1024); }

  void childStats(double& wall, double& cpu)
  {
    wall = 0;
    cpu = 0;

    std::vector<CallTree>::iterator i = _children.begin();
    std::vector<CallTree>::iterator j = _children.end();

    for (; i != j; ++i)
    {
      CallTree& ct = *i;

      wall += ct._wall;
      cpu += ct._system + ct._user;
    }
  }

private:
  const char* _call;
  double _wall, _user, _system, _childUser, _childSystem;
  size_t _ncalls;
  std::size_t _startMem, _endMem;
  std::size_t _thread;
  std::vector<CallTree> _children;
};

class MemPredicate : public std::binary_function<CallTree, CallTree, bool>
{
public:
  bool operator()(const CallTree& ct1, const CallTree& ct2)
  {
    return ct1.memoryUsage() > ct2.memoryUsage();
  }
}; // lint !e1509

const std::size_t FIELD_WIDTH = 36;
const std::size_t DECIMAL_PLACES = 2;
const std::size_t DECIMAL_FIELD = DECIMAL_PLACES + 5;

void
CallTree::outputItem(std::ostream& oss, bool showSystem, uint32_t indent) const
{
  std::string indentation(indent, ' ');
  for (size_t n = 0; n < indentation.size(); n += 2)
  {
    indentation[n] = '-';
  }

  std::string item = indentation + _call;
  if (item.size() >= FIELD_WIDTH)
  {
    item.resize(FIELD_WIDTH - 1);
  }

  oss.setf(std::ios::left, std::ios::adjustfield);
  oss << std::setw(int(FIELD_WIDTH)) << item;
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss.setf(std::ios::fixed, std::ios::floatfield);
  oss.precision(MetricsUtil::getElapsedTimeDecimalPlaces());
  oss << std::setw(int(MetricsUtil::getElapsedTimeFieldWidth())) << _wall;
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss.setf(std::ios::fixed, std::ios::floatfield);
  oss.precision(MetricsUtil::getCPUTimeDecimalPlaces());
  oss << std::setw(int(MetricsUtil::getCPUTimeFieldWidth())) << (_system + _user);

  if (UNLIKELY(showSystem))
  {
    oss.setf(std::ios::right, std::ios::adjustfield);
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss.precision(MetricsUtil::getCPUTimeDecimalPlaces());
    oss << std::setw(int(MetricsUtil::getCPUTimeFieldWidth())) << _user;
    oss.setf(std::ios::right, std::ios::adjustfield);
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss.precision(MetricsUtil::getCPUTimeDecimalPlaces());
    oss << std::setw(int(MetricsUtil::getCPUTimeFieldWidth())) << _system;
  }

  oss.setf(std::ios::right, std::ios::adjustfield);
  oss << std::setw(int(DECIMAL_FIELD)) << _ncalls << "\n";
}

void
CallTree::outputMemoryItem(std::ostream& oss, uint32_t indent) const
{
  std::string indentation(indent, ' ');
  for (size_t n = 0; n < indentation.size(); n += 2)
  {
    indentation[n] = '-';
  }

  std::string item = indentation + _call;
  if (item.size() >= FIELD_WIDTH)
  {
    item.resize(FIELD_WIDTH - 1);
  }

  size_t startMemMB = _startMem / (1024 * 1024);
  size_t endMemMB = _endMem / (1024 * 1024);

  oss.setf(std::ios::left, std::ios::adjustfield);
  oss << std::setw(int(FIELD_WIDTH)) << item;
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss << std::setw(int(DECIMAL_FIELD)) << startMemMB;
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss << std::setw(int(DECIMAL_FIELD)) << endMemMB;
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss << std::setw(int(DECIMAL_FIELD)) << memoryUsage();
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss << std::setw(int(DECIMAL_FIELD)) << _ncalls;
  oss << "\n";
}

void
CallTree::output(std::ostream& oss, bool showSystem, uint32_t indent) const
{
  if (_call != nullptr)
  {
    outputItem(oss, showSystem, indent);
    ++indent;
  }

  for (std::vector<CallTree>::const_reverse_iterator i = _children.rbegin(); i != _children.rend();
       ++i)
  {
    i->output(oss, showSystem, indent);
  }
}

void
CallTree::outputMemory(std::ostream& oss, uint32_t indent) const
{
  if (_call != nullptr)
  {
    outputMemoryItem(oss, indent);
    ++indent;
  }

  for (std::vector<CallTree>::const_reverse_iterator i = _children.rbegin(); i != _children.rend();
       ++i)
  {
    i->outputMemory(oss, indent);
  }
}

void
CallTree::addToCallMap(std::map<std::string, CallTree>& m) const
{
  if (_call != nullptr)
  {
    const std::string key(_call);
    const std::map<std::string, CallTree>::iterator i = m.find(key);
    if (i != m.end())
    {
      i->second._wall += _wall;
      i->second._user += _user;
      i->second._system += _system;
      i->second._childUser += _childUser;
      i->second._childSystem += _childSystem;
      i->second._ncalls += _ncalls;
      i->second._thread = _thread;
    }
    else
    {
      m.insert(std::make_pair(key, *this));
    }
  }

  for (const auto& elem : _children)
  {
    elem.addToCallMap(m);
  }
}

void
CallTree::recordMetrics(MetricsMan& metricsMan, std::map<const char*, int>& factors) const
{
  if (LIKELY(_ncalls > 0))
  {
    std::map<const char*, int>::const_iterator i = factors.find(_call);
    if (LIKELY(i == factors.end()))
    {
      int n;
      for (n = 0; n != MetricsUtil::MAX_METRICS_ENUM; ++n)
      {
        if (UNLIKELY(MetricsUtil::factorDesc(n).c_str() == _call))
        {
          break;
        }
      }

      i = factors.insert(std::make_pair(_call, n)).first;
    }

    if (UNLIKELY(i->second >= 0 && i->second < MetricsUtil::MAX_METRICS_ENUM))
    {
      for (size_t n = 0; n != _ncalls; ++n)
      {
        metricsMan.update(i->second, _wall / double(_ncalls));
      }
    }
  }
}

void
outputMetricsHeading(std::ostream& os, bool showSystemTime)
{
  const std::string headings[6] = {"CALL", "TIME", "CPU", "USER", "SYS", "CALLS"};

  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::setw(int(FIELD_WIDTH)) << headings[0];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(MetricsUtil::getElapsedTimeFieldWidth())) << headings[1];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(MetricsUtil::getCPUTimeFieldWidth())) << headings[2];

  if (UNLIKELY(showSystemTime))
  {
    os.setf(std::ios::right, std::ios::adjustfield);
    os << std::setw(int(MetricsUtil::getCPUTimeFieldWidth())) << headings[3];
    os.setf(std::ios::right, std::ios::adjustfield);
    os << std::setw(int(MetricsUtil::getCPUTimeFieldWidth())) << headings[4];
  }

  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[5] << "\n";

  for (size_t n = 0; n !=
                         MetricsUtil::getElapsedTimeFieldWidth() +
                             MetricsUtil::getCPUTimeFieldWidth() + DECIMAL_FIELD + FIELD_WIDTH;
       ++n)
  {
    os << '-';
  }

  os << "\n";
}

void
outputMemoryHeading(std::ostream& os)
{
  const std::string headings[5] = {"CALL", "BEFORE", "AFTER", "DIFF", "CALLS"};

  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::setw(int(FIELD_WIDTH)) << headings[0];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[1];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[2];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[3];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[4] << "\n";

  for (size_t n = 0; n != DECIMAL_FIELD * 4 + FIELD_WIDTH; ++n)
  {
    os << '-';
  }

  os << "\n";
}

void
outputAllocationHeading(std::ostream& os)
{
  os << "\n\nOBJECT ALLOCATIONS\n\n";

  const std::string headings[4] = {"OBJECT", "NUM", "SIZE", "MEM"};

  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::setw(int(FIELD_WIDTH)) << headings[0];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[1];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[2];
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << headings[3] << "\n";

  for (size_t n = 0; n != DECIMAL_FIELD * 3 + FIELD_WIDTH; ++n)
  {
    os << '-';
  }

  os << "\n";
}

struct ObjectAllocation
{
  ObjectAllocation(const char* name, size_t size, size_t number)
    : name(name), size(size), number(number)
  {
  }

  bool operator<(const ObjectAllocation& o) const { return consumption() > o.consumption(); }

  std::size_t consumption() const { return size * number; }

  const char* name;
  std::size_t size, number;
};

void
outputObjectAllocation(std::ostream& os, const ObjectAllocation& alloc)
{
  std::string name = alloc.name; // lint !e530
  if (name.size() >= FIELD_WIDTH)
  {
    name.resize(FIELD_WIDTH - 1);
  }

  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::setw(int(FIELD_WIDTH)) << name;
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << alloc.number;
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(int(DECIMAL_FIELD)) << alloc.size;
  os.setf(std::ios::right, std::ios::adjustfield);

  std::ostringstream consumption;
  size_t iconsume = alloc.consumption() / 1024;
  const char* append = "KB";
  if (iconsume >= 1024 * 10)
  {
    iconsume /= 1024;
    append = "MB";
  }
  consumption << iconsume << append;
  os << std::setw(int(DECIMAL_FIELD)) << consumption.str();
  os << "\n";
}
} // unnamed namespace

bool
MetricsUtil::trxLatency(std::ostream& os, PricingTrx& trx, double* totalElapsed, double* totalCPU)
{
  const Trx& baseTrx = trx;
  const bool result = trxLatency(os, baseTrx, totalElapsed, totalCPU);

  size_t numTvlSegs = 0, numFM = 0, numPax = 0;
  size_t numPTF = 0;

  getPricingTrxDetail(trx, numTvlSegs, numFM, numPax, numPTF);
  dumpPricingTrxDetail(os, numTvlSegs, numFM, numPax, numPTF);

  dumpHeapMemDetail(os, trx);

  return result;
}

void
MetricsUtil::dumpHeapMemDetail(std::ostream& os, const Trx& trx)
{
  size_t mem(0);
  if (!Memory::changesFallback)
  {
    mem = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize();
  }
  else
  {
    mem = MemoryUsage::getVirtualMemorySize();
  }
  os << std::setw(int(DESC_WIDTH)) << "VIRTUAL MEM SIZE: " << std::setw(int(ITEM_WIDTH))
     << mem / (1024 * 1024) << " MB" << std::endl;
  char allocStats[1000];
  if (trx_malloc_get_stats(allocStats, sizeof(allocStats)) > 0)
  {
    os << allocStats;
  }
}

bool
MetricsUtil::trxLatency(std::ostream& os,
                        const MultiExchangeTrx& meTrx,
                        double* totalElapsed,
                        double* totalCPU)
{
  const Trx& baseTrx = meTrx;
  const bool result = trxLatency(os, baseTrx, totalElapsed, totalCPU);

  // dump some 'important vector size info for three PricingTrx
  size_t numTvlSegsNew = 0, numTvlSegsExc1 = 0, numTvlSegsExc2 = 0;
  size_t numFMNew = 0, numFMExc1 = 0, numFMExc2 = 0;
  size_t numPaxNew = 0, numPaxExc1 = 0, numPaxExc2 = 0;
  size_t numPTFNew = 0, numPTFExc1 = 0, numPTFExc2 = 0;

  if (!meTrx.skipNewPricingTrx())
    getPricingTrxDetail(*meTrx.newPricingTrx(), numTvlSegsNew, numFMNew, numPaxNew, numPTFNew);

  if (!meTrx.skipExcPricingTrx1())
    getPricingTrxDetail(*meTrx.excPricingTrx1(), numTvlSegsExc1, numFMExc1, numPaxExc1, numPTFExc1);

  if (!meTrx.skipExcPricingTrx2())
    getPricingTrxDetail(*meTrx.excPricingTrx2(), numTvlSegsExc2, numFMExc2, numPaxExc2, numPTFExc2);

  const size_t numTvlSegs = numTvlSegsNew + numTvlSegsExc1 + numTvlSegsExc2;
  const size_t numFM = numFMNew + numFMExc1 + numFMExc2;
  const size_t numPax = numPaxNew + numPaxExc1 + numPaxExc2;
  const size_t numPTF = numPTFNew + numPTFExc1 + numPTFExc2;

  dumpPricingTrxDetail(os, numTvlSegs, numFM, numPax, numPTF);
  dumpHeapMemDetail(os, meTrx);

  return result;
}

void
MetricsUtil::getPricingTrxDetail(
    PricingTrx& trx, size_t& numTvlSegs, size_t& numFM, size_t& numPax, size_t& numPTF)
{
  numTvlSegs = trx.travelSeg().size();
  numFM = getNumFM(trx);
  numPax = trx.paxType().size();

  numPTF = 0;

  const std::vector<FareMarket*>& fareMarkets = trx.fareMarket();
  std::vector<FareMarket*>::const_iterator k = fareMarkets.begin();
  const std::vector<FareMarket*>::const_iterator l = fareMarkets.end();

  // For performance purposes we're not adding fares to allPaxTypeFare vector
  // for ESV transaction, so print number of fares from paxTypeCortege.
  if (PricingTrx::ESV_TRX == trx.getTrxType())
  {
    for (; k != l; k++)
    {
      if (!(*k)->paxTypeCortege().empty())
      {
        numPTF += (*k)->paxTypeCortege().at(0).paxTypeFare().size();
      }
    }
  }
  else
  {
    for (; k != l; k++)
    {
      numPTF += (*k)->allPaxTypeFare().size();
    }
  }
}

size_t
MetricsUtil::getNumFM(PricingTrx& trx)
{
  size_t numFM = 0;

  if (trx.getTrxType() == PricingTrx::IS_TRX)
  {
    ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(trx);
    if (shoppingTrx.isSumOfLocalsProcessingEnabled())
    {
      numFM = trx.fareMarket().size();
    }
    else
    {
      std::vector<ShoppingTrx::Leg>::const_iterator sGLIter = shoppingTrx.legs().begin();
      std::vector<ShoppingTrx::Leg>::const_iterator sGLEndIter = shoppingTrx.legs().end();
      for (; sGLIter != sGLEndIter; ++sGLIter)
      {
        numFM += (*sGLIter).carrierIndex().root().size();
      }
    }
  }
  else
  {
    numFM = trx.fareMarket().size();
  }

  return numFM;
}

void
MetricsUtil::dumpPricingTrxDetail(
    std::ostream& os, size_t numTvlSegs, size_t numFM, size_t numPax, size_t numPTF)
{
  // dump some 'important vector size info
  os << std::endl;
  os << std::setw(int(DESC_WIDTH)) << "TRAVEL SEGMENTS: " << std::setw(int(ITEM_WIDTH))
     << numTvlSegs << std::endl;
  os << std::setw(int(DESC_WIDTH)) << "FARE MARKETS: " << std::setw(int(ITEM_WIDTH)) << numFM
     << std::endl;
  os << std::setw(int(DESC_WIDTH)) << "PAX TYPES: " << std::setw(int(ITEM_WIDTH)) << numPax
     << std::endl;
  os << std::setw(int(DESC_WIDTH)) << "PAXTYPEFARES: " << std::setw(int(ITEM_WIDTH)) << numPTF
     << std::endl;
}

bool
MetricsUtil::trxLatency(std::ostream& os, const Trx& trx, double* totalElapsed, double* totalCPU)
{
  typedef std::map<size_t, std::deque<Trx::Latency>> LatencyMap;
  LatencyMap latencyDataThread;
  for (const auto& elem : trx.latencyDataThread())
  {
    for (LatencyMap::const_iterator j = elem.begin(); j != elem.end(); ++j)
    {
      latencyDataThread.insert(*j);
    }
  }

  std::deque<CallTree> tree(1 + latencyDataThread.size());

  // lint -e{1561}
  tree.front().addChild(trx.latencyData().begin(), trx.latencyData().end());

  // variable to hold the accumulated cpu time of child tasks that execute in
  // threads other than the root transaction thread.
  double childTaskUserTime = 0, childTaskSystemTime = 0;
  size_t index = 1;
  // lint -e{530}
  LatencyMap::const_iterator i = latencyDataThread.begin();
  for (; i != latencyDataThread.end(); ++i)
  {
    tree[index].addChild(i->second.begin(), i->second.end());
    for (const auto& elem : i->second)
    {
      if (elem.toplevelTaskData)
      {
        childTaskUserTime += elem.userTime;
        childTaskSystemTime += elem.systemTime;
      }
    }
    ++index;
  }

  if (tree.size() > 1)
  {
    // Add the child thread cpu time to the very toplevel cpu time.
    tree.front().addToChildRootUserTime(childTaskUserTime);
    tree.front().addToChildRootSystemTime(childTaskSystemTime);
  }

  typedef std::vector<CallTree> CallList;
  CallList calls;

  std::map<std::string, CallTree> callMap;
  for (size_t n = 0; n != tree.size(); ++n)
  {
    tree[n].addToCallMap(callMap);
  }

  for (std::map<std::string, CallTree>::const_iterator i = callMap.begin(); i != callMap.end(); ++i)
  {
    calls.push_back(i->second);
  }

  if (trx.recordCPUTime())
  {
    const bool showSystemTime = showSystem.getValue();
    os << "\n**********************************************************"
       << "\n*                  TRANSACTION MEASUREMENTS              *"
       << "\n**********************************************************\n";

    for (std::deque<CallTree>::iterator j = tree.begin(); j != tree.end(); ++j)
    {
      const std::size_t index = j - tree.begin();

      if (index != 0)
      {
        os << "WORKER THREAD " << index << " (ID: " << j->children().back().thread() << ")"
           << "\n";
      }

      outputMetricsHeading(os, showSystemTime);
      j->output(os, showSystemTime);

      os << "\n\n";
    }

    os << "\nLINE ITEMS\n\n";

    std::sort(calls.begin(), calls.end());

    outputMetricsHeading(os, showSystemTime);

    for (std::vector<CallTree>::const_iterator i = calls.begin(); i != calls.end(); ++i)
    {
      i->outputItem(os, showSystemTime);
    }

    std::map<const char*, int> factors;

    MetricsMan& metricsMan = Global::metrics();
    for (std::vector<CallTree>::const_iterator i = calls.begin(); i != calls.end(); ++i)
    {
      i->recordMetrics(metricsMan, factors);
    }

    os << "\n\n";
  }

  if (trx.recordMemory())
  {
    os << "*****************************************************************\n"
       << "*                      MEMORY MEASUREMENTS                      *\n"
       << "*****************************************************************\n\n";

    outputMemoryHeading(os);
    tree.front().outputMemory(os);

    // now output a record of all objects allocated in the DataHandle
    outputAllocationHeading(os);

    std::map<const char*, DeleteList::Entry> items;
    trx.dataHandle().deleteList().collectMetrics(items);

    std::vector<ObjectAllocation> v;
    v.reserve(items.size());

    for (std::map<const char*, DeleteList::Entry>::const_iterator i = items.begin();
         i != items.end();
         ++i)
    {
      v.push_back(ObjectAllocation(i->first, i->second.size, i->second.nheap + i->second.npool));
    }

    std::sort(v.begin(), v.end());

    for (std::vector<ObjectAllocation>::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      outputObjectAllocation(os, *i);
    }

    os << "\n\n";
  }

  if (tree.size() > 0 && ((totalElapsed != nullptr) || totalCPU != nullptr))
  {
    CallTree& ct = tree.front();

    double wall = 0;
    double cpu = 0;

    ct.childStats(wall, cpu);

    if (totalElapsed != nullptr)
      *totalElapsed = wall;

    if (totalCPU != nullptr)
      *totalCPU = cpu;
  }

  return true;
}

const std::string&
MetricsUtil::factorDesc(const int factor)
{
  std::tr1::unordered_map<int, std::string>::const_iterator it = _factorDescs.find(factor);
  if (LIKELY(it != _factorDescs.end()))
    return it->second;

  return _emptyString;
}

const MetricsTimerImpl&
MetricsUtil::getTimerImpl()
{
  if (!_timerImplsLoaded)
    loadTimerImpls();
  return *_defaultTopLevelTimerImpl;
}

const MetricsTimerImpl&
MetricsUtil::getTimerImpl(bool isTopLevel, bool recordMetrics, bool recordTopLevelOnly)
{
  if (!_timerImplsLoaded)
    loadTimerImpls();

  if (UNLIKELY(!recordMetrics))
    return *_doNothingTimerImpl;

  if (isTopLevel)
    return *_defaultTopLevelTimerImpl;

  if (UNLIKELY(_forceTopLevelOnly))
    return *_doNothingTimerImpl;

  if (recordTopLevelOnly && !_forceLowLevel)
    return *_doNothingTimerImpl;

  return *_defaultTimerImpl;
}

void
MetricsUtil::loadTimerImpls()
{
  if (!_timerImplsLoaded)
  {
    // Get the mutex
    boost::lock_guard<boost::mutex> g(_loadTimerImplsMutex);

    if (!_timerImplsLoaded)
    {
      _doNothingTimerImpl = new MetricsTimerImpl;

      if (useHiResTimer.getValue())
      {
        ELAPSED_TIME_RESOLUTION = HIRES_ELAPSED_TIME_RESOLUTION;
        CPU_TIME_RESOLUTION = HIRES_CPU_TIME_RESOLUTION;
        ELAPSED_TIME_DECIMAL_PLACES = HIRES_ELAPSED_TIME_DECIMAL_PLACES;
        CPU_TIME_DECIMAL_PLACES = HIRES_CPU_TIME_DECIMAL_PLACES;

        _getThreadCPUTimeFunc = &getThreadCPUTimeFromHRT;

        _defaultTopLevelTimerImpl = new HRTMetricsTimerImpl;
        _defaultTimerImpl = _defaultTopLevelTimerImpl;
      }
      else
      {
        ELAPSED_TIME_RESOLUTION = LORES_ELAPSED_TIME_RESOLUTION;
        CPU_TIME_RESOLUTION = LORES_CPU_TIME_RESOLUTION;
        ELAPSED_TIME_DECIMAL_PLACES = LORES_ELAPSED_TIME_DECIMAL_PLACES;
        CPU_TIME_DECIMAL_PLACES = LORES_CPU_TIME_DECIMAL_PLACES;

        _getThreadCPUTimeFunc = &getThreadCPUTimeFromProc;

        _defaultTopLevelTimerImpl = new ProcMetricsTimerImpl;

        if (!collectCpuTopLevelOnly.getValue())
        {
          _defaultTimerImpl = _defaultTopLevelTimerImpl;
        }
        else
        {
          _defaultTimerImpl = new tse::ElapsedOnlyMetricsTimerImpl;
        }
      }
      _forceTopLevelOnly = forceTopLevelOnly.getValue();
      _forceLowLevel = forceLowLevel.getValue();
      _timerImplsLoaded = true;
    }
  }
}
} // namespace tse
