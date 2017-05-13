
#include <iostream>
#include <time.h>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/ShoppingTrx.h"
#include "Common/PaxTypeUtil.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Common/AltPricingUtil.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexPricingTrx.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& paxTypeCode)
  {
    if (paxTypeCode == "JNN" || paxTypeCode == "INF" || paxTypeCode == "CNN" ||
        paxTypeCode == "ADT")
      return *_memHandle.create<std::vector<const PaxTypeMatrix*> >();
    return DataHandleMock::getPaxTypeMatrix(paxTypeCode);
  }
};
}
class PaxTypeUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeUtilTest);
  CPPUNIT_TEST(testIsOnlyOnePassenger);
  CPPUNIT_TEST(testIsAnActualPaxInTrx);
  CPPUNIT_TEST(testShouldThrowWhenPaxTypeCodeInvalidForNotAncPrcV3);
  CPPUNIT_TEST(testShouldNotInitializeWhenPaxTypeCodeInvalidForAncPrcV3);
  CPPUNIT_TEST(testIsAnActualPaxInTrx2);
  CPPUNIT_TEST(testIsAdultWithTrx);
  CPPUNIT_TEST(testIsAdult);
  CPPUNIT_TEST(testIsChildWithTrx);
  CPPUNIT_TEST(testIsChild);
  CPPUNIT_TEST(testIsInfantWithTrx);
  CPPUNIT_TEST(testIsInfant);
  CPPUNIT_TEST(testNumSeatsForFareOneWithOnePax);
  CPPUNIT_TEST(testNumSeatsForFareZeroWithOnePaxFCCIgnoreAvail);
  CPPUNIT_TEST(testNumSeatsForExcludedPaxTypes);
  CPPUNIT_TEST(testRexTotalNumSeats);
  CPPUNIT_TEST(testIsCWTType);
  CPPUNIT_TEST(testIsPaxWithSpecifiedAge);
  CPPUNIT_TEST(testIsPaxWithSpecifiedAgeFailIncorrectFirstChar);
  CPPUNIT_TEST(testIsPaxWithSpecifiedAgeFailIncorrectAge);
  CPPUNIT_TEST(testGetPaxWithUnspecifiedAge);
  CPPUNIT_TEST(testExtractAgeFromPaxType_Positive);
  CPPUNIT_TEST(testExtractAgeFromPaxType_Negative);
  CPPUNIT_TEST(testHasNotAwardPaxType);
  CPPUNIT_TEST(testIsSpanishPaxType);

  CPPUNIT_TEST(testGetDifferenceInYears_negative);
  CPPUNIT_TEST(testGetDifferenceInYears_zero);
  CPPUNIT_TEST(testGetDifferenceInYears_positive);

  CPPUNIT_TEST(testParsePassengerWithAge_ContainsAge);
  CPPUNIT_TEST(testParsePassengerWithAge_NoAge);
  CPPUNIT_TEST(testParsePassengerWithAge_Age0);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _options = _memHandle.create<PricingOptions>();
    _agent = _memHandle.create<Agent>();
    _trx->setRequest(_request);
    _trx->setOptions(_options);
    _trx->getRequest()->ticketingAgent() = _agent;

    _agent->agentCity() = "DFW";
    _paxType = _memHandle.create<PaxType>();
    _paxType->number() = 1;

    _pInfo = _memHandle.create<PaxTypeInfo>();
    _pInfo->numberSeatsReq() = 1;
    _paxType->paxTypeInfo() = _pInfo;
    _trx->paxType().push_back(_paxType);
  }

  void tearDown() { _memHandle.clear(); }

  void testIsOnlyOnePassenger()
  {
    // No Pax type in trx

    _trx->paxType().clear();
    CPPUNIT_ASSERT(!PaxTypeUtil::isOnlyOnePassenger(*_trx));

    // One Pax type with one person

    PaxType* paxType1 = _trx->dataHandle().create<PaxType>();
    PaxTypeCode paxTypeCode1 = "TV1";
    StateCode stateCode = "";
    PaxTypeUtil::initialize(*_trx, *paxType1, paxTypeCode1, 1, 5, stateCode, 1);
    _trx->paxType().push_back(paxType1);

    CPPUNIT_ASSERT(PaxTypeUtil::isOnlyOnePassenger(*_trx));

    // One Pax type but more than one person

    _trx->paxType().pop_back();
    PaxTypeUtil::initialize(*_trx, *paxType1, paxTypeCode1, 5, 5, stateCode, 1);
    _trx->paxType().push_back(paxType1);
    CPPUNIT_ASSERT(!PaxTypeUtil::isOnlyOnePassenger(*_trx));

    // More than one passenger type

    PaxType* paxType2 = _trx->dataHandle().create<PaxType>();
    PaxTypeCode paxTypeCode = "INF";
    PaxTypeUtil::initialize(*_trx, *paxType2, paxTypeCode, 1, 0, stateCode, 1);
    _trx->paxType().push_back(paxType2);

    CPPUNIT_ASSERT(!PaxTypeUtil::isOnlyOnePassenger(*_trx));
  }
  void testIsAnActualPaxInTrx()
  {

    PaxType* paxType1 = _trx->dataHandle().create<PaxType>();
    PaxTypeCode paxTypeCode1 = "TV1";
    StateCode stateCode = "";
    PaxTypeUtil::initialize(*_trx, *paxType1, paxTypeCode1, 1, 0, stateCode, 1);
    _trx->paxType().push_back(paxType1);

    CarrierCode carrier = "AA";
    PaxTypeCode paxTypeCode = "ADT";

    CPPUNIT_ASSERT(PaxTypeUtil::isAnActualPaxInTrx(*_trx, carrier, paxTypeCode));
    paxTypeCode = "XXX";
    CPPUNIT_ASSERT(!PaxTypeUtil::isAnActualPaxInTrx(*_trx, carrier, paxTypeCode));
  }
  void testShouldThrowWhenPaxTypeCodeInvalidForNotAncPrcV3()
  {
    PaxType* paxType = _trx->dataHandle().create<PaxType>();
    PaxTypeCode paxTypeCode = "A";
    StateCode stateCode = "";

    CPPUNIT_ASSERT_THROW(PaxTypeUtil::initialize(*_trx, *paxType, paxTypeCode, 1, 0, stateCode, 1), ErrorResponseException);
  }
  void testShouldNotInitializeWhenPaxTypeCodeInvalidForAncPrcV3()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    PaxType* paxType = _trx->dataHandle().create<PaxType>();
    PaxTypeCode paxTypeCode = "A";
    StateCode stateCode = "";

    CPPUNIT_ASSERT(!PaxTypeUtil::initialize(*_trx, *paxType, paxTypeCode, 1, 0, stateCode, 1));
  }
  void testIsAnActualPaxInTrx2()
  {
    MyDataHandle mdh;
    PaxType* paxType1 = _trx->dataHandle().create<PaxType>();
    PaxTypeCode paxTypeCode1 = "CNN";
    StateCode stateCode = "";
    PaxTypeUtil::initialize(*_trx, *paxType1, paxTypeCode1, 1, 0, stateCode, 1);
    _trx->paxType().push_back(paxType1);

    CarrierCode carrier = "BA";
    PaxTypeCode paxTypeCode = "CNN";

    CPPUNIT_ASSERT(PaxTypeUtil::isAnActualPaxInTrx(*_trx, carrier, paxTypeCode));
    paxTypeCode = "XXX";
    CPPUNIT_ASSERT(!PaxTypeUtil::isAnActualPaxInTrx(*_trx, carrier, paxTypeCode));
    CPPUNIT_ASSERT(PaxTypeUtil::isAnActualPaxInTrx(*_trx, carrier, "CNN"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAnActualPaxInTrx(*_trx, carrier, "ADT"));
  }

  void testIsAdultWithTrx()
  {
    CPPUNIT_ASSERT(PaxTypeUtil::isAdult(*_trx, "TV1", "SABR"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdult(*_trx, "ADT", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult(*_trx, "JNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult(*_trx, "JNF", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult(*_trx, "CNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult(*_trx, "INF", "ATP"));
  }

  void testIsAdult()
  {
    CPPUNIT_ASSERT(PaxTypeUtil::isAdult("TV1", "SABR"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdult("ADT", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult("JNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult("JNF", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult("CNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdult("INF", "ATP"));
  }

  void testIsChildWithTrx()
  {
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild(*_trx, "TV1", "SABR"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild(*_trx, "ADT", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isChild(*_trx, "JNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild(*_trx, "JNF", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isChild(*_trx, "CNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild(*_trx, "INF", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isChild(*_trx, "C09", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild(*_trx, "C0X", "ATP"));
  }

  void testIsChild()
  {
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild("TV1", "SABR"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild("ADT", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isChild("JNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild("JNF", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isChild("CNN", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild("INF", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isChild("C09", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isChild("C0X", "ATP"));
  }

  void testIsInfantWithTrx()
  {
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant(*_trx, "TV1", "SABR"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant(*_trx, "ADT", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant(*_trx, "JNN", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isInfant(*_trx, "JNF", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant(*_trx, "CNN", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isInfant(*_trx, "INF", "ATP"));
  }

  void testIsInfant()
  {
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant("TV1", "SABR"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant("ADT", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant("JNN", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isInfant("JNF", "ATP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isInfant("CNN", "ATP"));
    CPPUNIT_ASSERT(PaxTypeUtil::isInfant("INF", "ATP"));
  }

  void testIsCWTType()
  {
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrAssociatedType(""));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrAssociatedType("ADT"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrAssociatedType("CNN"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrAssociatedType("INF"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrAssociatedType("INS"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrAssociatedType("UNN"));

    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("CNE"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("NEG"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("INE"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("CBC"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("PFA"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("CBI"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("JNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("JCB"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("JNF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("MIL"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("SRC"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("CTZ"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GCF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GCT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GDP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GEX"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GST"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GVT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("NAT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrAssociatedType("GV1"));

    CPPUNIT_ASSERT(PaxTypeUtil::isNegotiatedOrAssociatedType("NEG"));
    CPPUNIT_ASSERT(PaxTypeUtil::isNegotiatedOrAssociatedType("CNE"));
    CPPUNIT_ASSERT(PaxTypeUtil::isNegotiatedOrAssociatedType("INE"));

    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType(""));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("ADT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("CNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("INF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("INS"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("UNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("CBC"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("PFA"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("CBI"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("JNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("JCB"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("JNF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("MIL"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("SRC"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("CTZ"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GCF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GCT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GDP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GEX"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GST"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GVT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("NAT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isNegotiatedOrAssociatedType("GV1"));

    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType(""));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("ADT"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("CNN"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("INF"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("INS"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("UNN"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("NEG"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("CNE"));
    CPPUNIT_ASSERT(PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("INE"));

    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("CBC"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("PFA"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("CBI"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("JNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("JCB"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("JNF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("MIL"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("SRC"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("CTZ"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GCF"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GCT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GDP"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GEX"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GST"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GVT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("NAT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType("GV1"));
  }

  //-----------------------------------------------
  void testNumSeatsForFareOneWithOnePax()
  {
    PaxTypeFare ptf;
    setTrxAsMip();
    CPPUNIT_ASSERT(PaxTypeUtil::numSeatsForFare(*_trx, ptf) == 1);
  }

  //-----------------------------------------------
  void testNumSeatsForFareZeroWithOnePaxFCCIgnoreAvail()
  {
    PaxTypeFare ptf;
    setTrxAsPricing();
    _request->lowFareRequested() = 'N';
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
    _trx->fareCalcConfig()->noMatchAvail() = 'Y';
    CPPUNIT_ASSERT(PaxTypeUtil::numSeatsForFare(*_trx, ptf) == 0);
  }

  void testNumSeatsForExcludedPaxTypes()
  {
    ShoppingTrx* shpTrx = _memHandle.create<ShoppingTrx>();
    shpTrx->excludedPaxType().push_back(createPaxType(2, 1));
    shpTrx->excludedPaxType().push_back(createPaxType(3, 2));

    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(8u),
                         PaxTypeUtil::numSeatsForExcludedPaxTypes(*shpTrx));
  }

  void testRexTotalNumSeats()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    rexTrx.paxType().push_back(_paxType);
    _paxType->totalPaxNumber() = 3;
    uint16_t expectedNum = 3;

    CPPUNIT_ASSERT_EQUAL(PaxTypeUtil::totalNumSeats(rexTrx), expectedNum);
  }

  void testHasNotAwardPaxType()
  {
    std::vector<PaxType*> paxTypeVec;
    PaxType paxTypeFFY;
    paxTypeFFY.paxType() = "FFY";
    paxTypeVec.push_back(&paxTypeFFY);

    CPPUNIT_ASSERT_EQUAL(false, PaxTypeUtil::hasNotAwardPaxType(paxTypeVec));
    PaxType paxType2;
    paxType2.paxType() = "FFP";
    paxTypeVec.push_back(&paxType2);
    CPPUNIT_ASSERT_EQUAL(false, PaxTypeUtil::hasNotAwardPaxType(paxTypeVec));
    paxType2.paxType() = "TNF";
    CPPUNIT_ASSERT_EQUAL(false, PaxTypeUtil::hasNotAwardPaxType(paxTypeVec));
    paxType2.paxType() = "TNN";
    CPPUNIT_ASSERT_EQUAL(false, PaxTypeUtil::hasNotAwardPaxType(paxTypeVec));
    paxType2.paxType() = "ADT";
    CPPUNIT_ASSERT_EQUAL(true, PaxTypeUtil::hasNotAwardPaxType(paxTypeVec));
  }

  void testIsPaxWithSpecifiedAge()
  {
    CPPUNIT_ASSERT(PaxTypeUtil::isPaxWithSpecifiedAge("A01"));
    CPPUNIT_ASSERT(PaxTypeUtil::isPaxWithSpecifiedAge("C05"));
    CPPUNIT_ASSERT(PaxTypeUtil::isPaxWithSpecifiedAge("Z99"));
  }

  void testIsPaxWithSpecifiedAgeFailIncorrectFirstChar()
  {
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("001"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("505"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("999"));
  }

  void testIsPaxWithSpecifiedAgeFailIncorrectAge()
  {
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("A0N"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("CN5"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("ZNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("A0-"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("C-5"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("Z--"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("A0&"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("C&5"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isPaxWithSpecifiedAge("Z&&"));
  }

  void testGetPaxWithUnspecifiedAge()
  {
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ANN"), PaxTypeUtil::getPaxWithUnspecifiedAge('A'));
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("CNN"), PaxTypeUtil::getPaxWithUnspecifiedAge('C'));
  }

  void testExtractAgeFromPaxType_Positive()
  {
    PaxTypeCode ptc("C10");
    uint16_t age = 100;
    CPPUNIT_ASSERT(PaxTypeUtil::extractAgeFromPaxType(ptc, age));
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), age);
  }

  void testExtractAgeFromPaxType_Negative()
  {
    PaxTypeCode ptc("CNN");
    uint16_t age = 100;
    CPPUNIT_ASSERT(!PaxTypeUtil::extractAgeFromPaxType(ptc, age));
  }

  void testIsSpanishPaxType()
  {
    CPPUNIT_ASSERT(PaxTypeUtil::isSpanishPaxType("ADR"));
    CPPUNIT_ASSERT(PaxTypeUtil::isSpanishPaxType("CHR"));
    CPPUNIT_ASSERT(PaxTypeUtil::isSpanishPaxType("INR"));
    CPPUNIT_ASSERT(PaxTypeUtil::isSpanishPaxType("ISR"));
    CPPUNIT_ASSERT(PaxTypeUtil::isSpanishPaxType("UNR"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isSpanishPaxType("ADT"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isSpanishPaxType("CNN"));
    CPPUNIT_ASSERT(!PaxTypeUtil::isSpanishPaxType(""));
  }

  void testGetDifferenceInYears_negative()
  {
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2009, 2, 19)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(0),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2010, 2, 16)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(0),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2010, 1, 17)));
    }

  void testGetDifferenceInYears_zero()
  {
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2010, 2, 17)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(0),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2010, 2, 18)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(0),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2010, 3, 17)));
    }

  void testGetDifferenceInYears_positive()
  {
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2011, 2, 17)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(1),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2011, 2, 18)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(1),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2011, 3, 17)));

    CPPUNIT_ASSERT_EQUAL(uint16_t(5),
        PaxTypeUtil::getDifferenceInYears(DateTime(2010, 2, 17), DateTime(2015, 2, 19)));
  }

  void testParsePassengerWithAge_ContainsAge()
  {
    PaxType paxType;
    paxType.paxType() = "C08";

    PaxTypeUtil::parsePassengerWithAge(paxType);
    CPPUNIT_ASSERT(paxType.paxType() == "CNN");
    CPPUNIT_ASSERT(paxType.age() == 8u);
  }

  void testParsePassengerWithAge_NoAge()
  {
    PaxType paxType;
    paxType.paxType() = "CNN";

    PaxTypeUtil::parsePassengerWithAge(paxType);
    CPPUNIT_ASSERT(paxType.paxType() == "CNN");
    CPPUNIT_ASSERT(paxType.age() == 0u);
  }

  void testParsePassengerWithAge_Age0()
  {
    PaxType paxType;
    paxType.paxType() = "C00";

    CPPUNIT_ASSERT_THROW(PaxTypeUtil::parsePassengerWithAge(paxType), ErrorResponseException);
  }

  //-----------------------------------------------
  void setTrxAsMip()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    return;
  }

  //-----------------------------------------------
  void setTrxAsPricing()
  {
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    return;
  }

private:
  PaxType* createPaxType(uint16_t numPax, int numSeats)
  {
    PaxTypeInfo* pti = _memHandle.create<PaxTypeInfo>();
    pti->numberSeatsReq() = numSeats;
    PaxType* pt = _memHandle.create<PaxType>();
    pt->number() = numPax;
    pt->paxTypeInfo() = pti;
    return pt;
  }

  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  Agent* _agent;
  PaxType* _paxType;
  PaxTypeInfo* _pInfo;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeUtilTest);
}
