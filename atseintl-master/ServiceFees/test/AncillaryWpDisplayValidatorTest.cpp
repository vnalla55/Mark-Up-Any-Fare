#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag877Collector.h"
#include "ServiceFees/AncillaryWpDisplayValidator.h"
#include "ServiceFees/OCFees.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <vector>
#include <map>

using namespace std;
namespace tse
{

class AncillaryWpDisplayValidatorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(AncillaryWpDisplayValidatorTest);

  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_3_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_13_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_33_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_63_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_103_D);

  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_1_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_2_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_4_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_11_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_15_M);

  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_0_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_5_H);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_DOW);

  CPPUNIT_SKIP_TEST(testValidateS7Data_Fail_InterlineInd_N);
  CPPUNIT_TEST(testValidateS7Data_Fail_InterlineInd_Y);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();
    _paxType = _memHandle.create<PaxType>();
    _farePath->paxType() = _paxType;
    _trx = _memHandle.create<PricingTrx>();
    _seg1 = createAirSeg("AA", "B", CabinType::BUSINESS_CLASS);
    _seg1->departureDT() = DateTime(2011, 1, 18);
    _seg1->arrivalDT() = DateTime(2011, 1, 18, 3, 0, 0);
    _seg1->segmentOrder() = 1;
    _ocFees = _memHandle.create<OCFees>();
    _ocFees->subCodeInfo() = _memHandle.create<SubCodeInfo>();
    _trx->setRequest(_memHandle.create<AncRequest>());
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _tvlSegs.push_back(_seg1);
    _itin = _memHandle.create<Itin>();
    _itin->travelSeg() = _tvlSegs;
    _farePath->itin() = _itin;
    _endOfJourney = _tvlSegs.end();
    _diag = _memHandle.create<Diag877Collector>();
    createValidator(true, true, true);
    const std::string activationDate = "2000-01-01";
    TestConfigInitializer::setValue("EMD_VALIDATION_FLIGHT_RELATED_SERVICE_AND_PREPAID_BAGGAGE",
                                    activationDate,
                                    "EMD_ACTIVATION");

  }
  void tearDown() { _memHandle.clear(); }
  void createValidator(bool isInterational, bool isOneCarrier, bool isMarketingCarrier)
  {
    OcValidationContext ctx(*_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
    _validator = _memHandle.insert(new AncillaryWpDisplayValidator(ctx,
                                                                   _tvlSegs.begin(),
                                                                   _tvlSegs.end(),
                                                                   _endOfJourney,
                                                                   _ts2ss,
                                                                   isInterational,
                                                                   isOneCarrier,
                                                                   isMarketingCarrier,
                                                                   _diag));
  }

  AirSeg* createAirSeg(CarrierCode cxr, BookingCode bkg, CabinType::CabinTypeNew cabin)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->carrier() = cxr;
    airSeg->setOperatingCarrierCode(cxr);
    airSeg->setBookingCode(bkg);
    airSeg->bookedCabin().setClass(cabin);
    return airSeg;
  }

  void testSetPBDate_When_advPurchUnit_3_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "03";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 3, 28));
  }

  void testSetPBDate_When_advPurchUnit_13_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "13";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 3, 18));
  }

  void testSetPBDate_When_advPurchUnit_33_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "33";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 2, 26));
  }

  void testSetPBDate_When_advPurchUnit_63_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "63";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 1, 27));
  }

  void testSetPBDate_When_advPurchUnit_103_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "103";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2010, 12, 18));
  }

  void testSetPBDate_When_advPurchUnit_1_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "01";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 2, 28));
  }

  void testSetPBDate_When_advPurchUnit_2_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "02";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 1, 31));
  }

  void testSetPBDate_When_advPurchUnit_4_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "04";
    _seg1->departureDT() = DateTime(2012, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 11, 30));
  }

  void testSetPBDate_When_advPurchUnit_11_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "11";
    _seg1->departureDT() = DateTime(2012, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 4, 30));
  }

  void testSetPBDate_When_advPurchUnit_15_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "15";
    _seg1->departureDT() = DateTime(2012, 3, 15);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2010, 12, 15));
  }

  void testSetPBDate_When_advPurchUnit_0_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "0";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == DateTime(2011, 9, 18, 8, 15, 0));
  }

  void testSetPBDate_When_advPurchUnit_5_H()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "H";
    optSrvInfo.advPurchPeriod() = "05";
    DateTime dt = _trx->ticketingDate();
    DateTime calc = dt.addSeconds(18000);

    _seg1->departureDT() = dt.nextDay();
    DateTime comp = _seg1->departureDT();
    DateTime compareDT = comp.subtractSeconds(18000);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, calc));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == compareDT);
  }

  void testSetPBDate_When_advPurchUnit_DOW()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "01";
    optSrvInfo.advPurchPeriod() = "FRI";
    uint32_t days = 0;
    int advPinDays = 0;
    DateTime dt = _trx->ticketingDate();
    _seg1->departureDT() = dt.addDays(10);

    _validator->getAdvPurPeriod(optSrvInfo, _seg1->departureDT(), days, advPinDays);
    if (days == 0 && advPinDays == 0)
      days = 7;
    days += advPinDays;

    DateTime comp = _seg1->departureDT();
    DateTime compareDT = comp.subtractDays(days);

    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, _trx->ticketingDate()));
    CPPUNIT_ASSERT(_ocFees->purchaseByDate() == compareDT);
  }

  void testValidateS7Data_Fail_InterlineInd_N()
  {
    OptionalServicesInfo* osi = _memHandle.create<OptionalServicesInfo>();
    osi->interlineInd() = 'N';

    CPPUNIT_ASSERT_EQUAL(false,
                  _validator->checkInterlineIndicator(*osi));
  }

  void testValidateS7Data_Fail_InterlineInd_Y()
  {
    OptionalServicesInfo* osi = _memHandle.create<OptionalServicesInfo>();
    osi->interlineInd() = 'Y';
    CPPUNIT_ASSERT_EQUAL(true, _validator->checkInterlineIndicator(*osi));
  }


protected:
  TestMemHandle _memHandle;
  AncillaryWpDisplayValidator* _validator;
  FarePath* _farePath;
  Itin* _itin = nullptr;
  PaxType* _paxType;
  AirSeg* _seg1;
  PricingTrx* _trx;
  std::vector<TravelSeg*> _tvlSegs;
  std::vector<TravelSeg*>::const_iterator _endOfJourney;
  Ts2ss _ts2ss;
  Diag877Collector* _diag;
  OCFees* _ocFees;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryWpDisplayValidatorTest);
}
