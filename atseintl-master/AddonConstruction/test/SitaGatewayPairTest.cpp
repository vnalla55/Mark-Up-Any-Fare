#include "test/include/CppUnitHelperMacros.h"
#include "AddonConstruction/SitaGatewayPair.h"
#include "DBAccess/SITAFareInfo.h"
#include "DBAccess/SITAAddonFareInfo.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/RoutingConsts.h"

namespace tse
{
static const Indicator ARULE_INCLUDE = 'N';
static const Indicator ARULE_EXCLUDE = 'Y';
static const char BMPM_ANY = 'Y'; // match any fares
static const char CLFB_ROUTING_ONLY = 'N'; // match spec. fares with routings
static const char CLFB_MPM_ONLY = 'O'; // match spec. fares with mpm

class SitaGatewayPairTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SitaGatewayPairTest);

  CPPUNIT_TEST(matchRuleTariffEmpty);
  CPPUNIT_TEST(matchRuleTariffEqualPass);
  CPPUNIT_TEST(matchRuleTariffDifferentFail);

  CPPUNIT_TEST(matchApplicableRuleEmpty);
  CPPUNIT_TEST(matchApplicableRuleIncludeRulesEmpty);
  CPPUNIT_TEST(matchApplicableRuleIncludeRulesNotEmptyRuleIsFound);
  CPPUNIT_TEST(matchApplicableRuleIncludeRulesNotEmptyRuleIsNotFound);
  CPPUNIT_TEST(matchApplicableRuleExcludeRuleIsNotFound);
  CPPUNIT_TEST(matchApplicableRuleExcludeRuleIsFound);
  CPPUNIT_TEST(matchApplicableRuleNotProperValues);

  CPPUNIT_TEST(matchOWRT_OWMayBeDoubledPassOWMayBeDoubled);
  CPPUNIT_TEST(matchOWRT_OWMayBeDoubledPassOWMayNotBeDoubled);
  CPPUNIT_TEST(matchOWRT_OWMayBeDoubledFail);
  CPPUNIT_TEST(matchOWRT_OWMayNotBeDoubledPass);
  CPPUNIT_TEST(matchOWRT_OWMayNotBeDoubledFailOWMayBeDoubled);
  CPPUNIT_TEST(matchOWRT_OWMayNotBeDoubledFailRoundTrip);
  CPPUNIT_TEST(matchOWRT_RTPass);
  CPPUNIT_TEST(matchOWRT_RTFailOWMayBeDoubled);
  CPPUNIT_TEST(matchOWRT_RTFailOWMayNotBeDoubled);
  CPPUNIT_TEST(matchOWRT_Addon4Pass);
  CPPUNIT_TEST(matchOWRT_Addon4FailOneWayMayNotBeDoubled);
  CPPUNIT_TEST(matchOWRT_Addon4FailRT);
  CPPUNIT_TEST(matchOWRT_Addon5Fail);
  CPPUNIT_TEST(matchOWRT_Addon5PassOWMayNotBeDoubled);
  CPPUNIT_TEST(matchOWRT_Addon5PassOWMayBeDoubled);
  CPPUNIT_TEST(matchOWRT_Addon5PassRT);

  CPPUNIT_TEST(matchRoutingUnknown);
  CPPUNIT_TEST(matchRoutingAny);
  CPPUNIT_TEST(matchRoutingCLFBRoutingOnlyFareMileage);
  CPPUNIT_TEST(matchRoutingCLFBRoutingOnlyFareNotMileageAddonFareMileage);
  CPPUNIT_TEST(matchRoutingClfbRoutingOnlyFareNotMileageAddonFareNotMileageRoutingNumbersEqual);
  CPPUNIT_TEST(matchRoutingClfbRoutingOnlyFareNotMileageAddonFareNotMileageRoutingNumbersNotEqual);
  CPPUNIT_TEST(matchRoutingClfbMPMOnlyFareNotMileage);
  CPPUNIT_TEST(matchRoutingClfbMPMOnlyFareMileage);
  CPPUNIT_TEST(matchRouteCodePass);
  CPPUNIT_TEST(matchRouteCodeNoMatch);

  CPPUNIT_TEST(matchTariffFamilyBlank);
  CPPUNIT_TEST(matchTariffFamilyExactMatch);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchGPass);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchGFail);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchHPass);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchHFail);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchJPass);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchJFail);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchKPass);
  CPPUNIT_TEST(matchTariffFamilyGenericMatchKFail);

  CPPUNIT_TEST_SUITE_END();

  SITAAddonFareInfo _af;
  SITAFareInfo _sf;

public:
  void matchRuleTariffEmpty()
  {
    _af.ruleTariff() = 0;
    _sf.fareTariff() = 1;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRuleTariff(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRuleTariffEqualPass()
  {
    _af.ruleTariff() = 1;
    _sf.fareTariff() = 1;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRuleTariff(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRuleTariffDifferentFail()
  {
    _af.ruleTariff() = 1;
    _sf.fareTariff() = 2;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRuleTariff(_af, _sf) == FM_SITA_RTAR);
  }

  void matchOWRT_OWMayBeDoubledPassOWMayBeDoubled()
  {
    _af.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    _sf.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_OWMayBeDoubledPassOWMayNotBeDoubled()
  {
    _af.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    _sf.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_OWMayBeDoubledFail()
  {
    _af.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    _sf.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_OWMayNotBeDoubledPass()
  {
    _af.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    _sf.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_OWMayNotBeDoubledFailOWMayBeDoubled()
  {
    _af.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    _sf.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_OWMayNotBeDoubledFailRoundTrip()
  {
    _af.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    _sf.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_RTPass()
  {
    _af.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _sf.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_RTFailOWMayBeDoubled()
  {
    _af.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _sf.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_RTFailOWMayNotBeDoubled()
  {
    _af.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _sf.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_Addon4Pass()
  {
    _af.owrt() = SITA_OWRT_ADDON_4;
    _sf.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_Addon4FailOneWayMayNotBeDoubled()
  {
    _af.owrt() = SITA_OWRT_ADDON_4;
    _sf.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_Addon4FailRT()
  {
    _af.owrt() = SITA_OWRT_ADDON_4;
    _sf.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_Addon5Fail()
  {
    _af.owrt() = SITA_OWRT_ADDON_5;
    _sf.owrt() = ' ';
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_OWRT);
  }

  void matchOWRT_Addon5PassOWMayNotBeDoubled()
  {
    _af.owrt() = SITA_OWRT_ADDON_5;
    _sf.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_Addon5PassOWMayBeDoubled()
  {
    _af.owrt() = SITA_OWRT_ADDON_5;
    _sf.owrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchOWRT_Addon5PassRT()
  {
    _af.owrt() = SITA_OWRT_ADDON_5;
    _sf.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(SitaGatewayPair::matchOWRT(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRoutingUnknown()
  {
    _af.baseMPMInd() = ' ';
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_SITA_BMPM_UNKNOWN);
  }

  void matchRoutingAny()
  {
    _af.baseMPMInd() = BMPM_ANY;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRoutingCLFBRoutingOnlyFareMileage()
  {
    _af.baseMPMInd() = CLFB_ROUTING_ONLY;
    _sf.routingNumber() = MILEAGE_ROUTING;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_SITA_BMPM_CANT_WITH_MILEAGE);
  }

  void matchRoutingCLFBRoutingOnlyFareNotMileageAddonFareMileage()
  {
    _af.baseMPMInd() = CLFB_ROUTING_ONLY;
    _sf.routingNumber() = "0001";
    _af.baseFareRouting() = MILEAGE_ROUTING;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRoutingClfbRoutingOnlyFareNotMileageAddonFareNotMileageRoutingNumbersEqual()
  {
    _af.baseMPMInd() = CLFB_ROUTING_ONLY;
    _sf.routingNumber() = "0001";
    _af.baseFareRouting() = "0001";
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRoutingClfbRoutingOnlyFareNotMileageAddonFareNotMileageRoutingNumbersNotEqual()
  {
    _af.baseMPMInd() = CLFB_ROUTING_ONLY;
    _sf.routingNumber() = "0001";
    _af.baseFareRouting() = "0101";
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_SITA_BRTG);
  }

  void matchRoutingClfbMPMOnlyFareNotMileage()
  {
    _af.baseMPMInd() = CLFB_MPM_ONLY;
    _sf.routingNumber() = "0001";
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_SITA_BMPM_CANT_WITH_ROUTING);
  }

  void matchRoutingClfbMPMOnlyFareMileage()
  {
    _af.baseMPMInd() = CLFB_MPM_ONLY;
    _sf.routingNumber() = MILEAGE_ROUTING;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouting(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRouteCodePass()
  {
    _af.routeCode() = 11;
    _sf.routeCode() = 11;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouteCode(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchRouteCodeNoMatch()
  {
    _af.routeCode() = 11;
    _sf.routeCode() = 33;
    CPPUNIT_ASSERT(SitaGatewayPair::matchRouteCode(_af, _sf) == FM_SITA_ROUTE_CODE);
  }

  void matchTariffFamilyBlank()
  {
    _af.tariffFamily() = ' ';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchTariffFamilyExactMatch()
  {
    _af.tariffFamily() = 'G';
    _sf.tariffFamily() = 'G';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _af.tariffFamily() = 'H';
    _sf.tariffFamily() = 'H';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _af.tariffFamily() = 'J';
    _sf.tariffFamily() = 'J';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _af.tariffFamily() = 'K';
    _sf.tariffFamily() = 'K';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchTariffFamilyGenericMatchGPass()
  {
    _af.tariffFamily() = 'G';
    _sf.tariffFamily() = 'D';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'Q';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'U';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'V';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchTariffFamilyGenericMatchGFail()
  {
    _af.tariffFamily() = 'G';
    _sf.tariffFamily() = 'J';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'K';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'W';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'B';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'T';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'H';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
  }

  void matchTariffFamilyGenericMatchHPass()
  {
    _af.tariffFamily() = 'H';
    _sf.tariffFamily() = 'R';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'W';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'B';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchTariffFamilyGenericMatchHFail()
  {
    _af.tariffFamily() = 'H';
    _sf.tariffFamily() = 'G';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'D';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'J';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'K';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'Q';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'U';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'V';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'T';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
  }

  void matchTariffFamilyGenericMatchJPass()
  {
    _af.tariffFamily() = 'J';
    _sf.tariffFamily() = 'U';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'D';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'T';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'V';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'Q';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchTariffFamilyGenericMatchJFail()
  {
    _af.tariffFamily() = 'J';
    _sf.tariffFamily() = 'G';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'K';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'R';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'W';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'H';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'B';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
  }

  void matchTariffFamilyGenericMatchKPass()
  {
    _af.tariffFamily() = 'K';
    _sf.tariffFamily() = 'R';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'W';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
    _sf.tariffFamily() = 'B';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchTariffFamilyGenericMatchKFail()
  {
    _af.tariffFamily() = 'K';
    _sf.tariffFamily() = 'G';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'Q';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'J';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'U';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'V';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'D';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'H';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
    _sf.tariffFamily() = 'T';
    CPPUNIT_ASSERT(SitaGatewayPair::matchTariffFamily(_af, _sf) == FM_SITA_TARIFF_FAMILY);
  }

  void matchTariff() {}

  void matchApplicableRuleEmpty()
  {
    _sf.ruleNumber() = "";
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchApplicableRuleIncludeRulesEmpty()
  {
    _sf.ruleNumber() = "0000";
    _af.ruleExcludeInd() = ARULE_INCLUDE;
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchApplicableRuleIncludeRulesNotEmptyRuleIsFound()
  {
    _sf.ruleNumber() = "0000";
    _af.rules().insert("0000");
    _af.ruleExcludeInd() = ARULE_INCLUDE;
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchApplicableRuleIncludeRulesNotEmptyRuleIsNotFound()
  {
    _sf.ruleNumber() = "0000";
    _af.rules().insert("1111");
    _af.ruleExcludeInd() = ARULE_INCLUDE;
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_SITA_ARULE_INCLUDE);
  }

  void matchApplicableRuleExcludeRuleIsNotFound()
  {
    _sf.ruleNumber() = "0000";
    _af.rules().insert("1111");
    _af.ruleExcludeInd() = ARULE_EXCLUDE;
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_GOOD_MATCH);
  }

  void matchApplicableRuleExcludeRuleIsFound()
  {
    _sf.ruleNumber() = "0000";
    _af.rules().insert("0000");
    _af.ruleExcludeInd() = ARULE_EXCLUDE;
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_SITA_ARULE_EXCLUDE);
  }

  void matchApplicableRuleNotProperValues()
  {
    _sf.ruleNumber() = "0000";
    _af.rules().insert("0000");
    _af.ruleExcludeInd() = 'X';
    CPPUNIT_ASSERT(SitaGatewayPair::matchApplicableRule(_af, _sf) == FM_SITA_ARULE_UNKNOWN);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SitaGatewayPairTest);
}
