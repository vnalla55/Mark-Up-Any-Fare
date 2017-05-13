#include "test/include/CppUnitHelperMacros.h"

#include <vector>
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"

#include "ItinAnalyzer/InclusionCodePaxType.h"
#include "ItinAnalyzer/SpecialInclusionCodePaxType.h"
#include "ItinAnalyzer/GenericInclusionCodePaxType.h"
#include "ItinAnalyzer/ALLInclusionCodePaxType.h"
#include "ItinAnalyzer/NullInclusionCodePaxType.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DBAccess/FareDisplayInclCd.h"

namespace tse
{
class InclusionCodePaxTypeTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

  public:
    const std::vector<FareDisplayInclCd*>& getFareDisplayInclCd(const Indicator& userApplType,
                                                                const UserApplCode& userAppl,
                                                                const Indicator& pseudoCityType,
                                                                const PseudoCityCode& pseudoCity,
                                                                const InclusionCode& inclusionCode)
    {
      if (inclusionCode == "NLX" || inclusionCode == "CB")
      {
        std::vector<FareDisplayInclCd*>* ret =
            _memHandle.create<std::vector<FareDisplayInclCd*> >();
        ret->push_back(_memHandle.create<FareDisplayInclCd>());
        return *ret;
      }
      return DataHandleMock::getFareDisplayInclCd(
          userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
    }
  };

  CPPUNIT_TEST_SUITE(InclusionCodePaxTypeTest);
  CPPUNIT_TEST(testSpecial_AD);
  CPPUNIT_TEST(testSpecial_WEB);

  CPPUNIT_TEST(testGeneric_NLX);
  CPPUNIT_TEST(testGeneric_CB);
  CPPUNIT_TEST(testAll);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx.setRequest(&_request);
    _trx.setOptions(&_options);
    _billing.partitionID() = "1S";
    _trx.billing() = &_billing;
    _request.ticketingAgent() = &_agent;
    _agent.tvlAgencyPCC() = "UV39";
    _agent.mainTvlAgencyPCC() = "UV39";
  }
  void tearDown()
  {
    _memHandle.clear();
  }

  void testSpecial_AD()
  {
    _request.inclusionCode() = "AD";

    InclusionCodePaxType* incl = InclusionCodePaxType::getInclusionCodePaxType(_trx);
    CPPUNIT_ASSERT(incl != 0);
    CPPUNIT_ASSERT(dynamic_cast<SpecialInclusionCodePaxType*>(incl) != 0);
  }
  void testSpecial_WEB()
  {
    _request.inclusionCode() = "WEB";

    InclusionCodePaxType* incl = InclusionCodePaxType::getInclusionCodePaxType(_trx);
    CPPUNIT_ASSERT(incl != 0);
    CPPUNIT_ASSERT(dynamic_cast<SpecialInclusionCodePaxType*>(incl) != 0);
    CPPUNIT_ASSERT(dynamic_cast<ALLInclusionCodePaxType*>(incl) == 0);
  }
  void testGeneric_NLX()
  {
    MyDataHandle mdh;
    _request.inclusionCode() = "NLX";

    InclusionCodePaxType* incl = InclusionCodePaxType::getInclusionCodePaxType(_trx);
    CPPUNIT_ASSERT(incl != 0);
    CPPUNIT_ASSERT(dynamic_cast<GenericInclusionCodePaxType*>(incl) != 0);
  }
  void testGeneric_CB()
  {
    MyDataHandle msh;
    _request.inclusionCode() = "CB";

    InclusionCodePaxType* incl = InclusionCodePaxType::getInclusionCodePaxType(_trx);
    CPPUNIT_ASSERT(incl != 0);
    CPPUNIT_ASSERT(dynamic_cast<GenericInclusionCodePaxType*>(incl) != 0);
    CPPUNIT_ASSERT(dynamic_cast<NullInclusionCodePaxType*>(incl) == 0);
  }

  void testAll()
  {
    _request.inclusionCode() = "ALL";

    InclusionCodePaxType* incl = InclusionCodePaxType::getInclusionCodePaxType(_trx);
    CPPUNIT_ASSERT(incl != 0);
    CPPUNIT_ASSERT(dynamic_cast<ALLInclusionCodePaxType*>(incl) != 0);
  }

private:
  TestMemHandle _memHandle;
  FareDisplayTrx _trx;
  FareDisplayRequest _request;
  FareDisplayOptions _options;
  Billing _billing;
  Agent _agent;
};

CPPUNIT_TEST_SUITE_REGISTRATION(InclusionCodePaxTypeTest);
}
