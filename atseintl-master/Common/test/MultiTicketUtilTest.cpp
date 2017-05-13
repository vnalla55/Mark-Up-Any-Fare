#include <vector>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "Common/MultiTicketUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DiskCache.h"
#include "Diagnostic/Diag676Collector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class MultiTicketUtilTest: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MultiTicketUtilTest);
  CPPUNIT_TEST(testCreateDiag676);
  CPPUNIT_TEST(testFinishDiag676);
  CPPUNIT_TEST(testFindStopOversWhenDomesticLessThan4Hrs);
  CPPUNIT_TEST(testFindStopOversWhenDomesticExactly4Hrs);
  CPPUNIT_TEST(testFindStopOversWhenDomesticOver4Hrs);
  CPPUNIT_TEST(testFindStopOversWhenInternationalLessThan24Hrs);
  CPPUNIT_TEST(testFindStopOversWhenInternationalExactly24Hrs);
  CPPUNIT_TEST(testFindStopOversWhenInternationalOver24Hrs);
  CPPUNIT_TEST(testFindStopOversWhenArunkWithInternationalLessThan24Hrs);
  CPPUNIT_TEST(testFindStopOversWhenForeignDomesticOver24Hrs);
  CPPUNIT_TEST(testFindStopOversWhen2StopOvers);
  CPPUNIT_TEST(testFindStopOversWhen1Connection1StopOver);
  CPPUNIT_TEST(testFindStopOversWhenForcedStopOver);
  CPPUNIT_TEST(testFindStopOversWhenForcedConnection);
  CPPUNIT_TEST(testFindStopOversWhenArunkAfterStopOver);
  CPPUNIT_TEST(testIsMarriedSegReturnFalse);
  CPPUNIT_TEST(testIsMarriedSegReturnTrue);
  CPPUNIT_TEST(testFindStopOversWhenMarriedSegOver4Hrs);
  CPPUNIT_TEST(testFindStopOversWhenMarriedSegWithStopOver);
  CPPUNIT_TEST(testCreateMultiTicketItinsWhenNoStopOver);
  CPPUNIT_SKIP_TEST(testCreateMultiTicketItinsWhenOneStopOver);
  CPPUNIT_TEST(testCreateMultiTicketItinsWhenTwoStopOvers);
  CPPUNIT_TEST(testValidateValidatingCxrDataWhenSubItinsHaveNoGSA);
  CPPUNIT_TEST(testValidateValidatingCxrDataWhenSubItinsHaveGSA);
  CPPUNIT_TEST(testValidateTicketingAgreementWhenOnlySubItin1HasGSA);
  CPPUNIT_TEST(testIsSolutionFoundReturnFalseWhenNoFullItin);
  CPPUNIT_TEST(testIsSolutionFoundReturnFalseWhenNoMultiTicketMap);
  CPPUNIT_TEST(testIsSolutionFoundReturnTrueWhenAllItinsHaveFarePaths);
  CPPUNIT_TEST(testIsSolutionFoundReturnFalseAndSubItinsCleanedUpWhenOneSubItinHasNoFarePath);
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

void testCreateDiag676()
{
  _trx->diagnostic().diagnosticType() = Diagnostic676;
  _trx->diagnostic().activate();
  _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "ITIN"));
  _mtu->createDiag676();
  CPPUNIT_ASSERT_EQUAL(std::string("\n*****************   MULTI-TICKET ANALYSIS   *****************\n"),
                       _mtu->_diag676->str());
}

void testFinishDiag676()
{
  _trx->diagnostic().diagnosticType() = Diagnostic676;
  _trx->diagnostic().activate();
  _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "ITIN"));
  _mtu->finishDiag676();
  CPPUNIT_ASSERT(!_mtu->_diag676);
}

void testFindStopOversWhenDomesticLessThan4Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 5, 0, 0); // Jan 18, 2014 5am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==0);
}

void testFindStopOversWhenDomesticExactly4Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 7, 0, 0); // Jan 18, 2014 7am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==0);
}

void testFindStopOversWhenDomesticOver4Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 7, 1, 0); // Jan 18, 2014 7:01am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testFindStopOversWhenInternationalLessThan24Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);    // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 10, 0, 0); // Jan 18, 2014 10am
  seg2.geoTravelType() = GeoTravelType::International;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==0);
}

void testFindStopOversWhenInternationalExactly24Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 19, 3, 0, 0); // Jan 19, 2014 3am
  seg2.geoTravelType() = GeoTravelType::International;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==0);
}

void testFindStopOversWhenInternationalOver24Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);    // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 19, 10, 0, 0); // Jan 19, 2014 10am
  seg2.geoTravelType() = GeoTravelType::International;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testFindStopOversWhenArunkWithInternationalLessThan24Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2, seg3;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0); // Jan 18, 2014 3am
  seg2.segmentType() = Arunk;
  seg3.departureDT() = DateTime(2014, 1, 18, 10, 0, 0); // Jan 18, 2014 10am
  seg3.geoTravelType() = GeoTravelType::International;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  tvlSegs.push_back(&seg3);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testFindStopOversWhenForeignDomesticOver24Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);    // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 19, 10, 0, 0); // Jan 19, 2014 10am
  seg2.geoTravelType() = GeoTravelType::ForeignDomestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testFindStopOversWhen2StopOvers()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2, seg3;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0); // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 7, 5, 0); // Jan 18, 2014 7:05am
  seg2.arrivalDT() = DateTime(2014, 1, 18, 8, 0, 0); // Jan 18, 2014 8am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  seg3.departureDT() = DateTime(2014, 1, 18, 13, 0, 0); // Jan 18, 2014 1pm
  seg3.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  tvlSegs.push_back(&seg3);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==2);
}

void testFindStopOversWhen1Connection1StopOver()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2, seg3;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0); // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 5, 0, 0); // Jan 18, 2014 5am
  seg2.arrivalDT() = DateTime(2014, 1, 18, 8, 0, 0); // Jan 18, 2014 8am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  seg3.departureDT() = DateTime(2014, 1, 18, 13, 0, 0); // Jan 18, 2014 1pm
  seg3.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  tvlSegs.push_back(&seg3);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testFindStopOversWhenForcedStopOver()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg1.forcedStopOver() = 'T';
  seg2.departureDT() = DateTime(2014, 1, 18, 5, 0, 0); // Jan 18, 2014 5am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testFindStopOversWhenForcedConnection()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg1.forcedConx() = 'T';
  seg2.departureDT() = DateTime(2014, 1, 18, 7, 1, 0); // Jan 18, 2014 7:01am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==0);
}

void testFindStopOversWhenArunkAfterStopOver()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2, seg3, seg4;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0); // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 8, 0, 0); // Jan 18, 2014 8am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  seg3.segmentType() = Arunk;
  seg3.geoTravelType() = GeoTravelType::Domestic;
  seg4.departureDT() = DateTime(2014, 1, 18, 11, 0, 0); // Jan 18, 2014 11am
  seg4.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  tvlSegs.push_back(&seg3);
  tvlSegs.push_back(&seg4);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==2);
}

void testIsMarriedSegReturnFalse()
{
  AirSeg seg;
  seg.marriageStatus() = AirSeg::MARRIAGE_NONE;
  CPPUNIT_ASSERT(!_mtu->isMarriedSeg(&seg));
}

void testIsMarriedSegReturnTrue()
{
  AirSeg seg;
  seg.marriageStatus() = AirSeg::MARRIAGE_START;
  CPPUNIT_ASSERT(_mtu->isMarriedSeg(&seg));
}

void testFindStopOversWhenMarriedSegOver4Hrs()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg1.marriageStatus() = AirSeg::MARRIAGE_START;
  seg2.departureDT() = DateTime(2014, 1, 18, 8, 0, 0); // Jan 18, 2014 8am
  seg2.marriageStatus() = AirSeg::MARRIAGE_END;
  seg2.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==0);
}

void testFindStopOversWhenMarriedSegWithStopOver()
{
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2, seg3;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg1.marriageStatus() = AirSeg::MARRIAGE_START;
  seg2.departureDT() = DateTime(2014, 1, 18, 8, 0, 0); // Jan 18, 2014 8am
  seg2.arrivalDT() = DateTime(2014, 1, 18, 9, 0, 0);   // Jan 18, 2014 9am
  seg2.marriageStatus() = AirSeg::MARRIAGE_END;
  seg2.geoTravelType() = GeoTravelType::Domestic;
  seg3.departureDT() = DateTime(2014, 1, 18, 15, 0, 0); // Jan 18, 2014 3pm
  seg3.geoTravelType() = GeoTravelType::Domestic;
  tvlSegs.push_back(&seg1);
  tvlSegs.push_back(&seg2);
  tvlSegs.push_back(&seg3);
  size_t count = 0;
  TravelSeg* tvl = 0;
  _mtu->findStopOvers(tvlSegs, count, tvl);
  CPPUNIT_ASSERT(count==1);
}

void testCreateMultiTicketItinsWhenNoStopOver()
{
  Itin itin;
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg;
  itin.travelSeg().push_back(&seg);
  _trx->itin().push_back(&itin);
  _mtu->createMultiTicketItins(*_trx);
  CPPUNIT_ASSERT(_trx->itin().size()==1);
}

void testCreateMultiTicketItinsWhenOneStopOver()
{
  Itin itin;
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);   // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 7, 1, 0); // Jan 18, 2014 7:01am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  itin.travelSeg().push_back(&seg1);
  itin.travelSeg().push_back(&seg2);
  _trx->itin().push_back(&itin);
  _mtu->createMultiTicketItins(*_trx);
  CPPUNIT_ASSERT(_trx->itin().size()==3);
}

void testCreateMultiTicketItinsWhenTwoStopOvers()
{
  Itin itin;
  std::vector<TravelSeg*> tvlSegs;
  AirSeg seg1, seg2, seg3;
  seg1.arrivalDT() = DateTime(2014, 1, 18, 3, 0, 0);    // Jan 18, 2014 3am
  seg2.departureDT() = DateTime(2014, 1, 18, 7, 1, 0);  // Jan 18, 2014 7:01am
  seg2.arrivalDT() = DateTime(2014, 1, 18, 8, 0, 0);    // Jan 18, 2014 8am
  seg2.geoTravelType() = GeoTravelType::Domestic;
  seg3.departureDT() = DateTime(2014, 1, 18, 13, 0, 0); // Jan 18, 2014 1pm
  seg3.geoTravelType() = GeoTravelType::Domestic;
  itin.travelSeg().push_back(&seg1);
  itin.travelSeg().push_back(&seg2);
  itin.travelSeg().push_back(&seg3);
  _trx->itin().push_back(&itin);
  _mtu->createMultiTicketItins(*_trx);
  CPPUNIT_ASSERT(_trx->itin().size()==1);
}

void testValidateValidatingCxrDataWhenSubItinsHaveNoGSA()
{
  createMultiItins();
  ValidatingCxrGSAData validatingCxrGSAData;
  _fullItin->validatingCxrGsaData() = &validatingCxrGSAData;
  _mtu->validateValidatingCxrData();
  CPPUNIT_ASSERT(_trx->itin().size()==1);
}

void testValidateValidatingCxrDataWhenSubItinsHaveGSA()
{
  createMultiItins();
  ValidatingCxrGSAData validatingCxrGSAData;
  _fullItin->validatingCxrGsaData() = &validatingCxrGSAData;
  _subItin1->validatingCxrGsaData() = &validatingCxrGSAData;
  _subItin2->validatingCxrGsaData() = &validatingCxrGSAData;
  _mtu->validateValidatingCxrData();
  CPPUNIT_ASSERT(_trx->itin().size()==3);
}

void testValidateTicketingAgreementWhenOnlySubItin1HasGSA()
{
  createMultiItins();
  ValidatingCxrGSAData validatingCxrGSAData;
  _fullItin->validatingCxrGsaData() = &validatingCxrGSAData;
  _subItin1->validatingCxrGsaData() = &validatingCxrGSAData;
  _mtu->validateTicketingAgreement(*_trx);
  CPPUNIT_ASSERT(_trx->itin().size()==1);
}

void testIsSolutionFoundReturnFalseWhenNoFullItin()
{
  CPPUNIT_ASSERT(!_mtu->isMultiTicketSolutionFound(*_trx));
}

void testIsSolutionFoundReturnFalseWhenNoMultiTicketMap()
{
  Itin itin;
  itin.setMultiTktItinOrderNum(0);
  _trx->itin().push_back(&itin);
  CPPUNIT_ASSERT(!_mtu->isSolutionFound());
}

void testIsSolutionFoundReturnTrueWhenAllItinsHaveFarePaths()
{
  createMultiItins();
  FarePath fp, fp1, fp2;
  _fullItin->farePath().push_back(&fp);
  _subItin1->farePath().push_back(&fp1);
  _subItin2->farePath().push_back(&fp2);
  CPPUNIT_ASSERT(_mtu->isSolutionFound());
}

void testIsSolutionFoundReturnFalseAndSubItinsCleanedUpWhenOneSubItinHasNoFarePath()
{
  createMultiItins();
  FarePath fp, fp1;
  _fullItin->farePath().push_back(&fp);
  _subItin1->farePath().push_back(&fp1);
  CPPUNIT_ASSERT(!_mtu->isSolutionFound() && _trx->itin().size()==1);
}

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _mtu = _memHandle.insert(new MultiTicketUtil(*_trx));
    _fullItin = _memHandle.create<Itin>();
    _subItin1 = _memHandle.create<Itin>();
    _subItin2 = _memHandle.create<Itin>();
    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locRIO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocRIO.xml");
    _locSAO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSAO.xml");
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  AirSeg* createSegment(const Loc* origin, const Loc* destination, const CarrierCode cxr)
  {
    AirSeg* air = _memHandle.create<AirSeg>();
    air->origin() = origin;
    air->destination() = destination;
    air->carrier() = cxr;
    return air;
  }

  void createMultiItins()
  {
    _fullItin->setMultiTktItinOrderNum(0);
    _subItin1->setMultiTktItinOrderNum(1);
    _subItin2->setMultiTktItinOrderNum(2);
    _trx->itin().push_back(_fullItin);
    _trx->itin().push_back(_subItin1);
    _trx->itin().push_back(_subItin2);
    std::vector<Itin*> subItins;
    _trx->multiTicketMap().insert(PricingTrx::MultiTicketMap::value_type(_trx->itin().front(), subItins));
  }

private:
  TestMemHandle    _memHandle;
  MultiTicketUtil* _mtu;
  PricingTrx*      _trx;
  Itin*            _fullItin;
  Itin*            _subItin1;
  Itin*            _subItin2;
  const Loc*       _locDFW;
  const Loc*       _locRIO;
  const Loc*       _locSAO;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MultiTicketUtilTest);
}
