#include <boost/assign/std/vector.hpp>

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigMan.h"
#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "Xform/RequestXmlValidator.h"

using namespace boost::assign;
using namespace std;

namespace tse
{
namespace
{
class MaxPenaltyInfoDataHandleMock : public DataHandleMock
{
  PaxTypeInfo _paxTypeInfo;

public:
  const PaxTypeInfo* getPaxType(const PaxTypeCode& /*paxTypeCode*/, const VendorCode& /*vendor*/)
  {
    return &_paxTypeInfo;
  }
};
} // namespace

class RequestXmlValidatorStub : public RequestXmlValidator
{
public:
  RequestXmlValidatorStub()
    : RequestXmlValidator(), _zeroLoc(false), _timeDiff(0), _zeroNation(false), _zeroPtc(false)
  {
  }

  ~RequestXmlValidatorStub() {}
  bool _zeroLoc;
  short _timeDiff;
  Loc _loc;
  bool _zeroNation;
  Nation _nation;
  bool _zeroPtc;
  PaxTypeInfo _paxTypeInfo;

  short getTimeDiff(PricingTrx* trx, const DateTime& time) { return _timeDiff; }

  void getLocationCurrency(PricingTrx* trx)
  {
    trx->getRequest()->ticketingAgent()->currencyCodeAgent() = USD;
  }

  const Loc* getLocation(PricingTrx* trx, const LocCode& locCode)
  {
    if (_zeroLoc)
      return 0;
    return &_loc;
  }

  const Nation* getNation(PricingTrx* trx, const NationCode& nationCode)
  {
    if (_zeroNation)
      return 0;
    return &_nation;
  }

  const PaxTypeInfo* getPaxType(PricingTrx* trx, const PaxTypeCode& paxType)
  {
    if (_zeroPtc)
      return 0;
    return &_paxTypeInfo;
  }
};
class RequestXmlValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RequestXmlValidatorTest);

  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForRepriceTrx);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForExchangeTrx);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForAltPricingTrx);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForNoPnrPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForCommandPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForFareFamilyPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForRTWPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForCat35NetPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForTicketingEntry);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForAxessUser);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForDApricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForDPpricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForTktDesignatorPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForFutureDatePricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForPastDatePricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForASPricing);
  CPPUNIT_TEST(testValidateMultiTicketThrowsExceptionForMultiTicketNotActive);
  CPPUNIT_TEST(testValidateMultiTicketForMultiTicketActive);
  CPPUNIT_TEST(testValidateMultiTicketForMultiTicketNotActive);

  CPPUNIT_TEST(testConvertNumToStringReturnNumberCovertedToString);
  CPPUNIT_TEST(testGetAirlineCrsCodePuts1SinAgent);
  CPPUNIT_TEST(testGetAirlinePartitionCurrencyGetsEmptyCurrencyWhenAgentLocationZero);
  CPPUNIT_TEST(testGetAirlinePartitionCurrencyGetsUSDWhenAgentLocationUS);
  CPPUNIT_TEST(testGetSubscriberCrsCodeGetsFromTjrForSabre);
  CPPUNIT_TEST(testGetSubscriberCrsCodeGetsFromTjrForAxess);
  CPPUNIT_TEST(testGetSubscriberCrsCodeGetsFromTjrForAbacus);
  CPPUNIT_TEST(testGetSubscriberCurrencyGetsFromTjr);
  CPPUNIT_TEST(testGetSubscriberLocationDontGetLocationWhenAgentCityNotEmpty);
  CPPUNIT_TEST(testGetSubscriberLocationGetLocationFromCustomerWhenEmpty);
  CPPUNIT_TEST(testEntryFromSubscriberReturnFalseIfEntryFromAirline);
  CPPUNIT_TEST(testEntryFromSubscriberReturnTrueIfEntryFromAgency);
  CPPUNIT_TEST(testValidateLocThrowErrorExceptionWhenEntryNotFromPO);
  CPPUNIT_TEST(testValidateLocThrowNonFatalErrorResponseExceptionWhenEntryFromPO);
  CPPUNIT_TEST(testSameOrigDestThrowsExceptionWhenOrigDestSame);
  CPPUNIT_TEST(testSameOrigDestDontThrowsExceptionWhenOrigDestDiff);
  CPPUNIT_TEST(testValidateSaleTicketOverrideDontThrowsExceptionForNonWqTrx);
  CPPUNIT_TEST(
      testValidateSaleTicketOverrideDontThrowsExceptionForWqTrxWhenSaleTicketOverrideEmpty);
  CPPUNIT_TEST(testValidateSaleTicketOverrideThrowsExceptionWhenSaleOverrideInvalid);
  CPPUNIT_TEST(testValidateSaleTicketOverrideThrowsExceptionWhenTicketOverrideInvalid);
  CPPUNIT_SKIP_TEST(testValidateCurrencyCodeGetsFromTJRWhenOverrideEmpty);
  CPPUNIT_TEST(testSetBookingDateSetsBookingDateSameAsTicketingDate);
  CPPUNIT_TEST(testSetTicketingDateUnchangedWhenNotTimeZoneDiff);
  CPPUNIT_TEST(testSetTicketingDateAdjustTicketingDateWhenTimeZoneDiff);
  CPPUNIT_TEST(testRequestFromPOReturnFalseWhenRequestNotFromPO);
  CPPUNIT_TEST(testRequestFromPOReturnTrueWhenRequestFromPO);
  CPPUNIT_TEST(testValidateShipRegistryDontThrowsExceptionWhenShipRegistryEmpty);
  CPPUNIT_TEST(testValidateShipRegistryCountryDontThrowsExceptionWhenCountryValid);
  CPPUNIT_TEST(testValidateShipRegistryCountryThrowsExceptionWhenCountryInvalid);
  CPPUNIT_TEST(testValidateShipRegistryPaxTypeDontThrowsExceptionWhenSeaExists);
  CPPUNIT_TEST(testValidateShipRegistryPaxTypeThrowsExceptionWhenSeaNotExist);
  CPPUNIT_TEST(testValidateShipRegistryPaxTypeDontThrowsExceptionWhenSeaExistsWithOther);
  CPPUNIT_TEST(testValidateShipRegistryPaxTypeThrowsExceptionWhenSeaNotExistWithOther);
  CPPUNIT_TEST(testValidateDepartureDateDontThrowsExceptionWhenTvlDatesAreInSequence);
  CPPUNIT_TEST(testValidateDepartureDateThrowsExceptionWhenTvlDatesAreOutOfSequence);
  CPPUNIT_TEST(
      testValidateDepartureDateDontThrowsExceptionWhenDatesAreInSequenceAndPreviousHasNoPssDate);
  CPPUNIT_TEST(testValidateDepartureDateDontThrowsExceptionWhenThereIsOnlyOneTravelSeg);
  CPPUNIT_TEST(testValidateDepartureDateDontThrowsExceptionWhenCurrentTravelSegHasNoPssDate);
  CPPUNIT_TEST(testValidateTicketingDateDontThrowsExceptionWhenTktDateEarlierThanTvlDate);
  CPPUNIT_TEST(testValidateTicketingDateDontThrowsExceptionWhenTktDateEqualToTvlDate);
  CPPUNIT_TEST(testValidateTicketingDateThrowsExceptionWhenTktDateLaterThanTvlDate);
  CPPUNIT_TEST(testValidateNoPNRPricingDateThrowsExceptionWhenTvlDateBeyond331Days);
  CPPUNIT_TEST(testValidatePassengerTypeDontThrowsExceptionWhenValidPassengerType);
  CPPUNIT_TEST(testValidatePassengerTypeThrowsExceptionWhenInvalidPassengerType);

  CPPUNIT_TEST(validateGenParmForPassengerStatusTrueWhenStatusMissing);
  CPPUNIT_TEST(validateGenParmForPassengerStatusTrueWhenResidencyIsCity);
  CPPUNIT_TEST(validateGenParmForPassengerStatusTrueWhenResidencyIsCountry);
  CPPUNIT_TEST(validateGenParmForPassengerStatusTrueWhenResidencyIsCountryState);
  CPPUNIT_TEST(validateGenParmForPassengerStatusFalseWhenStatusOnly);
  CPPUNIT_TEST(validateGenParmForPassengerStatusFalseWhenCountryOnly);
  CPPUNIT_TEST(validateGenParmForPassengerStatusFalseWhenStateOnly);
  CPPUNIT_TEST(validateGenParmForPassengerStatusFalseWhenCityOnly);
  CPPUNIT_TEST(validateGenParmForPassengerStatusFalseWhenStatusAndCountryAndCity);
  CPPUNIT_TEST(validateIsValidCountryCodeFalse);
  CPPUNIT_TEST(validateIsValidCountryCodeTrue);
  CPPUNIT_TEST(validatePassengerStatusResidencyFalseWhenInvalidCountryCode);
  CPPUNIT_TEST(validatePassengerStatusResidencyTrueWhenValidCountryCode);
  CPPUNIT_TEST(validatePassengerStatusResidencyTrueWhenValidCountryState);
  CPPUNIT_TEST(validatePassengerStatusResidencyFalseWhenInvalidCityCode);
  CPPUNIT_TEST(validatePassengerStatusResidencyTrueWhenValidCityCode);
  CPPUNIT_TEST(validatePassengerStatusResidencyFalseWhenCountryCityMissing);

  CPPUNIT_TEST(validatePassengerStatusNationalityFalseWhenCountryMissing);
  CPPUNIT_TEST(validatePassengerStatusNationalityFalseWhenState);
  CPPUNIT_TEST(validatePassengerStatusNationalityFalseWhenCountryAndState);
  CPPUNIT_TEST(validatePassengerStatusNationalityFalseWhenCountryAndCity);
  CPPUNIT_TEST(validatePassengerStatusNationalityTrueWhenValidCountry);
  CPPUNIT_TEST(validatePassengerStatusNationalityFalseWhenInvalidCountry);

  CPPUNIT_TEST(validatePassengerStatusEmployeeFalseWhenCountryMissing);
  CPPUNIT_TEST(validatePassengerStatusEmployeeFalseWhenCity);
  CPPUNIT_TEST(validatePassengerStatusEmployeeFalseWhenCountryAndCity);
  CPPUNIT_TEST(validatePassengerStatusEmployeeFalseWhenInvalidCountry);
  CPPUNIT_TEST(validatePassengerStatusEmployeeTrueWhenValidCountry);

  CPPUNIT_TEST(testSetMOverrideReturnTrueWhenCurrencyOverrideExists);
  CPPUNIT_TEST(testSetMOverrideReturnFalseWhenCurrencyOverrideNotExist);
  CPPUNIT_TEST(testCheckCominationPDOorPDRorXRSOptionalParameters1);
  CPPUNIT_TEST(testCheckCominationPDOorPDRorXRSOptionalParameters2);
  CPPUNIT_TEST(testCheckCominationPDOorPDRorXRSOptionalParameters3);
  CPPUNIT_TEST(testCheckCominationPDOorPDRorXRSOptionalParametersNoThrowError);

  CPPUNIT_TEST(testValidateETicketOptionStatusWhenEtktIsOnInCustomerTbl);
  CPPUNIT_TEST(testValidateETicketOptionStatusWhenEtktIsOffInCustomerTbl);

  CPPUNIT_TEST(testGetDefaultPaxTypeWhenPaxTypeIsEmptyInCustomerTbl);
  CPPUNIT_TEST(testGetDefaultPaxTypeWhenPaxTypeIsValidInCustomerTbl);
  CPPUNIT_TEST(testGetDefaultPaxTypeWhenPaxTypeIsInvalidInCustomerTbl);

  CPPUNIT_TEST(testProcessMissingArunkSegForPOWhenNoPNRPricingTrxIsFalse);
  CPPUNIT_TEST(testNeedToAddArunkSegmentWhenEmptyVectorIsFalse);
  CPPUNIT_TEST(testNeedToAddArunkSegmentWhenCurrentSegmentIsArunkIsFalse);
  CPPUNIT_TEST(testNeedToAddArunkSegmentWhenPrevSegmentIsArunkIsFalse);
  CPPUNIT_TEST(testNeedToAddArunkSegmentWhenNoArunkIsFalse);
  CPPUNIT_TEST(testNeedToAddArunkSegmentWhenArunkIsTrue);
  CPPUNIT_TEST(testNeedToAddArunkSegmentWhenSameAirPortIsFalse);

  CPPUNIT_TEST(testValidateFareQuoteCurrencyIndicatorWhenFareQuoteCurIsOffInCustomerTbl);
  CPPUNIT_TEST(testValidateFareQuoteCurrencyIndicatorWhenFareQuoteCurIsOnInCustomerTbl);

  CPPUNIT_TEST(testGetIataDontChangeAgentIataWhenAlreadyPopulated);
  CPPUNIT_TEST(testGetIataGetsFromCustomer);
  CPPUNIT_TEST(testSetForcedStopoverForNoPnrPricingWhenNotForceConx);
  CPPUNIT_TEST(testSetForcedStopoverForNoPnrPricingWhenForceConx);
  CPPUNIT_TEST(testSetForcedStopoverForNoPnrPricingWhenForceConxInLastSegment);

  CPPUNIT_TEST(testValidateItinForLanHistoricalDontThrowsExceptionWhenThereIsOnlyOneTravelSeg);
  CPPUNIT_SKIP_TEST(testValidateItinForLanHistoricalDontThrowsExceptionWhenTvlDatesAreInSequence);
  CPPUNIT_SKIP_TEST(testValidateItinForLanHistoricalThrowsExceptionWhenTvlDatesAreNotInSequence);
  CPPUNIT_SKIP_TEST(testValidateItinForLanHistoricalDontThrowsExceptionWhenArunkTravelSeg);
  CPPUNIT_SKIP_TEST(testValidateItinForLanHistoricalDontThrowsExceptionWhenOpenTravelSeg);
  CPPUNIT_TEST(testValidateItinForLanHistoricalThrowsExceptionWhenFlownAfterUnflownSeg);
  CPPUNIT_TEST(testValidateItinForLanHistoricalDontThrowsExceptionWhenUnfownAfterFlownSeg);

  CPPUNIT_TEST(testValidatePassengerTypeWithAgesThrowsExceptionWhenWrongPaxTypeLength);
  CPPUNIT_TEST(testValidatePassengerTypeWithAgesWhenPaxTypeIsNotAlpha2Digits);
  CPPUNIT_TEST(testValidatePassengerTypeWithAgesWhenPaxTypeIsAlpha2Digits);
  CPPUNIT_TEST(testValidatePassengerTypeWithAgesThrowsExceptionWhenAlpha2GigitsNotForSabre);
  CPPUNIT_TEST(testValidatePassengerTypeWithAgesThrowsExceptionWhenAlpha2GigitsForInfant);
  CPPUNIT_TEST(testValidatePassengerTypeThrowsExceptionWhenInvalidPaxTypeForSabre);
  CPPUNIT_TEST(testValidateMaxPenaltyInfo);

  CPPUNIT_TEST(testSetOperatingCarrierCodeForNonAirSegment);
  CPPUNIT_TEST(testSetOperatingCarrierCodeForAirSegWhenOprtCodeIsNotEmtpy);
  CPPUNIT_TEST(testSetOperatingCarrierCodeForAirSegWhenOprtCodeIsEmtpyNoFlight);
  CPPUNIT_TEST(testSetOperatingCarrierCodeForAirSegWhenOprtCodeIsEmtpyWithFlight);
  CPPUNIT_TEST(testValidateCxrOverrideThrowsExceptionWhenCXRdoesnotmachTvlSeg);

  CPPUNIT_TEST(testValidateSideTrips_ABCBD);
  CPPUNIT_TEST(testValidateSideTrips_ABC_BD);
  CPPUNIT_TEST(testValidateSideTrips_AB_CBD);
  CPPUNIT_TEST(testValidateSideTrips_ABC_DBD);
  CPPUNIT_TEST(testValidateSideTrips_ABCBCBD);
  CPPUNIT_TEST(testValidateSideTrips_ABCB_CBD);
  CPPUNIT_TEST(testValidateSideTrips_ABCB_DEDF);
  CPPUNIT_TEST(testValidateSideTrips_ABCD_fail);
  CPPUNIT_TEST(testValidateSideTrips_ABCBD_fail);
  CPPUNIT_TEST(testValidateSideTrips_ABC_DE_fail);
  CPPUNIT_TEST(testSetTimeZoneDiffBTWOrigAndPointOfSale);

  CPPUNIT_TEST(testRequestCrossDependenciesForAmountDiscountAndPlusUp);
  CPPUNIT_TEST(testRequestCrossDependenciesForPercentageDiscountAndPlusUp);
  CPPUNIT_TEST(testRequestCrossDependenciesForPercentageAndAmountMixed);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    DateTime todaysDate = DateTime::localTime();
    _request.ticketingDT() = todaysDate;
    _request.ticketingAgent() = &_agent;
    _trx.setRequest(&_request);
    TestConfigInitializer::setValue("PROJECT_331_PLUS_DAYS_ENABLED", "Y", "PRICING_SVC");
  }

  void tearDown() { _memHandle.clear(); }

  void testValidateMultiTicketThrowsExceptionForRepriceTrx()
  {
    Billing bill;
    bill.actionCode() = "WPR";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::REPRICING_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForExchangeTrx()
  {
    Billing bill;
    bill.actionCode() = "WPEX";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForAltPricingTrx()
  {
    Billing bill;
    bill.actionCode() = "WPA";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.altTrxType() = PricingTrx::WPA;
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForNoPnrPricing()
  {
    Billing bill;
    bill.actionCode() = "WQ";
    _trx.billing() = &bill;
    _trx.noPNRPricing() = true;
    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForCommandPricing()
  {
    Billing bill;
    bill.actionCode() = "WPQ";
    _trx.billing() = &bill;

    PricingOptions opt;
    opt.fbcSelected() = true;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForFareFamilyPricing()
  {
    Billing bill;
    bill.actionCode() = "WPT/";
    _trx.billing() = &bill;

    PricingOptions opt;
    opt.fareFamilyType() = 'N'; // either N/S/T
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForRTWPricing()
  {
    Billing bill;
    bill.actionCode() = "WPRW";
    _trx.billing() = &bill;

    PricingOptions opt;
    opt.setRtw(true);
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForCat35NetPricing()
  {
    Billing bill;
    bill.actionCode() = "FP";
    _trx.billing() = &bill;

    PricingOptions opt;
    opt.cat35Net() = 'T';
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForTicketingEntry()
  {
    Billing bill;
    bill.actionCode() = "W$";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    _request.ticketEntry() = 'T';
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForAxessUser()
  {
    Billing bill;
    bill.actionCode() = "WP";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    Customer tjr;
    tjr.crsCarrier() = AXESS_MULTIHOST_ID;
    tjr.hostName() = "AXES";
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForDApricing()
  {
    Billing bill;
    bill.actionCode() = "WPQ";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    MoneyAmount amount = 100;
    _request.addDiscAmount('1', '1', amount, "USD");

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForDPpricing()
  {
    Billing bill;
    bill.actionCode() = "WPQ";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    _request.discPercentages().insert(std::pair<int16_t, Percent>('1', 20.0));

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForTktDesignatorPricing()
  {
    Billing bill;
    bill.actionCode() = "WPQ";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    _request.specifiedTktDesignator().insert(std::pair<int16_t, TktDesignator>('1', "SABRE"));

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForFutureDatePricing()
  {
    Billing bill;
    bill.actionCode() = "WPB";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.addDays(5);
    _request.ticketingDT() = travelDate1;

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForPastDatePricing()
  {
    Billing bill;
    bill.actionCode() = "WPB";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.subtractDays(3);
    _request.ticketingDT() = travelDate1;

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForASPricing()
  {
    Billing bill;
    bill.actionCode() = "WP";
    bill.partitionID() = "AA";
    bill.aaaCity() = "DFW";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketThrowsExceptionForMultiTicketNotActive()
  {
    Billing bill;
    bill.actionCode() = "WP";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    Customer tjr;
    tjr.crsCarrier() = SABRE_MULTIHOST_ID;
    tjr.hostName() = "SABR";
    tjr.pricingApplTag4() = 'N';
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateMultiTicketRequest(&_trx),
                         NonFatalErrorResponseException);
  }

  void testValidateMultiTicketForMultiTicketActive()
  {
    Billing bill;
    bill.actionCode() = "WP";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    Customer tjr;
    tjr.crsCarrier() = SABRE_MULTIHOST_ID;
    tjr.hostName() = "SABR";
    tjr.pricingApplTag4() = 'Y';
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _request.multiTicketActive() = false;
    _reqXmlValidator.validateMultiTicketRequest(&_trx);
    CPPUNIT_ASSERT(_request.multiTicketActive());
  }

  void testValidateMultiTicketForMultiTicketNotActive()
  {
    Billing bill;
    bill.actionCode() = "WP";
    _trx.billing() = &bill;

    PricingOptions opt;
    _trx.setOptions(&opt);
    _trx.setTrxType(PricingTrx::PRICING_TRX);
    Customer tjr;
    tjr.crsCarrier() = SABRE_MULTIHOST_ID;
    tjr.hostName() = "SABR";
    tjr.pricingApplTag4() = 'N';
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _request.diagnosticNumber() = Diagnostic198;
    _request.multiTicketActive() = false;
    _reqXmlValidator.validateMultiTicketRequest(&_trx);
    CPPUNIT_ASSERT(!_request.multiTicketActive());
  }

  void testConvertNumToStringReturnNumberCovertedToString()
  {
    int16_t num = 1;
    std::string expected = " 1";
    CPPUNIT_ASSERT_EQUAL(expected, _reqXmlValidator.convertNumToString(num));
    num = 12;
    expected = "12";
    CPPUNIT_ASSERT_EQUAL(expected, _reqXmlValidator.convertNumToString(num));
    num = 123;
    expected = "123";
    CPPUNIT_ASSERT_EQUAL(expected, _reqXmlValidator.convertNumToString(num));
  }

  void testGetAirlineCrsCodePuts1SinAgent()
  {
    _reqXmlValidator.getAirlineCrsCode(&_trx);
    CarrierCode expected = SABRE_MULTIHOST_ID;
    CPPUNIT_ASSERT_EQUAL(expected, _trx.getRequest()->ticketingAgent()->cxrCode());
  }

  void testGetAirlinePartitionCurrencyGetsEmptyCurrencyWhenAgentLocationZero()
  {
    _trx.getRequest()->ticketingAgent()->agentLocation() = 0;
    _reqXmlValidator.getAirlinePartitionCurrency(&_trx);
    CPPUNIT_ASSERT(_trx.getRequest()->ticketingAgent()->currencyCodeAgent().empty());
  }

  void testGetAirlinePartitionCurrencyGetsUSDWhenAgentLocationUS()
  {
    Loc loc;
    _trx.getRequest()->ticketingAgent()->agentLocation() = &loc;
    RequestXmlValidatorStub rxvStub;
    rxvStub.getAirlinePartitionCurrency(&_trx);
    string usd = USD;
    CPPUNIT_ASSERT_EQUAL(usd, _trx.getRequest()->ticketingAgent()->currencyCodeAgent());
  }

  void testGetSubscriberCrsCodeGetsFromTjrForSabre()
  {
    Customer tjr;
    tjr.crsCarrier() = SABRE_MULTIHOST_ID;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.getSubscriberCrsCode(&_trx);
    CarrierCode expectedCrsCode = _trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier();
    string expectedVendorCrsCode = "";
    CPPUNIT_ASSERT_EQUAL(expectedCrsCode, _trx.getRequest()->ticketingAgent()->cxrCode());
    CPPUNIT_ASSERT_EQUAL(expectedVendorCrsCode,
                         _trx.getRequest()->ticketingAgent()->vendorCrsCode());
  }

  void testGetSubscriberCrsCodeGetsFromTjrForAxess()
  {
    Customer tjr;
    tjr.crsCarrier() = AXESS_MULTIHOST_ID;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.getSubscriberCrsCode(&_trx);
    CarrierCode expectedCrsCode = SABRE_MULTIHOST_ID;
    string expectedVendorCrsCode = _trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier();
    CPPUNIT_ASSERT_EQUAL(expectedCrsCode, _trx.getRequest()->ticketingAgent()->cxrCode());
    CPPUNIT_ASSERT_EQUAL(expectedVendorCrsCode,
                         _trx.getRequest()->ticketingAgent()->vendorCrsCode());
  }

  void testGetSubscriberCrsCodeGetsFromTjrForAbacus()
  {
    Customer tjr;
    tjr.crsCarrier() = ABACUS_MULTIHOST_ID;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.getSubscriberCrsCode(&_trx);
    CarrierCode expectedCrsCode = _trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier();
    string expectedVendorCrsCode = _trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier();
    CPPUNIT_ASSERT_EQUAL(expectedCrsCode, _trx.getRequest()->ticketingAgent()->cxrCode());
    CPPUNIT_ASSERT_EQUAL(expectedVendorCrsCode,
                         _trx.getRequest()->ticketingAgent()->vendorCrsCode());
  }

  void testGetSubscriberCurrencyGetsFromTjr()
  {
    Customer tjr;
    tjr.defaultCur() = USD;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.getSubscriberCurrency(&_trx);
    string expected = _trx.getRequest()->ticketingAgent()->agentTJR()->defaultCur();
    CPPUNIT_ASSERT_EQUAL(expected, _trx.getRequest()->ticketingAgent()->currencyCodeAgent());
  }

  void testGetSubscriberLocationDontGetLocationWhenAgentCityNotEmpty()
  {
    Customer tjr;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _trx.getRequest()->ticketingAgent()->agentCity() = LOC_BWI;
    _reqXmlValidator.getSubscriberLocation(&_trx);
    const Loc* expected = 0;
    CPPUNIT_ASSERT_EQUAL(expected, _trx.getRequest()->ticketingAgent()->agentLocation());
  }

  void testGetSubscriberLocationGetLocationFromCustomerWhenEmpty()
  {
    Customer tjr;
    tjr.aaCity() = LOC_BWI;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _trx.getRequest()->ticketingAgent()->agentCity().clear();

    RequestXmlValidatorStub rxvStub;
    rxvStub.getSubscriberLocation(&_trx);
    const Loc* expected = &(rxvStub._loc);
    CPPUNIT_ASSERT_EQUAL(expected, _trx.getRequest()->ticketingAgent()->agentLocation());
    CPPUNIT_ASSERT_EQUAL(tjr.aaCity(), _trx.getRequest()->ticketingAgent()->agentCity());
  }

  void testEntryFromSubscriberReturnFalseIfEntryFromAirline()
  {
    _trx.getRequest()->ticketingAgent()->tvlAgencyPCC().clear();
    CPPUNIT_ASSERT(!_reqXmlValidator.entryFromSubscriber(&_trx));
  }

  void testEntryFromSubscriberReturnTrueIfEntryFromAgency()
  {
    Customer tjr;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "W0H3";
    CPPUNIT_ASSERT(_reqXmlValidator.entryFromSubscriber(&_trx));
  }

  void testValidateLocThrowErrorExceptionWhenEntryNotFromPO()
  {
    Billing bill;
    _trx.billing() = &bill;
    try
    {
      int16_t num = 1;
      _reqXmlValidator.validateLoc(&_trx, num);
    }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (ErrorResponseException& errorEx) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateLocThrowNonFatalErrorResponseExceptionWhenEntryFromPO()
  {
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    NoPNRPricingTrx wqTrx;
    wqTrx.billing() = &bill;
    try
    {
      int16_t num = 1;
      _reqXmlValidator.validateLoc(&wqTrx, num);
    }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (ErrorResponseException& errorEx) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testSameOrigDestThrowsExceptionWhenOrigDestSame()
  {
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    NoPNRPricingTrx wqTrx;
    wqTrx.billing() = &bill;
    AirSeg airSeg;
    airSeg.origAirport() = airSeg.destAirport() = LOC_BWI;
    try { _reqXmlValidator.validateSameOrigDest(&wqTrx, &airSeg); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testSameOrigDestDontThrowsExceptionWhenOrigDestDiff()
  {
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    NoPNRPricingTrx wqTrx;
    wqTrx.billing() = &bill;
    AirSeg airSeg;
    airSeg.origAirport() = LOC_BWI;
    airSeg.destAirport() = LOC_NYC;
    try { _reqXmlValidator.validateSameOrigDest(&wqTrx, &airSeg); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateSaleTicketOverrideDontThrowsExceptionForNonWqTrx()
  {
    try { _reqXmlValidator.validateSaleTicketOverride(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateSaleTicketOverrideDontThrowsExceptionForWqTrxWhenSaleTicketOverrideEmpty()
  {
    NoPNRPricingTrx wqTrx;
    _request.salePointOverride().clear();
    _request.ticketPointOverride().clear();
    wqTrx.setRequest(&_request);
    try { _reqXmlValidator.validateSaleTicketOverride(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateSaleTicketOverrideThrowsExceptionWhenSaleOverrideInvalid()
  {
    NoPNRPricingTrx wqTrx;
    _request.salePointOverride() = "INVALID";
    _request.ticketPointOverride().clear();
    wqTrx.setRequest(&_request);
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroLoc = true;
    try { rxvStub.validateSaleTicketOverride(&wqTrx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateSaleTicketOverrideThrowsExceptionWhenTicketOverrideInvalid()
  {
    NoPNRPricingTrx wqTrx;
    _request.salePointOverride().clear();
    _request.ticketPointOverride() = "INVALID";
    wqTrx.setRequest(&_request);
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroLoc = true;
    try { rxvStub.validateSaleTicketOverride(&wqTrx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateCurrencyCodeGetsFromTJRWhenOverrideEmpty()
  {
    PricingOptions opt;
    opt.currencyOverride().clear();
    _trx.setOptions(&opt);
    _trx.getRequest()->ticketingAgent()->currencyCodeAgent() = USD;
    _reqXmlValidator.validateCurrencyCode(&_trx);
    string expected = USD;
    string actual = opt.currencyOverride();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testSetBookingDateSetsBookingDateSameAsTicketingDate()
  {
    DateTime ticketingDate(2009, 10, 10, 4, 10, 10);
    _trx.getRequest()->ticketingDT() = ticketingDate;
    AirSeg airSeg;
    _reqXmlValidator.setBookingDate(&_trx, &airSeg);
    CPPUNIT_ASSERT_EQUAL(_trx.getRequest()->ticketingDT(), airSeg.bookingDT());
  }

  void testSetTicketingDateUnchangedWhenNotTimeZoneDiff()
  {
    DateTime ticketingDate(2009, 10, 10, 4, 10, 10);
    _trx.getRequest()->ticketingDT() = ticketingDate;
    RequestXmlValidatorStub rxvStub;
    rxvStub.setTicketingDate(&_trx, false);
    CPPUNIT_ASSERT_EQUAL(ticketingDate, _trx.getRequest()->ticketingDT());
  }

  void testSetTicketingDateAdjustTicketingDateWhenTimeZoneDiff()
  {
    DateTime ticketingDate(2009, 10, 10, 4, 10, 10);
    _trx.getRequest()->ticketingDT() = ticketingDate;
    RequestXmlValidatorStub rxvStub;
    rxvStub._timeDiff = 30;
    rxvStub.setTicketingDate(&_trx, false);
    DateTime expectedDate = ticketingDate.addSeconds(30 * 60);
    CPPUNIT_ASSERT_EQUAL(expectedDate, _trx.getRequest()->ticketingDT());
  }

  void testRequestFromPOReturnFalseWhenRequestNotFromPO()
  {
    Billing bill;
    _trx.billing() = &bill;
    CPPUNIT_ASSERT(!_reqXmlValidator.requestFromPo(&_trx));
  }

  void testRequestFromPOReturnTrueWhenRequestFromPO()
  {
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    _trx.billing() = &bill;
    CPPUNIT_ASSERT(_reqXmlValidator.requestFromPo(&_trx));
  }

  void testValidateShipRegistryDontThrowsExceptionWhenShipRegistryEmpty()
  {
    PricingOptions opt;
    opt.fareByRuleShipRegistry().clear();
    _trx.setOptions(&opt);
    try { _reqXmlValidator.validateShipRegistry(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateShipRegistryCountryDontThrowsExceptionWhenCountryValid()
  {
    PricingOptions opt;
    opt.fareByRuleShipRegistry() = UNITED_STATES;
    _trx.setOptions(&opt);
    RequestXmlValidatorStub rxvStub;
    try { rxvStub.validateShipRegistryCountry(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateShipRegistryCountryThrowsExceptionWhenCountryInvalid()
  {
    PricingOptions opt;
    opt.fareByRuleShipRegistry() = "ABCD";
    _trx.setOptions(&opt);
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = true;
    try { rxvStub.validateShipRegistryCountry(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateShipRegistryPaxTypeDontThrowsExceptionWhenSeaExists()
  {
    PaxType paxType;
    paxType.paxType() = SEA;
    _trx.paxType().push_back(&paxType);
    try { _reqXmlValidator.validateShipRegistryPaxType(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateShipRegistryPaxTypeThrowsExceptionWhenSeaNotExist()
  {
    PaxType paxType;
    paxType.paxType() = MIL;
    _trx.paxType().push_back(&paxType);
    try { _reqXmlValidator.validateShipRegistryPaxType(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateShipRegistryPaxTypeDontThrowsExceptionWhenSeaExistsWithOther()
  {
    PaxType paxType1, paxType2;
    paxType1.paxType() = MIL;
    paxType2.paxType() = SEA;
    _trx.paxType().push_back(&paxType1);
    _trx.paxType().push_back(&paxType2);
    try { _reqXmlValidator.validateShipRegistryPaxType(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateShipRegistryPaxTypeThrowsExceptionWhenSeaNotExistWithOther()
  {
    PaxType paxType1, paxType2;
    paxType1.paxType() = MIL;
    paxType2.paxType() = ADULT;
    _trx.paxType().push_back(&paxType1);
    _trx.paxType().push_back(&paxType2);
    try { _reqXmlValidator.validateShipRegistryPaxType(&_trx); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateDepartureDateDontThrowsExceptionWhenTvlDatesAreInSequence()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg1, airSeg2;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.addDays(5);
    DateTime travelDate2 = todayDate.addDays(7);
    airSeg1.departureDT() = travelDate1;
    airSeg2.departureDT() = travelDate2;
    airSeg1.pssDepartureDate() = "2010-05-01";
    airSeg2.pssDepartureDate() = "2010-05-03";
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    DateTime ticketingDate = todayDate;
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);

    try { rxvStub.validateDepartureDate(&wqTrx, &itin, itin.travelSeg()[1], 1); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateDepartureDateThrowsExceptionWhenTvlDatesAreOutOfSequence()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg1, airSeg2;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.addDays(7);
    DateTime travelDate2 = todayDate.addDays(5);
    airSeg1.departureDT() = travelDate1;
    airSeg2.departureDT() = travelDate2;
    airSeg1.pssDepartureDate() = "2010-05-03";
    airSeg2.pssDepartureDate() = "2010-05-01";
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    DateTime ticketingDate = todayDate;
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);

    try { rxvStub.validateDepartureDate(&wqTrx, &itin, itin.travelSeg()[1], 1); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateDepartureDateDontThrowsExceptionWhenDatesAreInSequenceAndPreviousHasNoPssDate()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.addDays(5);
    DateTime travelDate2 = todayDate.addDays(6);
    DateTime travelDate3 = todayDate.addDays(5);
    airSeg1.departureDT() = travelDate1;
    airSeg2.departureDT() = travelDate2;
    airSeg3.departureDT() = travelDate3;
    airSeg1.pssDepartureDate() = "2010-05-01";
    airSeg3.pssDepartureDate() = "2010-05-01";
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    DateTime ticketingDate = todayDate;
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);
    try { rxvStub.validateDepartureDate(&wqTrx, &itin, itin.travelSeg()[2], 2); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateDepartureDateDontThrowsExceptionWhenThereIsOnlyOneTravelSeg()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg1;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.addDays(5);
    airSeg1.departureDT() = travelDate1;
    airSeg1.pssDepartureDate() = "2010-05-01";
    itin.travelSeg().push_back(&airSeg1);
    DateTime ticketingDate = todayDate;
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);
    try { rxvStub.validateDepartureDate(&wqTrx, &itin, itin.travelSeg()[0], 0); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateDepartureDateDontThrowsExceptionWhenCurrentTravelSegHasNoPssDate()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate1 = todayDate.addDays(5);
    DateTime travelDate2 = todayDate.addDays(6);
    DateTime travelDate3 = todayDate.addDays(5);
    airSeg1.departureDT() = travelDate1;
    airSeg2.departureDT() = travelDate2;
    airSeg3.departureDT() = travelDate3;
    airSeg1.pssDepartureDate() = "2010-05-01";
    airSeg3.pssDepartureDate() = "2010-05-01";
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    DateTime ticketingDate = todayDate;
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);
    try { rxvStub.validateDepartureDate(&wqTrx, &itin, itin.travelSeg()[1], 1); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateTicketingDateDontThrowsExceptionWhenTktDateEarlierThanTvlDate()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate = todayDate.addDays(5);
    airSeg.departureDT() = travelDate;
    itin.travelSeg().push_back(&airSeg);

    DateTime ticketingDate = todayDate.addDays(4);
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);
    try { rxvStub.validateTicketingDate(&wqTrx, &itin); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateTicketingDateDontThrowsExceptionWhenTktDateEqualToTvlDate()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate = todayDate.addDays(5);
    airSeg.departureDT() = travelDate;
    itin.travelSeg().push_back(&airSeg);

    DateTime ticketingDate = todayDate.addDays(5);
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);
    try { rxvStub.validateTicketingDate(&wqTrx, &itin); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
  }

  void testValidateTicketingDateThrowsExceptionWhenTktDateLaterThanTvlDate()
  {
    RequestXmlValidatorStub rxvStub;
    NoPNRPricingTrx wqTrx;
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    wqTrx.billing() = &bill;
    Itin itin;
    AirSeg airSeg;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate = todayDate.addDays(4);
    airSeg.departureDT() = travelDate;
    itin.travelSeg().push_back(&airSeg);

    DateTime ticketingDate = todayDate.addDays(5);
    _request.ticketingDT() = ticketingDate;
    wqTrx.setRequest(&_request);
    try { rxvStub.validateTicketingDate(&wqTrx, &itin); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidateNoPNRPricingDateThrowsExceptionWhenTvlDateBeyond331Days()
  {
    Billing bill;
    bill.requestPath() = PSS_PO_ATSE_PATH;
    _trx.billing() = &bill;
    RequestXmlValidatorStub rxvStub;
    DateTime todayDate = DateTime::localTime();
    DateTime travelDate = todayDate.addDays(RequestXmlValidator::NO_PNR_FUTURE_DAY + 1);
    try { rxvStub.validateTicketingDate(&_trx, travelDate, 1, false); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testValidatePassengerTypeDontThrowsExceptionWhenValidPassengerType()
  {
    NoPNRPricingTrx wqTrx;
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroPtc = false;
    PaxType paxType;
    paxType.paxType() = "XXX";
    try { rxvStub.validatePassengerType(&wqTrx, paxType); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(false); }
    catch (...) { CPPUNIT_ASSERT(false); }
    CPPUNIT_ASSERT(true);
    uint16_t expectedNum = 1;
    CPPUNIT_ASSERT_EQUAL(expectedNum, paxType.number());
  }

  void testValidatePassengerTypeThrowsExceptionWhenInvalidPassengerType()
  {
    NoPNRPricingTrx wqTrx;
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroPtc = true;
    PaxType paxType;
    paxType.paxType() = "XXX";
    try { rxvStub.validatePassengerType(&wqTrx, paxType); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void validateIsValidCountryCodeFalse()
  {
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = true;
    CPPUNIT_ASSERT(!rxvStub.isValidCountryCode(&_trx, _countryCode));
  }

  void validateIsValidCountryCodeTrue()
  {
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = false;
    CPPUNIT_ASSERT(rxvStub.isValidCountryCode(&_trx, _countryCode));
  }

  void validatePassengerStatusResidencyFalseWhenInvalidCityCode()
  {

    _residency = "DFW";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroLoc = true;
    CPPUNIT_ASSERT(!rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusResidencyTrueWhenValidCityCode()
  {

    _residency = "DFW";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroLoc = false;
    CPPUNIT_ASSERT(rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusResidencyTrueWhenValidCountryState()
  {

    _countryCode = "RU";
    _stateRegionCode = "TX";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = false;
    CPPUNIT_ASSERT(rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusResidencyTrueWhenValidCountryCode()
  {

    _countryCode = "RU";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = false;
    CPPUNIT_ASSERT(rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusResidencyFalseWhenInvalidCountryCode()
  {

    _countryCode = "RU";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = true;
    CPPUNIT_ASSERT(!rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusResidencyFalseWhenCountryCityMissing()
  {

    CPPUNIT_ASSERT(
        !_reqXmlValidator.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusEmployeeFalseWhenCountryMissing()
  {

    CPPUNIT_ASSERT(
        !_reqXmlValidator.validatePassengerStatusEmployee(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusEmployeeFalseWhenCity()
  {

    _residency = "TXX";
    CPPUNIT_ASSERT(
        !_reqXmlValidator.validatePassengerStatusEmployee(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusEmployeeFalseWhenCountryAndCity()
  {

    _countryCode = "RU";
    _residency = "TXX";
    CPPUNIT_ASSERT(
        !_reqXmlValidator.validatePassengerStatusEmployee(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusEmployeeFalseWhenInvalidCountry()
  {

    _countryCode = "RU";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = true;
    CPPUNIT_ASSERT(!rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusEmployeeTrueWhenValidCountry()
  {

    _countryCode = "RU";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = false;
    CPPUNIT_ASSERT(rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusNationalityFalseWhenCountryMissing()
  {

    CPPUNIT_ASSERT(!_reqXmlValidator.validatePassengerStatusNationality(
        &_trx, _countryCode, _stateRegionCode, _residency));
  }

  void validatePassengerStatusNationalityFalseWhenState()
  {

    _stateRegionCode = "TX";
    CPPUNIT_ASSERT(!_reqXmlValidator.validatePassengerStatusNationality(
        &_trx, _countryCode, _stateRegionCode, _residency));
  }

  void validatePassengerStatusNationalityFalseWhenCountryAndState()
  {

    _countryCode = "RU";
    _stateRegionCode = "TX";
    CPPUNIT_ASSERT(!_reqXmlValidator.validatePassengerStatusNationality(
        &_trx, _countryCode, _stateRegionCode, _residency));
  }

  void validatePassengerStatusNationalityFalseWhenCountryAndCity()
  {

    _countryCode = "RU";
    _residency = "TXX";
    CPPUNIT_ASSERT(!_reqXmlValidator.validatePassengerStatusNationality(
        &_trx, _countryCode, _stateRegionCode, _residency));
  }

  void validatePassengerStatusNationalityFalseWhenInvalidCountry()
  {

    _countryCode = "RU";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = true;
    CPPUNIT_ASSERT(!rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validatePassengerStatusNationalityTrueWhenValidCountry()
  {

    _countryCode = "RU";
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroNation = false;
    CPPUNIT_ASSERT(rxvStub.validatePassengerStatusResidency(&_trx, _countryCode, _residency));
  }

  void validateGenParmForPassengerStatusFalseWhenStatusOnly()
  {

    _currentCRC = RequestXmlValidator::RESIDENCY;
    CPPUNIT_ASSERT(!_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusFalseWhenCountryAndStateOnly()
  {

    _countryCode = "US";
    _stateRegionCode = "TX";
    CPPUNIT_ASSERT(!_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusFalseWhenStateOnly()
  {

    _stateRegionCode = "TX";
    CPPUNIT_ASSERT(!_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusFalseWhenCountryOnly()
  {

    _countryCode = "RU";
    CPPUNIT_ASSERT(!_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusFalseWhenCityOnly()
  {

    _residency = "DFW";
    CPPUNIT_ASSERT(!_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusFalseWhenStatusAndCountryAndCity()
  {

    _currentCRC = RequestXmlValidator::RESIDENCY;
    _residency = "DFW";
    _countryCode = "RU";
    CPPUNIT_ASSERT(!_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void initializePassengerStatusCombination()
  {
    _countryCode.clear();
    _stateRegionCode.clear();
    _currentCRC.clear();
    _residency.clear();
  }

  void validateGenParmForPassengerStatusTrueWhenStatusMissing()
  {

    CPPUNIT_ASSERT(_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusTrueWhenResidencyIsCity()
  {

    _currentCRC = RequestXmlValidator::RESIDENCY;
    _residency = "DFW";
    CPPUNIT_ASSERT(_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusTrueWhenResidencyIsCountry()
  {

    _currentCRC = RequestXmlValidator::RESIDENCY;
    _countryCode = "US";
    CPPUNIT_ASSERT(_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void validateGenParmForPassengerStatusTrueWhenResidencyIsCountryState()
  {

    _currentCRC = RequestXmlValidator::RESIDENCY;
    _countryCode = "US";
    _stateRegionCode = "UU"; // there is no funcional code in ATSE V2 to check it.
    CPPUNIT_ASSERT(_reqXmlValidator.validateGenParmForPassengerStatus(
        _countryCode, _stateRegionCode, _currentCRC, _residency));
  }

  void testSetMOverrideReturnTrueWhenCurrencyOverrideExists()
  {
    PricingOptions opt;
    opt.currencyOverride() = USD;
    opt.mOverride() = ' ';
    _trx.setOptions(&opt);
    // Customer tjr;
    //_trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    //_trx.getRequest()->ticketingAgent()->agentTJR()->defaultCur() = opt.currencyOverride();
    _reqXmlValidator.setMOverride(&_trx);
    CPPUNIT_ASSERT(_trx.getOptions()->isMOverride());
  }

  void testSetMOverrideReturnFalseWhenCurrencyOverrideNotExist()
  {
    PricingOptions opt;
    opt.currencyOverride().clear();
    opt.mOverride() = ' ';
    _trx.setOptions(&opt);
    // Customer tjr;
    //_trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    //_trx.getRequest()->ticketingAgent()->agentTJR()->defaultCur() = NUC;
    //_reqXmlValidator.setMOverride(&_trx);
    CPPUNIT_ASSERT(!_trx.getOptions()->isMOverride());
  }

  void testCheckCominationPDOorPDRorXRSOptionalParameters1() // PDR and XRS
  {
    PricingOptions opt;
    opt.setPDRForFRRule(true);
    opt.setXRSForFRRule(true);
    _trx.setOptions(&opt);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.checkCominationPDOorPDRorXRSOptionalParameters(&_trx), ErrorResponseException);
  }

  void testCheckCominationPDOorPDRorXRSOptionalParameters2() // PDR or PDO
  {
    PricingOptions opt;
    opt.setPDRForFRRule(true);
    opt.setPDOForFRRule(true);
    _trx.setOptions(&opt);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.checkCominationPDOorPDRorXRSOptionalParameters(&_trx), ErrorResponseException);
  }

  void testCheckCominationPDOorPDRorXRSOptionalParameters3() // XRS or PDO
  {
    PricingOptions opt;
    opt.setPDOForFRRule(true);
    opt.setXRSForFRRule(true);
    _trx.setOptions(&opt);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.checkCominationPDOorPDRorXRSOptionalParameters(&_trx), ErrorResponseException);
  }

  void testCheckCominationPDOorPDRorXRSOptionalParametersNoThrowError()
  {
    PricingOptions opt;
    opt.setPDOForFRRule(false);
    opt.setPDRForFRRule(false);
    opt.setXRSForFRRule(true);
    _trx.setOptions(&opt);
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.checkCominationPDOorPDRorXRSOptionalParameters(&_trx));
  }

  void testValidateETicketOptionStatusWhenEtktIsOnInCustomerTbl()
  {
    Customer tjr;
    tjr.eTicketCapable() = 'Y';
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.validateETicketOptionStatus(&_trx);
    CPPUNIT_ASSERT(_trx.getRequest()->isElectronicTicket());
  }
  void testValidateETicketOptionStatusWhenEtktIsOffInCustomerTbl()
  {
    Customer tjr;
    tjr.eTicketCapable() = 'N';
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.validateETicketOptionStatus(&_trx);
    CPPUNIT_ASSERT(!_trx.getRequest()->isElectronicTicket());
  }

  void testGetDefaultPaxTypeWhenPaxTypeIsValidInCustomerTbl()
  {
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroPtc = false;

    Customer tjr;
    tjr.defaultPassengerType() = "MIL";
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    PaxType* pt = 0;
    pt = rxvStub.getDefaultPaxType(&_trx);
    CPPUNIT_ASSERT(pt->paxType() == "MIL");
  }

  void testGetDefaultPaxTypeWhenPaxTypeIsInvalidInCustomerTbl()
  {
    RequestXmlValidatorStub rxvStub;
    rxvStub._zeroPtc = true;

    Customer tjr;
    tjr.defaultPassengerType() = "GOV";
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    PaxType* pt = 0;
    pt = rxvStub.getDefaultPaxType(&_trx);
    CPPUNIT_ASSERT(pt->paxType() == "ADT");
  }

  void testGetDefaultPaxTypeWhenPaxTypeIsEmptyInCustomerTbl()
  {
    Customer tjr;
    // tjr.defaultPassengerType().clear();
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    PaxType* pt = 0;
    pt = _reqXmlValidator.getDefaultPaxType(&_trx);
    CPPUNIT_ASSERT(pt->paxType() == "ADT");
  }

  void testProcessMissingArunkSegForPOWhenNoPNRPricingTrxIsFalse()
  {
    Itin itin;
    AirSeg airSeg1;
    airSeg1.departureDT() = DateTime::localTime();
    bool isMissingArunkForPO = false;
    _reqXmlValidator.processMissingArunkSegForPO(&_trx, &itin, &airSeg1, isMissingArunkForPO);
    CPPUNIT_ASSERT(_trx.travelSeg().size() == 0);
  }

  void testNeedToAddArunkSegmentWhenEmptyVectorIsFalse()
  {
    AirSeg airSeg1;
    Itin itin;
    airSeg1.departureDT() = DateTime::localTime();
    CPPUNIT_ASSERT(!_reqXmlValidator.needToAddArunkSegment(&_trx, itin, airSeg1));
  }

  void testNeedToAddArunkSegmentWhenCurrentSegmentIsArunkIsFalse()
  {
    ArunkSeg arunkSegment;
    Itin itin;
    CPPUNIT_ASSERT(!_reqXmlValidator.needToAddArunkSegment(&_trx, itin, arunkSegment));
  }

  void testNeedToAddArunkSegmentWhenPrevSegmentIsArunkIsFalse()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    ArunkSeg arunkSegment;
    airSeg1.departureDT() = DateTime::localTime();
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&arunkSegment);
    CPPUNIT_ASSERT(!_reqXmlValidator.needToAddArunkSegment(&_trx, itin, airSeg2));
  }

  void testNeedToAddArunkSegmentWhenNoArunkIsFalse()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    airSeg2.boardMultiCity() = "CHI";
    airSeg1.offMultiCity() = "CHI";
    itin.travelSeg().push_back(&airSeg1);
    CPPUNIT_ASSERT(!_reqXmlValidator.needToAddArunkSegment(&_trx, itin, airSeg2));
  }

  void testNeedToAddArunkSegmentWhenArunkIsTrue()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    airSeg1.destAirport() = "IAD";
    airSeg1.offMultiCity() = "WAS";

    airSeg2.boardMultiCity() = "CHI";
    airSeg2.origAirport() = "ORD";
    itin.travelSeg().push_back(&airSeg1);
    CPPUNIT_ASSERT(_reqXmlValidator.needToAddArunkSegment(&_trx, itin, airSeg2));
  }

  void testNeedToAddArunkSegmentWhenSameAirPortIsFalse()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    airSeg1.offMultiCity() = "NYC";
    airSeg1.destAirport() = "EWR";
    airSeg2.boardMultiCity() = "EWR";
    airSeg2.origAirport() = "EWR";

    itin.travelSeg().push_back(&airSeg1);
    CPPUNIT_ASSERT(!_reqXmlValidator.needToAddArunkSegment(&_trx, itin, airSeg2));
  }

  void testValidateFareQuoteCurrencyIndicatorWhenFareQuoteCurIsOffInCustomerTbl()
  {
    FareDisplayTrx fqTrx;
    Customer tjr;
    tjr.fareQuoteCur() = 'N';
    Agent agentFQ;
    FareDisplayRequest requestFQ;
    FareDisplayOptions optFQ;

    requestFQ.ticketingAgent() = &agentFQ;
    fqTrx.setOptions(&optFQ);
    fqTrx.setRequest(&requestFQ);
    fqTrx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.validateFareQuoteCurrencyIndicator(&fqTrx);
    CPPUNIT_ASSERT(!fqTrx.getOptions()->isSellingCurrency());
  }

  void testValidateFareQuoteCurrencyIndicatorWhenFareQuoteCurIsOnInCustomerTbl()
  {
    FareDisplayTrx fqTrx;
    Customer tjr;
    tjr.fareQuoteCur() = 'Y';
    Agent agentFQ;
    FareDisplayRequest requestFQ;
    FareDisplayOptions optFQ;

    requestFQ.ticketingAgent() = &agentFQ;
    fqTrx.setOptions(&optFQ);
    fqTrx.setRequest(&requestFQ);
    fqTrx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.validateFareQuoteCurrencyIndicator(&fqTrx);
    CPPUNIT_ASSERT(fqTrx.getOptions()->isSellingCurrency());
  }

  void testGetIataDontChangeAgentIataWhenAlreadyPopulated()
  {
    Customer tjr;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    string iata = "111111";
    string homeIata = "222222";
    _trx.getRequest()->ticketingAgent()->tvlAgencyIATA() = iata;
    _trx.getRequest()->ticketingAgent()->homeAgencyIATA() = homeIata;
    _reqXmlValidator.getIata(&_trx);
    CPPUNIT_ASSERT_EQUAL(iata, _trx.getRequest()->ticketingAgent()->tvlAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(homeIata, _trx.getRequest()->ticketingAgent()->homeAgencyIATA());
  }

  void testGetIataGetsFromCustomer()
  {
    string iata = "111111";
    string homeIata = "222222";
    Customer tjr;
    tjr.arcNo() = iata;
    tjr.homeArcNo() = homeIata;
    _trx.getRequest()->ticketingAgent()->agentTJR() = &tjr;
    _reqXmlValidator.getIata(&_trx);
    CPPUNIT_ASSERT_EQUAL(iata, _trx.getRequest()->ticketingAgent()->tvlAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(homeIata, _trx.getRequest()->ticketingAgent()->homeAgencyIATA());
  }

  void testSetForcedStopoverForNoPnrPricingWhenNotForceConx()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    _reqXmlValidator.setForcedStopoverForNoPnrPricing(&itin);
    std::vector<TravelSeg*>& ts = itin.travelSeg();
    CPPUNIT_ASSERT(ts[0]->isForcedStopOver());
    CPPUNIT_ASSERT(ts[1]->isForcedStopOver());
    CPPUNIT_ASSERT(!ts[2]->isForcedStopOver());
  }

  void testSetForcedStopoverForNoPnrPricingWhenForceConx()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    airSeg2.forcedConx() = 'T';

    _reqXmlValidator.setForcedStopoverForNoPnrPricing(&itin);
    std::vector<TravelSeg*>& ts = itin.travelSeg();
    CPPUNIT_ASSERT(itin.travelSeg()[0]->isForcedStopOver());
    CPPUNIT_ASSERT(!ts[1]->isForcedStopOver());
    CPPUNIT_ASSERT(!ts[2]->isForcedStopOver());
  }

  void testSetForcedStopoverForNoPnrPricingWhenForceConxInLastSegment()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    airSeg3.forcedConx() = 'T';

    _reqXmlValidator.setForcedStopoverForNoPnrPricing(&itin);
    std::vector<TravelSeg*>& ts = itin.travelSeg();
    CPPUNIT_ASSERT(ts[0]->isForcedStopOver());
    CPPUNIT_ASSERT(ts[1]->isForcedStopOver());
    CPPUNIT_ASSERT(!ts[2]->isForcedStopOver());
  }

  void testValidateItinForLanHistoricalDontThrowsExceptionWhenThereIsOnlyOneTravelSeg()
  {
    Itin itin;
    AirSeg airSeg1;
    itin.travelSeg().push_back(&airSeg1);
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateItinForFlownSegments(itin));
    // CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateItinForLanHistorical(itin),
    // NonFatalErrorResponseException);
  }

  void testValidateItinForLanHistoricalDontThrowsExceptionWhenTvlDatesAreInSequence()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    DateTime todayDate = DateTime::localTime();
    DateTime tDate1 = todayDate.subtractDays(5);
    airSeg1.departureDT() = tDate1;
    airSeg1.arrivalDT() = tDate1;
    DateTime tDate2 = todayDate.subtractDays(4);
    airSeg2.departureDT() = tDate2;
    airSeg2.arrivalDT() = tDate2;

    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateItinForFlownSegments(itin));
  }

  void testValidateItinForLanHistoricalThrowsExceptionWhenTvlDatesAreNotInSequence()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    DateTime todayDate = DateTime::localTime();
    DateTime tDate1 = todayDate.subtractDays(4);
    airSeg1.arrivalDT() = tDate1;
    DateTime tDate2 = todayDate.subtractDays(5);
    airSeg2.departureDT() = tDate2;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateItinForFlownSegments(itin),
                         NonFatalErrorResponseException);
  }

  void testValidateItinForLanHistoricalDontThrowsExceptionWhenArunkTravelSeg()
  {
    Itin itin;
    ArunkSeg arunkSeg;
    AirSeg airSeg2;
    itin.travelSeg().push_back(&arunkSeg);
    itin.travelSeg().push_back(&airSeg2);
    DateTime todayDate = DateTime::localTime();
    DateTime tDate1 = todayDate.subtractDays(4);
    arunkSeg.arrivalDT() = tDate1;
    arunkSeg.segmentType() = Arunk;
    DateTime tDate2 = todayDate.subtractDays(5);
    airSeg2.departureDT() = tDate2;
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateItinForFlownSegments(itin));
  }

  void testValidateItinForLanHistoricalDontThrowsExceptionWhenOpenTravelSeg()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    DateTime todayDate = DateTime::localTime();
    DateTime tDate1 = todayDate.subtractDays(4);
    airSeg1.arrivalDT() = tDate1;
    airSeg1.segmentType() = Open;
    DateTime tDate2 = todayDate.subtractDays(5);
    airSeg2.departureDT() = tDate2;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateItinForFlownSegments(itin));
  }

  void testValidateItinForLanHistoricalThrowsExceptionWhenFlownAfterUnflownSeg()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    DateTime todayDate = DateTime::localTime();
    DateTime tDate1 = todayDate.subtractDays(5);
    airSeg1.departureDT() = tDate1;
    airSeg1.arrivalDT() = tDate1;
    DateTime tDate2 = todayDate.subtractDays(4);
    airSeg2.departureDT() = tDate2;
    airSeg2.arrivalDT() = tDate2;
    airSeg1.unflown() = true;
    airSeg2.unflown() = false;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateItinForFlownSegments(itin),
                         NonFatalErrorResponseException);
  }

  void testValidateItinForLanHistoricalDontThrowsExceptionWhenUnfownAfterFlownSeg()
  {
    Itin itin;
    AirSeg airSeg1, airSeg2;
    DateTime todayDate = DateTime::localTime();
    DateTime tDate1 = todayDate.subtractDays(5);
    airSeg1.departureDT() = tDate1;
    airSeg1.arrivalDT() = tDate1;
    DateTime tDate2 = todayDate.subtractDays(4);
    airSeg2.departureDT() = tDate2;
    airSeg2.arrivalDT() = tDate2;
    airSeg1.unflown() = false;
    airSeg2.unflown() = true;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);

    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateItinForFlownSegments(itin));
  }
  void testSetOperatingCarrierCodeForNonAirSegment()
  {
    AirSeg tSeg;
    tSeg.setMarketingCarrierCode("AA");
    tSeg.segmentType() = Arunk;
    _reqXmlValidator.setOperatingCarrierCode(tSeg);
    CPPUNIT_ASSERT(tSeg.operatingCarrierCode().empty());
  }

  void testSetOperatingCarrierCodeForAirSegWhenOprtCodeIsNotEmtpy()
  {
    AirSeg tSeg;
    tSeg.setMarketingCarrierCode("AA");
    tSeg.setOperatingCarrierCode("DL");
    tSeg.segmentType() = Air;
    _reqXmlValidator.setOperatingCarrierCode(tSeg);
    CPPUNIT_ASSERT(tSeg.operatingCarrierCode() == "DL");
  }

  void testSetOperatingCarrierCodeForAirSegWhenOprtCodeIsEmtpyNoFlight()
  {
    AirSeg tSeg;
    tSeg.setMarketingCarrierCode("AA");
    tSeg.segmentType() = Air;
    _reqXmlValidator.setOperatingCarrierCode(tSeg);
    CPPUNIT_ASSERT(tSeg.operatingCarrierCode().empty());
  }

  void testSetOperatingCarrierCodeForAirSegWhenOprtCodeIsEmtpyWithFlight()
  {
    AirSeg tSeg;
    tSeg.setMarketingCarrierCode("AA");
    tSeg.segmentType() = Air;
    tSeg.flightNumber() = 1234;
    _reqXmlValidator.setOperatingCarrierCode(tSeg);
    CPPUNIT_ASSERT(tSeg.operatingCarrierCode() == "AA");
  }

  void testValidatePassengerTypeWithAgesThrowsExceptionWhenWrongPaxTypeLength()
  {
    PaxType paxType;
    paxType.paxType() = "AD";
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerTypeWithAges(paxType),
                         NonFatalErrorResponseException);
  }

  void testValidatePassengerTypeWithAgesWhenPaxTypeIsNotAlpha2Digits()
  {
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.age() = 30;
    _reqXmlValidator.validatePassengerTypeWithAges(paxType);
    CPPUNIT_ASSERT(paxType.paxType() == "ADT");
    CPPUNIT_ASSERT(paxType.age() == 30);
  }

  void testValidatePassengerTypeWithAgesWhenPaxTypeIsAlpha2Digits()
  {
    PaxType paxType;
    paxType.paxType() = "A50";
    paxType.age() = 30;
    _reqXmlValidator.validatePassengerTypeWithAges(paxType);
    CPPUNIT_ASSERT(paxType.paxType() == "ANN");
    CPPUNIT_ASSERT(paxType.age() == 50);
  }

  void testValidatePassengerTypeWithAgesThrowsExceptionWhenAlpha2GigitsNotForSabre()
  {
    PaxType paxType, pT2, pT3;
    paxType.paxType() = "H50";
    pT2.paxType() = "K50";
    pT3.paxType() = "X50";
    paxType.age() = 30;
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerTypeWithAges(paxType),
                         NonFatalErrorResponseException);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerTypeWithAges(pT2),
                         NonFatalErrorResponseException);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerTypeWithAges(pT3),
                         NonFatalErrorResponseException);
  }

  void testValidatePassengerTypeWithAgesThrowsExceptionWhenAlpha2GigitsForInfant()
  {
    PaxType paxType, pT2;
    paxType.paxType() = "C00";
    pT2.paxType() = "C01";

    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerTypeWithAges(paxType),
                         NonFatalErrorResponseException);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerTypeWithAges(pT2),
                         NonFatalErrorResponseException);
  }

  void testValidatePassengerTypeThrowsExceptionWhenInvalidPaxTypeForSabre()
  {
    PaxType pT1, pT2, pT3, pT4;
    // paxtypes (ENF,HNN,KNN,XNN) are not valid for Sabre
    pT1.paxType() = "ENF";
    pT2.paxType() = "HNN";
    pT3.paxType() = "XNN";
    pT4.paxType() = "KNN";
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, pT1),
                         NonFatalErrorResponseException);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, pT2),
                         NonFatalErrorResponseException);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, pT3),
                         NonFatalErrorResponseException);
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, pT4),
                         NonFatalErrorResponseException);
  }

  void testValidateMaxPenaltyInfo()
  {
    MaxPenaltyInfoDataHandleMock dataHandle;

    PaxType paxType;
    paxType.paxType() = "C00";

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::INFO,
                                    {smp::BOTH},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::INFO,
                                    {smp::AFTER},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::INFO,
          {smp::BOTH, smp::CHANGEABLE},
          {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::INFO,
          {smp::BOTH},
          {smp::BOTH, smp::CHANGEABLE}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::INFO,
                                    {smp::BOTH, {}, Money(1.0, "USD")},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::INFO,
                                    {smp::BOTH},
                                    {smp::BOTH, {}, Money(1.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::OR,
                                    {smp::AFTER},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::AND,
                                    {smp::AFTER},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::OR,
          {smp::BOTH, smp::CHANGEABLE},
          {smp::BOTH, smp::CHANGEABLE}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::AND,
          {smp::BOTH, smp::CHANGEABLE},
          {smp::BOTH, smp::CHANGEABLE}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::OR,
                                    {smp::BOTH, {}, Money(100.0, "USD")},
                                    {smp::BOTH, {}, Money(100.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::AND,
                                    {smp::BOTH, {}, Money(100.0, "USD")},
                                    {smp::BOTH, {}, Money(100.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::OR,
                                    {smp::BOTH},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::AND,
                                    {smp::BOTH},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::OR,
          {smp::BOTH, smp::CHANGEABLE, Money(100.0, "USD")},
          {smp::BOTH, smp::CHANGEABLE, Money(100.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::AND,
          {smp::BOTH, smp::CHANGEABLE, Money(100.0, "USD")},
          {smp::BOTH, smp::CHANGEABLE, Money(100.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::OR,
          {smp::BOTH},
          {smp::BOTH, smp::CHANGEABLE, Money(100.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::AND,
          {smp::BOTH},
          {smp::BOTH, smp::CHANGEABLE, Money(100.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::OR,
          {smp::BOTH, smp::CHANGEABLE},
          {smp::BOTH, smp::NONCHANGEABLE}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{
          smp::AND,
          {smp::BOTH, smp::CHANGEABLE},
          {smp::BOTH, smp::NONCHANGEABLE}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::OR,
                                    {smp::BOTH, {}, Money(100.0, "USD")},
                                    {smp::BOTH, {}, Money(150.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::AND,
                                    {smp::BOTH, {}, Money(100.0, "USD")},
                                    {smp::BOTH, {}, Money(150.0, "USD")}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::OR,
                                    {smp::BOTH, {}, Money(10, "USD")},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType),
                           NonFatalErrorResponseException);
    }

    {
      MaxPenaltyInfo maxPenaltyInfo{smp::AND,
                                    {smp::BOTH, {}, Money(10, "USD")},
                                    {smp::BOTH}};
      paxType.maxPenaltyInfo() = &maxPenaltyInfo;
      CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validatePassengerType(&_trx, paxType));
    }
  }

  void testValidateCxrOverrideThrowsExceptionWhenCXRdoesnotmachTvlSeg()
  {
    RequestXmlValidatorStub rxvStub;
    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3;
    airSeg1.carrier() = "AA";
    airSeg2.carrier() = "BA";
    airSeg3.carrier() = "UA";
    airSeg1.segmentOrder() = 01;
    airSeg2.segmentOrder() = 02;
    airSeg3.segmentOrder() = 03;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    _request.governingCarrierOverrides().insert(std::pair<int16_t, CarrierCode>(01, "AA"));
    _request.governingCarrierOverrides().insert(std::pair<int16_t, CarrierCode>(02, "BA"));
    _request.governingCarrierOverrides().insert(std::pair<int16_t, CarrierCode>(03, "JL"));
    try { rxvStub.validateCxrOverride(&_trx, &itin); }
    catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
    catch (...) { CPPUNIT_ASSERT(false); }
  }
  void testValidateSideTrips_ABCBD()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "BBB", true), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_ABC_BD()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(false, "CCC", "BBB"), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_AB_CBD()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(false, "BBB", "CCC"),
        getTS(true, "CCC", "BBB", true), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_ABC_DBD()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(false, "CCC", "DDD"), getTS(true, "DDD", "BBB", true), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_ABCBCBD()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "BBB", true), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "BBB", true), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_ABCB_CBD()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "BBB", true), getTS(false, "BBB", "CCC"),
        getTS(true, "CCC", "BBB", true), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_ABCB_DEDF()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "BBB", true), getTS(false, "BBB", "DDD"),
        getTS(true, "DDD", "EEE", true), getTS(true, "EEE", "DDD", true), getTS(true, "DDD", "FFF");
    CPPUNIT_ASSERT_NO_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i));
  }
  void testValidateSideTrips_ABCD_fail()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "DDD");
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i),
                         NonFatalErrorResponseException);
  }
  void testValidateSideTrips_ABCBD_fail()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(true, "CCC", "BBB"), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i),
                         NonFatalErrorResponseException);
  }
  void testValidateSideTrips_ABC_DE_fail()
  {
    Itin* i = _memHandle.create<Itin>();
    i->travelSeg() += getTS(true, "AAA", "BBB"), getTS(true, "BBB", "CCC", true),
        getTS(false, "CCC", "DDD"), getTS(true, "BBB", "DDD");
    CPPUNIT_ASSERT_THROW(_reqXmlValidator.validateSideTrips(&_trx, *i),
                         NonFatalErrorResponseException);
  }

  // msd

  void testSetTimeZoneDiffBTWOrigAndPointOfSale()
  {
    Loc agentLoc;
    agentLoc.loc() = "SYD";
    _agent.agentLocation() = &agentLoc;
    const LocCode tvlLocT("LAX");
    short utcOffset = _reqXmlValidator.getTimeDiff(&_trx, tvlLocT);
    short expectedUtcOffset = 0;
    CPPUNIT_ASSERT_EQUAL(expectedUtcOffset, utcOffset);
  }

  void testRequestCrossDependenciesForAmountDiscountAndPlusUp()
  {
    RequestXmlValidator requestXmlValidator;
    PricingTrx* pricingTrx = nullptr;
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    pricingTrx = _memHandle.create<PricingTrx>();
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    pricingTrx->setOptions(pricingOptions);
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    PricingRequest* pricingRequest = _memHandle.create<PricingRequest>();
    pricingTrx->setRequest(pricingRequest);
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, 3.14, "USD");
    pricingRequest->addDiscountAmountNew(2, 2, -1.5 , "USD");
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string("DISCOUNT AMOUNT AND MARK UP AMOUNT NOT ALLOWED WITHIN SAME REQUEST"));
    }

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountPercentage(1, -5.0);
    pricingRequest->addDiscountPercentage(2, 8.0);
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string(
              "DISCOUNT PERCENTAGE AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST"));
    }

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, 3.14, "USD");
    pricingRequest->addDiscountAmountNew(2, 2, 1.5, "USD");
    pricingTrx->setRequest(pricingRequest);
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, -3.14, "USD");
    pricingRequest->addDiscountAmountNew(2, 2, -1.5, "USD");
    pricingTrx->setRequest(pricingRequest);
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    pricingRequest = _memHandle.create<PricingRequest>();
    Billing* billing = _memHandle.create<Billing>();
    pricingTrx->billing() = billing;
    pricingTrx->getOptions()->cabin().setAllCabin();
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string("SECONDARY ACTION CODE TC- NOT VALID WITH REQUEST ENTERED"));
    }
  }

  void testRequestCrossDependenciesForPercentageDiscountAndPlusUp()
  {
    RequestXmlValidator requestXmlValidator;
    PricingTrx* pricingTrx = nullptr;
    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    PricingRequest* pricingRequest = _memHandle.create<PricingRequest>();

    pricingTrx = _memHandle.create<PricingTrx>();

    pricingTrx->setOptions(pricingOptions);
    pricingTrx->setRequest(pricingRequest);

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountPercentage(1, 3.14);
    pricingRequest->addDiscountPercentage(2, -1.5 );
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string(
              "DISCOUNT PERCENTAGE AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST"));
    }

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountPercentage(1, 3.14);
    pricingRequest->addDiscountPercentage(2, 1.5);
    pricingTrx->setRequest(pricingRequest);
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountPercentage(1, -3.14);
    pricingRequest->addDiscountPercentage(2, -1.5);
    pricingTrx->setRequest(pricingRequest);
    CPPUNIT_ASSERT_NO_THROW(requestXmlValidator.checkRequestCrossDependencies(pricingTrx));
  }

  void testRequestCrossDependenciesForPercentageAndAmountMixed()
  {
    RequestXmlValidator requestXmlValidator;
    PricingTrx* pricingTrx = nullptr;
    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    PricingRequest* pricingRequest = _memHandle.create<PricingRequest>();

    pricingTrx = _memHandle.create<PricingTrx>();

    pricingTrx->setOptions(pricingOptions);
    pricingTrx->setRequest(pricingRequest);

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, 3.14, "USD");
    pricingRequest->addDiscountPercentage(1, 5.0);
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(e.message(), std::string("DISCOUNT AMOUNT AND DISCOUNT PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST"));
    }

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, -3.14, "USD");
    pricingRequest->addDiscountPercentage(1, 5.0);
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string("DISCOUNT PERCENTAGE AND MARK UP AMOUNT NOT ALLOWED WITHIN SAME REQUEST"));
    }


    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, 3.14, "USD");
    pricingRequest->addDiscountPercentage(1, -5.0);
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string("DISCOUNT AMOUNT AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST"));
    }

    pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->addDiscountAmountNew(1, 1, -3.14, "USD");
    pricingRequest->addDiscountPercentage(1, -5.0);
    pricingTrx->setRequest(pricingRequest);
    try
    {
      requestXmlValidator.checkRequestCrossDependencies(pricingTrx);
      CPPUNIT_FAIL("checkRequestCrossDependencies should throw NonFatalErrorResponseException");
    }
    catch (NonFatalErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT_EQUAL(
          e.message(),
          std::string("MARK UP AMOUNT AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST"));
    }
  }

  TravelSeg* getTS(bool air, const char* orig, const char* dest, bool sideTrip = false)
  {
    TravelSeg* ts;
    if (air)
      ts = _memHandle.create<AirSeg>();
    else
    {
      ts = _memHandle.create<ArunkSeg>();
      ts->segmentType() = Arunk;
    }
    ts->origAirport() = orig;
    ts->destAirport() = dest;
    if (sideTrip)
      ts->forcedSideTrip() = 'Y';
    return ts;
  }

  PricingTrx _trx;
  PricingRequest _request;
  Agent _agent;
  RequestXmlValidator _reqXmlValidator;
  NationCode _countryCode;
  NationCode _stateRegionCode;
  std::string _currentCRC;
  LocCode _residency;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RequestXmlValidatorTest);
}
