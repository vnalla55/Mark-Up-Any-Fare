#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/TseCodeTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/FareSelectorQualifierChain.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{
class FareSelectorQualifierChainTest : public CppUnit::TestFixture
{
  template <PaxTypeFare::FareDisplayState RET>
  class QualifierMock : public Qualifier
  {
  public:
    QualifierMock() {}
    ~QualifierMock() {}

  protected:
    const PaxTypeFare::FareDisplayState
    qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
    {
      return RET;
    }
  };

  CPPUNIT_TEST_SUITE(FareSelectorQualifierChainTest);
  CPPUNIT_TEST(testFareSelectorQualifierChain_Empty);
  CPPUNIT_TEST(testFareSelectorQualifierChain_Pass);
  CPPUNIT_TEST(testFareSelectorQualifierChain_Fail);
  CPPUNIT_TEST_SUITE_END();

  FareDisplayTrx* _fdTrx;
  PaxTypeFare* _ptFare;
  FareSelectorQualifierChain* _selector;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _ptFare = _memHandle.create<PaxTypeFare>();
    _selector = _memHandle.create<FareSelectorQualifierChain>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFareSelectorQualifierChain_Empty()
  {
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Pax_Type_Code,
                         _selector->qualifyPaxTypeFare(*_fdTrx, *_ptFare));
  }

  void testFareSelectorQualifierChain_Pass()
  {
    _selector->setSuccessor<QualifierMock<PaxTypeFare::FD_Valid> >(
        _memHandle.insert(new QualifierMock<PaxTypeFare::FD_Valid>()), *_fdTrx);
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _selector->qualifyPaxTypeFare(*_fdTrx, *_ptFare));
  }

  void testFareSelectorQualifierChain_Fail()
  {
    _selector->setSuccessor<QualifierMock<PaxTypeFare::FD_BookingCode> >(
        _memHandle.insert(new QualifierMock<PaxTypeFare::FD_BookingCode>()), *_fdTrx);
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_BookingCode,
                         _selector->qualifyPaxTypeFare(*_fdTrx, *_ptFare));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareSelectorQualifierChainTest);
}
