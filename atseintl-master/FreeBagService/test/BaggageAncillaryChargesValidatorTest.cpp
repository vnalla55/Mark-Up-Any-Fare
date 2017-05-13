#include "test/include/CppUnitHelperMacros.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/ServiceFeeUtil.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "FreeBagService/BaggageAncillaryChargesValidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
namespace
{
// MOCKS

class MyDataHandle : public DataHandleMock
{
public:
  MyDataHandle() {}

  const std::vector<SvcFeesSecurityInfo*>&
  getSvcFeesSecurity(const VendorCode& vendor, const int itemNo)
  {
    if (itemNo == 123)
      return *_memHandle.create<std::vector<SvcFeesSecurityInfo*> >();
    else
      return DataHandleMock::getSvcFeesSecurity(vendor, itemNo);
  }

private:
  TestMemHandle _memHandle;
};
}

class BaggageAncillaryChargesValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAncillaryChargesValidatorTest);

  CPPUNIT_TEST(testSelectFilter_filterDisabled_hardMatchDisabled);
  CPPUNIT_TEST(testSelectFilter_filterEnabled_hardMatchDisabled);
  CPPUNIT_TEST(testSelectFilter_filterEnabled_hardMatchEnabled);
  CPPUNIT_TEST(testSelectFilter_filterEnabled_hardMatchEnabled_subGroupCode0DG);
  CPPUNIT_TEST(testSelectFilter_PricingRq);
  CPPUNIT_TEST(testSelectFilter_subGroupCode0DG_PricingRq);

  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Pass_None);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Fail_Empty_FFStatus);

  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_Pass);
  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_Fail);

  CPPUNIT_TEST(testCheckFreeBagPieces_Pass);
  CPPUNIT_TEST(testCheckFreeBagPieces_Fail);

  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Blank);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NotAvailable);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NoEmd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Emd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NoBook_NoEmd);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NoBook_Emd);

  CPPUNIT_TEST(testCheckFeeApplication_NotAvail_Pass);
  CPPUNIT_TEST(testCheckFeeApplication_NotAvail_Fail_CurrencyTbl);
  CPPUNIT_TEST(testCheckFeeApplication_NotAvail_Fail_FFMileage);
  CPPUNIT_TEST(testCheckFeeApplication_NotAvail_Fail_ApplFee);
  CPPUNIT_TEST(testCheckFeeApplication_NoCurrency_C);
  CPPUNIT_TEST(testCheckFeeApplication_NoCurrency_P);
  CPPUNIT_TEST(testCheckFeeApplication_NoCurrency_H);
  CPPUNIT_TEST(testCheckFeeApplication_NoCurrency_Fail);
  CPPUNIT_TEST(testCheckFeeApplication_Currency_3);
  CPPUNIT_TEST(testCheckFeeApplication_Currency_4);
  CPPUNIT_TEST(testCheckFeeApplication_Currency_K);
  CPPUNIT_TEST(testCheckFeeApplication_Currency_F);
  CPPUNIT_TEST(testCheckFeeApplication_Currency_Fail);

  CPPUNIT_TEST(testCheckOccurrence);
  CPPUNIT_TEST(testCheckOccurrence_1st_blank);
  CPPUNIT_TEST(testCheckOccurrence_2nd_blank);
  CPPUNIT_TEST(testCheckOccurrence_both_blank);

  CPPUNIT_TEST(testGetLocForSfcValidation_Dot);
  CPPUNIT_TEST(testGetLocForSfcValidation_Not_Dot);

  CPPUNIT_TEST(testShouldProcessAdvPur);

  CPPUNIT_TEST(testCheckBaggageWeightUnit);

  CPPUNIT_TEST(testCheckTaxApplication_Pass_X);
  CPPUNIT_TEST(testCheckTaxApplication_Pass_Blank);
  CPPUNIT_TEST(testCheckTaxApplication_Fail);

  CPPUNIT_TEST(testCheckAvailability_Pass_Y);
  CPPUNIT_TEST(testCheckAvailability_Pass_N);
  CPPUNIT_TEST(testCheckAvailability_Fail);

  CPPUNIT_TEST(testCheckRuleBusterRuleMatchingFareClass_Pass);
  CPPUNIT_TEST(testCheckRuleBusterRuleMatchingFareClass_Fail);

  CPPUNIT_TEST(testCheckPassengerOccurrence_Pass);
  CPPUNIT_TEST(testCheckPassengerOccurrence_Fail_First);
  CPPUNIT_TEST(testCheckPassengerOccurrence_Fail_Last);

  CPPUNIT_TEST(testGetFareFromTrx_NoTrx);
  CPPUNIT_TEST(testGetFareFromTrx_EmptyFareMarket);
  CPPUNIT_TEST(testGetFareFromTrx_NoOneWayFares);
  CPPUNIT_TEST(testGetFareFromTrx_NoValidFares);
  CPPUNIT_TEST(testGetFareFromTrx_LargerAmount);
  CPPUNIT_TEST(testGetFareFromTrx_SmallerAmount);

  CPPUNIT_TEST(testCheckFare_Pass_EmptyPaxType);
  CPPUNIT_TEST(testCheckFare_Pass_Adult);
  CPPUNIT_TEST(testCheckFare_Fail_Directionality);
  CPPUNIT_TEST(testCheckFare_Fail_PrivateTariff);
  CPPUNIT_TEST(testCheckFare_Fail_NotPublished);
  CPPUNIT_TEST(testCheckFare_Fail_SpecialFare);
  CPPUNIT_TEST(testCheckFare_Fail_FareType);
  CPPUNIT_TEST(testCheckFare_Fail_PaxType);

  CPPUNIT_TEST(testGetFarePercentageAmount_Default);
  CPPUNIT_TEST(testGetFarePercentageAmount_H);
  CPPUNIT_TEST(testGetFarePercentageAmount_C);
  CPPUNIT_TEST(testGetFarePercentageAmount_P);

  CPPUNIT_TEST(testCheckSecurity_Pass_PublicPrivateInd);
  CPPUNIT_TEST(testCheckSecurity_Fail_PublicPrivateInd);
  CPPUNIT_TEST(testCheckSecurity_Fail_T183);
  CPPUNIT_TEST(testCheckSecurity_Pass_PublicPrivateInd_PricingRq);

  CPPUNIT_TEST(testFilterSegments_usDotVer2);
  CPPUNIT_TEST(testFilterSegments_nonUsDotVer2);
  CPPUNIT_TEST(testFilterSegments_usDotVer3);
  CPPUNIT_TEST(testFilterSegments_nonUsDotVer3);

  CPPUNIT_TEST_SUITE_END();

private:
  class PaxTypeFareBuilder
  {
  public:
    PaxTypeFareBuilder(FareMarket* market, PricingTrx& trx) : _market(market), _trx(trx)
    {
      _ptf = _memHandle.create<PaxTypeFare>();
      _ptf->fareClassAppSegInfo() = _memHandle.create<FareClassAppSegInfo>();

      _fare = _memHandle.create<Fare>();
      _fare->status().set(Fare::FS_ScopeIsDefined);

      _fareInfo = _memHandle.create<FareInfo>();
      _tcrInfo = _memHandle.create<TariffCrossRefInfo>();
      _fcaInfo = _memHandle.create<FareClassAppInfo>();
      _paxType = _memHandle.create<PaxType>();
    }

    PaxTypeFare* build()
    {
      _fare->setFareInfo(_fareInfo);
      _fare->setTariffCrossRefInfo(_tcrInfo);

      _ptf->fareClassAppInfo() = _fcaInfo;
      _ptf->initialize(_fare, _paxType, _market, _trx);

      return _ptf;
    }

    PaxTypeFareBuilder& withNucFareAmount(MoneyAmount amount)
    {
      _fare->_nucFareAmount = amount;
      return *this;
    }

    PaxTypeFareBuilder& withDirectionality(const Directionality& directionality)
    {
      _fareInfo->directionality() = directionality;
      return *this;
    }

    PaxTypeFareBuilder& publicTariff()
    {
      _tcrInfo->tariffCat() = 2;
      return *this;
    }

    PaxTypeFareBuilder& privateTariff()
    {
      _tcrInfo->tariffCat() = RuleConst::PRIVATE_TARIFF;
      return *this;
    }

    PaxTypeFareBuilder& publishedFare()
    {
      _fare->status().set(Fare::FS_PublishedFare);
      return *this;
    }

    PaxTypeFareBuilder& normalFare()
    {
      _fcaInfo->_pricingCatType = 'N';
      return *this;
    }

    PaxTypeFareBuilder& specialFare()
    {
      _fcaInfo->_pricingCatType = 'S';
      return *this;
    }

    PaxTypeFareBuilder& withFareType(const FareType& fareType)
    {
      _fcaInfo->_fareType = fareType;
      return *this;
    }

    PaxTypeFareBuilder& withPaxType(const PaxTypeCode& ptc)
    {
      _paxType->paxType() = ptc;
      return *this;
    }

    PaxTypeFareBuilder& withOwrt(const Indicator owrt)
    {
      _fareInfo->owrt() = owrt;
      return *this;
    }

    PaxTypeFareBuilder& withAmount(const MoneyAmount amount)
    {
      _fareInfo->fareAmount() = amount;
      return *this;
    }

    PaxTypeFareBuilder& withCarrier(const CarrierCode& carrier)
    {
      _fareInfo->carrier() = carrier;
      return *this;
    }

  private:
    TestMemHandle _memHandle;
    PaxTypeFare* _ptf;
    FareInfo* _fareInfo;
    TariffCrossRefInfo* _tcrInfo;
    Fare* _fare;
    FareClassAppInfo* _fcaInfo;
    PaxType* _paxType;
    FareMarket* _market;
    PricingTrx& _trx;
  };

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    MockGlobal::setMetricsMan(_memHandle.create<tse::MetricsMan>());

    _mdh = _memHandle.create<MyDataHandle>();
    _market = _memHandle.create<FareMarket>();
    _repricingTrx = _memHandle.create<RepricingTrx>();
    _repricingTrx->billing() = _memHandle.create<Billing>();
    _repricingTrx->fareMarket().push_back(_market);
    _validator = createValidator();
  }

  void tearDown()
  {
    MockGlobal::clear();
    _memHandle.clear();
  }

private:
  BaggageAncillaryChargesValidator* createValidator(bool isUsDot = false, int majorSchemaVersion = 2)
  {
    BaggageTravel* bagTvl = _memHandle.create<BaggageTravel>();
    return createValidator(*bagTvl, isUsDot, majorSchemaVersion);
  }

  BaggageAncillaryChargesValidator* createValidator(BaggageTravel& bagTvl, bool isUsDot, int majorSchemaVersion)
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = _memHandle.create<Itin>();
    farePath->itin()->setBaggageTripType(isUsDot ? BaggageTripType::TO_FROM_US
                                                 : BaggageTripType::OTHER);
    farePath->paxType() = _memHandle.create<PaxType>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->billing() = _memHandle.create<Billing>();
    PricingOptions* opts = _memHandle.create<PricingOptions>();
    opts->fareX() = false;
    AncRequest* ancRq = _memHandle.create<AncRequest>();
    ancRq->majorSchemaVersion() = majorSchemaVersion;
    ancRq->ticketingAgent() = _memHandle.create<Agent>();
    ancRq->ticketingAgent()->agentLocation() = _memHandle.create<Loc>();
    _trx->setRequest(ancRq);
    _trx->setOptions(opts);
    bagTvl.setupTravelData(*farePath);
    bagTvl._trx = _trx;

    CheckedPoint* checkedPoint = _memHandle.create<CheckedPoint>();
    Ts2ss* ts2ss = _memHandle.create<Ts2ss>();
    BagValidationOpt opt(bagTvl, *checkedPoint, *ts2ss, false, nullptr);
    return _memHandle.insert(new BaggageAncillaryChargesValidator(opt));
  }

  PaxTypeFareBuilder& createPaxTypeFareBuilder()
  {
    return *_memHandle.insert(new PaxTypeFareBuilder(_market, *_repricingTrx));
  }

  TestMemHandle _memHandle;
  DataHandleMock* _mdh;
  BaggageAncillaryChargesValidator* _validator;
  PricingTrx* _trx;
  FareMarket* _market;
  RepricingTrx* _repricingTrx;

  // TESTS
  void testSelectFilter_filterDisabled_hardMatchDisabled()
  {
    AncRequest* request = static_cast<AncRequest*>(_trx->getRequest());

    request->selectFirstChargeForOccurrence() = false;
    request->hardMatchIndicator() = false;

    CPPUNIT_ASSERT_EQUAL((BaggageAncillaryChargesValidator::S7Filter*)0,
                         _validator->selectS7Filter(""));
  }

  void testSelectFilter_filterEnabled_hardMatchDisabled()
  {
    AncRequest* request = static_cast<AncRequest*>(_trx->getRequest());

    request->selectFirstChargeForOccurrence() = true;
    request->hardMatchIndicator() = false;

    CPPUNIT_ASSERT_EQUAL((BaggageAncillaryChargesValidator::S7Filter*)0,
                         _validator->selectS7Filter(""));
  }

  void testSelectFilter_filterEnabled_hardMatchEnabled()
  {
    AncRequest* request = static_cast<AncRequest*>(_trx->getRequest());

    request->selectFirstChargeForOccurrence() = true;
    request->hardMatchIndicator() = true;

    CPPUNIT_ASSERT_EQUAL(
        (BaggageAncillaryChargesValidator::S7Filter*)&(_validator->_byOccurrenceFilter),
        _validator->selectS7Filter(""));
  }

  void testSelectFilter_filterEnabled_hardMatchEnabled_subGroupCode0DG()
  {
    AncRequest* request = static_cast<AncRequest*>(_trx->getRequest());

    request->selectFirstChargeForOccurrence() = true;
    request->hardMatchIndicator() = true;

    CPPUNIT_ASSERT_EQUAL(
        (BaggageAncillaryChargesValidator::S7Filter*)&(_validator->_byWeightFilter),
        _validator->selectS7Filter("0DG"));
  }

  void testSelectFilter_PricingRq()
  {
    AncRequest* request = static_cast<AncRequest*>(_trx->getRequest());
    request->selectFirstChargeForOccurrence() = false;
    request->hardMatchIndicator() = false;

    _validator->_isAncillaryBaggage = false;

    CPPUNIT_ASSERT_EQUAL(
        (BaggageAncillaryChargesValidator::S7Filter*)&(_validator->_byOccurrenceFilter),
        _validator->selectS7Filter(""));
  }

  void testSelectFilter_subGroupCode0DG_PricingRq()
  {
    AncRequest* request = static_cast<AncRequest*>(_trx->getRequest());
    request->selectFirstChargeForOccurrence() = false;
    request->hardMatchIndicator() = false;

    _validator->_isAncillaryBaggage = false;

    CPPUNIT_ASSERT_EQUAL(
        (BaggageAncillaryChargesValidator::S7Filter*)&(_validator->_byWeightFilter),
        _validator->selectS7Filter("0DG"));
  }

  void testCheckFrequentFlyerStatus_Pass_None()
  {
    OCFees ocFees;

    _validator->_allowSoftMatch = false;

    CPPUNIT_ASSERT(_validator->checkFrequentFlyerStatus(
        S7Builder(&_memHandle).withStatus(0).buildRef(), ocFees));
  }

  void testCheckFrequentFlyerStatus_Fail_Empty_FFStatus()
  {
    OCFees ocFees;

    _validator->_allowSoftMatch = false;

    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatus(
        S7Builder(&_memHandle).withStatus(1).buildRef(), ocFees));
  }

  void testCheckAdvPurchaseTktInd_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkAdvPurchaseTktInd(
        S7Builder(&_memHandle)
            .withAdvPurch(BaggageAncillaryChargesValidator::CHAR_BLANK)
            .buildRef()));
  }

  void testCheckAdvPurchaseTktInd_Fail()
  {
    CPPUNIT_ASSERT(
        !_validator->checkAdvPurchaseTktInd(S7Builder(&_memHandle).withAdvPurch('X').buildRef()));
  }

  void testCheckFreeBagPieces_Pass()
  {
    CPPUNIT_ASSERT(
        _validator->checkFreeBagPieces(S7Builder(&_memHandle).withBaggagePcs(-1).buildRef()));
  }

  void testCheckFreeBagPieces_Fail()
  {
    CPPUNIT_ASSERT(
        !_validator->checkFreeBagPieces(S7Builder(&_memHandle).withBaggagePcs(0).buildRef()));
  }

  void testCheckServiceNotAvailNoCharge_Defer()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(
        FAIL_S7_NOT_AVAIL_NO_CHANGE,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_Blank()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(PASS_S7,
                         _validator->checkServiceNotAvailNoCharge(
                             S7Builder(&_memHandle)
                                 .withNotAvailNoCharge(BaggageAncillaryChargesValidator::CHAR_BLANK)
                                 .withApplication('C')
                                 .buildRef(),
                             ocFees));
  }

  void testCheckServiceNotAvailNoCharge_NotAvailable()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_NOT_AVAILABLE)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_NoEmd()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_FREE_NO_EMD_ISSUED)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_Emd()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_FREE_EMD_ISSUED)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_NoBook_NoEmd()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_FREE_NO_BOOK_NO_EMD)
                .buildRef(),
            ocFees));
  }

  void testCheckServiceNotAvailNoCharge_NoBook_Emd()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(
                     BaggageAncillaryChargesValidator::SERVICE_FREE_NO_BOOK_EMD_ISSUED)
                .buildRef(),
            ocFees));
  }

  void testCheckFeeApplication_NotAvail_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_NOT_AVAILABLE)
            .buildRef()));
  }

  void testCheckFeeApplication_NotAvail_Fail_CurrencyTbl()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_NOT_AVAILABLE)
            .withFeesCurr(1)
            .buildRef()));
  }

  void testCheckFeeApplication_NotAvail_Fail_FFMileage()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_NOT_AVAILABLE)
            .withApplication('X')
            .buildRef()));
  }

  void testCheckFeeApplication_NotAvail_Fail_ApplFee()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(BaggageAncillaryChargesValidator::SERVICE_NOT_AVAILABLE)
            .withAppFee(1)
            .buildRef()));
  }

  void testCheckFeeApplication_NoCurrency_C()
  {
    CPPUNIT_ASSERT(
        _validator->checkFeeApplication(S7Builder(&_memHandle).withApplication('C').buildRef()));
  }

  void testCheckFeeApplication_NoCurrency_P()
  {
    CPPUNIT_ASSERT(
        _validator->checkFeeApplication(S7Builder(&_memHandle).withApplication('P').buildRef()));
  }

  void testCheckFeeApplication_NoCurrency_H()
  {
    CPPUNIT_ASSERT(
        _validator->checkFeeApplication(S7Builder(&_memHandle).withApplication('H').buildRef()));
  }

  void testCheckFeeApplication_NoCurrency_Fail()
  {
    CPPUNIT_ASSERT(
        !_validator->checkFeeApplication(S7Builder(&_memHandle).withApplication('X').buildRef()));
  }

  void testCheckFeeApplication_Currency_3()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle).withFeesCurr(1).withApplication('3').buildRef()));
  }

  void testCheckFeeApplication_Currency_4()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle).withFeesCurr(1).withApplication('4').buildRef()));
  }

  void testCheckFeeApplication_Currency_K()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle).withFeesCurr(1).withApplication('K').buildRef()));
  }

  void testCheckFeeApplication_Currency_F()
  {
    CPPUNIT_ASSERT(_validator->checkFeeApplication(
        S7Builder(&_memHandle).withFeesCurr(1).withApplication('F').buildRef()));
  }

  void testCheckFeeApplication_Currency_Fail()
  {
    CPPUNIT_ASSERT(!_validator->checkFeeApplication(
        S7Builder(&_memHandle).withFeesCurr(1).withApplication('X').buildRef()));
  }

  void testCheckOccurrence()
  {
    CPPUNIT_ASSERT(
        _validator->checkOccurrence(S7Builder(&_memHandle).withBaggageOccurrence(2, 4).buildRef()));
  }

  void testCheckOccurrence_1st_blank()
  {
    CPPUNIT_ASSERT(!_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(-1, 4).buildRef()));
  }

  void testCheckOccurrence_2nd_blank()
  {
    CPPUNIT_ASSERT(_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(4, -1).buildRef()));
  }

  void testCheckOccurrence_both_blank()
  {
    CPPUNIT_ASSERT(_validator->checkOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(-1, -1).buildRef()));
  }

  void testGetLocForSfcValidation_Dot()
  {
    BaggageAncillaryChargesValidator* validator = createValidator(true);

    validator->_baggageTravel.itin()->travelSeg().push_back(
        AirSegBuilder(&_memHandle).withOrigin("ABC").build());

    const Loc& returnedLoc = validator->getLocForSfcValidation();

    CPPUNIT_ASSERT_EQUAL(std::string("ABC"), std::string(returnedLoc.loc()));
  }

  void testGetLocForSfcValidation_Not_Dot()
  {
    BaggageAncillaryChargesValidator* validator = createValidator(false);
    Loc loc;
    loc.loc() = "DEF";
    Agent agent;
    agent.agentLocation() = &loc;
    _trx->getRequest()->ticketingAgent() = &agent;

    const Loc& returnedLoc = validator->getLocForSfcValidation();

    CPPUNIT_ASSERT_EQUAL(std::string("DEF"), std::string(returnedLoc.loc()));
  }

  void testShouldProcessAdvPur()
  {
    CPPUNIT_ASSERT(_validator->shouldProcessAdvPur(S7Builder(&_memHandle).buildRef()));
  }

  void testCheckBaggageWeightUnit()
  {
    CPPUNIT_ASSERT(_validator->checkBaggageWeightUnit(S7Builder(&_memHandle).buildRef()));
  }

  void testCheckTaxApplication_Pass_X()
  {
    CPPUNIT_ASSERT(
        _validator->checkTaxApplication(S7Builder(&_memHandle).withTaxIncl('X').buildRef()));
  }

  void testCheckTaxApplication_Pass_Blank()
  {
    CPPUNIT_ASSERT(_validator->checkTaxApplication(
        S7Builder(&_memHandle).withTaxIncl(OptionalServicesValidator::CHAR_BLANK).buildRef()));
  }

  void testCheckTaxApplication_Fail()
  {
    CPPUNIT_ASSERT(
        !_validator->checkTaxApplication(S7Builder(&_memHandle).withTaxIncl('Z').buildRef()));
  }

  void testCheckAvailability_Pass_Y()
  {
    CPPUNIT_ASSERT(_validator->checkAvailability(S7Builder(&_memHandle).withAvail('Y').buildRef()));
  }

  void testCheckAvailability_Pass_N()
  {
    CPPUNIT_ASSERT(_validator->checkAvailability(S7Builder(&_memHandle).withAvail('N').buildRef()));
  }

  void testCheckAvailability_Fail()
  {
    CPPUNIT_ASSERT(
        !_validator->checkAvailability(S7Builder(&_memHandle).withAvail('Z').buildRef()));
  }

  void testCheckRuleBusterRuleMatchingFareClass_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkRuleBusterRuleMatchingFareClass(
        S7Builder(&_memHandle).withRuleBuster("").buildRef()));
  }

  void testCheckRuleBusterRuleMatchingFareClass_Fail()
  {
    CPPUNIT_ASSERT(!_validator->checkRuleBusterRuleMatchingFareClass(
        S7Builder(&_memHandle).withRuleBuster("XYZ").buildRef()));
  }

  void testCheckPassengerOccurrence_Pass()
  {
    CPPUNIT_ASSERT(_validator->checkPassengerOccurrence(
        S7Builder(&_memHandle).withOccurrence(0, 0).buildRef()));
  }

  void testCheckPassengerOccurrence_Fail_First()
  {
    CPPUNIT_ASSERT(!_validator->checkPassengerOccurrence(
        S7Builder(&_memHandle).withOccurrence(1, 0).buildRef()));
  }

  void testCheckPassengerOccurrence_Fail_Last()
  {
    CPPUNIT_ASSERT(!_validator->checkPassengerOccurrence(
        S7Builder(&_memHandle).withOccurrence(0, 1).buildRef()));
  }

  void testGetFareFromTrx_NoTrx() { CPPUNIT_ASSERT(!_validator->getFareFromTrx(0)); }

  void testGetFareFromTrx_EmptyFareMarket()
  {
    CPPUNIT_ASSERT(!_validator->getFareFromTrx(_repricingTrx));
  }

  void testGetFareFromTrx_NoOneWayFares()
  {
    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(FROM)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ROUND_TRIP_MAYNOT_BE_HALVED)
                                            .withAmount(100)
                                            .build());

    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(FROM)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ROUND_TRIP_MAYNOT_BE_HALVED)
                                            .withAmount(200)
                                            .build());

    CPPUNIT_ASSERT(!_validator->getFareFromTrx(_repricingTrx));
  }

  void testGetFareFromTrx_NoValidFares()
  {
    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(TO)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ONE_WAY_MAYNOT_BE_DOUBLED)
                                            .withAmount(100)
                                            .build());

    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(TO)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ONE_WAY_MAYNOT_BE_DOUBLED)
                                            .withAmount(200)
                                            .build());

    CPPUNIT_ASSERT(!_validator->getFareFromTrx(_repricingTrx));
  }

  void testGetFareFromTrx_LargerAmount()
  {
    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(FROM)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ONE_WAY_MAYNOT_BE_DOUBLED)
                                            .withAmount(100)
                                            .build());

    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(FROM)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ONE_WAY_MAYNOT_BE_DOUBLED)
                                            .withAmount(200)
                                            .build());

    const PaxTypeFare* ptf = _validator->getFareFromTrx(_repricingTrx);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200), ptf->fareAmount());
  }

  void testGetFareFromTrx_SmallerAmount()
  {
    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(FROM)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ONE_WAY_MAYNOT_BE_DOUBLED)
                                            .withAmount(100)
                                            .build());

    _market->allPaxTypeFare().push_back(createPaxTypeFareBuilder()
                                            .withDirectionality(TO)
                                            .publicTariff()
                                            .publishedFare()
                                            .normalFare()
                                            .withFareType("EXX")
                                            .withPaxType("")
                                            .withOwrt(ONE_WAY_MAYNOT_BE_DOUBLED)
                                            .withAmount(200)
                                            .build());

    const PaxTypeFare* ptf = _validator->getFareFromTrx(_repricingTrx);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(100), ptf->fareAmount());
  }

  void testCheckFare_Pass_EmptyPaxType()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withNucFareAmount(10.0)
                           .withDirectionality(FROM)
                           .publicTariff()
                           .publishedFare()
                           .normalFare()
                           .withFareType("EXX")
                           .withPaxType("")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(_validator->checkFare(ptf));
  }

  void testCheckFare_Pass_Adult()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withNucFareAmount(10.0)
                           .withDirectionality(FROM)
                           .publicTariff()
                           .publishedFare()
                           .normalFare()
                           .withFareType("EXX")
                           .withPaxType("ADT")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(_validator->checkFare(ptf));
  }

  void testCheckFare_Fail_Directionality()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withDirectionality(TO)
                           .publicTariff()
                           .publishedFare()
                           .normalFare()
                           .withFareType("EXX")
                           .withPaxType("")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(!_validator->checkFare(ptf));
  }

  void testCheckFare_Fail_PrivateTariff()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withDirectionality(FROM)
                           .privateTariff()
                           .publishedFare()
                           .normalFare()
                           .withFareType("EXX")
                           .withPaxType("")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(!_validator->checkFare(ptf));
  }

  void testCheckFare_Fail_NotPublished()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withDirectionality(FROM)
                           .publicTariff()
                           .normalFare()
                           .withFareType("EXX")
                           .withPaxType("")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(!_validator->checkFare(ptf));
  }

  void testCheckFare_Fail_SpecialFare()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withDirectionality(FROM)
                           .publicTariff()
                           .publishedFare()
                           .specialFare()
                           .withFareType("EXX")
                           .withPaxType("")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(!_validator->checkFare(ptf));
  }

  void testCheckFare_Fail_FareType()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withDirectionality(FROM)
                           .publicTariff()
                           .publishedFare()
                           .normalFare()
                           .withFareType("XXX")
                           .withPaxType("")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(!_validator->checkFare(ptf));
  }

  void testCheckFare_Fail_PaxType()
  {
    PaxTypeFare* ptf = createPaxTypeFareBuilder()
                           .withDirectionality(FROM)
                           .publicTariff()
                           .publishedFare()
                           .normalFare()
                           .withFareType("EXX")
                           .withPaxType("INF")
                           .withCarrier("AA")
                           .build();

    CPPUNIT_ASSERT(!_validator->checkFare(ptf));
  }

  void testGetFarePercentageAmount_Default()
  {
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(0),
                         _validator->getFarePercentageAmount(
                             1000, S7Builder(&_memHandle).withApplication('X').buildRef()));
  }

  void testGetFarePercentageAmount_H()
  {
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(5),
                         _validator->getFarePercentageAmount(
                             1000, S7Builder(&_memHandle).withApplication('H').buildRef()));
  }

  void testGetFarePercentageAmount_C()
  {
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(10),
                         _validator->getFarePercentageAmount(
                             1000, S7Builder(&_memHandle).withApplication('C').buildRef()));
  }

  void testGetFarePercentageAmount_P()
  {
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(15),
                         _validator->getFarePercentageAmount(
                             1000, S7Builder(&_memHandle).withApplication('P').buildRef()));
  }

  void testCheckSecurity_Pass_PublicPrivateInd()
  {
    CPPUNIT_ASSERT(_validator->checkSecurity(S7Builder(&_memHandle).withPubPriv('X').buildRef()));
  }

  void testCheckSecurity_Fail_PublicPrivateInd()
  {
    CPPUNIT_ASSERT(
        !_validator->checkSecurity(S7Builder(&_memHandle)
                                       .withPubPriv(OptionalServicesValidator::T183_SCURITY_PRIVATE)
                                       .buildRef()));
  }

  void testCheckSecurity_Fail_T183()
  {
    CPPUNIT_ASSERT(!_validator->checkSecurity(S7Builder(&_memHandle).withFeesSec(123).buildRef()));
  }

  void testCheckSecurity_Pass_PublicPrivateInd_PricingRq()
  {
    _validator->_isAncillaryBaggage = false;

    CPPUNIT_ASSERT(_validator->checkSecurity(S7Builder(&_memHandle).withPubPriv('X').buildRef()));
  }

  void testFilterSegments_usDotVer2()
  {
    std::vector<TravelSeg*> segments;
    for(int i = 0; i < 5; ++i)
      segments.push_back(AirSegBuilder(&_memHandle).withOrigin("ABC").build());

    BaggageTravel& bagTvl = *(_memHandle.create<BaggageTravel>());
    bagTvl.updateSegmentsRange(segments.begin(), segments.end());
    BaggageAncillaryChargesValidator& usDotValidatorVer2_noCheckedSegments = *createValidator(bagTvl, true, 2);
    _trx->modifiableActivationFlags().setAB240(false);

    usDotValidatorVer2_noCheckedSegments.filterSegments();

    CPPUNIT_ASSERT(usDotValidatorVer2_noCheckedSegments._segI == segments.begin());
    CPPUNIT_ASSERT(usDotValidatorVer2_noCheckedSegments._segIE == segments.end());
    CPPUNIT_ASSERT(usDotValidatorVer2_noCheckedSegments.hasSegments() == true);
    segments[1]->setCheckedPortionOfTravelInd('T');
    segments[2]->setCheckedPortionOfTravelInd('T');
    segments[4]->setCheckedPortionOfTravelInd('T');

    bagTvl.updateSegmentsRange(segments.begin(), segments.end());

    BaggageAncillaryChargesValidator& usDotValidatorVer2 = *createValidator(bagTvl, true, 2);
    _trx->modifiableActivationFlags().setAB240(false);
    usDotValidatorVer2.filterSegments();

    CPPUNIT_ASSERT(usDotValidatorVer2._segI == segments.begin());
    CPPUNIT_ASSERT(usDotValidatorVer2._segIE == segments.end());
    CPPUNIT_ASSERT(usDotValidatorVer2.hasSegments() == true);
  }

  void testFilterSegments_nonUsDotVer2()
  {
    std::vector<TravelSeg*> segments;
    for(int i = 0; i < 5; ++i)
      segments.push_back(AirSegBuilder(&_memHandle).withOrigin("ABC").build());

    BaggageTravel& bagTvl = *(_memHandle.create<BaggageTravel>());
    bagTvl.updateSegmentsRange(segments.begin(), segments.end());
    BaggageAncillaryChargesValidator& nonUsDotValidatorVer2_noCheckedSegments = *createValidator(bagTvl, false, 2);
    _trx->modifiableActivationFlags().setAB240(false);

    nonUsDotValidatorVer2_noCheckedSegments.filterSegments();

    CPPUNIT_ASSERT(nonUsDotValidatorVer2_noCheckedSegments._segI == segments.begin());
    CPPUNIT_ASSERT(nonUsDotValidatorVer2_noCheckedSegments._segIE == segments.end());
    CPPUNIT_ASSERT(nonUsDotValidatorVer2_noCheckedSegments.hasSegments() == true);

    segments[1]->setCheckedPortionOfTravelInd('T');
    segments[2]->setCheckedPortionOfTravelInd('T');
    segments[4]->setCheckedPortionOfTravelInd('T');

    bagTvl.updateSegmentsRange(segments.begin(), segments.end());
    BaggageAncillaryChargesValidator& nonUsDotValidatorVer2 = *createValidator(bagTvl, false, 2);
    _trx->modifiableActivationFlags().setAB240(false);

    nonUsDotValidatorVer2.filterSegments();

    CPPUNIT_ASSERT(nonUsDotValidatorVer2._segI == segments.begin());
    CPPUNIT_ASSERT(nonUsDotValidatorVer2._segIE == segments.end());
    CPPUNIT_ASSERT(nonUsDotValidatorVer2.hasSegments() == true);
  }

  void testFilterSegments_usDotVer3()
  {
    std::vector<TravelSeg*> segments;
    for(int i = 0; i < 5; ++i)
      segments.push_back(AirSegBuilder(&_memHandle).withOrigin("ABC").build());

    BaggageTravel& bagTvl = *(_memHandle.create<BaggageTravel>());

    bagTvl.updateSegmentsRange(segments.begin(), segments.end());
    BaggageAncillaryChargesValidator& usDotValidatorVer3_noCheckedSegments = *createValidator(bagTvl, true, 3);
    _trx->modifiableActivationFlags().setAB240(true);

    usDotValidatorVer3_noCheckedSegments.filterSegments();

    CPPUNIT_ASSERT(usDotValidatorVer3_noCheckedSegments._segI == segments.begin());
    CPPUNIT_ASSERT(usDotValidatorVer3_noCheckedSegments._segIE == segments.end());
    CPPUNIT_ASSERT(usDotValidatorVer3_noCheckedSegments.hasSegments() == true);

    segments[1]->setCheckedPortionOfTravelInd('T');
    segments[2]->setCheckedPortionOfTravelInd('T');
    segments[4]->setCheckedPortionOfTravelInd('T');

    _trx->modifiableActivationFlags().setAB240(true);
    bagTvl.updateSegmentsRange(segments.begin(), segments.end());

    BaggageAncillaryChargesValidator& usDotValidatorVer3 = *createValidator(bagTvl, true, 3);
    usDotValidatorVer3.filterSegments();

    CPPUNIT_ASSERT(usDotValidatorVer3._segI == segments.begin());
    CPPUNIT_ASSERT(usDotValidatorVer3._segIE == segments.end());
    CPPUNIT_ASSERT(usDotValidatorVer3.hasSegments() == true);
  }

  void testFilterSegments_nonUsDotVer3()
  {
    std::vector<TravelSeg*> segments;
    for(int i = 0; i < 5; ++i)
      segments.push_back(AirSegBuilder(&_memHandle).withOrigin("ABC").build());

    BaggageTravel& bagTvl = *(_memHandle.create<BaggageTravel>());

    bagTvl.updateSegmentsRange(segments.begin(), segments.end());

    BaggageAncillaryChargesValidator& nonUsDotValidatorVer3_noCheckedSegments = *createValidator(bagTvl, false, 3);
    _trx->modifiableActivationFlags().setAB240(true);

    nonUsDotValidatorVer3_noCheckedSegments.filterSegments();

    CPPUNIT_ASSERT(nonUsDotValidatorVer3_noCheckedSegments._segI == segments.end());
    CPPUNIT_ASSERT(nonUsDotValidatorVer3_noCheckedSegments._segIE == segments.end());
    CPPUNIT_ASSERT(nonUsDotValidatorVer3_noCheckedSegments.hasSegments() == false);

    segments[1]->setCheckedPortionOfTravelInd('T');
    segments[2]->setCheckedPortionOfTravelInd('T');
    segments[4]->setCheckedPortionOfTravelInd('T');

    bagTvl.updateSegmentsRange(segments.begin(), segments.end());

    BaggageAncillaryChargesValidator& nonUsDotValidatorVer3 = *createValidator(bagTvl, false, 3);
    _trx->modifiableActivationFlags().setAB240(true);
    nonUsDotValidatorVer3.filterSegments();

    CPPUNIT_ASSERT(nonUsDotValidatorVer3._segI == segments.begin() + 1);
    CPPUNIT_ASSERT(nonUsDotValidatorVer3._segIE == segments.begin() + 3);
    CPPUNIT_ASSERT(nonUsDotValidatorVer3.hasSegments() == true);

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAncillaryChargesValidatorTest);

} // tse
