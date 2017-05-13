#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "Common/FreqFlyerUtils.h"
#include "Common/MetricsMan.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageChargesValidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
namespace
{
// MOCKS
}

class BaggageChargesValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageChargesValidatorTest);

  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Blank);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_FreeEmd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_FreeNoEmd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_FreeNoBookEmd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_FreeNoBookNoEmd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Pass);

  CPPUNIT_TEST(testCheckAndOrIndicator_Pass);
  CPPUNIT_TEST(testCheckAndOrIndicator_Fail);

  CPPUNIT_TEST(testCheckBaggageWeightUnit_Pass);
  CPPUNIT_TEST(testCheckBaggageWeightUnit_Fail);

  CPPUNIT_TEST(testCheckFeeApplication_Pass_Blank_Per_Checked_Portion);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_Blank_Per_Baggage_Travel);
  CPPUNIT_TEST(testCheckFeeApplication_Pass);
  CPPUNIT_TEST(testCheckFeeApplication_Fail_Blank_Currency);
  CPPUNIT_TEST(testCheckFeeApplication_Fail_Blank_Mileage);
  CPPUNIT_TEST(testCheckFeeApplication_Fail_Currency);
  CPPUNIT_TEST(testCheckFeeApplication_Fail_Mileage);
  CPPUNIT_TEST(testCheckFeeApplication_Fail_AppFee);

  CPPUNIT_TEST(testCheckOccurrence_Second_Pass_1);
  CPPUNIT_TEST(testCheckOccurrence_Second_Pass_2);
  CPPUNIT_TEST(testCheckOccurrence_Second_Pass_Blank);
  CPPUNIT_TEST(testCheckOccurrence_Second_Pass_3);
  CPPUNIT_TEST(testCheckOccurrence_Fail_FirstPiece_TooLow);
  CPPUNIT_TEST(testCheckOccurrence_Fail_FirstPiece_TooHigh);
  CPPUNIT_TEST(testCheckOccurrence_Both_Pass_1);
  CPPUNIT_TEST(testCheckOccurrence_Both_Pass_2);
  CPPUNIT_TEST(testCheckOccurrence_Both_Pass_Blank);
  CPPUNIT_TEST(testCheckOccurrence_Both_Pass_Both_Blank);

  CPPUNIT_TEST(testGetLocForSfcValidation);

  CPPUNIT_TEST(testBuildCharge);

  CPPUNIT_TEST(testSupplementAndAppendCharge);

  CPPUNIT_TEST(testFrequentFlyerTier_StatusCalculatedByBaggageTravelGreaterThanS7Status);
  CPPUNIT_TEST(testFrequentFlyerTier_OneProvided_SameCxr_GoodLevel);
  CPPUNIT_TEST(testFrequentFlyerTier_OneProvided_SameCxr_GoodLevel_NotActive);
  CPPUNIT_TEST(testFrequentFlyerTier_OneProvided_SameCxr_BadLevel);
  CPPUNIT_TEST(testFrequentFlyerTier_MultipleProvided);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    MockGlobal::setMetricsMan(_memHandle.create<tse::MetricsMan>());
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("FF_TIER_ACTIVATION_DATE", DateTime(2015, 1, 1), "PRICING_SVC");
    _validator = createValidator();
  }

  void tearDown()
  {
    _memHandle.clear();
    MockGlobal::clear();
  }

  BaggageChargesValidator* createValidator()
  {
    _bagTvl = _memHandle.create<BaggageTravel>();
    Itin* itin = _memHandle.create<Itin>();
    itin->setBaggageTripType(BaggageTripType::OTHER);
    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;
    farePath->paxType() = _memHandle.create<PaxType>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    trx->getRequest()->ticketingDT() = DateTime(2015, 9, 10);
    Loc* agentLoc = _memHandle.create<Loc>();
    agentLoc->loc() = "KRK";
    trx->getRequest()->ticketingAgent()->agentLocation() = agentLoc;
    trx->billing() = _memHandle.create<Billing>();
    PricingOptions* opts = _memHandle.create<PricingOptions>();
    opts->fareX() = false;
    opts->isWPwithOutAE() = true;
    trx->setOptions(opts);
    _bagTvl->setupTravelData(*farePath);
    _bagTvl->_trx = trx;

    CheckedPoint* checkedPoint = _memHandle.create<CheckedPoint>();
    Ts2ss* ts2ss = _memHandle.create<Ts2ss>();
    BagValidationOpt opt(*_bagTvl, *checkedPoint, *ts2ss, false, nullptr);
    return _memHandle.insert(new BaggageChargesValidator(opt));
  }

  TestMemHandle _memHandle;
  BaggageChargesValidator* _validator;
  BaggageTravel* _bagTvl;

public:
  // TESTS
  void testCheckServiceNotAvailNoCharge_Blank()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(PASS_S7,
                         _validator->checkServiceNotAvailNoCharge(
                             S7Builder(&_memHandle)
                                 .withNotAvailNoCharge(OptionalServicesValidator::CHAR_BLANK)
                                 .buildRef(),
                             ocFees));
  }

  void testCheckServiceNotAvailNoCharge_FreeEmd()
  {
    OCFees ocFees;
    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_EMD_ISSUED)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_FreeNoEmd()
  {
    OCFees ocFees;
    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_FreeNoBookEmd()
  {
    OCFees ocFees;
    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_BOOK_EMD_ISSUED)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_FreeNoBookNoEmd()
  {
    OCFees ocFees;
    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_BOOK_NO_EMD)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_Pass()
  {
    OCFees ocFees;
    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_NOT_AVAILABLE)
                .buildRef(),
            ocFees));
  }

  void testCheckAndOrIndicator_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkAndOrIndicator(
        S7Builder(&_memHandle).withAndOr(OptionalServicesValidator::CHAR_BLANK).buildRef()));
  }

  void testCheckAndOrIndicator_Fail()
  {
    CPPUNIT_ASSERT(
        !_validator->checkAndOrIndicator(S7Builder(&_memHandle).withAndOr('/').buildRef()));
  }

  void testCheckBaggageWeightUnit_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkBaggageWeightUnit(
        S7Builder(&_memHandle)
            .withBaggageWeightUnit(OptionalServicesValidator::CHAR_BLANK)
            .buildRef()));
  }

  void testCheckBaggageWeightUnit_Fail()
  {
    CPPUNIT_ASSERT(!_validator->checkBaggageWeightUnit(
        S7Builder(&_memHandle).withBaggageWeightUnit('K').buildRef()));
  }

  void testCheckFeeApplication_Pass_Blank_Per_Checked_Portion()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::CHAR_BLANK)
            .withFeesCurr(12345)
            .withApplication('3')
            .buildRef()));
  }

  void testCheckFeeApplication_Pass_Blank_Per_Baggage_Travel()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::CHAR_BLANK)
            .withFeesCurr(12345)
            .withApplication('4')
            .buildRef()));
  }

  void testCheckFeeApplication_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
            .withFeesCurr(0)
            .withApplication(OptionalServicesValidator::CHAR_BLANK)
            .withAppFee(0)
            .buildRef()));
  }

  void testCheckFeeApplication_Fail_Blank_Currency()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::CHAR_BLANK)
            .withFeesCurr(0)
            .withApplication('4')
            .buildRef()));
  }

  void testCheckFeeApplication_Fail_Blank_Mileage()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::CHAR_BLANK)
            .withFeesCurr(12345)
            .withApplication(OptionalServicesValidator::CHAR_BLANK)
            .buildRef()));
  }

  void testCheckFeeApplication_Fail_Currency()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
            .withFeesCurr(12345)
            .withApplication(OptionalServicesValidator::CHAR_BLANK)
            .withAppFee(0)
            .buildRef()));
  }

  void testCheckFeeApplication_Fail_Mileage()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
            .withFeesCurr(0)
            .withApplication('4')
            .withAppFee(0)
            .buildRef()));
  }

  void testCheckFeeApplication_Fail_AppFee()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
            .withFeesCurr(0)
            .withApplication(OptionalServicesValidator::CHAR_BLANK)
            .withAppFee(12345)
            .buildRef()));
  }

  void testCheckOccurrence_Second_Pass_1()
  {
    CPPUNIT_ASSERT(
        _validator->checkOccurrence(S7Builder(&_memHandle).withBaggageOccurrence(1, 2).buildRef()));
  }

  void testCheckOccurrence_Second_Pass_2()
  {
    CPPUNIT_ASSERT(
        _validator->checkOccurrence(S7Builder(&_memHandle).withBaggageOccurrence(2, 3).buildRef()));
  }

  void testCheckOccurrence_Second_Pass_Blank()
  {
    CPPUNIT_ASSERT(_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(2, -1).buildRef()));
  }

  void testCheckOccurrence_Second_Pass_3()
  {
    CPPUNIT_ASSERT(
        _validator->checkOccurrence(S7Builder(&_memHandle).withBaggageOccurrence(1, 1).buildRef()));
  }

  void testCheckOccurrence_Fail_FirstPiece_TooLow()
  {
    CPPUNIT_ASSERT(!_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(-1, 1).buildRef()));
  }

  void testCheckOccurrence_Fail_FirstPiece_TooHigh()
  {
    CPPUNIT_ASSERT(!_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(MAX_BAG_PIECES + 1, 2).buildRef()));
  }

  void testCheckOccurrence_Both_Pass_1()
  {
    CPPUNIT_ASSERT(
        _validator->checkOccurrence(S7Builder(&_memHandle).withBaggageOccurrence(1, 1).buildRef()));
  }

  void testCheckOccurrence_Both_Pass_2()
  {
    CPPUNIT_ASSERT(
        _validator->checkOccurrence(S7Builder(&_memHandle).withBaggageOccurrence(2, 2).buildRef()));
  }

  void testCheckOccurrence_Both_Pass_Blank()
  {
    CPPUNIT_ASSERT(_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(1, -1).buildRef()));
  }

  void testCheckOccurrence_Both_Pass_Both_Blank()
  {
    CPPUNIT_ASSERT(_validator->checkOccurrence(S7Builder(&_memHandle)
                                                   .withVendor(ATPCO_VENDOR_CODE)
                                                   .withBaggageOccurrence(-1, -1)
                                                   .buildRef()));

    CPPUNIT_ASSERT(_validator->checkOccurrence(
        S7Builder(&_memHandle).withVendor("MGMR").withBaggageOccurrence(0, 0).buildRef()));
  }

  void testGetLocForSfcValidation()
  {
    _validator->_baggageTravel.itin()->travelSeg().push_back(
        AirSegBuilder(&_memHandle).withLocations("KRK").build());

    CPPUNIT_ASSERT_EQUAL(std::string("KRK"),
                         std::string(_validator->getLocForSfcValidation().loc()));
  }

  void testBuildCharge()
  {
    SubCodeInfo s5;

    BaggageCharge* charge = _validator->buildCharge(s5);

    CPPUNIT_ASSERT(&s5 == charge->subCodeInfo());
  }

  void testSupplementAndAppendCharge()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle).withCarrier("LT").build();
    BaggageCharge charge;
    ChargeVector charges;

    _validator->_itin.travelSeg().push_back(_memHandle.create<AirSeg>());
    _validator->_segI = _validator->_itin.travelSeg().begin();
    _validator->_segIE = _validator->_itin.travelSeg().end();

    _validator->supplementAndAppendCharge(*s5, &charge, charges);

    CPPUNIT_ASSERT_EQUAL((size_t)1, charges.size());
    CPPUNIT_ASSERT(&charge == charges.front());
    CPPUNIT_ASSERT(s5 == charges.front()->subCodeInfo());
    CPPUNIT_ASSERT_EQUAL(charges.front()->carrierCode(), s5->carrier());
    CPPUNIT_ASSERT(charges.front()->farePath() == _validator->_farePath);
    CPPUNIT_ASSERT(charges.front()->travelStart() == *(_validator->_segI));
    CPPUNIT_ASSERT(charges.front()->travelEnd() == *(_validator->_segIE - 1));
  }

  void addFrequentFlyerStatus(const CarrierCode cxr, const uint16_t level)
  {
    PaxType::FreqFlyerTierWithCarrier* data =
        _memHandle.create<PaxType::FreqFlyerTierWithCarrier>();
    data->setCxr(cxr);
    data->setFreqFlyerTierLevel(level);
    _validator->_paxType.freqFlyerTierWithCarrier().push_back(data);
  }

  template <typename... Args>
  void addFrequentFlyerStatus(const CarrierCode cxr, const uint16_t level, const Args... args)
  {
    addFrequentFlyerStatus(cxr, level);
    addFrequentFlyerStatus(args...);
  }

  void testFrequentFlyerTier_StatusCalculatedByBaggageTravelGreaterThanS7Status()
  {
    OCFees ocFees;
    const OptionalServicesInfo& s7 =
        S7Builder(&_memHandle).withCarrier("UA").withStatus(1).buildRef();

    _bagTvl->setCarrier(s7.carrier());
    _bagTvl->_frequentFlyerTierLevel = 9;

    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatus(s7, ocFees));
  }

  void testFrequentFlyerTier_OneProvided_SameCxr_GoodLevel()
  {
    OCFees ocFees;
    const OptionalServicesInfo& s7 =
        S7Builder(&_memHandle).withCarrier("UA").withStatus(3).buildRef();
    addFrequentFlyerStatus("UA", 2);

    _bagTvl->setCarrier(s7.carrier());
    _bagTvl->setFreqFlyerTierLevel(
        freqflyerutils::determineFreqFlyerTierLevel(nullptr,
                                                    _bagTvl->paxType()->freqFlyerTierWithCarrier(),
                                                    _bagTvl->getCarrier(),
                                                    _bagTvl->_trx));
    CPPUNIT_ASSERT(_validator->checkFrequentFlyerStatus(s7, ocFees));
  }

  void testFrequentFlyerTier_OneProvided_SameCxr_GoodLevel_NotActive()
  {
    _validator->_baggageTravel._trx->getRequest()->ticketingDT() = DateTime(2010, 1, 1);
    OCFees ocFees;
    const OptionalServicesInfo& s7 =
        S7Builder(&_memHandle).withCarrier("UA").withStatus(3).buildRef();
    addFrequentFlyerStatus("UA", 2);

    _bagTvl->setCarrier(s7.carrier());
    _bagTvl->setFreqFlyerTierLevel(
        freqflyerutils::determineFreqFlyerTierLevel(nullptr,
                                                    _bagTvl->paxType()->freqFlyerTierWithCarrier(),
                                                    _bagTvl->getCarrier(),
                                                    _bagTvl->_trx));
    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatus(s7, ocFees));
  }

  void testFrequentFlyerTier_OneProvided_SameCxr_BadLevel()
  {
    OCFees ocFees;
    CarrierCode carrier = "UA";
    const OptionalServicesInfo& s7 =
        S7Builder(&_memHandle).withCarrier(carrier).withStatus(3).buildRef();
    addFrequentFlyerStatus(carrier, 5);

    _bagTvl->setCarrier(s7.carrier());
    _bagTvl->setFreqFlyerTierLevel(
        freqflyerutils::determineFreqFlyerTierLevel(nullptr,
                                                    _bagTvl->paxType()->freqFlyerTierWithCarrier(),
                                                    _bagTvl->getCarrier(),
                                                    _bagTvl->_trx));
    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatus(s7, ocFees));
  }

  void testFrequentFlyerTier_MultipleProvided()
  {
    OCFees ocFees;
    const OptionalServicesInfo& s7 =
        S7Builder(&_memHandle).withCarrier("UA").withStatus(3).buildRef();
    addFrequentFlyerStatus("AI", 7, "LH", 1, "UA", 5);

    _bagTvl->setCarrier(s7.carrier());
    _bagTvl->setFreqFlyerTierLevel(
        freqflyerutils::determineFreqFlyerTierLevel(nullptr,
                                                    _bagTvl->paxType()->freqFlyerTierWithCarrier(),
                                                    _bagTvl->getCarrier(),
                                                    _bagTvl->_trx));
    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatus(s7, ocFees));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageChargesValidatorTest);
} // tse
