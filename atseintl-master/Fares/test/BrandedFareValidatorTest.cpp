#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/BrandedFareValidator.h"
#include "DBAccess/DiskCache.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class BrandedFareValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandedFareValidatorTest);

  CPPUNIT_TEST(testPrintFareMarket_NoSegments);
  CPPUNIT_TEST(testPrintFareMarket_ArunkSegment);
  CPPUNIT_TEST(testPrintFareMarket_AirSegment);
  CPPUNIT_TEST(testPrintFareMarket_MixedSegments);

  CPPUNIT_TEST(testRegularFaresValidation);
  CPPUNIT_TEST(testRegularFaresValidation_isInvalidAtStart);
  CPPUNIT_TEST(testRegularFaresValidation_BasicValidation_Fail);
  CPPUNIT_TEST(testRegularFaresValidation_SelectionValidation_PrimaryBookingCode_Fail);
  CPPUNIT_TEST(testRegularFaresValidation_SelectionValidation_SecondaryBookingCode_Fail);
  CPPUNIT_TEST(testRegularFaresValidation_SelectionValidation_FareFamily_Fail);
  CPPUNIT_TEST(testRegularFaresValidation_SelectionValidation_FareBasis_Fail);
  CPPUNIT_TEST(testRegularFaresValidation_AMIssue);

  CPPUNIT_TEST(testExcludeFaresValidation);
  CPPUNIT_TEST(testExcludeFaresValidation_FareBasisExclude_Fail);
  CPPUNIT_TEST(testExcludeFaresValidation_FareFamilyExclude_Fail);

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
  static const uint16_t BRAND_INDEX = 0;
  static const FareClassCode FARE_CLASS, FARE_FAMILY;
  static const BookingCode BOOKING_CODE, SEC_BOOKING_CODE, OTHER_BOOKING_CODE;
  static const Indicator CORPID_IND = 'C';

  BrandedFareValidator* _val;
  PricingTrx* _trx;
  DiagManager* _diag;
  PricingRequest* _req;
  TestMemHandle _mem;

public:
  void setUp()
  {
    _mem.create<SpecificTestConfigInitializer>();

    _trx = _mem.create<PricingTrx>();

    _trx->diagnostic().diagnosticType() = Diagnostic499;
    _diag = _mem(new DiagManager(*_trx, Diagnostic499));
    _diag->collector().activate();

    _req = _mem.create<PricingRequest>();
    _trx->setRequest(_req);
    _req->brandId(BRAND_INDEX) = BOOKING_CODE;

    _val = _mem.create<BrandedFareValidator>(*_trx, *_diag);
  }

  void tearDown() { _mem.clear(); }

  std::string getDiagString()
  {
    _diag->collector().flushMsg();
    return _diag->collector().str();
  }

  FareMarket* createFareMarket()
  {
    FareMarket* fm = _mem.create<FareMarket>();
    fm->governingCarrier() = "AA";
    fm->setGlobalDirection(GlobalDirection::DO);
    fm->direction() = FMDirection::INBOUND;
    return fm;
  }

  template <typename T>
  T* createSeg(const LocCode& orig, const LocCode& dest)
  {
    T* seg = _mem.create<T>();
    seg->origAirport() = orig;
    seg->destAirport() = dest;
    return seg;
  }

  ArunkSeg* createArunk(const LocCode& orig, const LocCode& dest)
  {
    return createSeg<ArunkSeg>(orig, dest);
  }

  AirSeg* createAir(const LocCode& orig, const CarrierCode& cr, const LocCode& dest)
  {
    AirSeg* s = createSeg<AirSeg>(orig, dest);
    s->carrier() = cr;
    return s;
  }

  void testPrintFareMarket_NoSegments()
  {
    _val->printFareMarket(*createFareMarket());

    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  void testPrintFareMarket_ArunkSegment()
  {
    FareMarket& fm = *createFareMarket();
    fm.travelSeg() += createArunk("LAX", "DFW");
    _val->printFareMarket(fm);

    CPPUNIT_ASSERT_EQUAL(std::string(" \n"
                                     "LAX-DFW    /CXR-AA/ #GI-DO#  .IN.\n"),
                         getDiagString());
  }

  void testPrintFareMarket_AirSegment()
  {
    FareMarket& fm = *createFareMarket();
    fm.travelSeg() += createAir("DFW", "UA", "ORD");
    _val->printFareMarket(fm);

    CPPUNIT_ASSERT_EQUAL(std::string(" \n"
                                     "DFW-UA-ORD    /CXR-AA/ #GI-DO#  .IN.\n"),
                         getDiagString());
  }

  void testPrintFareMarket_MixedSegments()
  {
    FareMarket& fm = *createFareMarket();
    fm.travelSeg() += createArunk("LAX", "DFW"), createAir("DFW", "UA", "ORD");
    _val->printFareMarket(fm);

    CPPUNIT_ASSERT_EQUAL(std::string(" \n"
                                     "LAX-DFW-UA-ORD    /CXR-AA/ #GI-DO#  .IN.\n"),
                         getDiagString());
  }

  PaxTypeFare* createPaxTypeFare(const BookingCode& bc = BOOKING_CODE)
  {
    FareInfo* fi = _mem.create<FareInfo>();
    fi->fareClass() = FARE_CLASS;

    Fare* f = _mem.create<Fare>();
    f->setFareInfo(fi);

    PaxTypeFare* ptf = _mem.create<PaxTypeFare>();
    ptf->setFare(f);

    ptf->setMatchedCorpID(false);
    ptf->bookingCode() = bc;
    ptf->setIsShoppingFare();

    return ptf;
  }

  void populateBrandedFaresData()
  {
    _req->brandedFareBookingCode(BRAND_INDEX) += BOOKING_CODE;
    _req->brandedFareSecondaryBookingCode(BRAND_INDEX) += SEC_BOOKING_CODE;
  }

  void testRegularFaresValidation()
  {
    populateBrandedFaresData();
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  void testRegularFaresValidation_isInvalidAtStart()
  {
    populateBrandedFaresData();
    PaxTypeFare* ptf = createPaxTypeFare();
    ptf->fare()->setInvBrand(BRAND_INDEX, true);
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  void testRegularFaresValidation_BasicValidation_Fail()
  {
    populateBrandedFaresData();
    PaxTypeFare* ptf = createPaxTypeFare(OTHER_BOOKING_CODE);
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK - FBCODE ECHO     "
                                     "NOT IN SPECIFIED BKCODES, FBCODES NOR FFAMILIES\n"),
                         getDiagString());
  }

  void testRegularFaresValidation_SelectionValidation_PrimaryBookingCode_Fail()
  {
    populateBrandedFaresData();
    _req->brandedFareBookingCodeData(BRAND_INDEX)[BOOKING_CODE] = CORPID_IND;
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK - REQUIRE CORP ID FARE "
                                     "FOR ECHO     BKCODE B\n"),
                         getDiagString());
  }

  void testRegularFaresValidation_SelectionValidation_SecondaryBookingCode_Fail()
  {
    populateBrandedFaresData();
    _req->brandedFareSecondaryBookingCodeData(BRAND_INDEX)[SEC_BOOKING_CODE] = CORPID_IND;
    PaxTypeFare* ptf = createPaxTypeFare(SEC_BOOKING_CODE);
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK - REQUIRE CORP ID FARE "
                                     "FOR ECHO     BKCODE D\n"),
                         getDiagString());
  }

  void testRegularFaresValidation_SelectionValidation_FareFamily_Fail()
  {
    populateBrandedFaresData();
    _req->brandedFareFamilyData(BRAND_INDEX)[FARE_FAMILY] = CORPID_IND;
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK - REQUIRE CORP ID FARE "
                                     "FOR ECHO     FFAMILY -CHO\n"),
                         getDiagString());
  }

  void testRegularFaresValidation_SelectionValidation_FareBasis_Fail()
  {
    populateBrandedFaresData();
    _req->brandedFareBasisCodeData(BRAND_INDEX)[FARE_CLASS] = CORPID_IND;
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->regularFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK - REQUIRE CORP ID FARE "
                                     "FOR FBCODE ECHO    \n"),
                         getDiagString());
  }

  void testRegularFaresValidation_AMIssue()
  {
    TestConfigInitializer::setValue(
        "EXTENDED_BOOKING_CODE_VALIDATION_PARTITIONS_IDS", "AM|AA", "SHOPPING_OPT");

    populateBrandedFaresData();

    PricingRequest req;
    req.brandedFareBookingCode().push_back(OTHER_BOOKING_CODE);
    _trx->setRequest(&req);

    Billing billing;
    billing.partitionID() = "AM";
    _trx->billing() = &billing;

    PaxTypeFare* ptf = createPaxTypeFare("");

    PaxTypeFare::SegmentStatus segmentStatus;
    segmentStatus._bkgCodeReBook = OTHER_BOOKING_CODE;
    segmentStatus._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    std::vector<PaxTypeFare::SegmentStatus> segmentStatusRule2;
    segmentStatusRule2.push_back(segmentStatus);

    std::vector<PaxTypeFare*> fares(1, ptf);
    fares[0]->segmentStatusRule2() = segmentStatusRule2;

    BrandedFareValidator val(*_trx, *_diag);
    val.regularFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->fare()->isInvBrandCorpID(BRAND_INDEX));
    CPPUNIT_ASSERT(!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  void testExcludeFaresValidation()
  {
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->excludeFaresValidation(fares);

    CPPUNIT_ASSERT(!ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString());
  }

  void testExcludeFaresValidation_FareBasisExclude_Fail()
  {
    _req->brandedFareBasisCodeExclude(BRAND_INDEX) += FARE_CLASS;
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->excludeFaresValidation(fares);

    CPPUNIT_ASSERT(ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK - FBCODE ECHO     EXCLUDED.\n"),
                         getDiagString());
  }

  void testExcludeFaresValidation_FareFamilyExclude_Fail()
  {
    _req->brandedFareFamilyExclude(BRAND_INDEX) += FARE_FAMILY;
    PaxTypeFare* ptf = createPaxTypeFare();
    std::vector<PaxTypeFare*> fares(1, ptf);

    _val->excludeFaresValidation(fares);

    CPPUNIT_ASSERT(ptf->fare()->isInvBrand(BRAND_INDEX));
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BRANDED CHK FOR FBCODE ECHO     "
                                     "- FFAMILY -CHO EXCLUDED.\n"),
                         getDiagString());
  }
};

const FareClassCode BrandedFareValidatorTest::FARE_CLASS = "ECHO",
                    BrandedFareValidatorTest::FARE_FAMILY = "-CHO";
const BookingCode BrandedFareValidatorTest::BOOKING_CODE = "B",
                  BrandedFareValidatorTest::SEC_BOOKING_CODE = "D",
                  BrandedFareValidatorTest::OTHER_BOOKING_CODE = "Y";

CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFareValidatorTest);

} // tse
