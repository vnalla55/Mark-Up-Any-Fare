#include "test/include/CppUnitHelperMacros.h"

#include "Rules/CarrierApplicationValidator.h"
#include "DataModel/RexBaseRequest.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "DBAccess/DataHandle.h"
#include "Common/TseConsts.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/DiagCollector.h"
#include <stack>

namespace tse
{

static const Indicator FORBID = 'X';

class FakeCarrierApplicationValidator : public CarrierApplicationValidator
{
public:
  FakeCarrierApplicationValidator(const RexBaseRequest& request,
                                  const VoluntaryRefundsInfo& record3,
                                  DiagCollector* dc,
                                  DataHandle& dh)
    : CarrierApplicationValidator(request, record3, dc, dh)
  {
  }

  CxrTbl _cxrTbl;

protected:
  virtual const CxrTbl& getCarrierApplTbl() { return _cxrTbl; }
};

class CarrierApplicationValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarrierApplicationValidatorTest);
  CPPUNIT_TEST(testBlankCxrs);
  CPPUNIT_TEST(testSameCxr);
  CPPUNIT_TEST(testDifferentCxrsNoTable);
  CPPUNIT_TEST(testNoCurrentCxrTableExist);
  CPPUNIT_TEST(testAllow);
  CPPUNIT_TEST(testForbid);
  CPPUNIT_TEST(testAllowAll);
  CPPUNIT_TEST(testAllowAllForbidOne);
  CPPUNIT_TEST(testDiagnostic);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _cav = _memHandle.insert(
        new FakeCarrierApplicationValidator(*_memHandle.insert(new RexBaseRequest),
                                            *_memHandle.insert(new VoluntaryRefundsInfo),
                                            0,
                                            *_memHandle.insert(new DataHandle)));
  }

  void tearDown() { _memHandle.clear(); }

  void testBlankCxrs() { CPPUNIT_ASSERT(_cav->determineValidity()); }
  void setUpCxrs(const CarrierCode& exc, const CarrierCode& curr)
  {
    const_cast<CarrierCode&>(_cav->_excCxr) = exc;
    const_cast<CarrierCode&>(_cav->_cxr) = curr;
  }
  void testSameCxr()
  {
    setUpCxrs(CARRIER_9B, CARRIER_9B);
    CPPUNIT_ASSERT(_cav->determineValidity());
  }
  void testDifferentCxrsNoTable()
  {
    setUpCxrs(CARRIER_9B, CARRIER_2R);
    CPPUNIT_ASSERT(!_cav->determineValidity());
  }
  void setUpCxrTable()
  {
    setUpCxrs(CARRIER_9B, CARRIER_2R);
    const_cast<VoluntaryRefundsInfo&>(_cav->_record3).carrierApplTblItemNo() = 1;
  }
  void addCxrTblElement(CarrierCode cxr, Indicator appl)
  {
    CarrierApplicationInfo* cai = _memHandle.insert(new CarrierApplicationInfo);

    cai->carrier() = cxr;
    cai->applInd() = appl;

    static_cast<FakeCarrierApplicationValidator&>(*_cav)._cxrTbl.push_back(cai);
  }
  void testNoCurrentCxrTableExist()
  {
    setUpCxrTable();
    setUpCxrs(CARRIER_9B, "");
    CPPUNIT_ASSERT(!_cav->determineValidity());
  }
  void testAllow()
  {
    addCxrTblElement(CARRIER_2R, CarrierApplicationValidator::ALLOW);
    setUpCxrTable();
    CPPUNIT_ASSERT(_cav->determineValidity());
  }
  void testForbid()
  {
    addCxrTblElement(CARRIER_2R, FORBID);
    setUpCxrTable();
    CPPUNIT_ASSERT(!_cav->determineValidity());
  }
  void testAllowAll()
  {
    addCxrTblElement(DOLLAR_CARRIER, CarrierApplicationValidator::ALLOW);
    setUpCxrTable();
    CPPUNIT_ASSERT(_cav->determineValidity());
  }
  void testAllowAllForbidOne()
  {
    addCxrTblElement(DOLLAR_CARRIER, CarrierApplicationValidator::ALLOW);
    addCxrTblElement(CARRIER_2R, FORBID);
    setUpCxrTable();
    CPPUNIT_ASSERT(!_cav->determineValidity());
  }
  void testDiagnostic()
  {
    addCxrTblElement(DOLLAR_CARRIER, CarrierApplicationValidator::ALLOW);
    addCxrTblElement(CARRIER_2R, FORBID);
    setUpCxrTable();
    _cav->_dc = _memHandle.insert(new DiagCollector);
    _cav->_dc->activate();
    _cav->composeDiagnostic(false);
    CPPUNIT_ASSERT_EQUAL(std::string("\nCXR APPL TBL: 1"
                                     "\nVALIDATING CXR: 2R"
                                     "\nCXR APPL: Y  CXR: $$"
                                     "\nCXR APPL: N  CXR: 2R"
                                     "\nFAILED ITEM 0 - REFUND NOT ALLOWED ON 2R\n"),
                         _cav->_dc->str());
  }

private:
  CarrierApplicationValidator* _cav;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierApplicationValidatorTest);
}
