#include "test/include/CppUnitHelperMacros.h"
#include "Rules/PaxTypeCodeValidator.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class PaxTypeCodeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeCodeValidatorTest);

  CPPUNIT_TEST(testValidate_Blank_Blank_Pass);
  CPPUNIT_TEST(testValidate_ADT_Blank_Pass);
  CPPUNIT_TEST(testValidate_INF_INF_Pass);
  CPPUNIT_TEST(testValidate_INF_YTH_Fail);
  CPPUNIT_TEST(testValidate_INF_MIL_Fail_When_Inexact_Match);
  CPPUNIT_TEST(testValidate_Pass_When_MIL_Matches_To_ADT);
  CPPUNIT_TEST(testValidate_Pass_When_CNN_Matches_To_ADT);
  CPPUNIT_TEST(testValidate_Pass_When_CNN_Matches_To_Blank);
  CPPUNIT_TEST(testValidate_Pass_When_INF_Matches_To_Blank);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  PaxTypeCodeValidator* _val;
  PaxType* _pt;

  static const std::string label;

  PaxTypeCodeValidatorTest()
  {
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
  }

  void setUp()
  {
    _val = _memH.insert(new PaxTypeCodeValidator(0, label));
    _val->_dc = _memH.insert(new DiagCollector);
    _val->_dc->activate();
    _pt = _memH.insert(new PaxType());
  }

  void tearDown() { _memH.clear(); }

  std::string getDiagString()
  {
    _val->_dc->flushMsg();
    return _val->_dc->str();
  }

  enum
  {
    itemNoR3 = 3254
  };

  void testValidate_Blank_Blank_Pass()
  {
    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, ""));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC:    REFUND PTC: \n"), getDiagString());
  }

  void testValidate_ADT_Blank_Pass()
  {
    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, "ADT"));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC: ADT   REFUND PTC: \n"), getDiagString());
  }

  void testValidate_INF_INF_Pass()
  {
    _pt->paxType() = "INF";
    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, "INF"));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC: INF   REFUND PTC: INF\n"), getDiagString());
  }

  void testValidate_INF_YTH_Fail()
  {
    _pt->paxType() = "INF";
    populateActualPaxType();

    CPPUNIT_ASSERT(!_val->validate(itemNoR3, *_pt, "YTH"));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC: YTH   REFUND PTC: INF\n"
                                     "  FAILED ITEM 3254 - PTC NOT MATCHED\n"),
                         getDiagString());
  }

  void testValidate_INF_MIL_Fail_When_Inexact_Match()
  {
    _pt->paxType() = "INF";
    populateActualPaxType();

    CPPUNIT_ASSERT(!_val->validate(itemNoR3, *_pt, "MIL"));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC: MIL   REFUND PTC: INF\n"
                                     "  FAILED ITEM 3254 - PTC NOT MATCHED\n"),
                         getDiagString());
  }

  void testValidate_Pass_When_MIL_Matches_To_ADT()
  {
    _pt->paxType() = "MIL";
    populateActualPaxType();

    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, "ADT"));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC: ADT   REFUND PTC: MIL\n"), getDiagString());
  }

  void testValidate_Pass_When_CNN_Matches_To_ADT()
  {
    _pt->paxType() = "CNN";
    populateActualPaxType();

    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, "ADT"));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC: ADT   REFUND PTC: CNN\n"), getDiagString());
  }

  void testValidate_Pass_When_CNN_Matches_To_Blank()
  {
    _pt->paxType() = "CNN";
    populateActualPaxType();

    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, ""));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC:    REFUND PTC: CNN\n"), getDiagString());
  }

  void testValidate_Pass_When_INF_Matches_To_Blank()
  {
    _pt->paxType() = "INF";
    populateActualPaxType();

    CPPUNIT_ASSERT(_val->validate(itemNoR3, *_pt, ""));
    CPPUNIT_ASSERT_EQUAL(std::string("PTC:    REFUND PTC: INF\n"), getDiagString());
  }

  void populateActualPaxType()
  {
    PaxType* pt = _memH.insert(new PaxType());
    pt->paxType() = "MIL";
    std::vector<PaxType*>* vec = _memH.insert(new std::vector<PaxType*>());
    vec->push_back(pt);
    _pt->actualPaxType()[CarrierCode("AA")] = vec;
  }
};

const std::string
PaxTypeCodeValidatorTest::label("REFUND");

CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeCodeValidatorTest);
}
