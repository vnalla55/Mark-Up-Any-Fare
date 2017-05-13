
#include "Common/ShoppingTaxUtil.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class ShoppingTaxUtilFake : public ShoppingTaxUtil
{
public:
  ShoppingTaxUtilFake() : _taxNation(0) {}
  virtual ~ShoppingTaxUtilFake() {}

  const TaxNation* getTaxNation(const NationCode, DataHandle&) const
  {
    return _taxNation;
  }
  const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode&, DataHandle&) const
  {
    return _taxCode;
  }

  void addTaxCodeReg(TaxCodeReg* taxCodeReg) { _taxCode.push_back(taxCodeReg); }
  void clearTaxCodeReg() { _taxCode.clear(); }
  void setTaxNation(TaxNation* taxNation) { _taxNation = taxNation; }

private:
  std::vector<TaxCodeReg*> _taxCode;
  TaxNation* _taxNation;
};

class TaxDataFake : public std::multimap<TaxCode, TaxCodeReg*>
{
  using std::multimap<TaxCode, TaxCodeReg*>::insert;

public:
  void insert(TaxCodeReg& tr)
  {
    insert(value_type(tr.taxCode(), &tr));
  };
};

class FlightRangesMock : public ShoppingTaxUtil::FlightRanges
{
public:
  typedef std::pair<FlightNumber, FlightNumber> FlightRange;

  FlightRangesMock(const NationCode& nation,
                   const DateTime& tktDate,
                   const TaxNation* tn,
                   const TaxDataFake& taxData)
    : FlightRanges(nation, tktDate), _tn(tn), _taxData(taxData) {};
  ~FlightRangesMock() {};

  std::map<CarrierCode, std::vector<FlightRange> >& flightRanges()
  {
    return _flightRanges;
  };

  std::vector<TaxCodeReg*> taxCodes;

protected:
  const TaxNation* _tn;
  const TaxDataFake& _taxData;
  std::vector<TaxCodeReg*> _taxCodeRegs;
  virtual const TaxNation* getTaxNation()
  {
    return _tn;
  };
  virtual const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& tc)
  {
    _taxCodeRegs.clear();
    std::pair<TaxDataFake::const_iterator, TaxDataFake::const_iterator> r =
        _taxData.equal_range(tc);
    for (; r.first != r.second; ++r.first)
      _taxCodeRegs.push_back(r.first->second);
    return _taxCodeRegs;
  }
};

class ShoppingTaxUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShoppingTaxUtilTest);

  CPPUNIT_TEST(testInitFlightRangesReadAllTaxes);
  CPPUNIT_TEST(testInitFlightRangesIgnoreRecsWithoutFltNo);
  CPPUNIT_TEST(testInitFlightRangesCreateOneElementRange);
  CPPUNIT_TEST(testInitFlightRangesSelectUniqueRanges);
  CPPUNIT_TEST(testInitFlightRangesDistinguishCarriers);
  CPPUNIT_TEST(testInitFlightRangesTaxNationMissing);

  CPPUNIT_TEST(testBuildFltRangeKeySelectRange);
  CPPUNIT_TEST(testBuildFltRangeKeyFindAllRanges);
  CPPUNIT_TEST(testBuildFltRangeRangeDoesNotMatch);
  CPPUNIT_TEST(testBuildFltRangeMixedCarriers);
  CPPUNIT_TEST(testBuildFltRangeOtherNations);
  CPPUNIT_TEST(testBuildFltRangeBoundaries);

  CPPUNIT_TEST(testNationTransitHoursEmptyTaxNationNull);
  CPPUNIT_TEST(testNationTransitHoursEmptyTaxCodeEmpty);
  CPPUNIT_TEST(testNationTransitHoursEmptyTaxCodeRegEmpty);
  CPPUNIT_TEST(testNationTransitHoursEmptyRestTransitEmpty);
  CPPUNIT_TEST(testNationTransitHoursSingleRest);
  CPPUNIT_TEST(testNationTransitHoursSameRest);
  CPPUNIT_TEST(testNationTransitHoursSameRest2);
  CPPUNIT_TEST(testNationTransitHoursTwoRest);
  CPPUNIT_TEST(testNationTransitHoursFourRest);
  CPPUNIT_TEST(testNationTransitMinutesTest);
  CPPUNIT_TEST(testNationTransitMinutesRounding);

  CPPUNIT_TEST(testNationRestrWithNextStopover);
  CPPUNIT_TEST(testTaxFirstTravelDates);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  AirSeg* buildAirSeg(const NationCode& originNation,
                      const NationCode& destNation,
                      const CarrierCode& cxr,
                      FlightNumber fn)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    Loc* loc;
    seg->origin() = loc = _memHandle.create<Loc>();
    loc->nation() = originNation;
    seg->destination() = loc = _memHandle.create<Loc>();
    loc->nation() = destNation;
    seg->carrier() = cxr;
    seg->flightNumber() = fn;
    return seg;
  }

public:
  void setUp() {}

  void tearDown() { _memHandle.clear(); }

  void testInitFlightRangesReadAllTaxes()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("UB1");
    TaxDataFake td;
    TaxCodeReg tr1, tr2, tr3;
    TaxExemptionCarrier te1, te2, te3;

    te1.carrier() = "BA";
    te1.flight1() = 1;
    te1.flight2() = 2;

    te2.carrier() = "BA";
    te2.flight1() = 3;
    te2.flight2() = 4;

    te3.carrier() = "BA";
    te3.flight1() = 5;
    te3.flight2() = 6;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB1";
    tr2.exemptionCxr().push_back(te2);
    tr3.taxCode() = "UB1";
    tr3.exemptionCxr().push_back(te3);

    td.insert(tr1);
    td.insert(tr2);
    td.insert(tr3);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    CPPUNIT_ASSERT_EQUAL(3ul, fr.flightRanges()["BA"].size());
  }

  void testInitFlightRangesIgnoreRecsWithoutFltNo()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");
    tn.taxCodeOrder().push_back("UB1");
    TaxDataFake td;
    TaxCodeReg tr1, tr2, tr3;
    TaxExemptionCarrier te1, te2, te3;

    te1.carrier() = "BA";
    te1.flight1() = 1;
    te1.flight2() = 2;

    te2.carrier() = "BA";
    te2.flight1() = 3;
    te2.flight2() = 4;

    te3.carrier() = "BA";
    te3.flight1() = -1;
    te3.flight2() = -1;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);
    tr3.taxCode() = "UB1";
    tr3.exemptionCxr().push_back(te3);

    td.insert(tr1);
    td.insert(tr2);
    td.insert(tr3);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();
    CPPUNIT_ASSERT_EQUAL(2ul, fr.flightRanges()["BA"].size());
  }

  void testInitFlightRangesCreateOneElementRange()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");

    TaxDataFake td;
    TaxCodeReg tr;
    TaxExemptionCarrier te;

    te.carrier() = "BA";
    te.flight1() = 1;
    te.flight2() = -1;

    tr.taxCode() = "GB1";
    tr.exemptionCxr().push_back(te);

    td.insert(tr);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    const std::vector<FlightRangesMock::FlightRange>& ranges = fr.flightRanges()["BA"];
    CPPUNIT_ASSERT(std::find(ranges.begin(), ranges.end(), FlightRangesMock::FlightRange(1, 1)) !=
                   ranges.end());
  }

  void testInitFlightRangesSelectUniqueRanges()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2;

    te1.carrier() = "BA";
    te1.flight1() = 1;
    te1.flight2() = 2;

    te2.carrier() = "BA";
    te2.flight1() = 1;
    te2.flight2() = 2;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();
    const std::vector<FlightRangesMock::FlightRange>& ranges = fr.flightRanges()["BA"];

    CPPUNIT_ASSERT(std::find(ranges.begin(), ranges.end(), FlightRangesMock::FlightRange(1, 2)) !=
                   ranges.end());
    CPPUNIT_ASSERT_EQUAL(1ul, ranges.size());
  }

  void testInitFlightRangesDistinguishCarriers()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2;

    te1.carrier() = "BA";
    te1.flight1() = 1;
    te1.flight2() = 2;

    te2.carrier() = "UA";
    te2.flight1() = 1;
    te2.flight2() = 2;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();
    const std::vector<FlightRangesMock::FlightRange>& rangesBA = fr.flightRanges()["BA"];
    const std::vector<FlightRangesMock::FlightRange>& rangesUA = fr.flightRanges()["UA"];

    CPPUNIT_ASSERT(std::find(rangesBA.begin(),
                             rangesBA.end(),
                             FlightRangesMock::FlightRange(1, 2)) != rangesBA.end());
    CPPUNIT_ASSERT(std::find(rangesUA.begin(),
                             rangesUA.end(),
                             FlightRangesMock::FlightRange(1, 2)) != rangesUA.end());
    CPPUNIT_ASSERT_EQUAL(1ul, rangesBA.size());
    CPPUNIT_ASSERT_EQUAL(1ul, rangesUA.size());
    CPPUNIT_ASSERT_EQUAL(2ul, fr.flightRanges().size());
  }

  void testInitFlightRangesTaxNationMissing()
  {
    TaxDataFake td;
    FlightRangesMock fr("GB", DateTime::localTime(), 0, td);
    CPPUNIT_ASSERT_NO_THROW(fr.initFlightRanges());
    CPPUNIT_ASSERT(fr.flightRanges().empty());
  }

  void testBuildFltRangeKeySelectRange()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2;

    te1.carrier() = "BA";
    te1.flight1() = 100;
    te1.flight2() = 200;

    te2.carrier() = "BA";
    te2.flight1() = 300;
    te2.flight2() = 400;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    std::string key("XXX");
    Itin itin;
    itin.travelSeg().push_back(buildAirSeg("US", "GB", "BA", 103));

    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBA_1"), key);
  }

  void testBuildFltRangeKeyFindAllRanges()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2;

    te1.carrier() = "BA";
    te1.flight1() = 100;
    te1.flight2() = 300;

    te2.carrier() = "BA";
    te2.flight1() = 200;
    te2.flight2() = 400;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    std::string key("XXX");
    Itin itin;
    itin.travelSeg().push_back(buildAirSeg("US", "GB", "BA", 210));

    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBA_1_2"), key);
  }

  void testBuildFltRangeRangeDoesNotMatch()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2;

    te1.carrier() = "BA";
    te1.flight1() = 100;
    te1.flight2() = 200;

    te2.carrier() = "BA";
    te2.flight1() = 300;
    te2.flight2() = 400;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    std::string key("XXX");
    Itin itin;
    itin.travelSeg().push_back(buildAirSeg("US", "GB", "BA", 210)); // Doesn't match any range
    itin.travelSeg().push_back(buildAirSeg("GB", "PL", "BA", 340));

    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBABA_2"), key);

    static_cast<AirSeg*>(itin.travelSeg().back())->flightNumber() = 290;

    key = "XXX";
    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBABA"), key);
  }

  void testBuildFltRangeMixedCarriers()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2;

    te1.carrier() = "BA";
    te1.flight1() = 100;
    te1.flight2() = 200;

    te2.carrier() = "LH";
    te2.flight1() = 200;
    te2.flight2() = 400;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    std::string key("XXX");
    Itin itin;
    itin.travelSeg().push_back(buildAirSeg("US", "GB", "BA", 110));
    itin.travelSeg().push_back(buildAirSeg("GB", "DE", "LH", 210));

    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBA_1LH_1"), key);
  }

  void testBuildFltRangeOtherNations()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB1");
    tn.taxCodeOrder().push_back("GB2");

    TaxDataFake td;
    TaxCodeReg tr1, tr2;
    TaxExemptionCarrier te1, te2, te3;

    te1.carrier() = "BA";
    te1.flight1() = 100;
    te1.flight2() = 200;

    te2.carrier() = "LH";
    te2.flight1() = 200;
    te2.flight2() = 400;

    te3.carrier() = "LH";
    te3.flight1() = 500;
    te3.flight2() = 600;

    tr1.taxCode() = "GB1";
    tr1.exemptionCxr().push_back(te1);
    tr2.taxCode() = "GB2";
    tr2.exemptionCxr().push_back(te2);
    tr2.exemptionCxr().push_back(te3);

    td.insert(tr1);
    td.insert(tr2);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    std::string key("XXX");
    Itin itin;
    itin.travelSeg().push_back(buildAirSeg("US", "GB", "BA", 110));
    itin.travelSeg().push_back(buildAirSeg("GB", "DE", "LH", 210));
    itin.travelSeg().push_back(buildAirSeg("DE", "PL", "LH", 650));
    itin.travelSeg().push_back(buildAirSeg("PL", "RU", "LO", 550));
    itin.travelSeg().push_back(buildAirSeg("RU", "GB", "LH", 501));
    itin.travelSeg().push_back(buildAirSeg("GB", "CA", "BA", 140));
    itin.travelSeg().push_back(buildAirSeg("CA", "US", "AA", 350));

    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBA_1LH_1LH_2BA_1"), key);
  }

  void testBuildFltRangeBoundaries()
  {
    TaxNation tn;
    tn.taxCodeOrder().push_back("GB6");

    TaxDataFake td;
    TaxCodeReg tr;
    TaxExemptionCarrier te1, te2, te3, te4;

    te1.carrier() = "BA";
    te1.flight1() = 100;
    te1.flight2() = 200;

    te2.carrier() = "BA";
    te2.flight1() = 201;
    te2.flight2() = 240;

    te3.carrier() = "LH";
    te3.flight1() = 770;
    te3.flight2() = 776;

    te4.carrier() = "LH";
    te4.flight1() = 777;
    te4.flight2() = 777;

    tr.taxCode() = "GB6";
    tr.exemptionCxr().push_back(te1);
    tr.exemptionCxr().push_back(te2);
    tr.exemptionCxr().push_back(te3);
    tr.exemptionCxr().push_back(te4);

    td.insert(tr);

    FlightRangesMock fr("GB", DateTime::localTime(), &tn, td);
    fr.initFlightRanges();

    std::string key("XXX");
    Itin itin;
    itin.travelSeg().push_back(buildAirSeg("US", "GB", "BA", 100));
    itin.travelSeg().push_back(buildAirSeg("GB", "DE", "LH", 776));
    itin.travelSeg().push_back(buildAirSeg("DE", "GB", "LH", 777));
    itin.travelSeg().push_back(buildAirSeg("GB", "US", "BA", 201));

    fr.buildFltRangeKey(itin, key);
    CPPUNIT_ASSERT_EQUAL(std::string("XXX|FLTRNG-GBBA_1LH_1LH_2BA_2"), key);
  }

  void testNationTransitHoursEmptyTaxNationNull()
  {
    ShoppingTaxUtilFake stu;

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT(transitHours.empty());
    CPPUNIT_ASSERT(transitMinutes.empty());
  }

  void testNationTransitHoursEmptyTaxCodeEmpty()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    stu.setTaxNation(&tn);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT(transitHours.empty());
    CPPUNIT_ASSERT(transitMinutes.empty());
  }

  void testNationTransitHoursEmptyTaxCodeRegEmpty()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT(transitHours.empty());
    CPPUNIT_ASSERT(transitMinutes.empty());
  }

  void testNationTransitHoursEmptyRestTransitEmpty()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    stu.addTaxCodeReg(&taxCodeReg);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT(transitHours.empty());
    CPPUNIT_ASSERT(transitMinutes.empty());
  }

  void testNationTransitHoursSingleRest()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    trt.transitMinutes() = 2;
    taxCodeReg.restrictionTransit().push_back(trt);
    stu.addTaxCodeReg(&taxCodeReg);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT_EQUAL(transitHours.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT_EQUAL(transitMinutes.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT_EQUAL(*transitHours.begin(), Hours(1));
    CPPUNIT_ASSERT_EQUAL(*transitMinutes.begin(), Minutes(1 * 60 + 2));
  }

  void testNationTransitHoursSameRest()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    TaxRestrictionTransit trt2;
    trt2.transitHours() = 1;
    taxCodeReg.restrictionTransit().push_back(trt);
    taxCodeReg.restrictionTransit().push_back(trt2);
    stu.addTaxCodeReg(&taxCodeReg);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT_EQUAL(transitHours.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT_EQUAL(*transitHours.begin(), Hours(1));
    CPPUNIT_ASSERT_EQUAL(*transitMinutes.begin(), Minutes(1 * 60));
  }

  void testNationTransitHoursSameRest2()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    TaxRestrictionTransit trt2;
    trt2.transitHours() = 1;
    taxCodeReg.restrictionTransit().push_back(trt);
    taxCodeReg.restrictionTransit().push_back(trt2);
    stu.addTaxCodeReg(&taxCodeReg);
    TaxCodeReg taxCodeReg2;
    taxCodeReg2.restrictionTransit().push_back(trt);
    stu.addTaxCodeReg(&taxCodeReg2);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT_EQUAL(transitHours.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT_EQUAL(*transitHours.begin(), Hours(1));
    CPPUNIT_ASSERT_EQUAL(*transitMinutes.begin(), Minutes(1 * 60));
  }

  void testNationTransitHoursTwoRest()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    TaxRestrictionTransit trt2;
    trt2.transitHours() = 1;
    TaxRestrictionTransit trt3;
    trt3.transitHours() = 3;
    taxCodeReg.restrictionTransit().push_back(trt);
    taxCodeReg.restrictionTransit().push_back(trt2);
    taxCodeReg.restrictionTransit().push_back(trt3);
    stu.addTaxCodeReg(&taxCodeReg);
    TaxCodeReg taxCodeReg2;
    taxCodeReg2.restrictionTransit().push_back(trt);
    stu.addTaxCodeReg(&taxCodeReg2);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT_EQUAL(transitHours.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT_EQUAL(transitMinutes.size(), static_cast<size_t>(2));
    std::set<Hours>::const_iterator it = transitHours.begin();
    CPPUNIT_ASSERT_EQUAL(*it, Hours(1));
    CPPUNIT_ASSERT_EQUAL(*transitMinutes.begin(), Minutes(1 * 60));
    ++it;
    CPPUNIT_ASSERT_EQUAL(*it, Hours(3));

    std::set<Minutes>::const_iterator minIt = transitMinutes.begin();
    CPPUNIT_ASSERT_EQUAL(*minIt, Minutes(1 * 60));
    ++minIt;
    CPPUNIT_ASSERT_EQUAL(*minIt, Minutes(3 * 60));

  }

  void testNationTransitHoursFourRest()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    TaxRestrictionTransit trt2;
    trt2.transitHours() = 1;
    TaxRestrictionTransit trt3;
    trt3.transitHours() = 3;
    taxCodeReg.restrictionTransit().push_back(trt);
    taxCodeReg.restrictionTransit().push_back(trt2);
    taxCodeReg.restrictionTransit().push_back(trt3);
    stu.addTaxCodeReg(&taxCodeReg);
    TaxCodeReg taxCodeReg2;
    TaxRestrictionTransit trt24;
    trt24.transitHours() = 24;
    TaxRestrictionTransit trt12;
    trt12.transitHours() = 12;
    taxCodeReg2.restrictionTransit().push_back(trt);
    taxCodeReg2.restrictionTransit().push_back(trt24);
    taxCodeReg2.restrictionTransit().push_back(trt12);
    stu.addTaxCodeReg(&taxCodeReg2);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT_EQUAL(transitHours.size(), static_cast<size_t>(4));
    std::set<Hours>::const_iterator hourIt = transitHours.begin();
    std::set<Minutes>::const_iterator minuteIt = transitMinutes.begin();

    CPPUNIT_ASSERT_EQUAL(*hourIt, Hours(1));
    CPPUNIT_ASSERT_EQUAL(*minuteIt, Minutes(60));

    CPPUNIT_ASSERT_EQUAL(*(++hourIt), Hours(3));
    CPPUNIT_ASSERT_EQUAL(*(++minuteIt), Minutes(3 * 60));

    CPPUNIT_ASSERT_EQUAL(*(++hourIt), Hours(12));
    CPPUNIT_ASSERT_EQUAL(*(++minuteIt), Minutes(12 * 60));

    CPPUNIT_ASSERT_EQUAL(*(++hourIt), Hours(24));
    CPPUNIT_ASSERT_EQUAL(*(++minuteIt), Minutes(24 * 60));
  }

  void testNationTransitMinutesTest()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("ABC");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    trt.transitMinutes() = 15;
    TaxRestrictionTransit trt2;
    trt2.transitHours() = 1;
    trt2.transitMinutes() = 25;
    TaxRestrictionTransit trt3;
    trt3.transitHours() = 1;
    trt3.transitMinutes() = 45;
    taxCodeReg.restrictionTransit().push_back(trt);
    taxCodeReg.restrictionTransit().push_back(trt2);
    taxCodeReg.restrictionTransit().push_back(trt3);
    stu.addTaxCodeReg(&taxCodeReg);
    TaxCodeReg taxCodeReg2;
    taxCodeReg2.restrictionTransit().push_back(trt);
    stu.addTaxCodeReg(&taxCodeReg2);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    CPPUNIT_ASSERT_EQUAL(transitHours.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT_EQUAL(transitMinutes.size(), static_cast<size_t>(3));
    CPPUNIT_ASSERT_EQUAL(*transitHours.begin(), Hours(1));

    std::set<Minutes>::const_iterator it = transitMinutes.begin();
    CPPUNIT_ASSERT_EQUAL(*it, Minutes(1 * 60 + 15));
    ++it;
    CPPUNIT_ASSERT_EQUAL(*it, Minutes(1 * 60 + 25));
    ++it;
    CPPUNIT_ASSERT_EQUAL(*it, Minutes(1 * 60 + 45));
  }

  void testNationTransitMinutesRounding()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("AA");
    tn.taxCodeOrder().push_back("BB");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "AA";
    TaxRestrictionTransit trt;
    trt.transitHours() = 1;
    trt.transitMinutes() = -1;
    TaxRestrictionTransit trt2;
    trt2.transitHours() = -1;
    trt2.transitMinutes() = -1;
    TaxRestrictionTransit trt3;
    trt3.transitHours() = -1;
    trt3.transitMinutes() = 15;

    taxCodeReg.restrictionTransit().push_back(trt);
    taxCodeReg.restrictionTransit().push_back(trt2);
    taxCodeReg.restrictionTransit().push_back(trt3);
    stu.addTaxCodeReg(&taxCodeReg);

    TaxCodeReg taxCodeReg2;
    taxCodeReg2.taxCode() = "BB";
    TaxRestrictionTransit trt4;
    trt4.transitHours() = 2;
    trt4.transitMinutes() = -1;
    taxCodeReg2.restrictionTransit().push_back(trt4);
    stu.addTaxCodeReg(&taxCodeReg2);

    std::set<Hours> transitHours;
    std::set<Minutes> transitMinutes;
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec(), DateTime());

    std::set<Hours>::const_iterator hoursIt = transitHours.begin();
    std::set<Minutes>::const_iterator minutesIt = transitMinutes.begin();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), transitHours.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), transitMinutes.size());
    CPPUNIT_ASSERT_EQUAL(Hours(1), *hoursIt);
    CPPUNIT_ASSERT_EQUAL(Hours(2), *(++hoursIt));
    CPPUNIT_ASSERT_EQUAL(Minutes(15), *minutesIt);
    CPPUNIT_ASSERT_EQUAL(Minutes(60), *(++minutesIt));
    CPPUNIT_ASSERT_EQUAL(Minutes(120), *(++minutesIt));

    transitHours.clear();
    transitMinutes.clear();
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec{"AA"}, DateTime());
    hoursIt = transitHours.begin();
    minutesIt = transitMinutes.begin();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), transitHours.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), transitMinutes.size());
    CPPUNIT_ASSERT_EQUAL(Hours(1), *hoursIt);
    CPPUNIT_ASSERT_EQUAL(Hours(2), *(++hoursIt));
    CPPUNIT_ASSERT_EQUAL(Minutes(15), *minutesIt);
    CPPUNIT_ASSERT_EQUAL(Minutes(59), *(++minutesIt));
    CPPUNIT_ASSERT_EQUAL(Minutes(120), *(++minutesIt));

    transitHours.clear();
    transitMinutes.clear();
    stu.getNationTransitTimes(transitHours, transitMinutes, "DE", TaxCodesVec{"AA", "BB"}, DateTime());
    hoursIt = transitHours.begin();
    minutesIt = transitMinutes.begin();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), transitHours.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), transitMinutes.size());
    CPPUNIT_ASSERT_EQUAL(Hours(1), *hoursIt);
    CPPUNIT_ASSERT_EQUAL(Hours(2), *(++hoursIt));
    CPPUNIT_ASSERT_EQUAL(Minutes(15), *minutesIt);
    CPPUNIT_ASSERT_EQUAL(Minutes(59), *(++minutesIt));
    CPPUNIT_ASSERT_EQUAL(Minutes(119), *(++minutesIt));
  }

  void testNationRestrWithNextStopover()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("UT");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    taxCodeReg.nextstopoverrestr() = 'N';
    stu.addTaxCodeReg(&taxCodeReg);
    TaxCodeReg taxCodeReg2;
    taxCodeReg2.nextstopoverrestr() = 'Y';
    stu.addTaxCodeReg(&taxCodeReg2);

    CPPUNIT_ASSERT_MESSAGE("stopover rest - pass", stu.getRestrWithNextStopover(NationCode("DE"), DateTime()));

    ShoppingTaxUtilFake stu2;
    CPPUNIT_ASSERT_MESSAGE("stopover rest - empty TaxNation", !stu2.getRestrWithNextStopover(NationCode("DE"), DateTime()));
    TaxNation tn2;
    stu2.setTaxNation(&tn2);
    CPPUNIT_ASSERT_MESSAGE("stopover rest - empty TaxCode", !stu2.getRestrWithNextStopover(NationCode("DE"), DateTime()));
    tn2.taxCodeOrder().push_back("UT");
    CPPUNIT_ASSERT_MESSAGE("stopover rest - nextstopover not set", !stu2.getRestrWithNextStopover(NationCode("DE"), DateTime()));
    TaxCodeReg taxCodeReg3;
    taxCodeReg3.nextstopoverrestr() = 'N';
    stu2.addTaxCodeReg(&taxCodeReg3);
    CPPUNIT_ASSERT_MESSAGE("stopover rest - nextstopover = N", !stu2.getRestrWithNextStopover(NationCode("DE"), DateTime()));
    taxCodeReg3.nextstopoverrestr() = 'Y';
    CPPUNIT_ASSERT_MESSAGE("stopover rest - pass 2", stu2.getRestrWithNextStopover(NationCode("DE"), DateTime()));
  }

  void testTaxFirstTravelDates()
  {
    ShoppingTaxUtilFake stu;
    TaxNation tn;
    tn.taxCodeOrder().push_back("E7");
    stu.setTaxNation(&tn);
    TaxCodeReg taxCodeReg;
    taxCodeReg.firstTvlDate() = boost::gregorian::from_string("2015-11-01");
    stu.addTaxCodeReg(&taxCodeReg);
    TaxCodeReg taxCodeReg2;
    taxCodeReg2.firstTvlDate() = boost::gregorian::from_string("2015-11-10");
    stu.addTaxCodeReg(&taxCodeReg2);
    TaxCodeReg taxCodeReg3;
    taxCodeReg3.firstTvlDate() = boost::gregorian::from_string("2015-11-05");
    stu.addTaxCodeReg(&taxCodeReg3);
    TaxCodeReg taxCodeReg4;
    taxCodeReg4.firstTvlDate() = boost::gregorian::from_string("2015-11-06");
    stu.addTaxCodeReg(&taxCodeReg4);


    std::set<boost::gregorian::date> firstTravelDates;
    stu.getTaxFirstTravelDates(firstTravelDates, boost::gregorian::from_string("2015-11-01"), boost::gregorian::from_string("2015-11-10"), NationCode("DE"), DateTime());

    CPPUNIT_ASSERT(firstTravelDates.size() == 2u);
    CPPUNIT_ASSERT(firstTravelDates.find(boost::gregorian::from_string("2015-11-05")) != firstTravelDates.end());
    CPPUNIT_ASSERT(firstTravelDates.find(boost::gregorian::from_string("2015-11-06")) != firstTravelDates.end());
    CPPUNIT_ASSERT(firstTravelDates.find(boost::gregorian::from_string("2015-11-01")) == firstTravelDates.end());
    CPPUNIT_ASSERT(firstTravelDates.find(boost::gregorian::from_string("2015-11-10")) == firstTravelDates.end());
    CPPUNIT_ASSERT(firstTravelDates.find(boost::gregorian::from_string("2015-11-12")) == firstTravelDates.end());
    CPPUNIT_ASSERT(firstTravelDates.find(boost::gregorian::from_string("2015-10-30")) == firstTravelDates.end());

    ShoppingTaxUtilFake stu2;
    std::set<boost::gregorian::date> firstTravelDates2;
    stu2.getTaxFirstTravelDates(firstTravelDates2, boost::gregorian::from_string("2015-11-01"), boost::gregorian::from_string("2015-11-10"), NationCode("DE"), DateTime());
    CPPUNIT_ASSERT_MESSAGE("firstTravelDates empty", firstTravelDates2.empty());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(ShoppingTaxUtilTest);
}
