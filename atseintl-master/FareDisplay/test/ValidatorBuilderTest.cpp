#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/ValidatorBuilder.h"
#include "DataModel/FareDisplayRequest.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "test/include/MockGlobal.h"
#include "FareDisplay/Validator.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayRequest.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestFactoryManager.h"
#include "FareDisplay/PaxTypeValidator.h"
#include "FareDisplay/RequestedPaxTypeValidator.h"
#include "FareDisplay/QualifierPaxTypeValidator.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/FareDispInclFareType.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  FareDispRec1PsgType* getR1PsgType(const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    const Indicator& pseudoCityType,
                                    const PseudoCityCode& pseudoCity,
                                    const InclusionCode& inclusionCode,
                                    const PaxTypeCode& psgType)
  {
    FareDispRec1PsgType* ret = _memHandle.create<FareDispRec1PsgType>();
    ret->userApplType() = userApplType;
    ret->userAppl() = userAppl;
    ret->pseudoCityType() = pseudoCityType;
    ret->pseudoCity() = pseudoCity;
    ret->inclusionCode() = inclusionCode;
    ret->psgType() = psgType;
    return ret;
  }
  FareDispInclFareType* getFDFareType(const Indicator& userApplType,
                                      const UserApplCode& userAppl,
                                      const Indicator& pseudoCityType,
                                      const PseudoCityCode& pseudoCity,
                                      const InclusionCode& inclusionCode,
                                      const FareType& fareType)
  {
    FareDispInclFareType* ret = _memHandle.create<FareDispInclFareType>();
    ret->userApplType() = userApplType;
    ret->userAppl() = userAppl;
    ret->pseudoCityType() = pseudoCityType;
    ret->pseudoCity() = pseudoCity;
    ret->inclusionCode() = inclusionCode;
    ret->fareType() = fareType;
    return ret;
  }

public:
  const std::vector<FareDispRec1PsgType*>&
  getFareDispRec1PsgType(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const Indicator& pseudoCityType,
                         const PseudoCityCode& pseudoCity,
                         const InclusionCode& inclusionCode)
  {
    if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
        inclusionCode == "NLX")
    {
      std::vector<FareDispRec1PsgType*>* ret =
          _memHandle.create<std::vector<FareDispRec1PsgType*> >();
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "ADT"));
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "NEG"));
      return *ret;
    }
    else if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
             inclusionCode == "GRP")
    {
      std::vector<FareDispRec1PsgType*>* ret =
          _memHandle.create<std::vector<FareDispRec1PsgType*> >();
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "GIT"));
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "GIY"));
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "GRP"));
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "GSP"));
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "GYT"));
      ret->push_back(
          getR1PsgType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "VAG"));
      return *ret;
    }
    return DataHandleMock::getFareDispRec1PsgType(
        userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  }
  const std::vector<FareDispInclDsplType*>&
  getFareDispInclDsplType(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const InclusionCode& inclusionCode)
  {
    if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
        inclusionCode == "NLX")
      return *_memHandle.create<std::vector<FareDispInclDsplType*> >();
    else if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
             inclusionCode == "GRP")
      return *_memHandle.create<std::vector<FareDispInclDsplType*> >();
    return DataHandleMock::getFareDispInclDsplType(
        userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  }
  const std::vector<FareDispInclFareType*>&
  getFareDispInclFareType(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const InclusionCode& inclusionCode)
  {
    if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
        inclusionCode == "NLX")
      return *_memHandle.create<std::vector<FareDispInclFareType*> >();
    else if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
             inclusionCode == "GRP")
    {
      std::vector<FareDispInclFareType*>* ret =
          _memHandle.create<std::vector<FareDispInclFareType*> >();
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGA"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGC"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGI"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGL"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGM"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGN"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGO"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGP"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGV"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGX"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PGZ"));
      ret->push_back(
          getFDFareType(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, "PSA"));
      return *ret;
    }
    return DataHandleMock::getFareDispInclFareType(
        userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  }
  const std::vector<FareDispInclRuleTrf*>&
  getFareDispInclRuleTrf(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const Indicator& pseudoCityType,
                         const PseudoCityCode& pseudoCity,
                         const InclusionCode& inclusionCode)
  {
    if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
        inclusionCode == "NLX")
      return *_memHandle.create<std::vector<FareDispInclRuleTrf*> >();
    else if (userApplType == ' ' && userAppl == "" && pseudoCityType == ' ' && pseudoCity == "" &&
             inclusionCode == "GRP")
      return *_memHandle.create<std::vector<FareDispInclRuleTrf*> >();
    return DataHandleMock::getFareDispInclRuleTrf(
        userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  }
};
}
class ValidatorBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ValidatorBuilderTest);
  CPPUNIT_TEST(testBuild);
  CPPUNIT_TEST(testWeb);
  CPPUNIT_TEST(testBuildALL);
  CPPUNIT_TEST(testBuildGeneric);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _billing = _memHandle.create<Billing>();
    _agent = _memHandle.create<Agent>();
    _options = _memHandle.create<FareDisplayOptions>();
    _request = _memHandle.create<FareDisplayRequest>();
    _response = _memHandle.create<FareDisplayResponse>();
  }
  void tearDown() { _memHandle.clear(); }

  void testBuildALL()
  {
    // test ALL with no requested passenger type
    // expected response - no validators.

    std::vector<Validator*> validators;
    std::map<uint8_t, std::vector<Validator*> > inclusionCodeValidators;

    FareDisplayRequest request;
    FareDisplayResponse response;
    FareDisplayTrx trx;
    FareDisplayInclCd incl;

    trx.setRequest(&request);
    trx.fdResponse() = &response;
    request.inclusionCode() = "ALL";
    ValidatorBuilder builder(trx, validators, inclusionCodeValidators);
    builder.build();
    CPPUNIT_ASSERT(!validators.empty());
    CPPUNIT_ASSERT(validators.size() == 1);
    request.displayPassengerTypes().push_back("ABC");
    builder.build();
    CPPUNIT_ASSERT(!validators.empty());
  }

  void testBuild()
  {
    MyDataHandle mdh;
    std::vector<Validator*> validators;
    std::map<uint8_t, std::vector<Validator*> > inclusionCodeValidators;

    FareDisplayRequest request;
    FareDisplayResponse response;
    FareDisplayOptions options;
    FareDisplayTrx trx;
    FareDisplayInclCd incl;
    incl.userApplType() = ' ';
    incl.pseudoCityType() = ' ';
    response.fareDisplayInclCd() = &incl;
    trx.setRequest(&request);
    trx.fdResponse() = &response;
    trx.setOptions(&options);
    request.inclusionCode() = "NLX";
    incl.inclusionCode() = "NLX";
    ValidatorBuilder builder(trx, validators, inclusionCodeValidators);
    builder.build();
    CPPUNIT_ASSERT(!validators.empty());
    CPPUNIT_ASSERT(validators.size() == 1);

    validators.clear();
    inclusionCodeValidators.clear();
    CPPUNIT_ASSERT(validators.empty());
    request.inclusionCode() = "GRP";
    incl.inclusionCode() = "GRP";
    ValidatorBuilder builder2(trx, validators, inclusionCodeValidators);
    builder2.build();
    CPPUNIT_ASSERT(!validators.empty());
    CPPUNIT_ASSERT(validators.size() == 2);
    std::vector<Validator*>::iterator i(validators.begin());
    CPPUNIT_ASSERT(dynamic_cast<PaxTypeValidator*>((*i)) != 0);
  }

  void testBuildGeneric() {}

  void testWeb()
  {
    std::vector<Validator*> validators;
    std::map<uint8_t, std::vector<Validator*> > inclusionCodeValidators;

    FareDisplayRequest request;
    FareDisplayResponse response;
    FareDisplayOptions options;
    FareDisplayTrx trx;
    response.fareDisplayInclCd() = 0;
    trx.setRequest(&request);
    trx.fdResponse() = &response;
    trx.setOptions(&options);
    request.inclusionCode() = "WEB";
    ValidatorBuilder builder(trx, validators, inclusionCodeValidators);
    builder.build();
    CPPUNIT_ASSERT(!validators.empty());
    CPPUNIT_ASSERT(validators.size() == 2);
    std::vector<Validator*>::iterator j(validators.begin());
    CPPUNIT_ASSERT(dynamic_cast<PaxTypeValidator*>((*j)) != 0);

    validators.clear();
    inclusionCodeValidators.clear();
    CPPUNIT_ASSERT(validators.empty());
    request.displayPassengerTypes().push_back("ADT");
    ValidatorBuilder builder2(trx, validators, inclusionCodeValidators);
    builder2.build();
    CPPUNIT_ASSERT(!validators.empty());
    CPPUNIT_ASSERT(validators.size() == 2);
    std::vector<Validator*>::iterator i(validators.begin());
    CPPUNIT_ASSERT(dynamic_cast<RequestedPaxTypeValidator*>((*i)) != 0);
  }

private:
  FareDisplayTrx* _trx;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  FareDisplayResponse* _response;
  Agent* _agent;
  Billing* _billing;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ValidatorBuilderTest);
}
