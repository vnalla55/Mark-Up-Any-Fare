#include "test/include/CppUnitHelperMacros.h"

#include "Common/CabinType.h"

using namespace std;

namespace tse
{
class CabinTypeTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(CabinTypeTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testMinFaresLowerCabinFactory);
  CPPUNIT_TEST(testMinFaresHigherCabinFactory);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testOperatorEqual);
  CPPUNIT_TEST(testOperatorEqualEqual);
  CPPUNIT_TEST(testOperatorNotEqual);
  CPPUNIT_TEST(testOperatorGreaterThan);
  CPPUNIT_TEST(testOperatorLessThan);
  CPPUNIT_TEST(testOperatorGreaterThanEqualTo);
  CPPUNIT_TEST(testOperatorLessThanEqualTo);
  CPPUNIT_TEST(testIsValidCabin);
  CPPUNIT_TEST(testStreamingOperator);
  CPPUNIT_TEST(testSetClass);
  CPPUNIT_TEST(testIndex);
  CPPUNIT_TEST(testGeneralIndex);
  CPPUNIT_TEST(testSetGetClassFromAlphaNumPremiumFirst);
  CPPUNIT_TEST(testSetGetClassFromAlphaNumFirst);
  CPPUNIT_TEST(testSetGetClassFromAlphaNumPremiumBusiness);
  CPPUNIT_TEST(testSetGetClassFromAlphaNumBusiness);
  CPPUNIT_TEST(testSetGetClassFromAlphaNumPremiumEconomy);
  CPPUNIT_TEST(testSetGetClassFromAlphaNumEconomy);
  CPPUNIT_TEST(testAddOneLevelToCabinType);
  CPPUNIT_TEST(testCreateEmptyCabinBoolMapPremiumFirst);
  CPPUNIT_TEST(testCreateEmptyCabinBoolMapFirst);
  CPPUNIT_TEST(testCreateEmptyCabinBoolMapPremiumBusiness);
  CPPUNIT_TEST(testCreateEmptyCabinBoolMapBusiness);
  CPPUNIT_TEST(testCreateEmptyCabinBoolMapPremiumEconomy);
  CPPUNIT_TEST(testCreateEmptyCabinBoolMapEconomy);
  CPPUNIT_TEST(testGetClassAlphaNum);

  CPPUNIT_TEST_SUITE_END();

public:
  CabinType* cabin;
  void setUp() { cabin = new CabinType(); }

  void tearDown() { delete cabin; }

  void testConstructor() { CPPUNIT_ASSERT(cabin->isUndefinedClass()); }

  void testMinFaresLowerCabinFactory()
  {
    cabin->setPremiumFirstClass();

    CabinType cabin2 = cabin->minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isFirstClass());

    cabin2 = cabin2.minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isPremiumBusinessClass());

    cabin2 = cabin2.minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isBusinessClass());

    cabin2 = cabin2.minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isPremiumEconomyClass());

    cabin2 = cabin2.minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isEconomyClass());

    cabin2 = cabin2.minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());

    cabin->setUndefinedClass();
    cabin2 = cabin->minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());

    cabin->setUnknownClass();
    cabin2 = cabin->minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());

    cabin->setInvalidClass();
    cabin2 = cabin->minFaresLowerCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());
  }

  void testMinFaresHigherCabinFactory()
  {
    cabin->setPremiumFirstClass();
    CabinType cabin2 = cabin->minFaresHigherCabin();
    // This is the only way to test for SuperSonic
    CabinType sonic;
    CabinType::dummyData(sonic);
    CPPUNIT_ASSERT_EQUAL(sonic, cabin2);

    cabin->setEconomyClass();
    cabin2 = cabin->minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isPremiumEconomyClass());

    cabin2 = cabin2.minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isBusinessClass());

    cabin2 = cabin2.minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isPremiumBusinessClass());

    cabin2 = cabin2.minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isFirstClass());

    cabin2 = cabin2.minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isPremiumFirstClass());

    cabin2 = cabin2.minFaresHigherCabin();
    CPPUNIT_ASSERT_EQUAL(sonic, cabin2);

    cabin->setUndefinedClass();
    cabin2 = cabin->minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());

    cabin->setUnknownClass();
    cabin2 = cabin->minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());

    cabin->setInvalidClass();
    cabin2 = cabin->minFaresHigherCabin();
    CPPUNIT_ASSERT(cabin2.isUnknownClass());
  }

  void testCopyConstructor()
  {
    cabin->setBusinessClass();
    CabinType cabinCopy(*cabin);
    CPPUNIT_ASSERT_EQUAL(*cabin, cabinCopy);
  }

  void testOperatorEqual()
  {
    cabin->setBusinessClass();
    CabinType cabinCopy = *cabin;
    CPPUNIT_ASSERT_EQUAL(*cabin, cabinCopy);
  }

  void testOperatorEqualEqual()
  {
    cabin->setBusinessClass();
    CabinType cabinCopy = *cabin;
    CPPUNIT_ASSERT(*cabin == cabinCopy);

    cabinCopy.setEconomyClass();
    CPPUNIT_ASSERT(!(*cabin == cabinCopy));
  }

  void testOperatorNotEqual()
  {
    cabin->setBusinessClass();
    CabinType cabinCopy = *cabin;
    CPPUNIT_ASSERT(!(*cabin != cabinCopy));

    cabinCopy.setEconomyClass();
    CPPUNIT_ASSERT((*cabin != cabinCopy));
  }

  void testOperatorGreaterThan()
  {
    cabin->setBusinessClass();

    CabinType cabin2;
    CPPUNIT_ASSERT(*cabin > cabin2);

    cabin2.setFirstClass();
    CPPUNIT_ASSERT(!(cabin2 > *cabin));
  }

  void testOperatorLessThan()
  {
    cabin->setBusinessClass();
    CabinType cabin2;
    CPPUNIT_ASSERT(!(*cabin < cabin2));

    cabin2.setFirstClass();
    CPPUNIT_ASSERT(cabin2 < *cabin);
  }

  void testOperatorGreaterThanEqualTo()
  {
    cabin->setBusinessClass();
    CabinType cabin2;
    CPPUNIT_ASSERT(*cabin >= cabin2);

    cabin2.setBusinessClass();
    CPPUNIT_ASSERT(cabin2 >= *cabin);

    cabin2.setFirstClass();
    CPPUNIT_ASSERT(!(cabin2 >= *cabin));
  }

  void testOperatorLessThanEqualTo()
  {
    cabin->setBusinessClass();
    CabinType cabin2;
    CPPUNIT_ASSERT(!(*cabin <= cabin2));

    cabin2.setBusinessClass();
    CPPUNIT_ASSERT(cabin2 <= *cabin);

    cabin2.setFirstClass();
    CPPUNIT_ASSERT(cabin2 <= *cabin);
  }

  void testIsValidCabin()
  {
    cabin->setEconomyClass();
    CPPUNIT_ASSERT(cabin->isValidCabin());

    cabin->setFirstClass();
    CPPUNIT_ASSERT(cabin->isValidCabin());

    cabin->setBusinessClass();
    CPPUNIT_ASSERT(cabin->isValidCabin());

    cabin->setUnknownClass();
    CPPUNIT_ASSERT(!cabin->isValidCabin());

    cabin->setUndefinedClass();
    CPPUNIT_ASSERT(!cabin->isValidCabin());

    cabin->setInvalidClass();
    CPPUNIT_ASSERT(!cabin->isValidCabin());
  }

  void testStreamingOperator()
  {
    cabin->setBusinessClass();
    assertOstringstream(CabinType::BUSINESS_CLASS, *cabin);
  }

  // could be moved elsewhere
  template <class T>
  void assertOstringstream(const char& expected, T& streamable)
  {
    ostringstream stringStream;
    stringStream << streamable;
    CPPUNIT_ASSERT_EQUAL(string(1, expected), stringStream.str());
  }

  void testSetClass()
  {
    cabin->setClass('8'); // value for economy
    CPPUNIT_ASSERT(cabin->isEconomyClass());
    CPPUNIT_ASSERT_EQUAL('8', cabin->getCabinIndicator()); // converted to new value
    cabin->setClass('7'); // value for premium economy
    CPPUNIT_ASSERT(cabin->isPremiumEconomyClass());
    CPPUNIT_ASSERT_EQUAL('7', cabin->getCabinIndicator()); // converted to new value
  }
  void testIndex()
  {
    CPPUNIT_ASSERT_EQUAL(6, (int)cabin->size());
    cabin->setEconomyClass();
    CPPUNIT_ASSERT_EQUAL(0, (int)cabin->index());
    cabin->setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL(1, (int)cabin->index());
    cabin->setBusinessClass();
    CPPUNIT_ASSERT_EQUAL(2, (int)cabin->index());
    cabin->setPremiumBusinessClass();
    CPPUNIT_ASSERT_EQUAL(3, (int)cabin->index());
    cabin->setFirstClass();
    CPPUNIT_ASSERT_EQUAL(4, (int)cabin->index());
    cabin->setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL(5, (int)cabin->index());
  }

  void testGeneralIndex()
  {
    CPPUNIT_ASSERT_EQUAL(6, (int)cabin->size());
    cabin->setEconomyClass();
    CPPUNIT_ASSERT_EQUAL(0, (int)cabin->generalIndex());
    cabin->setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL(0, (int)cabin->generalIndex());
    cabin->setBusinessClass();
    CPPUNIT_ASSERT_EQUAL(2, (int)cabin->generalIndex());
    cabin->setPremiumBusinessClass();
    CPPUNIT_ASSERT_EQUAL(2, (int)cabin->generalIndex());
    cabin->setFirstClass();
    CPPUNIT_ASSERT_EQUAL(4, (int)cabin->generalIndex());
    cabin->setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL(4, (int)cabin->generalIndex());

    CPPUNIT_ASSERT_EQUAL(0, (int)cabin->generalIndex(CabinType::ECONOMY_CLASS));
    CPPUNIT_ASSERT_EQUAL(0, (int)cabin->generalIndex(CabinType::ECONOMY_CLASS_PREMIUM));
    CPPUNIT_ASSERT_EQUAL(2, (int)cabin->generalIndex(CabinType::BUSINESS_CLASS));
    CPPUNIT_ASSERT_EQUAL(2, (int)cabin->generalIndex(CabinType::BUSINESS_CLASS_PREMIUM));
    CPPUNIT_ASSERT_EQUAL(4, (int)cabin->generalIndex(CabinType::FIRST_CLASS));
    CPPUNIT_ASSERT_EQUAL(4, (int)cabin->generalIndex(CabinType::FIRST_CLASS_PREMIUM));
  }

  void testSetGetClassFromAlphaNumPremiumFirst()
  {
    cabin->setClassFromAlphaNum(CabinType::FIRST_CLASS_PREMIUM_ALPHA);
    CPPUNIT_ASSERT(cabin->isPremiumFirstClass());
    CPPUNIT_ASSERT(cabin->getClassAlphaNum() == CabinType::FIRST_CLASS_PREMIUM_ALPHA);
  }
  void testSetGetClassFromAlphaNumFirst()
  {
    cabin->setClassFromAlphaNum(CabinType::FIRST_CLASS_ALPHA);
    CPPUNIT_ASSERT(cabin->isFirstClass());
    CPPUNIT_ASSERT(cabin->getClassAlphaNum() == CabinType::FIRST_CLASS_ALPHA);
  }
  void testSetGetClassFromAlphaNumPremiumBusiness()
  {
    cabin->setClassFromAlphaNum(CabinType::BUSINESS_CLASS_PREMIUM_ALPHA);
    CPPUNIT_ASSERT(cabin->isPremiumBusinessClass());
    CPPUNIT_ASSERT(cabin->getClassAlphaNum() == CabinType::BUSINESS_CLASS_PREMIUM_ALPHA);
  }
  void testSetGetClassFromAlphaNumBusiness()
  {
    cabin->setClassFromAlphaNum(CabinType::BUSINESS_CLASS_ALPHA);
    CPPUNIT_ASSERT(cabin->isBusinessClass());
    CPPUNIT_ASSERT(cabin->getClassAlphaNum() == CabinType::BUSINESS_CLASS_ALPHA);
  }
  void testSetGetClassFromAlphaNumPremiumEconomy()
  {
    cabin->setClassFromAlphaNum(CabinType::ECONOMY_CLASS_PREMIUM_ALPHA);
    CPPUNIT_ASSERT(cabin->isPremiumEconomyClass());
    CPPUNIT_ASSERT(cabin->getClassAlphaNum() == CabinType::ECONOMY_CLASS_PREMIUM_ALPHA);
  }
  void testSetGetClassFromAlphaNumEconomy()
  {
    cabin->setClassFromAlphaNum(CabinType::ECONOMY_CLASS_ALPHA);
    CPPUNIT_ASSERT(cabin->isEconomyClass());
    CPPUNIT_ASSERT(cabin->getClassAlphaNum() == CabinType::ECONOMY_CLASS_ALPHA);
  }

  void testAddOneLevelToCabinType()
  {
    cabin->setPremiumFirstClass();
    CabinType fCabin = CabinType::addOneLevelToCabinType(*cabin);

    CPPUNIT_ASSERT(fCabin.isFirstClass());

    CabinType pbCabin = CabinType::addOneLevelToCabinType(fCabin);
    CPPUNIT_ASSERT(pbCabin.isPremiumBusinessClass());

    CabinType bCabin = CabinType::addOneLevelToCabinType(pbCabin);
    CPPUNIT_ASSERT(bCabin.isBusinessClass());

    CabinType peCabin = CabinType::addOneLevelToCabinType(bCabin);
    CPPUNIT_ASSERT(peCabin.isPremiumEconomyClass());

    CabinType eCabin = CabinType::addOneLevelToCabinType(peCabin);
    CPPUNIT_ASSERT(eCabin.isEconomyClass());

    CabinType iCabin = CabinType::addOneLevelToCabinType(eCabin);
    CPPUNIT_ASSERT(iCabin.isInvalidClass());
  }

  void testCreateEmptyCabinBoolMapPremiumFirst()
  {
    std::map<CabinType, bool> cabinMap = cabin->createEmptyCabinBoolMap();

    CabinType c;
    c.setPremiumFirstClass();

    CPPUNIT_ASSERT(cabinMap.size() == 6);
    std::map<CabinType, bool>::iterator i = cabinMap.find(c);
    CPPUNIT_ASSERT(i != cabinMap.end());
    CPPUNIT_ASSERT(i->second == false);
  }
  void testCreateEmptyCabinBoolMapFirst()
  {
    std::map<CabinType, bool> cabinMap = cabin->createEmptyCabinBoolMap();

    CabinType c;
    c.setFirstClass();

    CPPUNIT_ASSERT(cabinMap.size() == 6);
    std::map<CabinType, bool>::iterator i = cabinMap.find(c);
    CPPUNIT_ASSERT(i != cabinMap.end());
    CPPUNIT_ASSERT(i->second == false);
  }
  void testCreateEmptyCabinBoolMapPremiumBusiness()
  {
    std::map<CabinType, bool> cabinMap = cabin->createEmptyCabinBoolMap();

    CabinType c;
    c.setPremiumBusinessClass();

    CPPUNIT_ASSERT(cabinMap.size() == 6);
    std::map<CabinType, bool>::iterator i = cabinMap.find(c);
    CPPUNIT_ASSERT(i != cabinMap.end());
    CPPUNIT_ASSERT(i->second == false);
  }
  void testCreateEmptyCabinBoolMapBusiness()
  {
    std::map<CabinType, bool> cabinMap = cabin->createEmptyCabinBoolMap();

    CabinType c;
    c.setBusinessClass();

    CPPUNIT_ASSERT(cabinMap.size() == 6);
    std::map<CabinType, bool>::iterator i = cabinMap.find(c);
    CPPUNIT_ASSERT(i != cabinMap.end());
    CPPUNIT_ASSERT(i->second == false);
  }
  void testCreateEmptyCabinBoolMapPremiumEconomy()
  {
    std::map<CabinType, bool> cabinMap = cabin->createEmptyCabinBoolMap();

    CabinType c;
    c.setPremiumEconomyClass();

    CPPUNIT_ASSERT(cabinMap.size() == 6);
    std::map<CabinType, bool>::iterator i = cabinMap.find(c);
    CPPUNIT_ASSERT(i != cabinMap.end());
    CPPUNIT_ASSERT(i->second == false);
  }
  void testCreateEmptyCabinBoolMapEconomy()
  {
    std::map<CabinType, bool> cabinMap = cabin->createEmptyCabinBoolMap();

    CabinType c;
    c.setEconomyClass();

    CPPUNIT_ASSERT(cabinMap.size() == 6);
    std::map<CabinType, bool>::iterator i = cabinMap.find(c);
    CPPUNIT_ASSERT(i != cabinMap.end());
    CPPUNIT_ASSERT(i->second == false);
  }

  void testGetClassAlphaNum()
  {
    cabin->setEconomyClass();
    CPPUNIT_ASSERT_EQUAL('Y', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL('S', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setBusinessClass();
    CPPUNIT_ASSERT_EQUAL('C', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setPremiumBusinessClass();
    CPPUNIT_ASSERT_EQUAL('J', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setFirstClass();
    CPPUNIT_ASSERT_EQUAL('F', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL('P', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setInvalidClass();
    CPPUNIT_ASSERT_EQUAL(' ', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setUndefinedClass();
    CPPUNIT_ASSERT_EQUAL(' ', CabinType::getClassAlphaNum(cabin->index()));
    cabin->setUnknownClass();
    CPPUNIT_ASSERT_EQUAL(' ', CabinType::getClassAlphaNum(cabin->index()));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CabinTypeTest);
} // namespace tse
