#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/CommissionsProgramValidator.h"
#include "Rules/RuleUtil.cpp"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <vector>

//#define PERFORMANCE_TEST
#ifdef PERFORMANCE_TEST
#include <boost/chrono.hpp>

#define PERFORMANCE_ITERATIONS 10000
#endif

namespace tse
{
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

namespace
{
class CommissionsProgramValidatorMock : public CommissionsProgramValidator
{
public:
  CommissionsProgramValidatorMock(PricingTrx& trx, FarePath& fp, FareUsage& fu, Diag867Collector* diag)
    : CommissionsProgramValidator(trx, fp, fu, diag) {}

  bool isInLoc(const VendorCode& vendor, const LocKey& locKey, const Loc& loc, const CarrierCode&)
  {
    if (vendor == ATPCO_VENDOR_CODE || vendor == COS_VENDOR_CODE)
    {
      return locKey.loc() == loc.loc();
    }
    else
      return !isdigit(loc.loc()[0]) && !isdigit(locKey.loc()[0]);
  }

  bool isInZone(const VendorCode&, const LocCode& zone, const Loc&, CarrierCode)
  {
    return isdigit(zone[0]);
  }

};
}

class CommissionsProgramValidatorTest: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CommissionsProgramValidatorTest);
  //CPPUNIT_TEST(testMatchPointOfSale_Empty_Pass);
  CPPUNIT_TEST(testMatchPointOfSale_NotEmpty_Pass);
  CPPUNIT_TEST(testMatchPointOfSale_NotEmpty_GEO_FAIL);
  CPPUNIT_TEST(testMatchPointOfSale_NotEmpty_GEO_Pass_Excl_FAIL);

  CPPUNIT_TEST(testMatchPointOfOrigin_Empty_Pass);
  CPPUNIT_TEST(testMatchPointOfOrigin_NotEmpty_Pass);
  CPPUNIT_TEST(testMatchPointOfOrigin_NotEmpty_GEO_FAIL);
  CPPUNIT_TEST(testMatchPointOfOrigin_NotEmpty_GEO_Pass_Excl_FAIL);

  CPPUNIT_TEST(testMatchMarkets_Empty_Pass);
  CPPUNIT_TEST(testMatchMarkets_BothLocs_Empty_Pass);
  CPPUNIT_TEST(testMatchMarkets_BothLocs_NotEmpty_Pass);
  CPPUNIT_TEST(testMatchMarkets_BothLocs_NotEmpty_BiDirectional_Pass);
  CPPUNIT_TEST(testMatchMarkets_BothLocs_NotEmpty_BiDirectional_Fail);

  CPPUNIT_TEST(testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectN_Pass);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectY_Pass);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectN_FailOrig);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectY_FailOrigAndDest);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectY_PassDest);

  CPPUNIT_TEST(testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectN_Pass);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectY_Pass);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectN_FailDest);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectY_FailDestAndOrig);
  CPPUNIT_TEST(testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectY_PassOrig);

  CPPUNIT_TEST(testCheckTicketingDates_Pass);
  CPPUNIT_TEST(testCheckTicketingDates_Fail_EndTkt);
  CPPUNIT_TEST(testCheckTicketingDates_Pass_Inf);
  CPPUNIT_TEST(testCheckTicketingDates_TimeOk);
  CPPUNIT_TEST(testCheckTicketingDates_TimeFailed);

  CPPUNIT_TEST(testCheckTravelDates_Empty_Pass);
  CPPUNIT_TEST(testCheckTravelDates_OneRow_Pass);
  CPPUNIT_TEST(testCheckTravelDates_OneRow_Fail);
  CPPUNIT_TEST(testCheckTravelDates_TwoRows_FailFirst_PassSecond);
  CPPUNIT_TEST(testCheckTravelDates_TwoRows_Fail);

  CPPUNIT_TEST_SUITE_END();

  void testMatchPointOfSale_Empty_Pass()
  {
    CommissionProgramInfo cpi;
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchPointOfSale(cpi, cxr));
  }

  void testMatchPointOfSale_NotEmpty_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->loc() = "FRA";
    prog->pointOfSale()[1]->inclExclInd() = 'I';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchPointOfSale(*prog, cxr));
  }

  void testMatchPointOfSale_NotEmpty_GEO_FAIL()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->nation() = "KU ";
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchPointOfSale(*prog, cxr));
  }

  void testMatchPointOfSale_NotEmpty_GEO_Pass_Excl_FAIL()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->pointOfSale()[0]->inclExclInd() = 'E';
    prog->pointOfSale()[1]->inclExclInd() = 'E';
    _loc1->nation() = "AZ ";
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchPointOfSale(*prog, cxr));
  }

  void testMatchPointOfOrigin_Empty_Pass()
  {
    CommissionProgramInfo cpi;
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchPointOfOrigin(cpi, cxr));
  }

  void testMatchPointOfOrigin_NotEmpty_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->loc() = "FRA";
    prog->pointOfSale()[1]->inclExclInd() = 'I';
    CarrierCode cxr = "AA";
    _itin->travelSeg().push_back(_airSeg1);
    _itin->travelSeg().push_back(_airSeg2);
    CPPUNIT_ASSERT(_cpv->matchPointOfOrigin(*prog, cxr));
  }

  void testMatchPointOfOrigin_NotEmpty_GEO_FAIL()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->nation() = "KU ";
    CarrierCode cxr = "AA";
    _itin->travelSeg().push_back(_airSeg1);
    CPPUNIT_ASSERT(!_cpv->matchPointOfOrigin(*prog, cxr));
  }

  void testMatchPointOfOrigin_NotEmpty_GEO_Pass_Excl_FAIL()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->pointOfOrigin()[0]->inclExclInd() = 'E';
    prog->pointOfOrigin()[1]->inclExclInd() = 'E';
    _loc1->nation() = "AZ ";
    CarrierCode cxr = "AA";
    _itin->travelSeg().push_back(_airSeg1);
    CPPUNIT_ASSERT(!_cpv->matchPointOfOrigin(*prog, cxr));
  }

  void testMatchMarkets_Empty_Pass()
  {
    CommissionProgramInfo cpi;
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(cpi, cxr));
  }

  void testMatchMarkets_BothLocs_Empty_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->origin() = lk0;
    prog->markets()[0]->destination() = lk0;
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_BothLocs_NotEmpty_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->loc() = "AZ ";
    _loc2->loc() = "FRA";
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_BothLocs_NotEmpty_BiDirectional_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc2->loc() = "AZ ";
    _loc1->loc() = "FRA";
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_BothLocs_NotEmpty_BiDirectional_Fail()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectN_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->loc() = "AZ ";
    LocKey lk0;
    prog->markets()[0]->destination() = lk0;
    prog->markets()[0]->bidirectional() = 'N';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectY_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->loc() = "AZ ";
    LocKey lk0;
    prog->markets()[0]->destination() = lk0;
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectN_FailOrig()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->destination() = lk0;
    prog->markets()[0]->bidirectional() = 'N';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectY_FailOrigAndDest()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->destination() = lk0;
    prog->markets()[0]->bidirectional() = 'Y';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_NotEmpty_DestLocEmpty_BiDirectY_PassDest()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->destination() = lk0;
    _loc2->loc() = "AZ ";
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectN_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc2->loc() = "FRA";
    LocKey lk0;
    prog->markets()[0]->origin() = lk0;
    prog->markets()[0]->bidirectional() = 'N';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectY_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _loc1->loc() = "FRA";
    LocKey lk0;
    prog->markets()[0]->origin() = lk0;
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectN_FailDest()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->origin() = lk0;
    prog->markets()[0]->bidirectional() = 'N';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectY_FailDestAndOrig()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->origin() = lk0;
    prog->markets()[0]->bidirectional() = 'Y';
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(!_cpv->matchMarket(*prog, cxr));
  }

  void testMatchMarkets_OrigLoc_Empty_DestLocNotEmpty_BiDirectY_PassOrig()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    LocKey lk0;
    prog->markets()[0]->origin() = lk0;
    _loc1->loc() = "FRA";
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cpv->matchMarket(*prog, cxr));
  }

  void testCheckTicketingDates_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->startTktDate() = DateTime(2015, 11, 11);
    prog->endTktDate() = DateTime(2015, 11, 11);
    _trx->ticketingDate() = DateTime(2015, 11, 11);
    CPPUNIT_ASSERT(_cpv->matchTicketingDates(*prog));
  }

  void testCheckTicketingDates_Fail_EndTkt()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->startTktDate() = DateTime(2015, 12, 11);
    prog->endTktDate() = DateTime(2015, 11, 11);
    _trx->ticketingDate() = DateTime(2015, 11, 11);
    CPPUNIT_ASSERT(!_cpv->matchTicketingDates(*prog));
  }

  void testCheckTicketingDates_Pass_Inf()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->startTktDate() = DateTime(boost::date_time::pos_infin);
    prog->endTktDate() = DateTime(boost::date_time::pos_infin);
    _trx->ticketingDate() = DateTime(2015, 11, 11);
    CPPUNIT_ASSERT(_cpv->matchTicketingDates(*prog));
  }

  void testCheckTicketingDates_TimeOk()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->startTktDate() = DateTime(2015, 11, 11, 15, 29, 44);
    prog->endTktDate() = DateTime(2016, 11, 11);
    _trx->ticketingDate() = DateTime(2015, 11, 11, 15, 30, 44);
    CPPUNIT_ASSERT(_cpv->matchTicketingDates(*prog));
  }

  void testCheckTicketingDates_TimeFailed()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->startTktDate() = DateTime(2015, 11, 11, 15, 29, 44);
    prog->endTktDate() = DateTime(2016, 11, 11);
    _trx->ticketingDate() = DateTime(2015, 11, 11, 15, 28, 44);
    CPPUNIT_ASSERT(!_cpv->matchTicketingDates(*prog));
  }

//  Travel dates
  void testCheckTravelDates_Empty_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    prog->travelDates().clear();
    CPPUNIT_ASSERT(_cpv->matchTravelDates(*prog));
  }

  void testCheckTravelDates_OneRow_Pass()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _airSeg1->departureDT() = DateTime(2015, 11, 11, 15, 28, 44);
    CPPUNIT_ASSERT(_cpv->matchTravelDates(*prog));
  }

  void testCheckTravelDates_OneRow_Fail()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _airSeg1->departureDT() = DateTime(2016, 2, 10);
    prog->travelDates()[0]->firstTravelDate() = DateTime(2016, 3, 31);
    CPPUNIT_ASSERT(!_cpv->matchTravelDates(*prog));
  }

  void testCheckTravelDates_TwoRows_FailFirst_PassSecond()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _airSeg1->departureDT() = DateTime(2015, 12, 31, 15, 28, 44);

    prog->travelDates()[0]->endTravelDate() = DateTime(2015, 11, 30);
    CommissionTravelDatesSegInfo* travelDate1 (new CommissionTravelDatesSegInfo);
    travelDate1->firstTravelDate() = DateTime(2015, 12, 31);
    travelDate1->endTravelDate() = DateTime(boost::date_time::pos_infin);
    prog->travelDates().emplace_back(travelDate1);

    CPPUNIT_ASSERT(_cpv->matchTravelDates(*prog));
  }

  void testCheckTravelDates_TwoRows_Fail()
  {
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    _airSeg1->departureDT() = DateTime(2016, 2, 15, 14, 30, 00);

    prog->travelDates()[0]->endTravelDate() = DateTime(2015, 11, 30);
    CommissionTravelDatesSegInfo* travelDate1 (new CommissionTravelDatesSegInfo);
    travelDate1->firstTravelDate() = DateTime(2015, 12, 31);
    travelDate1->endTravelDate() = DateTime(2016, 1, 31);
    prog->travelDates().emplace_back(travelDate1);

    CPPUNIT_ASSERT(!_cpv->matchTravelDates(*prog));
  }


  void createCommissionProgramInfo1(CommissionProgramInfo* prog, LocKey& lk1, LocKey& lk2)
  {
    CommissionProgramInfo::dummyData(*prog);
    prog->vendor() = "COS";
    prog->effDate() = DateTime(2015, 11, 11, 15, 30, 44);
    prog->expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
    prog->programId() = 45909;
    prog->programName() = "COMMISSION PROGRAM TESTING";
    prog->pointOfSaleItemNo() = 2121390;
    prog->pointOfOriginItemNo() = 18009900;
    prog->travelDatesItemNo() = 4;
    prog->marketItemNo() = 12;
    prog->contractId() = 88;
    prog->maxConnectionTime() = 2400;

    prog->startTktDate() = DateTime(2015, 11, 11, 15, 30, 44);
    prog->endTktDate() = DateTime::emptyDate();
    prog->landAgreementInd() = 'Y';
    prog->qSurchargeInd() = 'Y';
    prog->throughFareInd() = 'N';

    CommissionLocSegInfo* loc1 = prog->pointOfSale()[0];
    loc1->vendor() = "COS";
    loc1->orderNo() = 33333;
    loc1->inclExclInd() = 'I';
    loc1->loc() = lk1;

    CommissionLocSegInfo* loc2 = prog->pointOfSale()[1];
    loc2->vendor() = "COS";
    loc2->orderNo() = 44444;
    loc2->inclExclInd() = 'E';
    loc2->loc() = lk2;

    CommissionLocSegInfo* origin1 = prog->pointOfOrigin()[0];
    origin1->vendor() = "COS";
    origin1->orderNo() = 33333;
    origin1->inclExclInd() = 'I';
    origin1->loc() = lk1;
    CommissionLocSegInfo* origin2 = prog->pointOfOrigin()[1];
    origin2->vendor() = "COS";
    origin2->orderNo() = 555;
    origin2->inclExclInd() = 'I';
    origin2->loc() = lk2;

    CommissionTravelDatesSegInfo* travelDate1 = prog->travelDates()[0];
    travelDate1->firstTravelDate() = DateTime(2015, 10, 11, 15, 30, 44);
    travelDate1->endTravelDate() = DateTime(boost::date_time::pos_infin);

    CommissionMarketSegInfo* market = prog->markets()[0];
    market->orderNo() = 818181;
    market->origin() = lk1;
    market->destination() = lk2;
    market->inclExclInd() = 'I';
    market->bidirectional() = 'Y';

  }

  void populateLocs(LocKey& lk1, LocKey& lk2)
  {
    lk1.loc() = "AZ ";
    lk1.locType() = 'N';
    lk2.loc() = "FRA";
    lk2.locType() = 'C';
  }

public:
  void setUp()
  {
    _memHandle.create<tse::SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _fp = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _fp->itin() = _itin;
    _fm = _memHandle.create<FareMarket>();
    _fu = _memHandle.create<FareUsage>();
    _loc1 = _memHandle.create<Loc>();
    _loc1->loc() = "DFW";
    _agent->agentLocation() = _loc1;
    _loc2 = _memHandle.create<Loc>();
    _loc2->loc() = "LON";

    _loc3 = _memHandle.create<Loc>();
    _loc3->loc() = "NYC";
    _loc4 = _memHandle.create<Loc>();
    _loc4->loc() = "FRA";

    _fm->boardMultiCity() = "DFW";
    _fm->offMultiCity() = "LON";
    _fm->origin() = _loc1;
    _fm->destination() = _loc2;
    _diag = _memHandle.create<Diag867Collector>();
    _cpv = _memHandle.insert(new CommissionsProgramValidatorMock(*_trx, *_fp, *_fu, _diag));
    _cpi = _memHandle.create<CommissionProgramInfo>();
    _cpi = _memHandle.create<CommissionProgramInfo>();
    _fu->paxTypeFare() = createPaxTypeFare();
    _airSeg1 = _memHandle.create<AirSeg>();
    _airSeg2 = _memHandle.create<AirSeg>();
    _airSeg1->carrier() = "AA";
    _airSeg1->setOperatingCarrierCode("AA");
    _airSeg1->setMarketingCarrierCode("AA");
    _airSeg1->origAirport() = "DFW";
    _airSeg1->destAirport() = "LGA";
    _airSeg1->boardMultiCity() = "DFW";
    _airSeg1->offMultiCity() = "NYC";
    _airSeg1->origin() = _loc1;
    _airSeg1->destination() = _loc3;

    _airSeg2->carrier() = "BA";
    _airSeg2->setOperatingCarrierCode("BA");
    _airSeg2->setMarketingCarrierCode("BA");
    _airSeg2->origAirport() = "JFK";
    _airSeg2->destAirport() = "LHR";
    _airSeg2->boardMultiCity() = "NYC";
    _airSeg2->offMultiCity() = "LON";

    _airSeg2->origin() = _loc3;
    _airSeg2->destination() = _loc2;
    _fu->travelSeg().push_back(_airSeg1);
    _fu->travelSeg().push_back(_airSeg2);
    _itin->travelSeg().push_back(_airSeg1);
    _itin->travelSeg().push_back(_airSeg2);
  }
  void tearDown()
  {
    _memHandle.clear();
  }

  PaxTypeFare* createPaxTypeFare()
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_carrier = "AA";
    fi->_fareClass = "FOCUS";
    fi->_ruleNumber = "0100";
    fi->fareAmount() = 12.34;
    fi->_currency = "USD";
    fi->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    fi->vendor() = "ATP";
    Fare* fare = _memHandle.create<Fare>();
    fare->nucFareAmount() = 12.34;
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();

    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    fcasi->_bookingCode[0] = "A ";
    fcasi->_paxType = "PXX";
    ptf->fareClassAppSegInfo() = fcasi;

    FareClassAppInfo* fca = _memHandle.create<FareClassAppInfo>();
    fca->_fareType = "XPN";
    fca->_displayCatType = 'L';
    ptf->fareClassAppInfo() = fca;

    TariffCrossRefInfo* tcr = _memHandle.create<TariffCrossRefInfo>();
    tcr->ruleTariff() = 3;
    tcr->tariffCat() = 0;
    fare->setTariffCrossRefInfo(tcr);

    ptf->setFare(fare);
    ptf->cabin().setPremiumFirstClass();
    ptf->fareMarket() = _fm;
    return ptf;
  }

  void createAirSegs()
  {
    _airSeg3 = _memHandle.create<AirSeg>();
    _airSeg3->carrier() = "LH";
    _airSeg3->setOperatingCarrierCode("LH");
    _airSeg3->setMarketingCarrierCode("AB");
    _airSeg3->origAirport() = "LGW";
    _airSeg3->destAirport() = "FRA";
    _airSeg1->boardMultiCity() = "LON";
    _airSeg1->offMultiCity() = "FRA";
    _airSeg1->origin() = _loc2;
    _airSeg1->destination() = _loc4;

    FareUsage* fu = const_cast<FareUsage*>(&_cpv->_fu);
    fu->travelSeg().push_back(_airSeg3);
  }

private:
  TestMemHandle       _memHandle;
  CommissionsProgramValidator* _cpv;
  FareMarket*         _fm;
  PricingTrx*         _trx;
  Itin*               _itin;
  PricingRequest*     _request;
  Agent*              _agent;
  FarePath*           _fp;
  CommissionProgramInfo* _cpi;
  FareUsage*          _fu;
  Diag867Collector*   _diag;
  Loc*                _loc1;
  Loc*                _loc2;
  Loc*                _loc3;
  Loc*                _loc4;
  AirSeg*             _airSeg1;
  AirSeg*             _airSeg2;
  AirSeg*             _airSeg3;
};
CPPUNIT_TEST_SUITE_REGISTRATION(CommissionsProgramValidatorTest);
}
