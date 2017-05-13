#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>

#include "DataModel/PricingTrx.h"
#include "DBAccess/DiskCache.h"
#include "Diagnostic/DiagManager.h"
#include "test/include/TestMemHandle.h"
#include "test/include/PrintCollection.h"
#include "Xform/AvailabilityAdjuster.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
using boost::assign::operator+=;

class AvailabilityAdjusterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AvailabilityAdjusterTest);

  CPPUNIT_TEST(testAdjustSeats_allPrefered);
  CPPUNIT_TEST(testAdjustSeats_noPrefered);
  CPPUNIT_TEST(testAdjustSeats_pariallyPrefered);
  CPPUNIT_TEST(testAdjustSeats_pariallyPreferedPariallySeats);
  CPPUNIT_TEST(testAdjustSeats_pariallyPreferedWithoutSeats);

  CPPUNIT_TEST(testProcess_primaryMatch);
  CPPUNIT_TEST(testProcess_secondaryMatch);
  CPPUNIT_TEST(testProcess_noMatch);

  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

protected:
  AvailabilityAdjuster* _adj;
  DiagManager* _diag;

  TestMemHandle _mem;

public:
  void setUp()
  {
    _mem.create<SpecificTestConfigInitializer>();
    PricingTrx* trx = _mem.create<PricingTrx>();
    trx->diagnostic().diagnosticType() = Diagnostic499;
    _diag = _mem(new DiagManager(*trx, Diagnostic499));
    _diag->collector().activate();

    _adj = _mem(new AvailabilityAdjuster(std::vector<BookingCode>(), std::vector<BookingCode>()));
  }

  void tearDown() { _mem.clear(); }

  std::string getDiagString()
  {
    _diag->collector().flushMsg();
    return _diag->collector().str();
  }

  typedef AvailabilityAdjuster::ClassOfServiceSet ClassOfServiceSet;

  struct ClassOfServiceStub
  {
    BookingCode code;
    uint16_t seats;
  };

  ClassOfService* createClassOfService(const ClassOfServiceStub& init)
  {
    ClassOfService* cos = _mem.create<ClassOfService>();
    cos->bookingCode() = init.code;
    cos->numSeats() = init.seats;
    return cos;
  }

  template <unsigned size>
  ClassOfServiceSet populateClassOfService(ClassOfServiceStub (&init)[size])
  {
    ClassOfServiceSet s;
    for (unsigned i = 0; i < size; ++i)
      s += createClassOfService(init[i]);
    return s;
  }

  ClassOfServiceSet populateClassOfService()
  {
    ClassOfServiceStub init[] = { { "A", 1 }, { "C", 2 }, { "B", 3 }, { "E", 4 }, { "D", 5 } };
    return populateClassOfService(init);
  }

  ClassOfServiceSet populateResetedClassOfService()
  {
    ClassOfServiceStub init[] = { { "A", 1 }, { "C", 0 }, { "B", 0 }, { "E", 4 }, { "D", 0 } };
    return populateClassOfService(init);
  }

  template <unsigned size>
  std::set<BookingCode> populateBookingCodes(BookingCode (&init)[size])
  {
    return std::set<BookingCode>(init, init + size);
  }

  std::set<BookingCode> populateBookingCodes()
  {
    BookingCode init[] = { "A", "E", "I", "J", "K" };
    return populateBookingCodes(init);
  }

  void testAdjustSeats_allPrefered()
  {
    BookingCode prefered[] = { "A", "B", "C", "D", "E" };

    ClassOfServiceSet cos = populateClassOfService();

    CPPUNIT_ASSERT(_adj->adjustSeats(populateBookingCodes(prefered), cos, *_diag));
    CPPUNIT_ASSERT_EQUAL(populateClassOfService(), cos);
    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  void testAdjustSeats_noPrefered()
  {
    BookingCode prefered[] = { "G", "H", "I", "J", "K" };

    ClassOfServiceSet cos = populateClassOfService();

    CPPUNIT_ASSERT(!_adj->adjustSeats(populateBookingCodes(prefered), cos, *_diag));
    CPPUNIT_ASSERT_EQUAL(populateClassOfService(), cos);
    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  std::string expectDiag(const char& bc)
  {
    return std::string("BRANDED CHK - SETTING THE AVAILABILITY OF BKCODE ") + BookingCode(bc) +
           " TO 0.\n";
  }

  void testAdjustSeats_pariallyPrefered()
  {
    ClassOfServiceSet cos = populateClassOfService();

    CPPUNIT_ASSERT(_adj->adjustSeats(populateBookingCodes(), cos, *_diag));
    CPPUNIT_ASSERT_EQUAL(populateResetedClassOfService(), cos);
    CPPUNIT_ASSERT_EQUAL(expectDiag('B') + expectDiag('C') + expectDiag('D'), getDiagString());
  }

  void testAdjustSeats_pariallyPreferedPariallySeats()
  {
    ClassOfServiceStub init[] = { { "A", 0 }, { "C", 3 }, { "B", 5 }, { "E", 4 }, { "D", 6 } };
    ClassOfServiceStub expect[] = { { "A", 0 }, { "C", 0 }, { "B", 0 }, { "E", 4 }, { "D", 0 } };

    ClassOfServiceSet cos = populateClassOfService(init);

    CPPUNIT_ASSERT(_adj->adjustSeats(populateBookingCodes(), cos, *_diag));
    CPPUNIT_ASSERT_EQUAL(populateClassOfService(expect), cos);
    CPPUNIT_ASSERT_EQUAL(expectDiag('B') + expectDiag('C') + expectDiag('D'), getDiagString());
  }

  void testAdjustSeats_pariallyPreferedWithoutSeats()
  {
    ClassOfServiceStub init[] = { { "A", 0 }, { "C", 3 }, { "B", 5 }, { "E", 0 }, { "D", 6 } };
    ClassOfServiceStub expect[] = { { "A", 0 }, { "C", 0 }, { "B", 0 }, { "E", 0 }, { "D", 0 } };

    ClassOfServiceSet cos = populateClassOfService(init);

    CPPUNIT_ASSERT(_adj->adjustSeats(populateBookingCodes(), cos, *_diag));
    CPPUNIT_ASSERT_EQUAL(populateClassOfService(expect), cos);
    CPPUNIT_ASSERT_EQUAL(expectDiag('B') + expectDiag('C') + expectDiag('D'), getDiagString());
  }

  std::vector<tse::ClassOfService*> getVector(const ClassOfServiceSet& cos)
  {
    return std::vector<tse::ClassOfService*>(cos.begin(), cos.end());
  }

  void testProcess_primaryMatch()
  {
    _adj->_primaryBookingCodes += "A", "B";
    _adj->_secondaryBookingCodes += "D", "E";

    ClassOfServiceStub expect[] = { { "A", 1 }, { "C", 0 }, { "B", 3 }, { "E", 0 }, { "D", 0 } };

    std::vector<tse::ClassOfService*> cos = getVector(populateClassOfService());

    _adj->process(cos, *_diag);

    CPPUNIT_ASSERT_EQUAL(getVector(populateClassOfService(expect)), cos);
    CPPUNIT_ASSERT_EQUAL(expectDiag('C') + expectDiag('D') + expectDiag('E'), getDiagString());
  }

  void testProcess_secondaryMatch()
  {
    _adj->_primaryBookingCodes += "Z", "Y";
    _adj->_secondaryBookingCodes += "D", "E";

    ClassOfServiceStub expect[] = { { "A", 0 }, { "C", 0 }, { "B", 0 }, { "E", 4 }, { "D", 5 } };

    std::vector<tse::ClassOfService*> cos = getVector(populateClassOfService());

    _adj->process(cos, *_diag);

    CPPUNIT_ASSERT_EQUAL(getVector(populateClassOfService(expect)), cos);
    CPPUNIT_ASSERT_EQUAL(expectDiag('A') + expectDiag('B') + expectDiag('C'), getDiagString());
  }

  void testProcess_noMatch()
  {
    _adj->_primaryBookingCodes += "Z", "Y";
    _adj->_secondaryBookingCodes += "G", "H";

    ClassOfServiceStub expect[] = { { "A", 0 }, { "C", 0 }, { "B", 0 }, { "E", 0 }, { "D", 0 } };

    std::vector<tse::ClassOfService*> cos = getVector(populateClassOfService());

    _adj->process(cos, *_diag);

    CPPUNIT_ASSERT_EQUAL(getVector(populateClassOfService(expect)), cos);
    CPPUNIT_ASSERT_EQUAL(expectDiag('A') + expectDiag('B') + expectDiag('C') + expectDiag('D') +
                             expectDiag('E'),
                         getDiagString());
  }
};

struct IsClassOfServiceEqual
{
  bool operator()(const ClassOfService* lh, const ClassOfService* rh) const
  {
    return (lh->bookingCode() == rh->bookingCode() && lh->numSeats() == rh->numSeats());
  }
};

bool operator==(const AvailabilityAdjusterTest::ClassOfServiceSet& lh,
                const AvailabilityAdjusterTest::ClassOfServiceSet& rh)
{
  return std::equal(lh.begin(), lh.end(), rh.begin(), IsClassOfServiceEqual());
}

bool operator==(const std::vector<ClassOfService*>& lh, const std::vector<ClassOfService*>& rh)
{
  return std::equal(lh.begin(), lh.end(), rh.begin(), IsClassOfServiceEqual());
}

std::ostream& operator<<(std::ostream& os, const ClassOfService* cos)
{
  return os << '(' << cos->bookingCode() << ',' << cos->numSeats() << ')';
}

CPPUNIT_TEST_SUITE_REGISTRATION(AvailabilityAdjusterTest);

} // tse
