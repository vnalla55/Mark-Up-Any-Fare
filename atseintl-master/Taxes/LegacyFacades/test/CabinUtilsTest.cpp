// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include <memory>
#include <vector>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/CabinType.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/SafeEnums.h"
#include "Taxes/LegacyFacades/CabinUtils.h"

#define PARAM_TESTS(_Inputs_, _Outputs_, _InitMethod_, _TestMethod_) \
  _InitMethod_(); \
  assert(_Inputs_.size() == _Outputs_.size()); \
  for (size_t i (0); i < _Inputs_.size(); ++i) \
  { \
    CPPUNIT_TEST_SUITE_ADD_TEST( \
    ( new CPPUNIT_NS::TestCaller<TestFixtureType>(context.getTestNameFor(#_TestMethod_ + boost::lexical_cast<std::string>(i)), \
                                                  &TestFixtureType::_TestMethod_, \
                                                  context.makeFixture() )) \
    ); \
  }


namespace tse
{

class CabinUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CabinUtilsTest);

  PARAM_TESTS(_cabinTestInputs, _cabinTestOutputs, initCabinTest, testTranslateCabin);
  CPPUNIT_TEST(testGetBookingCodeForTravelSegmentOWNotRebooked);
  CPPUNIT_TEST(testGetBookingCodeForTravelSegmentOWRebooked);
  CPPUNIT_TEST(testGetBookingCodeForTravelSegmentRTRebooked);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _itin.reset(new Itin());
    _fareUsage.reset(new FareUsage());
    _paxTypeFare.reset(new PaxTypeFare());
    _fareUsage->paxTypeFare() = _paxTypeFare.get();
    _cabin.reset(new CabinType());
  }

  static void initCabinTest()
  {
    using namespace boost::assign;

    _cabinTestCounter = 0;

    size_t testsCount = 9;
    _cabinTestInputs.resize(testsCount);
    _cabinTestInputs[0].setEconomyClass();
    _cabinTestInputs[1].setPremiumEconomyClass();
    _cabinTestInputs[2].setBusinessClass();
    _cabinTestInputs[3].setPremiumBusinessClass();
    _cabinTestInputs[4].setFirstClass();
    _cabinTestInputs[5].setPremiumFirstClass();
    _cabinTestInputs[6].setUnknownClass();
    _cabinTestInputs[7].setUndefinedClass();
    _cabinTestInputs[8].setInvalidClass();

    _cabinTestOutputs.clear();
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::Economy);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::PremiumEconomy);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::Business);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::PremiumBusiness);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::First);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::PremiumFirst);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::Blank);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::Blank);
    _cabinTestOutputs += tax::type::CabinCode(tax::type::CabinCode::Blank);
  }

  void testTranslateCabin()
  {
    CPPUNIT_ASSERT_EQUAL(_cabinTestOutputs[_cabinTestCounter],
                         CabinUtils::translateCabin(_cabinTestInputs[_cabinTestCounter]));
    ++_cabinTestCounter;
  }

  void testGetBookingCodeForTravelSegmentOWNotRebooked()
  {
    _cabin->setPremiumEconomyClass();
    TravelSeg* travelSegment = addTravelSegmentToItin("MUC", "LHR", "Y", *_cabin);
    _fareUsage->travelSeg().push_back(travelSegment);
    _paxTypeFare->segmentStatus().resize(1);
    _paxTypeFare->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);

    std::pair<std::string, tax::type::CabinCode> result =
        CabinUtils::getBookingCodeAndCabin(*_itin, *_fareUsage, travelSegment);
    CPPUNIT_ASSERT_EQUAL(std::string("Y"), result.first);
    CPPUNIT_ASSERT(tax::type::CabinCode::PremiumEconomy == result.second);
  }

  void testGetBookingCodeForTravelSegmentOWRebooked()
  {
    _cabin->setPremiumEconomyClass();
    CabinType rebookedCabin;
    rebookedCabin.setEconomyClass();
    TravelSeg* travelSegment = addTravelSegmentToItin("MUC", "LHR", "F", *_cabin);
    _fareUsage->travelSeg().push_back(travelSegment);
    _paxTypeFare->segmentStatus().resize(1);
    _paxTypeFare->segmentStatus()[0]._bkgCodeReBook = "C";
    _paxTypeFare->segmentStatus()[0]._reBookCabin = rebookedCabin;
    _paxTypeFare->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);

    std::pair<std::string, tax::type::CabinCode> result =
        CabinUtils::getBookingCodeAndCabin(*_itin, *_fareUsage, travelSegment);
    CPPUNIT_ASSERT_EQUAL(std::string("C"), result.first);
    CPPUNIT_ASSERT(tax::type::CabinCode::Economy == result.second);
  }

  void testGetBookingCodeForTravelSegmentRTRebooked()
  {
    _cabin->setBusinessClass();
    CabinType rebookedCabin;
    rebookedCabin.setEconomyClass();
    addTravelSegmentToItin("LHR", "MUC", "Y", *_cabin);
    _fareUsage->travelSeg().push_back(addTravelSegmentToItin("MUC", "KTW", "S", *_cabin));
    _fareUsage->travelSeg().push_back(addTravelSegmentToItin("KTW", "FRA", "F", *_cabin));
    addTravelSegmentToItin("FRA", "LHR", "Z", *_cabin);

    _paxTypeFare->segmentStatus().resize(2);
    _paxTypeFare->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
    _paxTypeFare->segmentStatus()[1]._bkgCodeReBook = "T";
    _paxTypeFare->segmentStatus()[1]._reBookCabin = rebookedCabin;
    _paxTypeFare->segmentStatus()[1]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);

    std::pair<std::string, tax::type::CabinCode> result =
        CabinUtils::getBookingCodeAndCabin(*_itin, *_fareUsage, _fareUsage->travelSeg().back());
    CPPUNIT_ASSERT_EQUAL(std::string("T"), result.first);
    CPPUNIT_ASSERT(tax::type::CabinCode::Economy == result.second);
  }
protected:
  TravelSeg* addTravelSegmentToItin(const std::string& origin,
                                    const std::string& destination,
                                    const BookingCode& bookingCode,
                                    const CabinType& cabin)
  {
    TravelSeg* travelSeg = createAirSegment();
    travelSeg->origin() = createLocation(origin);
    travelSeg->destination() = createLocation(destination);
    travelSeg->setBookingCode(bookingCode);
    travelSeg->bookedCabin() = cabin;

    _itin->travelSeg().push_back(travelSeg);
    return travelSeg;
  }

  TravelSeg* createAirSegment()
  {
    TravelSeg* airSeg = new AirSeg();
    _travelSegments.push_back(airSeg);
    return airSeg;
  }

  Loc* createLocation(const std::string& locCode)
  {
    _locations.push_back(new Loc());
    Loc* loc = &_locations.back();
    loc->loc() = locCode;
    return loc;
  }

private:
  static size_t _cabinTestCounter;
  static std::vector<CabinType> _cabinTestInputs;
  static std::vector<tax::type::CabinCode> _cabinTestOutputs;

  std::unique_ptr<Itin> _itin;
  std::unique_ptr<FareUsage> _fareUsage;
  std::unique_ptr<PaxTypeFare> _paxTypeFare;
  boost::ptr_vector<TravelSeg> _travelSegments;
  boost::ptr_vector<Loc> _locations;
  std::unique_ptr<CabinType> _cabin;
};

size_t CabinUtilsTest::_cabinTestCounter;
std::vector<CabinType> CabinUtilsTest::_cabinTestInputs;
std::vector<tax::type::CabinCode> CabinUtilsTest::_cabinTestOutputs;

CPPUNIT_TEST_SUITE_REGISTRATION(CabinUtilsTest);
}
