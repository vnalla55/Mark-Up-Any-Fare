#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/MCPCarrierUtil.h"
#include "DataModel/PricingTrx.h"

using namespace std;

namespace tse
{
class MCPCarrierUtilTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(MCPCarrierUtilTest);
  CPPUNIT_TEST(testGetConfig_EmptyHostFail);
  CPPUNIT_TEST(testGetConfig_HostPass);
  CPPUNIT_TEST(testGetConfig_EmptyMapFail);
  CPPUNIT_TEST(testGetConfig_MapPass);
  CPPUNIT_TEST(testGetCxrPair_EmptyStringFail);
  CPPUNIT_TEST(testGetCxrPair_InvTokenFail);
  CPPUNIT_TEST(testGetCxrPair_WrongNumCarriersFail);
  CPPUNIT_TEST(testGetCxrPair_WrongActualCarrierLenFail);
  CPPUNIT_TEST(testGetCxrPair_WrongPseudoCarrierLenFail);
  CPPUNIT_TEST(testGetCxrPair_Pass);
  CPPUNIT_TEST(testInitialize_WrongData);
  CPPUNIT_TEST(testInitialize_Pass);
  CPPUNIT_TEST(testSwapToActual_trxFlagOff);
  CPPUNIT_TEST(testSwapToActual_NoMatch);
  CPPUNIT_TEST(testSwapToActual_Match);
  CPPUNIT_TEST(testSwapToPseudo_trxFlagOff);
  CPPUNIT_TEST(testSwapToPseudo_NoMatch);
  CPPUNIT_TEST(testSwapToPseudo_Match);
  CPPUNIT_TEST(testIsPseudoCarrier_NoMatch);
  CPPUNIT_TEST(testIsPseudoCarrier_Match);
  CPPUNIT_TEST(testIsMcpHost_NoMatch);
  CPPUNIT_TEST(testIsMcpHost_Match);
  CPPUNIT_TEST(testIsSameGroupCarriers);

  CPPUNIT_TEST(testGetPreferedCarrier_CarrierToLong);
  CPPUNIT_TEST(testGetPreferedCarrier_CarrierToShort);
  CPPUNIT_TEST(testGetPreferedCarrier_NoCarrier);
  CPPUNIT_TEST(testGetPreferedCarrier_Pass);
  CPPUNIT_TEST(testGetPreferedCarrier2_CfgStringEmpty);
  CPPUNIT_TEST(testGetPreferedCarrier2_CfgStringToShort);
  CPPUNIT_TEST(testGetPreferedCarrier2_CfgStringToLong);
  CPPUNIT_TEST(testGetPreferedCarrier2_InvalidHostCarrier);
  CPPUNIT_TEST(testGetPreferedCarrier2_InvalidPrefCarriers);
  CPPUNIT_TEST(testGetPreferedCarrier2_Pass);

  CPPUNIT_TEST(testIsMcpHostNeutral_Match);
  CPPUNIT_TEST(testIsNeutralCarrier_Match);
  CPPUNIT_TEST(testIsNeutralCarrier_Not_Match);
  CPPUNIT_TEST(testSwapFromNeutralToActual);
  CPPUNIT_TEST(testGetPreferedCarriers_ForNeutralPartition);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->mcpCarrierSwap() = true;
  }
  void tearDown()
  {
    _temp.clear();
    _memHandle.clear();
  }

  void testGetConfig_EmptyHostFail()
  {
    std::string strHosts;
    MCPCarrierUtil::getConfig(Global::config(), "MCP_PRIME_HOSTS", strHosts);
    CPPUNIT_ASSERT(strHosts.empty());
  }
  void testGetConfig_HostPass()
  {
    initConfig();
    std::string strHosts;
    MCPCarrierUtil::getConfig(Global::config(), "MCP_PRIME_HOSTS", strHosts);
    CPPUNIT_ASSERT(!strHosts.empty());
  }
  void testGetConfig_EmptyMapFail()
  {
    std::string strMcpCarriers;
    MCPCarrierUtil::getConfig(Global::config(), "MCP_REAL_TO_PSEUDO_CXR_MAP", strMcpCarriers);
    CPPUNIT_ASSERT(strMcpCarriers.empty());
  }
  void testGetConfig_MapPass()
  {
    initConfig();
    std::string strMcpCarriers;
    MCPCarrierUtil::getConfig(Global::config(), "MCP_REAL_TO_PSEUDO_CXR_MAP", strMcpCarriers);
    CPPUNIT_ASSERT(!strMcpCarriers.empty());
  }
  void testGetCxrPair_EmptyStringFail()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getCxrPair("", _cxrPseudo, _cxrActual));
  }
  void testGetCxrPair_InvTokenFail()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getCxrPair("LA=0I", _cxrPseudo, _cxrActual));
  }
  void testGetCxrPair_WrongNumCarriersFail()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getCxrPair("LA-0I-BA", _cxrPseudo, _cxrActual));
  }
  void testGetCxrPair_WrongActualCarrierLenFail()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getCxrPair("LAT-0I", _cxrPseudo, _cxrActual));
  }
  void testGetCxrPair_WrongPseudoCarrierLenFail()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getCxrPair("LA-I", _cxrPseudo, _cxrActual));
  }
  void testGetCxrPair_Pass()
  {
    CPPUNIT_ASSERT(MCPCarrierUtil::getCxrPair("LA-0I", _cxrPseudo, _cxrActual));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LA"), _cxrActual);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("0I"), _cxrPseudo);
  }
  void testInitialize_WrongData()
  {
    initConfig("ABC|ADD-A|BA/CA|D-AS|A|CB", "A|ASD", "L-LA/LP/4M/XL|LPLP/4M/XL");
    CPPUNIT_ASSERT_EQUAL(size_t(0), MCPCarrierUtil::_mcpCxrMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), MCPCarrierUtil::_mcpHosts.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), MCPCarrierUtil::_preferedCarriers.size());
  }
  void testInitialize_Pass()
  {
    initConfig("LA-0I|LP-L1|4M-X0", "LA|LT", "LA-LA/LP/4M/XL|LP-LP/4M/XL");
    CPPUNIT_ASSERT_EQUAL(size_t(3), MCPCarrierUtil::_mcpCxrMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), MCPCarrierUtil::_mcpHosts.size());
    CPPUNIT_ASSERT_EQUAL(size_t(7), MCPCarrierUtil::_preferedCarriers.size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), MCPCarrierUtil::_preferedCarriers.count("LA"));
    CPPUNIT_ASSERT_EQUAL(size_t(3), MCPCarrierUtil::_preferedCarriers.count("LP"));
  }
  void testSwapToActual_trxFlagOff()
  {
    _trx->mcpCarrierSwap() = false;
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("0I"), MCPCarrierUtil::swapToActual(_trx, "0I"));
  }
  void testSwapToActual_NoMatch()
  {
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LA"), MCPCarrierUtil::swapToActual(_trx, "LA"));
  }
  void testSwapToActual_Match()
  {
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LA"), MCPCarrierUtil::swapToActual(_trx, "0I"));
  }
  void testSwapToPseudo_trxFlagOff()
  {
    _trx->mcpCarrierSwap() = false;
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LA"), MCPCarrierUtil::swapToPseudo(_trx, "LA"));
  }
  void testSwapToPseudo_NoMatch()
  {
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("0I"), MCPCarrierUtil::swapToPseudo(_trx, "0I"));
  }
  void testSwapToPseudo_Match()
  {
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("0I"), MCPCarrierUtil::swapToPseudo(_trx, "LA"));
  }
  void testIsPseudoCarrier_NoMatch()
  {
    initConfig();
    CPPUNIT_ASSERT(!MCPCarrierUtil::isPseudoCarrier("LA"));
  }
  void testIsPseudoCarrier_Match()
  {
    initConfig();
    CPPUNIT_ASSERT(MCPCarrierUtil::isPseudoCarrier("0I"));
  }
  void testIsMcpHost_NoMatch()
  {
    initConfig();
    CPPUNIT_ASSERT(!MCPCarrierUtil::isMcpHost("LO"));
  }
  void testIsMcpHost_Match()
  {
    initConfig();
    CPPUNIT_ASSERT(MCPCarrierUtil::isMcpHost("LA"));
  }
  void testIsSameGroupCarriers()
  {
    initConfig("LA-0I|LP-L1|4M-X0", "LA|LT", "LA-LA/LP/4M/XL|LP-LP/4M/XL");
    CPPUNIT_ASSERT(MCPCarrierUtil::isSameGroupCarriers("LA", "LP"));
    CPPUNIT_ASSERT(MCPCarrierUtil::isSameGroupCarriers("LA", "LA"));
    CPPUNIT_ASSERT(MCPCarrierUtil::isSameGroupCarriers("LA", "XL"));
    CPPUNIT_ASSERT(MCPCarrierUtil::isSameGroupCarriers("LP", "LP"));
    CPPUNIT_ASSERT(!MCPCarrierUtil::isSameGroupCarriers("LP", "LA"));
    CPPUNIT_ASSERT(!MCPCarrierUtil::isSameGroupCarriers("LP", "AA"));
    CPPUNIT_ASSERT(!MCPCarrierUtil::isSameGroupCarriers("AA", "DL"));
  }
  void testGetPreferedCarrier_CarrierToLong()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("LAN/LO", _temp));
  }
  void testGetPreferedCarrier_CarrierToShort()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("LA/L", _temp));
  }
  void testGetPreferedCarrier_NoCarrier()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("", _temp));
  }
  void testGetPreferedCarrier_Pass()
  {
    CPPUNIT_ASSERT(MCPCarrierUtil::getPreferedCarrier("LA/LP/4M/XL", _temp));
    CPPUNIT_ASSERT(_temp.size() == 4);
  }
  void testGetPreferedCarrier2_CfgStringEmpty()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("", _cxrActual, _temp));
  }
  void testGetPreferedCarrier2_CfgStringToShort()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("LA", _cxrActual, _temp));
  }
  void testGetPreferedCarrier2_CfgStringToLong()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("LA-LP-4M", _cxrActual, _temp));
  }
  void testGetPreferedCarrier2_InvalidHostCarrier()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("LAN-LA/LP/4M/XL", _cxrActual, _temp));
  }
  void testGetPreferedCarrier2_InvalidPrefCarriers()
  {
    CPPUNIT_ASSERT(!MCPCarrierUtil::getPreferedCarrier("LA-LAN/LP/4M/XL", _cxrActual, _temp));
  }
  void testGetPreferedCarrier2_Pass()
  {
    CPPUNIT_ASSERT(MCPCarrierUtil::getPreferedCarrier("LA-LA/LP/4M/XL", _cxrActual, _temp));
    CPPUNIT_ASSERT(_cxrActual == "LA");
    CPPUNIT_ASSERT(_temp.size() == 4);
  }

  void testIsMcpHostNeutral_Match()
  {
    initConfig();
    CPPUNIT_ASSERT(MCPCarrierUtil::isMcpHost("96"));
  }

  void testIsNeutralCarrier_Match()
  {
    initConfig();
    CPPUNIT_ASSERT(MCPCarrierUtil::isNeutralCarrier("96"));
  }
  void testIsNeutralCarrier_Not_Match()
  {
    initConfig();
    CPPUNIT_ASSERT(!MCPCarrierUtil::isNeutralCarrier("99"));
  }
  void testSwapFromNeutralToActual()
  {
    initConfig();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LA"), MCPCarrierUtil::swapFromNeutralToActual("96"));
  }
  void testGetPreferedCarriers_ForNeutralPartition()
  {
    const CarrierCode hostCarrier = "96";
    std::set<CarrierCode> carriers;
    CPPUNIT_ASSERT(MCPCarrierUtil::getPreferedCarriers(hostCarrier, carriers));
  }

private:
  void initConfig(const char* cxrMap = "LA-0I|LP-L1|4M-X0",
                  const char* hostList = "LA|LT|96",
                  const char* prefCrx = "LA-LA/LP/4M/XL|96-LA/LP/4M/XL",
                  const char* realToNeutralPart = "LA-96|LA-98")
  {
    Global::config().setValue("MCP_PRIME_HOSTS", hostList, "XFORMS_MCP");
    Global::config().setValue("MCP_REAL_TO_PSEUDO_CXR_MAP", cxrMap, "XFORMS_MCP");
    Global::config().setValue("MCP_PREFERRED_CARRIERS", prefCrx, "XFORMS_MCP");
    Global::config().setValue("MCP_ACTUAL_TO_NEUTRAL_PARTITION", realToNeutralPart, "XFORMS_MCP");

    MCPCarrierUtil::_mcpCxrMap.clear();
    MCPCarrierUtil::_mcpHosts.clear();
    MCPCarrierUtil::_preferedCarriers.clear();
    MCPCarrierUtil::_iapRestricted.clear();
    MCPCarrierUtil::_journeyByMarriageCarriers.clear();
    MCPCarrierUtil::_mcpNeutralCxrMap.clear();

    MCPCarrierUtil::initialize(Global::config());
  }
  TestMemHandle _memHandle;
  CarrierCode _cxrPseudo;
  CarrierCode _cxrActual;
  PricingTrx* _trx;
  std::vector<CarrierCode> _temp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MCPCarrierUtilTest);
} // namespace tse
