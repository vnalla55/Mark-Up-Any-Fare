#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Rules/DepartureValidator.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "Common/TseEnums.h"
#include "DataModel/Itin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingUnit.h"
#include <vector>

namespace tse
{

using namespace boost::assign;

class DepartureValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DepartureValidatorTest);

  CPPUNIT_TEST(testMatchBeforeAfterDeparture_BEFORE_unflown_Pass);
  CPPUNIT_TEST(testMatchBeforeAfterDeparture_BEFORE_flown_Fail);
  CPPUNIT_TEST(testMatchBeforeAfterDeparture_AFTER_unflown_Fail);
  CPPUNIT_TEST(testMatchBeforeAfterDeparture_AFTER_flown_Pass);
  CPPUNIT_TEST(testMatchBeforeAfterDeparture_NOAPPLY_flown_Pass);
  CPPUNIT_TEST(testMatchBeforeAfterDeparture_NOAPPLY_unflown_Pass);

  CPPUNIT_TEST(testMatch_NOAPPLY_Pass);
  CPPUNIT_TEST(testMatch_NoSeg_Fail);
  CPPUNIT_TEST(testMatch_TwoSegAirArunk_Pass);
  CPPUNIT_TEST(testMatch_TwoSegArunkAir_Pass);
  CPPUNIT_TEST(testMatch_OneAir_Pass);

  CPPUNIT_TEST(testValidateJR_Pass);
  CPPUNIT_TEST(testValidateJR_Fail);

  CPPUNIT_TEST(testValidateFC_Pass);
  CPPUNIT_TEST(testValidateFC_Fail);

  CPPUNIT_TEST(testValidatePU_Pass);
  CPPUNIT_TEST(testValidatePU_SoftPass);
  CPPUNIT_TEST(testValidatePU_Fail);

  CPPUNIT_TEST(testValidate_Pass);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  DepartureValidator* _val;
  bool _softPass;

  void setUp()
  {
    _val = _memH.insert(new DepartureValidator(0, 0, 0, 0));
    _softPass = false;
  }

  void tearDown() { _memH.clear(); }

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
    Flown = 0,
    Unflown = 1
  };

  TravelSeg* createTravelSeg(bool unflown, TravelSegType type = Air)
  {
    TravelSeg* seg = 0;
    seg = _memH.insert(type == Air ? static_cast<TravelSeg*>(new AirSeg)
                                   : static_cast<TravelSeg*>(new ArunkSeg));
    seg->segmentType() = type;
    seg->unflown() = unflown;
    return seg;
  }

  void testMatchBeforeAfterDeparture_BEFORE_unflown_Pass()
  {
    CPPUNIT_ASSERT(
        _val->matchBeforeAfterDeparture(*createTravelSeg(Unflown), DepartureValidator::BEFORE));
  }

  void testMatchBeforeAfterDeparture_BEFORE_flown_Fail()
  {
    CPPUNIT_ASSERT(
        !_val->matchBeforeAfterDeparture(*createTravelSeg(Flown), DepartureValidator::BEFORE));
  }

  void testMatchBeforeAfterDeparture_AFTER_unflown_Fail()
  {
    CPPUNIT_ASSERT(
        !_val->matchBeforeAfterDeparture(*createTravelSeg(Unflown), DepartureValidator::AFTER));
  }

  void testMatchBeforeAfterDeparture_AFTER_flown_Pass()
  {
    CPPUNIT_ASSERT(
        _val->matchBeforeAfterDeparture(*createTravelSeg(Flown), DepartureValidator::AFTER));
  }

  void testMatchBeforeAfterDeparture_NOAPPLY_flown_Pass()
  {
    CPPUNIT_ASSERT(
        _val->matchBeforeAfterDeparture(*createTravelSeg(Flown), DepartureValidator::NOT_APPLY));
  }

  void testMatchBeforeAfterDeparture_NOAPPLY_unflown_Pass()
  {
    CPPUNIT_ASSERT(
        _val->matchBeforeAfterDeparture(*createTravelSeg(Unflown), DepartureValidator::NOT_APPLY));
  }

  void testMatch_NOAPPLY_Pass()
  {
    std::vector<TravelSeg*> seg;
    CPPUNIT_ASSERT(_val->match(seg, DepartureValidator::NOT_APPLY));
  }

  void testMatch_NoSeg_Fail()
  {
    std::vector<TravelSeg*> seg;
    CPPUNIT_ASSERT(!_val->match(seg, DepartureValidator::BEFORE));
  }

  void testMatch_TwoSegAirArunk_Pass()
  {
    std::vector<TravelSeg*> seg;
    seg += createTravelSeg(Unflown, Air), createTravelSeg(Unflown, Arunk);
    CPPUNIT_ASSERT(_val->match(seg, DepartureValidator::BEFORE));
  }

  void testMatch_TwoSegArunkAir_Pass()
  {
    std::vector<TravelSeg*> seg;
    seg += createTravelSeg(Unflown, Arunk), createTravelSeg(Unflown, Air);
    CPPUNIT_ASSERT(_val->match(seg, DepartureValidator::BEFORE));
  }

  void testMatch_OneAir_Pass()
  {
    std::vector<TravelSeg*> seg;
    seg += createTravelSeg(Unflown, Air);
    CPPUNIT_ASSERT(_val->match(seg, DepartureValidator::BEFORE));
  }

  enum
  {
    R3itemNo = 4365
  };

  void setupJR()
  {
    attachDiag();
    _val->_itin = _memH.insert(new Itin);
    const_cast<std::vector<TravelSeg*>&>(_val->_itin->travelSeg()) += createTravelSeg(Unflown);
  }

  void testValidateJR_Pass()
  {
    setupJR();
    CPPUNIT_ASSERT(_val->validateJR(R3itemNo, DepartureValidator::BEFORE));
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF JR: B\n"), getDiagString());
  }

  void testValidateJR_Fail()
  {
    setupJR();
    CPPUNIT_ASSERT(!_val->validateJR(R3itemNo, DepartureValidator::AFTER));
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF JR: A\n"
                                     "  FAILED ITEM 4365 - DEPARTURE OF JR\n"),
                         getDiagString());
  }

  void setupFC()
  {
    attachDiag();
    _val->_fc = _memH.insert(new FareMarket);
    const_cast<std::vector<TravelSeg*>&>(_val->_fc->travelSeg()) += createTravelSeg(Unflown);
  }

  void testValidateFC_Pass()
  {
    setupFC();
    CPPUNIT_ASSERT(_val->validateFC(R3itemNo, DepartureValidator::BEFORE));
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF FC: B\n"), getDiagString());
  }

  void testValidateFC_Fail()
  {
    setupFC();
    CPPUNIT_ASSERT(!_val->validateFC(R3itemNo, DepartureValidator::AFTER));
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF FC: A\n"
                                     "  FAILED ITEM 4365 - DEPARTURE OF FC\n"),
                         getDiagString());
  }

  void setupPU()
  {
    attachDiag();
    _val->_pu = _memH.insert(new PricingUnit);
    const_cast<std::vector<TravelSeg*>&>(_val->_pu->travelSeg()) += createTravelSeg(Unflown);
  }

  void testValidatePU_Pass()
  {
    setupPU();
    CPPUNIT_ASSERT(_val->validatePU(R3itemNo, DepartureValidator::BEFORE, _softPass));
    CPPUNIT_ASSERT(!_softPass);
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF PU: B\n"), getDiagString());
  }

  void testValidatePU_SoftPass()
  {
    attachDiag();
    CPPUNIT_ASSERT(_val->validatePU(R3itemNo, DepartureValidator::BEFORE, _softPass));
    CPPUNIT_ASSERT(_softPass);
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF PU: B\n"
                                     "  NO PU INFO: SOFTPASS\n"),
                         getDiagString());
  }

  void testValidatePU_Fail()
  {
    setupPU();
    CPPUNIT_ASSERT(!_val->validatePU(R3itemNo, DepartureValidator::AFTER, _softPass));
    CPPUNIT_ASSERT(!_softPass);
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF PU: A\n"
                                     "  FAILED ITEM 4365 - DEPARTURE OF PU\n"),
                         getDiagString());
  }

  void testValidate_Pass()
  {
    attachDiag();
    setupFC();
    CPPUNIT_ASSERT(_val->validate(R3itemNo,
                                  DepartureValidator::NOT_APPLY,
                                  DepartureValidator::NOT_APPLY,
                                  DepartureValidator::NOT_APPLY,
                                  _softPass));
    CPPUNIT_ASSERT(_softPass);
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE OF PU:  \n"
                                     "  NO PU INFO: SOFTPASS\n"
                                     "DEPARTURE OF JR:  \n"
                                     "DEPARTURE OF FC:  \n"),
                         getDiagString());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DepartureValidatorTest);
}
