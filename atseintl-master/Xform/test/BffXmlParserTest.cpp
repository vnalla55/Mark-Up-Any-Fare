
#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/FlightFinderTrx.h"
#include "Common/Global.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/AirSeg.h"

#include "Xform/XMLShoppingHandler.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/ShoppingSchemaNames.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"

#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using namespace shopping;

namespace
{
const bool
_bCheckWellFormedness(false);

ILookupMap _elemLookupMap, _attrLookupMap;
bool
bInit(IXMLUtils::initLookupMaps(shoppingElementNames,
                                _NumberElementNames_,
                                _elemLookupMap,
                                shoppingAttributeNames,
                                _NumberAttributeNames_,
                                _attrLookupMap));
}

class BffXmlParserTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BffXmlParserTest);
  CPPUNIT_TEST(STEP2_oneWayOneDayRequested);
  CPPUNIT_TEST(STEP2_oneWayFiveDaysRequested);
  CPPUNIT_TEST(STEP2_roundTripOneDayRequested);
  CPPUNIT_TEST(STEP2_roundTripThreeDaysRequested);
  CPPUNIT_TEST(STEP3_roundTripOneDayRequested);
  CPPUNIT_TEST(STEP3_roundTripFiveDayRequested);
  CPPUNIT_TEST(STEP4_roundTripFiveDayRequested);
  CPPUNIT_TEST(STEP5_oneWayRequested);
  CPPUNIT_TEST(STEP5_roundTripRequested);
  CPPUNIT_TEST(STEP6_roundTripRequested);
  CPPUNIT_TEST_SUITE_END();

  typedef ShoppingTrx::AltDateInfo AltDateInfo;
  typedef std::map<DatePair, AltDateInfo*>::const_iterator AltDateIC;

  void parse(const std::string& request)
  {
    IValueString attrValueArray[_NumberAttributeNames_];
    int attrRefArray[_NumberAttributeNames_];
    IXMLSchema schema(_elemLookupMap,
                      _attrLookupMap,
                      _NumberAttributeNames_,
                      attrValueArray,
                      attrRefArray,
                      _bCheckWellFormedness);
    IParser parser(request, *_handler, schema);
    parser.parse();
  }

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();

    TestConfigInitializer::setValue("BFF_LOGIC", 'Y', "SHOPPING_OPT");

    _handler = _memHandle(new XMLShoppingHandler(*_memHandle(new (DataHandle))));
    _ffTrx = _handler->_bffParser.ffinderTrx();
    _ffTrx->journeyItin() = _memHandle(new Itin);
  }

  void tearDown() { _memHandle.clear(); }

  Itin* retrieveAltDateItin(const DatePair& pair)
  {
    AltDateIC found = _ffTrx->altDatePairs().find(pair);
    if (found != _ffTrx->altDatePairs().end())
      return (*found).second->journeyItin;
    else
      return 0;
  }

  bool isDateEqual(const DateTime& d1, const DateTime& d2)
  {
    return d1.year() == d2.year() && d1.month() == d2.month() && d1.day() == d2.day();
  }

  std::string buildRequest(const std::string& orig,
                           const std::string& dest,
                           const std::string& carrier,
                           const std::string& owrt,
                           unsigned int step,
                           const DateTime& startDate,
                           unsigned int numDays)
  {
    std::ostringstream ostr;
    DateTime startDT = startDate;
    ostr << "<ShoppingRequest N06=\"B\">"
            "<BFF "
            "A11=\"" << orig << "\" "
                                "A12=\"" << dest << "\" ";

    if (step == 5 || step == 6)
    {
      ostr << "D01=\"" << startDate.dateToString(YYYYMMDD, "-") << "\" ";
    }
    else
    {
      ostr << "D01=\"\" ";
    }

    if (step == 6)
    {
      ostr << "D02=\"" << startDate.dateToString(YYYYMMDD, "-") << "\" ";
    }
    else
    {
      ostr << "D02=\"\" ";
    }

    ostr << "B00=\"" << carrier << "\" "
                                   "N23=\"" << owrt << "\" "
                                                       "Q6T=\"" << step
         << "\">"
            "<RQF B50=\"QHXCN9\" C75=\"928.00\" C76=\"USD\"/>";
    if (step == 1)
    {
      ostr << "<DRG D01=\"" << startDate.dateToString(YYYYMMDD, "-") << "\" Q4V=\"" << numDays
           << "\" />";
    }
    else if (step == 3)
    {

      ostr << "<DRG D01=\"\" Q4V=\"" << numDays << "\" />";
      ostr << "<DTL D17=\"" << startDate.dateToString(YYYYMMDD, "-") << "\" />";
    }

    if (step == 2)
    {
      for (unsigned int i = 0; i < numDays; i++)
      {
        ostr << "<DTL D17=\"" << startDT.dateToString(YYYYMMDD, "-") << "\" ></DTL>";
        startDT = startDT.nextDay();
      }
    }
    else if (step == 4)
    {

      ostr << "<DTL D17=\"" << startDT.dateToString(YYYYMMDD, "-") << "\" >";
      for (unsigned int i = 0; i < numDays; i++)
      {
        ostr << "<IBL D18=\"" << startDT.dateToString(YYYYMMDD, "-") << "\" ></IBL>";
        startDT = startDT.nextDay();
      }
      ostr << "</DTL>";
    }
    ostr << "</BFF></ShoppingRequest>";

    return ostr.str();
  }

  void test_ASSERT_SEG_FM_COUNT(int segFmNumber,
                                int numDays,
                                DateTime& outboundDT,
                                const DateTime& inboundDT = DateTime::emptyDate(),
                                bool roundTrip = false)
  {
    DateTime inbDT = inboundDT;
    for (int count = 0; count < numDays; count++)
    {

      Itin* altJourneyItin = retrieveAltDateItin(DatePair(outboundDT, inbDT));
      CPPUNIT_ASSERT(altJourneyItin != 0);

      CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)altJourneyItin->travelSeg().size());
      CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)altJourneyItin->fareMarket().size());
      CPPUNIT_ASSERT(isDateEqual(outboundDT, altJourneyItin->travelSeg().front()->departureDT()));
      if (roundTrip)
      {
        CPPUNIT_ASSERT(isDateEqual(inbDT, altJourneyItin->travelSeg().back()->departureDT()));
      }
      if (!inbDT.isEmptyDate())
      {
        inbDT = inbDT.nextDay();
      }
      else
      {
        outboundDT = outboundDT.nextDay();
      }
    }
  }

protected:
  void STEP2_oneWayOneDayRequested()
  {
    int numDays = 1;
    int step = 2;
    std::string owrt = "O";
    DateTime depDT = DateTime(2008, Dec, 10);
    int segFmNumber = (owrt == "O") ? 1 : 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, depDT, numDays));
    parse(request);
    // One segment for One way
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, depDT);
  }

  void STEP2_oneWayFiveDaysRequested()
  {
    int numDays = 5;
    int step = 2;
    std::string owrt = "O";
    DateTime depDT = DateTime(2008, Dec, 10);
    int segFmNumber = (owrt == "O") ? 1 : 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, depDT, numDays));
    parse(request);

    // One segment for One way
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, depDT);
  }

  void STEP2_roundTripOneDayRequested()
  {
    int numDays = 1;
    int step = 2;
    std::string owrt = "R";
    DateTime depDT = DateTime(2008, Dec, 10);
    int segFmNumber = (owrt == "O") ? 1 : 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, depDT, numDays));
    parse(request);

    // Two segments for round trip
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, depDT, DateTime::emptyDate(), true);
  }

  void STEP2_roundTripThreeDaysRequested()
  {
    int numDays = 3;
    int step = 2;
    std::string owrt = "R";
    DateTime depDT = DateTime(2008, Dec, 10);
    int segFmNumber = (owrt == "O") ? 1 : 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, depDT, numDays));
    parse(request);

    // Two segments for round trip
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, depDT, DateTime::emptyDate(), true);
  }

  void STEP3_roundTripOneDayRequested()
  {
    int numDays = 1;
    int step = 3;
    std::string owrt = "R";
    DateTime outboundDT = DateTime(2008, Dec, 10);
    DateTime inboundDT = DateTime(2008, Dec, 10);
    int segFmNumber = 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, outboundDT, numDays));
    parse(request);

    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, outboundDT, inboundDT, true);
  }

  void STEP3_roundTripFiveDayRequested()
  {
    int numDays = 5;
    int step = 3;
    std::string owrt = "R";
    DateTime outboundDT = DateTime(2008, Dec, 10);
    DateTime inboundDT = DateTime(2008, Dec, 10);
    int segFmNumber = 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, outboundDT, numDays));
    parse(request);

    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, outboundDT, inboundDT, true);
  }

  void STEP4_roundTripFiveDayRequested()
  {
    int numDays = 5;
    int step = 4;
    std::string owrt = "R";
    DateTime outboundDT = DateTime(2008, Dec, 10);
    DateTime inboundDT = DateTime(2008, Dec, 10);
    int segFmNumber = 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, outboundDT, numDays));
    parse(request);

    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());

    test_ASSERT_SEG_FM_COUNT(segFmNumber, numDays, outboundDT, inboundDT, true);
  }

  void STEP5_oneWayRequested()
  {
    int numDays = 0;
    int step = 5;
    std::string owrt = "O";
    DateTime outboundDT = DateTime(2008, Dec, 10);
    int segFmNumber = 1;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, outboundDT, numDays));
    parse(request);

    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());
  }

  void STEP5_roundTripRequested()
  {
    int numDays = 0;
    int step = 5;
    std::string owrt = "R";
    DateTime outboundDT = DateTime(2008, Dec, 10);
    int segFmNumber = 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, outboundDT, numDays));
    parse(request);

    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());
  }

  void STEP6_roundTripRequested()
  {
    int numDays = 0;
    int step = 6;
    std::string owrt = "R";
    DateTime outboundDT = DateTime(2008, Dec, 10);
    int segFmNumber = 2;

    std::string request(buildRequest("MCO", "LON", "AA", owrt, step, outboundDT, numDays));
    parse(request);

    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(segFmNumber, (int)_ffTrx->journeyItin()->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(numDays, (int)_ffTrx->altDatePairs().size());
  }

  class MyDataHandle : public DataHandleMock
  {
  public:
    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (locCode == "MCO")
        return "MCO";
      return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
    }
  };

private:
  XMLShoppingHandler* _handler;
  FlightFinderTrx* _ffTrx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BffXmlParserTest);

} // tse
