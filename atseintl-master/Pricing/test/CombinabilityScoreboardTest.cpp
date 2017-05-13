#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Pricing/CombinabilityScoreboard.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag605Collector.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/FareClassAppInfo.h"
#include "Pricing/Combinations.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{

namespace
{
// MOCKS
}

class CombinabilityScoreboardTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CombinabilityScoreboardTest);

  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testValidate_Pass_OneWay);
  CPPUNIT_TEST(testValidate_Pass_CircleTrip);
  CPPUNIT_TEST(testValidate_Pass_Unknown);
  CPPUNIT_TEST(testValidate_Fail_EmptyPU);
  CPPUNIT_TEST(testValidate_Fail_EmptyFU);
  CPPUNIT_TEST(testValidate_Fail_Rec2Cat10FromFU_NoPUToEOE);
  CPPUNIT_TEST(testValidate_Fail_Rec2Cat10FromFU_NoOneway);

  CPPUNIT_TEST(testProcessSamePointTable993_NotOpenJaw);
  CPPUNIT_TEST(testProcessSamePointTable993_OneFU);
  CPPUNIT_TEST(testProcessSamePointTable993_FareMarketRoundTrip);
  CPPUNIT_TEST(testProcessSamePointTable993_FareMarketOneWay);

  CPPUNIT_TEST(testInvalidate_Pass_OneFU);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_RtPermitted_MirrorImage_Y);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_RtPermitted_MirrorImage_N);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_RtPermitted_MirrorImage_R);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_RtNotPermitted_MirrorImage_V);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_RtNotPermitted_MirrorImage_X);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_RtNotPermitted_MirrorImage_W);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_RtPermitted_NotMirrorImage_Y);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_RtPermitted_NotMirrorImage_V);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_RtNotPermitted_NotMirrorImage_N);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_RtNotPermitted_NotMirrorImage_X);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_Chk102Permitted);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_Chk102Permitted_SecondFU);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_Chk102Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_Chk102Carrier_3FU);
  CPPUNIT_TEST(testInvalidate_Pass_Roundtrip_Chk102Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_Chk102Rule);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_Chk102Tariff);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_Chk102Class);
  CPPUNIT_TEST(testInvalidate_Fail_Roundtrip_Chk102Type);
  CPPUNIT_TEST(testInvalidate_Pass_Circletrip_CtPermitted);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_CtNotPermitted);
  CPPUNIT_TEST(testInvalidate_Pass_Circletrip_Chk103Permitted);
  CPPUNIT_TEST(testInvalidate_Pass_Circletrip_Chk103Permitted_SecondFU);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_Chk103Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_Chk103Carrier_3FU);
  CPPUNIT_TEST(testInvalidate_Pass_Circletrip_Chk103Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_Chk103Rule);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_Chk103Tariff);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_Chk103Class);
  CPPUNIT_TEST(testInvalidate_Fail_Circletrip_Chk103Type);
  CPPUNIT_TEST(testInvalidate_Pass_OpenJaw_SojPermitted_DestOpenJaw);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_SojNotPermitted_DestOpenJaw);
  CPPUNIT_TEST(testInvalidate_Pass_OpenJaw_SojPermitted_OrigOpenJaw);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_SojNotPermitted_OrigOpenJaw);
  CPPUNIT_TEST(testInvalidate_Pass_OpenJaw_DojPermitted);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_DojNotPermitted);
  CPPUNIT_TEST(testInvalidate_Pass_OpenJaw_Chk101Permitted);
  CPPUNIT_TEST(testInvalidate_Pass_OpenJaw_Chk101Permitted_SecondFU);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_Chk101Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_Chk101Carrier_3FU);
  CPPUNIT_TEST(testInvalidate_Pass_OpenJaw_Chk101Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_Chk101Rule);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_Chk101Tariff);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_Chk101Class);
  CPPUNIT_TEST(testInvalidate_Fail_OpenJaw_Chk101Type);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_EoeNotPermitted);
  CPPUNIT_TEST(testInvalidate_Pass_OneWay_Chk104Permitted);
  CPPUNIT_TEST(testInvalidate_Pass_OneWay_Chk104Permitted_SecondFU);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_Chk104Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_Chk104Carrier_3FU);
  CPPUNIT_TEST(testInvalidate_Pass_OneWay_Chk104Carrier);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_Chk104Rule);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_Chk104Tariff);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_Chk104Class);
  CPPUNIT_TEST(testInvalidate_Fail_OneWay_Chk104Type);
  CPPUNIT_TEST(testInvalidate_Fail_UnknownPUType);

  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_EmptyFarePath);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_OneWay_Rec2Cat10FromFU_EoeInd);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_OneWay_Rec2Cat10FromFU_NoPUToEOE_Required);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_OneWay_Rec2Cat10FromFU_NoPUToEOE_D);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_OneWay_Rec2Cat10FromFU_Required);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_OneWay_Rec2Cat10FromFU_D);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_Rec2Cat10FromFU_EoeInd);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_Rec2Cat10FromFU_NoPUToEOE_NotPermitted);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Pass_Rec2Cat10FromFU_NoPUToEOE_B);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_Rec2Cat10FromFU_NotPermitted);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_Rec2Cat10FromFU_B);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_A);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_B);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_C);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_D);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_A);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_B);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_C);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_D);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_A);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_B);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_C);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_PU_D);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_A);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_B);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_C);
  CPPUNIT_TEST(testAnalyzeOneWay_2_Fail_SideTrip_FU_D);

  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_EmptyPU);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_DifferentCountry_SojInd_NoSameNationOJ_U);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_DifferentCountry_SojInd_NoSameNationOJ_V);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_DifferentCountry_DojInd_NoSameNationOJ_U);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_DifferentCountry_DojInd_NoSameNationOJ_V);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_SojInd_PUSubType_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_SojInd_PUSubType_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_DojInd_PUSubType_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_DojInd_PUSubType_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_SojInd_SameNationOJ_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_SojInd_SameNationOJ_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_DojInd_SameNationOJ_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SameCountry_DojInd_SameNationOJ_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_SojInd);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Pass_DojInd);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_DestinationOpenJawRequired);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_OriginOpenJawRequired);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_Max2IntlFares_PermittedS);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_Max2IntlFares_RestrictionsT);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_Max2IntlFares_NoSameNationOJ_PermittedS);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_Max2IntlFares_NoSameNationOJ_RestrictionsT);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_DifferentCountry_SojInd_U);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_DifferentCountry_SojInd_V);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_DifferentCountry_DojInd_U);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_DifferentCountry_DojInd_V);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_SojInd_IntlFares_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_SojInd_IntlFares_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_DojInd_IntlFares_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_DojInd_IntlFares_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_SojInd_PUSubType_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_SojInd_PUSubType_X);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_DojInd_PUSubType_W);
  CPPUNIT_TEST(testAnalyzeOpenJaw_Fail_SameCountry_DojInd_PUSubType_X);

  CPPUNIT_TEST(testAnalyzeRoundTrip_Pass_EmptyPU);
  CPPUNIT_TEST(testAnalyzeRoundTrip_Pass_Y_MirrorImage);
  CPPUNIT_TEST(testAnalyzeRoundTrip_Pass_Y_RoundTrip);
  CPPUNIT_TEST(testAnalyzeRoundTrip_Fail_MirrorImage_V);
  CPPUNIT_TEST(testAnalyzeRoundTrip_Fail_MirrorImage_X);
  CPPUNIT_TEST(testAnalyzeRoundTrip_Fail_MirrorImage_W);
  CPPUNIT_TEST(testAnalyzeRoundTrip_Fail_RoundTrip);

  CPPUNIT_TEST(testOutputDiag);
  CPPUNIT_TEST(testOutputDiag_NoSourceFU);
  CPPUNIT_TEST(testOutputDiag_NoSourceFU_NoSuffix);
  CPPUNIT_TEST(testOutputDiag_NoTargetFU);
  CPPUNIT_TEST(testOutputDiag_SourceAndTargetFUEqual);

  CPPUNIT_TEST(testDisplayDiag_1_NotActive);
  CPPUNIT_TEST(testDisplayDiag_1_PASSED_VALIDATION);
  CPPUNIT_TEST(testDisplayDiag_1_PASSED_SYSTEM_ASSUMPTION);
  CPPUNIT_TEST(testDisplayDiag_1_FAILED_UNSPECIFIED);
  CPPUNIT_TEST(testDisplayDiag_1_FAILED_REC_2_CAT_10_NOT_APPLICABLE);
  CPPUNIT_TEST(testDisplayDiag_1_FAILED_NO_REC_2_CAT_10);
  CPPUNIT_TEST(testDisplayDiag_1_NotEnoughFUs);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_RtPermitted);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_RtNotPermitted);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_RtNotPermitted_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Permitted);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Carrier_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Carrier);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Carrier_3FU);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Rule_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Rule);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Tariff_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Tariff);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Class_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Class);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Type_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Roundtrip_Chk102Type);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_CtPermitted);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_CtNotPermitted);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_CtNotPermitted_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Permitted);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Carrier_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Carrier);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Carrier_3FU);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Rule_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Rule);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Tariff_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Tariff);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Class_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Class);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Type_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_Circletrip_Chk103Type);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_SojPermitted_DestOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_SojNotPermitted_DestOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_SojNotPermitted_WrongFailureReason_DestOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_SojPermitted_OrigOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_SojNotPermitted_OrigOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_SojNotPermitted_WrongFailureReason_OrigOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_DojPermitted_DoubleOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_DojNotPermitted_DoubleOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_DojNotPermitted_WrongFailureReason_DoubleOpenJaw);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Permitted);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Carrier_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Carrier);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Carrier_3FU);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Rule_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Rule);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Tariff_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Tariff);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Class_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Class);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Type_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OpenJaw_Chk101Type);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_EoeNotPermitted);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_EoeNotPermitted_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_DojNotPermitted_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Permitted);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Carrier_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Carrier);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Carrier_3FU);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Rule_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Rule);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Tariff_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Tariff);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Class_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Class);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Type_WrongFailureReason);
  CPPUNIT_TEST(testDisplayDiag_1_OneWay_Chk104Type);
  CPPUNIT_TEST(testDisplayDiag_1_Unknown);

  CPPUNIT_TEST(testDisplayDiag_2_NotActive);
  CPPUNIT_TEST(testDisplayDiag_2_INVALID_DIAGNOSTIC_TOO_FEW_FARES);
  CPPUNIT_TEST(testDisplayDiag_2_PASSED_VALIDATION);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_UNSPECIFIED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_REC_2_CAT_10_NOT_APPLICABLE);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_NO_REC_2_CAT_10);
  CPPUNIT_TEST(testDisplayDiag_2_PASSED_SYSTEM_ASSUMPTION);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_REC2_SCOREBOARD_WithFareUsage);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_ROUND_TRIP_NOT_PERMITTED_WithFareUsages);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_CARRIER_REQUIRED_FOR_RT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_RULE_REQUIRED_FOR_RT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_TARIFF_REQUIRED_FOR_RT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_FARECLASS_REQUIRED_FOR_RT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_FARETYPE_REQUIRED_FOR_RT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_CIRCLE_TRIP_NOT_PERMITTED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_CARRIER_REQUIRED_FOR_CT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_RULE_REQUIRED_FOR_CT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_TARIFF_REQUIRED_FOR_CT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_FARECLASS_REQUIRED_FOR_CT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_FARETYPE_REQUIRED_FOR_CT);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_END_ON_END_NOT_PERMITTED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_END_ON_END_SAME_CARRIER_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_END_ON_END_SAME_RULE_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_END_ON_END_SAME_TARIFF_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_END_ON_END_SAME_FARECLASS_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_END_ON_END_SAME_FARETYPE_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_DESTINATION_OPEN_JAW_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_ORIGIN_OPEN_JAW_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_2_MAX_INTL_FARES_SAME_COUNTRY_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_OJ_DIFF_COUNTRY_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_OJ_SAME_COUNTRY_REQUIRED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_MIRROR_IMAGE_NOT_PERMITTED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_SIDE_TRIP_NOT_PERMITTED);
  CPPUNIT_TEST(testDisplayDiag_2_FAILED_REQUIRED_EOE_WITH_OTHER_PU);

  CPPUNIT_TEST(testIsMirrorImage_Pass);
  CPPUNIT_TEST(testIsMirrorImage_Fail_Fares);
  CPPUNIT_TEST(testIsMirrorImage_Fail_Owrt);
  CPPUNIT_TEST(testIsMirrorImage_Fail_PUType_Default);
  CPPUNIT_TEST(testIsMirrorImage_Fail_PUType_Unknown);
  CPPUNIT_TEST(testIsMirrorImage_Fail_PUType_OpenJaw);
  CPPUNIT_TEST(testIsMirrorImage_Fail_PUType_CircleTrip);
  CPPUNIT_TEST(testIsMirrorImage_Fail_PUType_OneWay);

  CPPUNIT_TEST(testSameYYGovCxr);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingUnit* _pu;
  DiagCollector* _dc;
  FarePath* _fp;
  FareMarket* _fm;
  FareUsage* _ffu;
  CombinabilityScoreboard* _comboScoreboard;
  PricingTrx* _trx;
  PricingRequest* _pr;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _pu = _memHandle.create<PricingUnit>();
    _dc = _memHandle.insert(new Diag605Collector());
    _fp = _memHandle.create<FarePath>();
    _fp->pricingUnit().push_back(_pu);
    _ffu = 0;
    _comboScoreboard = _memHandle.create<CombinabilityScoreboard>();
    _trx = _memHandle.create<PricingTrx>();
    _comboScoreboard->_trx = _trx;
    _pr = _memHandle.create<PricingRequest>();
    _trx->setRequest(_pr);
    _pr->ticketingDT() = DateTime(2025, 10, 10);
  }

  void tearDown() { _memHandle.clear(); }

  void createEmptyFU() { _pu->fareUsage().push_back(_memHandle.create<FareUsage>()); }

  FareUsage*
  createFU(Indicator fareInfoOwrt = ROUND_TRIP_MAYNOT_BE_HALVED, FareClassCode fareInfoFc = "")
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_owrt = fareInfoOwrt;
    fi->_fareClass = fareInfoFc;

    TariffCrossRefInfo* tcri = _memHandle.create<TariffCrossRefInfo>();

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);
    fare->setTariffCrossRefInfo(tcri);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = ptf;

    _pu->fareUsage().push_back(fu);

    return fu;
  }

  FareUsage* createFUWithCri(Indicator eoeInd = ' ',
                             Indicator sojInd = ' ',
                             Indicator dojInd = ' ',
                             Indicator sojorigIndestInd = ' ',
                             Indicator ct2Ind = ' ',
                             Indicator applInd = Combinations::REQUIRED,
                             Indicator fareInfoOwrt = ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    CombinabilityRuleInfo* cri = _memHandle.create<CombinabilityRuleInfo>();
    cri->eoeInd() = eoeInd;
    cri->sojInd() = sojInd;
    cri->dojInd() = dojInd;
    cri->sojorigIndestInd() = sojorigIndestInd;
    cri->ct2Ind() = ct2Ind;
    cri->applInd() = applInd;

    FareUsage* fu = createFU(fareInfoOwrt);
    fu->rec2Cat10() = cri;

    return fu;
  }

  void createInternationalFUWithCri(int count = 1,
                                    Indicator sojInd = ' ',
                                    Indicator dojInd = ' ',
                                    Indicator sojorigIndestInd = ' ')
  {
    for (int i = 0; i < count; ++i)
    {
      FareUsage* fu = createFUWithCri(' ', sojInd, dojInd, sojorigIndestInd);
      fu->paxTypeFare()->fare()->status().set(Fare::FS_International, true);
    }
  }

  void createFUWithFareMarket(const LocCode& boardMultiCity, const LocCode& offMultiCity)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->boardMultiCity() = boardMultiCity;
    fm->offMultiCity() = offMultiCity;

    FareUsage* fu = createFUWithCri();
    fu->paxTypeFare()->fareMarket() = fm;
    // bypassing RuleUtil::validateSamePoint
    fu->rec2Cat10()->samepointstblItemNo() = 0;
  }

  FareUsage* createFUForDiagTests(const std::string& fareClass, const std::string& ruleNumber)
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->fareClass() = fareClass;
    fi->ruleNumber() = ruleNumber;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = ptf;

    _pu->fareUsage().push_back(fu);

    return fu;
  }

  FareUsage* createFUWithCriAndPtfForDiagTests(const std::string& carrier = "",
                                               const std::string& ruleNumber = "",
                                               const std::string& fareClass = "",
                                               const std::string& fareType = "",
                                               int ruleTariff = 0)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    _pu->fareUsage().push_back(fu);

    CombinabilityRuleInfo* cri = _memHandle.create<CombinabilityRuleInfo>();
    initializeCri(*cri);
    fu->rec2Cat10() = cri;

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    initializePtf(*ptf, carrier, ruleNumber, fareClass, fareType, ruleTariff);
    fu->paxTypeFare() = ptf;

    return fu;
  }

  void initializeCri(CombinabilityRuleInfo& cri)
  {
    cri.dojSameCarrierInd() = ' ';
    cri.dojSameRuleTariffInd() = ' ';
    cri.dojSameFareInd() = ' ';

    cri.ct2SameCarrierInd() = ' ';
    cri.ct2SameRuleTariffInd() = ' ';
    cri.ct2SameFareInd() = ' ';

    cri.ct2plusSameCarrierInd() = ' ';
    cri.ct2plusSameRuleTariffInd() = ' ';
    cri.ct2plusSameFareInd() = ' ';

    cri.eoeSameCarrierInd() = ' ';
    cri.eoeSameRuleTariffInd() = ' ';
    cri.eoeSameFareInd() = ' ';

    cri.sojInd() = ' ';
    cri.dojInd() = ' ';
    cri.ct2Ind() = ' ';
    cri.ct2plusInd() = ' ';
    cri.eoeInd() = ' ';
  }

  void initializePtf(PaxTypeFare& ptf,
                     const std::string& carrier = "",
                     const std::string& ruleNumber = "",
                     const std::string& fareClass = "",
                     const std::string& fareType = "",
                     int ruleTariff = 0)
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->carrier() = carrier;
    fi->ruleNumber() = ruleNumber;
    fi->fareClass() = fareClass;
    // to make CombinabilityScoreboard::isMirrorImage() handling easier
    fi->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    TariffCrossRefInfo* tcri = _memHandle.create<TariffCrossRefInfo>();
    tcri->ruleTariff() = ruleTariff;

    FareClassAppInfo* fcti = _memHandle.create<FareClassAppInfo>();
    fcti->_fareType = fareType;
    ptf.fareClassAppInfo() = fcti;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);
    fare->setTariffCrossRefInfo(tcri);

    ptf.setFare(fare);
  }

  // ***** TESTS *****

  void testConstructor() {}

  void testValidate_Pass_OneWay()
  {
    createFUWithCri();
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testValidate_Pass_CircleTrip()
  {
    createFUWithCri();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testValidate_Pass_Unknown()
  {
    createFUWithCri();
    _pu->puType() = PricingUnit::Type::UNKNOWN;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testValidate_Fail_EmptyPU()
  {
    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testValidate_Fail_EmptyFU()
  {
    createEmptyFU();

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testValidate_Fail_Rec2Cat10FromFU_NoPUToEOE()
  {
    createFUWithCri(' ', ' ', ' ', ' ', ' ', Combinations::NOT_APPLICABLE);
    _pu->noPUToEOE() = false;
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testValidate_Fail_Rec2Cat10FromFU_NoOneway()
  {
    createFUWithCri(' ', ' ', ' ', ' ', ' ', Combinations::NOT_APPLICABLE);
    _pu->noPUToEOE() = true;
    _pu->puType() = PricingUnit::Type::UNKNOWN;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->validate(*_pu, _ffu, *_dc));
  }

  void testProcessSamePointTable993_NotOpenJaw()
  {
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    _pu->puSubType() = PricingUnit::UNKNOWN_SUBTYPE;

    _comboScoreboard->processSamePointTable993(*_pu, *_dc);
    CPPUNIT_ASSERT(PricingUnit::Type::UNKNOWN == _pu->puType());
    CPPUNIT_ASSERT_EQUAL(PricingUnit::UNKNOWN_SUBTYPE, _pu->puSubType());
  }

  void testProcessSamePointTable993_OneFU()
  {
    createFUWithFareMarket("DFW", "FRA");
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::UNKNOWN_SUBTYPE;

    _comboScoreboard->processSamePointTable993(*_pu, *_dc);
    CPPUNIT_ASSERT(PricingUnit::Type::OPENJAW == _pu->puType());
    CPPUNIT_ASSERT_EQUAL(PricingUnit::UNKNOWN_SUBTYPE, _pu->puSubType());
  }

  void testProcessSamePointTable993_FareMarketRoundTrip()
  {
    createFUWithFareMarket("DFW", "FRA");
    createFUWithFareMarket("FRA", "DFW");
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::UNKNOWN_SUBTYPE;

    _comboScoreboard->processSamePointTable993(*_pu, *_dc);
    CPPUNIT_ASSERT(PricingUnit::Type::OPENJAW == _pu->puType());
    CPPUNIT_ASSERT_EQUAL(PricingUnit::UNKNOWN_SUBTYPE, _pu->puSubType());
  }

  void testProcessSamePointTable993_FareMarketOneWay()
  {
    createFUWithFareMarket("DFW", "FRA");
    createFUWithFareMarket("FRA", "MUC");
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::UNKNOWN_SUBTYPE;

    _comboScoreboard->processSamePointTable993(*_pu, *_dc);
    CPPUNIT_ASSERT(PricingUnit::Type::OPENJAW == _pu->puType());
    CPPUNIT_ASSERT_EQUAL(PricingUnit::UNKNOWN_SUBTYPE, _pu->puSubType());
  }

  void testInvalidate_Pass_OneFU()
  {
    createFU();
    _pu->puType() = PricingUnit::Type::UNKNOWN;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Pass_Roundtrip_RtPermitted_MirrorImage_Y()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2Ind() = 'Y';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2Ind() = 'Y';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Pass_Roundtrip_RtPermitted_MirrorImage_N()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2Ind() = 'N';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2Ind() = 'N';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Pass_Roundtrip_RtPermitted_MirrorImage_R()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2Ind() = 'R';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2Ind() = 'R';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_Roundtrip_RtNotPermitted_MirrorImage_V()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2Ind() = 'V';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2Ind() = 'V';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_RT_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_Roundtrip_RtNotPermitted_MirrorImage_X()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2Ind() = 'X';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2Ind() = 'X';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_RT_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_Roundtrip_RtNotPermitted_MirrorImage_W()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2Ind() = 'W';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2Ind() = 'W';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_RT_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Pass_Roundtrip_RtPermitted_NotMirrorImage_Y()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests("AB");
    fu0->rec2Cat10()->ct2Ind() = 'Y';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests("CD");
    fu1->rec2Cat10()->ct2Ind() = 'Y';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Pass_Roundtrip_RtPermitted_NotMirrorImage_V()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests("AB");
    fu0->rec2Cat10()->ct2Ind() = 'V';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests("CD");
    fu1->rec2Cat10()->ct2Ind() = 'V';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_Roundtrip_RtNotPermitted_NotMirrorImage_N()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests("AB");
    fu0->rec2Cat10()->ct2Ind() = 'N';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests("CD");
    fu1->rec2Cat10()->ct2Ind() = 'N';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_RT_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_Roundtrip_RtNotPermitted_NotMirrorImage_X()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests("AB");
    fu0->rec2Cat10()->ct2Ind() = 'X';
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests("CD");
    fu1->rec2Cat10()->ct2Ind() = 'X';
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_RT_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Pass_Roundtrip_Chk102Permitted()
  {
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Pass_Roundtrip_Chk102Permitted_SecondFU()
  {
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARECLASS;
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_Roundtrip_Chk102Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR RT") != string::npos);
  }

  void testInvalidate_Fail_Roundtrip_Chk102Carrier_3FU()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR RT") != string::npos);
  }

  void testInvalidate_Pass_Roundtrip_Chk102Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_Roundtrip_Chk102Rule()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR RT") != string::npos);
  }

  void testInvalidate_Fail_Roundtrip_Chk102Tariff()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR RT") != string::npos);
  }

  void testInvalidate_Fail_Roundtrip_Chk102Class()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "Y");
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "F");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR RT") != string::npos);
  }

  void testInvalidate_Fail_Roundtrip_Chk102Type()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "ABC");
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "DEF");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR RT") != string::npos);
  }

  void testInvalidate_Pass_Circletrip_CtPermitted()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2plusInd() = Combinations::PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2plusInd() = Combinations::PERMITTED;
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_Circletrip_CtNotPermitted()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->ct2plusInd() = Combinations::NOT_PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->ct2plusInd() = Combinations::NOT_PERMITTED;
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_CT_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_ffu == fu0);
  }

  void testInvalidate_Pass_Circletrip_Chk103Permitted()
  {
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Pass_Circletrip_Chk103Permitted_SecondFU()
  {
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_Circletrip_Chk103Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR CT") != string::npos);
  }

  void testInvalidate_Fail_Circletrip_Chk103Carrier_3FU()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR CT") != string::npos);
  }

  void testInvalidate_Pass_Circletrip_Chk103Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_Circletrip_Chk103Rule()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR CT") != string::npos);
  }

  void testInvalidate_Fail_Circletrip_Chk103Tariff()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR CT") != string::npos);
  }

  void testInvalidate_Fail_Circletrip_Chk103Class()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "Y");
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "F");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR CT") != string::npos);
  }

  void testInvalidate_Fail_Circletrip_Chk103Type()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "ABC");
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "DEF");
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR CT") != string::npos);
  }

  void testInvalidate_Pass_OpenJaw_SojPermitted_DestOpenJaw()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->sojInd() = Combinations::PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->sojInd() = Combinations::PERMITTED;
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_OpenJaw_SojNotPermitted_DestOpenJaw()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_SOJ_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_ffu == fu0);
  }

  void testInvalidate_Pass_OpenJaw_SojPermitted_OrigOpenJaw()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->sojInd() = Combinations::PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->sojInd() = Combinations::PERMITTED;
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_OpenJaw_SojNotPermitted_OrigOpenJaw()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_SOJ_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_ffu == fu0);
  }

  void testInvalidate_Pass_OpenJaw_DojPermitted()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->dojInd() = Combinations::PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->dojInd() = Combinations::PERMITTED;
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Fail_OpenJaw_DojNotPermitted()
  {
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->dojInd() = Combinations::NOT_PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->dojInd() = Combinations::NOT_PERMITTED;
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_DOJ_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_ffu == fu0);
  }

  void testInvalidate_Pass_OpenJaw_Chk101Permitted()
  {
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Pass_OpenJaw_Chk101Permitted_SecondFU()
  {
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARECLASS;
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_OpenJaw_Chk101Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR OJ") != string::npos);
  }

  void testInvalidate_Fail_OpenJaw_Chk101Carrier_3FU()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR OJ") != string::npos);
  }

  void testInvalidate_Pass_OpenJaw_Chk101Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_OpenJaw_Chk101Rule()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR OJ") != string::npos);
  }

  void testInvalidate_Fail_OpenJaw_Chk101Tariff()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR OJ") != string::npos);
  }

  void testInvalidate_Fail_OpenJaw_Chk101Class()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "Y");
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "F");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR OJ") != string::npos);
  }

  void testInvalidate_Fail_OpenJaw_Chk101Type()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "ABC");
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "DEF");
    _pu->puType() = PricingUnit::Type::OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR OJ") != string::npos);
  }

  void testInvalidate_Fail_OneWay_EoeNotPermitted()
  {
    _dc->activate();
    FareUsage* fu0 = createFUWithCriAndPtfForDiagTests();
    fu0->rec2Cat10()->eoeInd() = Combinations::NOT_PERMITTED;
    FareUsage* fu1 = createFUWithCriAndPtfForDiagTests();
    fu1->rec2Cat10()->eoeInd() = Combinations::NOT_PERMITTED;
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_EOE_NOT_PERMITTED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("END ON END NOT PERMITTED") != string::npos);
    CPPUNIT_ASSERT(!_ffu);
  }

  void testInvalidate_Pass_OneWay_Chk104Permitted()
  {
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Pass_OneWay_Chk104Permitted_SecondFU()
  {
    createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB", "123", "Y");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARECLASS;
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_OneWay_Chk104Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR END ON END") != string::npos);
  }

  void testInvalidate_Fail_OneWay_Chk104Carrier_3FU()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    createFUWithCriAndPtfForDiagTests("CD");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR END ON END") != string::npos);
  }

  void testInvalidate_Pass_OneWay_Chk104Carrier()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("AB");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("AB");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testInvalidate_Fail_OneWay_Chk104Rule()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR END ON END") != string::npos);
  }

  void testInvalidate_Fail_OneWay_Chk104Tariff()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR END ON END") != string::npos);
  }

  void testInvalidate_Fail_OneWay_Chk104Class()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "Y");
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "F");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR END ON END") != string::npos);
  }

  void testInvalidate_Fail_OneWay_Chk104Type()
  {
    _dc->activate();
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "ABC");
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "DEF");
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR END ON END") != string::npos);
  }

  void testInvalidate_Fail_UnknownPUType()
  {
    createFUWithCriAndPtfForDiagTests();
    createFUWithCriAndPtfForDiagTests();
    _pu->puType() = PricingUnit::Type::UNKNOWN;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->invalidate(*_pu, _ffu, *_dc));
  }

  void testAnalyzeOneWay_2_Pass_EmptyFarePath()
  {
    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Pass_OneWay_Rec2Cat10FromFU_EoeInd()
  {
    createFUWithCri(Combinations::NOT_APPLICABLE);
    _pu->puType() = PricingUnit::Type::ONEWAY;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }
  void testAnalyzeOneWay_2_Pass_OneWay_Rec2Cat10FromFU_NoPUToEOE_Required()
  {
    createFUWithCri(Combinations::REQUIRED);
    _pu->puType() = PricingUnit::Type::ONEWAY;
    _pu->noPUToEOE() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Pass_OneWay_Rec2Cat10FromFU_NoPUToEOE_D()
  {
    createFUWithCri('D');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    _pu->noPUToEOE() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_OneWay_Rec2Cat10FromFU_Required()
  {
    createFUWithCri(Combinations::REQUIRED);
    _pu->puType() = PricingUnit::Type::ONEWAY;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_EOE_REQUIRED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_OneWay_Rec2Cat10FromFU_D()
  {
    createFUWithCri('D');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_EOE_REQUIRED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Pass_Rec2Cat10FromFU_EoeInd()
  {
    createFUWithCri(Combinations::NOT_APPLICABLE);
    _pu->puType() = PricingUnit::Type::UNKNOWN;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Pass_Rec2Cat10FromFU_NoPUToEOE_NotPermitted()
  {
    createFUWithCri(Combinations::NOT_PERMITTED);
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Pass_Rec2Cat10FromFU_NoPUToEOE_B()
  {
    createFUWithCri('B');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_Rec2Cat10FromFU_NotPermitted()
  {
    createFUWithCri(Combinations::NOT_PERMITTED);
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    _pu->noPUToEOE() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_EOE_NOT_PERMITTED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_Rec2Cat10FromFU_B()
  {
    createFUWithCri('B');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    _pu->noPUToEOE() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_EOE_NOT_PERMITTED, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_A()
  {
    FareUsage* fu = createFUWithCri('A');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_B()
  {
    FareUsage* fu = createFUWithCri('B');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_C()
  {
    FareUsage* fu = createFUWithCri('C');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_OW_D()
  {
    FareUsage* fu = createFUWithCri('D');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_A()
  {
    FareUsage* fu = createFUWithCri('A');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_B()
  {
    FareUsage* fu = createFUWithCri('B');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_C()
  {
    FareUsage* fu = createFUWithCri('C');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_OW_D()
  {
    FareUsage* fu = createFUWithCri('D');
    _pu->puType() = PricingUnit::Type::ONEWAY;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_A()
  {
    FareUsage* fu = createFUWithCri('A');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_B()
  {
    FareUsage* fu = createFUWithCri('B');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_C()
  {
    FareUsage* fu = createFUWithCri('C');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_PU_D()
  {
    FareUsage* fu = createFUWithCri('D');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = false;
    _pu->isSideTripPU() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_A()
  {
    FareUsage* fu = createFUWithCri('A');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_B()
  {
    FareUsage* fu = createFUWithCri('B');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;
    _pu->noPUToEOE() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_C()
  {
    FareUsage* fu = createFUWithCri('C');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOneWay_2_Fail_SideTrip_FU_D()
  {
    FareUsage* fu = createFUWithCri('D');
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    fu->hasSideTrip() = true;
    _pu->isSideTripPU() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOneWay(*_fp, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_EmptyPU()
  {
    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_DifferentCountry_SojInd_NoSameNationOJ_U()
  {
    createFUWithCri(' ', 'U', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_DifferentCountry_SojInd_NoSameNationOJ_V()
  {
    createFUWithCri(' ', 'V', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_DifferentCountry_DojInd_NoSameNationOJ_U()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'U', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_DifferentCountry_DojInd_NoSameNationOJ_V()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'V', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_SojInd_PUSubType_W()
  {
    createFUWithCri(' ', 'W', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_SojInd_PUSubType_X()
  {
    createFUWithCri(' ', 'X', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_DojInd_PUSubType_W()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'W', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_DojInd_PUSubType_X()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'X', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_SojInd_SameNationOJ_W()
  {
    createFUWithCri(' ', 'W', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_SojInd_SameNationOJ_X()
  {
    createFUWithCri(' ', 'X', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_DojInd_SameNationOJ_W()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'W', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SameCountry_DojInd_SameNationOJ_X()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'X', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_SojInd()
  {
    createFUWithCri(' ', 'Y', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Pass_DojInd()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'Y', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_DestinationOpenJawRequired()
  {
    createFUWithCri(' ', ' ', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::UNKNOWN_SUBTYPE;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_OriginOpenJawRequired()
  {
    createFUWithCri(' ', ' ', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::UNKNOWN_SUBTYPE;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_Max2IntlFares_PermittedS()
  {
    createInternationalFUWithCri(3, Combinations::PERMITTED_S);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_Max2IntlFares_RestrictionsT()
  {
    createInternationalFUWithCri(3, Combinations::RESTRICTIONS_T);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_Max2IntlFares_NoSameNationOJ_PermittedS()
  {
    createFUWithCri(' ', Combinations::PERMITTED_S);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_Max2IntlFares_NoSameNationOJ_RestrictionsT()
  {
    createFUWithCri(' ', Combinations::RESTRICTIONS_T);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_DifferentCountry_SojInd_U()
  {
    createFUWithCri(' ', 'U', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_DifferentCountry_SojInd_V()
  {
    createFUWithCri(' ', 'V', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_DifferentCountry_DojInd_U()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'U', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_DifferentCountry_DojInd_V()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'V', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    _pu->sameNationOJ() = true;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_SojInd_IntlFares_W()
  {
    createInternationalFUWithCri(3, 'W', ' ', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_SojInd_IntlFares_X()
  {
    createInternationalFUWithCri(3, 'X', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_DojInd_IntlFares_W()
  {
    createInternationalFUWithCri(3, Combinations::PERMITTED, 'W', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_DojInd_IntlFares_X()
  {
    createInternationalFUWithCri(3, Combinations::PERMITTED, 'X', Combinations::DEST_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_SojInd_PUSubType_W()
  {
    createFUWithCri(' ', 'W', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_SojInd_PUSubType_X()
  {
    createFUWithCri(' ', 'X', ' ', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_DojInd_PUSubType_W()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'W', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeOpenJaw_Fail_SameCountry_DojInd_PUSubType_X()
  {
    createFUWithCri(' ', Combinations::PERMITTED, 'X', Combinations::ORIGIN_OPEN_JAW_REQ);
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    _pu->sameNationOJ() = false;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeOpenJaw(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Pass_EmptyPU()
  {
    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Pass_Y_MirrorImage()
  {
    createFUWithCri(' ', ' ', ' ', ' ', 'Y');
    createFUWithCri(' ', ' ', ' ', ' ', 'Y');
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Pass_Y_RoundTrip()
  {
    createFUWithCri(' ', ' ', ' ', ' ', 'Y', ' ', ALL_WAYS);
    createFUWithCri(' ', ' ', ' ', ' ', 'Y');
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_PASSED, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Fail_MirrorImage_V()
  {
    createFUWithCri(' ', ' ', ' ', ' ', 'V');
    createFU();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Fail_MirrorImage_X()
  {
    createFUWithCri(' ', ' ', ' ', ' ', 'X');
    createFU();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Fail_MirrorImage_W()
  {
    createFUWithCri(' ', ' ', ' ', ' ', 'W');
    createFU();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testAnalyzeRoundTrip_Fail_RoundTrip()
  {
    createFUWithCri(' ', ' ', ' ', ' ', 'X', ' ', ALL_WAYS);
    createFU();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;

    CPPUNIT_ASSERT_EQUAL(CVR_UNSPECIFIED_FAILURE, _comboScoreboard->analyzeRoundTrip(*_pu, *_dc));
  }

  void testOutputDiag()
  {
    _dc->activate();
    FareUsage* sfu = createFUForDiagTests("Y", "1234");
    FareUsage* tfu = createFUForDiagTests("F", "4321");
    _comboScoreboard->outputDiag(*_dc, 0, 0, sfu, tfu);
    CPPUNIT_ASSERT(_dc->str().find("Y-1234 - F") != string::npos);
  }

  void testOutputDiag_NoSourceFU()
  {
    _dc->activate();
    _comboScoreboard->outputDiag(*_dc, "test_prefix", "test_suffix", 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("test_prefixtest_suffix") != string::npos);
  }

  void testOutputDiag_NoSourceFU_NoSuffix()
  {
    _dc->activate();
    _comboScoreboard->outputDiag(*_dc, "test_prefix", 0, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("test_prefix") != string::npos);
  }

  void testOutputDiag_NoTargetFU()
  {
    _dc->activate();
    FareUsage* sfu = createFUForDiagTests("Y", "1234");
    _comboScoreboard->outputDiag(*_dc, 0, 0, sfu, 0);
    CPPUNIT_ASSERT(_dc->str().find("Y-1234") != string::npos);
  }

  void testOutputDiag_SourceAndTargetFUEqual()
  {
    _dc->activate();
    FareUsage* sfu = createFUForDiagTests("Y", "1234");
    _comboScoreboard->outputDiag(*_dc, 0, 0, sfu, sfu);
    CPPUNIT_ASSERT(_dc->str().find("Y-1234") != string::npos);
  }

  void testDisplayDiag_1_NotActive()
  {
    _dc->deActivate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::PASSED_VALIDATION);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_PASSED_VALIDATION()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::PASSED_VALIDATION);
    CPPUNIT_ASSERT(_dc->str().find("PASSED RECORD 2 SCOREBOARD CHECK") != string::npos);
  }

  void testDisplayDiag_1_PASSED_SYSTEM_ASSUMPTION()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::PASSED_SYSTEM_ASSUMPTION);
    CPPUNIT_ASSERT(_dc->str().find("PASSED COMBINATION - SYSTEM ASSUMPTION") != string::npos);
  }

  void testDisplayDiag_1_FAILED_UNSPECIFIED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_UNSPECIFIED);
    CPPUNIT_ASSERT(_dc->str().find("FAILED COMBINATION") != string::npos);
  }

  void testDisplayDiag_1_FAILED_REC_2_CAT_10_NOT_APPLICABLE()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_REC_2_CAT_10_NOT_APPLICABLE);
    CPPUNIT_ASSERT(_dc->str().find("FAILED COMBINATION - RECORD 2 CAT 10 NOT APPLICABLE") !=
                   string::npos);
  }

  void testDisplayDiag_1_FAILED_NO_REC_2_CAT_10()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_NO_REC_2_CAT_10);
    CPPUNIT_ASSERT(_dc->str().find("FAILED COMBINATION - NO RECORD 2 CAT 10") != string::npos);
  }

  void testDisplayDiag_1_NotEnoughFUs()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::INVALID_DIAGNOSTIC_TOO_FEW_FARES);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_RtPermitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->ct2Ind() = Combinations::PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_ROUND_TRIP_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_RtNotPermitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->ct2Ind() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_ROUND_TRIP_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().find("ROUND TRIP NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_1_Roundtrip_RtNotPermitted_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->ct2Ind() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Permitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    createFUWithCriAndPtfForDiagTests();
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Carrier_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Carrier()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_RT);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_1_Roundtrip_Chk102Carrier_3FU()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("ABC");
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_RT);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_1_Roundtrip_Chk102Rule_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Rule()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_RULE_REQUIRED_FOR_RT);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_1_Roundtrip_Chk102Tariff_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Tariff()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->ct2SameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_TARIFF_REQUIRED_FOR_RT);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_1_Roundtrip_Chk102Class_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Class()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARECLASS_REQUIRED_FOR_RT);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_1_Roundtrip_Chk102Type_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Roundtrip_Chk102Type()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->ct2SameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARETYPE_REQUIRED_FOR_RT);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_CtPermitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->ct2plusInd() = Combinations::PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_CIRCLE_TRIP_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_CtNotPermitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->ct2plusInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_CIRCLE_TRIP_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().find("CIRCLE TRIP NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_CtNotPermitted_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->ct2plusInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Permitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    createFUWithCriAndPtfForDiagTests();
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Carrier_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Carrier()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_CT);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_Chk103Carrier_3FU()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("ABC");
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_CT);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_Chk103Rule_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Rule()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_RULE_REQUIRED_FOR_CT);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_Chk103Tariff_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Tariff()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->ct2plusSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_TARIFF_REQUIRED_FOR_CT);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_Chk103Class_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Class()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARECLASS_REQUIRED_FOR_CT);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_1_Circletrip_Chk103Type_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_Circletrip_Chk103Type()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->ct2plusSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARETYPE_REQUIRED_FOR_CT);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_SojPermitted_DestOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->sojInd() = Combinations::PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_SojNotPermitted_DestOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().find("SINGLE OJ NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_SojNotPermitted_WrongFailureReason_DestOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DEST_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_SojPermitted_OrigOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->sojInd() = Combinations::PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_SojNotPermitted_OrigOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().find("SINGLE OJ NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_SojNotPermitted_WrongFailureReason_OrigOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::ORIG_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->sojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_DojPermitted_DoubleOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->dojInd() = Combinations::PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_DojNotPermitted_DoubleOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->dojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().find("DOUBLE OJ NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_DojNotPermitted_WrongFailureReason_DoubleOpenJaw()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    _pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->dojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Permitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    createFUWithCriAndPtfForDiagTests();
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Carrier_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Carrier()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_Chk101Carrier_3FU()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("ABC");
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_Chk101Rule_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Rule()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_Chk101Tariff_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Tariff()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->dojSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_Chk101Class_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Class()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_1_OpenJaw_Chk101Type_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OpenJaw_Chk101Type()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->dojSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_1_OneWay_EoeNotPermitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->eoeInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_NOT_PERMITTED);
    CPPUNIT_ASSERT(_dc->str().find("END ON END NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_1_OneWay_EoeNotPermitted_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->eoeInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_DojNotPermitted_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests();
    fu->rec2Cat10()->dojInd() = Combinations::NOT_PERMITTED;
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Permitted()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    createFUWithCriAndPtfForDiagTests();
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Carrier_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Carrier()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_CARRIER_REQUIRED);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_1_OneWay_Chk104Carrier_3FU()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("ABC");
    fu->rec2Cat10()->eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    createFUWithCriAndPtfForDiagTests("ABC");
    createFUWithCriAndPtfForDiagTests("DEF");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_CARRIER_REQUIRED);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_1_OneWay_Chk104Rule_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Rule()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "123");
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    createFUWithCriAndPtfForDiagTests("", "456");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_RULE_REQUIRED);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_1_OneWay_Chk104Tariff_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Tariff()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "", 123);
    fu->rec2Cat10()->eoeSameRuleTariffInd() = Combinations::SAME_TARIFF;
    createFUWithCriAndPtfForDiagTests("", "", "", "", 456);

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_TARIFF_REQUIRED);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_1_OneWay_Chk104Class_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Class()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "A");
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARECLASS;
    createFUWithCriAndPtfForDiagTests("", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_FARECLASS_REQUIRED);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_1_OneWay_Chk104Type_WrongFailureReason()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_1_OneWay_Chk104Type()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::ONEWAY;
    FareUsage* fu = createFUWithCriAndPtfForDiagTests("", "", "", "A");
    fu->rec2Cat10()->eoeSameFareInd() = Combinations::SAME_FARETYPE;
    createFUWithCriAndPtfForDiagTests("", "", "", "B");

    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_FARETYPE_REQUIRED);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_1_Unknown()
  {
    _dc->activate();
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    createFUWithCriAndPtfForDiagTests();
    createFUWithCriAndPtfForDiagTests();

    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_2_NotActive()
  {
    _dc->deActivate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::INVALID_DIAGNOSTIC_TOO_FEW_FARES, 0, 0);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testDisplayDiag_2_INVALID_DIAGNOSTIC_TOO_FEW_FARES()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::INVALID_DIAGNOSTIC_TOO_FEW_FARES, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("INVALID DIAGNOSTIC - TOO FEW FARES") != string::npos);
  }

  void testDisplayDiag_2_PASSED_VALIDATION()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::PASSED_VALIDATION, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("PASSED RECORD 2 SCOREBOARD CHECK") != string::npos);
  }

  void testDisplayDiag_2_FAILED_UNSPECIFIED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(*_dc, *_pu, CombinabilityScoreboard::FAILED_UNSPECIFIED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("FAILED COMBINATION") != string::npos);
  }

  void testDisplayDiag_2_FAILED_REC_2_CAT_10_NOT_APPLICABLE()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_REC_2_CAT_10_NOT_APPLICABLE, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("FAILED COMBINATION - RECORD 2 CAT 10 NOT APPLICABLE") !=
                   string::npos);
  }

  void testDisplayDiag_2_FAILED_NO_REC_2_CAT_10()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_NO_REC_2_CAT_10, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("FAILED COMBINATION - NO RECORD 2 CAT 10") != string::npos);
  }

  void testDisplayDiag_2_PASSED_SYSTEM_ASSUMPTION()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::PASSED_SYSTEM_ASSUMPTION, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("PASSED COMBINATION - SYSTEM ASSUMPTION") != string::npos);
  }

  void testDisplayDiag_2_FAILED_REC2_SCOREBOARD_WithFareUsage()
  {
    _dc->activate();
    FareUsage* sfu = createFUForDiagTests("Y", "1234");
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_REC2_SCOREBOARD, sfu, 0);
    CPPUNIT_ASSERT(_dc->str().find("FAILED RECORD 2 SCOREBOARD CHECK") != string::npos);
    CPPUNIT_ASSERT(_dc->str().find("Y-1234") != string::npos);
    CPPUNIT_ASSERT(_dc->str().find("FARE") != string::npos);
  }

  void testDisplayDiag_2_FAILED_ROUND_TRIP_NOT_PERMITTED_WithFareUsages()
  {
    _dc->activate();
    FareUsage* sfu = createFUForDiagTests("Y", "1234");
    FareUsage* tfu = createFUForDiagTests("F", "4321");
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_ROUND_TRIP_NOT_PERMITTED, sfu, tfu);
    CPPUNIT_ASSERT(_dc->str().find("FAILED FARE") != string::npos);
    CPPUNIT_ASSERT(_dc->str().find("Y-1234 - F") != string::npos);
    CPPUNIT_ASSERT(_dc->str().find("ROUND TRIP NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_CARRIER_REQUIRED_FOR_RT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_RT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_RULE_REQUIRED_FOR_RT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_RULE_REQUIRED_FOR_RT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_TARIFF_REQUIRED_FOR_RT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_TARIFF_REQUIRED_FOR_RT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_FARECLASS_REQUIRED_FOR_RT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARECLASS_REQUIRED_FOR_RT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_FARETYPE_REQUIRED_FOR_RT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARETYPE_REQUIRED_FOR_RT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR RT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_CIRCLE_TRIP_NOT_PERMITTED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_CIRCLE_TRIP_NOT_PERMITTED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("CIRCLE TRIP NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_CARRIER_REQUIRED_FOR_CT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_CT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_RULE_REQUIRED_FOR_CT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_RULE_REQUIRED_FOR_CT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_TARIFF_REQUIRED_FOR_CT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_TARIFF_REQUIRED_FOR_CT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_FARECLASS_REQUIRED_FOR_CT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARECLASS_REQUIRED_FOR_CT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_FARETYPE_REQUIRED_FOR_CT()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARETYPE_REQUIRED_FOR_CT, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR CT") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SINGLE OJ NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("DOUBLE OJ NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR OJ") != string::npos);
  }

  void testDisplayDiag_2_FAILED_END_ON_END_NOT_PERMITTED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_NOT_PERMITTED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("END ON END NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_END_ON_END_SAME_CARRIER_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_CARRIER_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME CARRIER REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_2_FAILED_END_ON_END_SAME_RULE_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_RULE_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME RULE REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_2_FAILED_END_ON_END_SAME_TARIFF_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_TARIFF_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME TARIFF REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_2_FAILED_END_ON_END_SAME_FARECLASS_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_FARECLASS_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARECLASS REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_2_FAILED_END_ON_END_SAME_FARETYPE_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_END_ON_END_SAME_FARETYPE_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SAME FARETYPE REQ FOR END ON END") != string::npos);
  }

  void testDisplayDiag_2_FAILED_DESTINATION_OPEN_JAW_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_DESTINATION_OPEN_JAW_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("DEST OPEN JAW REQUIRED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_ORIGIN_OPEN_JAW_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_ORIGIN_OPEN_JAW_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("ORIGIN OPEN JAW REQUIRED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_2_MAX_INTL_FARES_SAME_COUNTRY_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_2_MAX_INTL_FARES_SAME_COUNTRY_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("2 MAX INTL FARES SAME COUNTRY REQUIRED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_OJ_DIFF_COUNTRY_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_OJ_DIFF_COUNTRY_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("OJ DIFF COUNTRY REQUIRED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_OJ_SAME_COUNTRY_REQUIRED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_OJ_SAME_COUNTRY_REQUIRED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("OJ SAME COUNTRY REQUIRED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_MIRROR_IMAGE_NOT_PERMITTED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_MIRROR_IMAGE_NOT_PERMITTED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("MIRROR IMAGE IS NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_SIDE_TRIP_NOT_PERMITTED()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_SIDE_TRIP_NOT_PERMITTED, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("SIDE TRIP IS NOT PERMITTED") != string::npos);
  }

  void testDisplayDiag_2_FAILED_REQUIRED_EOE_WITH_OTHER_PU()
  {
    _dc->activate();
    _comboScoreboard->displayDiag(
        *_dc, *_pu, CombinabilityScoreboard::FAILED_REQUIRED_EOE_WITH_OTHER_PU, 0, 0);
    CPPUNIT_ASSERT(_dc->str().find("REQUIRED EOE WITH OTHER PU") != string::npos);
  }

  void testIsMirrorImage_Pass()
  {
    createFU();
    createFU();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    CPPUNIT_ASSERT(_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_Fares()
  {
    createFU();
    createFU(ROUND_TRIP_MAYNOT_BE_HALVED, "ABC");
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_Owrt()
  {
    createFU(ALL_WAYS);
    createFU();
    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_PUType_Default()
  {
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_PUType_Unknown()
  {
    _pu->puType() = PricingUnit::Type::UNKNOWN;
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_PUType_OpenJaw()
  {
    _pu->puType() = PricingUnit::Type::OPENJAW;
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_PUType_CircleTrip()
  {
    _pu->puType() = PricingUnit::Type::CIRCLETRIP;
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  void testIsMirrorImage_Fail_PUType_OneWay()
  {
    _pu->puType() = PricingUnit::Type::ONEWAY;
    CPPUNIT_ASSERT(!_comboScoreboard->isMirrorImage(*_pu));
  }

  class PTFStub : public PaxTypeFare
  {
  public:
    PTFStub(TestMemHandle& memHandle, CarrierCode cxr, CarrierCode govCxr)
    {
      _carrier = cxr;
      _fareMarket = memHandle(new FareMarket());
      _fareMarket->governingCarrier() = govCxr;
      _fare = memHandle(new Fare());
      _fare->setFareInfo(memHandle(new FareInfo()));
      _fareClassAppInfo = memHandle(new FareClassAppInfo());
    }
  };

  void testSameYYGovCxr()
  {
    PTFStub ptf1(_memHandle, INDUSTRY_CARRIER, CARRIER_WS);
    PTFStub ptf2(_memHandle, CARRIER_WS, CARRIER_WS);

    size_t carrierCount = 0, s1 = 0, s2, s3, s4, s5;

    _comboScoreboard->comparePaxTypeFares(ptf1, ptf2, carrierCount, s2, s3, s4, s5);
    CPPUNIT_ASSERT_EQUAL(++s1, carrierCount);
    _comboScoreboard->comparePaxTypeFares(ptf2, ptf1, carrierCount, s2, s3, s4, s5);
    CPPUNIT_ASSERT_EQUAL(++s1, carrierCount);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinabilityScoreboardTest);

} // namespace tse
