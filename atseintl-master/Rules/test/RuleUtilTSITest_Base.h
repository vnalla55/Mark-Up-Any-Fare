#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <time.h>
#include <vector>
#include <set>
#include <stdarg.h>
#include <boost/assign/std/vector.hpp>

#include "Rules/RuleUtilTSI.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "test/testdata/TestLocFactory.h"
#include "DBAccess/TSIInfo.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace boost::assign;

namespace tse
{

class RuleUtilTSITest_Base;

// grant acces to the testing classes thrue the Mock
class RuleUtilTSIMock : public RuleUtilTSI
{
  friend class RuleUtilTSITest_Base;
  friend class RuleUtilTSITest_GetLocationMatchings;
  friend class RuleUtilTSITest_CheckGeoData;
  friend class RuleUtilTSITest_CheckGeoNotType;
  friend class RuleUtilTSITest_GetLoopItemToSetLoopSet;
  friend class RuleUtilTSITest_CheckLoopMatch;
  friend class RuleUtilTSITest_CheckMatchCriteria;
  friend class RuleUtilTSITest_CheckTSIMatchTravelSeg;
  friend class RuleUtilTSITest_GetGeoLocaleFromItin;
  friend class RuleUtilTSITest_GetGeoData;
  friend class RuleUtilTSITest_ReverseTravel;
  friend class RuleUtilTSITest_GetTSIOrigDestCheck;
  friend class RuleUtilTSITest_IdentifyIntlDomTransfers;
  friend class RuleUtilTSITest_ProcessTravelSegs;
  friend class RuleUtilTSITest_ScopeTSIGeo;
  friend class RuleUtilTSITest_WriteTSIDiag;
  friend class RuleUtilTSITest_TSISetupTurnAround;
  friend class RuleUtilTSITest_TSISetupGateway;
  friend class RuleUtilTSITest_TSISetupOverWater;
  friend class RuleUtilTSITest_TSISetupInternational;
  friend class RuleUtilTSITest_TSISetupChain;
  friend class RuleUtilTSITest_SetupTravelSegMarkup;
};

class RuleUtilTSITest_Base : public CppUnit::TestFixture
{
public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _options = _memHandle.create<PricingOptions>();
    _trx->setRequest(_request);
    _trx->setOptions(_options);
    _locSFO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _locSYD = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _segSFOSYD = _memHandle.create<AirSeg>();
    _segSFOSYD->origin() = _locSFO;
    _segSFOSYD->destination() = _locSYD;
  }
  void tearDown() { _memHandle.clear(); }
  // helper function
  RuleUtilTSI::TSIData* initTSIData(LocTypeCode lt1 = ' ',
                                    LocCode lc1 = "",
                                    LocTypeCode lt2 = ' ',
                                    LocCode lc2 = "",
                                    char geoCheck = 'O',
                                    char geoOut = ' ',
                                    char geoNotType = ' ')
  {
    TSIInfo* tsiInfo = _memHandle.create<TSIInfo>();
    RuleConst::TSIScopeType* tsiScope = _memHandle.create<RuleConst::TSIScopeType>();
    VendorCode* vendor = _memHandle.create<VendorCode>();

    tsiInfo->geoOut() = geoOut;
    tsiInfo->geoCheck() = geoCheck;
    tsiInfo->geoNotType() = geoNotType;
    *tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    *vendor = "ATP";
    RuleUtilTSI::TSIData* tsiData =
        _memHandle.insert(new RuleUtilTSI::TSIData(*tsiInfo, *tsiScope, *vendor, 0, 0, 0, 0));
    tsiData->locType1() = lt1;
    tsiData->locType2() = lt2;
    tsiData->locCode1() = lc1;
    tsiData->locCode2() = lc2;
    return tsiData;
  }

protected:
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  Loc* _locSFO;
  Loc* _locSYD;
  AirSeg* _segSFOSYD;

  TestMemHandle _memHandle;
};
}
