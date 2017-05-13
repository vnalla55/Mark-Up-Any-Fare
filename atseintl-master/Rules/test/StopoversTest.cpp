#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/VCTR.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurfaceSeg.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/StopoversInfoSeg.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/CategoryRuleItem.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleSetPreprocessor.h"
#include "Rules/Stopovers.h"
#include "Rules/StopoversInfoWrapper.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestCarrierPreferenceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestFallbackUtil.h"

#include <iostream>
#include <set>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/tokenizer.hpp>
#include <time.h>

using namespace std;
using namespace boost::assign;

namespace tse
{
FALLBACKVALUE_DECL(fallbackAPO37838Record1EffDateCheck);

using namespace boost::assign;

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  NUCInfo* getNuc(double nucFactor,
                  double roundingFactor,
                  int nucFactorNodec,
                  int roundingFactorNodec,
                  RoundingRule rule)
  {
    NUCInfo* ret = _memHandle.create<NUCInfo>();
    ret->_nucFactor = nucFactor;
    ret->_roundingFactor = roundingFactor;
    ret->_nucFactorNodec = nucFactorNodec;
    ret->_roundingFactorNodec = roundingFactorNodec;
    ret->_roundingRule = rule;
    return ret;
  }
  GeoRuleItem* getGeo(int tsi, LocTypeCode lt, LocCode lc)
  {
    GeoRuleItem* ret = _memHandle.create<GeoRuleItem>();
    ret->tsi() = tsi;
    ret->loc1().locType() = lt;
    ret->loc1().loc() = lc;
    return ret;
  }
  MultiAirportCity* getMC(LocCode city, LocCode loc)
  {
    MultiAirportCity* ret = _memHandle.create<MultiAirportCity>();
    ret->city() = city;
    ret->airportCode() = loc;
    return ret;
  }
  TSIInfo* getTsi(int tsi,
                  char geoCheck,
                  char loopDirection,
                  char loopMatch,
                  char scope,
                  char type,
                  char mc = '*')
  {
    TSIInfo* ret = _memHandle.create<TSIInfo>();
    ret->tsi() = tsi;
    ret->geoRequired() = ' ';
    ret->geoNotType() = ' ';
    ret->geoOut() = ' ';
    ret->geoItinPart() = ' ';
    ret->geoCheck() = geoCheck;
    ret->loopDirection() = loopDirection;
    ret->loopOffset() = 0;
    ret->loopToSet() = 0;
    ret->loopMatch() = loopMatch;
    ret->scope() = scope;
    ret->type() = type;
    if (mc != '*')
      ret->matchCriteria().push_back((TSIInfo::TSIMatchCriteria)mc);
    return ret;
  }

public:
  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "FI")
      return TestCarrierPreferenceFactory::create(
          "/vobs/atseintl/test/testdata/data/CarrierPreference/CarrierPreference_BLANK.xml");
    return DataHandleMock::getCarrierPreference(carrier, date);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "EWR")
      return "EWR";
    if (locCode == "HNL")
      return "HNL";
    else if (locCode == "JFK")
      return "NYC";
    else if (locCode == "LGA")
      return "NYC";
    else if (locCode == "LGW")
      return "LON";
    else if (locCode == "MSP")
      return "MSP";
    else if (locCode == "REK")
      return "REK";
    else if (locCode == "SYD")
      return "SYD";
    else if (locCode == "MEL")
      return "MEL";
    else if (locCode == "SIN")
      return "SIN";
    else if (locCode == "OGG")
      return "OGG";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    if (currency == "USD")
      return getNuc(1, 1, 8, 0, NEAREST);
    else if (currency == "EUR")
      return getNuc(0.833831000, 1, 9, 0, UP);
    else if (currency == "GBP")
      return getNuc(0.68871800, 1, 8, 0, NEAREST);
    else if (currency == "KWD")
      return getNuc(0.29131500, 1, 8, 0, UP);

    return DataHandleMock::getNUCFirst(currency, carrier, date);
  }
  bool getGeneralRuleAppTariffRule(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& tariffNumber,
                                   const RuleNumber& ruleNumber,
                                   CatNumber catNum,
                                   RuleNumber& ruleNumOut,
                                   TariffNumber& tariffNumOut)
  {
    if (carrier == "" && ruleNumber == "0000" && catNum == 8)
      return false;
    return DataHandleMock::getGeneralRuleAppTariffRule(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut);
  }
  //msd
  bool getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& tariffNumber,
                                            const RuleNumber& ruleNumber,
                                            CatNumber catNum,
                                            RuleNumber& ruleNumOut,
                                            TariffNumber& tariffNumOut,
                                            const DateTime& tvlDate)
  {
    if (carrier == "" && ruleNumber == "0000" && catNum == 8)
      return false;
    return DataHandleMock::getGeneralRuleAppTariffRuleByTvlDate(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut, tvlDate);
  }
  //msd
  const std::vector<GeneralFareRuleInfo*>&
  getGeneralFareRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& rule,
                     const CatNumber& category,
                     const DateTime& date,
                     const DateTime& applDate = DateTime::emptyDate())
  {
    if (carrier == "")
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    return DataHandleMock::getGeneralFareRule(
        vendor, carrier, ruleTariff, rule, category, date, applDate);
  }
  const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemNumber)
  {
    std::vector<GeoRuleItem*>& ret = *_memHandle.create<std::vector<GeoRuleItem*> >();
    if (itemNumber == 3)
    {
      ret.push_back(getGeo(1, ' ', ""));
      return ret;
    }
    else if (itemNumber == 14)
    {
      ret.push_back(getGeo(7, ' ', ""));
      return ret;
    }
    else if (itemNumber == 78)
    {
      ret.push_back(getGeo(10, ' ', ""));
      return ret;
    }
    else if (itemNumber == 122)
    {
      ret.push_back(getGeo(0, 'N', "US"));
      return ret;
    }
    else if (itemNumber == 205)
    {
      ret.push_back(getGeo(31, ' ', ""));
      return ret;
    }
    else if (itemNumber == 437)
    {
      ret.push_back(getGeo(19, ' ', ""));
      return ret;
    }
    return DataHandleMock::getGeoRuleItem(vendor, itemNumber);
  }
  const TSIInfo* getTSI(int key)
  {
    if (key == 1)
      return getTsi(1, 'O', 'F', 'O', 'S', 'O');
    else if (key == 7)
      return getTsi(7, 'O', 'F', 'I', 'A', 'O', 'W');
    else if (key == 10)
      return getTsi(10, 'D', 'B', 'O', 'S', 'D');
    else if (key == 19)
      return getTsi(19, 'B', 'F', 'A', 'A', 'B', 'D');
    else if (key == 31)
      return getTsi(31, 'B', 'F', 'A', 'A', 'B', 'A');
    return DataHandleMock::getTSI(key);
  }
  const RuleItemInfo* getRuleItemInfo(const CategoryRuleInfo* rule,
                                      const CategoryRuleItemInfo* item,
                                      const DateTime& applDate = DateTime::emptyDate())
  {
    return 0;
  }
  const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
  {
    std::vector<MultiAirportCity*>& ret = *_memHandle.create<std::vector<MultiAirportCity*> >();
    if (city == "JFK")
    {
      ret.push_back(getMC("NYC", "JFK"));
      return ret;
    }
    else if (city == "DAL")
    {
      ret.push_back(getMC("DFW", "DAL"));
      return ret;
    }
    else if (city == "LGA")
    {
      ret.push_back(getMC("LON", "LGA"));
      return ret;
    }
    else if (city == "SIN")
    {
      ret.push_back(getMC("SIN", "SIN"));
      return ret;
    }
    else if (city == "MEL")
    {
      ret.push_back(getMC("MEL", "MEL"));
      return ret;
    }
    else if (city == "SYD")
    {
      ret.push_back(getMC("SYD", "SYD"));
      return ret;
    }
    else if (city == "OGG")
    {
      ret.push_back(getMC("OGG", "OGG"));
      return ret;
    }

    else if (city == "LON")
      return ret;

    return DataHandleMock::getMultiAirportCity(city);
  }
};
}

class StopoversTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(StopoversTest);
  CPPUNIT_TEST(testValidateFare0);
  CPPUNIT_TEST(testValidateFare1);
  CPPUNIT_TEST(testValidateFare2);
  CPPUNIT_TEST(testValidateFare3);
  CPPUNIT_TEST(testValidateFare4);
  CPPUNIT_TEST(testValidateFare5);
  CPPUNIT_TEST(testValidateFare6);
  CPPUNIT_TEST(testValidateFare7);
  CPPUNIT_TEST(testValidateFare8);
  CPPUNIT_TEST(testValidateFare9);
  CPPUNIT_TEST(testMaxStopoverTimeExceed_FareFail);

  CPPUNIT_TEST(testValidatePU0);
  CPPUNIT_TEST(testValidatePU1);
  CPPUNIT_TEST(testValidatePU2);
  CPPUNIT_TEST(testValidatePU3);
  CPPUNIT_TEST(testValidatePU4);
  CPPUNIT_TEST(testValidatePU5);
  CPPUNIT_TEST(testValidatePU6);
  CPPUNIT_TEST(testValidatePU7);
  CPPUNIT_TEST(testValidatePU8);
  CPPUNIT_TEST(testValidatePU9);
  CPPUNIT_TEST(testValidatePU10);
  CPPUNIT_TEST(testValidatePU11);
  CPPUNIT_TEST(testValidatePU12);
  CPPUNIT_TEST(testValidatePU13);
  CPPUNIT_TEST(testValidatePU14);
  CPPUNIT_TEST(testValidatePU15);
  CPPUNIT_TEST(testValidatePU16);
  CPPUNIT_TEST(testValidatePU17);
  CPPUNIT_TEST(testValidatePU18);
  CPPUNIT_TEST(testValidatePU19);
  CPPUNIT_TEST(testValidatePU20);
  CPPUNIT_TEST(testValidatePU21);
  CPPUNIT_TEST(testValidatePU22);
  CPPUNIT_TEST(testValidatePU23);
  CPPUNIT_TEST(testValidatePU24);
  CPPUNIT_TEST(testValidatePU25);
  CPPUNIT_TEST(testValidatePU26);
  CPPUNIT_TEST(testValidatePU27);
  CPPUNIT_TEST(testValidatePU28);
  CPPUNIT_TEST(testValidatePU29);
  CPPUNIT_TEST(testValidatePU_PermittedReachedSameCharge);
  CPPUNIT_TEST(testValidatePU_PermittedReachedOtherCharge);
  CPPUNIT_TEST(testValidatePU_RtwIgnoreInOutOr);

  CPPUNIT_TEST(testLeastRestrictive1);
  CPPUNIT_TEST(testLeastRestrictive2);
  CPPUNIT_TEST(testLeastRestrictive3);
  CPPUNIT_TEST(testGatewayRestrictions1);
  CPPUNIT_TEST(testCarrierRestrictions1);
  CPPUNIT_TEST(testSurfaceSectors1);
  CPPUNIT_TEST(testRegressionPL9779);
  CPPUNIT_TEST(testRegressionPL8848);
  CPPUNIT_TEST(testRegressionPL8868);
  CPPUNIT_TEST(testRegressionPL9755);
  CPPUNIT_TEST(testRegressionPL10364);
  CPPUNIT_TEST(testRegressionPL19790);

  CPPUNIT_TEST(testRegressionSPR83301);
  CPPUNIT_TEST(testRegressionSPR83995);

  CPPUNIT_TEST(testAbacusCmdPrcSurcharge);
  CPPUNIT_TEST(testAccumulateMaxStopoversWhenStopoverLimited);

  CPPUNIT_TEST(testStayTimeWithinSameDay);
  CPPUNIT_TEST(testStayTimeNotInSameDay);
  CPPUNIT_TEST(testStayTimeWithinNextDay);
  CPPUNIT_TEST(testStayTimeOverNextDay);
  CPPUNIT_TEST(testStayTimeWithin2Days);
  CPPUNIT_TEST(testStayTimeOver2Days);
  CPPUNIT_TEST(testValidateStopoversInfo_M2010_true_MaxBlankOutNotBlankInBlank);
  CPPUNIT_TEST(testValidateStopoversInfo_M2010_true_MaxBlankOutBlankInNotBlank);
  CPPUNIT_TEST(testValidateStopoversInfo_M2010_false_Phase1Act_OutInEither_MaxBlank);
  CPPUNIT_TEST(testMandateStopsOutInboundZero);
  CPPUNIT_TEST(testMandateStopsInboundZero);
  CPPUNIT_TEST(testChkApplScope_FC_SCOPE);
  CPPUNIT_TEST(testChkApplScope_PU_SCOPE);
  CPPUNIT_TEST(testCheckPreconditions_UnavailTagNotAvail);
  CPPUNIT_TEST(testCheckPreconditions_UnavailTagTextOnly);
  CPPUNIT_TEST(testCheckPreconditions_RtwMaxBlank);
  CPPUNIT_TEST(testCheckPreconditions_Pass);
  CPPUNIT_TEST(testFareUsageDirToValidate_UnknownDir_ifOutboundMaxEmpty);
  CPPUNIT_TEST(testFareUsageDirToValidate_UnknownDir_ifInboundMaxEmpty);
  CPPUNIT_TEST(testIsPuScope);
  CPPUNIT_TEST(testCheckPaxType_reduntant_Charge2Appl);
  CPPUNIT_TEST(testPrintDiagHeader_noSameCarrierRestAfterPhase2);
  CPPUNIT_TEST(testPrintDiagHeader_noCharge1TotalAfterPhase2);
  CPPUNIT_TEST(testCheckStopoversInSameLoc_Nation);
  CPPUNIT_TEST(testValidate_PASS_First1Nation10Add1Nation20);
  CPPUNIT_TEST(testValidate_FAIL_With1Nation10);

  // SPR 132099 and 124817 test cases
  CPPUNIT_TEST(test_arunk_at_begining_no_SO_fvo);
  CPPUNIT_TEST(test_arunk_at_begining_no_SO_pvo);
  CPPUNIT_TEST(test_arunk_at_begining_with_SO_fvo);
  CPPUNIT_TEST(test_arunk_at_begining_with_SO_pvo);
  CPPUNIT_TEST(test_seg_and_arunk_passes_diff_rec3_fvo);
  CPPUNIT_TEST(test_seg_and_arunk_passes_diff_rec3_pvo);
  CPPUNIT_TEST(test_seg_pass_but_arunk_fail_diff_rec3_fvo);
  CPPUNIT_TEST(test_seg_pass_but_arunk_fail_diff_rec3_pvo);
  CPPUNIT_TEST(test_seg_fail_but_arunk_pass_diff_rec3_fvo);
  CPPUNIT_TEST(test_seg_fail_but_arunk_pass_diff_rec3_pvo);
  CPPUNIT_TEST(test_two_seg_an_arunk_pass_diff_rec3_fvo);
  CPPUNIT_TEST(test_two_seg_an_arunk_pass_diff_rec3_pvo);
  CPPUNIT_TEST(test_two_seg_an_arunk_pass_same_rec3_fvo);
  CPPUNIT_TEST(test_two_seg_an_arunk_pass_same_rec3_pvo);
  CPPUNIT_TEST(test_arunk_two_so_fail_r3_by_count_check_fvo);
  CPPUNIT_TEST(test_arunk_two_so_fail_r3_by_count_check_pvo);
  CPPUNIT_TEST(test_seg_and_arunk_passes_diff_rec3_two_so_fvo);
  CPPUNIT_TEST(test_seg_so_pass_but_arunk_fail_passed_by_command_pricing);
  CPPUNIT_TEST(test_inclusive_record3_fvo);
  CPPUNIT_TEST(test_inclusive_record3_pvo);
  CPPUNIT_TEST(test_so_not_permitted_with_arunk_in_same_R3_pvo);
  CPPUNIT_TEST(test_so_not_permitted_with_arunk_in_diff_R3_pvo);
  CPPUNIT_TEST(test_so_not_permitted_on_SIN_in_both_dir_pvo);
  CPPUNIT_TEST(testMaxExceed_Pass);
  CPPUNIT_TEST(testMaxExceed_MostRestrictive_Fail);
  CPPUNIT_TEST(testMaxExceed_MaxBlank_Pass);
  CPPUNIT_TEST(testMaxExceed_MaxBlank_Fail);

  CPPUNIT_TEST(testProcessSamePointRestrictions_Stopovers);
  CPPUNIT_TEST(testProcessSamePointRestrictions_Connections);
  CPPUNIT_TEST(testProcessSamePointRestrictions_Transfers);
  CPPUNIT_TEST(testProcessSamePointRestrictions_None);
  CPPUNIT_TEST(testProcessSamePointRestrictions_TransEwrRtw);

  CPPUNIT_TEST(testCheckRequiredSegmentsSatisfied_NoRequired);
  CPPUNIT_TEST(testCheckRequiredSegmentsSatisfied_FcLevel);
  CPPUNIT_TEST(testCheckRequiredSegmentsSatisfied_PuLevel);
  CPPUNIT_TEST(testCheckNotAllowedRecurringSegments_PuLevel);

  CPPUNIT_TEST(testGtwyRestrictionsNoRecurringSegmentsAndGtwySoNotAllowed);
  CPPUNIT_TEST(testGtwyRestrictionsNoRecurringSegmentsAndGtwySoRequired);
  CPPUNIT_TEST(testGtwyRestrictionsNoRecurringSegmentsAndGtwySoPermitted);
  CPPUNIT_TEST(testDoNotCheckInBoundSOCount_WhenValidatingAtPUwithOutboundRule);
  CPPUNIT_TEST(testGtwyRequiredWithEmbeddedArunk);
  CPPUNIT_TEST_SUITE_END();

public:
  // Default Constructor
  StopoversTest()
  {
    // Create Locations -- with Factory to avoid DB calls
    // Built in constructor since they will not be changed in the tests

    locMap["SFO"] = sfo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    locMap["DFW"] = dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    locMap["DAL"] = dal = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDAL.xml");
    locMap["SJC"] = sjc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJC.xml");
    locMap["JFK"] = jfk = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    locMap["BOS"] = bos = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    locMap["LGA"] = lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    locMap["LAX"] = lax = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    locMap["IAH"] = iah = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocIAH.xml");
    locMap["MEL"] = mel = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");
    locMap["SYD"] = syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    locMap["HKG"] = hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    locMap["NRT"] = nrt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    locMap["MIA"] = mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");
    locMap["YYZ"] = yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    locMap["YVR"] = yvr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVR.xml");
    locMap["LHR"] = lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");
    locMap["GIG"] = gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");
    locMap["HNL"] = hnl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHNL.xml");
    locMap["STT"] = stt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSTT.xml");
    locMap["ANC"] = anc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocANC.xml");
    locMap["SJU"] = sju = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJU.xml");
    locMap["CDG"] = cdg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");
    locMap["MEX"] = mex = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    locMap["BOM"] = bom = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOM.xml");
    locMap["DUS"] = dus = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUS.xml");
    locMap["SOF"] = sof = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSOF.xml");
    locMap["MXP"] = mxp = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMXP.xml");
    locMap["LGW"] = lgw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGW.xml");
    locMap["SIN"] = sin = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");
    locMap["TPE"] = tpe = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTPE.xml");
    locMap["PVG"] = pvg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPVG.xml");
    locMap["OGG"] = ogg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOGG.xml");
    locMap["ORD"] = ord = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocORD.xml");
    locMap["OTP"] = otp = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOTP.xml");
    locMap["DUB"] = dub = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml");
    locMap["KEF"] = kef = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocKEF.xml");
    locMap["FRA"] = fra = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    locMap["KWI"] = kwi = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocKWI.xml");
    locMap["MCT"] = mct = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMCT.xml");
    locMap["DXB"] = dxb = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDXB.xml");
    locMap["ICN"] = icn = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocICN.xml");
    locMap["CBR"] = icn = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCBR.xml");
    locMap["TYO"] = tyo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    locMap["OSA"] = osa = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOSA.xml");
    locMap["EWR"] = ewr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _agent = _memHandle.create<Agent>();
    _req = _memHandle.create<PricingRequest>();
    _req->ticketingDT() = DateTime::localTime();
    _req->ticketingAgent() = _agent;
    _trx->setRequest(_req);

    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    TrxUtil::enableAbacus();
    _trx->dataHandle().get(_abacusTJR);
    _trx->dataHandle().get(_abacusAgent);
    _abacusTJR->crsCarrier() = "1B";
    _abacusTJR->hostName() = "ABAC";
    _abacusAgent->agentTJR() = _abacusTJR;
    _itin = _memHandle.create<Itin>();
    _farePath = _memHandle.create<FarePath>();
    _itin->farePath() += _farePath;
    _farePath->itin() = _itin;
    _so = _memHandle.create<Stopovers>();
    _soInfoWrapper = _memHandle.create<StopoversInfoWrapper>();
    createPaxType("ADT");
    _itin->calculationCurrency() = "USD";
    _memHandle.create<MyDataHandle>();
    _util = _memHandle.insert(new ServiceFeeUtil(*_trx));
    fallback::value::fallbackAPO37838Record1EffDateCheck.set(true);
  }

  void tearDown() { _memHandle.clear(); }

  void initCategoryRuleInfo(CategoryRuleInfo& crInfo,
                            const StopoversInfo& soInfo,
                            uint32_t relInd = CategoryRuleItemInfo::THEN)
  {
    if (crInfo.categoryRuleItemInfoSet().empty())
      crInfo.addItemInfoSetNosync(new CategoryRuleItemInfoSet);

    CategoryRuleItemInfoSet& criis = *crInfo.categoryRuleItemInfoSet().back();
    criis.push_back(CategoryRuleItemInfo());

    CategoryRuleItemInfo& crii = criis.back();
    crii.setItemcat(8u);
    crii.setOrderNo(uint32_t(criis.size()));
    crii.setItemNo(soInfo.itemNo());
    crii.setRelationalInd(static_cast<CategoryRuleItemInfo::LogicalOperators>(relInd));
  }

  StopoversInfoWrapper& createSoInfoWrapper(const StopoversInfo& soInfo, PricingUnit& pu)
  {
    CategoryRuleInfo& crInfo = *_memHandle.create<CategoryRuleInfo>();
    createBaseCategoryRuleInfo(crInfo);
    initCategoryRuleInfo(crInfo, soInfo);

    FareUsage* fu = pu.fareUsage().front();
    RuleSetPreprocessor& rsp = *_memHandle.create<RuleSetPreprocessor>();
    rsp.process(*_trx, &crInfo, pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    StopoversInfoWrapper& soInfoWrapper = *_memHandle.create<StopoversInfoWrapper>();
    soInfoWrapper.soInfo(&soInfo);
    soInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, pu);

    return soInfoWrapper;
  }

  void createItin1()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AA";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 8, 15, 13, 25, 0); //  6:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 8, 15, 17, 1, 0); // 12:01PM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 22, 35, 0); //  5:35PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 16, 2, 16, 0); // 10:16PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 3;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = false;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 8, 21, 12, 28, 0); //  8:28AM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 8, 21, 16, 3, 0); // 11:03AM CST

    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 4;
    dfw_sfo->origin() = dfw;
    dfw_sfo->destination() = sfo;
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "AA";
    dfw_sfo->geoTravelType() = GeoTravelType::Domestic;
    dfw_sfo->departureDT() = DateTime(2004, 8, 21, 16, 47, 0); // 11:47AM CST
    dfw_sfo->arrivalDT() = DateTime(2004, 8, 21, 20, 21, 0); //  1:21PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_sfo);

    // Create the FareUsages
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu3->travelSeg().push_back(jfk_dfw);
    fu4->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = true;
    fu4->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu4);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu3);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(dfw_jfk);
    pu2->travelSeg().push_back(jfk_dfw);

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);
    _farePath->pricingUnit().push_back(pu2);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, dfw, GlobalDirection::US, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dfw, jfk, GlobalDirection::US, GeoTravelType::Domestic);
    FareMarket* fm3 = createFareMarket(jfk, dfw, GlobalDirection::US, GeoTravelType::Domestic);
    FareMarket* fm4 = createFareMarket(dfw, sfo, GlobalDirection::US, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("USD", fm3);
    PaxTypeFare* ptf4 = createPTF("USD", fm4);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm3->travelSeg().push_back(jfk_dfw);
    fm4->travelSeg().push_back(dfw_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket() += fm1, fm2, fm3, fm4;
  }

  void createItin2()
  {
    // Create the travel segments
    //
    AirSeg* mex_dfw = _memHandle.create<AirSeg>();
    mex_dfw->pnrSegment() = 1;
    mex_dfw->origin() = mex;
    mex_dfw->destination() = dfw;
    mex_dfw->stopOver() = false;
    mex_dfw->carrier() = "AA";
    mex_dfw->geoTravelType() = GeoTravelType::International;
    mex_dfw->departureDT() = DateTime(2004, 8, 15, 12, 0, 0); //  7:00AM CST
    mex_dfw->arrivalDT() = DateTime(2004, 8, 15, 14, 41, 0); //  9:41AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = false;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 8, 15, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 8, 16, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 8, 16, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 8, 16, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 8, 30, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 8, 30, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 8, 30, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 8, 30, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 8, 30, 18, 35, 0); // 2:35PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 8, 30, 22, 8, 0); // 5:08PM CST

    AirSeg* dfw_mex = _memHandle.create<AirSeg>();
    dfw_mex->pnrSegment() = 8;
    dfw_mex->origin() = dfw;
    dfw_mex->destination() = mex;
    dfw_mex->stopOver() = false;
    dfw_mex->carrier() = "AA";
    dfw_mex->geoTravelType() = GeoTravelType::International;
    dfw_mex->departureDT() = DateTime(2004, 8, 31, 0, 10, 0); // 7:10PM CST
    dfw_mex->arrivalDT() = DateTime(2004, 8, 31, 2, 40, 0); // 9:40PM CST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(mex_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_mex);

    _trx->travelSeg().push_back(mex_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_mex);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mex_dfw);
    pu1->travelSeg().push_back(dfw_mex);
    pu2->travelSeg().push_back(dfw_jfk);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu2->travelSeg().push_back(jfk_dfw);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = dfw_mex;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);
    _farePath->pricingUnit().push_back(pu2);
    _farePath->pricingUnit().push_back(pu3);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(mex, dfw, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, mex, GlobalDirection::ZZ, GeoTravelType::International);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("EUR", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("USD", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mex_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_mex);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
    _itin->fareMarket().push_back(fm5);
    _itin->fareMarket().push_back(fm6);
  }

  void createItin3()
  {
    // Create the travel segments
    //
    AirSeg* mex_dfw = _memHandle.create<AirSeg>();
    mex_dfw->pnrSegment() = 1;
    mex_dfw->origin() = mex;
    mex_dfw->destination() = dfw;
    mex_dfw->stopOver() = false;
    mex_dfw->carrier() = "AA";
    mex_dfw->geoTravelType() = GeoTravelType::International;
    mex_dfw->departureDT() = DateTime(2004, 8, 15, 12, 0, 0); //  7:00AM CST
    mex_dfw->arrivalDT() = DateTime(2004, 8, 15, 14, 41, 0); //  9:41AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 8, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 8, 17, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 8, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 8, 17, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 8, 30, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 8, 30, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 8, 30, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 8, 30, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 8, 30, 18, 35, 0); // 2:35PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 8, 30, 22, 8, 0); // 5:08PM CST

    AirSeg* dfw_mex = _memHandle.create<AirSeg>();
    dfw_mex->pnrSegment() = 8;
    dfw_mex->origin() = dfw;
    dfw_mex->destination() = mex;
    dfw_mex->stopOver() = false;
    dfw_mex->carrier() = "AA";
    dfw_mex->geoTravelType() = GeoTravelType::International;
    dfw_mex->departureDT() = DateTime(2004, 8, 31, 0, 10, 0); // 7:10PM CST
    dfw_mex->arrivalDT() = DateTime(2004, 8, 31, 2, 40, 0); // 9:40PM CST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(mex_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_mex);

    _trx->travelSeg().push_back(mex_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_mex);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mex_dfw);
    pu1->travelSeg().push_back(dfw_mex);
    pu2->travelSeg().push_back(dfw_jfk);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu2->travelSeg().push_back(jfk_dfw);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = dfw_mex;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit() += pu1, pu2, pu3;

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(mex, dfw, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, mex, GlobalDirection::ZZ, GeoTravelType::International);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("EUR", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("USD", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mex_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_mex);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
    _itin->fareMarket().push_back(fm5);
    _itin->fareMarket().push_back(fm6);
  }

  void createItin4()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AE";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 8, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 8, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 8, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 8, 17, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 8, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 8, 17, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 8, 30, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 8, 30, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 8, 30, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 8, 30, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 8, 31, 22, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 9, 1, 3, 54, 0); // 8:54PM CST

    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 8;
    dfw_sfo->origin() = dfw;
    dfw_sfo->destination() = sfo;
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "AA";
    dfw_sfo->geoTravelType() = GeoTravelType::Domestic;
    dfw_sfo->departureDT() = DateTime(2004, 9, 2, 2, 51, 0); //  9:51PM CST
    dfw_sfo->arrivalDT() = DateTime(2004, 9, 2, 6, 24, 0); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit() += pu1, pu2, pu3;

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, dfw, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, sfo, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("GBP", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("GBP", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;
    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
    _itin->fareMarket().push_back(fm5);
    _itin->fareMarket().push_back(fm6);
  }

  FareMarket* createFareMarket(const Loc* origin,
                               const Loc* destination,
                               const GlobalDirection& globalDir,
                               const GeoTravelType& geoTvlType)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->origin() = origin;
    fm->destination() = destination;
    fm->setGlobalDirection(globalDir);
    fm->geoTravelType() = geoTvlType;
    return fm;
  }

  TariffCrossRefInfo* createTariffCrossRefInfo()
  {
    TariffCrossRefInfo* tcrInfo = _memHandle.create<TariffCrossRefInfo>();
    tcrInfo->vendor() = "ATP";
    tcrInfo->carrier() = "BA";
    tcrInfo->crossRefType() = INTERNATIONAL;
    tcrInfo->globalDirection() = GlobalDirection::ZZ;
    tcrInfo->ruleTariff() = 1;
    tcrInfo->fareTariff() = 1;
    return tcrInfo;
  }

  TariffCrossRefInfo*
  createTariffCrossRefInfo(const std::string& vendor, const std::string& carrier)
  {
    TariffCrossRefInfo* tcrInfo = _memHandle.create<TariffCrossRefInfo>();
    tcrInfo->vendor() = vendor;
    tcrInfo->carrier() = carrier;
    tcrInfo->crossRefType() = INTERNATIONAL;
    tcrInfo->globalDirection() = GlobalDirection::ZZ;
    tcrInfo->ruleTariff() = 1;
    tcrInfo->fareTariff() = 1;
    return tcrInfo;
  }

  PaxTypeFare*
  createPTF(const CurrencyCode& currency, FareMarket* fm, TariffCrossRefInfo* tcri = NULL)
  {
    const string paxTypeName = _paxType->paxType();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    fare->setCategoryValid(8);
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_vendor = "ATP";
    fi->_globalDirection = fm->getGlobalDirection();
    fi->_owrt = ONE_WAY_MAY_BE_DOUBLED;
    fi->_currency = currency;
    if (NULL == tcri)
      tcri = _memHandle.create<TariffCrossRefInfo>();
    Fare::FareState fareState =
        (fm->geoTravelType() == GeoTravelType::Domestic ? Fare::FS_Domestic : Fare::FS_International);
    fare->initialize(fareState, fi, *fm, tcri);
    ptf->initialize(fare, _paxType, fm);
    FareClassAppInfo* fcai = _memHandle.create<FareClassAppInfo>();
    ptf->fareClassAppInfo() = fcai;
    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    ptf->fareClassAppSegInfo() = fcasi;
    if (paxTypeName == "CNN" || paxTypeName == "INF")
    {
      ptf->status().set(PaxTypeFare::PTF_Discounted);
      PaxTypeFareRuleData* ptfrd = _memHandle.create<PaxTypeFareRuleData>();
      DiscountInfo* di = _memHandle.create<DiscountInfo>();
      di->farecalcInd() = RuleConst::CALCULATED;
      ptfrd->ruleItemInfo() = di;
      di->discPercent() = paxTypeName == "CNN" ? 50.0 : 10.0;
      ptf->setRuleData(19, _trx->dataHandle(), ptfrd);
    }
    return ptf;
  }

  void createPaxType(const std::string paxType)
  {
    _paxType = _memHandle.create<PaxType>();
    _paxType->paxType() = paxType;
    PaxTypeInfo* pti = _memHandle.create<PaxTypeInfo>();
    if (paxType == "CNN")
    {
      pti->childInd() = 'Y';
    }
    else if (paxType == "INF")
    {
      pti->infantInd() = 'Y';
    }
    else
    {
      pti->adultInd() = 'Y';
    }

    _paxType->paxTypeInfo() = pti;
    _farePath->paxType() = _paxType;
  }

  void createItin5(const std::string paxType = "ADT")
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AE";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 8, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 8, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 8, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 8, 17, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 8, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 8, 17, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 8, 30, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 8, 30, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 8, 30, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 8, 30, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 8, 31, 22, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 9, 1, 3, 54, 0); // 8:54PM CST

    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 8;
    dfw_sfo->origin() = dfw;
    dfw_sfo->destination() = sfo;
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "AA";
    dfw_sfo->geoTravelType() = GeoTravelType::Domestic;
    dfw_sfo->departureDT() = DateTime(2004, 9, 2, 2, 51, 0); //  9:51PM CST
    dfw_sfo->arrivalDT() = DateTime(2004, 9, 2, 6, 24, 0); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit() += pu1, pu2, pu3;

    createPaxType(paxType);
    // Create and initialize the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, dfw, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, sfo, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("GBP", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("GBP", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket() += fm1, fm2, fm3, fm4, fm5, fm6;
  }

  void createItin6()
  {
    _itin->calculationCurrency() = "NUC";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AE";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 10, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 10, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 10, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 10, 17, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 10, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 10, 17, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 11, 29, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 11, 29, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 11, 29, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 11, 29, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 11, 30, 22, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 12, 1, 3, 54, 0); // 8:54PM CST

    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 8;
    dfw_sfo->origin() = dfw;
    dfw_sfo->destination() = sfo;
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "AA";
    dfw_sfo->geoTravelType() = GeoTravelType::Domestic;
    dfw_sfo->departureDT() = DateTime(2004, 12, 2, 2, 51, 0); //  9:51PM CST
    dfw_sfo->arrivalDT() = DateTime(2004, 12, 2, 6, 24, 0); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit() += pu1, pu2, pu3;

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, dfw, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, sfo, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("GBP", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("GBP", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
    _itin->fareMarket().push_back(fm5);
    _itin->fareMarket().push_back(fm6);
  }

  void createItin7()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "NW";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 10, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 10, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "NW";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "NW";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 10, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 10, 17, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "NW";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 10, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 10, 17, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "NW";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 11, 29, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 11, 29, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "NW";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 11, 29, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 11, 29, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "NW";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 11, 30, 22, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 12, 1, 3, 54, 0); // 8:54PM CST

    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 8;
    dfw_sfo->origin() = dfw;
    dfw_sfo->destination() = sfo;
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "NW";
    dfw_sfo->geoTravelType() = GeoTravelType::Domestic;
    dfw_sfo->departureDT() = DateTime(2004, 12, 2, 2, 51, 0); //  9:51PM CST
    dfw_sfo->arrivalDT() = DateTime(2004, 12, 2, 6, 24, 0); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit() += pu1, pu2, pu3;

    // Create and initialize the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, dfw, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, sfo, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("GBP", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("GBP", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    fm1->governingCarrier() = "NW";
    fm2->governingCarrier() = "NW";
    fm3->governingCarrier() = "NW";
    fm4->governingCarrier() = "NW";
    fm5->governingCarrier() = "NW";
    fm6->governingCarrier() = "NW";

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
    _itin->fareMarket().push_back(fm5);
    _itin->fareMarket().push_back(fm6);
  }

  void createItin8()
  {
    _itin->calculationCurrency() = "NUC";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AE";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 10, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 10, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 10, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 10, 17, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = true;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 10, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 10, 17, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 11, 29, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 11, 29, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 11, 29, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 11, 29, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 11, 30, 22, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 12, 1, 3, 54, 0); // 8:54PM CST

    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 8;
    dfw_sfo->origin() = dfw;
    dfw_sfo->destination() = sfo;
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "AA";
    dfw_sfo->geoTravelType() = GeoTravelType::Domestic;
    dfw_sfo->departureDT() = DateTime(2004, 12, 2, 2, 51, 0); //  9:51PM CST
    dfw_sfo->arrivalDT() = DateTime(2004, 12, 2, 6, 24, 0); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu1->fareUsage().push_back(fu5);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu3);
    pu2->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu2->travelSeg().push_back(lhr_cdg);
    pu2->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = lhr_jfk;
    pu2->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit() += pu1, pu2;

    // Create and initialize the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, dfw, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dfw, lhr, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm3 = createFareMarket(lhr, cdg, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm4 = createFareMarket(cdg, lhr, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm5 = createFareMarket(lhr, dfw, GlobalDirection::AT, GeoTravelType::International);
    FareMarket* fm6 = createFareMarket(dfw, sfo, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    PaxTypeFare* ptf3 = createPTF("GBP", fm3);
    PaxTypeFare* ptf4 = createPTF("EUR", fm4);
    PaxTypeFare* ptf5 = createPTF("GBP", fm5);
    PaxTypeFare* ptf6 = createPTF("USD", fm6);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
    _itin->fareMarket().push_back(fm5);
    _itin->fareMarket().push_back(fm6);
  }

  void createItin9()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AA";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 10, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 10, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 3;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 10, 16, 21, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 10, 17, 1, 54, 0); // 8:54PM CST

    AirSeg* dfw_mia = _memHandle.create<AirSeg>();
    dfw_mia->pnrSegment() = 4;
    dfw_mia->origin() = dfw;
    dfw_mia->destination() = mia;
    dfw_mia->stopOver() = false;
    dfw_mia->carrier() = "AA";
    dfw_mia->geoTravelType() = GeoTravelType::Domestic;
    dfw_mia->departureDT() = DateTime(2004, 10, 17, 12, 1, 0); //  7:01AM CST
    dfw_mia->arrivalDT() = DateTime(2004, 10, 17, 14, 39, 0); // 10:39AM EST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_mia);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_mia);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_dfw);
    fu1->travelSeg().push_back(dfw_mia);

    // Set the directionality
    fu1->inbound() = false;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_mia);

    pu1->turnAroundPoint() = jfk_dfw;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, mia, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "AA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_dfw);
    fm1->travelSeg().push_back(dfw_mia);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
  }

  void createItin10()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AA";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 10, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 10, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = false;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 11, 40, 0); //  6:40AM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 14, 59, 0); // 10:59AM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 3;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = false;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 10, 15, 16, 10, 0); // 12:10PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 10, 15, 19, 51, 0); //  2:51PM CST

    AirSeg* dfw_mia = _memHandle.create<AirSeg>();
    dfw_mia->pnrSegment() = 4;
    dfw_mia->origin() = dfw;
    dfw_mia->destination() = mia;
    dfw_mia->stopOver() = false;
    dfw_mia->carrier() = "AA";
    dfw_mia->geoTravelType() = GeoTravelType::Domestic;
    dfw_mia->departureDT() = DateTime(2004, 10, 15, 20, 16, 0); // 3:16PM CST
    dfw_mia->arrivalDT() = DateTime(2004, 10, 15, 22, 59, 0); // 6:59PM EST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_mia);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_mia);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_dfw);
    fu1->travelSeg().push_back(dfw_mia);

    // Set the directionality
    fu1->inbound() = false;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_mia);

    pu1->turnAroundPoint() = jfk_dfw;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, mia, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "AA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_dfw);
    fm1->travelSeg().push_back(dfw_mia);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
  }

  void createItin11()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AA";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 10, 15, 7, 25, 0); // 12:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 10, 15, 10, 44, 0); //  5:44AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "NW";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 3;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 10, 16, 21, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 10, 17, 1, 54, 0); // 8:54PM CST

    AirSeg* dfw_mia = _memHandle.create<AirSeg>();
    dfw_mia->pnrSegment() = 4;
    dfw_mia->origin() = dfw;
    dfw_mia->destination() = mia;
    dfw_mia->stopOver() = false;
    dfw_mia->carrier() = "NW";
    dfw_mia->geoTravelType() = GeoTravelType::Domestic;
    dfw_mia->departureDT() = DateTime(2004, 10, 17, 12, 1, 0); //  7:01AM CST
    dfw_mia->arrivalDT() = DateTime(2004, 10, 17, 14, 39, 0); // 10:39AM EST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_mia);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_mia);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_dfw);
    fu1->travelSeg().push_back(dfw_mia);

    // Set the directionality
    fu1->inbound() = false;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_mia);

    pu1->turnAroundPoint() = jfk_dfw;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, mia, GlobalDirection::ZZ, GeoTravelType::Domestic);
    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_dfw);
    fm1->travelSeg().push_back(dfw_mia);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
  }

  void createItin12_Rtw()
  {
    // Create the travel segments
    //
    AirSeg* mex_dfw = _memHandle.create<AirSeg>();
    mex_dfw->pnrSegment() = 1;
    mex_dfw->origin() = mex;
    mex_dfw->destination() = dfw;
    mex_dfw->stopOver() = false;
    mex_dfw->carrier() = "AA";
    mex_dfw->geoTravelType() = GeoTravelType::International;
    mex_dfw->departureDT() = DateTime(2004, 8, 15, 12, 0, 0); //  7:00AM CST
    mex_dfw->arrivalDT() = DateTime(2004, 8, 15, 14, 41, 0); //  9:41AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = false;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = true;
    jfk_lhr->forcedStopOver() = 'Y';
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 8, 15, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 8, 16, 6, 20, 0); //  6:20AM GMT

    AirSeg* lhr_cdg = _memHandle.create<AirSeg>();
    lhr_cdg->pnrSegment() = 4;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = false;
    lhr_cdg->carrier() = "BA";
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 8, 16, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 8, 16, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 5;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 8, 30, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 8, 30, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_jfk = _memHandle.create<AirSeg>();
    lhr_jfk->pnrSegment() = 6;
    lhr_jfk->origin() = lhr;
    lhr_jfk->destination() = jfk;
    lhr_jfk->stopOver() = false;
    lhr_jfk->carrier() = "BA";
    lhr_jfk->geoTravelType() = GeoTravelType::International;
    lhr_jfk->departureDT() = DateTime(2004, 8, 30, 10, 0, 0); // 10:00AM GMT
    lhr_jfk->arrivalDT() = DateTime(2004, 8, 30, 16, 30, 0); // 12:30PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = true;
    jfk_dfw->forcedStopOver() = 'Y';
    jfk_dfw->carrier() = "AA";
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 8, 30, 18, 35, 0); // 2:35PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 8, 30, 22, 8, 0); // 5:08PM CST

    AirSeg* dfw_mex = _memHandle.create<AirSeg>();
    dfw_mex->pnrSegment() = 8;
    dfw_mex->origin() = dfw;
    dfw_mex->destination() = mex;
    dfw_mex->stopOver() = false;
    dfw_mex->carrier() = "AA";
    dfw_mex->geoTravelType() = GeoTravelType::International;
    dfw_mex->departureDT() = DateTime(2004, 8, 31, 0, 10, 0); // 7:10PM CST
    dfw_mex->arrivalDT() = DateTime(2004, 8, 31, 2, 40, 0); // 9:40PM CST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(mex_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_jfk);
    _itin->travelSeg().push_back(jfk_dfw);
    _itin->travelSeg().push_back(dfw_mex);

    _trx->travelSeg().push_back(mex_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);
    _trx->travelSeg().push_back(jfk_dfw);
    _trx->travelSeg().push_back(dfw_mex);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_jfk);
    fu2->travelSeg().push_back(jfk_dfw);
    fu2->travelSeg().push_back(dfw_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mex_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_mex);

    pu1->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(mex, cdg, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(cdg, mex, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("EUR", fm1);
    PaxTypeFare* ptf2 = createPTF("EUR", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mex_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_jfk);
    fm2->travelSeg().push_back(jfk_dfw);
    fm2->travelSeg().push_back(dfw_mex);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItin12_PermittedReached()
  {
    AirSeg* mex_dfw = _memHandle.create<AirSeg>();
    mex_dfw->pnrSegment() = 1;
    mex_dfw->origin() = mex;
    mex_dfw->destination() = dfw;
    mex_dfw->stopOver() = true;
    mex_dfw->forcedStopOver() = 'Y';
    mex_dfw->carrier() = "AA";
    mex_dfw->geoTravelType() = GeoTravelType::International;
    mex_dfw->departureDT() = DateTime(2004, 8, 15, 12, 0, 0); //  7:00AM CST
    mex_dfw->arrivalDT() = DateTime(2004, 8, 15, 14, 40, 0); //  9:41AM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->forcedStopOver() = 'Y';
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 16, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 16, 21, 0, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 3;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->forcedStopOver() = 'Y';
    jfk_lhr->carrier() = "AA";
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 8, 17, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 8, 17, 6, 20, 0); //  6:20AM GMT

    _itin->travelSeg() += mex_dfw, dfw_jfk, jfk_lhr;
    _trx->travelSeg() = _itin->travelSeg();

    FareMarket* fm = createFareMarket(mex, jfk, GlobalDirection::ZZ, GeoTravelType::International);
    fm->travelSeg() = _itin->travelSeg();
    PaxTypeFare* ptf = createPTF("EUR", fm);

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->travelSeg() = _itin->travelSeg();
    fu->inbound() = false;
    fu->paxTypeFare() = ptf;

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->travelSeg() = _itin->travelSeg();
    pu->fareUsage() += fu;

    _farePath->pricingUnit() += pu;
    _itin->fareMarket() += fm;
  }

  void createItin13()
  {
    // Create the travel segments
    //
    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 1;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->flightNumber() = 1882;
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 10, 15, 17, 30, 0); // 12:30PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 10, 15, 21, 8, 0); //  5:08PM EST

    AirSeg* jfk_lhr = _memHandle.create<AirSeg>();
    jfk_lhr->pnrSegment() = 2;
    jfk_lhr->origin() = jfk;
    jfk_lhr->destination() = lhr;
    jfk_lhr->stopOver() = false;
    jfk_lhr->carrier() = "AA";
    jfk_lhr->flightNumber() = 100;
    jfk_lhr->geoTravelType() = GeoTravelType::International;
    jfk_lhr->departureDT() = DateTime(2004, 10, 16, 22, 20, 0); //  6:20PM EST
    jfk_lhr->arrivalDT() = DateTime(2004, 10, 17, 6, 20, 0); //  6:20AM GMT

    SurfaceSeg* lhr_cdg = _memHandle.create<SurfaceSeg>();
    lhr_cdg->pnrSegment() = 3;
    lhr_cdg->origin() = lhr;
    lhr_cdg->destination() = cdg;
    lhr_cdg->stopOver() = false;
    // lhr_cdg->carrier()       = "BA";
    // lhr_cdg->flightNumber()  = 304;
    lhr_cdg->geoTravelType() = GeoTravelType::International;
    lhr_cdg->departureDT() = DateTime(2004, 10, 17, 7, 20, 0); //  7:20AM GMT
    lhr_cdg->arrivalDT() = DateTime(2004, 10, 18, 8, 40, 0); //  9:40AM

    AirSeg* cdg_lhr = _memHandle.create<AirSeg>();
    cdg_lhr->pnrSegment() = 4;
    cdg_lhr->origin() = cdg;
    cdg_lhr->destination() = lhr;
    cdg_lhr->stopOver() = false;
    cdg_lhr->carrier() = "BA";
    cdg_lhr->flightNumber() = 303;
    cdg_lhr->geoTravelType() = GeoTravelType::International;
    cdg_lhr->departureDT() = DateTime(2004, 11, 29, 6, 40, 0); //  7:40AM
    cdg_lhr->arrivalDT() = DateTime(2004, 11, 29, 8, 0, 0); //  8:00AM GMT

    AirSeg* lhr_lga = _memHandle.create<AirSeg>();
    lhr_lga->pnrSegment() = 5;
    lhr_lga->origin() = lhr;
    lhr_lga->destination() = lga;
    lhr_lga->stopOver() = false;
    lhr_lga->carrier() = "AA";
    lhr_lga->flightNumber() = 101;
    lhr_lga->geoTravelType() = GeoTravelType::International;
    lhr_lga->departureDT() = DateTime(2004, 11, 29, 10, 0, 0); // 10:00AM GMT
    lhr_lga->arrivalDT() = DateTime(2004, 11, 29, 16, 30, 0); // 12:30PM EST

    SurfaceSeg* lga_jfk = _memHandle.create<SurfaceSeg>();
    lga_jfk->pnrSegment() = 6;
    lga_jfk->origin() = lga;
    lga_jfk->destination() = jfk;
    lga_jfk->stopOver() = false;
    // lga_jfk->carrier()       = "AA";
    // lga_jfk->flightNumber()  = 101;
    lga_jfk->geoTravelType() = GeoTravelType::Domestic;
    lga_jfk->departureDT() = DateTime(2004, 11, 29, 16, 30, 0); // 12:30PM EST
    lga_jfk->arrivalDT() = DateTime(2004, 11, 30, 22, 59, 0); //  5:59PM EST

    AirSeg* jfk_dfw = _memHandle.create<AirSeg>();
    jfk_dfw->pnrSegment() = 7;
    jfk_dfw->origin() = jfk;
    jfk_dfw->destination() = dfw;
    jfk_dfw->stopOver() = false;
    jfk_dfw->carrier() = "AA";
    jfk_dfw->flightNumber() = 1345;
    jfk_dfw->geoTravelType() = GeoTravelType::Domestic;
    jfk_dfw->departureDT() = DateTime(2004, 11, 30, 22, 59, 0); // 5:59PM EST
    jfk_dfw->arrivalDT() = DateTime(2004, 12, 1, 3, 54, 0); // 8:54PM CST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_lhr);
    _itin->travelSeg().push_back(lhr_cdg);
    _itin->travelSeg().push_back(cdg_lhr);
    _itin->travelSeg().push_back(lhr_lga);
    _itin->travelSeg().push_back(lga_jfk);
    _itin->travelSeg().push_back(jfk_dfw);

    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_lga);
    _trx->travelSeg().push_back(lga_jfk);
    _trx->travelSeg().push_back(jfk_dfw);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_lga);
    fu2->travelSeg().push_back(lga_jfk);
    fu2->travelSeg().push_back(jfk_dfw);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_lga);
    pu1->travelSeg().push_back(lga_jfk);
    pu1->travelSeg().push_back(jfk_dfw);

    pu1->turnAroundPoint() = cdg_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(dfw, cdg, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(cdg, dfw, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "AA";
    fm2->governingCarrier() = "BA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("EUR", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_lga);
    fm2->travelSeg().push_back(lga_jfk);
    fm2->travelSeg().push_back(jfk_dfw);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItin14()
  {
    _itin->calculationCurrency() = "NUC";

    // Create the travel segments
    //
    AirSeg* bom_lhr = _memHandle.create<AirSeg>();
    bom_lhr->pnrSegment() = 1;
    bom_lhr->origin() = bom;
    bom_lhr->destination() = lhr;
    bom_lhr->stopOver() = true;
    bom_lhr->carrier() = "BA";
    bom_lhr->flightNumber() = 138;
    bom_lhr->geoTravelType() = GeoTravelType::International;
    bom_lhr->departureDT() = DateTime(2005, 12, 15, 19, 40, 0); //  2:40AM
    bom_lhr->arrivalDT() = DateTime(2005, 12, 16, 7, 5, 0); //  7:05AM GMT

    AirSeg* lhr_dus = _memHandle.create<AirSeg>();
    lhr_dus->pnrSegment() = 2;
    lhr_dus->origin() = lhr;
    lhr_dus->destination() = dus;
    lhr_dus->stopOver() = false;
    lhr_dus->carrier() = "BA";
    lhr_dus->flightNumber() = 938;
    lhr_dus->geoTravelType() = GeoTravelType::International;
    lhr_dus->departureDT() = DateTime(2005, 12, 18, 10, 20, 0); // 10:20AM GMT
    lhr_dus->arrivalDT() = DateTime(2005, 12, 18, 11, 40, 0); // 12:40PM

    AirSeg* dus_lhr = _memHandle.create<AirSeg>();
    dus_lhr->pnrSegment() = 3;
    dus_lhr->origin() = dus;
    dus_lhr->destination() = lhr;
    dus_lhr->stopOver() = true;
    dus_lhr->carrier() = "BA";
    dus_lhr->flightNumber() = 939;
    dus_lhr->geoTravelType() = GeoTravelType::International;
    dus_lhr->departureDT() = DateTime(2005, 12, 30, 13, 5, 0); //  2:05PM
    dus_lhr->arrivalDT() = DateTime(2005, 12, 30, 14, 35, 0); //  2:35PM GMT

    AirSeg* lhr_bom = _memHandle.create<AirSeg>();
    lhr_bom->pnrSegment() = 4;
    lhr_bom->origin() = lhr;
    lhr_bom->destination() = bom;
    lhr_bom->stopOver() = false;
    lhr_bom->carrier() = "BA";
    lhr_bom->flightNumber() = 139;
    lhr_bom->geoTravelType() = GeoTravelType::International;
    lhr_bom->departureDT() = DateTime(2006, 1, 1, 10, 40, 0); // 10:40AM GMT
    lhr_bom->arrivalDT() = DateTime(2006, 1, 2, 5, 40, 0); // 12:40PM

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(bom_lhr);
    _itin->travelSeg().push_back(lhr_dus);
    _itin->travelSeg().push_back(dus_lhr);
    _itin->travelSeg().push_back(lhr_bom);

    _trx->travelSeg().push_back(bom_lhr);
    _trx->travelSeg().push_back(lhr_dus);
    _trx->travelSeg().push_back(dus_lhr);
    _trx->travelSeg().push_back(lhr_bom);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(bom_lhr);
    fu1->travelSeg().push_back(lhr_dus);
    fu2->travelSeg().push_back(dus_lhr);
    fu2->travelSeg().push_back(lhr_bom);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(bom_lhr);
    pu1->travelSeg().push_back(lhr_dus);
    pu1->travelSeg().push_back(dus_lhr);
    pu1->travelSeg().push_back(lhr_bom);

    pu1->turnAroundPoint() = dus_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(bom, dus, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dus, bom, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "BA";
    fm2->governingCarrier() = "BA";

    // Create and initialize the PaxTypeFares
    //
    TariffCrossRefInfo* tcrInfo1 = createTariffCrossRefInfo();
    TariffCrossRefInfo* tcrInfo2 = createTariffCrossRefInfo();

    PaxTypeFare* ptf1 = createPTF("GBP", fm1, tcrInfo1);
    PaxTypeFare* ptf2 = createPTF("GBP", fm2, tcrInfo2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(bom_lhr);
    fm1->travelSeg().push_back(lhr_dus);
    fm2->travelSeg().push_back(dus_lhr);
    fm2->travelSeg().push_back(lhr_bom);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL9779()
  {
    _itin->calculationCurrency() = "EUR";

    // Create the travel segments
    //
    AirSeg* sof_lhr = _memHandle.create<AirSeg>();
    sof_lhr->pnrSegment() = 1;
    sof_lhr->origin() = sof;
    sof_lhr->destination() = lhr;
    sof_lhr->stopOver() = true;
    sof_lhr->carrier() = "BA";
    sof_lhr->flightNumber() = 891;
    sof_lhr->geoTravelType() = GeoTravelType::International;
    sof_lhr->departureDT() = DateTime(2005, 11, 7, 13, 25, 0); // 2:25PM GMT+1
    sof_lhr->arrivalDT() = DateTime(2005, 11, 7, 15, 45, 0); // 3:45PM GMT

    AirSeg* lhr_mxp = _memHandle.create<AirSeg>();
    lhr_mxp->pnrSegment() = 2;
    lhr_mxp->origin() = lhr;
    lhr_mxp->destination() = mxp;
    lhr_mxp->stopOver() = true;
    lhr_mxp->carrier() = "BA";
    lhr_mxp->flightNumber() = 574;
    lhr_mxp->geoTravelType() = GeoTravelType::International;
    lhr_mxp->departureDT() = DateTime(2005, 11, 12, 18, 45, 0); // 6:45PM GMT
    lhr_mxp->arrivalDT() = DateTime(2005, 11, 12, 20, 40, 0); // 9:40PM GMT+1

    AirSeg* mxp_lhr = _memHandle.create<AirSeg>();
    mxp_lhr->pnrSegment() = 3;
    mxp_lhr->origin() = mxp;
    mxp_lhr->destination() = lhr;
    mxp_lhr->stopOver() = true;
    mxp_lhr->carrier() = "BA";
    mxp_lhr->flightNumber() = 571;
    mxp_lhr->geoTravelType() = GeoTravelType::International;
    mxp_lhr->departureDT() = DateTime(2005, 11, 22, 7, 5, 0); // 7:05AM GMT+1
    mxp_lhr->arrivalDT() = DateTime(2005, 11, 22, 9, 15, 0); // 9:15AM GMT

    AirSeg* lgw_sof = _memHandle.create<AirSeg>();
    lgw_sof->pnrSegment() = 4;
    lgw_sof->origin() = lgw;
    lgw_sof->destination() = sof;
    lgw_sof->stopOver() = false;
    lgw_sof->carrier() = "BA";
    lgw_sof->flightNumber() = 2864;
    lgw_sof->geoTravelType() = GeoTravelType::International;
    lgw_sof->departureDT() = DateTime(2005, 11, 24, 22, 0, 0); // 10:00PM GMT
    lgw_sof->arrivalDT() = DateTime(2005, 11, 25, 3, 0, 0); //  4:00AM GMT+1

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sof_lhr);
    _itin->travelSeg().push_back(lhr_mxp);
    _itin->travelSeg().push_back(mxp_lhr);
    _itin->travelSeg().push_back(lgw_sof);

    _trx->travelSeg().push_back(sof_lhr);
    _trx->travelSeg().push_back(lhr_mxp);
    _trx->travelSeg().push_back(mxp_lhr);
    _trx->travelSeg().push_back(lgw_sof);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sof_lhr);
    fu1->travelSeg().push_back(lhr_mxp);
    fu2->travelSeg().push_back(mxp_lhr);
    fu2->travelSeg().push_back(lgw_sof);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sof_lhr);
    pu1->travelSeg().push_back(lhr_mxp);
    pu1->travelSeg().push_back(mxp_lhr);
    pu1->travelSeg().push_back(lgw_sof);

    pu1->turnAroundPoint() = mxp_lhr;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sof, mxp, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(mxp, sof, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "BA";
    fm2->governingCarrier() = "BA";

    // Create and initialize the PaxTypeFares
    //
    TariffCrossRefInfo* tcrInfo1 = createTariffCrossRefInfo();
    TariffCrossRefInfo* tcrInfo2 = createTariffCrossRefInfo();

    PaxTypeFare* ptf1 = createPTF("EUR", fm1, tcrInfo1);
    PaxTypeFare* ptf2 = createPTF("EUR", fm2, tcrInfo2);
    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sof_lhr);
    fm1->travelSeg().push_back(lhr_mxp);
    fm2->travelSeg().push_back(mxp_lhr);
    fm2->travelSeg().push_back(lgw_sof);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL8820()
  {
    _itin->calculationCurrency() = "SGD";

    // Create the travel segments
    //
    AirSeg* sin_tpe = _memHandle.create<AirSeg>();
    sin_tpe->pnrSegment() = 1;
    sin_tpe->origin() = sin;
    sin_tpe->destination() = tpe;
    sin_tpe->stopOver() = true;
    sin_tpe->carrier() = "SQ";
    sin_tpe->flightNumber() = 872;
    sin_tpe->geoTravelType() = GeoTravelType::International;
    sin_tpe->departureDT() = DateTime(2005, 11, 13, 0, 35, 0); // 8:35AM GMT+8
    sin_tpe->arrivalDT() = DateTime(2005, 11, 13, 5, 5, 0); // 1:05PM GMT+8

    AirSeg* tpe_lax = _memHandle.create<AirSeg>();
    tpe_lax->pnrSegment() = 2;
    tpe_lax->origin() = tpe;
    tpe_lax->destination() = lax;
    tpe_lax->stopOver() = true;
    tpe_lax->carrier() = "SQ";
    tpe_lax->flightNumber() = 30;
    tpe_lax->geoTravelType() = GeoTravelType::International;
    tpe_lax->departureDT() = DateTime(2005, 11, 20, 14, 55, 0); // 10:55PM GMT+8
    tpe_lax->arrivalDT() = DateTime(2005, 11, 21, 14, 40, 0); // 6:40PM PST

    AirSeg* lax_sfo = _memHandle.create<AirSeg>();
    lax_sfo->pnrSegment() = 3;
    lax_sfo->origin() = lax;
    lax_sfo->destination() = sfo;
    lax_sfo->stopOver() = true;
    lax_sfo->carrier() = "US";
    lax_sfo->flightNumber() = 6717;
    lax_sfo->geoTravelType() = GeoTravelType::Domestic;
    lax_sfo->departureDT() = DateTime(2005, 11, 23, 14, 45, 0); // 6:45AM PST
    lax_sfo->arrivalDT() = DateTime(2005, 11, 23, 16, 8, 0); // 8:08AM PST

    AirSeg* sfo_sin = _memHandle.create<AirSeg>();
    sfo_sin->pnrSegment() = 4;
    sfo_sin->origin() = sfo;
    sfo_sin->destination() = sin;
    sfo_sin->stopOver() = false;
    sfo_sin->carrier() = "SQ";
    sfo_sin->flightNumber() = 15;
    sfo_sin->geoTravelType() = GeoTravelType::International;
    sfo_sin->departureDT() = DateTime(2005, 11, 25, 20, 30, 0); // 12:30PM PST
    sfo_sin->arrivalDT() = DateTime(2005, 11, 27, 4, 55, 0); // 12:55AM GMT+8

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sin_tpe);
    _itin->travelSeg().push_back(tpe_lax);
    _itin->travelSeg().push_back(lax_sfo);
    _itin->travelSeg().push_back(sfo_sin);

    _trx->travelSeg().push_back(sin_tpe);
    _trx->travelSeg().push_back(tpe_lax);
    _trx->travelSeg().push_back(lax_sfo);
    _trx->travelSeg().push_back(sfo_sin);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sin_tpe);
    fu1->travelSeg().push_back(tpe_lax);
    fu2->travelSeg().push_back(lax_sfo);
    fu2->travelSeg().push_back(sfo_sin);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sin_tpe);
    pu1->travelSeg().push_back(tpe_lax);
    pu1->travelSeg().push_back(lax_sfo);
    pu1->travelSeg().push_back(sfo_sin);

    pu1->turnAroundPoint() = lax_sfo;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sin, lax, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(lax, sin, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "SQ";
    fm2->governingCarrier() = "SQ";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("SGD", fm1);
    PaxTypeFare* ptf2 = createPTF("SGD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sin_tpe);
    fm1->travelSeg().push_back(tpe_lax);
    fm2->travelSeg().push_back(lax_sfo);
    fm2->travelSeg().push_back(sfo_sin);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL8848()
  {
    _itin->calculationCurrency() = "CNY";

    // Create the travel segments
    //
    AirSeg* pvg_nrt = _memHandle.create<AirSeg>();
    pvg_nrt->pnrSegment() = 1;
    pvg_nrt->origin() = pvg;
    pvg_nrt->destination() = nrt;
    pvg_nrt->stopOver() = true;
    pvg_nrt->carrier() = "NH";
    pvg_nrt->flightNumber() = 922;
    pvg_nrt->geoTravelType() = GeoTravelType::International;
    pvg_nrt->departureDT() = DateTime(2005, 8, 29, 2, 15, 0); // 10:15AM GMT+8
    pvg_nrt->arrivalDT() = DateTime(2005, 8, 29, 5, 15, 0); //  2:15PM GMT+9

    AirSeg* nrt_hnl = _memHandle.create<AirSeg>();
    nrt_hnl->pnrSegment() = 2;
    nrt_hnl->origin() = nrt;
    nrt_hnl->destination() = hnl;
    nrt_hnl->stopOver() = false;
    nrt_hnl->carrier() = "UA";
    nrt_hnl->flightNumber() = 880;
    nrt_hnl->geoTravelType() = GeoTravelType::International;
    nrt_hnl->departureDT() = DateTime(2005, 8, 31, 10, 0, 0); // 7:00PM GMT+9
    nrt_hnl->arrivalDT() = DateTime(2005, 8, 31, 17, 25, 0); // 7:25AM GMT-10

    AirSeg* hnl_ogg = _memHandle.create<AirSeg>();
    hnl_ogg->pnrSegment() = 3;
    hnl_ogg->origin() = hnl;
    hnl_ogg->destination() = ogg;
    hnl_ogg->stopOver() = true;
    hnl_ogg->carrier() = "UA";
    hnl_ogg->flightNumber() = 5201;
    hnl_ogg->geoTravelType() = GeoTravelType::Domestic;
    hnl_ogg->departureDT() = DateTime(2005, 9, 1, 19, 0, 0); // 9:00AM GMT-10
    hnl_ogg->arrivalDT() = DateTime(2005, 9, 1, 20, 0, 0); // 10:00AM GMT-10

    AirSeg* ogg_sfo = _memHandle.create<AirSeg>();
    ogg_sfo->pnrSegment() = 4;
    ogg_sfo->origin() = ogg;
    ogg_sfo->destination() = sfo;
    ogg_sfo->stopOver() = false;
    ogg_sfo->carrier() = "UA";
    ogg_sfo->flightNumber() = 34;
    ogg_sfo->geoTravelType() = GeoTravelType::Domestic;
    ogg_sfo->departureDT() = DateTime(2005, 9, 5, 23, 0, 0); // 1:00PM GMT-10
    ogg_sfo->arrivalDT() = DateTime(2005, 9, 6, 4, 50, 0); // 8:50PM PST

    AirSeg* sfo_ord = _memHandle.create<AirSeg>();
    sfo_ord->pnrSegment() = 5;
    sfo_ord->origin() = sfo;
    sfo_ord->destination() = ord;
    sfo_ord->stopOver() = true;
    sfo_ord->carrier() = "UA";
    sfo_ord->flightNumber() = 158;
    sfo_ord->geoTravelType() = GeoTravelType::Domestic;
    sfo_ord->departureDT() = DateTime(2005, 9, 6, 7, 22, 0); // 11:22PM PST
    sfo_ord->arrivalDT() = DateTime(2005, 9, 6, 11, 18, 0); // 5:18AM CST

    AirSeg* ord_pvg = _memHandle.create<AirSeg>();
    ord_pvg->pnrSegment() = 6;
    ord_pvg->origin() = ord;
    ord_pvg->destination() = pvg;
    ord_pvg->stopOver() = false;
    ord_pvg->carrier() = "UA";
    ord_pvg->flightNumber() = 835;
    ord_pvg->geoTravelType() = GeoTravelType::International;
    ord_pvg->departureDT() = DateTime(2005, 9, 8, 18, 15, 0); // 12:15PM CST
    ord_pvg->arrivalDT() = DateTime(2005, 9, 9, 7, 35, 0); // 3:35PM GMT+8

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(pvg_nrt);
    _itin->travelSeg().push_back(nrt_hnl);
    _itin->travelSeg().push_back(hnl_ogg);
    _itin->travelSeg().push_back(ogg_sfo);
    _itin->travelSeg().push_back(sfo_ord);
    _itin->travelSeg().push_back(ord_pvg);

    _trx->travelSeg().push_back(pvg_nrt);
    _trx->travelSeg().push_back(nrt_hnl);
    _trx->travelSeg().push_back(hnl_ogg);
    _trx->travelSeg().push_back(ogg_sfo);
    _trx->travelSeg().push_back(sfo_ord);
    _trx->travelSeg().push_back(ord_pvg);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(pvg_nrt);
    fu1->travelSeg().push_back(nrt_hnl);
    fu1->travelSeg().push_back(hnl_ogg);
    fu1->travelSeg().push_back(ogg_sfo);
    fu1->travelSeg().push_back(sfo_ord);
    fu2->travelSeg().push_back(ord_pvg);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(pvg_nrt);
    pu1->travelSeg().push_back(nrt_hnl);
    pu1->travelSeg().push_back(hnl_ogg);
    pu1->travelSeg().push_back(ogg_sfo);
    pu1->travelSeg().push_back(sfo_ord);
    pu1->travelSeg().push_back(ord_pvg);

    pu1->turnAroundPoint() = ord_pvg;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(pvg, ord, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(ord, pvg, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "UA";
    fm2->governingCarrier() = "UA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("CNY", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(pvg_nrt);
    fm1->travelSeg().push_back(nrt_hnl);
    fm1->travelSeg().push_back(hnl_ogg);
    fm1->travelSeg().push_back(ogg_sfo);
    fm1->travelSeg().push_back(sfo_ord);
    fm2->travelSeg().push_back(ord_pvg);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL8868()
  {
    _itin->calculationCurrency() = "EUR";

    // Create the travel segments
    //
    AirSeg* otp_lhr = _memHandle.create<AirSeg>();
    otp_lhr->pnrSegment() = 1;
    otp_lhr->origin() = otp;
    otp_lhr->destination() = lhr;
    otp_lhr->stopOver() = true;
    otp_lhr->carrier() = "BA";
    otp_lhr->flightNumber() = 887;
    otp_lhr->geoTravelType() = GeoTravelType::International;
    otp_lhr->departureDT() = DateTime(2005, 10, 14, 13, 50, 0); // 3:50PM GMT+2
    otp_lhr->arrivalDT() = DateTime(2005, 10, 14, 17, 15, 0); // 5:15PM GMT

    AirSeg* lgw_dub = _memHandle.create<AirSeg>();
    lgw_dub->pnrSegment() = 2;
    lgw_dub->origin() = lgw;
    lgw_dub->destination() = dub;
    lgw_dub->stopOver() = false;
    lgw_dub->carrier() = "BA";
    lgw_dub->flightNumber() = 8080;
    lgw_dub->geoTravelType() = GeoTravelType::International;
    lgw_dub->departureDT() = DateTime(2005, 10, 18, 6, 30, 0); // 6:30AM GMT
    lgw_dub->arrivalDT() = DateTime(2005, 10, 18, 7, 55, 0); // 7:55AM GMT

    AirSeg* dub_lgw = _memHandle.create<AirSeg>();
    dub_lgw->pnrSegment() = 3;
    dub_lgw->origin() = dub;
    dub_lgw->destination() = lgw;
    dub_lgw->stopOver() = true;
    dub_lgw->carrier() = "BA";
    dub_lgw->flightNumber() = 8081;
    dub_lgw->geoTravelType() = GeoTravelType::International;
    dub_lgw->departureDT() = DateTime(2005, 10, 22, 8, 30, 0); // 8:30AM GMT
    dub_lgw->arrivalDT() = DateTime(2005, 10, 22, 9, 50, 0); // 9:50AM GMT

    AirSeg* lhr_otp = _memHandle.create<AirSeg>();
    lhr_otp->pnrSegment() = 4;
    lhr_otp->origin() = lhr;
    lhr_otp->destination() = otp;
    lhr_otp->stopOver() = false;
    lhr_otp->carrier() = "BA";
    lhr_otp->flightNumber() = 886;
    lhr_otp->geoTravelType() = GeoTravelType::International;
    lhr_otp->departureDT() = DateTime(2005, 10, 27, 9, 45, 0); // 9:45AM GMT
    lhr_otp->arrivalDT() = DateTime(2005, 10, 27, 13, 00, 0); // 3:00PM GMT+2

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(otp_lhr);
    _itin->travelSeg().push_back(lgw_dub);
    _itin->travelSeg().push_back(dub_lgw);
    _itin->travelSeg().push_back(lhr_otp);

    _trx->travelSeg().push_back(otp_lhr);
    _trx->travelSeg().push_back(lgw_dub);
    _trx->travelSeg().push_back(dub_lgw);
    _trx->travelSeg().push_back(lhr_otp);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(otp_lhr);
    fu1->travelSeg().push_back(lgw_dub);
    fu2->travelSeg().push_back(dub_lgw);
    fu2->travelSeg().push_back(lhr_otp);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(otp_lhr);
    pu1->travelSeg().push_back(lgw_dub);
    pu1->travelSeg().push_back(dub_lgw);
    pu1->travelSeg().push_back(lhr_otp);

    pu1->turnAroundPoint() = dub_lgw;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(bom, dus, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(dus, bom, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->origin() = otp;
    fm1->destination() = dub;
    fm2->origin() = dub;
    fm2->destination() = otp;

    fm1->governingCarrier() = "BA";
    fm2->governingCarrier() = "BA";

    // Create and initialize the PaxTypeFares
    //
    TariffCrossRefInfo* tcrInfo1 = createTariffCrossRefInfo();
    TariffCrossRefInfo* tcrInfo2 = createTariffCrossRefInfo();

    PaxTypeFare* ptf1 = createPTF("EUR", fm1, tcrInfo1);
    PaxTypeFare* ptf2 = createPTF("EUR", fm2, tcrInfo2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(otp_lhr);
    fm1->travelSeg().push_back(lgw_dub);
    fm2->travelSeg().push_back(dub_lgw);
    fm2->travelSeg().push_back(lhr_otp);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL9755()
  {
    _itin->calculationCurrency() = "CNY";

    // Create the travel segments
    //
    AirSeg* pvg_nrt = _memHandle.create<AirSeg>();
    pvg_nrt->pnrSegment() = 1;
    pvg_nrt->origin() = pvg;
    pvg_nrt->destination() = nrt;
    pvg_nrt->stopOver() = true;
    pvg_nrt->carrier() = "NH";
    pvg_nrt->flightNumber() = 922;
    pvg_nrt->geoTravelType() = GeoTravelType::International;
    pvg_nrt->departureDT() = DateTime(2005, 8, 29, 2, 15, 0); // 10:15AM GMT+8
    pvg_nrt->arrivalDT() = DateTime(2005, 8, 29, 5, 15, 0); //  2:15PM GMT+9

    AirSeg* nrt_hnl = _memHandle.create<AirSeg>();
    nrt_hnl->pnrSegment() = 2;
    nrt_hnl->origin() = nrt;
    nrt_hnl->destination() = hnl;
    nrt_hnl->stopOver() = false;
    nrt_hnl->carrier() = "UA";
    nrt_hnl->flightNumber() = 880;
    nrt_hnl->geoTravelType() = GeoTravelType::International;
    nrt_hnl->departureDT() = DateTime(2005, 8, 31, 10, 0, 0); // 7:00PM GMT+9
    nrt_hnl->arrivalDT() = DateTime(2005, 8, 31, 17, 25, 0); // 7:25AM GMT-10

    AirSeg* hnl_ogg = _memHandle.create<AirSeg>();
    hnl_ogg->pnrSegment() = 3;
    hnl_ogg->origin() = hnl;
    hnl_ogg->destination() = ogg;
    hnl_ogg->stopOver() = true;
    hnl_ogg->carrier() = "UA";
    hnl_ogg->flightNumber() = 5201;
    hnl_ogg->geoTravelType() = GeoTravelType::Domestic;
    hnl_ogg->departureDT() = DateTime(2005, 9, 1, 19, 0, 0); // 9:00AM GMT-10
    hnl_ogg->arrivalDT() = DateTime(2005, 9, 1, 20, 0, 0); // 10:00AM GMT-10

    AirSeg* ogg_sfo = _memHandle.create<AirSeg>();
    ogg_sfo->pnrSegment() = 4;
    ogg_sfo->origin() = ogg;
    ogg_sfo->destination() = sfo;
    ogg_sfo->stopOver() = false;
    ogg_sfo->carrier() = "UA";
    ogg_sfo->flightNumber() = 34;
    ogg_sfo->geoTravelType() = GeoTravelType::Domestic;
    ogg_sfo->departureDT() = DateTime(2005, 9, 5, 23, 0, 0); // 1:00PM GMT-10
    ogg_sfo->arrivalDT() = DateTime(2005, 9, 6, 4, 50, 0); // 8:50PM PST

    AirSeg* sfo_ord = _memHandle.create<AirSeg>();
    sfo_ord->pnrSegment() = 5;
    sfo_ord->origin() = sfo;
    sfo_ord->destination() = ord;
    sfo_ord->stopOver() = true;
    sfo_ord->carrier() = "UA";
    sfo_ord->flightNumber() = 158;
    sfo_ord->geoTravelType() = GeoTravelType::Domestic;
    sfo_ord->departureDT() = DateTime(2005, 9, 6, 7, 22, 0); // 11:22PM PST
    sfo_ord->arrivalDT() = DateTime(2005, 9, 6, 11, 18, 0); // 5:18AM CST

    AirSeg* ord_pvg = _memHandle.create<AirSeg>();
    ord_pvg->pnrSegment() = 6;
    ord_pvg->origin() = ord;
    ord_pvg->destination() = pvg;
    ord_pvg->stopOver() = false;
    ord_pvg->carrier() = "UA";
    ord_pvg->flightNumber() = 835;
    ord_pvg->geoTravelType() = GeoTravelType::International;
    ord_pvg->departureDT() = DateTime(2005, 9, 8, 18, 15, 0); // 12:15PM CST
    ord_pvg->arrivalDT() = DateTime(2005, 9, 9, 7, 35, 0); // 3:35PM GMT+8

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(pvg_nrt);
    _itin->travelSeg().push_back(nrt_hnl);
    _itin->travelSeg().push_back(hnl_ogg);
    _itin->travelSeg().push_back(ogg_sfo);
    _itin->travelSeg().push_back(sfo_ord);
    _itin->travelSeg().push_back(ord_pvg);

    _trx->travelSeg().push_back(pvg_nrt);
    _trx->travelSeg().push_back(nrt_hnl);
    _trx->travelSeg().push_back(hnl_ogg);
    _trx->travelSeg().push_back(ogg_sfo);
    _trx->travelSeg().push_back(sfo_ord);
    _trx->travelSeg().push_back(ord_pvg);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(pvg_nrt);
    fu1->travelSeg().push_back(nrt_hnl);
    fu1->travelSeg().push_back(hnl_ogg);
    fu1->travelSeg().push_back(ogg_sfo);
    fu1->travelSeg().push_back(sfo_ord);
    fu2->travelSeg().push_back(ord_pvg);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(pvg_nrt);
    pu1->travelSeg().push_back(nrt_hnl);
    pu1->travelSeg().push_back(hnl_ogg);
    pu1->travelSeg().push_back(ogg_sfo);
    pu1->travelSeg().push_back(sfo_ord);
    pu1->travelSeg().push_back(ord_pvg);

    pu1->turnAroundPoint() = ord_pvg;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(pvg, ord, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(ord, pvg, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "UA";
    fm2->governingCarrier() = "UA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("CNY", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(pvg_nrt);
    fm1->travelSeg().push_back(nrt_hnl);
    fm1->travelSeg().push_back(hnl_ogg);
    fm1->travelSeg().push_back(ogg_sfo);
    fm1->travelSeg().push_back(sfo_ord);
    fm2->travelSeg().push_back(ord_pvg);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL10364()
  {
    // Create the travel segments
    //
    AirSeg* lga_bos = _memHandle.create<AirSeg>();
    lga_bos->pnrSegment() = 1;
    lga_bos->origin() = lga;
    lga_bos->destination() = bos;
    lga_bos->stopOver() = true;
    lga_bos->carrier() = "AA";
    lga_bos->flightNumber() = 4803;
    lga_bos->geoTravelType() = GeoTravelType::Domestic;
    lga_bos->departureDT() = DateTime(2005, 11, 17, 12, 0, 0); // 7:00AM GMT-5
    lga_bos->arrivalDT() = DateTime(2005, 11, 17, 13, 11, 0); // 8:11PM GMT-5

    AirSeg* bos_kef = _memHandle.create<AirSeg>();
    bos_kef->pnrSegment() = 2;
    bos_kef->origin() = bos;
    bos_kef->destination() = kef;
    bos_kef->stopOver() = false;
    bos_kef->carrier() = "FI";
    bos_kef->flightNumber() = 630;
    bos_kef->geoTravelType() = GeoTravelType::International;
    bos_kef->departureDT() = DateTime(2005, 12, 7, 13, 35, 0); // 8:35PM GMT-5
    bos_kef->arrivalDT() = DateTime(2005, 12, 8, 6, 40, 0); // 6:40AM GMT

    AirSeg* kef_fra = _memHandle.create<AirSeg>();
    kef_fra->pnrSegment() = 3;
    kef_fra->origin() = kef;
    kef_fra->destination() = fra;
    kef_fra->stopOver() = true;
    kef_fra->carrier() = "FI";
    kef_fra->flightNumber() = 520;
    kef_fra->geoTravelType() = GeoTravelType::International;
    kef_fra->departureDT() = DateTime(2005, 12, 8, 7, 35, 0); // 7:35AM GMT
    kef_fra->arrivalDT() = DateTime(2005, 12, 8, 13, 0, 0); // 12:00PM GMT+1

    AirSeg* fra_kef = _memHandle.create<AirSeg>();
    fra_kef->pnrSegment() = 4;
    fra_kef->origin() = fra;
    fra_kef->destination() = kef;
    fra_kef->stopOver() = false;
    fra_kef->carrier() = "FI";
    fra_kef->flightNumber() = 521;
    fra_kef->geoTravelType() = GeoTravelType::International;
    fra_kef->departureDT() = DateTime(2005, 12, 26, 12, 40, 0); // 1:40PM GMT+1
    fra_kef->arrivalDT() = DateTime(2005, 12, 26, 16, 30, 0); // 4:30PM GMT

    AirSeg* kef_bos = _memHandle.create<AirSeg>();
    kef_bos->pnrSegment() = 5;
    kef_bos->origin() = kef;
    kef_bos->destination() = bos;
    kef_bos->stopOver() = true;
    kef_bos->carrier() = "FI";
    kef_bos->flightNumber() = 631;
    kef_bos->geoTravelType() = GeoTravelType::International;
    kef_bos->departureDT() = DateTime(2005, 12, 26, 17, 0, 0); // 5:00PM GMT
    kef_bos->arrivalDT() = DateTime(2005, 12, 26, 22, 50, 0); // 5:50PM GMT-5

    AirSeg* bos_lga = _memHandle.create<AirSeg>();
    bos_lga->pnrSegment() = 6;
    bos_lga->origin() = bos;
    bos_lga->destination() = lga;
    bos_lga->stopOver() = false;
    bos_lga->carrier() = "AA";
    bos_lga->flightNumber() = 4810;
    bos_lga->geoTravelType() = GeoTravelType::Domestic;
    bos_lga->departureDT() = DateTime(2006, 1, 16, 14, 0, 0); // 9:00AM GMT-5
    bos_lga->arrivalDT() = DateTime(2006, 1, 16, 15, 14, 0); // 10:14AM GMT-5

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(lga_bos);
    _itin->travelSeg().push_back(bos_kef);
    _itin->travelSeg().push_back(kef_fra);
    _itin->travelSeg().push_back(fra_kef);
    _itin->travelSeg().push_back(kef_bos);
    _itin->travelSeg().push_back(bos_lga);

    _trx->travelSeg().push_back(lga_bos);
    _trx->travelSeg().push_back(bos_kef);
    _trx->travelSeg().push_back(kef_fra);
    _trx->travelSeg().push_back(fra_kef);
    _trx->travelSeg().push_back(kef_bos);
    _trx->travelSeg().push_back(bos_lga);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(lga_bos);
    fu1->travelSeg().push_back(bos_kef);
    fu1->travelSeg().push_back(kef_fra);
    fu2->travelSeg().push_back(fra_kef);
    fu2->travelSeg().push_back(kef_bos);
    fu2->travelSeg().push_back(bos_lga);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(lga_bos);
    pu1->travelSeg().push_back(bos_kef);
    pu1->travelSeg().push_back(kef_fra);
    pu1->travelSeg().push_back(fra_kef);
    pu1->travelSeg().push_back(kef_bos);
    pu1->travelSeg().push_back(bos_lga);

    pu1->turnAroundPoint() = fra_kef;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(lga, fra, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(fra, lga, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "FI";
    fm2->governingCarrier() = "FI";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(lga_bos);
    fm1->travelSeg().push_back(bos_kef);
    fm1->travelSeg().push_back(kef_fra);
    fm2->travelSeg().push_back(fra_kef);
    fm2->travelSeg().push_back(kef_bos);
    fm2->travelSeg().push_back(bos_lga);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionPL19790()
  {
    _itin->calculationCurrency() = "NUC";

    // Create the travel segments
    //
    AirSeg* ord_bos = _memHandle.create<AirSeg>();
    ord_bos->pnrSegment() = 1;
    ord_bos->origin() = ord;
    ord_bos->destination() = bos;
    ord_bos->stopOver() = true;
    ord_bos->carrier() = "AA";
    ord_bos->flightNumber() = 1210;
    ord_bos->geoTravelType() = GeoTravelType::Domestic;
    ord_bos->departureDT() = DateTime(2009, 10, 20, 12, 0, 0); // 7:00AM GMT-5
    ord_bos->arrivalDT() = DateTime(2009, 10, 21, 13, 11, 0); // 8:11PM GMT-5

    AirSeg* bos_lhr = _memHandle.create<AirSeg>();
    bos_lhr->pnrSegment() = 2;
    bos_lhr->origin() = bos;
    bos_lhr->destination() = lhr;
    bos_lhr->stopOver() = false;
    bos_lhr->carrier() = "BA";
    bos_lhr->flightNumber() = 238;
    bos_lhr->geoTravelType() = GeoTravelType::International;
    bos_lhr->departureDT() = DateTime(2009, 10, 22, 12, 0, 0); // 8:35PM GMT-5
    bos_lhr->arrivalDT() = DateTime(2009, 10, 22, 13, 11, 0); // 6:40AM GMT

    AirSeg* lhr_dfw = _memHandle.create<AirSeg>();
    lhr_dfw->pnrSegment() = 3;
    lhr_dfw->origin() = lhr;
    lhr_dfw->destination() = dfw;
    lhr_dfw->stopOver() = true;
    lhr_dfw->carrier() = "BA";
    lhr_dfw->flightNumber() = 193;
    lhr_dfw->geoTravelType() = GeoTravelType::International;
    lhr_dfw->departureDT() = DateTime(2009, 10, 30, 7, 35, 0); // 7:35AM GMT
    lhr_dfw->arrivalDT() = DateTime(2009, 10, 30, 13, 0, 0); // 12:00PM GMT+1

    AirSeg* dfw_ord = _memHandle.create<AirSeg>();
    dfw_ord->pnrSegment() = 4;
    dfw_ord->origin() = dfw;
    dfw_ord->destination() = ord;
    dfw_ord->stopOver() = false;
    dfw_ord->carrier() = "AA";
    dfw_ord->flightNumber() = 2360;
    dfw_ord->geoTravelType() = GeoTravelType::Domestic;
    dfw_ord->departureDT() = DateTime(2009, 11, 1, 17, 0, 0); // 5:00PM GMT
    dfw_ord->arrivalDT() = DateTime(2009, 11, 1, 22, 50, 0); // 5:50PM GMT-5

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(ord_bos);
    _itin->travelSeg().push_back(bos_lhr);
    _itin->travelSeg().push_back(lhr_dfw);
    _itin->travelSeg().push_back(dfw_ord);

    _trx->travelSeg().push_back(ord_bos);
    _trx->travelSeg().push_back(bos_lhr);
    _trx->travelSeg().push_back(lhr_dfw);
    _trx->travelSeg().push_back(dfw_ord);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(ord_bos);
    fu1->travelSeg().push_back(bos_lhr);
    fu2->travelSeg().push_back(lhr_dfw);
    fu2->travelSeg().push_back(dfw_ord);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu = _memHandle.create<PricingUnit>();

    pu->puType() = PricingUnit::Type::CIRCLETRIP;

    // Attach the fare usages to the pricing units
    //
    pu->fareUsage().push_back(fu1);
    pu->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu->travelSeg().push_back(ord_bos);
    pu->travelSeg().push_back(bos_lhr);
    pu->travelSeg().push_back(lhr_dfw);
    pu->travelSeg().push_back(dfw_ord);

    pu->turnAroundPoint() = lhr_dfw;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(ord, lhr, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(lhr, ord, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "BA";
    fm2->governingCarrier() = "BA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(ord_bos);
    fm1->travelSeg().push_back(bos_lhr);
    fm2->travelSeg().push_back(lhr_dfw);
    fm2->travelSeg().push_back(dfw_ord);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionSPR83301()
  {
    _itin->calculationCurrency() = "KWD";

    // Create the travel segments
    //
    AirSeg* kwi_fra = _memHandle.create<AirSeg>();
    kwi_fra->pnrSegment() = 1;
    kwi_fra->origin() = kwi;
    kwi_fra->destination() = fra;
    kwi_fra->stopOver() = true;
    kwi_fra->carrier() = "LH";
    kwi_fra->flightNumber() = 625;
    kwi_fra->geoTravelType() = GeoTravelType::International;
    kwi_fra->departureDT() = DateTime(2006, 6, 27, 23, 25, 0); // 1:25AM GMT+2
    kwi_fra->arrivalDT() = DateTime(2006, 6, 28, 5, 20, 0); // 6:20AM GMT+1

    AirSeg* fra_iah = _memHandle.create<AirSeg>();
    fra_iah->pnrSegment() = 2;
    fra_iah->origin() = fra;
    fra_iah->destination() = iah;
    fra_iah->stopOver() = true;
    fra_iah->carrier() = "LH";
    fra_iah->flightNumber() = 440;
    fra_iah->geoTravelType() = GeoTravelType::International;
    fra_iah->departureDT() = DateTime(2006, 7, 1, 8, 35, 0); // 9:35AM GMT+1
    fra_iah->arrivalDT() = DateTime(2006, 7, 1, 19, 30, 0); // 1:30PM GMT-6

    AirSeg* iah_fra = _memHandle.create<AirSeg>();
    iah_fra->pnrSegment() = 3;
    iah_fra->origin() = iah;
    iah_fra->destination() = fra;
    iah_fra->stopOver() = true;
    iah_fra->carrier() = "LH";
    iah_fra->flightNumber() = 441;
    iah_fra->geoTravelType() = GeoTravelType::International;
    iah_fra->departureDT() = DateTime(2006, 7, 23, 21, 20, 0); // 3:20PM GMT-6
    iah_fra->arrivalDT() = DateTime(2006, 7, 23, 7, 25, 0); // 8:25AM GMT+1

    AirSeg* fra_kwi = _memHandle.create<AirSeg>();
    fra_kwi->pnrSegment() = 4;
    fra_kwi->origin() = fra;
    fra_kwi->destination() = kwi;
    fra_kwi->stopOver() = false;
    fra_kwi->carrier() = "LH";
    fra_kwi->flightNumber() = 624;
    fra_kwi->geoTravelType() = GeoTravelType::International;
    fra_kwi->departureDT() = DateTime(2006, 7, 25, 12, 5, 0); // 1:05PM GMT+1
    fra_kwi->arrivalDT() = DateTime(2006, 7, 25, 17, 15, 0); // 7:15PM GMT+2

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(kwi_fra);
    _itin->travelSeg().push_back(fra_iah);
    _itin->travelSeg().push_back(iah_fra);
    _itin->travelSeg().push_back(fra_kwi);

    _trx->travelSeg().push_back(kwi_fra);
    _trx->travelSeg().push_back(fra_iah);
    _trx->travelSeg().push_back(iah_fra);
    _trx->travelSeg().push_back(fra_kwi);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(kwi_fra);
    fu1->travelSeg().push_back(fra_iah);
    fu2->travelSeg().push_back(iah_fra);
    fu2->travelSeg().push_back(fra_kwi);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(kwi_fra);
    pu1->travelSeg().push_back(fra_iah);
    pu1->travelSeg().push_back(iah_fra);
    pu1->travelSeg().push_back(fra_kwi);

    pu1->turnAroundPoint() = iah_fra;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(kwi, iah, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(iah, kwi, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "LH";
    fm2->governingCarrier() = "LH";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("KWD", fm1);
    PaxTypeFare* ptf2 = createPTF("KWD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(kwi_fra);
    fm1->travelSeg().push_back(fra_iah);
    fm2->travelSeg().push_back(iah_fra);
    fm2->travelSeg().push_back(fra_kwi);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createItinRegressionSPR83995()
  {
    _itin->calculationCurrency() = "OMR";

    // Create the travel segments
    //
    AirSeg* mct_dxb = _memHandle.create<AirSeg>();
    mct_dxb->pnrSegment() = 1;
    mct_dxb->origin() = mct;
    mct_dxb->destination() = dxb;
    mct_dxb->stopOver() = true;
    mct_dxb->carrier() = "EK";
    mct_dxb->flightNumber() = 863;
    mct_dxb->geoTravelType() = GeoTravelType::International;
    mct_dxb->departureDT() = DateTime(2006, 8, 1, 7, 45, 0); // 10:45AM GMT+3
    mct_dxb->arrivalDT() = DateTime(2006, 8, 1, 8, 45, 0); // 11:45AM GMT+3

    AirSeg* dxb_icn = _memHandle.create<AirSeg>();
    dxb_icn->pnrSegment() = 2;
    dxb_icn->origin() = dxb;
    dxb_icn->destination() = icn;
    dxb_icn->stopOver() = true;
    dxb_icn->carrier() = "EK";
    dxb_icn->flightNumber() = 322;
    dxb_icn->geoTravelType() = GeoTravelType::International;
    dxb_icn->departureDT() = DateTime(2006, 8, 5, 0, 0, 0); // 3:00AM GMT+3
    dxb_icn->arrivalDT() = DateTime(2006, 8, 5, 8, 45, 0); // 4:45PM GMT+8

    AirSeg* icn_dxb = _memHandle.create<AirSeg>();
    icn_dxb->pnrSegment() = 3;
    icn_dxb->origin() = icn;
    icn_dxb->destination() = dxb;
    icn_dxb->stopOver() = true;
    icn_dxb->carrier() = "EK";
    icn_dxb->flightNumber() = 323;
    icn_dxb->geoTravelType() = GeoTravelType::International;
    icn_dxb->departureDT() = DateTime(2006, 8, 10, 15, 55, 0); // 11:55PM GMT+8
    icn_dxb->arrivalDT() = DateTime(2006, 8, 11, 2, 10, 0); //  5:10AM GMT+3

    AirSeg* dxb_mct = _memHandle.create<AirSeg>();
    dxb_mct->pnrSegment() = 4;
    dxb_mct->origin() = dxb;
    dxb_mct->destination() = mct;
    dxb_mct->stopOver() = false;
    dxb_mct->carrier() = "EK";
    dxb_mct->flightNumber() = 862;
    dxb_mct->geoTravelType() = GeoTravelType::International;
    dxb_mct->departureDT() = DateTime(2006, 8, 15, 5, 25, 0); // 8:25AM GMT+3
    dxb_mct->arrivalDT() = DateTime(2006, 8, 15, 6, 25, 0); // 9:25AM GMT+3

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(mct_dxb);
    _itin->travelSeg().push_back(dxb_icn);
    _itin->travelSeg().push_back(icn_dxb);
    _itin->travelSeg().push_back(dxb_mct);

    _trx->travelSeg().push_back(mct_dxb);
    _trx->travelSeg().push_back(dxb_icn);
    _trx->travelSeg().push_back(icn_dxb);
    _trx->travelSeg().push_back(dxb_mct);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mct_dxb);
    fu1->travelSeg().push_back(dxb_icn);
    fu2->travelSeg().push_back(icn_dxb);
    fu2->travelSeg().push_back(dxb_mct);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mct_dxb);
    pu1->travelSeg().push_back(dxb_icn);
    pu1->travelSeg().push_back(icn_dxb);
    pu1->travelSeg().push_back(dxb_mct);

    pu1->turnAroundPoint() = icn_dxb;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(mct, icn, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(icn, mct, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "BA";
    fm2->governingCarrier() = "BA";

    // Create and initialize the PaxTypeFares
    //
    TariffCrossRefInfo* tcrInfo1 = createTariffCrossRefInfo();
    TariffCrossRefInfo* tcrInfo2 = createTariffCrossRefInfo();

    PaxTypeFare* ptf1 = createPTF("EUR", fm1, tcrInfo1);
    PaxTypeFare* ptf2 = createPTF("EUR", fm2, tcrInfo2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mct_dxb);
    fm1->travelSeg().push_back(dxb_icn);
    fm2->travelSeg().push_back(icn_dxb);
    fm2->travelSeg().push_back(dxb_mct);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void createBaseStopoversInfo(StopoversInfo& soInfo)
  {
    soInfo.vendor() = "ATP";
    soInfo.itemNo() = 1;
    soInfo.unavailTag() = ' ';
    soInfo.geoTblItemNoBtw() = 0;
    soInfo.geoTblItemNoAnd() = 0;
    soInfo.geoTblItemNoGateway() = 0;
    soInfo.gtwyInd() = ' ';
    soInfo.samePointsTblItemNo() = 0;
    soInfo.samePntStops() = 0;
    soInfo.samePntTransit() = 0;
    soInfo.samePntConnections() = 0;
    soInfo.noStopsMin() = "00";
    soInfo.noStopsMax() = "XX";
    soInfo.noStopsOutbound() = "";
    soInfo.noStopsInbound() = "";
    soInfo.minStayTime() = 0;
    soInfo.maxStayTime() = 0;
    soInfo.minStayTimeUnit() = ' ';
    soInfo.maxStayTimeUnit() = ' ';
    soInfo.outOrReturnInd() = ' ';
    soInfo.sameCarrierInd() = ' ';
    soInfo.ojStopoverInd() = ' ';
    soInfo.ct2StopoverInd() = ' ';
    soInfo.ct2PlusStopoverInd() = ' ';

    soInfo.charge1FirstAmt() = 0;
    soInfo.charge1AddAmt() = 0;
    soInfo.charge1NoDec() = 0;
    soInfo.charge1Appl() = ' ';
    soInfo.charge1Total() = ' ';
    soInfo.charge1First() = "";
    soInfo.charge1AddNo() = "";
    soInfo.charge1Cur() = "";

    soInfo.charge2FirstAmt() = 0;
    soInfo.charge2AddAmt() = 0;
    soInfo.charge2NoDec() = 0;
    soInfo.charge2Appl() = ' ';
    soInfo.charge2Total() = ' ';
    soInfo.charge2First() = "";
    soInfo.charge2AddNo() = "";
    soInfo.charge2Cur() = "";
  }

  void createBaseStopoversInfoSeg(StopoversInfoSeg* soInfoSeg)
  {
    soInfoSeg->orderNo() = 1;
    soInfoSeg->carrierInd() = ' ';
    soInfoSeg->noStops() = "";
    soInfoSeg->carrierIn() = "";
    soInfoSeg->stopoverGeoAppl() = ' ';
    soInfoSeg->carrierOut() = "";
    soInfoSeg->stopoverInOutInd() = ' ';
    soInfoSeg->chargeInd() = ' ';
    soInfoSeg->loc1().locType() = LOCTYPE_NONE;
    soInfoSeg->loc1().loc() = "";
    soInfoSeg->loc2().locType() = LOCTYPE_NONE;
    soInfoSeg->loc2().loc() = "";
  }

  void createBaseCategoryRuleInfo(CategoryRuleInfo& crInfo)
  {
    crInfo.categoryNumber() = RuleConst::STOPOVER_RULE;
    crInfo.vendorCode() = "ATP";
    crInfo.tariffNumber() = 1;
    crInfo.carrierCode() = "NW";
    crInfo.ruleNumber() = "0001";
    crInfo.sequenceNumber() = 1;
  }

  void testValidateFare0()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    FareMarket* fm = _itin->fareMarket()[0];
    PricingUnit* pu = _farePath->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Test error condition.
    //
    const StopoversInfo* soInfo = 0;

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, soInfo, *fm);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 0", ret == FAIL);
  }

  void testValidateFare1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    FareMarket* fm = _itin->fareMarket()[0];
    PricingUnit* pu = _farePath->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 1", finalPass);
  }

  void testValidateFare2()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    FareMarket* fm = _itin->fareMarket()[1];
    PricingUnit* pu = _farePath->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 2", finalPass);
  }

  void testValidateFare3()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    FareMarket* fm = _itin->fareMarket()[2];
    PricingUnit* pu = _farePath->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 3", finalPass);
  }

  void testValidateFare4()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    FareMarket* fm = _itin->fareMarket()[3];
    PricingUnit* pu = _farePath->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 4", finalPass);
  }

  void testValidateFare5()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    FareMarket* fm = _itin->fareMarket()[1];
    PricingUnit* pu = _farePath->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Minimum 1 stopover required
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMin() = "01";

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 5", !finalPass);
  }

  void testValidateFare6()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    FareMarket* fm = _itin->fareMarket()[1];
    PricingUnit* pu = _farePath->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Stopover record 3 not available for use.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.unavailTag() = 'X';
    soInfo.noStopsMin() = "01";

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 6", !finalPass);
  }

  void testValidateFare7()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    FareMarket* fm = _itin->fareMarket()[1];
    PricingUnit* pu = _farePath->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Stopover record 3 for text purpose only.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.unavailTag() = 'Y';
    soInfo.noStopsMin() = "01";

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 7", ret == SKIP);
  }

  void testValidateFare8()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    FareMarket* fm = _itin->fareMarket()[1];

    FareInfo fi;
    fi._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare f;
    f.setFareInfo(&fi);

    PaxTypeFare fare;
    fare.initialize(&f, 0, 0);

    //
    // Round trip fare always passes fare validation.
    //
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMin() = "01";

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 8", finalPass);
  }


  void testAccumulateMaxStopoversWhenStopoverLimited()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    FareMarket* fm = _itin->fareMarket()[1];
    PricingUnit* pu = _farePath->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "01";

    _soInfoWrapper->soInfo(&soInfo);
    _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    CPPUNIT_ASSERT_MESSAGE("maxStopoversPermitted should be set in Stopovers class",
                           _soInfoWrapper->maxStopoversPermitted() == 1);
  }

  void testValidateFare9()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    FareMarket* fm = _itin->fareMarket()[1];

    FareInfo fi;
    fi._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare f;
    f.setFareInfo(&fi);

    PaxTypeFare fare;
    fare.initialize(&f, 0, 0);

    //
    // Round trip fare will fail if noStopsMax = 0 and fare has a stopover.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "0";

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 9", !finalPass);
  }

  //APO-44549: max stop time in R3 is checked against the stopover time at fc level
  //the fare is failed if the so time exceeds the max time in R3
  //  Two FCs: FC1: PVG-NRT-HNL with SO in NRT
  //           FC2: HNL-OGG-PVG  with SO in OGG
  //  FC1 rec3: max stopover time is  1 day.
  void testMaxStopoverTimeExceed_FareFail()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinApo31441();

    FareMarket* fm = _itin->fareMarket()[0];
    PricingUnit* pu = _farePath->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Max stopover time is 1 day
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'D';

    _soInfoWrapper->soInfo(&soInfo);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == PASS || ret == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      Record3ReturnTypes finalRet = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass = (finalRet == PASS || finalRet == SOFTPASS);
    }
    else
    {
      finalPass = (ret == PASS || ret == SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("testMaxStopoverTimeExceed_FareFail-FcScope", !finalPass);

  }


  void testValidatePU0()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Test error condition.
    //

    const StopoversInfo* soInfo = 0;

    Record3ReturnTypes ret = _so->validate(*_trx, soInfo, *_farePath, *pu, *fu);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 0", ret == FAIL);
  }

  void testValidatePU1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    CategoryRuleInfo crInfo1;

    crInfo1.categoryNumber() = RuleConst::STOPOVER_RULE;
    crInfo1.vendorCode() = "ATP";
    crInfo1.tariffNumber() = 1;
    crInfo1.carrierCode() = "NW";
    crInfo1.ruleNumber() = "0001";

    CategoryRuleInfo crInfo2;

    crInfo2.categoryNumber() = RuleConst::STOPOVER_RULE;
    crInfo2.vendorCode() = "ATP";
    crInfo2.tariffNumber() = 2;
    crInfo2.carrierCode() = "NW";
    crInfo2.ruleNumber() = "0002";

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 1", finalPass);
  }

  void testValidatePU2()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin1();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 2", finalPass);
  }

  void testValidatePU3()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 3", finalPass);
  }

  void testValidatePU4()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[2];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited stopovers. Stop time >= 4 hours qualifies as a stopover.
    // Stopovers permitted in NYC and Zone 1.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.minStayTime() = 4;
    soInfo.minStayTimeUnit() = 'H';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    soInfoSeg2->loc1().loc() = "00001";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 4", finalPass);
  }

  void testValidatePU5()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // One stopover permitted in FRA, LON, MAD or PAR
    //
    // LHR (LON) is permitted
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.samePointsTblItemNo() = 636;
    soInfo.noStopsMax() = "01";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "FRA";
    soInfoSeg1->loc2().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc2().loc() = "LON";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg2->loc1().loc() = "MAD";
    soInfoSeg2->loc2().locType() = LOCTYPE_CITY;
    soInfoSeg2->loc2().loc() = "PAR";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 5", finalPass);
  }

  void testValidatePU6()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin3();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // No stopover permitted in NYC
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "02";
    soInfo.noStopsOutbound() = "01";
    soInfo.noStopsInbound() = "01";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->stopoverGeoAppl() = 'N';
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "NYC";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 6", !finalPass);
  }

  void testValidatePU7()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin9();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Maximum of 1 stopover permitted per location
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.samePntStops() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 7", !finalPass);
  }

  void testValidatePU8()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // One stopover permitted on either the outbound or inbound
    //  portion of travel, but not both.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "01";
    soInfo.outOrReturnInd() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 8", !finalPass);
  }

  void testValidatePU9()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[1];

    //
    // Up to 1 stopover permitted on either the outbound or inbound
    //  portion of travel.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "01";
    soInfo.outOrReturnInd() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 9", !finalPass);
  }

  void testValidatePU10()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin10();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Maximum of 1 connection permitted per location.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.samePntConnections() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 10", !finalPass);
  }

  void testValidatePU11()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // @TODO: This unit test needs some rework. Check the data in the
    //         Table993 and make sure that it is being used properly.
    //        May need to use a different Table993 and different
    //         locations (ie. a different _itin)
    //

    //
    //  This rule references a Table993 for same points data.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.samePointsTblItemNo() = 2959;
    soInfo.samePntConnections() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 11", finalPass);
  }

  void testValidatePU12()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    //  Maximum of 2 stopovers permitted, one on the outbound portion
    //   of travel and one on the inbound.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "02";
    soInfo.noStopsOutbound() = "01";
    soInfo.noStopsInbound() = "01";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 12", finalPass);
  }

  //APO-44549: MaxStayTime in R3 should is validated at FC scope.
  //At pu scope, the max time validation is done only if R3 belongs to fu being validated.
  //The so duration in JFK(fu2) is over 24 hours but max stoptime in  R3 is 24 hours.
  // fu2 belongs to pu2 and fu2 r3 is being validated ..this validation should fail.
  void testValidatePU13()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited stopovers. Maximum stay time = 24 hours
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 24;
    soInfo.maxStayTimeUnit() = 'H';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 13", !finalPass);
  }

  void testValidatePU14()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Minimum of 3 stopovers required.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMin() = "03";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 14", !finalPass);
  }

  void testValidatePU15()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Stopover record 3 not available for use.
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.unavailTag() = 'X';
    soInfo.noStopsMin() = "03";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 15", !finalPass);
  }

  void testValidatePU16()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Stopover record 3 for text purpose only.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.unavailTag() = 'Y';
    soInfo.noStopsMin() = "03";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 16", !finalPass);
  }

  void testValidatePU17()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Test error condition: Open Jaw pricing unit required.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMin() = "03";
    soInfo.ojStopoverInd() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 17", !finalPass);
  }

  void testValidatePU18()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Test error condition: Circle Trip 2 pricing unit required.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMin() = "03";
    soInfo.ct2StopoverInd() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 18", !finalPass);
  }

  void testValidatePU19()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin2();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Test error condition: Circle Trip 2+ pricing unit required.
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMin() = "03";
    soInfo.ct2PlusStopoverInd() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 19", !finalPass);
  }

  void testValidatePU20()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 20A", ret == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 20B", finalPass);
  }

  void testValidatePU21()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';
    soInfo.charge1FirstAmt() = 100;
    soInfo.charge1AddAmt() = 50;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "01";
    soInfo.charge1Cur() = "USD";
    soInfo.charge2FirstAmt() = 60;
    soInfo.charge2AddAmt() = 30;
    soInfo.charge2NoDec() = 2;
    soInfo.charge2First() = "01";
    soInfo.charge2AddNo() = "01";
    soInfo.charge2Cur() = "GBP";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1 = (retFu1 == PASS);
    bool validatePass2 = (retFu2 == PASS);
    bool finalPass = false;

    if (validatePass1 && validatePass2)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        if (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS &&
            _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS)
        {
          finalPass = true;
        }
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 21A", validatePass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 21B", validatePass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 21C", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);

    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"),
                         (*fu1->stopoverSurcharges().begin()).second->currencyCode());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());
  }

  void testValidatePU22()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';
    soInfo.charge1FirstAmt() = 100;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "XX";
    soInfo.charge1Cur() = "USD";
    soInfo.charge2FirstAmt() = 60;
    soInfo.charge2NoDec() = 2;
    soInfo.charge2First() = "01";
    soInfo.charge2AddNo() = "XX";
    soInfo.charge2Cur() = "GBP";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1 = (retFu1 == PASS);
    bool validatePass2 = (retFu2 == PASS);
    bool finalPass = false;

    if (validatePass1 && validatePass2)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        if (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS &&
            _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS)
        {
          finalPass = true;
        }
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 22A", validatePass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 22B", validatePass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 22C", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"),
                         (*fu1->stopoverSurcharges().begin()).second->currencyCode());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());
  }

  void testValidatePU23()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin5("CNN");

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';
    soInfo.charge1FirstAmt() = 100;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1Appl() = '3';
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "XX";
    soInfo.charge1Cur() = "USD";
    soInfo.charge2FirstAmt() = 60;
    soInfo.charge2NoDec() = 2;
    soInfo.charge2Appl() = '3';
    soInfo.charge2First() = "01";
    soInfo.charge2AddNo() = "XX";
    soInfo.charge2Cur() = "GBP";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1 = (retFu1 == PASS);
    bool validatePass2 = (retFu2 == PASS);
    bool finalPass = false;

    if (validatePass1 && validatePass2)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        if (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS &&
            _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS)
        {
          finalPass = true;
        }
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 23A", validatePass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 23B", validatePass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 23C", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu1->stopoverSurcharges().begin()).second->currencyCode());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());
  }

  void testValidatePU24()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin5("INF");

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';
    soInfo.charge1FirstAmt() = 100;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1Appl() = '5';
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "XX";
    soInfo.charge1Cur() = "USD";
    soInfo.charge2FirstAmt() = 60;
    soInfo.charge2NoDec() = 2;
    soInfo.charge2Appl() = '5';
    soInfo.charge2First() = "01";
    soInfo.charge2AddNo() = "XX";
    soInfo.charge2Cur() = "GBP";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1 = (retFu1 == PASS);
    bool validatePass2 = (retFu2 == PASS);
    bool finalPass = false;

    if (validatePass1 && validatePass2)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        if (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS &&
            _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS)
        {
          finalPass = true;
        }
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 24A", validatePass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 24B", validatePass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 24C", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    // Infant is discounted, but the fare amount is zero so there is no
    //  stopover surchargef for any of the stopovers.

    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->stopoverSurcharges().size());
  }

  void testValidatePU25()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin6();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';
    soInfo.charge1FirstAmt() = 100;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1Appl() = '5';
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "XX";
    soInfo.charge1Cur() = "USD";
    soInfo.charge2FirstAmt() = 60;
    soInfo.charge2NoDec() = 2;
    soInfo.charge2Appl() = '5';
    soInfo.charge2First() = "01";
    soInfo.charge2AddNo() = "XX";
    soInfo.charge2Cur() = "GBP";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1 = (retFu1 == PASS);
    bool validatePass2 = (retFu2 == PASS);
    bool finalPass = false;

    if (validatePass1 && validatePass2)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        if (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS &&
            _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS)
        {
          finalPass = true;
        }
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 25A", validatePass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 25B", validatePass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 25C", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu1->stopoverSurcharges().begin()).second->currencyCode());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());
  }

  void testValidatePU26()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin6();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Two stopovers permitted within the US, first one free second at 100USD
    // Unlimited stopovers outside the US at 100USD
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.charge1AddAmt() = 100;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1Appl() = '5';
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "XX";
    soInfo.charge1Cur() = "USD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->noStops() = "01";
    soInfoSeg1->chargeInd() = '1';
    soInfoSeg1->loc1().locType() = LOCTYPE_NATION;
    soInfoSeg1->loc1().loc() = "US";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->noStops() = "XX";
    soInfoSeg2->chargeInd() = '2';

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1 = (retFu1 == PASS);
    bool validatePass2 = (retFu2 == PASS);
    bool finalPass = false;

    if (validatePass1 && validatePass2)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        if (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS &&
            _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS)
        {
          finalPass = true;
        }
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 26A", validatePass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 26B", validatePass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 26C", finalPass);
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu2->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());
  }

  void testValidatePU27()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin6();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // The original code below does not make the scope different than PU scope.
    // To make it's FC scope the StopOverInfo maxNumber should be empty and
    // at least out oin should have the value.
    // Update puType in order to keep FC scope logic for this test case
    pu->puType() = PricingUnit::Type::ONEWAY;

    // Two stopovers permitted within the US, first one free second at 100USD
    // Unlimited stopovers outside the US at 100USD
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax() = "02";
    soInfo1.charge1AddAmt() = 100;
    soInfo1.charge1NoDec() = 2;
    soInfo1.charge1Appl() = '5';
    soInfo1.charge1First() = "01";
    soInfo1.charge1AddNo() = "XX";
    soInfo1.charge1Cur() = "USD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->noStops() = "02";
    soInfo1Seg1->chargeInd() = '1';
    soInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo1Seg1->loc1().loc() = "US";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg2);
    soInfo1Seg2->orderNo() = 2;
    soInfo1Seg2->noStops() = "XX";
    soInfo1Seg2->chargeInd() = '2';

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segs().push_back(soInfo1Seg2);
    soInfo1.segCnt() = 2;

    //
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    //  soInfo2.noStopsMax()          = "01";
    // two lines below change the scope to FC scope from PU scope
    soInfo2.noStopsMax() = "";
    soInfo2.noStopsOutbound() = "01";
    soInfo2.charge1AddAmt() = 100;
    soInfo2.charge1NoDec() = 2;
    soInfo2.charge1Appl() = '5';
    soInfo2.charge1First() = "01";
    soInfo2.charge1AddNo() = "XX";
    soInfo2.charge1Cur() = "USD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg1);
    soInfo2Seg1->noStops() = "02";
    soInfo2Seg1->chargeInd() = '1';
    soInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo2Seg1->loc1().loc() = "US";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg2);
    soInfo2Seg2->orderNo() = 2;
    soInfo2Seg2->noStops() = "XX";
    soInfo2Seg2->chargeInd() = '2';

    soInfo2.segs().push_back(soInfo2Seg1);
    soInfo2.segs().push_back(soInfo2Seg2);
    soInfo2.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes ret1Fu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret2Fu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    bool validatePass1A = (ret1Fu1 == PASS);
    bool validatePass2A = (ret2Fu1 == PASS);
    Record3ReturnTypes ret1Final = SKIP;

    if (validatePass1A && validatePass2A)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        ret1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }
    bool final1Pass = (ret1Final == PASS);

    _soInfoWrapper->clearResults();
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes ret1Fu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret2Fu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1B = (ret1Fu2 == PASS);
    bool validatePass2B = (ret2Fu2 == PASS);
    Record3ReturnTypes ret2Final = SKIP;

    if (validatePass1B && validatePass2B)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        ret2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }
    bool final2Pass = (ret2Final == PASS);

    bool finalPass = (final1Pass && final2Pass);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 27A", validatePass1A);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 27B", validatePass1B);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 27C", validatePass2A);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 27D", validatePass2B);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 27E", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu2->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());

    CPPUNIT_ASSERT((*fu2->stopoverSurcharges().begin()).second->isSegmentSpecific());
  }

  void testValidatePU28()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin3();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // No stopover permitted in Dallas
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "02";
    soInfo.noStopsOutbound() = "01";
    soInfo.noStopsInbound() = "01";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->stopoverGeoAppl() = 'N';
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "DAL";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 28", finalPass);
  }

  void testValidatePU29()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin14();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax() = "01";
    soInfo1.charge1NoDec() = 2;
    soInfo1.charge1Cur() = "EUR";

    //
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.charge1FirstAmt() = 50;
    soInfo2.charge1NoDec() = 2;
    soInfo2.charge1Cur() = "USD";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "BA";

    PaxTypeFareRuleData ptfRuleData;
    ptfRuleData.categoryRuleInfo() = &crInfo;

    CategoryRuleItemInfo criInfo;
    criInfo.setItemcat(8);
    criInfo.setOrderNo(1);
    criInfo.setItemNo(1);
    criInfo.setRelationalInd(CategoryRuleItemInfo::THEN);

    CategoryRuleItemInfoSet* crInfoSet = new CategoryRuleItemInfoSet();
    crInfoSet->push_back(criInfo);

    crInfo.addItemInfoSetNosync(crInfoSet);

    PaxTypeFare::PaxTypeFareAllRuleData ptfAllRuleData;
    ptfAllRuleData.chkedRuleData = false;
    ptfAllRuleData.chkedGfrData = false;
    ptfAllRuleData.fareRuleData = &ptfRuleData;
    ptfAllRuleData.gfrRuleData = 0;

    vector<PricingUnit*>::iterator puIter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator puIterEnd = _farePath->pricingUnit().end();
    for (; puIter != puIterEnd; ++puIter)
    {
      vector<FareUsage*>::iterator fuIter = (*puIter)->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = (*puIter)->fareUsage().end();
      for (; fuIter != fuIterEnd; ++fuIter)
        (*(*fuIter)->paxTypeFare()->paxTypeFareRuleDataMap())[8] = &ptfAllRuleData;
    }

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes ret1Fu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret2Fu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    bool validatePass1A = (ret1Fu1 == PASS);
    bool validatePass2A = (ret2Fu1 == PASS);
    Record3ReturnTypes ret1Final = SKIP;

    if (validatePass1A && validatePass2A)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        ret1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }
    bool final1Pass = (ret1Final == PASS);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes ret1Fu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret2Fu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    bool validatePass1B = (ret1Fu2 == PASS);
    bool validatePass2B = (ret2Fu2 == PASS);
    Record3ReturnTypes ret2Final = SKIP;

    if (validatePass1B && validatePass2B)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        ret2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }
    bool final2Pass = (ret2Final == PASS);

    bool finalPass = (final1Pass && final2Pass);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 29A", validatePass1A);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 29B", validatePass1B);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 29C", validatePass2A);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 29D", validatePass2B);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 29E", finalPass);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50, (*fu2->stopoverSurcharges().begin()).second->amount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NUC"),
                         (*fu2->stopoverSurcharges().begin()).second->currencyCode());
  }

  void testValidatePU_PermittedReachedSameCharge()
  {
    createItin12_PermittedReached();

    PricingUnit& pu = *_farePath->pricingUnit().front();
    FareUsage& fu = *pu.fareUsage().front();

    StopoversInfoSeg* seg1 = new StopoversInfoSeg();
    createBaseStopoversInfoSeg(seg1);
    seg1->noStops() = "01";
    seg1->loc1().locType() = LOCTYPE_NATION;
    seg1->loc1().loc() = "US";
    seg1->chargeInd() = '1';

    StopoversInfoSeg* seg2 = new StopoversInfoSeg();
    createBaseStopoversInfoSeg(seg2);
    seg2->noStops() = "XX";
    seg2->chargeInd() = '1';

    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "XX";
    soInfo.segs() += seg1, seg2;
    soInfo.segCnt() = 2;

    StopoversInfoWrapper& soInfoWrapper = createSoInfoWrapper(soInfo, pu);
    Stopovers so;

    Record3ReturnTypes ret = so.validate(*_trx, &soInfoWrapper, *_farePath, pu, fu);
    if (ret == tse::PASS && soInfoWrapper.needToProcessResults())
      ret = soInfoWrapper.processResults(*_trx, *_farePath, pu, fu);

    CPPUNIT_ASSERT_EQUAL(tse::FAIL, ret);
  }

  void testValidatePU_PermittedReachedOtherCharge()
  {
    createItin12_PermittedReached();

    PricingUnit& pu = *_farePath->pricingUnit().front();
    FareUsage& fu = *pu.fareUsage().front();

    StopoversInfoSeg* seg1 = new StopoversInfoSeg();
    createBaseStopoversInfoSeg(seg1);
    seg1->noStops() = "01";
    seg1->loc1().locType() = LOCTYPE_NATION;
    seg1->loc1().loc() = "US";
    seg1->chargeInd() = '1';

    StopoversInfoSeg* seg2 = new StopoversInfoSeg();
    createBaseStopoversInfoSeg(seg2);
    seg2->noStops() = "XX";
    seg2->chargeInd() = '2';

    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "XX";
    soInfo.segs() += seg1, seg2;
    soInfo.segCnt() = 2;

    StopoversInfoWrapper& soInfoWrapper = createSoInfoWrapper(soInfo, pu);
    Stopovers so;

    Record3ReturnTypes ret = so.validate(*_trx, &soInfoWrapper, *_farePath, pu, fu);
    if (ret == tse::PASS && soInfoWrapper.needToProcessResults())
      ret = soInfoWrapper.processResults(*_trx, *_farePath, pu, fu);

    CPPUNIT_ASSERT_EQUAL(tse::PASS, ret);
  }

  void testValidatePU_RtwIgnoreInOutOr()
  {
    createItin12_Rtw();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.noStopsMax() = "10";
    soInfo.noStopsInbound() = "00";
    soInfo.noStopsOutbound() = "00";
    soInfo.outOrReturnInd() = 'X';

    RuleSetPreprocessor rsp;
    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    rsp.process(*_trx, &crInfo, *pu, *fu);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _so->setRtw(true);
    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu);

    if (ret == PASS && _soInfoWrapper->needToProcessResults())
      ret = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu, false);

    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testLeastRestrictive1()
  {
    // 1/31/2005: AA - This unit test is no longer possible due to the changes
    //                 in CategoryRuleItemInfo and CategoryRuleItemSet for
    //                 the lazy evaluation of RuleItemInfo.

    /*
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin7();

    PricingUnit* pu  = _farePath->pricingUnit()[1];
    FareUsage*   fu1 = pu->fareUsage()[0];
    FareUsage*   fu2 = pu->fareUsage()[1];

    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax()          = "01";
    soInfo1.charge1FirstAmt()     = 10;
    soInfo1.charge1NoDec()        = 2;
    soInfo1.charge1First()        = "01";
    soInfo1.charge1Cur()          = "USD";

    //
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo()              = 2;
    soInfo2.noStopsMax()          = "02";
    soInfo2.charge1FirstAmt()     = 20;
    soInfo2.charge1NoDec()        = 2;
    soInfo2.charge1First()        = "02";
    soInfo2.charge1Cur()          = "USD";

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo1 = new CategoryRuleItemInfo();
    criInfo1->itemcat()        = 8;
    criInfo1->orderNo()        = 1;
    criInfo1->itemNo()         = 1;
    criInfo1->relationalInd()  = CategoryRuleItemInfo::THEN;
    criInfo1->inOutInd()       = ' ';
    criInfo1->directionality() = ' ';
    criInfo1->ruleItemInfo()   = &soInfo1;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo2 = new CategoryRuleItemInfo();
    criInfo2->itemcat()        = 8;
    criInfo2->orderNo()        = 2;
    criInfo2->itemNo()         = 2;
    criInfo2->relationalInd()  = CategoryRuleItemInfo::OR;
    criInfo2->inOutInd()       = ' ';
    criInfo2->directionality() = ' ';
    criInfo2->ruleItemInfo()   = &soInfo2;

    //
    StopoversInfo soInfo3;

    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo()              = 3;
    soInfo3.noStopsMax()          = "01";
    soInfo3.charge1FirstAmt()     = 10;
    soInfo3.charge1NoDec()        = 2;
    soInfo3.charge1First()        = "01";
    soInfo3.charge1Cur()          = "USD";

    //
    StopoversInfo soInfo4;

    createBaseStopoversInfo(soInfo4);
    soInfo4.itemNo()              = 4;
    soInfo4.noStopsMax()          = "02";
    soInfo4.charge1FirstAmt()     = 20;
    soInfo4.charge1NoDec()        = 2;
    soInfo4.charge1First()        = "02";
    soInfo4.charge1Cur()          = "USD";

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo3 = new CategoryRuleItemInfo();
    criInfo3->itemcat()        = 8;
    criInfo3->orderNo()        = 3;
    criInfo3->itemNo()         = 3;
    criInfo3->relationalInd()  = CategoryRuleItemInfo::THEN;
    criInfo3->inOutInd()       = ' ';
    criInfo3->directionality() = ' ';
    criInfo3->ruleItemInfo()   = &soInfo3;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo4 = new CategoryRuleItemInfo();
    criInfo4->itemcat()        = 8;
    criInfo4->orderNo()        = 4;
    criInfo4->itemNo()         = 4;
    criInfo4->relationalInd()  = CategoryRuleItemInfo::AND;
    criInfo4->inOutInd()       = ' ';
    criInfo4->directionality() = ' ';
    criInfo4->ruleItemInfo()   = &soInfo4;

    CategoryRuleInfo crInfo1;

    createBaseCategoryRuleInfo(crInfo1);

    CategoryRuleInfo crInfo2;

    createBaseCategoryRuleInfo(crInfo2);
    crInfo2.tariffNumber()   = 2;
    crInfo2.ruleNumber()     = "0002";

    CategoryRuleItemInfoSet criInfoSet1;

    criInfoSet1.push_back(criInfo1);
    criInfoSet1.push_back(criInfo2);

    CategoryRuleItemInfoSet criInfoSet2;

    criInfoSet2.push_back(criInfo3);
    criInfoSet2.push_back(criInfo4);

    std::vector<CategoryRuleItemInfoSet*> criSet;

    criSet.push_back(&criInfoSet1);
    criSet.push_back(&criInfoSet2);

    CategoryRuleItemSet cris;

    Record3ReturnTypes ret1 =
      cris.process(*_trx, crInfo1, *_farePath, *pu, *fu1, criSet, false);

    Record3ReturnTypes ret2 =
      cris.process(*_trx, crInfo1, *_farePath, *pu, *fu2, criSet, false);


    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 1A",
                           ret1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 1B",
                           ret2 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 1C",
                           fu1->stopovers().size() == 1 &&
                           fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 1D",
                           fu1->stopoverSurcharges().size() == 1 &&
                           fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 1E",
           (*fu1->stopoverSurcharges().begin()).second->amount() == 20 &&
           (*fu1->stopoverSurcharges().begin()).second->currencyCode() == "USD");

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 1F",
           (*fu2->stopoverSurcharges().begin()).second->amount() == 20 &&
           (*fu2->stopoverSurcharges().begin()).second->currencyCode() == "USD");
    */
  }

  void testLeastRestrictive2()
  {
    // 1/31/2005: AA - This unit test is no longer possible due to the changes
    //                 in CategoryRuleItemInfo and CategoryRuleItemSet for
    //                 the lazy evaluation of RuleItemInfo.

    /*
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin7();

    PricingUnit* pu  = _farePath->pricingUnit()[1];
    FareUsage*   fu1 = pu->fareUsage()[0];
    FareUsage*   fu2 = pu->fareUsage()[1];

    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax()          = "01";
    soInfo1.charge1FirstAmt()     = 10;
    soInfo1.charge1NoDec()        = 2;
    soInfo1.charge1First()        = "01";
    soInfo1.charge1Cur()          = "USD";

    //
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo()              = 2;
    soInfo2.noStopsMax()          = "02";
    soInfo2.charge1FirstAmt()     = 20;
    soInfo2.charge1NoDec()        = 2;
    soInfo2.charge1First()        = "02";
    soInfo2.charge1Cur()          = "USD";

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo1 = new CategoryRuleItemInfo();
    criInfo1->itemcat()        = 8;
    criInfo1->orderNo()        = 1;
    criInfo1->itemNo()         = 1;
    criInfo1->relationalInd()  = CategoryRuleItemInfo::THEN;
    criInfo1->inOutInd()       = ' ';
    criInfo1->directionality() = ' ';
    criInfo1->ruleItemInfo()   = &soInfo1;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo2 = new CategoryRuleItemInfo();
    criInfo2->itemcat()        = 8;
    criInfo2->orderNo()        = 2;
    criInfo2->itemNo()         = 2;
    criInfo2->relationalInd()  = CategoryRuleItemInfo::OR;
    criInfo2->inOutInd()       = ' ';
    criInfo2->directionality() = ' ';
    criInfo2->ruleItemInfo()   = &soInfo2;

    //
    StopoversInfo soInfo3;

    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo()              = 3;
    soInfo3.noStopsMax()          = "01";
    soInfo3.charge1FirstAmt()     = 10;
    soInfo3.charge1NoDec()        = 2;
    soInfo3.charge1First()        = "01";
    soInfo3.charge1Cur()          = "USD";

    //
    StopoversInfo soInfo4;

    createBaseStopoversInfo(soInfo4);
    soInfo4.itemNo()              = 4;
    soInfo4.noStopsMax()          = "02";
    soInfo4.charge1FirstAmt()     = 20;
    soInfo4.charge1NoDec()        = 2;
    soInfo4.charge1First()        = "02";
    soInfo4.charge1Cur()          = "USD";

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo3 = new CategoryRuleItemInfo();
    criInfo3->itemcat()        = 8;
    criInfo3->orderNo()        = 3;
    criInfo3->itemNo()         = 3;
    criInfo3->relationalInd()  = CategoryRuleItemInfo::THEN;
    criInfo3->inOutInd()       = ' ';
    criInfo3->directionality() = ' ';
    criInfo3->ruleItemInfo()   = &soInfo3;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo4 = new CategoryRuleItemInfo();
    criInfo4->itemcat()        = 8;
    criInfo4->orderNo()        = 4;
    criInfo4->itemNo()         = 4;
    criInfo4->relationalInd()  = CategoryRuleItemInfo::OR;
    criInfo4->inOutInd()       = ' ';
    criInfo4->directionality() = ' ';
    criInfo4->ruleItemInfo()   = &soInfo4;

    CategoryRuleInfo crInfo1;

    createBaseCategoryRuleInfo(crInfo1);

    CategoryRuleInfo crInfo2;

    crInfo2.categoryNumber() = RuleConst::STOPOVER_RULE;
    createBaseCategoryRuleInfo(crInfo2);
    crInfo2.tariffNumber()   = 2;
    crInfo2.ruleNumber()     = "0002";

    CategoryRuleItemInfoSet criInfoSet1;

    criInfoSet1.push_back(criInfo1);
    criInfoSet1.push_back(criInfo2);

    CategoryRuleItemInfoSet criInfoSet2;

    criInfoSet2.push_back(criInfo3);
    criInfoSet2.push_back(criInfo4);

    std::vector<CategoryRuleItemInfoSet*> criSet;

    criSet.push_back(&criInfoSet1);
    criSet.push_back(&criInfoSet2);

    CategoryRuleItemSet cris;

    Record3ReturnTypes ret1 =
      cris.process(*_trx, crInfo1, *_farePath, *pu, *fu1, criSet, false);

    Record3ReturnTypes ret2 =
      cris.process(*_trx, crInfo1, *_farePath, *pu, *fu2, criSet, false);


    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 2A",
                           ret1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 2B",
                           ret2 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 2C",
                           fu1->stopovers().size() == 1 &&
                           fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 2D",
                           fu1->stopoverSurcharges().size() == 1 &&
                           fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 2E",
           (*fu1->stopoverSurcharges().begin()).second->amount() == 20 &&
           (*fu1->stopoverSurcharges().begin()).second->currencyCode() == "USD");

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 2F",
           (*fu2->stopoverSurcharges().begin()).second->amount() == 20 &&
           (*fu2->stopoverSurcharges().begin()).second->currencyCode() == "USD");
    */
  }

  void testLeastRestrictive3()
  {
    // 1/31/2005: AA - This unit test is no longer possible due to the changes
    //                 in CategoryRuleItemInfo and CategoryRuleItemSet for
    //                 the lazy evaluation of RuleItemInfo.

    /*
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin7();

    PricingUnit* pu  = _farePath->pricingUnit()[1];
    FareUsage*   fu1 = pu->fareUsage()[0];
    FareUsage*   fu2 = pu->fareUsage()[1];

    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax()          = "01";
    soInfo1.charge1FirstAmt()     = 10;
    soInfo1.charge1NoDec()        = 2;
    soInfo1.charge1First()        = "01";
    soInfo1.charge1Cur()          = "USD";

    //
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo()              = 2;
    soInfo2.charge1FirstAmt()     = 20;
    soInfo2.charge1NoDec()        = 2;
    soInfo2.charge1First()        = "02";
    soInfo2.charge1Cur()          = "USD";

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo1 = new CategoryRuleItemInfo();
    criInfo1->itemcat()        = 8;
    criInfo1->orderNo()        = 1;
    criInfo1->itemNo()         = 1;
    criInfo1->relationalInd()  = CategoryRuleItemInfo::THEN;
    criInfo1->inOutInd()       = ' ';
    criInfo1->directionality() = ' ';
    criInfo1->ruleItemInfo()   = &soInfo1;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo2 = new CategoryRuleItemInfo();
    criInfo2->itemcat()        = 8;
    criInfo2->orderNo()        = 2;
    criInfo2->itemNo()         = 2;
    criInfo2->relationalInd()  = CategoryRuleItemInfo::OR;
    criInfo2->inOutInd()       = ' ';
    criInfo2->directionality() = ' ';
    criInfo2->ruleItemInfo()   = &soInfo2;

    //
    StopoversInfo soInfo3;

    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo()              = 3;
    soInfo3.noStopsMax()          = "01";
    soInfo3.charge1FirstAmt()     = 10;
    soInfo3.charge1NoDec()        = 2;
    soInfo3.charge1First()        = "01";
    soInfo3.charge1Cur()          = "USD";

    //
    StopoversInfo soInfo4;

    createBaseStopoversInfo(soInfo4);
    soInfo4.itemNo()              = 4;
    soInfo4.noStopsMax()          = "02";
    soInfo4.charge1FirstAmt()     = 20;
    soInfo4.charge1NoDec()        = 2;
    soInfo4.charge1First()        = "02";
    soInfo4.charge1Cur()          = "USD";

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo3 = new CategoryRuleItemInfo();
    criInfo3->itemcat()        = 8;
    criInfo3->orderNo()        = 3;
    criInfo3->itemNo()         = 3;
    criInfo3->relationalInd()  = CategoryRuleItemInfo::THEN;
    criInfo3->inOutInd()       = ' ';
    criInfo3->directionality() = ' ';
    criInfo3->ruleItemInfo()   = &soInfo3;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo* criInfo4 = new CategoryRuleItemInfo();
    criInfo4->itemcat()        = 8;
    criInfo4->orderNo()        = 4;
    criInfo4->itemNo()         = 4;
    criInfo4->relationalInd()  = CategoryRuleItemInfo::AND;
    criInfo4->inOutInd()       = ' ';
    criInfo4->directionality() = ' ';
    criInfo4->ruleItemInfo()   = &soInfo4;

    CategoryRuleInfo crInfo1;

    createBaseCategoryRuleInfo(crInfo1);

    CategoryRuleInfo crInfo2;

    createBaseCategoryRuleInfo(crInfo2);
    crInfo2.tariffNumber()   = 2;
    crInfo2.ruleNumber()     = "0002";

    CategoryRuleItemInfoSet criInfoSet1;

    criInfoSet1.push_back(criInfo1);
    criInfoSet1.push_back(criInfo2);

    CategoryRuleItemInfoSet criInfoSet2;

    criInfoSet2.push_back(criInfo3);
    criInfoSet2.push_back(criInfo4);

    std::vector<CategoryRuleItemInfoSet*> criSet;

    criSet.push_back(&criInfoSet1);
    criSet.push_back(&criInfoSet2);

    CategoryRuleItemSet cris;

    Record3ReturnTypes ret1 =
      cris.process(*_trx, crInfo1, *_farePath, *pu, *fu1, criSet, false);

    Record3ReturnTypes ret2 =
      cris.process(*_trx, crInfo1, *_farePath, *pu, *fu2, criSet, false);


    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 3A",
                           ret1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 3B",
                           ret2 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 3C",
                           fu1->stopovers().size() == 1 &&
                           fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 3D",
                           fu1->stopoverSurcharges().size() == 1 &&
                           fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 3E",
           (*fu1->stopoverSurcharges().begin()).second->amount() == 20 &&
           (*fu1->stopoverSurcharges().begin()).second->currencyCode() == "USD");

    CPPUNIT_ASSERT_MESSAGE("Error: Least Restrictive - Test 3F",
           (*fu2->stopoverSurcharges().begin()).second->amount() == 20 &&
           (*fu2->stopoverSurcharges().begin()).second->currencyCode() == "USD");
    */
  }

  void testGatewayRestrictions1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin6();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];

    // Stopovers only permitted at US and GB gateways
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.gtwyInd() = 'P';
    soInfo1.noStopsMax() = "04";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->noStops() = "02";
    soInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo1Seg1->loc1().loc() = "US";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg2);

    soInfo1Seg2->orderNo() = 2;
    soInfo1Seg2->noStops() = "02";
    soInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    soInfo1Seg2->loc1().loc() = "GB";

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segs().push_back(soInfo1Seg2);
    soInfo1.segCnt() = 2;

    // Stopover not permitted at US gateway
    //
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.gtwyInd() = 'N';
    soInfo2.noStopsMax() = "02";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg1);
    soInfo2Seg1->noStops() = "XX";
    soInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo2Seg1->loc1().loc() = "US";

    soInfo2.segs().push_back(soInfo2Seg1);
    soInfo2.segCnt() = 1;

    // Stopover only permitted at gateways
    //
    StopoversInfo soInfo3;

    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo() = 3;
    soInfo3.gtwyInd() = 'P';
    soInfo3.noStopsMax() = "02";

    // Stopover not permitted at gateways
    //
    StopoversInfo soInfo4;

    createBaseStopoversInfo(soInfo4);
    soInfo4.itemNo() = 4;
    soInfo4.gtwyInd() = 'N';
    soInfo4.noStopsMax() = "02";

    // Stopover only permitted at transatlantic gateway
    //
    StopoversInfo soInfo5;

    createBaseStopoversInfo(soInfo5);
    soInfo5.itemNo() = 5;
    soInfo5.geoTblItemNoGateway() = 205; // 995 has TSI 31 with no geo
    soInfo5.gtwyInd() = 'P';
    soInfo5.noStopsMax() = "02";

    // Stopover required at transatlantic gateway except within the US
    //
    StopoversInfo soInfo6;

    createBaseStopoversInfo(soInfo6);
    soInfo6.itemNo() = 6;
    soInfo6.geoTblItemNoGateway() = 205; // 995 has TSI 31 with no geo
    soInfo6.gtwyInd() = 'P';
    soInfo6.noStopsMax() = "02";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo6Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo6Seg1);
    soInfo6Seg1->noStops() = "XX";
    soInfo6Seg1->stopoverGeoAppl() = 'N';
    soInfo6Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo6Seg1->loc1().loc() = "US";

    soInfo6.segs().push_back(soInfo6Seg1);
    soInfo6.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    _soInfoWrapper->fareUsage() = fu1;

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes ret1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo3);
    Record3ReturnTypes ret3 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo4);
    Record3ReturnTypes ret4 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo5);
    Record3ReturnTypes ret5 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->clearResults();
    _soInfoWrapper->soInfo(&soInfo6);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    Record3ReturnTypes ret6 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes ret6final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);

    CPPUNIT_ASSERT_MESSAGE("Error: GatewayRestrictions - Test 1A", ret1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: GatewayRestrictions - Test 1B", ret2 == FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error: GatewayRestrictions - Test 1C", ret3 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: GatewayRestrictions - Test 1D", ret4 == FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error: GatewayRestrictions - Test 1E", ret5 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: GatewayRestrictions - Test 1F",
                           ret6 == PASS && ret6final == FAIL);
  }

  void testCarrierRestrictions1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;

    createItin11();

    PricingUnit* pu1 = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu1->fareUsage()[0];

    // All stopovers must have the same inbound and outbound carriers
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.sameCarrierInd() = 'X';

    // Stopovers at the departure point of overwater segments must have the
    //  same inbound and outbound carriers.
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.geoTblItemNoBtw() = 14; // 995 has TSI 7 with no geo
    soInfo2.sameCarrierInd() = 'X';

    // Stopovers within the US must have the same inbound and outbound carriers.
    StopoversInfo soInfo3;

    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo() = 3;
    soInfo3.geoTblItemNoBtw() = 122; // 995 has no TSI. Loc = US
    soInfo3.sameCarrierInd() = 'X';

    // Stopovers between and including the departure from the PU origin
    //  and the arrival at the PU destination must have the same
    //  inbound and outbound carriers.
    StopoversInfo soInfo4;

    createBaseStopoversInfo(soInfo4);
    soInfo4.itemNo() = 4;
    soInfo4.geoTblItemNoBtw() = 3; // 995 has TSI 01 with no geo
    soInfo4.geoTblItemNoAnd() = 78; // 995 has TSI 10 with no geo
    soInfo4.sameCarrierInd() = 'X';

    // Stopovers between and including the departure from the PU origin
    //  and the arrival at the PU destination must have the same
    //  inbound and outbound carriers.
    //
    // Same as the test above except with the table 995's reversed.
    //  The result should be the same.
    StopoversInfo soInfo5;

    createBaseStopoversInfo(soInfo5);
    soInfo5.itemNo() = 5;
    soInfo5.geoTblItemNoBtw() = 78; // 995 has TSI 10 with no geo
    soInfo5.geoTblItemNoAnd() = 3; // 995 has TSI 01 with no geo
    soInfo5.sameCarrierInd() = 'X';

    // Stopovers at origin or destination of domestic sectors must
    //  have the same inbound and outbound carriers.
    StopoversInfo soInfo6;

    createBaseStopoversInfo(soInfo6);
    soInfo6.itemNo() = 6;
    soInfo6.geoTblItemNoBtw() = 437; // 995 has TSI 19 with no geo
    soInfo6.sameCarrierInd() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu1, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu1);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes ret1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu1);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu1);

    _soInfoWrapper->soInfo(&soInfo3);
    Record3ReturnTypes ret3 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu1);

    _soInfoWrapper->soInfo(&soInfo4);
    Record3ReturnTypes ret4 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu1);

    _soInfoWrapper->soInfo(&soInfo5);
    Record3ReturnTypes ret5 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu1);

    _soInfoWrapper->soInfo(&soInfo6);
    Record3ReturnTypes ret6 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu1);

    CPPUNIT_ASSERT_MESSAGE("Error: CarrierRestrictions - Test 1A", ret1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: CarrierRestrictions - Test 1B", ret2 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: CarrierRestrictions - Test 1C", ret3 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: CarrierRestrictions - Test 1D", ret4 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: CarrierRestrictions - Test 1E", ret5 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error: CarrierRestrictions - Test 1F", ret6 == PASS);
  }

  void testSurfaceSectors1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin13();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // No stopover permitted in London
    //

    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->stopoverGeoAppl() = 'N';
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "LON";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu1;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    bool finalPass = false;

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error Surface Sectors: Test 1", !finalPass);

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->stopoverGeoAppl() = 'N';
    soInfoSeg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfoSeg1->loc1().loc() = "JFK";

    _soInfoWrapper->clearResults();
    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu2;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    ret = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    if ((ret == PASS) && _soInfoWrapper->needToProcessResults())
    {
      finalPass = (_soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) == PASS);
    }
    else
    {
      finalPass = (ret == PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error Surface Sectors: Test 2", !finalPass);
  }

  void testRegressionPL9779()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionPL9779();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Two stopovers permitted within LON, first one free second at 50EUR
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.charge1AddAmt() = 50;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1Appl() = '3';
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "01";
    soInfo.charge1Cur() = "EUR";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->noStops() = "01";
    soInfoSeg1->chargeInd() = '1';
    soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg1->loc1().loc() = "LON";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->noStops() = "01";
    soInfoSeg2->chargeInd() = '2';
    soInfoSeg2->loc1().locType() = LOCTYPE_CITY;
    soInfoSeg2->loc1().loc() = "LON";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 21;
    crInfo.carrierCode() = "BA";
    crInfo.ruleNumber() = "2111";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->fareUsage() = fu1;
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu1B = SKIP;
    if (retFu1A == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1B = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->clearResults();
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    _soInfoWrapper->fareUsage() = fu2;
    Record3ReturnTypes retFu2A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    Record3ReturnTypes retFu2B = SKIP;
    if (retFu2A == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2B = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test B", retFu2A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test C",
                           (retFu1B == PASS) && (retFu2B == PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test D",
                           fu1->stopovers().size() == 1 && fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test E",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test F",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 0);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8779 - Test G",
                           (*fu2->stopoverSurcharges().begin()).second->amount() == 50 &&
                               (*fu2->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "EUR");
  }

  void testRegressionPL8820()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionPL8820();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Two free stopovers in Area 3, but not in China
    //  plus unlimited stopovers anywhere else at 125SGD or 600HKD each
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.charge1AddAmt() = 125;
    soInfo.charge1First() = "02";
    soInfo.charge1AddNo() = "XX";
    soInfo.charge1Cur() = "SGD";
    soInfo.charge2AddAmt() = 600;
    soInfo.charge2First() = "02";
    soInfo.charge2AddNo() = "XX";
    soInfo.charge2Cur() = "HKD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg1);
    soInfoSeg1->stopoverGeoAppl() = 'N';
    soInfoSeg1->loc1().locType() = LOCTYPE_NATION;
    soInfoSeg1->loc1().loc() = "CN";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfoSeg2);
    soInfoSeg2->orderNo() = 2;
    soInfoSeg2->noStops() = "02";
    soInfoSeg2->chargeInd() = '1';
    soInfoSeg2->loc1().locType() = LOCTYPE_AREA;
    soInfoSeg2->loc1().loc() = "3";

    soInfo.segs().push_back(soInfoSeg1);
    soInfo.segs().push_back(soInfoSeg2);
    soInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 3;
    crInfo.carrierCode() = "SQ";
    crInfo.ruleNumber() = "5375";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->fareUsage() = fu1;

    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu1B = SKIP;
    if (retFu1A == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1B = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->fareUsage() = fu2;
    Record3ReturnTypes retFu2A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    Record3ReturnTypes retFu2B = SKIP;
    if (retFu2A == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2B = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test B", retFu2A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test C",
                           (retFu1B == PASS) && (retFu2B == PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test D",
                           fu1->stopovers().size() == 1 && fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test E",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test F",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 0);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8820 - Test G",
                           (*fu2->stopoverSurcharges().begin()).second->amount() == 125 &&
                               (*fu2->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "SGD");
  }

  void testRegressionPL8848()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionPL8848();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Update puType in order to keep FC scope logic for this test case
    pu->puType() = PricingUnit::Type::ONEWAY;

    // 3 free stopovers permitted anywhere except HNL
    // AND 1 stopover permitted in HNL at 2000CNY or any other point at 1250CNY.
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax() = "03";
    soInfo1.charge1Appl() = '5';
    soInfo1.charge1Cur() = "CNY";
    soInfo1.charge2Cur() = "CNY";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->stopoverGeoAppl() = 'N';
    soInfo1Seg1->chargeInd() = '1';
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "HNL";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg2);

    soInfo1Seg2->orderNo() = 2;
    soInfo1Seg2->noStops() = "03";
    soInfo1Seg2->chargeInd() = '1';

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segs().push_back(soInfo1Seg2);
    soInfo1.segCnt() = 2;

    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.noStopsMax() = "01";
    soInfo2.charge1FirstAmt() = 1250;
    soInfo2.charge1AddAmt() = 2000;
    soInfo2.charge1Appl() = '5';
    soInfo2.charge1First() = "01";
    soInfo2.charge1AddNo() = "01";
    soInfo2.charge1Cur() = "CNY";
    soInfo2.charge2Cur() = "CNY";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg1);
    soInfo2Seg1->stopoverGeoAppl() = 'N';
    soInfo2Seg1->chargeInd() = '1';
    soInfo2Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo2Seg1->loc1().loc() = "HNL";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg2);
    soInfo2Seg2->orderNo() = 2;
    soInfo2Seg2->noStops() = "01";
    soInfo2Seg2->chargeInd() = '1';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg3 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg3);

    soInfo2Seg3->orderNo() = 3;
    soInfo2Seg3->noStops() = "01";
    soInfo2Seg3->chargeInd() = '2';
    soInfo2Seg3->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo2Seg3->loc1().loc() = "HNL";

    soInfo2.segs().push_back(soInfo2Seg1);
    soInfo2.segs().push_back(soInfo2Seg2);
    soInfo2.segs().push_back(soInfo2Seg3);
    soInfo2.segCnt() = 3;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 3;
    crInfo.carrierCode() = "UA";
    crInfo.ruleNumber() = "5815";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu1B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    Record3ReturnTypes retFu1Final = SKIP;
    if ((retFu1A == PASS) && (retFu1B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu2;
    Record3ReturnTypes retFu2A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu2B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    Record3ReturnTypes retFu2Final = SKIP;
    if ((retFu2A == PASS) && (retFu2B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test B", retFu1B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test C", retFu2A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test D", retFu2B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test E",
                           (retFu1Final == PASS) && (retFu2Final == PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test F",
                           fu1->stopovers().size() == 3 && fu2->stopovers().size() == 0);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test G",
                           fu1->stopoverSurcharges().size() == 3 &&
                               fu2->stopoverSurcharges().size() == 0);

    FareUsage::StopoverSurchargeMultiMapCI iter =
        fu1->stopoverSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test H",
                           (*iter).second->amount() == 0);

    iter = fu1->stopoverSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test I",
                           (*iter).second->amount() == 2000 &&
                               (*iter).second->currencyCode() == "CNY");

    iter = fu1->stopoverSurcharges().find(fu1->travelSeg()[2]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8848 - Test J",
                           (*iter).second->amount() == 0);
  }

  void testRegressionPL8868()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionPL8868();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // 2 stopovers permitted, 1 in LON at 50EUR plus 1 in LON at 100EUR
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax() = "02";
    soInfo1.charge1FirstAmt() = 50;
    soInfo1.charge1AddAmt() = 100;
    soInfo1.charge1NoDec() = 2;
    soInfo1.charge1First() = "01";
    soInfo1.charge1AddNo() = "01";
    soInfo1.charge1Cur() = "EUR";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->chargeInd() = '1';
    soInfo1Seg1->loc1().locType() = LOCTYPE_CITY;
    soInfo1Seg1->loc1().loc() = "LON";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg2 = new StopoversInfoSeg();
    createBaseStopoversInfoSeg(soInfo1Seg2);

    soInfo1Seg2->orderNo() = 2;
    soInfo1Seg2->chargeInd() = '2';
    soInfo1Seg2->loc1().locType() = LOCTYPE_CITY;
    soInfo1Seg2->loc1().loc() = "LON";

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segs().push_back(soInfo1Seg2);
    soInfo1.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 21;
    crInfo.carrierCode() = "BA";
    crInfo.ruleNumber() = "5595";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    Record3ReturnTypes retFu1Final = SKIP;
    if (retFu1 == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->clearResults();
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    _soInfoWrapper->fareUsage() = fu2;
    Record3ReturnTypes retFu2 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    Record3ReturnTypes retFu2Final = SKIP;
    if (retFu2 == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test A", retFu1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test B", retFu2 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test C", retFu1Final == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test C2", retFu2Final == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test D",
                           fu1->stopovers().size() == 1 && fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test E",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test F",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 50 &&
                               (*fu1->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "EUR");

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8868 - Test G",
                           (*fu2->stopoverSurcharges().begin()).second->amount() == 100 &&
                               (*fu2->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "EUR");
  }

  void testRegressionPL9755()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionPL9755();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Update puType in order to keep FC scope logic for this test case
    pu->puType() = PricingUnit::Type::ONEWAY;

    // 3 free stopovers permitted anywhere except HNL
    // AND 1 stopover permitted in HNL at 1500CNY or any other point at 1250CNY.
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax() = "03";
    soInfo1.charge1Appl() = '5';
    soInfo1.charge1Cur() = "CNY";
    soInfo1.charge2Cur() = "CNY";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->stopoverGeoAppl() = 'N';
    soInfo1Seg1->chargeInd() = '1';
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "HNL";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg2);

    soInfo1Seg2->orderNo() = 2;
    soInfo1Seg2->noStops() = "03";
    soInfo1Seg2->chargeInd() = '1';

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segs().push_back(soInfo1Seg2);
    soInfo1.segCnt() = 2;

    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.noStopsMax() = "01";
    soInfo2.charge1FirstAmt() = 1250;
    soInfo2.charge1AddAmt() = 1500;
    soInfo2.charge1Appl() = '5';
    soInfo2.charge1First() = "01";
    soInfo2.charge1AddNo() = "01";
    soInfo2.charge1Cur() = "CNY";
    soInfo2.charge2Cur() = "CNY";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg1);
    soInfo2Seg1->stopoverGeoAppl() = 'N';
    soInfo2Seg1->chargeInd() = '1';
    soInfo2Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo2Seg1->loc1().loc() = "HNL";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg2);
    soInfo2Seg2->orderNo() = 2;
    soInfo2Seg2->noStops() = "01";
    soInfo2Seg2->chargeInd() = '1';

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg3 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo2Seg3);

    soInfo2Seg3->orderNo() = 3;
    soInfo2Seg3->noStops() = "01";
    soInfo2Seg3->chargeInd() = '2';
    soInfo2Seg3->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo2Seg3->loc1().loc() = "HNL";

    soInfo2.segs().push_back(soInfo2Seg1);
    soInfo2.segs().push_back(soInfo2Seg2);
    soInfo2.segs().push_back(soInfo2Seg3);
    soInfo2.segCnt() = 3;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 3;
    crInfo.carrierCode() = "UA";
    crInfo.ruleNumber() = "5815";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu1B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    Record3ReturnTypes retFu1Final = SKIP;
    if ((retFu1A == PASS) && (retFu1B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes retFu2A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu2B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    Record3ReturnTypes retFu2Final = SKIP;
    if ((retFu2A == PASS) && (retFu2B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test B", retFu1B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test C", retFu2A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test D", retFu2B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test E",
                           (retFu1Final == PASS) && (retFu2Final == PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test F",
                           fu1->stopovers().size() == 3 && fu2->stopovers().size() == 0);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test G",
                           fu1->stopoverSurcharges().size() == 3 &&
                               fu2->stopoverSurcharges().size() == 0);

    FareUsage::StopoverSurchargeMultiMapCI iter =
        fu1->stopoverSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test H",
                           (*iter).second->amount() == 0);

    iter = fu1->stopoverSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test I",
                           (*iter).second->amount() == 1500 &&
                               (*iter).second->currencyCode() == "CNY");

    iter = fu1->stopoverSurcharges().find(fu1->travelSeg()[2]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 9755 - Test J",
                           (*iter).second->amount() == 0);
  }

  void testRegressionPL10364()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;

    createItinRegressionPL10364();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    //
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.charge2Cur() = "USD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->chargeInd() = '1';
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "REK";

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segCnt() = 1;

    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.geoTblItemNoGateway() = 122;
    soInfo2.gtwyInd() = 'Y';
    soInfo2.noStopsMax() = "01";
    soInfo2.charge1FirstAmt() = 50;
    soInfo2.charge1Appl() = '5';
    soInfo2.charge1Cur() = "USD";
    soInfo2.charge2Cur() = "USD";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "FI";
    crInfo.ruleNumber() = "2650";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    fu1->stopoverSurcharges().clear();
    fu2->stopoverSurcharges().clear();
    _soInfoWrapper->clearResults();
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->clearResults();
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu1B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    Record3ReturnTypes retFu1Final = SKIP;
    if ((retFu1A == PASS) && (retFu1B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->clearResults();
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu2;
    Record3ReturnTypes retFu2A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu2B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    Record3ReturnTypes retFu2Final = SKIP;
    if ((retFu2A == PASS) && (retFu2B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 10364 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 10364 - Test B", retFu1B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 10364 - Test C", retFu2A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 10364 - Test D", retFu2B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 10364 - Test E",
                           (retFu1Final == FAIL) && (retFu2Final == FAIL));
  }

  void testRegressionPL19790()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;

    createItinRegressionPL19790();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    // 2 STOPOVERS PERMITTED IN MSP AT USD 100.00
    // OR - 1 STOPOVER PERMITTED AT USD 50.00
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.itemNo() = 400575;
    soInfo1.noStopsMax() = "02";
    soInfo1.charge1FirstAmt() = 100;
    soInfo1.charge1NoDec() = 2;
    soInfo1.charge1Appl() = '5';
    soInfo1.charge1Cur() = "USD";
    soInfo1.charge2Cur() = "USD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "MSP";

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segCnt() = 1;

    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 64324;
    soInfo2.noStopsMax() = "01";
    soInfo2.charge1FirstAmt() = 50;
    soInfo2.charge1NoDec() = 2;
    soInfo2.charge1Appl() = '5';
    soInfo2.charge1Cur() = "USD";
    soInfo2.charge2Cur() = "USD";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 389;
    crInfo.carrierCode() = "BA";
    crInfo.ruleNumber() = "JP01";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu1B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    Record3ReturnTypes retFu1Final = SKIP;
    if ((retFu1A == PASS) && (retFu1B == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 19790 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 19790 - Test B", retFu1B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 19790 - Test C", retFu1Final == FAIL);
  }

  void testRegressionSPR83301()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionSPR83301();
    _itin->calculationCurrency() = "NUC";

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    //
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.itemNo() = 9140;
    soInfo1.noStopsMax() = "01";
    soInfo1.charge1Cur() = "USD";
    soInfo1.charge2Cur() = "USD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->chargeInd() = '1';
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "FRA";

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segCnt() = 1;

    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 33993;
    soInfo2.gtwyInd() = 'Y';
    soInfo2.geoTblItemNoGateway() = 122;
    soInfo2.noStopsMax() = "01";
    soInfo2.charge1Cur() = "USD";
    soInfo2.charge2Cur() = "USD";

    StopoversInfo soInfo3;

    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo() = 179520;
    soInfo3.noStopsMax() = "01";
    soInfo3.charge1FirstAmt() = 30;
    soInfo3.charge1NoDec() = 3;
    soInfo3.charge1Cur() = "KWD";
    soInfo3.charge2NoDec() = 3;
    soInfo3.charge2Cur() = "KWD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo3Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo3Seg1);
    soInfo3Seg1->chargeInd() = '1';
    soInfo3Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo3Seg1->loc1().loc() = "AT";
    soInfo3Seg1->loc2().locType() = LOCTYPE_NATION;
    soInfo3Seg1->loc2().loc() = "CH";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo3Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo3Seg2);
    soInfo3Seg2->orderNo() = 2;
    soInfo3Seg2->chargeInd() = '1';
    soInfo3Seg2->loc1().locType() = LOCTYPE_NATION;
    soInfo3Seg2->loc1().loc() = "DE";
    soInfo3Seg2->loc2().locType() = LOCTYPE_NATION;
    soInfo3Seg2->loc2().loc() = "FR";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo3Seg3 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo3Seg3);
    soInfo3Seg3->orderNo() = 3;
    soInfo3Seg3->chargeInd() = '1';
    soInfo3Seg3->loc1().locType() = LOCTYPE_NATION;
    soInfo3Seg3->loc1().loc() = "GB";
    soInfo3Seg3->loc2().locType() = LOCTYPE_NATION;
    soInfo3Seg3->loc2().loc() = "NL";

    soInfo3.segs().push_back(soInfo3Seg1);
    soInfo3.segs().push_back(soInfo3Seg2);
    soInfo3.segs().push_back(soInfo3Seg3);
    soInfo3.segCnt() = 3;

    StopoversInfo soInfo4;

    createBaseStopoversInfo(soInfo4);
    soInfo4.itemNo() = 235639;
    soInfo4.noStopsMax() = "01";
    soInfo4.charge1FirstAmt() = 75;
    soInfo4.charge1AddAmt() = 0;
    soInfo4.charge1NoDec() = 3;
    soInfo4.charge1Cur() = "KWD";
    soInfo4.charge2NoDec() = 3;
    soInfo4.charge2Cur() = "KWD";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo4Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo4Seg1);
    soInfo4Seg1->chargeInd() = '1';
    soInfo4Seg1->loc1().locType() = LOCTYPE_NATION;
    soInfo4Seg1->loc1().loc() = "ES";

    soInfo4.segs().push_back(soInfo4Seg1);
    soInfo4.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "LH";
    crInfo.ruleNumber() = "W313";
    crInfo.sequenceNumber() = 10001;

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes retFu1A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu1B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    _soInfoWrapper->soInfo(&soInfo3);
    Record3ReturnTypes retFu1C = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    _soInfoWrapper->soInfo(&soInfo4);
    Record3ReturnTypes retFu1D = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    Record3ReturnTypes retFu1Final = SKIP;
    if ((retFu1A == PASS) && (retFu1B == PASS) && (retFu1C == PASS) && (retFu1D == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    Record3ReturnTypes retFu2A = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes retFu2B = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    _soInfoWrapper->soInfo(&soInfo3);
    Record3ReturnTypes retFu2C = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);
    _soInfoWrapper->soInfo(&soInfo4);
    Record3ReturnTypes retFu2D = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2);

    Record3ReturnTypes retFu2Final = SKIP;
    if ((retFu2A == PASS) && (retFu2B == PASS) && (retFu2C == PASS) && (retFu2D == PASS))
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test A", retFu1A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test B", retFu1B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test C", retFu1C == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test D", retFu1D == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test E", retFu2A == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test F", retFu2B == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test G", retFu2C == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test H", retFu2D == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test I",
                           (retFu1Final == PASS) && (retFu2Final == PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test J",
                           fu1->stopovers().size() == 1 && fu2->stopovers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test K",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83301 - Test L",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 0 &&
                               (*fu1->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "NUC");

    CPPUNIT_ASSERT_MESSAGE(
        "Error in validate: Regression SPR 83301 - Test M",
        (*fu2->stopoverSurcharges().begin()).second->unconvertedAmount() == 30 &&
            (*fu2->stopoverSurcharges().begin()).second->unconvertedCurrencyCode() == "KWD");
  }

  void testRegressionSPR83995()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinRegressionSPR83995();

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    //
    //
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.itemNo() = 262723;
    soInfo1.charge1NoDec() = 2;
    soInfo1.charge1Cur() = "OMR";
    soInfo1.charge2Cur() = "OMR";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->stopoverGeoAppl() = 'N';
    soInfo1Seg1->stopoverInOutInd() = 'O';
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "DXB";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg2 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg2);
    soInfo1Seg2->orderNo() = 2;
    soInfo1Seg2->noStops() = "XX";
    soInfo1Seg2->chargeInd() = '1';

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segs().push_back(soInfo1Seg2);
    soInfo1.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 33;
    crInfo.carrierCode() = "EK";
    crInfo.ruleNumber() = "OM01";

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes retFu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    Record3ReturnTypes retFu1Final = SKIP;
    if (retFu1 == PASS)
    {
      if (_soInfoWrapper->needToProcessResults())
      {
        retFu1Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83995 - Test A", retFu1 == PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression SPR 83995 - Test B", retFu1Final == FAIL);
  }

  void testAbacusCmdPrcSurcharge()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItin4();

    PricingUnit* pu = _farePath->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    fu1->paxTypeFare()->fareMarket()->fareBasisCode() = "Y";
    fu2->paxTypeFare()->fareMarket()->fareBasisCode() = "Y";

    //
    // Unlimited stopovers. Maximum stay time = 1 month
    //
    StopoversInfo soInfo;

    createBaseStopoversInfo(soInfo);
    soInfo.maxStayTime() = 1;
    soInfo.maxStayTimeUnit() = 'M';
    soInfo.charge1FirstAmt() = 100;
    soInfo.charge1AddAmt() = 50;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1First() = "01";
    soInfo.charge1AddNo() = "01";
    soInfo.charge1Cur() = "USD";
    soInfo.charge2FirstAmt() = 60;
    soInfo.charge2AddAmt() = 30;
    soInfo.charge2NoDec() = 2;
    soInfo.charge2First() = "01";
    soInfo.charge2AddNo() = "01";
    soInfo.charge2Cur() = "GBP";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    _soInfoWrapper->soInfo(&soInfo);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    // Regular command pricing
    _options->fareX() = false;

    CPPUNIT_ASSERT( PASS == _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) );

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchA",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchB",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 100 &&
                               (*fu1->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "USD");

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchC",
                           (*fu2->stopoverSurcharges().begin()).second->currencyCode() == "USD");

    // FareX
    _options->fareX() = true;
    fu1->stopoverSurcharges().clear();
    fu2->stopoverSurcharges().clear();
    _soInfoWrapper->clearResults();

    CPPUNIT_ASSERT( PASS == _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) );

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchD",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchE",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 100 &&
                               (*fu1->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "USD");

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchF",
                           (*fu2->stopoverSurcharges().begin()).second->currencyCode() == "USD");

    // Abacus Agent
    _req->ticketingAgent() = _abacusAgent;
    _options->fareX() = false;
    fu1->stopoverSurcharges().clear();
    fu2->stopoverSurcharges().clear();
    _soInfoWrapper->clearResults();

    CPPUNIT_ASSERT( PASS == _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) );

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchG",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchH",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 100 &&
                               (*fu1->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "USD");

    // Abacus, no command pricing
    fu1->paxTypeFare()->fareMarket()->fareBasisCode() = "";
    fu2->paxTypeFare()->fareMarket()->fareBasisCode() = "";

    _soInfoWrapper->clearResults();

    CPPUNIT_ASSERT( _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu2) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1) );
    CPPUNIT_ASSERT( PASS == _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu2) );

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchI",
                           fu1->stopoverSurcharges().size() == 1 &&
                               fu2->stopoverSurcharges().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test SurchJ",
                           (*fu1->stopoverSurcharges().begin()).second->amount() == 100 &&
                               (*fu1->stopoverSurcharges().begin()).second->currencyCode() ==
                                   "USD");
  }

  void testStayTimeWithinSameDay()
  {
    DateTime arrivalDT(2007, 07, 01, 22, 59, 59);
    DateTime departDT(2007, 07, 01, 23, 59, 59);
    CPPUNIT_ASSERT_EQUAL(true, _so->isStayTimeWithinDays(arrivalDT, departDT, 0));
  }

  void testStayTimeNotInSameDay()
  {
    DateTime arrivalDT(2007, 07, 01, 23, 59, 59);
    DateTime departDT(2007, 07, 02, 00, 59, 59);
    CPPUNIT_ASSERT_EQUAL(false, _so->isStayTimeWithinDays(arrivalDT, departDT, 0));
  }

  void testStayTimeWithinNextDay()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 02, 13, 59, 59);
    CPPUNIT_ASSERT_EQUAL(true, _so->isStayTimeWithinDays(arrivalDT, departDT, 1));
  }

  void testStayTimeOverNextDay()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 03, 01, 59, 59);
    CPPUNIT_ASSERT_EQUAL(false, _so->isStayTimeWithinDays(arrivalDT, departDT, 1));
  }

  void testStayTimeWithin2Days()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 03, 13, 59, 59);
    CPPUNIT_ASSERT_EQUAL(true, _so->isStayTimeWithinDays(arrivalDT, departDT, 2));
  }

  void testStayTimeOver2Days()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 04, 01, 59, 59);
    CPPUNIT_ASSERT_EQUAL(false, _so->isStayTimeWithinDays(arrivalDT, departDT, 2));
  }

  void setMaxOutIn(StopoversInfo& soInfo, const char* max, const char* out, const char* in)
  {
    soInfo.noStopsMax() = max;
    soInfo.noStopsOutbound() = out;
    soInfo.noStopsInbound() = in;
  }

  void testValidateStopoversInfo_M2010_false_Phase1Act_OutInEither_MaxBlank()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "1", "1");
    StopoversInfoSeg* soInfoSeg = new StopoversInfoSeg;
    soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_EITHER;
    soInfo.segs().push_back(soInfoSeg);
    CPPUNIT_ASSERT(!_so->validateStopoversInfo_Mandate2010(soInfo));
  }

  void testValidateStopoversInfo_M2010_true_MaxBlankOutNotBlankInBlank()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "01", "");
    CPPUNIT_ASSERT(_so->validateStopoversInfo_Mandate2010(soInfo));
  }

  void testValidateStopoversInfo_M2010_true_MaxBlankOutBlankInNotBlank()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "", "01");
    CPPUNIT_ASSERT(_so->validateStopoversInfo_Mandate2010(soInfo));
  }

  void testMandateStopsOutInboundZero()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "0", "0");
    CPPUNIT_ASSERT(_so->validateStopoversInfo_Mandate2010(soInfo));
  }

  void testMandateStopsInboundZero()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "1", "0");
    CPPUNIT_ASSERT(_so->validateStopoversInfo_Mandate2010(soInfo));
  }

  void testFareUsageDirToValidate_UnknownDir_ifOutboundMaxEmpty()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "", "01");
    CPPUNIT_ASSERT(FMDirection::UNKNOWN == _so->fareUsageDirToValidate(soInfo));
  }

  void testFareUsageDirToValidate_UnknownDir_ifInboundMaxEmpty()
  {
    StopoversInfo soInfo;
    setMaxOutIn(soInfo, "", "01", "");
    CPPUNIT_ASSERT(FMDirection::UNKNOWN == _so->fareUsageDirToValidate(soInfo));
  }

  void testChkApplScope_FC_SCOPE()
  {
    StopoversInfo soInfo;
    soInfo.noStopsMax() = "";
    soInfo.noStopsOutbound() = "01";
    _so->chkApplScope(soInfo);
    CPPUNIT_ASSERT_EQUAL(Stopovers::FC_SCOPE, _so->_applScope);
  }

  void testChkApplScope_PU_SCOPE()
  {
    StopoversInfo soInfo;
    soInfo.noStopsMax() = "2";
    _so->chkApplScope(soInfo);
    CPPUNIT_ASSERT_EQUAL(Stopovers::PU_SCOPE, _so->_applScope);
  }

  void testCheckPreconditions_UnavailTagNotAvail()
  {
    StopoversInfo soInfo;
    soInfo.unavailTag() = Stopovers::UNAVAIL_TAG_NOT_AVAIL;
    Record3ReturnTypes ret = SKIP;
    CPPUNIT_ASSERT(!_so->checkPreconditions(soInfo, ret, 0));
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testCheckPreconditions_UnavailTagTextOnly()
  {
    StopoversInfo soInfo;
    soInfo.unavailTag() = Stopovers::UNAVAIL_TAG_TEXT_ONLY;
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT(!_so->checkPreconditions(soInfo, ret, 0));
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testCheckPreconditions_RtwMaxBlank()
  {
    StopoversInfo soInfo;
    soInfo.unavailTag() = ' ';
    soInfo.noStopsMax() = "";
    soInfo.noStopsInbound() = "01";
    soInfo.noStopsOutbound() = "01";
    _so->setRtw(true);
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT(!_so->checkPreconditions(soInfo, ret, 0));
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testCheckPreconditions_Pass()
  {
    StopoversInfo soInfo;
    soInfo.unavailTag() = ' ';
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT(_so->checkPreconditions(soInfo, ret, 0));
  }

  FareUsage* buildFU1_VCTR_ATP_AA_2_210_0()
  {
    FareInfo* fi1 = _memHandle.create<FareInfo>();
    Fare* fare1 = _memHandle.create<Fare>();
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    FareUsage* fu1 = _memHandle.create<FareUsage>();

    fu1->paxTypeFare() = ptf1;
    fi1->vendor() = "ATP";
    fi1->carrier() = "AA";
    fi1->fareTariff() = 2;
    fi1->ruleNumber() = "210";
    fi1->sequenceNumber() = 0;
    fare1->setFareInfo(fi1);
    ptf1->setFare(fare1);

    return fu1;
  }

  void testIsPuScope()
  {
    CPPUNIT_ASSERT( !_so->isPuScope() );

    _so->_applScope = Stopovers::PU_SCOPE;
    CPPUNIT_ASSERT( _so->isPuScope() );
  }

  FarePath* createADT_FarePath()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    PaxType* ptADT = _memHandle.create<PaxType>();
    PaxTypeInfo* ptInfo = _memHandle.create<PaxTypeInfo>();
    ptInfo->adultInd() = 'Y';
    ptADT->paxTypeInfo() = ptInfo;
    ptADT->paxType() = "ADT";

    farePath->paxType() = ptADT;
    return farePath;
  }

  FareUsage* createFU_VendorATP() { return buildFU1_VCTR_ATP_AA_2_210_0(); }

  void testCheckPaxType_reduntant_Charge2Appl()
  {
    StopoversInfo soInfo;
    _soInfoWrapper->soInfo(&soInfo);

    soInfo.charge1Appl() = RuleConst::CHARGE_PAX_ANY;
    soInfo.charge2Appl() = RuleConst::CHARGE_PAX_INFANT;

    FarePath* fpADT = createADT_FarePath();
    FareUsage* fuATP = createFU_VendorATP();

    CPPUNIT_ASSERT(_so->checkPaxType(*_trx, *fpADT, *fuATP, *_soInfoWrapper));
  }

  bool foundInDiagnostic(const char* text)
  {
    StopoversInfo soInfo;
    soInfo.charge1Total() = YES;

    _soInfoWrapper->soInfo(&soInfo);

    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    DiagManager diag(*_trx, Diagnostic308);

    _so->printDiagHeader(diag.collector(), *_soInfoWrapper, 0);
    diag.collector().flushMsg();
    return (string::npos != _trx->diagnostic().toString().find(text));
  }

  void testPrintDiagHeader_noSameCarrierRestAfterPhase2()
  {
    CPPUNIT_ASSERT( !foundInDiagnostic("CARRIER REST") );
  }

  void testPrintDiagHeader_noCharge1TotalAfterPhase2()
  {
    CPPUNIT_ASSERT(!foundInDiagnostic("PER TKT"));
  }

  void testCheckStopoversInSameLoc_Nation()
  {
    LocCode nationUS = "US";
    LocCode nationJP = "JP";
    Loc locDFW, locNRT, locLAX;
    locDFW.nation() = nationUS;
    locNRT.nation() = nationJP;
    locLAX.nation() = nationUS;

    Stopovers::StopoversInfoSegMarkup soISM;
    StopoversInfoSeg soIS;
    soISM.initialize(&soIS);
    VendorCode vendor = "ATP";

    CPPUNIT_ASSERT(_so->checkStopoversInSameLoc(*_trx, 0, vendor, LOCTYPE_NATION, locDFW, soISM));
    soISM.increaseMatchCount(false, false);
    CPPUNIT_ASSERT_EQUAL((uint16_t)(1), soISM.stoSameLocCnts()[nationUS]._matchCount);

    CPPUNIT_ASSERT(_so->checkStopoversInSameLoc(*_trx, 0, vendor, LOCTYPE_NATION, locNRT, soISM));
    soISM.increaseMatchCount(false, false);
    CPPUNIT_ASSERT_EQUAL((uint16_t)(1), soISM.stoSameLocCnts()[nationJP]._matchCount);

    CPPUNIT_ASSERT(_so->checkStopoversInSameLoc(*_trx, 0, vendor, LOCTYPE_NATION, locLAX, soISM));
    soISM.increaseMatchCount(false, false);
    CPPUNIT_ASSERT_EQUAL((uint16_t)(2), soISM.stoSameLocCnts()[nationUS]._matchCount);
    CPPUNIT_ASSERT_EQUAL((uint16_t)(1), soISM.stoSameLocCnts()[nationJP]._matchCount);
  }

  void buildStopoverInfoWithin1Nation10AddWithin1Nation20()
  {
    _soInfo.charge1FirstAmt() = 10;
    _soInfo.charge1AddAmt() = 20;
    _soInfo.charge1Cur() = "USD";
    _soInfo.segCnt() = 2;
    _soInfo.charge1First() = "1";
    _soInfo.charge1AddNo() = "1";

    StopoversInfoSeg* seg1 = buildSTO_SegInfo_1_Within1Nation();
    seg1->chargeInd() = Stopovers::SEG_USE_CHARGE_1;
    StopoversInfoSeg* seg2 = buildSTO_SegInfo_1_Within1Nation();
    seg2->chargeInd() = Stopovers::SEG_USE_CHARGE_2;
    _soInfo.segs().push_back(seg1);
    _soInfo.segs().push_back(seg2);
  }

  void buildStopoverInfoWithin1Nation10()
  {
    _soInfo.charge1FirstAmt() = 10;
    _soInfo.charge1Cur() = "USD";
    _soInfo.segCnt() = 1;
    _soInfo.charge1First() = "1";

    StopoversInfoSeg* seg1 = buildSTO_SegInfo_1_Within1Nation();
    seg1->chargeInd() = Stopovers::SEG_USE_CHARGE_1;
    _soInfo.segs().push_back(seg1);
  }

  StopoversInfoSeg* buildSTO_SegInfo_1_Within1Nation()
  {
    StopoversInfoSeg* seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(seg1);
    seg1->noStops() = "01";
    seg1->stopoverGeoAppl() = Stopovers::SEG_GEO_APPL_BLANK;
    seg1->loc1().locType() = LOCTYPE_NATION;
    seg1->loc1().loc() = LOCTYPE_NONE;
    return seg1;
  }

  void buildItinStopSameNationTwice()
  {
    // Create the travel segments
    //
    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 1;
    sfo_dfw->origin() = sfo;
    sfo_dfw->destination() = dfw;
    sfo_dfw->stopOver() = true;
    sfo_dfw->carrier() = "AA";
    sfo_dfw->geoTravelType() = GeoTravelType::Domestic;
    sfo_dfw->departureDT() = DateTime(2004, 8, 15, 13, 25, 0); //  6:25AM PST
    sfo_dfw->arrivalDT() = DateTime(2004, 8, 15, 17, 1, 0); // 12:01PM CST

    AirSeg* dfw_jfk = _memHandle.create<AirSeg>();
    dfw_jfk->pnrSegment() = 2;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = true;
    dfw_jfk->carrier() = "AA";
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2004, 8, 15, 22, 35, 0); //  5:35PM CST
    dfw_jfk->arrivalDT() = DateTime(2004, 8, 16, 2, 16, 0); // 10:16PM EST

    AirSeg* jfk_mia = _memHandle.create<AirSeg>();
    jfk_mia->pnrSegment() = 3;
    jfk_mia->origin() = jfk;
    jfk_mia->destination() = mia;
    jfk_mia->stopOver() = true;
    jfk_mia->carrier() = "AA";
    jfk_mia->geoTravelType() = GeoTravelType::Domestic;
    jfk_mia->departureDT() = DateTime(2004, 8, 21, 12, 28, 0); //  8:28AM EST
    jfk_mia->arrivalDT() = DateTime(2004, 8, 21, 16, 3, 0); // 11:03AM CST

    AirSeg* mia_sfo = _memHandle.create<AirSeg>();
    mia_sfo->pnrSegment() = 4;
    mia_sfo->origin() = mia;
    mia_sfo->destination() = sfo;
    mia_sfo->stopOver() = false;
    mia_sfo->carrier() = "AA";
    mia_sfo->geoTravelType() = GeoTravelType::Domestic;
    mia_sfo->departureDT() = DateTime(2004, 8, 21, 16, 47, 0); // 11:47AM CST
    mia_sfo->arrivalDT() = DateTime(2004, 8, 21, 20, 21, 0); //  1:21PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(sfo_dfw);
    _itin->travelSeg().push_back(dfw_jfk);
    _itin->travelSeg().push_back(jfk_mia);
    _itin->travelSeg().push_back(mia_sfo);

    _trx->travelSeg().push_back(sfo_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_mia);
    _trx->travelSeg().push_back(mia_sfo);

    // Create the FareUsages
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_mia);
    fu2->travelSeg().push_back(mia_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_mia);
    pu1->travelSeg().push_back(mia_sfo);

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(sfo, mia, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(mia, sfo, GlobalDirection::ZZ, GeoTravelType::Domestic);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_mia);
    fm2->travelSeg().push_back(mia_sfo);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);

    // Attach the _itin to the transaction
    //
    _trx->itin().push_back(_itin);
  }

  void testValidate_PASS_First1Nation10Add1Nation20()
  {
    buildStopoverInfoWithin1Nation10AddWithin1Nation20();
    buildItinStopSameNationTwice();
    PricingUnit& pu = *(_farePath->pricingUnit().front());

    pu.puType() = PricingUnit::Type::CIRCLETRIP;
    _soInfoWrapper->soInfo(&_soInfo);
    _soInfo.noStopsMax() = "04";
    _req->ticketingDT() = DateTime(2025, 4, 2);
    CPPUNIT_ASSERT_EQUAL(
        PASS, _so->validate(*_trx, _soInfoWrapper, *_farePath, pu, *pu.fareUsage().front()));
  }

  void testValidate_FAIL_With1Nation10()
  {
    buildStopoverInfoWithin1Nation10();
    buildItinStopSameNationTwice();
    PricingUnit& pu = *(_farePath->pricingUnit().front());

    pu.puType() = PricingUnit::Type::CIRCLETRIP;
    _soInfoWrapper->soInfo(&_soInfo);
    _soInfo.noStopsMax() = "04";
    _req->ticketingDT() = DateTime(2025, 4, 2);

    //  CPPUNIT_ASSERT_EQUAL( tse::FAIL, _so->validate(*_trx, _soInfoWrapper, *_farePath, pu,
    // *pu.fareUsage().front()) );
  }

  /*** Test cases related to SPR 139022 ***/
  struct Segment
  {
    bool so;
    uint16_t num;
    uint16_t flightNum;
    std::string carrier;
    std::string origin;
    std::string destination;
    std::string deptDT;
    std::string arvlDT;
    std::string key;
    GeoTravelType geoTravelType;

    Segment(uint16_t n,
            uint16_t fn,
            const char* c,
            const char* o,
            const char* d,
            const char* ddt,
            const char* adt)
      : num(n), flightNum(n), carrier(c), origin(o), destination(d), deptDT(ddt), arvlDT(adt)
    {
      key = origin;
      key += destination;
      so = false;
      geoTravelType = GeoTravelType::Domestic;
    }
  };

  void split(const std::string& str, std::vector<int>& vec)
  {
    vec.clear();
    boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
    for (string t : tokens)
      vec.push_back(atoi(t.c_str()));
  }

  DateTime getDT(const std::string& ds)
  {
    std::vector<int> vec;
    split(ds, vec);
    return DateTime(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
  }

  ArunkSeg* create_arunk(int segNum, const Loc* startLoc, const Loc* endLoc)
  {
    ArunkSeg* arunk = _memHandle.create<ArunkSeg>();
    arunk->pnrSegment() = segNum++;
    arunk->origin() = startLoc;
    arunk->destination() = endLoc;
    arunk->segmentType() = Arunk;
    return arunk;
  }

  TravelSeg* create_airSeg(const Segment& seg)
  {
    if (seg.carrier == "" || seg.flightNum == 0)
    {
      return create_arunk(seg.num, locMap[seg.origin], locMap[seg.destination]);
    }

    AirSeg* as = _memHandle.create<AirSeg>();
    as->pnrSegment() = seg.num;
    as->origin() = locMap[seg.origin];
    as->destination() = locMap[seg.destination];
    as->carrier() = seg.carrier;
    as->flightNumber() = seg.flightNum;
    as->geoTravelType() = seg.geoTravelType;
    as->stopOver() = seg.so;
    as->departureDT() = getDT(seg.deptDT);
    as->arrivalDT() = getDT(seg.arvlDT);
    return as;
  }

  void create_airSeg(const std::vector<Segment>& segments, std::map<std::string, TravelSeg*>& asMap)
  {
    for (std::vector<Segment>::const_iterator iter = segments.begin(); iter != segments.end();
         ++iter)
    {
      asMap[iter->key] = create_airSeg(*iter);
    }
  }

  void create_itin_and_trx(const std::vector<Segment>& segments,
                           std::map<std::string, TravelSeg*>& asMap)
  {
    create_airSeg(segments, asMap);

    for (std::vector<Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it)
    {
      _itin->travelSeg().push_back(asMap[it->key]);
      _trx->travelSeg().push_back(asMap[it->key]);
    }
  }

  void create_trx(const std::vector<Segment>& segs, std::map<std::string, TravelSeg*>& asMap)
  {
    create_itin_and_trx(segs, asMap);

    FareUsage* fu1 = _memHandle.create<FareUsage>();
    fu1->travelSeg().push_back(asMap["BOMSIN"]);
    fu1->travelSeg().push_back(asMap["SINSYD"]);
    fu1->travelSeg().push_back(asMap["SYDMEL"]);
    fu1->travelSeg().push_back(asMap["MELCBR"]);

    FareUsage* fu2 = _memHandle.create<FareUsage>();
    fu2->travelSeg().push_back(asMap["CBRSYD"]);
    fu2->travelSeg().push_back(asMap["SYDSIN"]);
    fu2->travelSeg().push_back(asMap["SINBOM"]);

    fu1->inbound() = false;
    fu2->inbound() = true;

    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu1->puType() = PricingUnit::Type::ROUNDTRIP;
    pu1->turnAroundPoint() = asMap["MELCBR"];
    pu1->geoTravelType() = GeoTravelType::International;

    _farePath->pricingUnit().push_back(pu1);

    FareMarket* fm1 = createFareMarket(
        locMap["BOM"], locMap["CBR"], GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(
        locMap["CBR"], locMap["BOM"], GlobalDirection::ZZ, GeoTravelType::International);

    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach travelSeg to FareMarket
    fm1->travelSeg().push_back(asMap["BOMSIN"]);
    fm1->travelSeg().push_back(asMap["SINSYD"]);
    fm1->travelSeg().push_back(asMap["SYDMEL"]);
    fm1->travelSeg().push_back(asMap["MELCBR"]);
    fm1->direction() = FMDirection::OUTBOUND;

    fm2->travelSeg().push_back(asMap["CBRSYD"]);
    fm2->travelSeg().push_back(asMap["SYDSIN"]);
    fm2->travelSeg().push_back(asMap["SINBOM"]);
    fm2->direction() = FMDirection::INBOUND;

    // Attach FareMarket to Itin
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void create_trx2(const std::vector<Segment>& segs, std::map<std::string, TravelSeg*>& asMap)
  {
    create_itin_and_trx(segs, asMap);
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    fu1->travelSeg().push_back(asMap["BOMSIN"]);
    fu1->travelSeg().push_back(asMap["SINSYD"]);
    fu1->travelSeg().push_back(asMap["SYDCBR"]);

    fu2->travelSeg().push_back(asMap["CBRSYD"]);
    fu2->travelSeg().push_back(asMap["SYDMEL"]); // arunk
    fu2->travelSeg().push_back(asMap["MELSIN"]);
    fu2->travelSeg().push_back(asMap["SINBOM"]);

    fu1->inbound() = false;
    fu2->inbound() = true;

    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu1->puType() = PricingUnit::Type::ROUNDTRIP;
    pu1->turnAroundPoint() = asMap["SYDCBR"];
    pu1->geoTravelType() = GeoTravelType::International;

    _farePath->pricingUnit().push_back(pu1);

    FareMarket* fm1 = createFareMarket(
        locMap["BOM"], locMap["CBR"], GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(
        locMap["CBR"], locMap["BOM"], GlobalDirection::ZZ, GeoTravelType::International);

    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach travelSeg to FareMarket
    fm1->travelSeg().push_back(asMap["BOMSIN"]);
    fm1->travelSeg().push_back(asMap["SINSYD"]);
    fm1->travelSeg().push_back(asMap["SYDCBR"]);
    fm1->direction() = FMDirection::OUTBOUND;

    fm2->travelSeg().push_back(asMap["CBRSYD"]);
    fm2->travelSeg().push_back(asMap["SYDMEL"]);
    fm2->travelSeg().push_back(asMap["MELSIN"]);
    fm2->travelSeg().push_back(asMap["SINBOM"]);
    fm2->direction() = FMDirection::INBOUND;

    // Attach FareMarket to Itin
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  void setStopoversInfo(StopoversInfo& soInfo,
                        uint32_t itemNo,
                        const std::string& maxSo,
                        const std::string& outSo,
                        const std::string& inSo,
                        MoneyAmount charge1FirstAmt)
  {
    createBaseStopoversInfo(soInfo);
    soInfo.itemNo() = itemNo;
    soInfo.noStopsMax() = maxSo;
    soInfo.noStopsOutbound() = outSo;
    soInfo.noStopsInbound() = inSo;
    soInfo.charge1FirstAmt() = charge1FirstAmt;
    soInfo.charge1NoDec() = 2;
    soInfo.charge1Cur() = "USD";
  }

  void setStopoversInfoSeg(StopoversInfo& soInfo, LocTypeCode locTypeCode, const std::string& loc)
  {
    StopoversInfoSeg* soInfoSeg = new StopoversInfoSeg();
    createBaseStopoversInfoSeg(soInfoSeg);
    soInfoSeg->chargeInd() = '1';
    soInfoSeg->loc1().locType() = locTypeCode;
    soInfoSeg->loc1().loc() = loc;
    soInfo.segs().push_back(soInfoSeg);
    soInfo.segCnt() = soInfo.segs().size();
  }

  void create_itin_Arunk_BOMSIN_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(std::vector<Segment>& segs)
  {
    // NO Stopover and ARUNK at the begining
    Segment s1(1, 0, "", "BOM", "SIN", "", ""); // ARUNK
    Segment s2(2, 3950, "QF", "SIN", "SYD", "2012 03 18 18 00 0", "2012 03 19 23 35 0");
    Segment s3(3, 528, "QF", "SYD", "MEL", "2012 03 20 02 05 0", "2012 03 20 03 35 0");
    Segment s4(4, 125, "QF", "MEL", "CBR", "2012 03 20 07 30 0", "2012 03 20 10 55 0");
    Segment s5(5, 114, "QF", "CBR", "SYD", "2012 03 24 00 15 0", "2012 03 24 03 45 0");
    Segment s6(6, 5, "QF", "SYD", "SIN", "2012 03 24 05 55 0", "2012 03 24 13 45 0");
    Segment s7(7, 3951, "QF", "SIN", "BOM", "2012 03 25 11 10 0", "2012 03 25 16 20 0");

    s1.so = false;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = false;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::International;
    segs.push_back(s3);
    s4.so = false;
    s4.geoTravelType = GeoTravelType::International;
    segs.push_back(s4);
    s5.so = false;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = false;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  void create_itin_Arunk_SINSYDso_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(std::vector<Segment>& segs)
  {
    // NO Stopover and ARUNK at the begining
    Segment s1(1, 0, "", "BOM", "SIN", "", ""); // ARUNK
    Segment s2(2, 3950, "QF", "SIN", "SYD", "2012 03 09 18 00 0", "2012 03 10 23 35 0"); // so
    Segment s3(3, 528, "QF", "SYD", "MEL", "2012 03 20 02 05 0", "2012 03 20 03 35 0");
    Segment s4(4, 125, "QF", "MEL", "CBR", "2012 03 20 07 30 0", "2012 03 20 10 55 0");
    Segment s5(5, 114, "QF", "CBR", "SYD", "2012 03 24 00 15 0", "2012 03 24 03 45 0");
    Segment s6(6, 5, "QF", "SYD", "SIN", "2012 03 24 05 55 0", "2012 03 24 13 45 0");
    Segment s7(7, 3951, "QF", "SIN", "BOM", "2012 03 25 11 10 0", "2012 03 25 16 20 0");

    s1.so = false;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = true;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::International;
    segs.push_back(s3);
    s4.so = false;
    s4.geoTravelType = GeoTravelType::International;
    segs.push_back(s4);
    s5.so = false;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = false;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  void create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(std::vector<Segment>& segs)
  {
    Segment s1(1, 3950, "QF", "BOM", "SIN", "2012 03 09 18 00 0", "2012 03 10 23 35 0"); // so
    Segment s2(2, 0, "", "SIN", "SYD", "", ""); // ARUNK
    Segment s3(3, 528, "QF", "SYD", "MEL", "2012 03 20 02 05 0", "2012 03 20 03 35 0");
    Segment s4(4, 125, "QF", "MEL", "CBR", "2012 03 20 07 30 0", "2012 03 20 10 55 0");
    Segment s5(5, 114, "QF", "CBR", "SYD", "2012 03 24 00 15 0", "2012 03 24 03 45 0");
    Segment s6(6, 5, "QF", "SYD", "SIN", "2012 03 24 05 55 0", "2012 03 24 13 45 0");
    Segment s7(7, 3951, "QF", "SIN", "BOM", "2012 03 25 11 10 0", "2012 03 25 16 20 0");

    s1.so = true;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = false;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::International;
    segs.push_back(s3);
    s4.so = false;
    s4.geoTravelType = GeoTravelType::International;
    segs.push_back(s4);
    s5.so = false;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = false;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  void create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(std::vector<Segment>& segs)
  {
    Segment s1(1, 3950, "QF", "BOM", "SIN", "2012 03 09 18 00 0", "2012 03 10 23 35 0"); // so
    Segment s2(2, 0, "", "SIN", "SYD", "", ""); // ARUNK
    Segment s3(3, 528, "QF", "SYD", "MEL", "2012 03 20 02 05 0", "2012 03 20 03 35 0");
    Segment s4(4, 125, "QF", "MEL", "CBR", "2012 03 20 07 30 0", "2012 03 20 10 55 0");
    Segment s5(5, 114, "QF", "CBR", "SYD", "2012 03 24 00 15 0", "2012 03 24 03 45 0");
    Segment s6(6, 5, "QF", "SYD", "SIN", "2012 03 24 05 55 0", "2012 03 24 13 45 0"); // so
    Segment s7(7, 3951, "QF", "SIN", "BOM", "2012 03 27 11 10 0", "2012 03 27 16 20 0");

    s1.so = true;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = false;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::International;
    segs.push_back(s3);
    s4.so = false;
    s4.geoTravelType = GeoTravelType::International;
    segs.push_back(s4);
    s5.so = false;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = true;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  void create_itin_BOMSIN_SINSYDso_SYDCBR_CBRSYDso_Arunk_MELSIN_SINBOM(std::vector<Segment>& segs)
  {
    Segment s1(1, 3950, "QF", "BOM", "SIN", "2012 03 09 18 00 0", "2012 03 10 23 35 0");
    Segment s2(2, 114, "QF", "SIN", "SYD", "2012 03 10 03 20 0", "2012 03 10 10 40 0"); // so
    Segment s3(3, 528, "QF", "SYD", "CBR", "2012 03 14 02 05 0", "2012 03 14 03 35 0");

    Segment s4(4, 125, "QF", "CBR", "SYD", "2012 03 20 07 30 0", "2012 03 20 10 55 0"); // so
    Segment s5(5, 0, "", "SYD", "MEL", "", "");
    Segment s6(6, 5, "QF", "MEL", "SIN", "2012 03 24 05 55 0", "2012 03 24 13 45 0");
    Segment s7(7, 3951, "QF", "SIN", "BOM", "2012 03 25 11 10 0", "2012 03 25 16 20 0");

    s1.so = false;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = true;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::International;
    segs.push_back(s3);
    s4.so = true;
    s4.geoTravelType = GeoTravelType::International;
    segs.push_back(s4);
    s5.so = false;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = false;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  // FC Scope
  bool processResults_Fail(StopoversInfo& soInfo, PaxTypeFare& ptf)
  {
    _soInfoWrapper->soInfo(&soInfo);
    if (SOFTPASS == _so->validate(*_trx, *_itin, ptf, _soInfoWrapper, *ptf.fareMarket()))
    {
      if (_soInfoWrapper->needToProcessResults())
        return (FAIL == _soInfoWrapper->processResults(*_trx, ptf));
    }
    return false;
  }

  bool processResults_SoftPass(StopoversInfo& soInfo, PaxTypeFare& ptf)
  {
    _soInfoWrapper->soInfo(&soInfo);
    if (SOFTPASS == _so->validate(*_trx, *_itin, ptf, _soInfoWrapper, *ptf.fareMarket()))
    {
      if (_soInfoWrapper->needToProcessResults())
        return (SOFTPASS == _soInfoWrapper->processResults(*_trx, ptf));
    }
    return false;
  }

  // PU Scope
  bool processResults_Pass(StopoversInfo& soInfo, PricingUnit& pu, FareUsage& fu)
  {
    _soInfoWrapper->soInfo(&soInfo);
    if (_so->validate(*_trx, _soInfoWrapper, *_farePath, pu, fu) == PASS &&
        _soInfoWrapper->needToProcessResults())
    {
      return (_soInfoWrapper->processResults(*_trx, *_farePath, pu, fu) == PASS);
    }
    return false;
  }

  bool processResults_Fail(StopoversInfo& soInfo, PricingUnit& pu, FareUsage& fu)
  {
    _soInfoWrapper->soInfo(&soInfo);
    if (_so->validate(*_trx, _soInfoWrapper, *_farePath, pu, fu) == PASS &&
        _soInfoWrapper->needToProcessResults())
    {
      return (_soInfoWrapper->processResults(*_trx, *_farePath, pu, fu) == FAIL);
    }
    return false;
  }

  Record3ReturnTypes check_validate_fvo(PaxTypeFare& ptf)
  {
    return _so->validate(*_trx, *_itin, ptf, _soInfoWrapper, *ptf.fareMarket());
  }

  // calls processResult in Fare Validation Service
  Record3ReturnTypes check_processResults_fvo(PaxTypeFare& ptf)
  {
    return _soInfoWrapper->processResults(*_trx, ptf);
  }

  Record3ReturnTypes check_validate_pvo(PricingUnit& pu, FareUsage& fu)
  {
    return _so->validate(*_trx, _soInfoWrapper, *_farePath, pu, fu);
  }

  // calls processResult for Pricing Service
  Record3ReturnTypes check_processResults_pvo(PricingUnit& pu, FareUsage& fu)
  {
    return _soInfoWrapper->processResults(*_trx, *_farePath, pu, fu);
  }

  /* 1. Arunk segment is at beginning of FareMarket.
   * It does not need to pass any record3. Even if it would not pass any record3,
   * we should have overall pass.
   */
  void set_test_arunk_at_begining_no_SO(std::vector<Segment>& segs,
                                        std::map<std::string, TravelSeg*>& asMap,
                                        StopoversInfo& soInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(soInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(soInfo, LOCTYPE_CITY, "LON");

    create_itin_Arunk_BOMSIN_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_arunk_at_begining_no_SO_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo soInfo;
    set_test_arunk_at_begining_no_SO(segs, asMap, soInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo);
    CPPUNIT_ASSERT(check_validate_fvo(*ptf1) == SOFTPASS);
    CPPUNIT_ASSERT(!_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(check_validate_fvo(*ptf2) == SOFTPASS);
    CPPUNIT_ASSERT(!_soInfoWrapper->needToProcessResults());
  }

  void test_arunk_at_begining_no_SO_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo soInfo;
    set_test_arunk_at_begining_no_SO(segs, asMap, soInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo);
    CPPUNIT_ASSERT(check_validate_pvo(*pu, *fu1) == PASS);
    CPPUNIT_ASSERT(!_soInfoWrapper->needToProcessResults());

    CPPUNIT_ASSERT(check_validate_pvo(*pu, *fu2) == PASS);
    CPPUNIT_ASSERT(!_soInfoWrapper->needToProcessResults());
  }

  void set_test_arunk_at_begining_with_SO(std::vector<Segment>& segs,
                                          std::map<std::string, TravelSeg*>& asMap,
                                          StopoversInfo& soInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(soInfo, ++itemNo, maxSo, outSo, inSo, 150);
    setStopoversInfoSeg(soInfo, LOCTYPE_NATION, "AU");

    create_itin_Arunk_SINSYDso_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_arunk_at_begining_with_SO_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo soInfo;
    set_test_arunk_at_begining_with_SO(segs, asMap, soInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo);
    CPPUNIT_ASSERT(check_validate_fvo(*ptf1) == SOFTPASS);
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(check_processResults_fvo(*ptf1) == SOFTPASS);

    CPPUNIT_ASSERT(check_validate_fvo(*ptf2) == SOFTPASS);
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(check_processResults_fvo(*ptf2) == SOFTPASS);
  }

  void test_arunk_at_begining_with_SO_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo soInfo;
    set_test_arunk_at_begining_with_SO(segs, asMap, soInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        150, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
  }

  /*
   * 2. There are multiple record3, the stopover segment and arunk segment passes different record3.
   * We have overall pass result and one surcharge
   */
  void set_test_seg_and_arunk_passes_diff_rec3(std::vector<Segment> segs,
                                               std::map<std::string, TravelSeg*> asMap,
                                               StopoversInfo& firstSoInfo,
                                               StopoversInfo& secondSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 150);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_NATION, "AU");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_seg_and_arunk_passes_diff_rec3_fvo()
  {
    StopoversInfo firstSoInfo, secondSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_seg_and_arunk_passes_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf1));
  }

  void test_seg_and_arunk_passes_diff_rec3_pvo()
  {
    StopoversInfo firstSoInfo, secondSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_seg_and_arunk_passes_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
  }

  /*
   * 3. There are multiple record3. The stopover segment pass one record3 but arunk segment
   * does not pass any record3. We have overall FAIL result
   */
  void set_test_seg_pass_but_arunk_fail_diff_rec3(std::vector<Segment>& segs,
                                                  std::map<std::string, TravelSeg*>& asMap,
                                                  StopoversInfo& firstSoInfo,
                                                  StopoversInfo& secondSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 150);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_NATION, "NZ");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_seg_pass_but_arunk_fail_diff_rec3_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_pass_but_arunk_fail_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf1));
  }

  void test_seg_pass_but_arunk_fail_diff_rec3_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_pass_but_arunk_fail_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopoverSurcharges().size());
  }

  /*
   * 4. There are multiple record3. The arunk segment passes one record3 but stopover segment
   * does not pass any record3. We have overall FAIL result.
   */
  void set_test_seg_fail_but_arunk_pass_diff_rec3(std::vector<Segment>& segs,
                                                  std::map<std::string, TravelSeg*>& asMap,
                                                  StopoversInfo& firstSoInfo,
                                                  StopoversInfo& secondSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "LON");
    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 150);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_NATION, "AU");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_seg_fail_but_arunk_pass_diff_rec3_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_fail_but_arunk_pass_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf1));
  }

  void test_seg_fail_but_arunk_pass_diff_rec3_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_fail_but_arunk_pass_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::THEN, &firstSoInfo);
    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));

    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::AND, &secondSoInfo);
    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopoverSurcharges().size());
  }

  /*
   * 5. There are multiple record3.
   * There is one stopover following no arunk segment passing and
   * a stopover segment followed by an arunk segment passing different record3.
   * We have overall pass result and two surcharges
   */
  void set_test_two_seg_an_arunk_pass_diff_rec3(std::vector<Segment>& segs,
                                                std::map<std::string, TravelSeg*>& asMap,
                                                StopoversInfo& firstSoInfo,
                                                StopoversInfo& secondSoInfo,
                                                StopoversInfo& thirdSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("1"), outSo("1"), inSo("0");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SIN");

    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 150);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_CITY, "SYD");

    maxSo = "1";
    outSo = "0";
    inSo = "1";
    setStopoversInfo(thirdSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(thirdSoInfo, LOCTYPE_CITY, "SIN");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_two_seg_an_arunk_pass_diff_rec3_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo, thirdSoInfo;
    set_test_two_seg_an_arunk_pass_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo, thirdSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf1));

    _soInfoWrapper->soInfo(&thirdSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf2));
  }

  void test_two_seg_an_arunk_pass_diff_rec3_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo, thirdSoInfo;
    set_test_two_seg_an_arunk_pass_diff_rec3(segs, asMap, firstSoInfo, secondSoInfo, thirdSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    _soInfoWrapper->soInfo(&thirdSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());
  }

  /*
   * 6. There is single record3. There is one stopover following no arunk segment passing and
   * a stopover segment followed by an arunk segment passing.
   * We have overall pass result and two surcharges
   * */
  void set_test_two_seg_an_arunk_pass_same_rec3(std::vector<Segment>& segs,
                                                std::map<std::string, TravelSeg*>& asMap,
                                                StopoversInfo& firstSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SYD");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_two_seg_an_arunk_pass_same_rec3_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo;
    set_test_two_seg_an_arunk_pass_same_rec3(segs, asMap, firstSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf1));

    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf2));
  }

  void test_two_seg_an_arunk_pass_same_rec3_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo;
    set_test_two_seg_an_arunk_pass_same_rec3(segs, asMap, firstSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());
  }

  /*
   * 7. There is single record3 only allowing one stopover.
   * There is one stopover following no arunk segment passing and a stopover segment
   * followed by an arunk segment passing. We have overall fail result by count check
   */
  void set_test_arunk_two_so_fail_r3_by_count_check(std::vector<Segment>& segs,
                                                    std::map<std::string, TravelSeg*>& asMap,
                                                    StopoversInfo& firstSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("1"), outSo("1"), inSo("");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SYD");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_arunk_two_so_fail_r3_by_count_check_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo;
    set_test_arunk_two_so_fail_r3_by_count_check(segs, asMap, firstSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf1));

    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf2));
  }

  void test_arunk_two_so_fail_r3_by_count_check_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo;
    set_test_arunk_two_so_fail_r3_by_count_check(segs, asMap, firstSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    size_t soCnt = fu1->stopovers().size();
    soCnt += fu2->stopovers().size();
    size_t surchargeCnt = fu1->stopoverSurcharges().size();
    surchargeCnt += fu2->stopoverSurcharges().size();

    CPPUNIT_ASSERT_EQUAL(size_t(2), soCnt);
    CPPUNIT_ASSERT_EQUAL(size_t(1), surchargeCnt);
  }

  /*
   * 8. There are multiple record3. There is one stopover following no arunk segment
   * passing one record3 and a stopover segment followed by an arunk segment passing another
   *record3.
   * We have overall pass result and two surcharges
   *
   * >> similar to test 5, modified itin with same test result
   */
  void set_test_seg_and_arunk_passes_diff_rec3_two_so(std::vector<Segment>& segs,
                                                      std::map<std::string, TravelSeg*>& asMap,
                                                      StopoversInfo& firstSoInfo,
                                                      StopoversInfo& secondSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SYD");
    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_CITY, "MEL");

    create_itin_BOMSIN_SINSYDso_SYDCBR_CBRSYDso_Arunk_MELSIN_SINBOM(segs);
    create_trx2(segs, asMap);
  }

  void test_seg_and_arunk_passes_diff_rec3_two_so_fvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_and_arunk_passes_diff_rec3_two_so(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf1));

    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf2));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf2));
  }

  void test_seg_and_arunk_passes_diff_rec3_two_so_pvo()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_and_arunk_passes_diff_rec3_two_so(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    size_t soCnt = fu1->stopovers().size();
    soCnt += fu2->stopovers().size();
    size_t surchargeCnt = fu1->stopoverSurcharges().size();
    surchargeCnt += fu2->stopoverSurcharges().size();

    CPPUNIT_ASSERT_EQUAL(size_t(2), soCnt);
    CPPUNIT_ASSERT_EQUAL(size_t(1), surchargeCnt);
  }

  /*
   * 9. There are multiple record3. There is one stopover following no arunk segment passing
   * and a stopover segment followed by an arunk segment not passing a record3,
   * with command pricing we have overall pass result and one surcharge
   */
  void setCmdPricing(PaxTypeFare& ptf, const char* fc)
  {
    Fare* f = ptf.fare();
    FareInfo* fi = const_cast<FareInfo*>(f->fareInfo());
    fi->fareClass() = fc;

    ptf.fareMarket()->fareBasisCode() = fc;
    ptf.fareMarket()->fbcUsage() = COMMAND_PRICE_FBC;

    _util->setFareIndicator(ptf, 25);
  }

  void set_test_seg_so_pass_but_arunk_fail_pass_by_command_pricing(
      std::vector<Segment>& segs,
      std::map<std::string, TravelSeg*>& asMap,
      StopoversInfo& firstSoInfo,
      StopoversInfo& secondSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SYD");

    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_CITY, "LON");

    create_itin_BOMSIN_SINSYDso_SYDCBR_CBRSYDso_Arunk_MELSIN_SINBOM(segs);
    create_trx2(segs, asMap);
  }

  void test_seg_so_pass_but_arunk_fail_passed_by_command_pricing()
  {
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    StopoversInfo firstSoInfo, secondSoInfo;
    set_test_seg_so_pass_but_arunk_fail_pass_by_command_pricing(
        segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    PaxTypeFare* ptf1 = pu->fareUsage()[0]->paxTypeFare();
    PaxTypeFare* ptf2 = pu->fareUsage()[1]->paxTypeFare();
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    // Command pricing
    // WPQ<fareBasisCode> and XML tag:B50
    setCmdPricing(*ptf1, "Y");
    CPPUNIT_ASSERT(ptf1->isCmdPricing());
    CPPUNIT_ASSERT(ptf1->isFareByRule());

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    setCmdPricing(*ptf2, "Y");
    CPPUNIT_ASSERT(ptf2->isCmdPricing());
    CPPUNIT_ASSERT(ptf2->isFareByRule());

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->stopoverSurcharges().size());
  }

  /*
   * 10. There are multiple Record 3. First Record3 matches segment but does not match
   * arunk. Second Record 3 matches both. We should apply second Record3
   */
  void set_test_inclusive_record3(std::vector<Segment> segs,
                                  std::map<std::string, TravelSeg*> asMap,
                                  StopoversInfo& firstSoInfo,
                                  StopoversInfo& secondSoInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("2"), outSo("1"), inSo("1");
    setStopoversInfo(firstSoInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(firstSoInfo, LOCTYPE_CITY, "SIN");

    maxSo = "3";
    outSo = "2";
    inSo = "1";
    setStopoversInfo(secondSoInfo, ++itemNo, maxSo, outSo, inSo, 150);
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfoSeg(secondSoInfo, LOCTYPE_CITY, "SYD");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSIN_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_inclusive_record3_fvo()
  {
    StopoversInfo firstSoInfo, secondSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_inclusive_record3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage*   fu1 = pu->fareUsage()[0];
    PaxTypeFare* ptf1 = fu1->paxTypeFare();

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_fvo(*ptf1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(SOFTPASS == check_validate_fvo(*ptf1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(SOFTPASS == check_processResults_fvo(*ptf1));
  }

  void test_inclusive_record3_pvo()
  {
    StopoversInfo firstSoInfo, secondSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_inclusive_record3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        150, (*fu1->stopoverSurcharges().begin()).second->amount(), EPSILON);
  }

  /*
   * 11. CTA - CTB (ARUNK) - CTC
   * Outbound Fare Rule:
   * (PricingUnit) Stopover Not permitted at CTB.
   * (PricingUnit) Stopover permitted everywhere else.
   *
   * Inbound Fare Rule:
   * (PricingUnit) Stopover permitted everywhere.
   * */
  void set_test_so_not_permitted_with_arunk_in_same_R3(std::vector<Segment> segs,
                                                       std::map<std::string, TravelSeg*> asMap,
                                                       StopoversInfo& soInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("02"), outSo("01"), inSo("01");
    setStopoversInfo(soInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(soInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfoSeg(soInfo, LOCTYPE_CITY, "");

    // SIN -> SEG_GEO_APPL_NOT_PERMITTED
    soInfo.segs()[0]->stopoverGeoAppl() = 'N';

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_so_not_permitted_with_arunk_in_same_R3_pvo()
  {
    StopoversInfo firstSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_so_not_permitted_with_arunk_in_same_R3(segs, asMap, firstSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&firstSoInfo);
    _soInfoWrapper->fareUsage() = fu1;
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu1));

    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopoverSurcharges().size());

    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->stopoverSurcharges().size());
  }

  // Test case related to SPR139022
  void set_test_so_not_permitted_with_arunk_in_diff_R3(std::vector<Segment> segs,
                                                       std::map<std::string, TravelSeg*> asMap,
                                                       StopoversInfo& soInfo1,
                                                       StopoversInfo& soInfo2)
  {
    uint32_t itemNo = 0;
    std::string maxSo("02"), outSo("01"), inSo("01");
    setStopoversInfo(soInfo1, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(soInfo1, LOCTYPE_CITY, "SIN");
    setStopoversInfoSeg(soInfo1, LOCTYPE_CITY, "");
    // SIN -> SEG_GEO_APPL_NOT_PERMITTED
    soInfo1.segs()[0]->stopoverGeoAppl() = 'N';

    setStopoversInfo(soInfo2, ++itemNo, maxSo, outSo, inSo, 120);
    setStopoversInfoSeg(soInfo2, LOCTYPE_CITY, "");

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_so_not_permitted_with_arunk_in_diff_R3_pvo()
  {
    StopoversInfo firstSoInfo, secondSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_so_not_permitted_with_arunk_in_diff_R3(segs, asMap, firstSoInfo, secondSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->fareUsage() = fu1;

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu2));

    _soInfoWrapper->soInfo(&secondSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->stopoverSurcharges().size());
  }

  /*
   * #2. Travel:
   * Outbound: CTA - CTB ( ARUNK) - CTC
   *
   * Outbound Fare Rule:
   * (PricingUnit) Stopover Not permitted at CTB.
   * (PricingUnit) Stopover permitted everywhere else.
   *
   * Inbound Fare Rule:
   * (PricingUnit) Stopover Not permitted at CTB.
   * (PricingUnit) Stopover permitted everywhere else.
  */
  void set_test_so_not_permitted_on_SIN_in_both_dir(std::vector<Segment> segs,
                                                    std::map<std::string, TravelSeg*> asMap,
                                                    StopoversInfo& soInfo)
  {
    uint32_t itemNo = 0;
    std::string maxSo("02"), outSo("01"), inSo("01");
    setStopoversInfo(soInfo, ++itemNo, maxSo, outSo, inSo, 100);
    setStopoversInfoSeg(soInfo, LOCTYPE_CITY, "SIN");
    setStopoversInfoSeg(soInfo, LOCTYPE_CITY, "");
    // SIN -> SEG_GEO_APPL_NOT_PERMITTED
    soInfo.segs()[0]->stopoverGeoAppl() = 'N';

    create_itin_BOMSINso_Arunk_SYDMEL_MELCBR_CBRSYD_SYDSINso_SINBOM(segs);
    create_trx(segs, asMap);
  }

  void test_so_not_permitted_on_SIN_in_both_dir_pvo()
  {
    StopoversInfo firstSoInfo;
    std::vector<Segment> segs;
    std::map<std::string, TravelSeg*> asMap;
    set_test_so_not_permitted_on_SIN_in_both_dir(segs, asMap, firstSoInfo);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->fareUsage() = fu1;

    _soInfoWrapper->soInfo(&firstSoInfo);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu1));

    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu2));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu2));

    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu1->stopoverSurcharges().size());

    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->stopovers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->stopoverSurcharges().size());
  }

  /*
   * 3 stopovers: 1 in outbout and 2 in inbound
   */
  void create_itin_for_testMaxExceed(std::vector<Segment>& segs)
  {
    Segment s1(1, 28, "JL", "HKG", "TYO", "2013 01 17 15 45 0", "2013 01 17 20 25 0"); // so
    Segment s2(2, 7016, "JL", "NRT", "LAX", "2013 01 22 15 55 0", "2013 01 22 08 35 0");
    Segment s3(3, 7516, "JL", "LAX", "SFO", "2013 01 22 11 35 0", "2013 01 22  0 50 0");

    Segment s4(4, 7517, "JL", "SFO", "LAX", "2013 01 27 07 10 0", "2013 01 27 08 20 0"); // so
    Segment s5(5, 61, "JL", "LAX", "NRT", "2013 02 02 11 55 0", "2013 02 03 16 45 0");
    Segment s6(6, 103, "JL", "TYO", "OSA", "2013 02 03 07 30 0", "2013 02 04 08 40 0"); // so
    Segment s7(7, 7053, "JL", "OSA", "HKG", "2013 02 07 10 00 0", "2013 02 07 13 25 0");

    s1.so = true;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = false;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::Domestic;
    segs.push_back(s3);
    s4.so = true;
    s4.geoTravelType = GeoTravelType::Domestic;
    segs.push_back(s4);
    s5.so = false;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = true;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  // 5 stopovers: 2 in outbout and 3 in inbound
  void create_itin_for_testMaxExceed_5SO(std::vector<Segment>& segs)
  {
    Segment s1(1, 28, "JL", "HKG", "TYO", "2013 01 17 15 45 0", "2013 01 17 20 25 0"); // so
    Segment s2(2, 7016, "JL", "NRT", "LAX", "2013 01 22 15 55 0", "2013 01 22 08 35 0"); // so
    Segment s3(3, 7516, "JL", "LAX", "SFO", "2013 01 24 11 35 0", "2013 01 24  0 50 0");

    Segment s4(4, 7517, "JL", "SFO", "LAX", "2013 01 27 07 10 0", "2013 01 27 08 20 0"); // so
    Segment s5(5, 61, "JL", "LAX", "NRT", "2013 02 03 11 55 0", "2013 02 04 16 45 0"); // so
    Segment s6(6, 103, "JL", "TYO", "OSA", "2013 02 05 07 30 0", "2013 02 05 08 40 0"); // so
    Segment s7(7, 7053, "JL", "OSA", "HKG", "2013 02 07 10 00 0", "2013 02 07 13 25 0");

    s1.so = true;
    s1.geoTravelType = GeoTravelType::International;
    segs.push_back(s1);
    s2.so = true;
    s2.geoTravelType = GeoTravelType::International;
    segs.push_back(s2);
    s3.so = false;
    s3.geoTravelType = GeoTravelType::Domestic;
    segs.push_back(s3);
    s4.so = true;
    s4.geoTravelType = GeoTravelType::Domestic;
    segs.push_back(s4);
    s5.so = true;
    s5.geoTravelType = GeoTravelType::International;
    segs.push_back(s5);
    s6.so = true;
    s6.geoTravelType = GeoTravelType::International;
    segs.push_back(s6);
    s7.so = false;
    s7.geoTravelType = GeoTravelType::International;
    segs.push_back(s7);
  }

  void set_trx_for_testMaxExceed(std::map<std::string, TravelSeg*> asMap)
  {
    // create Fare Component
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    fu1->travelSeg().push_back(asMap["HKGTYO"]);
    fu1->travelSeg().push_back(asMap["NRTLAX"]);
    fu1->travelSeg().push_back(asMap["LAXSFO"]);

    fu2->travelSeg().push_back(asMap["SFOLAX"]);
    fu2->travelSeg().push_back(asMap["LAXNRT"]); // arunk
    fu2->travelSeg().push_back(asMap["TYOOSA"]);
    fu2->travelSeg().push_back(asMap["OSAHKG"]);

    fu1->inbound() = false;
    fu2->inbound() = true;

    // create Pricing Unit
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu1->puType() = PricingUnit::Type::ROUNDTRIP;
    pu1->turnAroundPoint() = asMap["LAXSFO"];

    _farePath->pricingUnit().push_back(pu1);

    // create Fare Market
    FareMarket* fm1 = createFareMarket(
        locMap["HKG"], locMap["SFO"], GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(
        locMap["SFO"], locMap["HKG"], GlobalDirection::ZZ, GeoTravelType::International);

    fm1->governingCarrier() = "JP";
    fm2->governingCarrier() = "JP";

    TariffCrossRefInfo* tcrInfo1 = createTariffCrossRefInfo("ATP", "JP");
    TariffCrossRefInfo* tcrInfo2 = createTariffCrossRefInfo("ATP", "JP");

    PaxTypeFare* ptf1 = createPTF("USD", fm1, tcrInfo1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2, tcrInfo2);

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    fm1->travelSeg().push_back(asMap["HKGTYO"]);
    fm1->travelSeg().push_back(asMap["NRTLAX"]);
    fm1->travelSeg().push_back(asMap["LAXSFO"]);
    fm1->direction() = FMDirection::OUTBOUND;

    fm2->travelSeg().push_back(asMap["SFOLAX"]);
    fm2->travelSeg().push_back(asMap["LAXNRT"]);
    fm2->travelSeg().push_back(asMap["TYOOSA"]);
    fm2->travelSeg().push_back(asMap["OSAHKG"]);
    fm2->direction() = FMDirection::INBOUND;

    // Attach FareMarket to Itin
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

  /* Not Most restrictive test case
   * Record 3
   * 1. THEN MAX=2 IN=1 OUT=1
   *    recurring: LOC: JP/NATION
   * 2. AND MAX=4 IN=2 OUT=2
   * RESULT: PASS
   */
  void testMaxExceed_Pass()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    std::vector<Segment> segs;
    create_itin_for_testMaxExceed(segs);

    std::map<std::string, TravelSeg*> asMap;
    create_itin_and_trx(segs, asMap);
    set_trx_for_testMaxExceed(asMap);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    // FareUsage*   fu2 = pu->fareUsage()[1];

    StopoversInfo r3x;
    setStopoversInfo(r3x, 377871, "02", "01", "01", 0);
    setStopoversInfoSeg(r3x, LOCTYPE_NATION, "JP"); // recurring segment
    r3x.segs()[0]->stopoverGeoAppl() = ' '; // BLANK

    StopoversInfo r3y;
    setStopoversInfo(r3y, 603803, "04", "02", "02", 0);

    RuleSetPreprocessor rsp;
    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "JP";

    PaxTypeFareRuleData ptfRuleData;
    ptfRuleData.categoryRuleInfo() = &crInfo;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo criInfo;
    criInfo.setItemcat(8);
    criInfo.setOrderNo(1);
    criInfo.setItemNo(1);
    criInfo.setRelationalInd(CategoryRuleItemInfo::THEN);

    // Memory will be deleted by CategoryRuleItemInfo object
    CategoryRuleItemInfoSet* crInfoSet = new CategoryRuleItemInfoSet();
    crInfoSet->push_back(criInfo);
    crInfo.addItemInfoSetNosync(crInfoSet);

    PaxTypeFare::PaxTypeFareAllRuleData ptfAllRuleData;
    ptfAllRuleData.chkedRuleData = false;
    ptfAllRuleData.chkedGfrData = false;
    ptfAllRuleData.fareRuleData = &ptfRuleData;
    ptfAllRuleData.gfrRuleData = 0;

    vector<PricingUnit*>::iterator puIter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator puIterEnd = _farePath->pricingUnit().end();
    for (; puIter != puIterEnd; ++puIter)
    {
      vector<FareUsage*>::iterator fuIter = (*puIter)->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = (*puIter)->fareUsage().end();
      for (; fuIter != fuIterEnd; ++fuIter)
      {
        (*(*fuIter)->paxTypeFare()->paxTypeFareRuleDataMap())[8] = &ptfAllRuleData;
      }
    }

    // apply r3x and r3y on fare component 1
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);
    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::THEN, &r3x);
    _soInfoWrapper->soInfo(&r3x);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));

    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::AND, &r3y);
    _soInfoWrapper->soInfo(&r3y);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));

    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
  }

  /*
   * Most Restrictive case
   * Record 3
   * OUTBOUND
   * THEN MAX=2 IN=1 OUT=1
   *    LOC1=JP TYPE=N GEO-APPL=BLANK
   *
   * INBOUND
   * THEN MAX=4 IN=2 OUT=2
   *
   * ITIN: 1 SO on O/B and 2 SO on I/B
   *
   * RESULT: FAIL
   */
  void testMaxExceed_MostRestrictive_Fail()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    std::vector<Segment> segs;
    create_itin_for_testMaxExceed(segs);

    std::map<std::string, TravelSeg*> asMap;
    create_itin_and_trx(segs, asMap);
    set_trx_for_testMaxExceed(asMap);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    StopoversInfo r3x;
    setStopoversInfo(r3x, 377871, "02", "01", "01", 0);
    setStopoversInfoSeg(r3x, LOCTYPE_NATION, "JP"); // recurring segment
    r3x.segs()[0]->stopoverGeoAppl() = ' '; // BLANK

    StopoversInfo r3y;
    setStopoversInfo(r3y, 603803, "04", "02", "02", 0);

    RuleSetPreprocessor rsp;
    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "JP";

    PaxTypeFareRuleData ptfRuleData;
    ptfRuleData.categoryRuleInfo() = &crInfo;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo criInfo;
    criInfo.setItemcat(8);
    criInfo.setOrderNo(1);
    criInfo.setItemNo(1);
    criInfo.setRelationalInd(CategoryRuleItemInfo::THEN);

    // Memory will be deleted by CategoryRuleItemInfo object
    CategoryRuleItemInfoSet* crInfoSet = new CategoryRuleItemInfoSet();
    crInfoSet->push_back(criInfo);
    crInfo.addItemInfoSetNosync(crInfoSet);

    PaxTypeFare::PaxTypeFareAllRuleData ptfAllRuleData;
    ptfAllRuleData.chkedRuleData = false;
    ptfAllRuleData.chkedGfrData = false;
    ptfAllRuleData.fareRuleData = &ptfRuleData;
    ptfAllRuleData.gfrRuleData = 0;

    vector<PricingUnit*>::iterator puIter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator puIterEnd = _farePath->pricingUnit().end();
    for (; puIter != puIterEnd; ++puIter)
    {
      vector<FareUsage*>::iterator fuIter = (*puIter)->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = (*puIter)->fareUsage().end();
      for (; fuIter != fuIterEnd; ++fuIter)
      {
        (*(*fuIter)->paxTypeFare()->paxTypeFareRuleDataMap())[8] = &ptfAllRuleData;
      }
    }

    // apply r3x on outbound
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&r3x);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu1));

    // apply r3y on outbound
    _soInfoWrapper->soInfo(&r3y);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
  }

  /*
   * ITIN: 1 SO on O/B and 2 SO on I/B
   *
   * Record 3
   * OUTBOUND
   * THEN MAX=BLANK OUT=1 IN=1
   * AND  MAX=BLANK OUT=1 IN=1
   *
   * RESULT: PASS
   */
  void testMaxExceed_MaxBlank_Pass()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    std::vector<Segment> segs;
    create_itin_for_testMaxExceed(segs);

    std::map<std::string, TravelSeg*> asMap;
    create_itin_and_trx(segs, asMap);
    set_trx_for_testMaxExceed(asMap);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    StopoversInfo r3x;
    setStopoversInfo(r3x, 377871, "", "01", "01", 0);
    // setStopoversInfoSeg(r3x, LOCTYPE_NATION, "JP"); // recurring segment
    // r3x.segs()[0]->stopoverGeoAppl()  = ' ';//BLANK

    StopoversInfo r3y;
    setStopoversInfo(r3y, 603803, "", "01", "01", 0);

    RuleSetPreprocessor rsp;
    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "JP";

    PaxTypeFareRuleData ptfRuleData;
    ptfRuleData.categoryRuleInfo() = &crInfo;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo criInfo;
    criInfo.setItemcat(8);
    criInfo.setOrderNo(1);
    criInfo.setItemNo(1);
    criInfo.setRelationalInd(CategoryRuleItemInfo::THEN);

    // Memory will be deleted by CategoryRuleItemInfo object
    CategoryRuleItemInfoSet* crInfoSet = new CategoryRuleItemInfoSet();
    crInfoSet->push_back(criInfo);
    crInfo.addItemInfoSetNosync(crInfoSet);

    PaxTypeFare::PaxTypeFareAllRuleData ptfAllRuleData;
    ptfAllRuleData.chkedRuleData = false;
    ptfAllRuleData.chkedGfrData = false;
    ptfAllRuleData.fareRuleData = &ptfRuleData;
    ptfAllRuleData.gfrRuleData = 0;

    vector<PricingUnit*>::iterator puIter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator puIterEnd = _farePath->pricingUnit().end();
    for (; puIter != puIterEnd; ++puIter)
    {
      vector<FareUsage*>::iterator fuIter = (*puIter)->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = (*puIter)->fareUsage().end();
      for (; fuIter != fuIterEnd; ++fuIter)
      {
        (*(*fuIter)->paxTypeFare()->paxTypeFareRuleDataMap())[8] = &ptfAllRuleData;
      }
    }

    // apply r3x on outbound
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&r3x);
    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::THEN, &r3x);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu1));

    // apply r3y on outbound
    _soInfoWrapper->soInfo(&r3y);
    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::AND, &r3y);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
  }

  /*
   * ITIN: 2 SO on O/B and 3 SO on I/B
   *
   * Record 3
   * OUTBOUND
   * THEN MAX=BLANK OUT=1 IN=1 r3x
   * AND  MAX=BLANK OUT=1 IN=1 r3y
   *
   * RESULT: PASS
   */
  void testMaxExceed_MaxBlank_Fail()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    std::vector<Segment> segs;
    create_itin_for_testMaxExceed_5SO(segs);

    std::map<std::string, TravelSeg*> asMap;
    create_itin_and_trx(segs, asMap);
    set_trx_for_testMaxExceed(asMap);

    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    StopoversInfo r3x;
    setStopoversInfo(r3x, 377871, "", "01", "01", 0);

    StopoversInfo r3y;
    setStopoversInfo(r3y, 603803, "", "01", "01", 0);

    RuleSetPreprocessor rsp;
    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "JP";

    PaxTypeFareRuleData ptfRuleData;
    ptfRuleData.categoryRuleInfo() = &crInfo;

    // Memory will be deleted by CategoryRuleItemInfoSet object
    CategoryRuleItemInfo criInfo;
    criInfo.setItemcat(8);
    criInfo.setOrderNo(1);
    criInfo.setItemNo(1);
    criInfo.setRelationalInd(CategoryRuleItemInfo::THEN);

    // Memory will be deleted by CategoryRuleItemInfo object
    CategoryRuleItemInfoSet* crInfoSet = new CategoryRuleItemInfoSet();
    crInfoSet->push_back(criInfo);
    crInfo.addItemInfoSetNosync(crInfoSet);

    PaxTypeFare::PaxTypeFareAllRuleData ptfAllRuleData;
    ptfAllRuleData.chkedRuleData = false;
    ptfAllRuleData.chkedGfrData = false;
    ptfAllRuleData.fareRuleData = &ptfRuleData;
    ptfAllRuleData.gfrRuleData = 0;

    vector<PricingUnit*>::iterator puIter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator puIterEnd = _farePath->pricingUnit().end();
    for (; puIter != puIterEnd; ++puIter)
    {
      vector<FareUsage*>::iterator fuIter = (*puIter)->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = (*puIter)->fareUsage().end();
      for (; fuIter != fuIterEnd; ++fuIter)
      {
        (*(*fuIter)->paxTypeFare()->paxTypeFareRuleDataMap())[8] = &ptfAllRuleData;
      }
    }

    // apply r3x on outbound
    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&r3x);
    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::THEN, &r3x);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(FAIL == check_processResults_pvo(*pu, *fu1));

    // apply r3y on outbound
    _soInfoWrapper->soInfo(&r3y);
    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::AND, &r3y);
    CPPUNIT_ASSERT(PASS == check_validate_pvo(*pu, *fu1));
    CPPUNIT_ASSERT(_soInfoWrapper->needToProcessResults());
    CPPUNIT_ASSERT(PASS == check_processResults_pvo(*pu, *fu1));
  }

  void testProcessSamePointRestrictions_Stopovers()
  {
    TSMBuilder tsmBuilder(_memHandle);
    Stopovers::TravelSegMarkupContainer tsmc;
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::STOPOVER).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::CONNECTION).build());
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::STOPOVER).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::NONE).build());

    _soInfo.samePntStops() = 1;
    _soInfo.samePntConnections() = 1;

    std::string failReason;
    CPPUNIT_ASSERT_EQUAL(Stopovers::FAIL,
                         _so->processSamePointRestrictions(*_trx, 0, tsmc, _soInfo, failReason));
    CPPUNIT_ASSERT(failReason.find("TOO MANY STOPOVERS") != std::string::npos);
  }

  void testProcessSamePointRestrictions_Connections()
  {
    TSMBuilder tsmBuilder(_memHandle);
    Stopovers::TravelSegMarkupContainer tsmc;
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::CONNECTION).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::STOPOVER).build());
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::CONNECTION).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::NONE).build());

    _soInfo.samePntStops() = 1;
    _soInfo.samePntConnections() = 1;

    std::string failReason;
    CPPUNIT_ASSERT_EQUAL(Stopovers::FAIL,
                         _so->processSamePointRestrictions(*_trx, 0, tsmc, _soInfo, failReason));
    CPPUNIT_ASSERT(failReason.find("TOO MANY CONNECTIONS") != std::string::npos);
  }

  void testProcessSamePointRestrictions_Transfers()
  {
    TSMBuilder tsmBuilder(_memHandle);
    Stopovers::TravelSegMarkupContainer tsmc;
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::CONNECTION).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::NONE).build());
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::STOPOVER).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::NONE).build());

    _soInfo.samePntStops() = 1;
    _soInfo.samePntConnections() = 1;
    _soInfo.samePntTransit() = 1;

    std::string failReason;
    CPPUNIT_ASSERT_EQUAL(Stopovers::FAIL,
                         _so->processSamePointRestrictions(*_trx, 0, tsmc, _soInfo, failReason));
    CPPUNIT_ASSERT(failReason.find("TOO MANY TRANSFERS") != std::string::npos);
  }

  void testProcessSamePointRestrictions_None()
  {
    TSMBuilder tsmBuilder(_memHandle);
    Stopovers::TravelSegMarkupContainer tsmc;
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::CONNECTION).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::NONE).build());
    tsmc.push_back(tsmBuilder.withLoc(sfo, dfw).withSoType(Stopovers::STOPOVER).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, sfo).withSoType(Stopovers::NONE).build());

    _soInfo.samePntStops() = 1;
    _soInfo.samePntConnections() = 1;
    _soInfo.samePntTransit() = 2;

    std::string failReason;
    CPPUNIT_ASSERT_EQUAL(Stopovers::CONTINUE,
                         _so->processSamePointRestrictions(*_trx, 0, tsmc, _soInfo, failReason));
  }

  void testProcessSamePointRestrictions_TransEwrRtw()
  {
    TSMBuilder tsmBuilder(_memHandle);
    Stopovers::TravelSegMarkupContainer tsmc;
    tsmc.push_back(tsmBuilder.withLoc(sfo, jfk).withSoType(Stopovers::CONNECTION).build());
    tsmc.push_back(tsmBuilder.withLoc(jfk, dfw).withSoType(Stopovers::NONE).build());
    tsmc.push_back(tsmBuilder.withLoc(dfw, ewr).withSoType(Stopovers::STOPOVER).build());
    tsmc.push_back(tsmBuilder.withLoc(ewr, sfo).withSoType(Stopovers::NONE).build());

    _soInfo.samePntStops() = 0;
    _soInfo.samePntConnections() = 0;
    _soInfo.samePntTransit() = 1;
    _options->setRtw(true);

    std::string failReason;
    CPPUNIT_ASSERT_EQUAL(Stopovers::FAIL,
                         _so->processSamePointRestrictions(*_trx, 0, tsmc, _soInfo, failReason));
    CPPUNIT_ASSERT(failReason.find("TOO MANY TRANSFERS") != std::string::npos);
  }

  void setupCheckRequiredSegmentsSatisfied(bool puLevel, bool puScope)
  {
    createBaseStopoversInfo(_soInfo);
    _soInfo.noStopsMax() = puScope ? "XX" : "";
    _soInfoWrapper->soInfo(&_soInfo);
    _soInfoWrapper->fareUsage() = puLevel ? _memHandle.create<FareUsage>() : 0;
  }

  void testCheckRequiredSegmentsSatisfied_NoRequired()
  {
    setupCheckRequiredSegmentsSatisfied(true, true);

    SoISMBuilder b(_memHandle);
    std::vector<Stopovers::StopoversInfoSegMarkup*> markups;
    markups += b.appl(' ').dir(' ').noStops("01").build(0);
    markups += b.appl('N').dir(' ').noStops("00").build(0);

    std::string failReason;
    CPPUNIT_ASSERT(_so->checkRequiredSegmentsSatisfied(*_soInfoWrapper, markups, failReason));
  }

  void testCheckRequiredSegmentsSatisfied_FcLevel()
  {
    setupCheckRequiredSegmentsSatisfied(false, true); // fc level, pu scope

    SoISMBuilder b(_memHandle);
    std::vector<Stopovers::StopoversInfoSegMarkup*> markups;
    markups += b.order(1).appl('R').dir(' ').noStops("05").build(4);
    markups += b.order(2).appl('R').dir('E').noStops("04").build(3);
    markups += b.order(3).appl('R').dir('I').noStops("04").build(4);
    markups += b.order(4).appl('R').dir('I').noStops("04").build(3);

    std::string failReason;
    CPPUNIT_ASSERT(!_so->checkRequiredSegmentsSatisfied(*_soInfoWrapper, markups, failReason));
    CPPUNIT_ASSERT_EQUAL(std::string("STOPOVER REQUIRED BY RULE SEGMENT: 4"), failReason);
  }

  void testCheckRequiredSegmentsSatisfied_PuLevel()
  {
    setupCheckRequiredSegmentsSatisfied(true, false); // pu level, fc scope

    SoISMBuilder b(_memHandle);
    std::vector<Stopovers::StopoversInfoSegMarkup*> markups;
    markups += b.order(1).appl('R').dir('O').noStops("05").build(5);
    markups += b.order(2).appl('R').dir(' ').noStops("02").build(1);

    std::string failReason;
    CPPUNIT_ASSERT(!_so->checkRequiredSegmentsSatisfied(*_soInfoWrapper, markups, failReason));
    CPPUNIT_ASSERT_EQUAL(std::string("STOPOVER REQUIRED BY RULE SEGMENT: 2"), failReason);
  }

  void createItinApo31441()
  {
    _itin->calculationCurrency() = "CNY";

    // Create the travel segments
    //
    AirSeg* pvg_nrt = _memHandle.create<AirSeg>();
    pvg_nrt->pnrSegment() = 1;
    pvg_nrt->origin() = pvg;
    pvg_nrt->destination() = nrt;
    pvg_nrt->stopOver() = true;
    pvg_nrt->carrier() = "NH";
    pvg_nrt->flightNumber() = 922;
    pvg_nrt->geoTravelType() = GeoTravelType::International;
    pvg_nrt->departureDT() = DateTime(2005, 8, 29, 2, 15, 0); // 10:15AM GMT+8
    pvg_nrt->arrivalDT() = DateTime(2005, 8, 29, 5, 15, 0); //  2:15PM GMT+9

    AirSeg* nrt_hnl = _memHandle.create<AirSeg>();
    nrt_hnl->pnrSegment() = 2;
    nrt_hnl->origin() = nrt;
    nrt_hnl->destination() = hnl;
    nrt_hnl->stopOver() = false;
    nrt_hnl->carrier() = "UA";
    nrt_hnl->flightNumber() = 880;
    nrt_hnl->geoTravelType() = GeoTravelType::International;
    nrt_hnl->departureDT() = DateTime(2005, 8, 31, 10, 0, 0); // 7:00PM GMT+9
    nrt_hnl->arrivalDT() = DateTime(2005, 8, 31, 17, 25, 0); // 7:25AM GMT-10

    AirSeg* hnl_ogg = _memHandle.create<AirSeg>();
    hnl_ogg->pnrSegment() = 3;
    hnl_ogg->origin() = hnl;
    hnl_ogg->destination() = ogg;
    hnl_ogg->stopOver() = true;
    hnl_ogg->carrier() = "UA";
    hnl_ogg->flightNumber() = 5201;
    hnl_ogg->geoTravelType() = GeoTravelType::Domestic;
    hnl_ogg->departureDT() = DateTime(2005, 9, 1, 19, 0, 0); // 9:00AM GMT-10
    hnl_ogg->arrivalDT() = DateTime(2005, 9, 1, 20, 0, 0); // 10:00AM GMT-10

    AirSeg* ogg_pvg = _memHandle.create<AirSeg>();
    ogg_pvg->pnrSegment() = 4;
    ogg_pvg->origin() = ogg;
    ogg_pvg->destination() = sfo;
    ogg_pvg->stopOver() = false;
    ogg_pvg->carrier() = "UA";
    ogg_pvg->flightNumber() = 34;
    ogg_pvg->geoTravelType() = GeoTravelType::Domestic;
    ogg_pvg->departureDT() = DateTime(2005, 9, 5, 23, 0, 0); // 1:00PM GMT-10
    ogg_pvg->arrivalDT() = DateTime(2005, 9, 6, 4, 50, 0); // 8:50PM PST


    // Attach the travel segments to the Itin and the Transaction
    //
    _itin->travelSeg().push_back(pvg_nrt);
    _itin->travelSeg().push_back(nrt_hnl);
    _itin->travelSeg().push_back(hnl_ogg);
    _itin->travelSeg().push_back(ogg_pvg);

    _trx->travelSeg().push_back(pvg_nrt);
    _trx->travelSeg().push_back(nrt_hnl);
    _trx->travelSeg().push_back(hnl_ogg);
    _trx->travelSeg().push_back(ogg_pvg);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(pvg_nrt);
    fu1->travelSeg().push_back(nrt_hnl);
    fu2->travelSeg().push_back(hnl_ogg);
    fu2->travelSeg().push_back(ogg_pvg);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(pvg_nrt);
    pu1->travelSeg().push_back(nrt_hnl);
    pu1->travelSeg().push_back(hnl_ogg);
    pu1->travelSeg().push_back(ogg_pvg);

    pu1->turnAroundPoint() = nrt_hnl;

    // Attach the pricing units to the fare path
    //
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(pvg, hnl, GlobalDirection::ZZ, GeoTravelType::Domestic);
    FareMarket* fm2 = createFareMarket(hnl, pvg, GlobalDirection::ZZ, GeoTravelType::Domestic);

    fm1->governingCarrier() = "UA";
    fm2->governingCarrier() = "UA";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("CNY", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(pvg_nrt);
    fm1->travelSeg().push_back(nrt_hnl);
    fm2->travelSeg().push_back(hnl_ogg);
    fm2->travelSeg().push_back(ogg_pvg);

    // Attach the fare markets to the _itin
    //
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
  }

// Fix for APO31441:
//  Two FCs: FC1: PVG-NRT-HNL with SO in NRT
//           FC2: HNL-OGG-PVG  with SO in OGG
//  FC1 rec3. does not allow SO in OGG
//  Fc2 rec3  does not allow SO in NRT.
//  Both FCs will pass fc level validation but  fail pu level validation.
void testCheckNotAllowedRecurringSegments_PuLevel()
{
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinApo31441();
    
    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    // 1 free stopovers permitted anywhere except OGG 
    StopoversInfo soInfo1;

    createBaseStopoversInfo(soInfo1);
    soInfo1.noStopsMax() = "01";
    soInfo1.charge1Appl() = '5';
    soInfo1.charge1Cur() = "CNY";
    soInfo1.charge2Cur() = "CNY";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo1Seg1 = new StopoversInfoSeg();

    createBaseStopoversInfoSeg(soInfo1Seg1);
    soInfo1Seg1->stopoverGeoAppl() = 'N';
    soInfo1Seg1->chargeInd() = ' ';
    soInfo1Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo1Seg1->loc1().loc() = "OGG";

    soInfo1.segs().push_back(soInfo1Seg1);
    soInfo1.segCnt() = 1;
    //check the first FC against soInfo1
    FareMarket* fm = _itin->fareMarket()[0];
    pu = _farePath->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();
    _soInfoWrapper->soInfo(&soInfo1);

    Record3ReturnTypes ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);
    CPPUNIT_ASSERT_MESSAGE("Error in validating fare1", ret == SOFTPASS);
    
    StopoversInfo soInfo2;

    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.noStopsMax() = "01";
    soInfo2.charge1FirstAmt() = 1250;
    soInfo2.charge1AddAmt() = 2000;
    soInfo2.charge1Appl() = '5';
    soInfo2.charge1First() = "01";
    soInfo2.charge1AddNo() = "01";
    soInfo2.charge1Cur() = "CNY";
    soInfo2.charge2Cur() = "CNY";

    // Memory will be deleted by StopoversInfo object
    StopoversInfoSeg* soInfo2Seg1 = new StopoversInfoSeg();
    // 1 stopover permitted everywhere except NRT
    createBaseStopoversInfoSeg(soInfo2Seg1);
    soInfo2Seg1->stopoverGeoAppl() = 'N';
    soInfo2Seg1->chargeInd() = ' ';
    soInfo2Seg1->loc1().locType() = LOCTYPE_AIRPORT;
    soInfo2Seg1->loc1().loc() = "NRT";
    
    soInfo2.segs().push_back(soInfo2Seg1);
    soInfo2.segCnt() = 1;
    //check the second FC against soInfo2
    fm = _itin->fareMarket()[1];
    pu = _farePath->pricingUnit()[0];
    fare = pu->fareUsage()[1]->paxTypeFare();
    _soInfoWrapper->soInfo(&soInfo2);
    ret = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm);
    CPPUNIT_ASSERT_MESSAGE("Error in validating fare2", ret == SOFTPASS);

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 3;
    crInfo.carrierCode() = "UA";
    crInfo.ruleNumber() = "5815";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu);

    _soInfoWrapper->soInfo(&soInfo1);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes retPu = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate testAPO31441 ", retPu == FAIL);

}
// outbound FC: KWI-FRA + FRA-MIA with CX in FRA
// Inbound FC : MIA-DFW(stopover inDFW) +Arnk DFW-JFK+ JFK-KWI
// JFK is gtwy. Also is SO loc.
void createItinWithStopoverThenArunkAtGateway()
{
    AirSeg* kwi_fra = _memHandle.create<AirSeg>();
    kwi_fra->pnrSegment() = 1;
    kwi_fra->origin() = kwi;
    kwi_fra->destination() = fra;
    kwi_fra->stopOver() = false;
    kwi_fra->carrier() = "LH";
    kwi_fra->geoTravelType() = GeoTravelType::International;
    kwi_fra->departureDT() = DateTime(2015, 1, 23, 5, 25, 0); // 05:25AM IST 
    kwi_fra->arrivalDT() =   DateTime(2015, 1, 23, 9, 44, 0); //  9:44AM 

    AirSeg* fra_mia = _memHandle.create<AirSeg>();
    fra_mia->pnrSegment() = 2;
    fra_mia->origin() = fra;
    fra_mia->destination() = mia;
    fra_mia->stopOver() = false;
    fra_mia->carrier() = "LH";
    fra_mia->geoTravelType() = GeoTravelType::International;
    fra_mia->departureDT() = DateTime(2015, 1, 23, 10, 30, 0); //10:30 AM 
    fra_mia->arrivalDT() =   DateTime(2015, 1, 23, 14, 50, 0); //  2:50PM 


    AirSeg* mia_dfw = _memHandle.create<AirSeg>();
    mia_dfw->pnrSegment() = 3;
    mia_dfw->origin() = mia;
    mia_dfw->destination() = dfw;
    mia_dfw->stopOver() = true;
    mia_dfw->carrier() = "UA";
    mia_dfw->geoTravelType() = GeoTravelType::Domestic;
    mia_dfw->departureDT() = DateTime(2015, 1, 25, 14, 30, 0); //2:30 PM 
    mia_dfw->arrivalDT() =   DateTime(2015, 1, 25, 17, 30, 0); //  5:30PM 


    SurfaceSeg* dfw_jfk = _memHandle.create<SurfaceSeg>();
    dfw_jfk->pnrSegment() = 4;
    dfw_jfk->origin() = dfw;
    dfw_jfk->destination() = jfk;
    dfw_jfk->stopOver() = false;
    // dfw_jfk->carrier()       = "BA";
    // dfw_jfk->flightNumber()  = 304;
    dfw_jfk->geoTravelType() = GeoTravelType::Domestic;
    dfw_jfk->departureDT() = DateTime(2015, 1, 27, 18, 30, 0); // 6:30pm 
    dfw_jfk->arrivalDT() =   DateTime(2015, 1, 27, 20, 40, 0); //  8:40pm 

    AirSeg* jfk_kwi = _memHandle.create<AirSeg>();
    jfk_kwi->pnrSegment() = 5;
    jfk_kwi->origin() = jfk;
    jfk_kwi->destination() = kwi;
    jfk_kwi->stopOver() = false ;
    jfk_kwi->carrier() = "LX";
    jfk_kwi->geoTravelType() = GeoTravelType::International ;
    jfk_kwi->departureDT() = DateTime(2015, 1, 27, 22, 30, 0); //10:30 PM 
    jfk_kwi->arrivalDT() =   DateTime(2015, 1, 28, 10, 30, 0); //  10:30AM 
    // Attach the travel segments to the Itin and the Transaction
    _itin->travelSeg().push_back(kwi_fra); 
    _itin->travelSeg().push_back(fra_mia); 
    _itin->travelSeg().push_back(mia_dfw); 
    _itin->travelSeg().push_back(dfw_jfk); 
    _itin->travelSeg().push_back(jfk_kwi ); 

    _trx->travelSeg().push_back(kwi_fra);
    _trx->travelSeg().push_back(fra_mia);
    _trx->travelSeg().push_back(mia_dfw);
    _trx->travelSeg().push_back(dfw_jfk);
    _trx->travelSeg().push_back(jfk_kwi);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(kwi_fra);
    fu1->travelSeg().push_back(fra_mia);
    fu2->travelSeg().push_back(mia_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_kwi);
    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    // Attach the fare usages to the pricing units
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    pu1->travelSeg().push_back(kwi_fra);
    pu1->travelSeg().push_back(fra_mia);
    pu1->travelSeg().push_back(mia_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_kwi);

    pu1->turnAroundPoint() = fra_mia;

    // Attach the pricing units to the fare path
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    //
    FareMarket* fm1 = createFareMarket(kwi, mia, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(mia, kwi, GlobalDirection::ZZ, GeoTravelType::International);

    fm1->governingCarrier() = "LH";
    fm2->governingCarrier() = "LX";

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(kwi_fra);
    fm1->travelSeg().push_back(fra_mia);
    
    fm2->travelSeg().push_back(mia_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_kwi);

    // Attach the fare markets to the _itin
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
}

//apo-32079: R3 has no recurring segments but has geo table gateway item number. 
//itinerary includes a arunk segment following airsegment with stopovers.
//Both end points of the arnk segment qualify as  SO locations while one end of 
// the arunk qualifies as a gtway for SO R3.
//stopover is at dfw. dfw to jfk is arunk. gateway is jfk. this itin 
//is disllowed as jfk is also a SO locatioO location
void  testGtwyRestrictionsNoRecurringSegmentsAndGtwySoNotAllowed()
{
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    
    createItinWithStopoverThenArunkAtGateway();

    PricingUnit* pu1 = _farePath->pricingUnit()[0];
    
    FareUsage* fu2 = pu1->fareUsage()[1];

    StopoversInfo soInfo1;
    createBaseStopoversInfo(soInfo1);
    soInfo1.itemNo() = 1;
    soInfo1.geoTblItemNoGateway() = 122; // 995 has no TSI. Loc = US
    soInfo1.gtwyInd() = 'N';
    soInfo1.noStopsMax() = "01";


    RuleSetPreprocessor rsp;
    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);
    rsp.process(*_trx, &crInfo, *pu1, *fu2);

    _soInfoWrapper->doRuleSetPreprocessing(*_trx, rsp, *pu1);
    _soInfoWrapper->fareUsage() = fu2;
    _soInfoWrapper->soInfo(&soInfo1);
    //JFK is a gateway in arunk segment while 995 does not allow any so in gtwy. we should fail here
    Record3ReturnTypes ret1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu2);
    CPPUNIT_ASSERT_MESSAGE("Error: NoRecurringSegGatewayRestrictionsNotPermitted", ret1 == FAIL);
}
    
// DFW is SO loc but SO shouldbe at gtwy. but JFK is also a SO loc. JFK is gtway. this itin should pass    
void  testGtwyRestrictionsNoRecurringSegmentsAndGtwySoRequired()
{
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    
    createItinWithStopoverThenArunkAtGateway();

    PricingUnit* pu1 = _farePath->pricingUnit()[0];
    
    FareUsage* fu2 = pu1->fareUsage()[1];
    StopoversInfo soInfo2;
    createBaseStopoversInfo(soInfo2);
    soInfo2.itemNo() = 2;
    soInfo2.geoTblItemNoGateway() = 122; // 995 has no TSI. Loc = US
    soInfo2.gtwyInd() = 'P'; //gateway required
    soInfo2.noStopsMax() = "01";
    //Gtwy is required in stopover. so we should pass in this case  as both dfw and jfk are stopover locs
    _soInfoWrapper->soInfo(&soInfo2);
    Record3ReturnTypes ret1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu2);
    CPPUNIT_ASSERT_MESSAGE("Error: NoRecurringSegGatewayRestrictionsRequired", ret1 == PASS);
}
// DFW is SO loc and gtway SO is permitted. So we pass this itin.
void  testGtwyRestrictionsNoRecurringSegmentsAndGtwySoPermitted()
{
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();
    
    createItinWithStopoverThenArunkAtGateway();

    PricingUnit* pu1 = _farePath->pricingUnit()[0];
    
    FareUsage* fu2 = pu1->fareUsage()[1];

    StopoversInfo soInfo3;
    createBaseStopoversInfo(soInfo3);
    soInfo3.itemNo() = 3;
    soInfo3.geoTblItemNoGateway() = 122; // 995 has no TSI. Loc = US
    soInfo3.gtwyInd() = 'P'; //gateway permitted
    soInfo3.noStopsMax() = "01";
    //Gtwy is permitted in stopover. so we should pass in this case  as both dfw and jfk are stopover
    _soInfoWrapper->soInfo(&soInfo3);
    Record3ReturnTypes ret1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu1, *fu2);
    CPPUNIT_ASSERT_MESSAGE("Error: NoRecurringSegGatewayRestrictionsPermitted ", ret1 == PASS);
}
void createItinApo36029()
{
   _itin->calculationCurrency() = "CNY";

   // Create the travel segments
   AirSeg* pvg_hnl = _memHandle.create<AirSeg>();
   pvg_hnl->pnrSegment() = 1;
   pvg_hnl->origin() = pvg;
   pvg_hnl->destination() = hnl;
   pvg_hnl->stopOver() = false;
   pvg_hnl->carrier() = "UA";
   pvg_hnl->flightNumber() = 922;
   pvg_hnl->geoTravelType() = GeoTravelType::International;
   pvg_hnl->departureDT() = DateTime(2005, 8, 29, 2, 15, 0); // 10:15AM GMT+8
   pvg_hnl->arrivalDT() = DateTime(2005, 8, 29, 5, 15, 0); //  2:15PM GMT+9

   AirSeg* hnl_ogg = _memHandle.create<AirSeg>();
   hnl_ogg->pnrSegment() = 2;
   hnl_ogg->origin() = hnl;
   hnl_ogg->destination() = ogg;
   hnl_ogg->stopOver() = true;
   hnl_ogg->carrier() = "UA";
   hnl_ogg->flightNumber() = 880;
   hnl_ogg->geoTravelType() = GeoTravelType::Domestic;
   hnl_ogg->departureDT() = DateTime(2005, 8, 31, 10, 0, 0); // 7:00PM GMT+9
   hnl_ogg->arrivalDT() = DateTime(2005, 8, 31, 12, 25, 0); // 9:25PM GMT+9

   AirSeg* ogg_dfw = _memHandle.create<AirSeg>();
   ogg_dfw->pnrSegment() = 3;
   ogg_dfw->origin() = ogg;
   ogg_dfw->destination() = dfw;
   ogg_dfw->stopOver() = true;
   ogg_dfw->carrier() = "UA";
   ogg_dfw->flightNumber() = 5201;
   ogg_dfw->geoTravelType() = GeoTravelType::Domestic;
   ogg_dfw->departureDT() = DateTime(2005, 9, 2, 19, 0, 0); // 9:00AM GMT-10
   ogg_dfw->arrivalDT() = DateTime(2005, 9, 2, 20, 0, 0); // 10:00AM GMT-10

   AirSeg* dfw_pvg = _memHandle.create<AirSeg>();
   dfw_pvg->pnrSegment() = 4;
   dfw_pvg->origin() = dfw;
   dfw_pvg->destination() = pvg;
   dfw_pvg->stopOver() = false;
   dfw_pvg->carrier() = "UA";
   dfw_pvg->flightNumber() = 34;
   dfw_pvg->geoTravelType() = GeoTravelType::International;
   dfw_pvg->departureDT() = DateTime(2005, 9, 5, 23, 0, 0); // 1:00PM GMT-10
   dfw_pvg->arrivalDT() = DateTime(2005, 9, 6, 4, 50, 0); // 8:50PM PST
   // Attach the travel segments to the Itin and the Transaction
   _itin->travelSeg().push_back(pvg_hnl);
   _itin->travelSeg().push_back(hnl_ogg);
   _itin->travelSeg().push_back(ogg_dfw);
   _itin->travelSeg().push_back(dfw_pvg);

   _trx->travelSeg().push_back(pvg_hnl);
   _trx->travelSeg().push_back(hnl_ogg);
   _trx->travelSeg().push_back(ogg_dfw);
   _trx->travelSeg().push_back(dfw_pvg);
   
   // Create the FareUsages, PaxTypeFares, Fares and FareInfos
   FareUsage* fu1 = _memHandle.create<FareUsage>();
   FareUsage* fu2 = _memHandle.create<FareUsage>();

   // Attach the travel segments to the fare usages
   fu1->travelSeg().push_back(pvg_hnl);
   fu2->travelSeg().push_back(hnl_ogg);
   fu2->travelSeg().push_back(ogg_dfw);
   fu2->travelSeg().push_back(dfw_pvg);

   // Set the directionality
   fu1->inbound() = false;
   fu2->inbound() = true;
   // Create the pricing units
   PricingUnit* pu1 = _memHandle.create<PricingUnit>();
   pu1->puType() = PricingUnit::Type::ROUNDTRIP;

   // Attach the fare usages to the pricing units
   pu1->fareUsage().push_back(fu1);
   pu1->fareUsage().push_back(fu2);
    
   // Attach the travel segs to the pricing units
   pu1->travelSeg().push_back(pvg_hnl);
   pu1->travelSeg().push_back(hnl_ogg);
   pu1->travelSeg().push_back(ogg_dfw);
   pu1->travelSeg().push_back(dfw_pvg);

   pu1->turnAroundPoint() = pvg_hnl;
   // Attach the pricing units to the fare path
   _farePath->pricingUnit().push_back(pu1);
   // Create the FareMarkets
   FareMarket* fm1 = createFareMarket(pvg, hnl, GlobalDirection::ZZ, GeoTravelType::Domestic);
   FareMarket* fm2 = createFareMarket(hnl, pvg, GlobalDirection::ZZ, GeoTravelType::Domestic);

   fm1->governingCarrier() = "UA";
   fm2->governingCarrier() = "UA";
   // Create and initialize the PaxTypeFares
   TariffCrossRefInfo* tcrInfo1 = createTariffCrossRefInfo("ATP", "CN");
   TariffCrossRefInfo* tcrInfo2 = createTariffCrossRefInfo("ATP", "CN");
   PaxTypeFare* ptf1 = createPTF("USD", fm1,tcrInfo1);
   PaxTypeFare* ptf2 = createPTF("USD", fm2,tcrInfo2);
   // Attach the PaxTypeFares to the FareUsages
   fu1->paxTypeFare() = ptf1;
   fu2->paxTypeFare() = ptf2;
   // Attach the travel segs to the fare markets
   fm1->travelSeg().push_back(pvg_hnl);
   fm2->travelSeg().push_back(hnl_ogg);
   fm2->travelSeg().push_back(ogg_dfw);
   fm2->travelSeg().push_back(dfw_pvg);
   // Attach the fare markets to the _itin
   _itin->fareMarket().push_back(fm1);
   _itin->fareMarket().push_back(fm2);
}

//APO36029:soInfo.numStopsObound and numStopsIbound counts should notbe validated at pu level. 
//Inbound fc has 2 so.  outbound fc has 0 so. outbound cat8 r3 has numStopsInbound=1.
//Two Fcs:Outbound FC1 PVG-HNL with 0 Stopovers
//         FC2: HNL-OGG-DFW-PVG with so in OGG and  DFW .  2 so in Inbound fc
// O/B rec3(x1) allow to 2 max so with 1 max inbound and 1 max outbound
// I/B rec3(y1) allows 10 stopovers
// this unit test should pass PU againt  x1 even though we have 2 stopovers on i/b fc
// which exceed the allowed 1 max inbound count in x1
void testDoNotCheckInBoundSOCount_WhenValidatingAtPUwithOutboundRule()
{
    _trx->diagnostic().diagnosticType() = Diagnostic308;
    _trx->diagnostic().activate();

    createItinApo36029();
    
    PricingUnit* pu = _farePath->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    
    //Outbound fare R3:2so max,1 i/b so + 1 o/b so
    StopoversInfo outbound_soInfo;

    createBaseStopoversInfo(outbound_soInfo);
    outbound_soInfo.itemNo() = 2;
    outbound_soInfo.noStopsMax() = "02";
    outbound_soInfo.noStopsOutbound() = "01";
    outbound_soInfo.noStopsInbound() = "01";

    
    //Inbound fare R3:10 so max,5 i/b so + 5 o/b so
    StopoversInfo inbound_soInfo;
    createBaseStopoversInfo(inbound_soInfo);
    inbound_soInfo.itemNo() = 3;
    inbound_soInfo.noStopsMax() = "10";
    inbound_soInfo.noStopsOutbound() = "5";
    inbound_soInfo.noStopsInbound() = "5";
    
    _soInfoWrapper->sumNumStopoversMax(CategoryRuleItemInfo::THEN, &outbound_soInfo);
    
    _soInfoWrapper->soInfo(&outbound_soInfo);
    _soInfoWrapper->fareUsage() = fu1;
    Record3ReturnTypes ret1Fu1 = _so->validate(*_trx, _soInfoWrapper, *_farePath, *pu, *fu1);
    CPPUNIT_ASSERT_MESSAGE("ret1Fu1:", ret1Fu1 ==PASS);

    //set one stopover seg as not matched to force seg count check 
    TravelSeg*  ts1 = fu2->travelSeg()[0];
    _soInfoWrapper->setMatched(ts1, fu2, StopoversTravelSegWrapper::NOT_MATCHED);
    //there is a bug in stopovers.cpp..so unset the maxSoexceed value in wrapper.
    StopoversTravelSegWrapperMapI stswIter;
    stswIter = _soInfoWrapper->stopoversTravelSegWrappers().find(ts1);
    if (stswIter != _soInfoWrapper->stopoversTravelSegWrappers().end() ) 
    {
      StopoversTravelSegWrapper& stsw = (*stswIter).second;
      stsw.maxSOExceeded() = 0;
    }
   
    Record3ReturnTypes ret2Final = SKIP;
    //we are applying the outbound fare rule on the whole pu. this should pass
    if (_soInfoWrapper->needToProcessResults())
        ret2Final = _soInfoWrapper->processResults(*_trx, *_farePath, *pu, *fu1);
    CPPUNIT_ASSERT_MESSAGE("Error in validate apo36029", PASS == ret2Final); 
    
}
// outbound FC: FRA-MIA
// Inbound FC : MIA-DFW(stopover inDFW) +Arnk DFW-SJC+SJC-ORD+ORD-FRA
// SO is DFW. ORD is gtway. Inbound FC requires stopover at gtway.
// So, the inbound FC should fail.
void createItinWithStopoverThenArunkThenGatewaySegment()
{
    AirSeg* fra_mia = _memHandle.create<AirSeg>();
    fra_mia->pnrSegment() = 1;
    fra_mia->origin() = fra;
    fra_mia->destination() = mia;
    fra_mia->stopOver() = false;
    fra_mia->carrier() = "LH";
    fra_mia->geoTravelType() = GeoTravelType::International;
    fra_mia->departureDT() = DateTime(2015, 1, 23, 5, 25, 0); // 05:25AM IST
    fra_mia->arrivalDT() =   DateTime(2015, 1, 23, 9, 44, 0); //  9:44AM


    AirSeg* mia_dfw = _memHandle.create<AirSeg>();
    mia_dfw->pnrSegment() = 2;
    mia_dfw->origin() = mia;
    mia_dfw->destination() = dfw;
    mia_dfw->stopOver() = true;
    mia_dfw->carrier() = "UA";
    mia_dfw->geoTravelType() = GeoTravelType::Domestic;
    mia_dfw->departureDT() = DateTime(2015, 1, 25, 14, 30, 0); //2:30 PM
    mia_dfw->arrivalDT() =   DateTime(2015, 1, 25, 17, 30, 0); //  5:30PM

    SurfaceSeg* dfw_sjc = _memHandle.create<SurfaceSeg>();
    dfw_sjc->pnrSegment() = 3;
    dfw_sjc->origin() = dfw;
    dfw_sjc->destination() = sjc;
    dfw_sjc->stopOver() = false;
    dfw_sjc->geoTravelType() = GeoTravelType::Domestic;
    dfw_sjc->departureDT() = DateTime(2015, 1, 27, 18, 30, 0); // 6:30pm
    dfw_sjc->arrivalDT() =   DateTime(2015, 1, 27, 20, 40, 0); //  8:40pm

    AirSeg* sjc_ord = _memHandle.create<AirSeg>();
    sjc_ord->pnrSegment() = 4;
    sjc_ord->origin() = sjc;
    sjc_ord->destination() = ord;
    sjc_ord->stopOver() = false ;
    sjc_ord->carrier() = "UA";
    sjc_ord->geoTravelType() = GeoTravelType::Domestic;
    sjc_ord->departureDT() = DateTime(2015, 1, 27, 22, 30, 0); //10:30 PM
    sjc_ord->arrivalDT() =   DateTime(2015, 1, 28, 10, 30, 0); //  10:30AM

    AirSeg* ord_fra = _memHandle.create<AirSeg>();
    ord_fra->pnrSegment() = 5;
    ord_fra->origin() = ord;
    ord_fra->destination() = fra;
    ord_fra->stopOver() = false ;
    ord_fra->carrier() = "LH";
    ord_fra->geoTravelType() = GeoTravelType::International ;
    ord_fra->departureDT() = DateTime(2015, 1, 28, 14, 30, 0); //
    ord_fra->arrivalDT() =   DateTime(2015, 1, 28, 10, 30, 0); //  10:30AM
    // Attach the travel segments to the Itin and the Transaction
    _itin->travelSeg().push_back(fra_mia);
    _itin->travelSeg().push_back(mia_dfw);
    _itin->travelSeg().push_back(dfw_sjc);
    _itin->travelSeg().push_back(sjc_ord);
    _itin->travelSeg().push_back(ord_fra);

    _trx->travelSeg().push_back(fra_mia);
    _trx->travelSeg().push_back(mia_dfw);
    _trx->travelSeg().push_back(dfw_sjc);
    _trx->travelSeg().push_back(sjc_ord);
    _trx->travelSeg().push_back(ord_fra);
    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(fra_mia);

    fu2->travelSeg().push_back(mia_dfw);
    fu2->travelSeg().push_back(dfw_sjc);
    fu2->travelSeg().push_back(sjc_ord);
    fu2->travelSeg().push_back(ord_fra);
    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;
    // Create the pricing units
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    // Attach the fare usages to the pricing units
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    pu1->travelSeg().push_back(fra_mia);
    pu1->travelSeg().push_back(mia_dfw);
    pu1->travelSeg().push_back(dfw_sjc);
    pu1->travelSeg().push_back(sjc_ord);
    pu1->travelSeg().push_back(ord_fra);

    pu1->turnAroundPoint() = fra_mia;
    // Attach the pricing units to the fare path
    _farePath->pricingUnit().push_back(pu1);

    // Create the FareMarkets
    FareMarket* fm1 = createFareMarket(fra, mia, GlobalDirection::ZZ, GeoTravelType::International);
    FareMarket* fm2 = createFareMarket(mia, fra, GlobalDirection::ZZ, GeoTravelType::International);

    fm1->governingCarrier() = "LH";
    fm2->governingCarrier() = "LH";
    // Create and initialize the PaxTypeFares
    PaxTypeFare* ptf1 = createPTF("USD", fm1);
    PaxTypeFare* ptf2 = createPTF("USD", fm2);
    // Attach the PaxTypeFares to the FareUsages
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(fra_mia);

    fm2->travelSeg().push_back(mia_dfw);
    fm2->travelSeg().push_back(dfw_sjc);
    fm2->travelSeg().push_back(sjc_ord);
    fm2->travelSeg().push_back(ord_fra);

    // Attach the fare markets to the _itin
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
}

// APO-44172: embedded arnk does not mark gateways incorrectly
// APO-45157:R3 has GTWY_REQUIRED with recurring segments.
// fc travel segs = airseg1+arunk+airSeg2+airseg3
// Gateway is at dest of airseg2. Stopover at dest of airseg1
// Stopover is not at gateway.  So, fare should fail when gtwy is required in R3
void testGtwyRequiredWithEmbeddedArunk()
{
   _trx->diagnostic().diagnosticType() = Diagnostic308;
   _trx->diagnostic().activate();

   createItinWithStopoverThenArunkThenGatewaySegment();
   PricingUnit* pu1 = _farePath->pricingUnit()[0];

   StopoversInfo soInfo1;
   createBaseStopoversInfo(soInfo1);
   soInfo1.itemNo() = 1;
   soInfo1.geoTblItemNoGateway() = 0;
   soInfo1.gtwyInd() = 'P'; //gateway required
   soInfo1.noStopsMax() = "01";

   // 2 recurring segments
   // so not permitted in nyc and kwi
   StopoversInfoSeg* soInfoSeg1 = new StopoversInfoSeg();
   createBaseStopoversInfoSeg(soInfoSeg1);
   soInfoSeg1->loc1().locType() = LOCTYPE_CITY;
   soInfoSeg1->loc1().loc() = "NYC";
   soInfoSeg1->stopoverGeoAppl() =  Stopovers::SEG_GEO_APPL_NOT_PERMITTED;

   StopoversInfoSeg* soInfoSeg2 = new StopoversInfoSeg();
   createBaseStopoversInfoSeg(soInfoSeg2);
   soInfoSeg2->loc1().locType() = LOCTYPE_CITY;
   soInfoSeg2->loc1().loc() = "KWI";
   soInfoSeg2->stopoverGeoAppl() =  Stopovers::SEG_GEO_APPL_NOT_PERMITTED;

   soInfo1.segs().push_back(soInfoSeg1);
   soInfo1.segs().push_back(soInfoSeg2);
   soInfo1.segCnt() = 2;

   _soInfoWrapper->soInfo(&soInfo1);

    PaxTypeFare* fare = pu1->fareUsage()[1]->paxTypeFare();
    FareMarket* fm2 = _itin->fareMarket()[1];
    //ORD is a gateway; SO is in DFW
    Record3ReturnTypes ret1 = _so->validate(*_trx, *_itin, *fare, _soInfoWrapper, *fm2);

    bool finalPass = false;
    if ((ret1 == PASS || ret1 == SOFTPASS) && (_soInfoWrapper->needToProcessResults()))
    {
      ret1 = _soInfoWrapper->processResults(*_trx, *fare);
      finalPass= (ret1 == PASS || ret1 == SOFTPASS);
    }
    CPPUNIT_ASSERT_MESSAGE("Error:testGtwyRequiredWithEmbeddedArunkFCLevel", finalPass );

    finalPass = false;
    FareUsage* fu = pu1->fareUsage()[1];
    ret1 = _so->validate(*_trx, &soInfo1, *_farePath, *pu1, *fu );
    finalPass= (ret1 == PASS || ret1 == SOFTPASS);
    CPPUNIT_ASSERT_MESSAGE("Error:testGtwyRequiredWithEmbeddedArunk", !finalPass );
}

private:
  tse::PricingTrx* _trx;
  tse::PricingRequest* _req;
  tse::PricingOptions* _options;
  tse::Agent* _agent;
  tse::Itin* _itin;
  tse::FarePath* _farePath;
  tse::StopoversInfoWrapper* _soInfoWrapper;
  tse::Stopovers* _so;
  tse::PaxType* _paxType;
  tse::TestMemHandle _memHandle;

  // Locations
  //
  const tse::Loc* sfo;
  const tse::Loc* dfw;
  const tse::Loc* dal; // Love Field
  const tse::Loc* sjc; // San Jose
  const tse::Loc* jfk;
  const tse::Loc* bos;
  const tse::Loc* lga;
  const tse::Loc* lax;
  const tse::Loc* iah; // Houston
  const tse::Loc* mel;
  const tse::Loc* syd;
  const tse::Loc* hkg; // Hong Kong
  const tse::Loc* nrt; // Tokyo, Narita
  const tse::Loc* mia;
  const tse::Loc* yyz; // Toronto, Canada
  const tse::Loc* yvr; // Vancouver, Canada
  const tse::Loc* lhr; // London
  const tse::Loc* gig; // Brazil
  const tse::Loc* hnl; // Honalulu (Hawaii)
  const tse::Loc* stt; // St Thomas (Virgin Islands)
  const tse::Loc* anc; // Anchorage, Alaska
  const tse::Loc* sju; // San Juan (Puerto Rico)
  const tse::Loc* cdg; // Paris
  const tse::Loc* mex;
  const tse::Loc* bom;
  const tse::Loc* dus;
  const tse::Loc* sof;
  const tse::Loc* mxp;
  const tse::Loc* lgw;
  const tse::Loc* sin;
  const tse::Loc* tpe;
  const tse::Loc* pvg;
  const tse::Loc* ogg;
  const tse::Loc* ord;
  const tse::Loc* otp;
  const tse::Loc* dub;
  const tse::Loc* kef;
  const tse::Loc* fra;
  const tse::Loc* kwi;
  const tse::Loc* mct;
  const tse::Loc* dxb;
  const tse::Loc* icn;
  const tse::Loc* tyo;
  const tse::Loc* osa;
  const tse::Loc* ewr;

  tse::Agent* _abacusAgent;
  tse::Customer* _abacusTJR;
  tse::StopoversInfo _soInfo;
  std::map<std::string, const Loc*> locMap;

  static tse::DateTime _dateAfterPhase2;
  ServiceFeeUtil* _util;

  class TSMBuilder
  {
  public:
    TSMBuilder(TestMemHandle& memHandle) : _memHandle(memHandle) { clear(); }
    TSMBuilder& withLoc(const Loc* org, const Loc* dst)
    {
      _tsm->travelSeg()->origin() = org;
      _tsm->travelSeg()->destination() = dst;
      return *this;
    }
    TSMBuilder& withSoType(Stopovers::StopType sotype)
    {
      _tsm->stopType() = sotype;
      return *this;
    }
    Stopovers::TravelSegMarkup* build()
    {
      Stopovers::TravelSegMarkup* tsm = _tsm;
      clear();
      return tsm;
    }
  private:
    void clear()
    {
      AirSeg* ts = _memHandle.create<AirSeg>();
      _tsm = _memHandle.create<Stopovers::TravelSegMarkup>(ts);
    }
    TestMemHandle& _memHandle;
    Stopovers::TravelSegMarkup* _tsm;
  };

  class SoISMBuilder
  {
    TestMemHandle& _memHandle;
    StopoversInfoSeg* _soInfoSeg;

  public:
    SoISMBuilder(TestMemHandle& memHandle) : _memHandle(memHandle) { clear(); }
    void clear() { _soInfoSeg = _memHandle.create<StopoversInfoSeg>(); }
    SoISMBuilder& appl(Indicator v)
    {
      _soInfoSeg->stopoverGeoAppl() = v;
      return *this;
    }
    SoISMBuilder& noStops(const std::string& v)
    {
      _soInfoSeg->noStops() = v;
      return *this;
    }
    SoISMBuilder& dir(Indicator v)
    {
      _soInfoSeg->stopoverInOutInd() = v;
      return *this;
    }
    SoISMBuilder& order(int v)
    {
      _soInfoSeg->orderNo() = v;
      return *this;
    }
    Stopovers::StopoversInfoSegMarkup* build(uint16_t matchCount = 0)
    {
      Stopovers::StopoversInfoSegMarkup* markup =
          _memHandle.create<Stopovers::StopoversInfoSegMarkup>();
      markup->initialize(_soInfoSeg);
      markup->matchCount() = matchCount;
      clear();
      return markup;
    }
  };
};
CPPUNIT_TEST_SUITE_REGISTRATION(StopoversTest);

DateTime StopoversTest::_dateAfterPhase2 = DateTime(2010, 4, 5, 14, 20, 0);
}
