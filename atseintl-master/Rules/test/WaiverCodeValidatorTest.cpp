#include "Common/DateTime.h"
#include "DBAccess/Waiver.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/WaiverCodeValidator.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{

using namespace boost::assign;

class MockWaiverCodeValidator : public WaiverCodeValidator
{
public:
  MockWaiverCodeValidator()
    : WaiverCodeValidator(*_dataH, 0, log4cxx::Logger::getLogger("null"), _label), _dataH(0)
  {
  }

  virtual void getWaiver(const VendorCode& vendor, int itemNo, const DateTime& applDate) {}

  DataHandle* _dataH;

  static const std::string _label;
};

const std::string
MockWaiverCodeValidator::_label("EXCHANGE");

class WaiverCodeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(WaiverCodeValidatorTest);

  CPPUNIT_TEST(testMatch_TableNotEmpty_Pass);
  CPPUNIT_TEST(testMatch_TableNotEmpty_Fail);
  CPPUNIT_TEST(testMatch_TableEmpty_Fail);
  CPPUNIT_TEST(testMatch_ItemBlank_Pass);
  CPPUNIT_TEST(testMatch_CodeEmpty_Fail);
  CPPUNIT_TEST(testValidate_TableNotEmpty_Pass);
  CPPUNIT_TEST(testValidate_TableNotEmpty_Fail);
  CPPUNIT_TEST(testValidate_TableEmpty_Fail);
  CPPUNIT_TEST(testValidate_CodeEmpty_Fail);
  CPPUNIT_TEST(testValidate_ItemBlank_Pass);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  WaiverCodeValidator* _val;
  DateTime _date;
  const std::string empty, illness, actOfGod;

  WaiverCodeValidatorTest() : illness("2"), actOfGod("3")
  {
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
  }

  void setUp() { _val = _memH.insert(new MockWaiverCodeValidator); }

  void tearDown() { _memH.clear(); }

  Waiver* createWaiver(Waiver::WaiverCode code)
  {
    Waiver* w = _memH.insert(new Waiver);
    w->waiver() = code;
    return w;
  }

  void populateWaiver()
  {
    _val->_waivers += createWaiver(Waiver::DEATH), createWaiver(Waiver::ACTS_OF_GOD),
        createWaiver(Waiver::MILITARY);
  }

  void attachDiag()
  {
    _val->_dc = _memH.insert(new DiagCollector);
    _val->_dc->activate();
  }

  std::string getDiagString()
  {
    _val->_dc->flushMsg();
    return _val->_dc->str();
  }

  enum
  {
    itemBlank = 0,
    itemNo = 69,
    R3itemNo = 3245
  };

  void testMatch_TableNotEmpty_Pass()
  {
    populateWaiver();
    CPPUNIT_ASSERT(_val->match("ATP", itemNo, _date, actOfGod));
  }

  void testValidate_TableNotEmpty_Pass()
  {
    attachDiag();
    populateWaiver();
    CPPUNIT_ASSERT(_val->validate(R3itemNo, "ATP", itemNo, _date, actOfGod));
    CPPUNIT_ASSERT_EQUAL(std::string("WAIVER CODES: 1 3 5  EXCHANGE WAIVER CODE: 3\n"),
                         getDiagString());
  }

  void testMatch_TableNotEmpty_Fail()
  {
    populateWaiver();
    CPPUNIT_ASSERT(!_val->match("ATP", itemNo, _date, illness));
  }

  void testValidate_TableNotEmpty_Fail()
  {
    attachDiag();
    populateWaiver();
    CPPUNIT_ASSERT(!_val->validate(R3itemNo, "ATP", itemNo, _date, illness));
    CPPUNIT_ASSERT_EQUAL(std::string("WAIVER CODES: 1 3 5  EXCHANGE WAIVER CODE: 2\n"
                                     "  FAILED ITEM 3245 - WAIVER CODE NOT MATCHED\n"),
                         getDiagString());
  }

  void testMatch_TableEmpty_Fail() { CPPUNIT_ASSERT(!_val->match("ATP", itemNo, _date, illness)); }

  void testValidate_TableEmpty_Fail()
  {
    attachDiag();
    CPPUNIT_ASSERT(!_val->validate(R3itemNo, "ATP", itemNo, _date, illness));
    CPPUNIT_ASSERT_EQUAL(std::string("WAIVER CODES:  EXCHANGE WAIVER CODE: 2\n"
                                     "  FAILED ITEM 3245 - WAIVER CODE NOT MATCHED\n"),
                         getDiagString());
  }

  void testMatch_ItemBlank_Pass()
  {
    CPPUNIT_ASSERT(_val->match("ATP", itemBlank, _date, actOfGod));
  }

  void testValidate_ItemBlank_Pass()
  {
    attachDiag();
    CPPUNIT_ASSERT(_val->validate(R3itemNo, "ATP", itemBlank, _date, actOfGod));
    CPPUNIT_ASSERT_EQUAL(std::string("WAIVER CODES:  EXCHANGE WAIVER CODE: 3\n"), getDiagString());
  }

  void testMatch_CodeEmpty_Fail()
  {
    populateWaiver();
    CPPUNIT_ASSERT(!_val->match("ATP", itemNo, _date, empty));
  }

  void testValidate_CodeEmpty_Fail()
  {
    attachDiag();
    populateWaiver();
    CPPUNIT_ASSERT(!_val->validate(R3itemNo, "ATP", itemNo, _date, empty));
    CPPUNIT_ASSERT_EQUAL(std::string("WAIVER CODES: 1 3 5  EXCHANGE WAIVER CODE: \n"
                                     "  FAILED ITEM 3245 - WAIVER CODE NOT MATCHED\n"),
                         getDiagString());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WaiverCodeValidatorTest);
}
