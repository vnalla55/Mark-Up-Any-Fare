#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "test/include/TestMemHandle.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestLogger.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/ArunkSeg.h"
#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Diagnostic/Diag191Collector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);
using boost::assign::operator+=;

class ValidatingCarrierUpdaterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ValidatingCarrierUpdaterTest);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Empty);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_AllTheSame);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_ChangedOnce_OneSegment);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_ChangedOnce_TwoSegments);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_ChangedOnce_ThreeSegments);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_ChangedTwice);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_ChangedTriple);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_WithGaps);

  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence_NoChangeBefore);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence_ChangeBefore);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence_NoChangeInMiddle);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence_ChangeInMiddle);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence_WithGaps);
  CPPUNIT_TEST(testFindCarrierBetweenAreas_Area321Sequence_WithGaps_RealExample);

  CPPUNIT_TEST(testFindCarrierBetweenSubAreas_Empty);
  CPPUNIT_TEST(testFindCarrierBetweenSubAreas_AllTheSame);
  CPPUNIT_TEST(testFindCarrierBetweenSubAreas_ChangedOnce);

  CPPUNIT_TEST(testFindCarrierBetweenCountries_Empty);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_AllTheSame);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_ChangedOnce);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_DomesticUSCA);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_DomesticScandinavia);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_NonDomesticScandinavia);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_Scandinavia_WithOneGap);
  CPPUNIT_TEST(testFindCarrierBetweenCountries_Scandinavia_WithTwoGaps);

  CPPUNIT_TEST(testFind_Empty);
  CPPUNIT_TEST(testFind_AllDomestic);
  CPPUNIT_TEST(testFind_AreaChanged);
  CPPUNIT_TEST(testFind_SubAreaChanged);
  CPPUNIT_TEST(testFind_NationChanged);

  CPPUNIT_TEST(testFilterSegments_Surface);
  CPPUNIT_TEST(testFilterSegments_Industry);
  CPPUNIT_TEST(testFilterSegments_Mixed);

  CPPUNIT_TEST(testUpdateByOverride_NoUpdate);
  CPPUNIT_TEST(testUpdateByOverride_FromRequest);
  CPPUNIT_TEST(testUpdateByOverride_IsItinUpdated);
  CPPUNIT_TEST(testUpdateByOverride_PartitionID);
  CPPUNIT_TEST(testUpdateValidatingCxrList);
  CPPUNIT_TEST(testProcessSops);
  CPPUNIT_TEST(testProcessSopsMultiSp);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierHasNeutralVcx);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierHasNeutralVcxForMultiSp);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxNoSwap);
  CPPUNIT_TEST(testSetDefaultValidatingCxrForCommandPricing);

  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxNoSwapWithMultiSp);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxWithSwap);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierMultipleVcxMarketingCxrEmpty);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxSingleMarketingCxr);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxSingleMarketingCxrWithMultiSp);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxMultipleMarketingCxr);
  CPPUNIT_TEST(testDetermineDefaultValidatingCarrierSingleVcxMultipleMarketingCxrWithMultiSp);
  CPPUNIT_TEST(testFindDefValCxrFromPreferredWithSingleVC);
  CPPUNIT_TEST(testFindDefValCxrFromPreferredWithMultiVC);
  CPPUNIT_TEST(testFindDefValCxrFromPreferredFail);

  CPPUNIT_TEST(testAddNSPInCspi_Pass);
  CPPUNIT_TEST(testAddNSPInCspi_AlreadyExist);
  CPPUNIT_TEST(testPrintNSPDiag191Info_noSMV_noIEV);
  CPPUNIT_TEST(testPrintNSPDiag191Info_noSMV_IEV);
  CPPUNIT_TEST(testPrintNSPDiag191Info_SMV_IEV);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = create<PricingTrx>();
    _trx->setRequest(create<PricingRequest>());
    _updater = _memH(new ValidatingCarrierUpdater(*_trx));
  }

  void tearDown() { _memH.clear(); }

  template <typename T>
  T* create()
  {
    return _memH.create<T>();
  }
  void setPricingRequest(const vcx::Pos& pos)
  {
    Loc* loc = _memH.create<Loc>();
    loc->nation() = pos.country;
    Agent* agent = _memH.create<Agent>();
    agent->agentLocation() = loc;
    agent->cxrCode() = pos.primeHost;
    PricingRequest* request = _memH.create<PricingRequest>();
    request->ticketingAgent() = agent;
    request->electronicTicket() = 'T';
    _trx->setRequest( request );
  }
  void checkDefaultValidatingCarrier(PricingTrx& trx,
      Itin& itin,
      const std::vector<CarrierCode>& validatingCxr,
      bool retVal,
      CarrierCode expValCxr,
      CarrierCode expMktCxr)
  {
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    DefaultValidatingCarrierFinder dvcFinder(trx, itin);
    CPPUNIT_ASSERT(retVal==dvcFinder.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
    if (!expValCxr.empty())
      CPPUNIT_ASSERT(defaultValidatingCxr == expValCxr);
    if (!expMktCxr.empty())
      CPPUNIT_ASSERT(defaultMarketingCxr == expMktCxr);
  }

  Loc* createLoc(const IATAAreaCode& area)
  {
    Loc* loc = create<Loc>();
    loc->area() = area;
    return loc;
  }

  Loc* createLocSub(const IATASubAreaCode& area)
  {
    Loc* loc = createLoc(IATA_AREA1);
    loc->subarea() = area;
    return loc;
  }

  Loc* createLocNat(const NationCode& nation)
  {
    Loc* loc = createLocSub(IATA_SUB_AREA_11());
    loc->nation() = nation;
    return loc;
  }

  AirSeg* createAirSeg(const CarrierCode& carrier, const Loc* origin = 0, const Loc* dest = 0)
  {
    AirSeg* seg = create<AirSeg>();
    seg->carrier() = carrier;
    seg->origin() = origin;
    seg->destination() = dest;
    return seg;
  }

  typedef ValidatingCarrierUpdater::AirSegmentVec AirSegmentVec;

  template <typename T, int size, typename C>
  AirSegmentVec createSegs(T (&init)[size], C locCreator, T skip = T())
  {
    CPPUNIT_ASSERT(size > 1);

    AirSegmentVec segments;
    for (int i = 1; i < size; ++i)
    {
      if (init[i - 1] == skip || init[i] == skip)
        continue;
      segments += createAirSeg(std::string(2, char(64 + i)),
                               (this->*locCreator)(init[i - 1]),
                               (this->*locCreator)(init[i]));
    }
    return segments;
  }

  void testFindCarrierBetweenAreas_Empty()
  {
    AirSegmentVec segments;

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenAreas_AllTheSame()
  {
    IATAAreaCode areas[] = { IATA_AREA2, IATA_AREA2, IATA_AREA2, IATA_AREA2 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenAreas_ChangedOnce_OneSegment()
  {
    IATAAreaCode areas[] = { IATA_AREA1, IATA_AREA2 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), result);
  }

  void testFindCarrierBetweenAreas_ChangedOnce_TwoSegments()
  {
    IATAAreaCode areas[] = { IATA_AREA1, IATA_AREA1, IATA_AREA2 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenAreas_ChangedOnce_ThreeSegments()
  {
    IATAAreaCode areas[] = { IATA_AREA1, IATA_AREA1, IATA_AREA1, IATA_AREA2 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), result);
  }

  void testFindCarrierBetweenAreas_ChangedTwice()
  {
    IATAAreaCode areas[] = { IATA_AREA1, IATA_AREA1, IATA_AREA2, IATA_AREA3, IATA_AREA3 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenAreas_ChangedTriple()
  {
    IATAAreaCode areas[] = {
      IATA_AREA1, IATA_AREA1, IATA_AREA2, IATA_AREA3, IATA_AREA2, IATA_AREA2
    };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenAreas_WithGaps()
  {
    IATAAreaCode surface = "SURFACE";
    IATAAreaCode areas[] = { IATA_AREA3, IATA_AREA3, surface, IATA_AREA2, IATA_AREA2, surface,
                             IATA_AREA1, IATA_AREA1, surface, IATA_AREA3, IATA_AREA2 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc, surface);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("JJ"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence()
  {
    IATAAreaCode areas[] = { IATA_AREA3, IATA_AREA2, IATA_AREA1, IATA_AREA1 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence_NoChangeBefore()
  {
    IATAAreaCode areas[] = { IATA_AREA3, IATA_AREA3, IATA_AREA3, IATA_AREA2, IATA_AREA1 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("DD"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence_ChangeBefore()
  {
    IATAAreaCode areas[] = {
      IATA_AREA3, IATA_AREA3, IATA_AREA1, IATA_AREA3, IATA_AREA2, IATA_AREA1
    };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence_NoChangeInMiddle()
  {
    IATAAreaCode areas[] = { IATA_AREA3, IATA_AREA2, IATA_AREA2, IATA_AREA2, IATA_AREA1 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("DD"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence_ChangeInMiddle()
  {
    IATAAreaCode areas[] = {
      IATA_AREA3, IATA_AREA2, IATA_AREA3, IATA_AREA1, IATA_AREA2, IATA_AREA1
    };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence_WithGaps()
  {
    IATAAreaCode surface = "SURFACE";
    IATAAreaCode areas[] = { IATA_AREA3, IATA_AREA3, surface, IATA_AREA2, IATA_AREA2, surface,
                             IATA_AREA3, IATA_AREA2, surface, IATA_AREA2, IATA_AREA1 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc, surface);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("JJ"), result);
  }

  void testFindCarrierBetweenAreas_Area321Sequence_WithGaps_RealExample()
  {
    IATAAreaCode surface = "SURFACE";
    IATAAreaCode areas[] = { IATA_AREA3, IATA_AREA2, surface,    IATA_AREA1,
                             IATA_AREA1, surface,    IATA_AREA2, IATA_AREA1 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc, surface);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("GG"), result);
  }

  void testFindCarrierBetweenSubAreas_Empty()
  {
    IATASubAreaCode areas[] = {
        IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_11()};
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLocSub);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenSubAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenSubAreas_AllTheSame()
  {
    IATASubAreaCode areas[] = {
        IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_11()};
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLocSub);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenSubAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenSubAreas_ChangedOnce()
  {
    IATASubAreaCode areas[] = {
        IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_12(), IATA_SUB_AREA_12()};
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLocSub);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenSubAreas(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenCountries_Empty()
  {
    AirSegmentVec segments;

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenCountries_AllTheSame()
  {
    NationCode nations[] = { UNITED_STATES, UNITED_STATES, UNITED_STATES, UNITED_STATES };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenCountries_ChangedOnce()
  {
    NationCode nations[] = { UNITED_STATES, UNITED_STATES, MEXICO, MEXICO };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenCountries_DomesticUSCA()
  {
    NationCode nations[] = { UNITED_STATES, CANADA, CANADA, UNITED_STATES };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(!_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(BAD_CARRIER), result);
  }

  void testFindCarrierBetweenCountries_DomesticScandinavia()
  {
    NationCode nations[] = { DENMARK, DENMARK, NORWAY, SWEDEN };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BB"), result);
  }

  void testFindCarrierBetweenCountries_NonDomesticScandinavia()
  {
    NationCode nations[] = { DENMARK, NORWAY, SWEDEN, GERMANY, SWEDEN };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), result);
  }

  void testFindCarrierBetweenCountries_Scandinavia_WithOneGap()
  {
    NationCode surface = "SF";
    NationCode nations[] = { DENMARK, DENMARK, surface, NORWAY, NORWAY, DENMARK };
    AirSegmentVec segments =
        createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat, surface);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("EE"), result);
  }

  void testFindCarrierBetweenCountries_Scandinavia_WithTwoGaps()
  {
    NationCode surface = "SF";
    NationCode nations[] = { DENMARK, DENMARK, surface, NORWAY, NORWAY, surface, SWEDEN, DENMARK };
    AirSegmentVec segments =
        createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat, surface);

    CarrierCode result(BAD_CARRIER);
    CPPUNIT_ASSERT(_updater->findCarrierBetweenCountries(segments, result));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("GG"), result);
  }

  void testFind_Empty()
  {
    AirSegmentVec segments;
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), _updater->find(segments));
  }

  void testFind_AllDomestic()
  {
    NationCode nations[] = { CANADA, CANADA, CANADA };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), _updater->find(segments));
  }

  void testFind_AreaChanged()
  {
    IATAAreaCode areas[] = { IATA_AREA1, IATA_AREA1, IATA_AREA1, IATA_AREA2 };
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLoc);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), _updater->find(segments));
  }

  void testFind_SubAreaChanged()
  {
    IATASubAreaCode areas[] = {
        IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_11(), IATA_SUB_AREA_12()};
    AirSegmentVec segments = createSegs(areas, &ValidatingCarrierUpdaterTest::createLocSub);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), _updater->find(segments));
  }

  void testFind_NationChanged()
  {
    NationCode nations[] = { UNITED_STATES, UNITED_STATES, UNITED_STATES, MEXICO };
    AirSegmentVec segments = createSegs(nations, &ValidatingCarrierUpdaterTest::createLocNat);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), _updater->find(segments));
  }

  template <int size>
  void populate(TravelSeg* (&travel)[size], AirSegmentVec& expect, std::vector<TravelSeg*>& source)
  {
    std::vector<AirSeg*> air;
    for (unsigned i = 0; i < size; ++i)
      air += createAirSeg(std::string(2, char(64 + i)));

    expect.assign(air.begin(), air.end());

    for (unsigned i = 0; i < size; ++i)
      source += air[i], travel[i];
  }

  void testFilterSegments_Surface()
  {
    TravelSeg* surface[] = { create<ArunkSeg>(), create<ArunkSeg>(), create<ArunkSeg>(),
                             create<ArunkSeg>() };
    AirSegmentVec result, expect;
    std::vector<TravelSeg*> source;
    populate(surface, expect, source);

    _updater->filterSegments(source, result);

    CPPUNIT_ASSERT(expect == result);
  }

  void testFilterSegments_Industry()
  {
    TravelSeg* industry[] = { createAirSeg("YY"), createAirSeg("YY"), createAirSeg("YY"),
                              createAirSeg("YY") };
    AirSegmentVec result, expect;
    std::vector<TravelSeg*> source;
    populate(industry, expect, source);

    _updater->filterSegments(source, result);

    CPPUNIT_ASSERT(expect == result);
  }

  void testFilterSegments_Mixed()
  {
    TravelSeg* mixed[] = { create<ArunkSeg>(), createAirSeg("YY"), createAirSeg("YY"),
                           create<ArunkSeg>() };
    AirSegmentVec result, expect;
    std::vector<TravelSeg*> source;
    populate(mixed, expect, source);

    _updater->filterSegments(source, result);

    CPPUNIT_ASSERT(expect == result);
  }

  void populateTrx(const std::string& validatingCarrier, const std::string& partitionID = "")
  {
    _trx->getRequest()->validatingCarrier() = validatingCarrier;

    _trx->billing() = create<Billing>();
    _trx->billing()->partitionID() = partitionID;

    tse::ConfigMan config;
    config.setValue("MCP_PRIME_HOSTS", partitionID, "XFORMS_MCP");
    MCPCarrierUtil::initialize(config);
  }

  void testUpdateByOverride_NoUpdate()
  {
    populateTrx("");
    Itin& itin = *create<Itin>();

    CPPUNIT_ASSERT(!_updater->updateByOverride(itin));
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), itin.validatingCarrier());
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), _trx->getRequest()->validatingCarrier());
  }

  void testUpdateByOverride_FromRequest()
  {
    populateTrx(CARRIER_9B);
    Itin& itin = *create<Itin>();

    CPPUNIT_ASSERT(_updater->updateByOverride(itin));
    CPPUNIT_ASSERT_EQUAL(CARRIER_9B, itin.validatingCarrier());
    CPPUNIT_ASSERT_EQUAL(CARRIER_9B, _trx->getRequest()->validatingCarrier());
  }

  void testUpdateByOverride_IsItinUpdated()
  {
    populateTrx(CARRIER_9B);
    Itin& itin = *create<Itin>();
    itin.validatingCarrier() = CARRIER_2R;

    CPPUNIT_ASSERT(_updater->updateByOverride(itin));
    CPPUNIT_ASSERT_EQUAL(CARRIER_2R, itin.validatingCarrier());
    CPPUNIT_ASSERT_EQUAL(CARRIER_9B, _trx->getRequest()->validatingCarrier());
  }

  void testUpdateByOverride_PartitionID()
  {
    populateTrx("", CARRIER_9B);
    Itin& itin = *create<Itin>();
    itin.validatingCarrier() = CARRIER_2R;

    CPPUNIT_ASSERT(_updater->updateByOverride(itin));
    CPPUNIT_ASSERT_EQUAL(CARRIER_9B, itin.validatingCarrier());
    CPPUNIT_ASSERT_EQUAL(CARRIER_9B, _trx->getRequest()->validatingCarrier());
  }

  void testUpdateValidatingCxrList()
  {
     try
     {
       Itin itin ;
       populateTrx("IB");

       AirSeg airSeg1, airSeg2;
       airSeg1.setMarketingCarrierCode("AA");
       airSeg1.setOperatingCarrierCode("AA");
       airSeg2.setMarketingCarrierCode("PG");
       airSeg2.setOperatingCarrierCode("PG");

       std::vector<TravelSeg*> tSegs;
       tSegs.push_back(&airSeg1);
       tSegs.push_back(&airSeg2);
       itin.travelSeg() = tSegs;

       CountrySettlementPlanInfo cspi;
       cspi.setCountryCode("US");
       _trx->countrySettlementPlanInfo() = &cspi;

       Agent ta;
       _trx->getRequest()->ticketingAgent() = &ta;
       std::string hashString = "AAPG|AAPG";

       _updater->updateValidatingCxrList(itin, 0);

     }
     catch (const ErrorResponseException& e)
     {
       if (e.code() ==  ErrorResponseException::VALIDATING_CXR_ERROR);
       {
          CPPUNIT_ASSERT(1);
          return;
       }
     }
     CPPUNIT_ASSERT(0);
  }

  AirSeg* buildSegment(std::string origin,
                       std::string destination,
                       std::string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime(),
                       bool stopOver = false)
  {
    AirSeg* airSeg;
    _dataHandle.get(airSeg);

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    Loc* locorig, *locdest;
    _dataHandle.get(locorig);
    _dataHandle.get(locdest);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(carrier);
    airSeg->boardMultiCity() = locorig->loc();
    airSeg->offMultiCity() = locdest->loc();
    airSeg->stopOver() = stopOver;

    return airSeg;
  }


  void setupOneLegSops(ShoppingTrx& trx)
  {
    Itin* itn(0);

    trx.legs().push_back(ShoppingTrx::Leg());
    size_t leg(trx.legs().size() - 1);
    trx.schedulingOptionIndices().resize(leg + 1);
    trx.indicesToSchedulingOption().resize(leg + 1);

    uint32_t originalSopId = 0;

    AirSeg* airSeg1 = buildSegment("JFK", "DFW", "LH");

    _dataHandle.get(itn);
    itn->travelSeg() += airSeg1;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    ++originalSopId;
    airSeg1 = buildSegment("JFK", "DFW", "DL");

    _dataHandle.get(itn);
    itn->travelSeg() += airSeg1;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    ++originalSopId;
    airSeg1 = buildSegment("JFK", "WAS", "DL");

    _dataHandle.get(itn);
    itn->travelSeg() += airSeg1;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    ++originalSopId;
    airSeg1 = buildSegment("JFK", "WAS", "AA");

    _dataHandle.get(itn);
    itn->travelSeg() += airSeg1;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    CPPUNIT_ASSERT_EQUAL(trx.legs().size(), (size_t)1);
    CPPUNIT_ASSERT_EQUAL(trx.legs()[leg].sop().size(), (size_t)4);
  }

  void testProcessSops()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    ShoppingTrx* trx(0);
    _dataHandle.get(trx);

    setupOneLegSops(*trx);
    SopIdVec sops = {0};

    Loc* loc = _memH.create<Loc>();
    loc->nation() = "US";
    Agent* agent = _memH.create<Agent>();
    agent->agentLocation() = loc;
    agent->cxrCode() = "1S";
    PricingRequest* request = _memH.create<PricingRequest>();
    request->ticketingAgent() = agent;
    request->electronicTicket() = 'T';
    trx->setRequest(request);
    trx->setValidatingCxrGsaApplicable(true);

    CountrySettlementPlanInfo cspi;
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setCountryCode("US");
    cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    trx->countrySettlementPlanInfo() = &cspi;
    trx->setTrxType(PricingTrx::IS_TRX);

    ValidatingCarrierUpdater updater(*trx);

    CPPUNIT_ASSERT(!updater.processSops(*trx, sops));

    ValidatingCxrGSAData vcgd;
    trx->validatingCxrHashMap()["LH|"] = &vcgd;
    CPPUNIT_ASSERT(updater.processSops(*trx, sops));
  }

  void testProcessSopsMultiSp()
  {
    ShoppingTrx* trx(0);
    _dataHandle.get(trx);

    setupOneLegSops(*trx);
    SopIdVec sops = {0};

    Loc* loc = _memH.create<Loc>();
    loc->nation() = "US";
    Agent* agent = _memH.create<Agent>();
    agent->agentLocation() = loc;
    agent->cxrCode() = "1S";
    PricingRequest* request = _memH.create<PricingRequest>();
    request->ticketingAgent() = agent;
    request->electronicTicket() = 'T';
    trx->setRequest(request);
    trx->setValidatingCxrGsaApplicable(true);

    CountrySettlementPlanInfo cspi;
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setCountryCode("US");
    cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    trx->addCountrySettlementPlanInfo(&cspi);
    trx->setTrxType(PricingTrx::IS_TRX);

    ValidatingCarrierUpdater updater(*trx);

    CPPUNIT_ASSERT(!updater.processSops(*trx, sops));

    ValidatingCxrGSAData vcgd;
    //trx->validatingCxrHashMap()["LH|"] = &vcgd;
    SpValidatingCxrGSADataMap spValCxrGsaDataMap;
    spValCxrGsaDataMap["BSP"]=&vcgd;
    trx->hashSpValidatingCxrGsaDataMap()["LH|"] = &spValCxrGsaDataMap;
    bool res = updater.processSops(*trx, sops);
    CPPUNIT_ASSERT(res);
  }

  void testDetermineDefaultValidatingCarrierHasNeutralVcx()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;
    validatingCxrGSAData.isNeutralValCxr() = true;

    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    std::vector<CarrierCode> validatingCxr;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    validatingCxr.push_back("AA");

    ValidatingCarrierUpdater updater(*_trx);
    CPPUNIT_ASSERT(updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");

    validatingCxr.push_back("BB");

    CPPUNIT_ASSERT(!updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  }

  void testDetermineDefaultValidatingCarrierHasNeutralVcxForMultiSp()
  {
    CountrySettlementPlanInfo cspi;
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setCountryCode("US");
    cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    _trx->addCountrySettlementPlanInfo(&cspi);

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData valCxrGsaData;

    valCxrGsaData.validatingCarriersData()["AA"] = v;
    valCxrGsaData.isNeutralValCxr() = true;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &valCxrGsaData;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    std::vector<CarrierCode> valCxrs;
    valCxrs.push_back("AA");

    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    DefaultValidatingCarrierFinder dvcFinder(*_trx, itin);
    CPPUNIT_ASSERT(true == dvcFinder.determineDefaultValidatingCarrier(
          valCxrs,
          defaultValidatingCxr,
          defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
    CPPUNIT_ASSERT(defaultMarketingCxr == "");

    valCxrGsaData.validatingCarriersData()["BB"] = v;
    valCxrs.push_back("BB");

    defaultValidatingCxr=defaultMarketingCxr="";
    CPPUNIT_ASSERT(false == dvcFinder.determineDefaultValidatingCarrier(
          valCxrs,
          defaultValidatingCxr,
          defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "");
    CPPUNIT_ASSERT(defaultMarketingCxr == "");
  }

  void testSetDefaultValidatingCxrForCommandPricing()
  {
    CountrySettlementPlanInfo cspi1, cspi2;
    cspi1.setSettlementPlanTypeCode("BSP");
    cspi1.setCountryCode("CA");
    cspi1.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    cspi2.setSettlementPlanTypeCode("GEN");
    cspi2.setCountryCode("CA");
    cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);

    _trx->addCountrySettlementPlanInfo(&cspi1);
    _trx->addCountrySettlementPlanInfo(&cspi2);

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData valCxrGsaData1, valCxrGsaData2;

    valCxrGsaData1.validatingCarriersData()["AA"] = v;
    valCxrGsaData1.validatingCarriersData()["BA"] = v;
    valCxrGsaData1.isNeutralValCxr() = true;

    valCxrGsaData2.validatingCarriersData()["BA"] = v;
    valCxrGsaData2.validatingCarriersData()["UA"] = v;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &valCxrGsaData1;
    spGsaDataMap["GEN"] = &valCxrGsaData2;

    const SettlementPlanType primarySp = "BSP";

    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1, airSeg2, airSeg3;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("BA");
    airSeg3.setMarketingCarrierCode("UA");

    airSeg1.origin()=createLoc(IATA_AREA1);
    airSeg1.destination()=createLoc(IATA_AREA1);

    airSeg2.origin()=createLoc(IATA_AREA1);
    airSeg2.destination()=createLoc(IATA_AREA1);

    airSeg3.origin()=createLoc(IATA_AREA1);
    airSeg3.destination()=createLoc(IATA_AREA1);

    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);

    Itin itin;
    itin.travelSeg() = tSegs;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    CarrierCode defValCxr, defMktCxr;
    CPPUNIT_ASSERT(_updater->setDefaultValidatingCxrForCommandPricing(itin,
          primarySp,
          defValCxr,
          defMktCxr));
    CPPUNIT_ASSERT(defValCxr == "BA");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxNoSwap()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;
    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    std::vector<CarrierCode> validatingCxr;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    validatingCxr.push_back("AA");

    ValidatingCarrierUpdater updater(*_trx);
    CPPUNIT_ASSERT(updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxNoSwapWithMultiSp()
  {
    CountrySettlementPlanInfo cspi2;
    cspi2.setSettlementPlanTypeCode("BSP");
    cspi2.setCountryCode("US");
    cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    _trx->addCountrySettlementPlanInfo(&cspi2);

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;
    validatingCxrGSAData.validatingCarriersData()["AA"] = v;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &validatingCxrGSAData;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    std::vector<CarrierCode> valCxrs;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    valCxrs.push_back("AA");

    DefaultValidatingCarrierFinder dvcFinder(*_trx, itin);
    CPPUNIT_ASSERT(true == dvcFinder.determineDefaultValidatingCarrier(
          valCxrs,
          defaultValidatingCxr,
          defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
    CPPUNIT_ASSERT(defaultMarketingCxr == "");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxWithSwap()
  {
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;
    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    std::vector<CarrierCode> validatingCxr;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    validatingCxr.push_back("AA");

    ValidatingCarrierUpdater updater(*_trx);
    itin.validatingCxrGsaData()->gsaSwapMap()["DL"].insert("G2");
    CPPUNIT_ASSERT(!updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));

    checkDefaultValidatingCarrier(*_trx, itin, validatingCxr, false, "", "");
  }

  void testDetermineDefaultValidatingCarrierMultipleVcxMarketingCxrEmpty()
  {
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;
    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    std::vector<CarrierCode> validatingCxr;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    validatingCxr.push_back("AA");
    validatingCxr.push_back("BB");

    ValidatingCarrierUpdater updater(*_trx);
    CPPUNIT_ASSERT(!updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));

    checkDefaultValidatingCarrier(*_trx, itin, validatingCxr, false, "", "");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxSingleMarketingCxr()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;
    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    std::vector<CarrierCode> validatingCxr;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    validatingCxr.push_back("AA");

    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1;

    airSeg1.setMarketingCarrierCode("AA");

    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;
    itin.validatingCxrGsaData()->gsaSwapMap()["DL"].insert("G2");

    ValidatingCarrierUpdater updater(*_trx);
    CPPUNIT_ASSERT(updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
    CPPUNIT_ASSERT(defaultMarketingCxr == "AA");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxSingleMarketingCxrWithMultiSp()
  {
    CountrySettlementPlanInfo cspi;
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setCountryCode("US");
    cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    _trx->addCountrySettlementPlanInfo(&cspi);

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData valCxrGsaData;
    valCxrGsaData.validatingCarriersData()["AA"] = v;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &valCxrGsaData;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    std::vector<CarrierCode> valCxrs;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    valCxrs.push_back("AA");

    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1;

    airSeg1.setMarketingCarrierCode("AA");

    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;
    valCxrGsaData.gsaSwapMap()["DL"].insert("G2");

    DefaultValidatingCarrierFinder dvcFinder(*_trx, itin);
    CPPUNIT_ASSERT(true == dvcFinder.determineDefaultValidatingCarrier(
          valCxrs,
          defaultValidatingCxr,
          defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
    CPPUNIT_ASSERT(defaultMarketingCxr == "AA");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxMultipleMarketingCxr()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;
    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    std::vector<CarrierCode> validatingCxr;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    validatingCxr.push_back("AA");
    validatingCxr.push_back("DL");

    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1, airSeg2, airSeg3;

    airSeg1.setMarketingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("DL");
    airSeg3.setMarketingCarrierCode("ML");

    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;
    itin.validatingCxrGsaData()->gsaSwapMap()["DL"].insert("G2");

    ValidatingCarrierUpdater updater(*_trx);
    CPPUNIT_ASSERT(updater.determineDefaultValidatingCarrier(itin,validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
    CPPUNIT_ASSERT(defaultMarketingCxr == "AA");
  }

  void testDetermineDefaultValidatingCarrierSingleVcxMultipleMarketingCxrWithMultiSp()
  {
    CountrySettlementPlanInfo cspi2;
    cspi2.setSettlementPlanTypeCode("BSP");
    cspi2.setCountryCode("US");
    cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    _trx->addCountrySettlementPlanInfo(&cspi2);

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;
    validatingCxrGSAData.validatingCarriersData()["AA"] = v;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &validatingCxrGSAData;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    std::vector<CarrierCode> valCxrs;
    CarrierCode defaultValidatingCxr, defaultMarketingCxr;
    valCxrs.push_back("AA");
    valCxrs.push_back("DL");

    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1, airSeg2, airSeg3;

    airSeg1.setMarketingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("DL");
    airSeg3.setMarketingCarrierCode("ML");

    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;
    validatingCxrGSAData.gsaSwapMap()["DL"].insert("G2");

    DefaultValidatingCarrierFinder dvcFinder(*_trx, itin);
    CPPUNIT_ASSERT(true == dvcFinder.determineDefaultValidatingCarrier(
          valCxrs,
          defaultValidatingCxr,
          defaultMarketingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
    CPPUNIT_ASSERT(defaultMarketingCxr == "AA");
  }

  void testFindDefValCxrFromPreferredWithSingleVC()
  {
    std::vector<CarrierCode> validatingCxr;
    std::vector<CarrierCode> preferredValidatingCxr;
    CarrierCode defaultValidatingCxr;

    validatingCxr.push_back("LH");
    validatingCxr.push_back("NW");
    validatingCxr.push_back("AA");

    preferredValidatingCxr.push_back("NW");

    ValidatingCarrierUpdater vcu(*_trx);
    CPPUNIT_ASSERT(true == vcu.findDefValCxrFromPreferred(validatingCxr, preferredValidatingCxr, defaultValidatingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "NW");

  }

  void testFindDefValCxrFromPreferredWithMultiVC()
  {
    std::vector<CarrierCode> validatingCxr;
    std::vector<CarrierCode> preferredValidatingCxr;
    CarrierCode defaultValidatingCxr;

    validatingCxr.push_back("LH");
    validatingCxr.push_back("NW");
    validatingCxr.push_back("AA");

    preferredValidatingCxr.push_back("BA");
    preferredValidatingCxr.push_back("AA");

    ValidatingCarrierUpdater vcu(*_trx);
    CPPUNIT_ASSERT(true == vcu.findDefValCxrFromPreferred(validatingCxr, preferredValidatingCxr, defaultValidatingCxr));
    CPPUNIT_ASSERT(defaultValidatingCxr == "AA");
  }

  void testFindDefValCxrFromPreferredFail()
  {
    std::vector<CarrierCode> validatingCxr;
    std::vector<CarrierCode> preferredValidatingCxr;
    CarrierCode defaultValidatingCxr;

    validatingCxr.push_back("LH");
    validatingCxr.push_back("NW");
    validatingCxr.push_back("AA");

    preferredValidatingCxr.push_back("BA");

    ValidatingCarrierUpdater vcu(*_trx);
    CPPUNIT_ASSERT(false == vcu.findDefValCxrFromPreferred(validatingCxr, preferredValidatingCxr, defaultValidatingCxr));
    CPPUNIT_ASSERT(true == defaultValidatingCxr.empty());
  }

  void testAddNSPInCspi_Pass()
  {
    CountrySettlementPlanInfo cspi1, cspi2;
    cspi1.setSettlementPlanTypeCode("BSP");
    cspi1.setCountryCode("CA");
    cspi1.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    cspi2.setSettlementPlanTypeCode("GEN");
    cspi2.setCountryCode("CA");
    cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);

    _trx->addCountrySettlementPlanInfo(&cspi1);
    _trx->addCountrySettlementPlanInfo(&cspi2);


    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    ValidatingCarrierUpdater vcxrUpdater(*_trx);
    vcxrUpdater.addNSPInCspi();
    bool testResult = false;
    const std::vector<CountrySettlementPlanInfo*>& cspInfos = _trx->countrySettlementPlanInfos();
    for (const CountrySettlementPlanInfo* cspInfo : cspInfos)
    {
      if(cspInfo->getSettlementPlanTypeCode() == "NSP")
      {
        testResult = true;
        break;
      }
    }

    CPPUNIT_ASSERT(testResult == true);
    CPPUNIT_ASSERT((uint16_t)cspInfos.size() == (uint16_t)3);
  }

  void testAddNSPInCspi_AlreadyExist()
  {
    CountrySettlementPlanInfo cspi1, cspi2;
    cspi1.setSettlementPlanTypeCode("BSP");
    cspi1.setCountryCode("CA");
    cspi1.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    cspi2.setSettlementPlanTypeCode("NSP");
    cspi2.setCountryCode("CA");
    cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);

    _trx->addCountrySettlementPlanInfo(&cspi1);
    _trx->addCountrySettlementPlanInfo(&cspi2);

    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    ValidatingCarrierUpdater vcxrUpdater(*_trx);
    vcxrUpdater.addNSPInCspi();
    bool testResult = false;
    const std::vector<CountrySettlementPlanInfo*>& cspInfos = _trx->countrySettlementPlanInfos();
    for (const CountrySettlementPlanInfo* cspInfo : cspInfos)
    {
      if(cspInfo->getSettlementPlanTypeCode() == "NSP")
      {
        testResult = true;
        break;
      }
    }

    CPPUNIT_ASSERT(testResult == true);
    CPPUNIT_ASSERT((uint16_t)cspInfos.size() == (uint16_t)2);

  }
  void testPrintNSPDiag191Info_noSMV_noIEV()
  {
    vcx::Pos pos( "US", "1S" );
      setPricingRequest( pos );

    Diag191Collector diag;
    diag.activate();

    _trx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;
    _trx->getRequest()->spvCxrsCode().push_back("AA");
    _trx->getRequest()->spvCxrsCode().push_back("DL");

    ValidatingCarrierUpdater vcxrUpdater(*_trx);
    vcxrUpdater.printNSPDiag191Info(&diag);

    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    F"));
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "NSP CARRIERS:    AA,DL"));

  }
  void testPrintNSPDiag191Info_noSMV_IEV()
    {
      vcx::Pos pos( "US", "1S" );
      setPricingRequest( pos );

      Diag191Collector diag;
      diag.activate();

      _trx->getRequest()->spvInd() = tse::spValidator::noSMV_IEV;

      _trx->getRequest()->spvCxrsCode().push_back("AA");
      _trx->getRequest()->spvCxrsCode().push_back("DL");

      _trx->getRequest()->spvCntyCode().push_back("AU");
      _trx->getRequest()->spvCntyCode().push_back("FR");

      ValidatingCarrierUpdater vcxrUpdater(*_trx);
      vcxrUpdater.printNSPDiag191Info(&diag);

      CPPUNIT_ASSERT(strstr(diag.str().c_str(), "SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    T"));
      CPPUNIT_ASSERT(strstr(diag.str().c_str(), "NSP CARRIERS:    AA,DL"));
      CPPUNIT_ASSERT(strstr(diag.str().c_str(), "NSP COUNTRIES:    AU,FR"));

    }

  void testPrintNSPDiag191Info_SMV_IEV()
  {
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    Diag191Collector diag;
    diag.activate();

    ValidatingCarrierUpdater vcxrUpdater(*_trx);
    vcxrUpdater.printNSPDiag191Info(&diag);

    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "SETTLEMENT PLAN VALIDATION:    T, IET VALIDATION:    T"));

  }

private:
  PricingTrx* _trx;
  DataHandle _dataHandle;
  TestMemHandle _memH;
  ValidatingCarrierUpdater* _updater;
  RootLoggerGetOff off;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValidatingCarrierUpdaterTest);

} // tse
