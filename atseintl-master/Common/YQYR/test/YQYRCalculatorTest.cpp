#include "test/include/CppUnitHelperMacros.h"

#include "Common/TrxUtil.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurfaceSeg.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "DBAccess/YQYRFees.h"
#include "DBAccess/YQYRFeesNonConcur.h"
#include "DBAccess/ZoneInfo.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/MergedFareMarket.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
FALLBACKVALUE_DECL(yqyrGetYQYRPreCalcLessLock)

class YQYRCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(YQYRCalculatorTest);

  CPPUNIT_TEST(testValidatePTCBlank);
  CPPUNIT_TEST(testValidatePTCValidTypes);
  CPPUNIT_TEST(testValidatePTCSabreGv1Type);

  CPPUNIT_TEST(testValidatePercent);
  CPPUNIT_TEST(testValidatePOS);
  CPPUNIT_TEST(testValidatePOT);

  CPPUNIT_TEST(testValidateAgencyPCC);
  CPPUNIT_TEST(testValidateAgencyIATA);
  CPPUNIT_TEST(testValidateAgencyBlank);

  CPPUNIT_TEST(testValidateTravelDate);
  CPPUNIT_TEST(testValidateTravelDateHistorical);
  CPPUNIT_TEST(testValidateTravelDateValid);

  CPPUNIT_TEST(testValidateTicketingDateFirst);
  CPPUNIT_TEST(testValidateTicketingDateLast);
  CPPUNIT_TEST(testValidateTicketingDateHistorical);

  CPPUNIT_TEST(testValidateJourneyBlank);
  CPPUNIT_TEST(testValidateJourneyLocs);
  CPPUNIT_TEST(testValidateJourneyViaLoc);
  CPPUNIT_TEST(testValidateJourneyValid);

  CPPUNIT_TEST(testValidateLocsTo);
  CPPUNIT_TEST(testValidateLocsFrom);
  CPPUNIT_TEST(testValidateLocsBetween);

  CPPUNIT_TEST(testValidateWithinNoAir);
  CPPUNIT_TEST(testValidateWithinOrigin);
  CPPUNIT_TEST(testValidateWithinDestination);

  CPPUNIT_TEST(testValidateJourneyViaEmpty);
  CPPUNIT_TEST(testValidateJourneyViaAirport);
  CPPUNIT_TEST(testValidateJourneyViaZone);

  CPPUNIT_TEST(testCheckValidatingCxr);
  CPPUNIT_TEST(testValidateReturnToOrigin);

  CPPUNIT_TEST(testValidateSectorLocs);
  CPPUNIT_TEST(testValidateSectorIntl);
  CPPUNIT_TEST(testValidateSectorEquip);
  CPPUNIT_TEST(testValidateSectorCarrier);
  CPPUNIT_TEST(testValidateSectorBookingCode);
  CPPUNIT_TEST(testValidateSectorFareBasis);

  CPPUNIT_TEST(testValidatePortionViaLoc);
  CPPUNIT_TEST(testValidatePortionLocs);
  CPPUNIT_TEST(testValidatePortionStopoverTime);
  CPPUNIT_TEST(testValidatePortionIntl);
  CPPUNIT_TEST(testValidatePortionEquip);
  CPPUNIT_TEST(testValidatePortionCarrier);
  CPPUNIT_TEST(testValidatePortionFareBasisBookingCode);

  CPPUNIT_TEST(testValidateIntlDomestic);
  CPPUNIT_TEST(testValidateIntlInternational);

  CPPUNIT_TEST(testValidateEquipment);

  CPPUNIT_TEST(testValidateCarrierTableEmpty);
  CPPUNIT_TEST(testValidateCarrierTableCarriers);
  CPPUNIT_TEST(testValidateCarrierTableFlights);

  CPPUNIT_TEST(testCheckLocation);

  CPPUNIT_TEST(testValidateBookingCodeEmpty);
  CPPUNIT_TEST(testValidateBookingCodeSoftPass);
  CPPUNIT_TEST(testValidateBookingCodePass);
  CPPUNIT_TEST(testValidateBookingCodeFailed);

  CPPUNIT_TEST(testValidateFbcTktDsg);
  CPPUNIT_SKIP_TEST(testValidateVia);

  CPPUNIT_TEST(testDetermineCarriersEmpty);
  CPPUNIT_TEST(testDetermineCarriersNoFees);
  CPPUNIT_TEST(testDetermineCarriersNoS2CarrierPresent);
  CPPUNIT_TEST(testDetermineCarriers);
  CPPUNIT_TEST(testDetermineCarriersInvalidT190);

  CPPUNIT_TEST(testValidateT190Null);
  CPPUNIT_TEST(testValidateT190DollarCarrier);
  CPPUNIT_TEST(testValidateT190NotSameCarrier);
  CPPUNIT_TEST(testValidateT190ApplInd);
  CPPUNIT_TEST(testValidateT190Valid);

  CPPUNIT_TEST(testYQYRApplicationEmpty);
  CPPUNIT_TEST(testYQYRApplication);
  CPPUNIT_TEST(testYQYRApplicationFeeApplInd);
  CPPUNIT_TEST(testYQYRApplicationCompare);

  CPPUNIT_TEST(testInitMFMsFmp);
  CPPUNIT_TEST(testInitMFMsFmpMatrix);

  CPPUNIT_TEST(testInitFBCMap);
  CPPUNIT_TEST(testInitFBCMapNoFarePath);
  CPPUNIT_TEST(testInitFBCMapForMFMEmpty);
  CPPUNIT_TEST(testInitFBCMapForMFM);

  CPPUNIT_TEST(testGetMFMs);

  CPPUNIT_TEST(testInitializeYQYRListEmpty);
  CPPUNIT_TEST(testInitializeYQYRList);

  CPPUNIT_TEST(testAnalyzeItinDomestic);
  CPPUNIT_TEST(testAnalyzeItinInternational);
  CPPUNIT_TEST(testIsSimpleItinDomestic);
  CPPUNIT_TEST(testIsSimpleItinDomesticReturn);
  CPPUNIT_TEST(testIsSimpleItinInternationalSurface);
  CPPUNIT_TEST(testIsSimpleItinInternationalAir);

  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcessDirectionOut);
  CPPUNIT_TEST(testProcessDirectionIn);
  CPPUNIT_TEST(testProcessDirectionOther);

  CPPUNIT_TEST(testYQYRPathExtend);
  CPPUNIT_TEST(testYQYRPathAdd);

  CPPUNIT_TEST(testMatchFeeRecordFailJourney);
  CPPUNIT_TEST(testMatchFeeRecordFailValidatingCxr);
  CPPUNIT_TEST(testMatchFeeRecordFailReturnToOrigin);
  CPPUNIT_TEST(testMatchFeeRecordFailPTC);
  CPPUNIT_TEST(testMatchFeeRecordFailPOS);
  CPPUNIT_TEST(testMatchFeeRecordFailPOT);
  CPPUNIT_TEST(testMatchFeeRecordFailAgency);
  CPPUNIT_TEST(testMatchFeeRecordFailTravelDate);
  CPPUNIT_TEST(testMatchFeeRecordFailPercent);
  CPPUNIT_TEST(testMatchFeeRecordPortion);
  CPPUNIT_TEST(testMatchFeeRecordSector);

  CPPUNIT_TEST(testMakeSliceKey);
  CPPUNIT_TEST(testProcessSliceEndBeforeStart);
  CPPUNIT_TEST(testProcessSliceFromCache);
  CPPUNIT_TEST(testProcessSliceFail);
  CPPUNIT_TEST(testProcessSlicePass);
  CPPUNIT_TEST(testProcessCarrier);

  CPPUNIT_TEST(testMatchFareBasisFromCache);
  CPPUNIT_TEST(testMatchFareBasisEmpty);
  CPPUNIT_TEST(testMatchFareBasisImplEmpty);
  CPPUNIT_TEST(testMatchFareBasisImplNoTktDesignator);
  CPPUNIT_TEST(testMatchFareBasisImplNoTktDesignatorNoMatchClass);
  CPPUNIT_TEST(testMatchFareBasisImplNoTktDesignatorMatchClass);
  CPPUNIT_TEST(testMatchFareBasisImplNoPtfDesignator);
  CPPUNIT_TEST(testMatchFareBasisImplNotMatchingFareBasisCodes);
  CPPUNIT_TEST(testMatchFareBasisImplEmptyTktDesignator);
  CPPUNIT_TEST(testMatchFareBasisImplOnlyOneTktDesignatorAsterisk);
  CPPUNIT_TEST(testMatchFareBasisImplOnlyOneTktDesignatorLongAsterisk);
  CPPUNIT_TEST(testMatchFareBasisImplOnlyOneTktDesignatorHypenMatch);
  CPPUNIT_TEST(testMatchFareBasisImplOnlyOneTktDesignatorHypenNoMatch);
  CPPUNIT_TEST(testMatchFareBasisImplOnlyOnePtfDesignator);
  CPPUNIT_TEST(testMatchFareBasisImplFirstTktDesignatorAsterisk);
  CPPUNIT_TEST(testMatchFareBasisImplFirstTktDesignatorEmpty);
  CPPUNIT_TEST(testMatchFareBasisImplSecondTktDesignatorAsterisk);
  CPPUNIT_TEST(testMatchFareBasisImplSecondTktDesignatorOnlyAsterisk);
  CPPUNIT_TEST(testMatchFareBasisImplSecondDesignatorMatch);
  CPPUNIT_TEST(testMatchFareBasisImplSecondDesignatorNoMatch);

  CPPUNIT_TEST(testFindMatchingPathsOneFPEmpty);
  CPPUNIT_TEST(testFindMatchingPathsOneFPMatched);
  CPPUNIT_TEST(testFindMatchingPathsOutbound);
  CPPUNIT_TEST(testFindMatchingPathsInbound);
  CPPUNIT_TEST(testFindMatchingPathsOutboundSkip);
  CPPUNIT_TEST(testFindMatchingPathsInboundSkip);
  CPPUNIT_TEST(testFindMatchingPathsJourney);
  CPPUNIT_TEST(testFindMatchingPathsJourneyOutboundSkip);
  CPPUNIT_TEST(testFindMatchingPathsJourneyInboundSkip);
  CPPUNIT_TEST(testFindMatchingPathsBlank);
  CPPUNIT_TEST(testFindMatchingPathsBlankOutboundSkip);
  CPPUNIT_TEST(testFindMatchingPathsBlankInboundSkip);
  CPPUNIT_TEST(testFindMatchingPathsUnknown);
  CPPUNIT_TEST(testFindMatchingPathsEmpty);

  CPPUNIT_TEST(testChargeFarePathEmptyBucket);
  CPPUNIT_TEST(testChargeFarePathEmpty);
  CPPUNIT_TEST(testChargeFarePathOutbound);
  CPPUNIT_TEST(testChargeFarePathInbound);
  CPPUNIT_TEST(testChargeFarePathJourney);
  CPPUNIT_TEST(testChargeFarePathBlank);
  CPPUNIT_TEST(testChargeFarePathUnknown);
  CPPUNIT_TEST(testChargeFarePathOverrideCurrency);

  CPPUNIT_TEST(testCarrierFeeCodeBucketLowerBoundEmpty);
  CPPUNIT_TEST(testCarrierFeeCodeBucketLowerBoundOutbound);
  CPPUNIT_TEST(testCarrierFeeCodeBucketLowerBoundInbound);
  CPPUNIT_TEST(testCarrierFeeCodeBucketLowerBoundJourney);
  CPPUNIT_TEST(testCarrierFeeCodeBucketLowerBoundBlank);
  CPPUNIT_TEST(testCarrierFeeCodeBucketLowerBoundUnknown);

  CPPUNIT_TEST(testFindLowerBound);
  CPPUNIT_TEST(testFindLowerBoundEmpty);
  CPPUNIT_TEST(testFindLowerBoundOverrideCurrency);

  CPPUNIT_TEST(testMergeEmpty);
  CPPUNIT_TEST(testMerge);

  CPPUNIT_TEST(testMatchPathFailFareBasisNotFound);
  CPPUNIT_TEST(testMatchPathFailFareBasisNoMatch);
  CPPUNIT_TEST(testMatchPathFailBookingCodeNotFound);
  CPPUNIT_TEST(testMatchPathFailBookingCodeNoMatch);
  CPPUNIT_TEST(testMatchPathPass);

  CPPUNIT_TEST(testPrepareMatchMaps);
  CPPUNIT_TEST(testAreYQYRsExemptedAllTaxes);
  CPPUNIT_TEST(testAreYQYRsExemptedSpecificTaxesEmpty);
  CPPUNIT_TEST(testAreYQYRsExemptedSpecificTaxesYQYR);
  CPPUNIT_TEST(testAreYQYRsExemptedSpecificTaxesNoYQYR);
  CPPUNIT_TEST(testAreYQYRsExemptedNoTax);

  CPPUNIT_TEST(testCalculatePercentageFee);
  CPPUNIT_TEST(testCalculatePercentageFeeNotMatchingCurrency);

  CPPUNIT_TEST(testYQYRLBFactory);
  CPPUNIT_TEST(testYQYRLBFactoryOld);
  CPPUNIT_TEST(testYQYRLBFactorySimpleItin);

  CPPUNIT_TEST(testGetFareBreaksFromFP);
  CPPUNIT_TEST(testGetFareBreaksFromMFM);

  CPPUNIT_TEST(testMileageSameLoc);
  CPPUNIT_TEST(testMileageSubstitution);
  CPPUNIT_TEST(testMileageUniqueTPM);
  CPPUNIT_TEST(testMileageNonUniqueTPM);
  CPPUNIT_TEST(testMileageUniqueMPM);
  CPPUNIT_TEST(testMileageNonUniqueMPM);
  CPPUNIT_TEST(testMileageGCM);

  CPPUNIT_TEST(testApplCountNoPortion);
  CPPUNIT_TEST(testApplCountNoConnectExemptInd);
  CPPUNIT_SKIP_TEST(testApplCount);
  CPPUNIT_TEST(testApplCountStopoverTime);
  CPPUNIT_TEST(testApplCountNoBlankFeeApplInd);

  CPPUNIT_TEST(testAddToValCxrMap);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _request = _memHandle.create<PricingRequest>();
    _request->ticketingAgent() = _memHandle.create<Agent>();
    _request->ticketingAgent()->currencyCodeAgent() = "GBP";
    Loc* agentLoc = _memHandle.create<Loc>();
    agentLoc->nation() = "GB";
    _request->ticketingAgent()->agentLocation() = agentLoc;
    _request->specifiedTktDesignator()[1] = "AAA";

    _itin = _memHandle.create<Itin>();

    AirSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "DFW";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    seg1->fareBasisCode() = "AAA";
    seg1->setBookingCode("YY");
    seg1->setMarketingCarrierCode("AA");
    _itin->travelSeg().push_back(seg1);

    TravelSeg* seg2 = _memHandle.create<AirSeg>();
    Loc* origin2 = _memHandle.create<Loc>();
    origin2->loc() = "DFW";
    Loc* destination2 = _memHandle.create<Loc>();
    destination2->loc() = "LAX";
    seg2->origin() = origin2;
    seg2->destination() = destination2;
    seg2->fareBasisCode() = "BBB";
    seg2->setBookingCode("ZZ");
    _itin->travelSeg().push_back(seg2);

    TravelSeg* seg3 = _memHandle.create<AirSeg>();
    Loc* origin3 = _memHandle.create<Loc>();
    origin3->loc() = "LAX";
    Loc* destination3 = _memHandle.create<Loc>();
    destination3->loc() = "SFO";
    seg3->origin() = origin3;
    seg3->destination() = destination3;
    _itin->travelSeg().push_back(seg3);

    TravelSeg* seg4 = _memHandle.create<AirSeg>();
    Loc* origin4 = _memHandle.create<Loc>();
    origin4->loc() = "SFO";
    Loc* destination4 = _memHandle.create<Loc>();
    destination4->loc() = "KRK";
    seg4->origin() = origin4;
    seg4->destination() = destination4;
    _itin->travelSeg().push_back(seg4);

    _farePath = _memHandle.create<FarePath>();
    _farePath->paxType() = _memHandle.create<PaxType>();

    _trx->setRequest(_request);

    _yq = _memHandle.create<YQYRCalculator>(*_trx, *_itin, _farePath);
    _yq->_furthestSeg = seg3; // setting turnaround point
    _yq->_furthestLoc = seg3->destination();
    _itin->validatingCarrier() = "VC";
    _yq->_valCxrMap["VC"] = _yq->_valCxrContext = _memHandle.create<YQYRCalculator::ValCxrContext>("VC");
  }

  void tearDown() { _memHandle.clear(); }

  void testValidatePTCBlank()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->psgType() = "";
    CPPUNIT_ASSERT(_yq->validatePTC(*fees));

    fees->psgType() = " ";
    CPPUNIT_ASSERT(_yq->validatePTC(*fees));
  }

  void testValidatePTCValidTypes()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->psgType() = CHILD;
    _farePath->paxType()->paxType() = INFANT;
    CPPUNIT_ASSERT(_yq->validatePTC(*fees));

    fees->psgType() = CHILD;
    _farePath->paxType()->paxType() = CHILD;
    CPPUNIT_ASSERT(_yq->validatePTC(*fees));

    fees->psgType() = CHILD;
    _farePath->paxType()->paxType() = ADULT;
    CPPUNIT_ASSERT(!_yq->validatePTC(*fees));
  }

  void testValidatePTCSabreGv1Type()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    _farePath->paxType()->paxType() = GV1;
    fees->psgType() = GDP;
    CPPUNIT_ASSERT(_yq->validatePTC(*fees));

    _farePath->paxType()->paxType() = GV1;
    fees->psgType() = GVT;
    CPPUNIT_ASSERT(!_yq->validatePTC(*fees));
  }

  void testValidatePercent()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->percent() = 0;

    CPPUNIT_ASSERT(_yq->validatePercent(*fees));

    fees->percent() = 10;
    CPPUNIT_ASSERT_EQUAL(_yq->_isOnlineItin, _yq->validatePercent(*fees));
  }

  void testValidatePOS()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->posLoc() = "";

    CPPUNIT_ASSERT(_yq->validatePOS(*fees));

    fees->posLoc() = "PL";
    fees->posLocType() = NATION;
    CPPUNIT_ASSERT(!_yq->validatePOS(*fees));
  }

  void testValidatePOT()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->potLoc() = "";

    CPPUNIT_ASSERT(_yq->validatePOT(*fees));

    fees->potLoc() = "PL";
    fees->potLocType() = NATION;
    CPPUNIT_ASSERT(!_yq->validatePOT(*fees));
  }

  void testValidateAgencyPCC()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->posLocaleType() = 'T';

    fees->posAgencyPCC() = "ABC";
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = "ABC";
    CPPUNIT_ASSERT(_yq->validateAgency(*fees));

    fees->posAgencyPCC() = "AAAAA";
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = "ABC";
    _trx->getRequest()->ticketingAgent()->officeDesignator() = "AAAAA";
    CPPUNIT_ASSERT(_yq->validateAgency(*fees));
  }

  void testValidateAgencyIATA()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->posLocaleType() = 'I';

    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = "ABC";
    fees->posIataTvlAgencyNo() = "ABC";
    CPPUNIT_ASSERT(_yq->validateAgency(*fees));

    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = "";
    _trx->getRequest()->ticketingAgent()->airlineIATA() = "AA";
    fees->posIataTvlAgencyNo() = "AA";
    CPPUNIT_ASSERT(_yq->validateAgency(*fees));

    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = "";
    _trx->getRequest()->ticketingAgent()->airlineIATA() = "";
    CPPUNIT_ASSERT(!_yq->validateAgency(*fees));
  }

  void testValidateAgencyBlank()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->posLocaleType() = ' ';
    CPPUNIT_ASSERT(_yq->validateAgency(*fees));

    fees->posLocaleType() = 'X';
    CPPUNIT_ASSERT(!_yq->validateAgency(*fees));
  }

  void testValidateTravelDate()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    _itin->setTravelDate(DateTime(2013, 2, 2));
    fees->effDate() = DateTime(2013, 3, 3);
    const_cast<DateTime&>(_yq->_travelDate) = _itin->travelDate();
    CPPUNIT_ASSERT(!_yq->validateTravelDate(*fees));

    _itin->setTravelDate(DateTime(2013, 2, 2));
    fees->effDate() = DateTime(2013, 1, 1);
    fees->discDate() = DateTime(2013, 1, 1);
    const_cast<DateTime&>(_yq->_travelDate) = _itin->travelDate();
    CPPUNIT_ASSERT(!_yq->validateTravelDate(*fees));

    _itin->setTravelDate(DateTime(2013, 2, 2));
    fees->effDate() = DateTime(2013, 1, 1);
    fees->discDate() = DateTime(2013, 3, 3);
    fees->expireDate() = DateTime(2013, 1, 1);
    const_cast<DateTime&>(_yq->_travelDate) = _itin->travelDate();
    CPPUNIT_ASSERT(!_yq->validateTravelDate(*fees));
  }

  void testValidateTravelDateHistorical()
  {
    boost::gregorian::date_duration coupleOfDays(5);
    DateTime past(DateTime::localTime().date() - coupleOfDays);
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    MockGlobal::setAllowHistorical(true);

    _itin->setTravelDate(DateTime(2013, 2, 2));
    const_cast<DateTime&>(_yq->_travelDate) = _itin->travelDate();
    fees->effDate() = DateTime(2013, 1, 1);
    fees->discDate() = DateTime(2013, 3, 3);
    _yq->_trx.dataHandle().refreshHist(past);
    fees->modDate() = _trx->getRequest()->ticketingDT().subtractDays(1);
    fees->expireDate() = DateTime(2013, 1, 1);
    CPPUNIT_ASSERT(!_yq->validateTravelDate(*fees));
  }

  void testValidateTravelDateValid()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    _itin->setTravelDate(DateTime(2013, 2, 2));
    fees->effDate() = DateTime(2013, 1, 1);
    fees->discDate() = DateTime(2013, 3, 3);
    fees->expireDate() = DateTime(2013, 3, 3);
    const_cast<DateTime&>(_yq->_travelDate) = _itin->travelDate();
    CPPUNIT_ASSERT(_yq->validateTravelDate(*fees));
  }

  void testValidateTicketingDateFirst()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    MockGlobal::setAllowHistorical(false);

    fees->firstTktDate() = (_request->ticketingDT().subtractDays(1)).date();
    fees->lastTktDate() = fees->firstTktDate().addDays(10);
    CPPUNIT_ASSERT(_yq->validateTicketingDate(*fees));
    fees->firstTktDate() = fees->firstTktDate().addDays(1);
    CPPUNIT_ASSERT(_yq->validateTicketingDate(*fees));
    fees->firstTktDate() = fees->firstTktDate().addDays(1);
    CPPUNIT_ASSERT(!_yq->validateTicketingDate(*fees));
  }

  void testValidateTicketingDateLast()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    MockGlobal::setAllowHistorical(false);

    fees->firstTktDate() = (_request->ticketingDT().subtractDays(1)).date();
    fees->lastTktDate() = fees->firstTktDate().addDays(2) + Hours(23) + Minutes(59) + Seconds(59);
    CPPUNIT_ASSERT(_yq->validateTicketingDate(*fees));
    fees->lastTktDate() = fees->lastTktDate().subtractDays(1);
    CPPUNIT_ASSERT(_yq->validateTicketingDate(*fees));
    fees->lastTktDate() = fees->lastTktDate().subtractDays(1);
    CPPUNIT_ASSERT(!_yq->validateTicketingDate(*fees));
  }

  void testValidateTicketingDateHistorical()
  {
    MockGlobal::setAllowHistorical(true);
    DateTime past(DateTime::localTime().subtractDays(5));
    _yq->_trx.dataHandle().setTicketDate(past);
    _request->ticketingDT() = past;
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->firstTktDate() = past.addDays(100).date();
    fees->lastTktDate() = fees->firstTktDate().addDays(2) + Hours(23) + Minutes(59) + Seconds(59);
    CPPUNIT_ASSERT(_yq->validateTicketingDate(*fees));
  }

  void testValidateJourneyBlank()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->journeyLoc1() = "";
    fees->journeyLoc2() = "";
    fees->whollyWithinLoc() = "";
    CPPUNIT_ASSERT(_yq->validateJourney(*fees));
  }

  void testValidateJourneyLocs()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->journeyLoc1() = "ABC";
    fees->journeyLocType1() = 'Z';
    fees->journeyLoc2() = "DEF";
    fees->journeyLocType2() = 'Z';
    fees->journeyLoc1Ind() = 'A';
    CPPUNIT_ASSERT(!_yq->validateJourney(*fees));
  }

  void testValidateJourneyViaLoc()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->journeyLoc1() = "";
    fees->journeyLoc2() = "";
    fees->viaLoc() = "XYZ";
    fees->viaLocType() = 'Z';
    CPPUNIT_ASSERT(!_yq->validateJourney(*fees));

    fees->journeyLoc1() = "";
    fees->journeyLoc2() = "";
    fees->viaLoc() = "";
    fees->whollyWithinLoc() = "XYZ";
    fees->whollyWithinLocType() = 'Z';
    CPPUNIT_ASSERT(!_yq->validateJourney(*fees));
  }

  void testValidateJourneyValid()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->journeyLoc1() = "KRK";
    fees->journeyLocType1() = 'P';
    fees->journeyLoc2() = "SFO";
    fees->journeyLocType2() = 'P';
    fees->viaLoc() = "";
    fees->whollyWithinLoc() = "";
    CPPUNIT_ASSERT(_yq->validateJourney(*fees));
  }

  void testValidateLocsFrom()
  {
    char dir = '1';
    Loc frontLoc;
    Loc backLoc;
    frontLoc.loc() = "KRK";
    backLoc.loc() = "DFW";

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "", 'P', backLoc, "DFW", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "KRK", 'P', backLoc, "", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "KRK", 'P', backLoc, "DFW", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(!_yq->validateLocs(dir, frontLoc, "000", 'Z', backLoc, "000", 'Z', "ATP", "AA"));

    CPPUNIT_ASSERT(!_yq->validateLocs(dir, frontLoc, "5", 'U', backLoc, "5", 'U', "ATP", "AA"));
  }

  void testValidateLocsTo()
  {
    char dir = '2';
    Loc frontLoc;
    Loc backLoc;
    frontLoc.loc() = "DFW";
    backLoc.loc() = "KRK";

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "", 'P', backLoc, "DFW", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "KRK", 'P', backLoc, "", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "KRK", 'P', backLoc, "DFW", 'P', "ATP", "AA"));
  }

  void testValidateLocsBetween()
  {
    char dir = 'B';
    Loc frontLoc;
    Loc backLoc;
    frontLoc.loc() = "KRK";
    backLoc.loc() = "DFW";

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "", 'P', backLoc, "DFW", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "KRK", 'P', backLoc, "", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "KRK", 'P', backLoc, "DFW", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "", 'P', backLoc, "KRK", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "DFW", 'P', backLoc, "", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateLocs(dir, frontLoc, "DFW", 'P', backLoc, "KRK", 'P', "ATP", "AA"));
  }

  void testValidateWithinNoAir()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<SurfaceSeg>();
    segs.push_back(seg);

    CPPUNIT_ASSERT(_yq->validateWithin(segs.begin(), segs.end(), "KRK", 'U', "ATP", "AA"));
  }

  void testValidateWithinOrigin()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "KRK";

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "ABC";

    seg->origin() = origin;
    seg->destination() = destination;
    segs.push_back(seg);

    CPPUNIT_ASSERT(!_yq->validateWithin(segs.begin(), segs.end(), "ABC", 'P', "ATP", "AA"));
  }

  void testValidateWithinDestination()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "ABC";

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "DFW";

    seg->origin() = origin;
    seg->destination() = destination;
    segs.push_back(seg);

    CPPUNIT_ASSERT(!_yq->validateWithin(segs.begin(), segs.end(), "ABC", 'P', "ATP", "AA"));
  }

  void testValidateJourneyViaEmpty()
  {
    CPPUNIT_ASSERT(_yq->validateJourneyVia(
        _itin->travelSeg().begin(), _itin->travelSeg().end(), "", 'P', "ATP", "AA"));
  }

  void testValidateJourneyViaAirport()
  {
    CPPUNIT_ASSERT(!_yq->validateJourneyVia(
        _itin->travelSeg().begin(), _itin->travelSeg().end(), "ABC", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(_yq->validateJourneyVia(
        _itin->travelSeg().begin(), _itin->travelSeg().end(), "DFW", 'P', "ATP", "AA"));
  }

  void testValidateJourneyViaZone()
  {
    CPPUNIT_ASSERT(!_yq->validateJourneyVia(
        _itin->travelSeg().begin(), _itin->travelSeg().end(), "0", 'Z', "ATP", "AA"));

    CPPUNIT_ASSERT(!_yq->validateJourneyVia(
        _itin->travelSeg().begin(), _itin->travelSeg().end(), "0", 'U', "ATP", "AA"));
  }

  void testCheckValidatingCxr()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->taxCarrierApplTblItemNo() = 0;
    CPPUNIT_ASSERT(_yq->checkValidatingCxr(*fees));

    fees->vendor() = "ATP";
    fees->taxCarrierApplTblItemNo() = 1;
    CPPUNIT_ASSERT(!_yq->checkValidatingCxr(*fees));

    fees->vendor() = "ATP";
    fees->taxCarrierApplTblItemNo() = 2;
    CPPUNIT_ASSERT(_yq->checkValidatingCxr(*fees));
  }

  void testValidateReturnToOrigin()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->returnToOrigin() = 'Y';
    CPPUNIT_ASSERT_EQUAL(_yq->_returnsToOrigin, _yq->validateReturnToOrigin(*fees));

    fees->returnToOrigin() = 'N';
    CPPUNIT_ASSERT_EQUAL(!_yq->_returnsToOrigin, _yq->validateReturnToOrigin(*fees));

    fees->returnToOrigin() = ' ';
    CPPUNIT_ASSERT(_yq->validateReturnToOrigin(*fees));

    fees->returnToOrigin() = 'X';
    CPPUNIT_ASSERT(!_yq->validateReturnToOrigin(*fees));
  }

  void testValidateSectorLocs()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->directionality() = '1';
    fees->sectorPortionLoc1() = "AAA";
    fees->sectorPortionLocType1() = 'P';
    fees->sectorPortionLoc2() = "AAA";
    fees->sectorPortionLocType2() = 'P';
    fees->vendor() = "ATP";

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));
  }

  void testValidateSectorIntl()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->intlDomInd() = 'X';

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));
  }

  void testValidateSectorEquip()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->equipCode() = "ABC";

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));
  }

  void testValidateSectorCarrier()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->taxCarrierFltTblItemNo() = 1;

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));
  }

  void testValidateSectorBookingCode()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "AB";
    fees->bookingCode2() = "AB";
    fees->bookingCode3() = "AB";

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));

    fees->sectorPortionOfTvlInd() = 'P';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));
  }

  void testValidateSectorFareBasis()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "AB";
    fees->bookingCode2() = "AB";
    fees->bookingCode3() = "AB";
    fees->sectorPortionOfTvlInd() = 'P';
    fees->fareBasis() = "AAA";

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));

    _yq->_fbcBySegNo.insert(std::make_pair(0, "BBB"));
    _yq->_fbcBySegNo.insert(std::make_pair(0, "AAA"));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_CONDITIONALLY),
                         static_cast<uint8_t>(_yq->validateSector(*fees, 0)));
  }

  void testValidatePortionViaLoc()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->sectorPortionViaLoc() = "KRK";
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED_NO_CHANCE),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 0)));

    fees->sectorPortionLoc1() = "";
    fees->sectorPortionLoc2() = "";
    fees->sectorPortionViaLoc() = "KRK";
    fees->stopConnectInd() = 'C';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 2)));
  }

  void testValidatePortionLocs()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->directionality() = '1';
    fees->sectorPortionLoc1() = "AAA";
    fees->sectorPortionLoc2() = "AAA";
    fees->sectorPortionLocType1() = 'P';
    fees->sectorPortionLocType2() = 'P';
    fees->sectorPortionViaLoc() = "";
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));
  }

  void testValidatePortionStopoverTime()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->directionality() = '1';
    fees->sectorPortionLoc1() = "KRK";
    fees->sectorPortionLoc2() = "LAX";
    fees->sectorPortionLocType1() = 'P';
    fees->sectorPortionLocType2() = 'P';
    fees->sectorPortionViaLoc() = "";
    fees->stopConnectInd() = 'S';

    fees->stopConnectUnit() = 'D';
    fees->stopConnectTime() = 0;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    fees->stopConnectUnit() = 'H';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    fees->stopConnectUnit() = 'M';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    fees->stopConnectUnit() = 'N';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    fees->stopConnectUnit() = 'D';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));
  }

  void testValidatePortionIntl()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->sectorPortionLoc1() = "KRK";
    fees->sectorPortionLoc2() = "LAX";
    fees->stopConnectUnit() = 'D';
    fees->stopConnectTime() = 0;
    fees->intlDomInd() = 'X';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));
  }

  void testValidatePortionEquip()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->equipCode() = "ABC";
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));
  }

  void testValidatePortionCarrier()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->taxCarrierFltTblItemNo() = 1;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));
  }

  void testValidatePortionFareBasisBookingCode()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    _yq->_fbcBySegNo.insert(std::make_pair(0, "AAA"));
    fees->fareBasis() = "BBB";
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED_NO_CHANCE),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    fees->fareBasis() = "AAA";
    fees->bookingCode1() = "AB";
    fees->bookingCode2() = "AB";
    fees->bookingCode3() = "AB";
    fees->sectorPortionOfTvlInd() = 'P';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    fees->sectorPortionOfTvlInd() = ' ';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    _yq->_fbcBySegNo.insert(std::make_pair(1, "AAA"));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));

    _yq->_fbcBySegNo.insert(std::make_pair(1, "BBB"));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
                         static_cast<uint8_t>(_yq->validatePortion(*fees, 0, 1)));
  }

  void testValidateIntlDomestic()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    Loc* destination = _memHandle.create<Loc>();
    destination->nation() = "PL";
    seg->origin() = origin;
    seg->destination() = destination;
    segs.push_back(seg);

    CPPUNIT_ASSERT(_yq->validateIntl(segs.begin(), segs.end(), ' '));
    CPPUNIT_ASSERT(_yq->validateIntl(segs.begin(), segs.end(), 'D'));
    CPPUNIT_ASSERT(!_yq->validateIntl(segs.begin(), segs.end(), 'I'));
    CPPUNIT_ASSERT(!_yq->validateIntl(segs.begin(), segs.end(), '*'));
  }

  void testValidateIntlInternational()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    Loc* destination = _memHandle.create<Loc>();
    destination->nation() = "US";
    seg->origin() = origin;
    seg->destination() = destination;
    segs.push_back(seg);

    CPPUNIT_ASSERT(_yq->validateIntl(segs.begin(), segs.end(), ' '));
    CPPUNIT_ASSERT(!_yq->validateIntl(segs.begin(), segs.end(), 'D'));
    CPPUNIT_ASSERT(_yq->validateIntl(segs.begin(), segs.end(), 'I'));
    CPPUNIT_ASSERT(!_yq->validateIntl(segs.begin(), segs.end(), '*'));
  }

  void testValidateEquipment()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    seg->equipmentType() = "EQ";
    segs.push_back(seg);

    CPPUNIT_ASSERT(_yq->validateEquipment(segs.begin(), segs.end(), ""));
    CPPUNIT_ASSERT(!_yq->validateEquipment(segs.begin(), segs.end(), "AB"));
    CPPUNIT_ASSERT(_yq->validateEquipment(segs.begin(), segs.end(), "EQ"));
  }

  void testValidateCarrierTableEmpty()
  {
    std::vector<TravelSeg*> segs;
    TaxCarrierFlightInfo* t186 = _memHandle.create<TaxCarrierFlightInfo>();

    CPPUNIT_ASSERT(_yq->validateCarrierTable(segs.begin(), segs.end(), t186));
  }

  void testValidateCarrierTableCarriers()
  {
    std::vector<TravelSeg*> segs;
    TaxCarrierFlightInfo* t186 = _memHandle.create<TaxCarrierFlightInfo>();

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->setMarketingCarrierCode("BB");
    seg->setOperatingCarrierCode("BB");
    seg->marketingFlightNumber() = 123;
    segs.push_back(seg);

    CarrierFlightSeg* tseg1 = new CarrierFlightSeg();
    tseg1->marketingCarrier() = "AA";
    tseg1->flt1() = 123;
    t186->segs().push_back(tseg1);

    CarrierFlightSeg* tseg2 = new CarrierFlightSeg();
    tseg2->operatingCarrier() = "AA";
    tseg2->flt1() = 123;
    t186->segs().push_back(tseg2);
    CPPUNIT_ASSERT(!_yq->validateCarrierTable(segs.begin(), segs.end(), t186));
  }

  void testValidateCarrierTableFlights()
  {
    std::vector<TravelSeg*> segs;
    TaxCarrierFlightInfo* t186 = _memHandle.create<TaxCarrierFlightInfo>();

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->marketingFlightNumber() = 123;
    segs.push_back(seg);

    CarrierFlightSeg* tseg = new CarrierFlightSeg();
    t186->segs().push_back(tseg);

    tseg->flt1() = -1; // ANY_FLIGHT
    CPPUNIT_ASSERT(_yq->validateCarrierTable(segs.begin(), segs.end(), t186));

    tseg->flt1() = 0;
    CPPUNIT_ASSERT(!_yq->validateCarrierTable(segs.begin(), segs.end(), t186));

    tseg->flt2() = 0;
    tseg->flt1() = 123;
    CPPUNIT_ASSERT(_yq->validateCarrierTable(segs.begin(), segs.end(), t186));

    tseg->flt1() = 999;
    tseg->flt2() = 123;
    CPPUNIT_ASSERT(!_yq->validateCarrierTable(segs.begin(), segs.end(), t186));

    tseg->flt1() = 100;
    tseg->flt2() = 999;
    CPPUNIT_ASSERT(_yq->validateCarrierTable(segs.begin(), segs.end(), t186));
  }

  void testCheckLocation()
  {
    Loc loc;

    loc.loc() = "KRK";
    CPPUNIT_ASSERT(_yq->checkLocation(loc, "KRK", 'P', "ATP", "AA"));

    CPPUNIT_ASSERT(!_yq->checkLocation(loc, "0", 'Z', "ATP", "AA"));

    CPPUNIT_ASSERT(!_yq->checkLocation(loc, "0", 'U', "ATP", "AA"));
  }

  void testValidateBookingCodeEmpty()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    seg->setBookingCode("AB");
    segs.push_back(seg);

    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::MatchResult::UNCONDITIONALLY),
        static_cast<uint8_t>(_yq->validateBookingCode(segs.begin(), segs.end(), *fees)));
  }

  void testValidateBookingCodeSoftPass()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "AA";

    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    seg->setBookingCode("AB");
    segs.push_back(seg);

    _yq->_oneFPMode = false;
    _trx->noPNRPricing() = true;

    fees->sectorPortionOfTvlInd() = 'P';
    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::MatchResult::P_CONDITIONALLY),
        static_cast<uint8_t>(_yq->validateBookingCode(segs.begin(), segs.end(), *fees)));

    fees->sectorPortionOfTvlInd() = 'S';
    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::MatchResult::S_CONDITIONALLY),
        static_cast<uint8_t>(_yq->validateBookingCode(segs.begin(), segs.end(), *fees)));
  }

  void testValidateBookingCodePass()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "A";
    fees->bookingCode2() = "B";

    _yq->_itin.travelSeg()[0]->setBookingCode("A");
    _yq->_itin.travelSeg()[1]->setBookingCode("B");
    _yq->_rbdBySegNo[0] = "A";
    _yq->_rbdBySegNo[1] = "B";

    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::MatchResult::UNCONDITIONALLY),
        static_cast<uint8_t>(_yq->validateBookingCode(
            _yq->_itin.travelSeg().begin(), _yq->_itin.travelSeg().begin() + 2, *fees)));
  }

  void testValidateBookingCodeFailed()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "A";
    fees->bookingCode2() = "B";
    fees->bookingCode3() = "C";

    _yq->_itin.travelSeg()[0]->setBookingCode("J");
    _yq->_rbdBySegNo[0] = "J";

    fees->sectorPortionOfTvlInd() = 'P';
    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::MatchResult::P_FAILED),
        static_cast<uint8_t>(_yq->validateBookingCode(
            _yq->_itin.travelSeg().begin(), _yq->_itin.travelSeg().begin() + 1, *fees)));

    fees->sectorPortionOfTvlInd() = 'S';
    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::MatchResult::S_FAILED),
        static_cast<uint8_t>(_yq->validateBookingCode(
            _yq->_itin.travelSeg().begin(), _yq->_itin.travelSeg().begin() + 1, *fees)));
  }

  void testValidateFbcTktDsg()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();

    fees->fareBasis() = "";
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::FBCMatch::ALL),
                         static_cast<uint8_t>(_yq->validateFbcTktDsg(0, *fees)));

    fees->fareBasis() = "AAA";

    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::FBCMatch::NONE),
                         static_cast<uint8_t>(_yq->validateFbcTktDsg(0, *fees)));

    _yq->_fbcBySegNo.insert(std::make_pair(0, "AAA"));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::FBCMatch::ALL),
                         static_cast<uint8_t>(_yq->validateFbcTktDsg(0, *fees)));

    _yq->_fbcBySegNo.insert(std::make_pair(0, "BBB"));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::FBCMatch::SOME),
                         static_cast<uint8_t>(_yq->validateFbcTktDsg(0, *fees)));
  }

  void testValidateVia()
  {
    std::vector<TravelSeg*> segs;
    TravelSeg* seg = _memHandle.create<AirSeg>();
    seg->forcedStopOver() = 'T';
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "KRK";
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "DFW";
    seg->origin() = origin;
    seg->destination() = destination;
    segs.push_back(seg);

    CPPUNIT_ASSERT(_yq->validateVia(segs.begin(), segs.end(), "", 'P', "ATP", "AA", ' ', 1, true));

    CPPUNIT_ASSERT(_yq->validateVia(segs.begin(), segs.end(), "", 'P', "ATP", "AA", 'S', 1, true));

    CPPUNIT_ASSERT(
        _yq->validateVia(segs.begin(), segs.end(), "DFW", 'P', "ATP", "AA", ' ', 1, true));

    CPPUNIT_ASSERT(
        !_yq->validateVia(segs.begin(), segs.end(), "DFW", 'P', "ATP", "AA", 'C', 1, true));

    CPPUNIT_ASSERT(
        _yq->validateVia(segs.begin(), segs.end(), "DFW", 'P', "ATP", "AA", 'S', 1, true));
  }

  void testDetermineCarriersEmpty()
  {
    _itin->validatingCarrier() = "";
    _yq->determineCarriers();

    CPPUNIT_ASSERT_EQUAL(size_t(0), _yq->_carriers.size());
  }

  void testDetermineCarriersNoFees()
  {
    _itin->validatingCarrier() = "NN";
    _yq->determineCarriers();

    CPPUNIT_ASSERT_EQUAL(size_t(0), _yq->_carriers.size());
  }

  void testDetermineCarriersNoS2CarrierPresent()
   {
     _itin->validatingCarrier() = "NN";

     AirSeg* seg = _memHandle.create<AirSeg>();
     Loc* origin1 = _memHandle.create<Loc>();
     origin1->loc() = "KRK";
     Loc* destination1 = _memHandle.create<Loc>();
     destination1->loc() = "DFW";
     seg->origin() = origin1;
     seg->destination() = destination1;
     seg->fareBasisCode() = "AAA";
     seg->setBookingCode("YY");
     seg->setMarketingCarrierCode("NN");
     _itin->travelSeg().push_back(seg);
     _yq->determineCarriers();

     CPPUNIT_ASSERT_EQUAL(size_t(1), _yq->_carriers.size());
   }

  void testDetermineCarriers()
  {
    _itin->validatingCarrier() = "AA";

    TravelSeg* surf = _memHandle.create<SurfaceSeg>();
    _itin->travelSeg().push_back(surf);

    _yq->determineCarriers();

    CPPUNIT_ASSERT_EQUAL(size_t(2), _yq->_carriers.size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), _yq->_carriers[0]);
    CPPUNIT_ASSERT_EQUAL(CarrierCode(""), _yq->_carriers[1]);
  }

  void testDetermineCarriersInvalidT190()
  {
    _itin->validatingCarrier() = "CC";

    _yq->determineCarriers();

    CPPUNIT_ASSERT_EQUAL(size_t(0), _yq->_carriers.size());
  }

  void testValidateT190Null()
  {
    TaxCarrierAppl* appl = 0;

    CPPUNIT_ASSERT(_yq->validateT190(appl, "AA"));
  }

  void testValidateT190DollarCarrier()
  {
    TaxCarrierAppl* appl = _memHandle.create<TaxCarrierAppl>();
    TaxCarrierApplSeg* segment = new TaxCarrierApplSeg;
    segment->carrier() = DOLLAR_CARRIER;
    appl->segs().push_back(segment);

    CPPUNIT_ASSERT(_yq->validateT190(appl, "AA"));
  }

  void testValidateT190NotSameCarrier()
  {
    TaxCarrierAppl* appl = _memHandle.create<TaxCarrierAppl>();
    TaxCarrierApplSeg* segment = new TaxCarrierApplSeg;
    segment->carrier() = "BB";
    appl->segs().push_back(segment);

    CPPUNIT_ASSERT(!_yq->validateT190(appl, "AA"));
  }

  void testValidateT190ApplInd()
  {
    TaxCarrierAppl* appl = _memHandle.create<TaxCarrierAppl>();
    TaxCarrierApplSeg* segment = new TaxCarrierApplSeg;
    segment->carrier() = "AA";
    segment->applInd() = 'X';
    appl->segs().push_back(segment);

    CPPUNIT_ASSERT(!_yq->validateT190(appl, "AA"));
  }

  void testValidateT190Valid()
  {
    TaxCarrierAppl* appl = _memHandle.create<TaxCarrierAppl>();
    TaxCarrierApplSeg* segment = new TaxCarrierApplSeg;
    segment->carrier() = "AA";
    appl->segs().push_back(segment);

    CPPUNIT_ASSERT(_yq->validateT190(appl, "AA"));
  }

  void testYQYRApplicationEmpty()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 0;
    fees->cur() = "PLN";

    PricingOptions* opt = _memHandle.create<PricingOptions>();
    opt->currencyOverride() = "USD";
    _trx->setOptions(opt);

    YQYRCalculator::YQYRApplication appl(fees, 1, 3, *_trx, "");
    CPPUNIT_ASSERT_EQUAL(1, appl.first);
    CPPUNIT_ASSERT_EQUAL(3, appl.last);
    CPPUNIT_ASSERT_EQUAL(0.0, appl.amount);
  }

  void testYQYRApplication()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "PLN";

    PricingOptions* opt = _memHandle.create<PricingOptions>();
    opt->currencyOverride() = "USD";
    _trx->setOptions(opt);

    YQYRCalculator::YQYRApplication appl(fees, 1, 3, *_trx, "");
    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      CPPUNIT_ASSERT_EQUAL(1, appl.first);
      CPPUNIT_ASSERT_EQUAL(3, appl.last);
      // 100.0 * 3 * 1 = 300.0
      CPPUNIT_ASSERT_EQUAL(300.0, appl.amount);
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(1, appl.first);
      CPPUNIT_ASSERT_EQUAL(3, appl.last);
      // 100.0 * 3 * 1.55 / 4.86 = 95.67 -> 95.7
      CPPUNIT_ASSERT_EQUAL(95.7, appl.amount);
    }
  }

  void testYQYRApplicationFeeApplInd()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 0;
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 3, *_trx, "");
    CPPUNIT_ASSERT_EQUAL('1', appl.feeApplInd());
  }

  void testYQYRApplicationCompare()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* feesLess = _memHandle.create<YQYRFees>();
    feesLess->feeAmount() = 100;
    feesLess->cur() = "GBP";

    YQYRFees* feesMore = _memHandle.create<YQYRFees>();
    feesMore->feeAmount() = 300;
    feesMore->cur() = "GBP";

    YQYRCalculator::YQYRApplication applLess(feesLess, 1, 3, *_trx, "");
    YQYRCalculator::YQYRApplication applMore(feesMore, 1, 3, *_trx, "");

    CPPUNIT_ASSERT(applLess < applMore);
    CPPUNIT_ASSERT(!(applMore < applLess));
  }

  void testInitMFMsFmp()
  {
    FareMarketPath* path = _memHandle.create<FareMarketPath>();
    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    path->fareMarketPath().push_back(mfm);
    _yq->_fareMarketPath = path;

    _yq->initMFMs();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_allMFMs.size());
    CPPUNIT_ASSERT_EQUAL(mfm, _yq->_allMFMs.front());
  }

  void testInitMFMsFmpMatrix()
  {
    std::vector<MergedFareMarket*>* vec = _memHandle.create<std::vector<MergedFareMarket*> >();
    FmpMatrixPtr fmpm(new FareMarketPathMatrix(*_trx, *_itin, *vec));
    _itin->fmpMatrix() = fmpm;
    FareMarketPath* path = _memHandle.create<FareMarketPath>();
    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    path->fareMarketPath().push_back(mfm);
    _itin->fmpMatrix()->fareMarketPathMatrix().push_back(path);

    _yq->initMFMs();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_allMFMs.size());
    CPPUNIT_ASSERT_EQUAL(mfm, _yq->_allMFMs.front());
  }

  void testInitFBCMap()
  {
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();

    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->fareClass() = "AB/CD";
    fare->setFareInfo(fi);
    ptf->setFare(fare);

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->travelSeg().push_back(_itin->travelSeg()[0]);
    fu->travelSeg().push_back(_itin->travelSeg()[1]);
    PaxTypeFare::SegmentStatus ss;
    ss._bkgCodeReBook = "XY";
    ss._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
    fu->segmentStatus().push_back(ss);
    fu->paxTypeFare() = ptf;

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage().push_back(fu);

    FarePath* path = _memHandle.create<FarePath>();
    path->pricingUnit().push_back(pu);

    _yq->initFBCMap(path);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), _yq->_fbcBySegNo.size());
    CPPUNIT_ASSERT_EQUAL(std::string("AB/AAA"), _yq->_fbcBySegNo.find(0)->second);
    CPPUNIT_ASSERT_EQUAL(std::string("AB/AAA"), _yq->_fbcBySegNo.find(1)->second);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), _yq->_rbdBySegNo.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<BookingCode>("XY"), _yq->_rbdBySegNo[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<BookingCode>("ZZ"), _yq->_rbdBySegNo[1]);
  }

  void testInitFBCMapNoFarePath()
  {
    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg().push_back(_itin->travelSeg()[0]);
    mfm->mergedFareMarket().push_back(fm);
    _yq->_allMFMs.push_back(mfm);
    _yq->initFBCMap(0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _yq->_fbcBySegNo.size());
  }

  void testInitFBCMapForMFMEmpty()
  {
    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg().push_back(_itin->travelSeg()[0]);
    mfm->mergedFareMarket().push_back(fm);
    _yq->initFBCMapForMFM(mfm);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _yq->_fbcBySegNo.size());
  }

  void testInitFBCMapForMFM()
  {
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();

    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    _trx->itin().push_back(_itin);

    FareMarket* fm = _memHandle.create<FareMarket>();

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->fareClass() = "AB/CD";
    fare->setFareInfo(fi);
    ptf->setFare(fare);
    ptf->fareMarket() = fm;

    PaxTypeBucket* ptc = _memHandle.create<PaxTypeBucket>();
    ptc->requestedPaxType() = _farePath->paxType();
    ptc->paxTypeFare().push_back(ptf);

    fm->travelSeg().push_back(_itin->travelSeg()[0]);
    fm->paxTypeCortege().push_back(*ptc);

    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    mfm->mergedFareMarket().push_back(fm);

    _yq->initFBCMapForMFM(mfm);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_fbcBySegNo.size());
    CPPUNIT_ASSERT_EQUAL(std::string("AB/AAA"), _yq->_fbcBySegNo.find(0)->second);
  }

  void testGetMFMs()
  {
    FareMarketPath* fmp = _memHandle.create<FareMarketPath>();
    std::vector<MergedFareMarket*> mfm;

    MergedFareMarket* mfm1 = _memHandle.create<MergedFareMarket>();
    MergedFareMarket* mfm2 = _memHandle.create<MergedFareMarket>();

    fmp->fareMarketPath().push_back(mfm1);

    std::pair<MergedFareMarket*, std::vector<FareMarketPath*> > sideTrip;
    sideTrip.first = _memHandle.create<MergedFareMarket>();
    FareMarketPath* sideFmp = _memHandle.create<FareMarketPath>();
    sideFmp->fareMarketPath().push_back(mfm2);
    sideTrip.second.push_back(sideFmp);
    fmp->sideTrips().insert(sideTrip);

    _yq->getMFMs(fmp, mfm);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), mfm.size());
    CPPUNIT_ASSERT(std::find(mfm.begin(), mfm.end(), mfm1) != mfm.end());
    CPPUNIT_ASSERT(std::find(mfm.begin(), mfm.end(), mfm2) != mfm.end());
  }

  void testInitializeYQYRListEmpty() { CPPUNIT_ASSERT(!_yq->initializeYQYRList("NO")); }

  void testInitializeYQYRList()
  {
    CPPUNIT_ASSERT(_yq->initializeYQYRList("AA"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_feesByCode.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_feesByCode.front().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<TaxCode>("AB"), _yq->_feesByCode.front().front()->taxCode());
  }

  void testAnalyzeItinDomestic()
  {
    Itin* itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    origin1->nation() = "PL";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "WAW";
    destination1->nation() = "PL";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    ((AirSeg*)seg1)->carrier() = "LO";
    itin->travelSeg().push_back(seg1);

    TravelSeg* seg2 = _memHandle.create<AirSeg>();
    Loc* origin2 = _memHandle.create<Loc>();
    origin2->loc() = "WAW";
    Loc* destination2 = _memHandle.create<Loc>();
    destination2->loc() = "KRK";
    destination2->nation() = "PL";
    seg2->origin() = origin2;
    seg2->destination() = destination2;
    ((AirSeg*)seg2)->carrier() = "LH";
    itin->travelSeg().push_back(seg2);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();

    YQYRCalculator* yq = _memHandle.create<YQYRCalculator>(*_trx, *itin, farePath);

    yq->analyzeItin(_farePath);

    CPPUNIT_ASSERT_EQUAL(const_cast<const TravelSeg*>(seg2), yq->_furthestSeg);
    CPPUNIT_ASSERT(yq->_returnsToOrigin);
    CPPUNIT_ASSERT(!yq->_isOnlineItin);
  }

  void testAnalyzeItinInternational()
  {
    Itin* itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    origin1->nation() = "PL";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "DFW";
    destination1->nation() = "US";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    seg1->forcedStopOver() = 'T';
    itin->travelSeg().push_back(seg1);

    TravelSeg* seg2 = _memHandle.create<AirSeg>();
    Loc* origin2 = _memHandle.create<Loc>();
    origin2->loc() = "DFW";
    Loc* destination2 = _memHandle.create<Loc>();
    destination2->loc() = "LAX";
    seg2->origin() = origin2;
    seg2->destination() = destination2;
    seg2->forcedStopOver() = 'T';
    itin->travelSeg().push_back(seg2);

    TravelSeg* seg3 = _memHandle.create<AirSeg>();
    Loc* origin3 = _memHandle.create<Loc>();
    origin3->loc() = "LAX";
    Loc* destination3 = _memHandle.create<Loc>();
    destination3->loc() = "WAW";
    destination3->nation() = "PL";
    seg3->origin() = origin3;
    seg3->destination() = destination3;
    seg3->forcedStopOver() = 'T';
    itin->travelSeg().push_back(seg3);

    TravelSeg* seg4 = _memHandle.create<AirSeg>();
    Loc* origin4 = _memHandle.create<Loc>();
    origin4->loc() = "WAW";
    Loc* destination4 = _memHandle.create<Loc>();
    destination4->loc() = "KRK";
    destination4->nation() = "PL";
    seg4->origin() = origin4;
    seg4->destination() = destination4;
    seg4->forcedStopOver() = 'T';
    itin->travelSeg().push_back(seg4);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();

    YQYRCalculator* yq = _memHandle.create<YQYRCalculator>(*_trx, *itin, farePath);

    yq->analyzeItin(_farePath);

    CPPUNIT_ASSERT_EQUAL(const_cast<const TravelSeg*>(seg2), yq->_furthestSeg);
    CPPUNIT_ASSERT(yq->_returnsToOrigin);
    CPPUNIT_ASSERT(yq->_isOnlineItin);
  }

  void testIsSimpleItinDomestic()
  {
    Itin* itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    origin1->nation() = "PL";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "WAW";
    destination1->nation() = "PL";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    seg1->boardMultiCity() = "KRK";
    seg1->offMultiCity() = "WAW";
    itin->travelSeg().push_back(seg1);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();

    YQYRCalculator* yq = _memHandle.create<YQYRCalculator>(*_trx, *itin, farePath);

    CPPUNIT_ASSERT(yq->isSimpleItin(*_trx, *itin));
  }

  void testIsSimpleItinDomesticReturn()
  {
    Itin* itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    origin1->nation() = "PL";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "WAW";
    destination1->nation() = "PL";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    itin->travelSeg().push_back(seg1);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();

    YQYRCalculator* yq = _memHandle.create<YQYRCalculator>(*_trx, *itin, farePath);

    CPPUNIT_ASSERT(yq->isSimpleItin(*_trx, *itin));
  }

  void testIsSimpleItinInternationalSurface()
  {
    Itin* itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    origin1->nation() = "PL";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "WAW";
    destination1->nation() = "PL";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    itin->travelSeg().push_back(seg1);

    TravelSeg* seg2 = _memHandle.create<SurfaceSeg>();
    Loc* origin2 = _memHandle.create<Loc>();
    origin2->loc() = "WAW";
    Loc* destination2 = _memHandle.create<Loc>();
    destination2->loc() = "DFW";
    destination2->nation() = "US";
    seg2->origin() = origin2;
    seg2->destination() = destination2;
    itin->travelSeg().push_back(seg2);

    TravelSeg* seg3 = _memHandle.create<AirSeg>();
    Loc* origin3 = _memHandle.create<Loc>();
    origin3->loc() = "DFW";
    Loc* destination3 = _memHandle.create<Loc>();
    destination3->loc() = "KRK";
    destination3->nation() = "PL";
    seg3->origin() = origin3;
    seg3->destination() = destination3;
    itin->travelSeg().push_back(seg3);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();

    YQYRCalculator* yq = _memHandle.create<YQYRCalculator>(*_trx, *itin, farePath);

    CPPUNIT_ASSERT(!yq->isSimpleItin(*_trx, *itin));
  }

  void testIsSimpleItinInternationalAir()
  {
    Itin* itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = "KRK";
    origin1->nation() = "PL";
    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = "WAW";
    destination1->nation() = "PL";
    seg1->origin() = origin1;
    seg1->destination() = destination1;
    itin->travelSeg().push_back(seg1);

    TravelSeg* seg2 = _memHandle.create<AirSeg>();
    Loc* origin2 = _memHandle.create<Loc>();
    origin2->loc() = "WAW";
    Loc* destination2 = _memHandle.create<Loc>();
    destination2->loc() = "DFW";
    destination2->nation() = "US";
    seg2->origin() = origin2;
    seg2->destination() = destination2;
    itin->travelSeg().push_back(seg2);

    TravelSeg* seg3 = _memHandle.create<AirSeg>();
    Loc* origin3 = _memHandle.create<Loc>();
    origin3->loc() = "DFW";
    Loc* destination3 = _memHandle.create<Loc>();
    destination3->loc() = "KRK";
    destination3->nation() = "PL";
    seg3->origin() = origin3;
    seg3->destination() = destination3;
    itin->travelSeg().push_back(seg3);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();

    YQYRCalculator* yq = _memHandle.create<YQYRCalculator>(*_trx, *itin, farePath);

    CPPUNIT_ASSERT(!yq->isSimpleItin(*_trx, *itin));
  }

  void testProcess()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    _itin->calculationCurrency() = "USD";
    _yq->_carriers.push_back("NO");

    _yq->process();

    CPPUNIT_ASSERT_EQUAL(0.0, _yq->_lowerBound);
  }

  void testProcessDirectionOut()
  {
    _yq->_currentBucket = _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    _yq->_feeList = _memHandle.create<YQYRCalculator::FeeListT>(_poolFactory);
    _yq->processDirection("AA", YQYRCalculator::Directions::OUTBOUND, *_yq->_feeList);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_currentBucket->feesBySlice.size());
  }

  void testProcessDirectionIn()
  {
    _itin->travelSeg().push_back(_memHandle.create<SurfaceSeg>());
    _yq->_currentBucket = _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    _yq->_feeList = _memHandle.create<YQYRCalculator::FeeListT>(_poolFactory);
    _yq->processDirection("BB", YQYRCalculator::Directions::INBOUND, *_yq->_feeList);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _yq->_currentBucket->feesBySlice.size());
  }

  void testProcessDirectionOther()
  {
    _yq->_currentBucket = _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    _yq->_feeList = _memHandle.create<YQYRCalculator::FeeListT>(_poolFactory);
    _yq->processDirection("AA", static_cast<YQYRCalculator::Directions>(10), *_yq->_feeList);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _yq->_currentBucket->feesBySlice.size());
  }

  void testYQYRPathExtend()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 300;
    fees->cur() = "GBP";

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath e1;
    YQYRCalculator::YQYRPath* e2;
    e2 = e1.extend(appl);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), e1.yqyrs.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), e2->yqyrs.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<MoneyAmount>(300), e2->amount);

    delete e2;
  }

  void testYQYRPathAdd()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 300;
    fees->cur() = "GBP";

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath e1;
    e1.add(appl);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), e1.yqyrs.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<MoneyAmount>(300), e1.amount);
  }

  void testMatchFeeRecordFailJourney()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->journeyLoc1() = "ABC";
    fees->journeyLocType1() = 'Z';
    fees->journeyLoc2() = "DEF";
    fees->journeyLocType2() = 'Z';
    fees->journeyLoc1Ind() = 'A';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailValidatingCxr()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->vendor() = "ATP";
    fees->taxCarrierApplTblItemNo() = 1;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailReturnToOrigin()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'N';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailPTC()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->psgType() = CHILD;
    _farePath->paxType()->paxType() = ADULT;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailPOS()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->posLoc() = "PL";
    fees->posLocType() = NATION;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailPOT()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->potLoc() = "PL";
    fees->potLocType() = NATION;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailAgency()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->posLocaleType() = 'X';
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailTravelDate()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    _itin->setTravelDate(DateTime(2013, 2, 2));
    fees->effDate() = DateTime(2013, 3, 3);
    const_cast<DateTime&>(_yq->_travelDate) = _itin->travelDate();
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordFailPercent()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->percent() = 10;
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::NEVER),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordPortion()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->sectorPortionOfTvlInd() = 'P';
    fees->firstTktDate() = fees->lastTktDate() = _request->ticketingDT();
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::UNCONDITIONALLY),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMatchFeeRecordSector()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->sectorPortionOfTvlInd() = 'S';
    fees->firstTktDate() = fees->lastTktDate() = _request->ticketingDT();
    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(YQYRCalculator::MatchResult::UNCONDITIONALLY),
                         static_cast<uint8_t>(_yq->matchFeeRecord(fees, 0, 3)));
  }

  void testMakeSliceKey() { CPPUNIT_ASSERT_EQUAL(0x0102, _yq->makeSliceKey(1, 2)); }

  void testProcessSliceEndBeforeStart()
  {
    CPPUNIT_ASSERT_EQUAL(static_cast<const std::vector<const YQYRCalculator::YQYRPath*>*>(0),
                         _yq->processSlice(1, 0, *_yq->_feeList));
  }

  void testProcessSliceFromCache()
  {
    std::vector<const YQYRCalculator::YQYRPath*>* ypv =
        _memHandle.create<std::vector<const YQYRCalculator::YQYRPath*> >();
    _yq->_subSliceResults.insert(std::make_pair(_yq->makeSliceKey(5, 6), ypv));
    CPPUNIT_ASSERT_EQUAL(const_cast<const std::vector<const YQYRCalculator::YQYRPath*>*>(ypv),
                         _yq->processSlice(5, 6, *_yq->_feeList));
  }

  void testProcessSliceFail()
  {
    _yq->_currentBucket = _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    _yq->_feeList = _memHandle.create<YQYRCalculator::FeeListT>(_poolFactory);

    YQYRFees* fees;

    fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'N';
    fees->carrier() = "A0";
    _yq->_feeList->push_back(fees);

    fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->carrier() = "A1";
    fees->sectorPortionOfTvlInd() = 'S';
    fees->fareBasis() = "AAA";
    _yq->_feeList->push_back(fees);

    fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->carrier() = "A2";
    fees->sectorPortionOfTvlInd() = 'P';
    fees->fareBasis() = "AAA";
    fees->stopConnectInd() = 'C';
    _yq->_feeList->push_back(fees);

    const std::vector<const YQYRCalculator::YQYRPath*>* v = _yq->processSlice(0, 2, *_yq->_feeList);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), v->size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), (*v)[0]->yqyrs.size());
  }

  void testProcessSlicePass()
  {
    _yq->_currentBucket = _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    _yq->_feeList = _memHandle.create<YQYRCalculator::FeeListT>(_poolFactory);

    _yq->_fbcBySegNo.insert(std::make_pair(0, "AAA"));
    _yq->_fbcBySegNo.insert(std::make_pair(0, "BBB"));
    _yq->_fbcBySegNo.insert(std::make_pair(1, "AAA"));
    _yq->_fbcBySegNo.insert(std::make_pair(1, "BBB"));
    _yq->_yqyrApplCountLimit = 100;

    YQYRFees* fees;

    fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->carrier() = "A1";
    fees->sectorPortionOfTvlInd() = 'S';
    fees->fareBasis() = "AAA";
    fees->firstTktDate() = fees->lastTktDate() = _request->ticketingDT();
    _yq->_feeList->push_back(fees);

    fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->carrier() = "A2";
    fees->sectorPortionOfTvlInd() = 'P';
    fees->fareBasis() = "AAA";
    fees->firstTktDate() = fees->lastTktDate() = _request->ticketingDT();
    _yq->_feeList->push_back(fees);

    fees = _memHandle.create<YQYRFees>();
    fees->returnToOrigin() = 'Y';
    fees->carrier() = "A0";
    fees->firstTktDate() = fees->lastTktDate() = _request->ticketingDT();
    _yq->_feeList->push_back(fees);

    const std::vector<const YQYRCalculator::YQYRPath*>* v = _yq->processSlice(0, 1, *_yq->_feeList);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), v->size());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A1"), (*v)[0]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A1"), (*v)[1]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A1"), (*v)[2]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A2"), (*v)[3]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A2"), (*v)[4]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A2"), (*v)[5]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A2"), (*v)[6]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A0"), (*v)[7]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A0"), (*v)[8]->yqyrs[0].yqyr->carrier());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("A0"), (*v)[9]->yqyrs[0].yqyr->carrier());
  }

  void testProcessCarrier()
  {
    _yq->_valCxrContext->_concurringCarriers.push_back("AA");
    _yq->processCarrier("AA");
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_valCxrContext->_feeBuckets.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<CarrierCode>("AA"), _yq->_valCxrContext->_feeBuckets[0]->cxr);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _yq->_valCxrContext->_feeBuckets[0]->feesBySlice.size());
    CPPUNIT_ASSERT_EQUAL(
        static_cast<uint8_t>(YQYRCalculator::Directions::OUTBOUND),
        static_cast<uint8_t>(_yq->_valCxrContext->_feeBuckets[0]->feesBySlice[0]->dir));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         _yq->_valCxrContext->_feeBuckets[0]->feesBySlice[0]->yqyrpaths.size());
  }

  void testMatchFareBasisFromCache()
  {
    _yq->_fbcMatchMap.insert(std::make_pair(YQYRCalculator::FBCMatchMapKey("ABC", "DEF"), true));
    _yq->_fbcMatchMap.insert(std::make_pair(YQYRCalculator::FBCMatchMapKey("GHI", "JKL"), false));
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC", "DEF"));
    CPPUNIT_ASSERT(!_yq->matchFareBasis("GHI", "JKL"));
  }

  void testMatchFareBasisEmpty() { CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC", "")); }

  void testMatchFareBasisImplEmpty() { CPPUNIT_ASSERT(!YQYR::YQYRUtils::matchFareBasisCode("ABC", "")); }

  void testMatchFareBasisImplNoTktDesignator()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC", "ABC/DEF"));
  }

  void testMatchFareBasisImplNoTktDesignatorNoMatchClass()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC", "AB-CD"));
  }

  void testMatchFareBasisImplNoTktDesignatorMatchClass()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC", "ABC"));
  }

  void testMatchFareBasisImplNoPtfDesignator()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/DEF", "ABC"));
  }

  void testMatchFareBasisImplNotMatchingFareBasisCodes()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/DEF", "GHI/JKL"));
  }

  void testMatchFareBasisImplEmptyTktDesignator()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC/", "ABC/JKL"));
  }

  void testMatchFareBasisImplOnlyOneTktDesignatorAsterisk()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC/*", "ABC/JKL"));
  }

  void testMatchFareBasisImplOnlyOneTktDesignatorLongAsterisk()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC/J*", "ABC/JKL"));
  }

  void testMatchFareBasisImplOnlyOneTktDesignatorHypenMatch()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC/J-", "ABC/JKL"));
  }

  void testMatchFareBasisImplOnlyOneTktDesignatorHypenNoMatch()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/A-", "ABC/JKL"));
  }

  void testMatchFareBasisImplOnlyOnePtfDesignator()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/DEF/GHI", "ABC/JKL"));
  }

  void testMatchFareBasisImplFirstTktDesignatorAsterisk()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/D*/GHI", "ABC/JKL/MNO"));
  }

  void testMatchFareBasisImplFirstTktDesignatorEmpty()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC//GHI", "ABC/DEF/JKL"));
  }

  void testMatchFareBasisImplSecondTktDesignatorAsterisk()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/DEF/G*", "ABC/DEF/MNO"));
  }

  void testMatchFareBasisImplSecondTktDesignatorOnlyAsterisk()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC/DEF/*", "ABC/DEF/MNO"));
  }

  void testMatchFareBasisImplSecondDesignatorMatch()
  {
    CPPUNIT_ASSERT(_yq->matchFareBasis("ABC/DEF/GHI", "ABC/DEF/GHI"));
  }

  void testMatchFareBasisImplSecondDesignatorNoMatch()
  {
    CPPUNIT_ASSERT(!_yq->matchFareBasis("ABC/DEF/GHI", "ABC/DEF/MNO"));
  }

  void testFindMatchingPathsOneFPEmpty()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = true;

    YQYRCalculator::FeesBySlice* fees = _memHandle.create<YQYRCalculator::FeesBySlice>();
    fees->dir = YQYRCalculator::Directions::INBOUND;

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(fees);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT(results.empty());
  }

  void testFindMatchingPathsOneFPMatched()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = true;

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
    CPPUNIT_ASSERT_EQUAL(const_cast<const YQYRFees*>(fees), results[0].yqyr);
  }

  void testFindMatchingPathsOutbound()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::OUTBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
    CPPUNIT_ASSERT_EQUAL(const_cast<const YQYRFees*>(fees), results[0].yqyr);
  }

  void testFindMatchingPathsInbound()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
    CPPUNIT_ASSERT_EQUAL(const_cast<const YQYRFees*>(fees), results[0].yqyr);
  }

  void testFindMatchingPathsOutboundSkip()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    _request->originBasedRTPricing() = true;
    _trx->outboundDepartureDate() = DateTime(2013, 1, 1);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::OUTBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT(results.empty());
  }

  void testFindMatchingPathsInboundSkip()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    _request->originBasedRTPricing() = true;
    _trx->outboundDepartureDate() = DateTime();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT(results.empty());
  }

  void testFindMatchingPathsJourney()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '2';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
    CPPUNIT_ASSERT_EQUAL(const_cast<const YQYRFees*>(fees), results[0].yqyr);
  }

  void testFindMatchingPathsJourneyOutboundSkip()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    _request->originBasedRTPricing() = true;
    _trx->outboundDepartureDate() = DateTime(2013, 1, 1);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '2';

    YQYRCalculator::YQYRApplication appl(fees, 0, 0, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::OUTBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
  }

  void testFindMatchingPathsJourneyInboundSkip()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    _request->originBasedRTPricing() = true;
    _trx->outboundDepartureDate() = DateTime();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '2';

    YQYRCalculator::YQYRApplication appl(fees, 3, 3, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
  }

  void testFindMatchingPathsBlank()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
    CPPUNIT_ASSERT_EQUAL(const_cast<const YQYRFees*>(fees), results[0].yqyr);
  }

  void testFindMatchingPathsBlankOutboundSkip()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    _request->originBasedRTPricing() = true;
    _trx->outboundDepartureDate() = DateTime(2013, 1, 1);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::OUTBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
  }

  void testFindMatchingPathsBlankInboundSkip()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    _request->originBasedRTPricing() = true;
    _trx->outboundDepartureDate() = DateTime();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), results.size());
  }

  void testFindMatchingPathsUnknown()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeApplInd() = '?';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT(results.empty());
  }

  void testFindMatchingPathsEmpty()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    std::vector<YQYRCalculator::YQYRApplication> results;
    _yq->_oneFPMode = false;

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findMatchingPaths(farePath, _yq->_valCxrContext->_cxr, results);

    CPPUNIT_ASSERT(results.empty());
  }

  void testChargeFarePathEmptyBucket()
  {
    FarePath* farePath = _memHandle.create<FarePath>();

    CPPUNIT_ASSERT_EQUAL(0.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathEmpty()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    CPPUNIT_ASSERT_EQUAL(0.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathOutbound()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::OUTBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    CPPUNIT_ASSERT_EQUAL(100.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathInbound()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    CPPUNIT_ASSERT_EQUAL(100.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathJourney()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '2';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    CPPUNIT_ASSERT_EQUAL(100.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathBlank()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    CPPUNIT_ASSERT_EQUAL(100.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathUnknown()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '?';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    CPPUNIT_ASSERT_EQUAL(0.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
  }

  void testChargeFarePathOverrideCurrency()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    opt->currencyOverride() = "USD";
    _trx->setOptions(opt);
    FarePath* farePath = _memHandle.create<FarePath>();

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);
    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      // 100 (GBP) * 1.55 (USD rate) = 155.0 (USD)
      CPPUNIT_ASSERT_EQUAL(155.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(100.0, _yq->chargeFarePath(*farePath, _itin->validatingCarrier()));
    }
  }

  void testCarrierFeeCodeBucketLowerBoundEmpty()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    CPPUNIT_ASSERT_EQUAL(0.0, bucket->lowerBound());
  }

  void testCarrierFeeCodeBucketLowerBoundOutbound()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::OUTBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    CPPUNIT_ASSERT_EQUAL(100.0, bucket->lowerBound());
  }

  void testCarrierFeeCodeBucketLowerBoundInbound()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '1';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->dir = YQYRCalculator::Directions::INBOUND;
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    CPPUNIT_ASSERT_EQUAL(100.0, bucket->lowerBound());
  }

  void testCarrierFeeCodeBucketLowerBoundJourney()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '2';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    CPPUNIT_ASSERT_EQUAL(100.0, bucket->lowerBound());
  }

  void testCarrierFeeCodeBucketLowerBoundBlank()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    CPPUNIT_ASSERT_EQUAL(100.0, bucket->lowerBound());
  }

  void testCarrierFeeCodeBucketLowerBoundUnknown()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = '?';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    CPPUNIT_ASSERT_THROW(bucket->lowerBound(), std::runtime_error);
  }

  void testFindLowerBound()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findLowerBound();

    CPPUNIT_ASSERT_EQUAL(100.0, _yq->_lowerBound);
  }

  void testFindLowerBoundEmpty()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    _yq->findLowerBound();

    CPPUNIT_ASSERT_EQUAL(0.0, _yq->_lowerBound);
  }

  void testFindLowerBoundOverrideCurrency()
  {
    _itin->calculationCurrency() = "GBP";
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    opt->currencyOverride() = "USD";
    _trx->setOptions(opt);

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->feeAmount() = 100;
    fees->cur() = "GBP";
    fees->feeApplInd() = ' ';

    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);

    YQYRCalculator::FeesBySlice* feesBySlice = _memHandle.create<YQYRCalculator::FeesBySlice>();
    feesBySlice->yqyrpaths.push_back(path);

    YQYRCalculator::CarrierFeeCodeBucket* bucket =
        _memHandle.create<YQYRCalculator::CarrierFeeCodeBucket>();
    bucket->feesBySlice.push_back(feesBySlice);

    _yq->_valCxrContext->_feeBuckets.push_back(bucket);

    _yq->findLowerBound();

    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      // 100 (GBP) * 1.55 (USD rate) = 155.0 (USD)
      CPPUNIT_ASSERT_EQUAL(155.0, _yq->_lowerBound);
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(100.0, _yq->_lowerBound);
    }
  }

  void testMergeEmpty()
  {
    const YQYRFees* fees1 = _memHandle.create<YQYRFees>();

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    YQYRCalculator::YQYRApplication appl(fees1, 1, 1, *_trx, "");
    std::vector<const YQYRCalculator::YQYRPath*>* ypvIn =
        _memHandle.create<std::vector<const YQYRCalculator::YQYRPath*> >();
    std::vector<const YQYRCalculator::YQYRPath*>* ypvOut =
        _memHandle.create<std::vector<const YQYRCalculator::YQYRPath*> >();

    _yq->_yqyrApplCountLimit = 100;
    _yq->merge(path, appl, ypvIn, ypvOut);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), ypvOut->size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), (*ypvOut)[0]->yqyrs.size());
    CPPUNIT_ASSERT_EQUAL(fees1, (*ypvOut)[0]->yqyrs[0].yqyr);
  }

  void testMerge()
  {
    const YQYRFees* fees1 = _memHandle.create<YQYRFees>();
    const YQYRFees* fees2 = _memHandle.create<YQYRFees>();

    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    YQYRCalculator::YQYRApplication appl(fees1, 1, 1, *_trx, "");

    YQYRCalculator::YQYRApplication applIn(fees2, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath* pathIn = _memHandle.create<YQYRCalculator::YQYRPath>();
    pathIn->add(applIn);
    std::vector<const YQYRCalculator::YQYRPath*>* ypvIn =
        _memHandle.create<std::vector<const YQYRCalculator::YQYRPath*> >();
    ypvIn->push_back(pathIn);
    std::vector<const YQYRCalculator::YQYRPath*>* ypvOut =
        _memHandle.create<std::vector<const YQYRCalculator::YQYRPath*> >();

    _yq->_yqyrApplCountLimit = 100;
    _yq->merge(path, appl, ypvIn, ypvOut);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), ypvOut->size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), (*ypvOut)[0]->yqyrs.size());
    CPPUNIT_ASSERT_EQUAL(fees1, (*ypvOut)[0]->yqyrs[0].yqyr);
    CPPUNIT_ASSERT_EQUAL(fees2, (*ypvOut)[0]->yqyrs[1].yqyr);
  }

  void testMatchPathFailFareBasisNotFound()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->fareBasis() = "ABC";
    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);
    std::map<int, std::string> fbcBySeg;
    std::map<int, BookingCode> bookingCodesBySeg;
    CPPUNIT_ASSERT(!_yq->matchPath(path, fbcBySeg, bookingCodesBySeg));
  }

  void testMatchPathFailFareBasisNoMatch()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->fareBasis() = "ABC";
    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);
    std::map<int, std::string> fbcBySeg;
    fbcBySeg.insert(std::make_pair(1, "DEF"));
    std::map<int, BookingCode> bookingCodesBySeg;
    CPPUNIT_ASSERT(!_yq->matchPath(path, fbcBySeg, bookingCodesBySeg));
  }

  void testMatchPathFailBookingCodeNotFound()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "XY";
    fees->bookingCode2() = "XY";
    fees->bookingCode3() = "XY";
    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);
    std::map<int, std::string> fbcBySeg;
    std::map<int, BookingCode> bookingCodesBySeg;
    CPPUNIT_ASSERT(!_yq->matchPath(path, fbcBySeg, bookingCodesBySeg));
  }

  void testMatchPathFailBookingCodeNoMatch()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->bookingCode1() = "XY";
    fees->bookingCode2() = "XY";
    fees->bookingCode3() = "XY";
    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);
    std::map<int, std::string> fbcBySeg;
    std::map<int, BookingCode> bookingCodesBySeg;
    bookingCodesBySeg.insert(std::make_pair(1, "ZY"));
    CPPUNIT_ASSERT(!_yq->matchPath(path, fbcBySeg, bookingCodesBySeg));
  }

  void testMatchPathPass()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->fareBasis() = "ABC";
    fees->bookingCode1() = "XY";
    fees->bookingCode2() = "XY";
    fees->bookingCode3() = "XY";
    YQYRCalculator::YQYRApplication appl(fees, 1, 1, *_trx, "");
    YQYRCalculator::YQYRPath* path = _memHandle.create<YQYRCalculator::YQYRPath>();
    path->add(appl);
    std::map<int, std::string> fbcBySeg;
    fbcBySeg.insert(std::make_pair(1, "ABC"));
    std::map<int, BookingCode> bookingCodesBySeg;
    bookingCodesBySeg.insert(std::make_pair(1, "XY"));
    CPPUNIT_ASSERT(_yq->matchPath(path, fbcBySeg, bookingCodesBySeg));
  }

  void testPrepareMatchMaps()
  {
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();

    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    PaxTypeFare::SegmentStatus ss;
    ss._bkgCodeReBook = "XY";
    ss._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->fareClass() = "AB/CD";
    fi->carrier() = "AA";
    fare->setFareInfo(fi);
    ptf->setFare(fare);

    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = ptf;
    fareUsage->travelSeg().push_back(_itin->travelSeg()[0]);
    fareUsage->segmentStatus().push_back(ss);

    PricingUnit* unit = _memHandle.create<PricingUnit>();
    unit->fareUsage().push_back(fareUsage);
    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->pricingUnit().push_back(unit);

    std::map<int, std::string> fbcBySeg;
    std::map<int, BookingCode> bookingCodesBySeg;

    _yq->prepareMatchMaps(farePath, fbcBySeg, bookingCodesBySeg);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), fbcBySeg.size());
    CPPUNIT_ASSERT_EQUAL(std::string("AB/AAA"), fbcBySeg.find(0)->second);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), bookingCodesBySeg.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<BookingCode>("XY"), bookingCodesBySeg[0]);
  }

  void testAreYQYRsExemptedAllTaxes()
  {
    _request->exemptAllTaxes() = 'T';

    CPPUNIT_ASSERT(!getYQYRCalculatorFromFactory());
  }

  void testAreYQYRsExemptedSpecificTaxesEmpty()
  {
    _request->exemptSpecificTaxes() = 'T';

    CPPUNIT_ASSERT(!getYQYRCalculatorFromFactory());
  }

  void testAreYQYRsExemptedSpecificTaxesYQYR()
  {
    _request->exemptSpecificTaxes() = 'T';
    _request->taxIdExempted().push_back("YQ");

    CPPUNIT_ASSERT(!getYQYRCalculatorFromFactory());
  }

  void testAreYQYRsExemptedSpecificTaxesNoYQYR()
  {
    _request->exemptSpecificTaxes() = 'T';
    _request->taxIdExempted().push_back("AY");

    CPPUNIT_ASSERT(getYQYRCalculatorFromFactory());
  }

  void testAreYQYRsExemptedNoTax()
  {
    _request->exemptAllTaxes() = 'F';
    _request->exemptSpecificTaxes() = 'F';

    CPPUNIT_ASSERT(getYQYRCalculatorFromFactory());
  }

  void testCalculatePercentageFee()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    FarePath* fp = _memHandle.create<FarePath>();
    fp->setTotalNUCAmount(100.0);
    fp->calculationCurrency() = "GBP";
    fp->baseFareCurrency() = "GBP";
    CPPUNIT_ASSERT_EQUAL(50.0, _yq->calculatePercentageFee(0.5, fp));
  }

  void testCalculatePercentageFeeNotMatchingCurrency()
  {
    _itin->useInternationalRounding() = false;
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    opt->currencyOverride() = "USD";
    _trx->setOptions(opt);

    FarePath* fp = _memHandle.create<FarePath>();
    fp->setTotalNUCAmount(486.0);
    fp->calculationCurrency() = "GBP";
    fp->baseFareCurrency() = "PLN";

    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      // 486 (GBP) * 4.86 (PLN) * 1 (USD) = 2361.96 (USD)
      CPPUNIT_ASSERT_EQUAL(2361.96, _yq->calculatePercentageFee(1.0, fp));
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(486.0 * 1.55, _yq->calculatePercentageFee(1.0, fp));
    }
  }

  void testYQYRLBFactory()
  {
    fallback::value::yqyrGetYQYRPreCalcLessLock.set(false);
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRLBFactory factory(*_trx, *_itin);
    factory._isSimpleItin = false;

    std::vector<MergedFareMarket*>* vec = _memHandle.create<std::vector<MergedFareMarket*> >();
    FmpMatrixPtr fmpm(new FareMarketPathMatrix(*_trx, *_itin, *vec));
    _itin->fmpMatrix() = fmpm;

    FareMarketPath* fmp = _memHandle.create<FareMarketPath>();
    PaxType* paxType = _memHandle.create<PaxType>();

    YQYRCalculator* calc = factory.getYQYRPreCalc(fmp, paxType);

    CPPUNIT_ASSERT_EQUAL(const_cast<const FareMarketPath*>(fmp), calc->_fareMarketPath);
    CPPUNIT_ASSERT_EQUAL(const_cast<const PaxType*>(paxType), calc->_paxType);

    YQYRCalculator* calc2 = factory.getYQYRPreCalc(fmp, paxType);
    CPPUNIT_ASSERT_EQUAL(calc, calc2);
  }

  void testYQYRLBFactoryOld()
  {
    fallback::value::yqyrGetYQYRPreCalcLessLock.set(true);
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRLBFactory factory(*_trx, *_itin);
    factory._isSimpleItin = false;

    std::vector<MergedFareMarket*>* vec = _memHandle.create<std::vector<MergedFareMarket*>>();
    FmpMatrixPtr fmpm(new FareMarketPathMatrix(*_trx, *_itin, *vec));
    _itin->fmpMatrix() = fmpm;

    FareMarketPath* fmp = _memHandle.create<FareMarketPath>();
    PaxType* paxType = _memHandle.create<PaxType>();

    YQYRCalculator* calc = factory.getYQYRPreCalc(fmp, paxType);
    CPPUNIT_ASSERT_EQUAL(const_cast<const FareMarketPath*>(fmp), calc->_fareMarketPath);
    CPPUNIT_ASSERT_EQUAL(const_cast<const PaxType*>(paxType), calc->_paxType);

    YQYRCalculator* calc2 = factory.getYQYRPreCalc(fmp, paxType);
    CPPUNIT_ASSERT_EQUAL(calc, calc2);
  }

  void testYQYRLBFactorySimpleItin()
  {
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);

    YQYRLBFactory factory(*_trx, *_itin);
    factory._isSimpleItin = true;

    std::vector<MergedFareMarket*>* vec = _memHandle.create<std::vector<MergedFareMarket*> >();
    FmpMatrixPtr fmpm(new FareMarketPathMatrix(*_trx, *_itin, *vec));
    _itin->fmpMatrix() = fmpm;

    FareMarketPath* fmp = _memHandle.create<FareMarketPath>();
    PaxType* paxType = _memHandle.create<PaxType>();

    YQYRCalculator* calc = factory.getYQYRPreCalc(fmp, paxType);

    CPPUNIT_ASSERT_EQUAL(static_cast<const FareMarketPath*>(0), calc->_fareMarketPath);
    CPPUNIT_ASSERT_EQUAL(const_cast<const PaxType*>(paxType), calc->_paxType);
  }

  void testGetFareBreaksFromFP()
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->travelSeg().push_back(_itin->travelSeg()[1]);

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage().push_back(fu);

    FarePath* path = _memHandle.create<FarePath>();
    path->pricingUnit().push_back(pu);

    std::set<TravelSeg*> fareBreaks;
    _yq->getFareBreaks(path, fareBreaks);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), fareBreaks.size());
    CPPUNIT_ASSERT(fareBreaks.find(_itin->travelSeg()[0]) != fareBreaks.end());
    CPPUNIT_ASSERT(fareBreaks.find(_itin->travelSeg()[1]) != fareBreaks.end());
  }

  void testGetFareBreaksFromMFM()
  {
    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    mfm->travelSeg().push_back(_itin->travelSeg()[1]);

    _yq->_allMFMs.push_back(mfm);
    _yq->_fareMarketPath = _memHandle.create<FareMarketPath>();
    std::set<TravelSeg*> fareBreaks;
    _yq->getFareBreaks(0, fareBreaks);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), fareBreaks.size());
    CPPUNIT_ASSERT(fareBreaks.find(_itin->travelSeg()[0]) != fareBreaks.end());
    CPPUNIT_ASSERT(fareBreaks.find(_itin->travelSeg()[1]) != fareBreaks.end());
  }

  void testMileageSameLoc()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();
    originLoc->loc() = "AAA";
    destinationLoc->loc() = "AAA";

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;

    CPPUNIT_ASSERT_EQUAL((uint32_t)0, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                      route.back()->destination(),
                                                                      globals, _itin->travelDate(), *_trx));
  }

  void testMileageSubstitution()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();
    originLoc->loc() = "AXX";
    destinationLoc->loc() = "BXX";

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;

    CPPUNIT_ASSERT_EQUAL((uint32_t)100, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                       route.back()->destination(),
                                                                       globals, _itin->travelDate(), *_trx));
  }

  void testMileageUniqueTPM()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();
    originLoc->loc() = "AAA";
    destinationLoc->loc() = "BBB";

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;

    CPPUNIT_ASSERT_EQUAL((uint32_t)100, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                        route.back()->destination(),
                                                                        globals, _itin->travelDate(), *_trx));
  }

  void testMileageNonUniqueTPM()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();
    originLoc->loc() = "AAA";
    destinationLoc->loc() = "CCC";

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;
    globals.push_back(GlobalDirection::ZZ);
    globals.push_back(GlobalDirection::DO);

    CPPUNIT_ASSERT_EQUAL((uint32_t)300, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                        route.back()->destination(),
                                                                        globals, _itin->travelDate(), *_trx));
  }

  void testMileageUniqueMPM()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();
    originLoc->loc() = "BBB";
    destinationLoc->loc() = "CCC";

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;

    CPPUNIT_ASSERT_EQUAL((uint32_t)100, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                        route.back()->destination(),
                                                                        globals, _itin->travelDate(), *_trx));
  }

  void testMileageNonUniqueMPM()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();
    originLoc->loc() = "BBB";
    destinationLoc->loc() = "DDD";

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;
    globals.push_back(GlobalDirection::ZZ);
    globals.push_back(GlobalDirection::DO);

    CPPUNIT_ASSERT_EQUAL((uint32_t)300, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                        route.back()->destination(),
                                                                        globals, _itin->travelDate(), *_trx));
  }

  void testMileageGCM()
  {
    TravelSeg* originSeg = _memHandle.create<AirSeg>();
    TravelSeg* destinationSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destinationLoc = _memHandle.create<Loc>();

    originLoc->loc() = "ZZZ";
    originLoc->lathem() = 'N';
    originLoc->latdeg() = 70;
    originLoc->lnghem() = 'E';
    originLoc->lngdeg() = 70;

    destinationLoc->loc() = "YYY";
    destinationLoc->lathem() = 'N';
    destinationLoc->latdeg() = 70;
    destinationLoc->lnghem() = 'E';
    destinationLoc->lngdeg() = 75;

    std::vector<TravelSeg*> route;
    originSeg->origin() = originLoc;
    destinationSeg->destination() = destinationLoc;
    route.push_back(originSeg);
    route.push_back(destinationSeg);

    std::vector<GlobalDirection> globals;

    CPPUNIT_ASSERT_EQUAL((uint32_t)118, YQYR::YQYRUtils::journeyMileage(route.front()->origin(),
                                                                        route.back()->destination(),
                                                                        globals, _itin->travelDate(), *_trx));
  }

  void testApplCountNoPortion()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->sectorPortionOfTvlInd() = '?';

    CPPUNIT_ASSERT_EQUAL(1, _yq->applCount(fees, 1, 2));
  }

  void testApplCountNoConnectExemptInd()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->sectorPortionOfTvlInd() = 'P';
    fees->connectExemptInd() = '?';
    fees->feeApplInd() = RuleConst::BLANK;

    CPPUNIT_ASSERT_EQUAL(2, _yq->applCount(fees, 1, 2));
  }

  void testApplCount()
  {
    _itin->travelSeg()[3]->forcedStopOver() = 'T';

    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->sectorPortionOfTvlInd() = 'P';
    fees->connectExemptInd() = 'X';
    fees->feeApplInd() = RuleConst::BLANK;
    fees->stopConnectUnit() = 'D';
    fees->stopConnectTime() = 0;

    CPPUNIT_ASSERT_EQUAL(2, _yq->applCount(fees, 1, 4));
  }

  void testApplCountStopoverTime()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->sectorPortionOfTvlInd() = 'P';
    fees->connectExemptInd() = 'X';
    fees->feeApplInd() = RuleConst::BLANK;

    fees->stopConnectUnit() = 'D';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(1, _yq->applCount(fees, 1, 2));

    fees->stopConnectUnit() = 'H';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(1, _yq->applCount(fees, 1, 2));

    fees->stopConnectUnit() = 'M';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(1, _yq->applCount(fees, 1, 2));

    fees->stopConnectUnit() = 'N';
    fees->stopConnectTime() = 1;
    CPPUNIT_ASSERT_EQUAL(1, _yq->applCount(fees, 1, 2));
  }

  void testApplCountNoBlankFeeApplInd()
  {
    YQYRFees* fees = _memHandle.create<YQYRFees>();
    fees->sectorPortionOfTvlInd() = 'P';
    fees->connectExemptInd() = '?';
    fees->feeApplInd() = '1';

    CPPUNIT_ASSERT_EQUAL(1, _yq->applCount(fees, 1, 2));
  }

  void testAddToValCxrMap()
  {
    CarrierCode carrier1{"AA"};
    CarrierCode carrier2{"BB"};
    YQYRCalculator::ValCxrContext* someContext1 =
        _memHandle.create<YQYRCalculator::ValCxrContext>(carrier1);
    YQYRCalculator::ValCxrContext* someContext2 =
        _memHandle.create<YQYRCalculator::ValCxrContext>(carrier2);
    YQYRCalculator::ValCxrMap firstMap;
    firstMap[carrier1] = someContext1;

    _yq->_valCxrMap.clear();
    _yq->_valCxrMap[carrier2] = someContext2;
    _yq->addToValCxrMap(firstMap);
    auto& valCxrMap = _yq->getValCxrMap();
    auto it = valCxrMap.find(carrier1);
    CPPUNIT_ASSERT(it != valCxrMap.cend());
    CPPUNIT_ASSERT(it->second == someContext1);
    it = valCxrMap.find(carrier2);
    CPPUNIT_ASSERT(it != valCxrMap.cend());
    CPPUNIT_ASSERT(it->second == someContext2);
  }

private:
  YQYRCalculator* getYQYRCalculatorFromFactory()
  {
    YQYRLBFactory factory(*_trx, *_itin);
    factory._isSimpleItin = false;
    std::vector<MergedFareMarket*>* vec = _memHandle.create<std::vector<MergedFareMarket*> >();
    FmpMatrixPtr fmpm(new FareMarketPathMatrix(*_trx, *_itin, *vec));
    _itin->fmpMatrix() = fmpm;
    FareMarketPath* fmp = _memHandle.create<FareMarketPath>();
    PaxType* paxType = _memHandle.create<PaxType>();

    return factory.getYQYRPreCalc(fmp, paxType);
  }

  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

  public:
    const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& key)
    {
      std::vector<const PaxTypeMatrix*>* matrix =
          _memHandle.create<std::vector<const PaxTypeMatrix*> >();
      PaxTypeMatrix* p = _memHandle.create<PaxTypeMatrix>();
      p->atpPaxType() = GDP;
      matrix->push_back(p);
      return *matrix;
    }

    const ZoneInfo*
    getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date)
    {
      ZoneInfo* zoneInfo = _memHandle.create<ZoneInfo>();
      return zoneInfo;
    }

    const TaxCarrierAppl* getTaxCarrierAppl(const VendorCode& vendor, int itemNo)
    {
      if (itemNo == 1 || itemNo == 0)
      {
        return 0;
      }
      else if (itemNo == 10)
      {
        TaxCarrierAppl* appl = _memHandle.create<TaxCarrierAppl>();
        TaxCarrierApplSeg* segment = new TaxCarrierApplSeg;
        segment->applInd() = 'X';
        appl->segs().push_back(segment);
        return appl;
      }
      else
      {
        TaxCarrierAppl* appl = _memHandle.create<TaxCarrierAppl>();
        TaxCarrierApplSeg* segment = new TaxCarrierApplSeg;
        segment->carrier() = DOLLAR_CARRIER;
        appl->segs().push_back(segment);
        return appl;
      }
    }

    const TaxCarrierFlightInfo* getTaxCarrierFlight(const VendorCode& vendor, int itemNo)
    {
      TaxCarrierFlightInfo* info = _memHandle.create<TaxCarrierFlightInfo>();
      return info;
    }

    const std::vector<YQYRFees*>& getYQYRFees(const CarrierCode& carrier, const DateTime& date)
    {
      std::vector<YQYRFees*>* vec = _memHandle.create<std::vector<YQYRFees*> >();
      if (carrier == "AA")
      {
        YQYRFees* fee = _memHandle.create<YQYRFees>();
        fee->taxCode() = "AB";
        fee->carrier() = "AA";
        vec->push_back(fee);
      }

      return *vec;
    }

    const std::vector<YQYRFees*>& getYQYRFees(const CarrierCode& carrier)
    {
      std::vector<YQYRFees*>* vec = _memHandle.create<std::vector<YQYRFees*> >();
      if (carrier == "AA")
      {
        YQYRFees* fee = _memHandle.create<YQYRFees>();
        fee->taxCode() = "AB";
        fee->carrier() = "AA";
        vec->push_back(fee);
      }

      return *vec;
    }

    const std::vector<YQYRFeesNonConcur*>&
    getYQYRFeesNonConcur(const CarrierCode& carrier, const DateTime& date)
    {
      std::vector<YQYRFeesNonConcur*>* vec = _memHandle.create<std::vector<YQYRFeesNonConcur*> >();
      if (carrier == "AA")
      {
        YQYRFeesNonConcur* fee = _memHandle.create<YQYRFeesNonConcur>();
        fee->selfAppl() = 'S';
        vec->push_back(fee);
      }
      else if (carrier == "BB")
      {
        YQYRFeesNonConcur* fee = _memHandle.create<YQYRFeesNonConcur>();
        fee->selfAppl() = 'X';
        fee->taxCarrierApplTblItemNo() = 0;
        fee->vendor() = "ATP";
        vec->push_back(fee);
      }
      else if (carrier == "CC")
      {
        YQYRFeesNonConcur* fee = _memHandle.create<YQYRFeesNonConcur>();
        fee->selfAppl() = 'X';
        fee->taxCarrierApplTblItemNo() = 10;
        fee->vendor() = "ATP";
        vec->push_back(fee);
      }

      return *vec;
    }

    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
    {
      std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
      BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
      bsr->primeCur() = primeCur;
      bsr->cur() = cur;
      bsr->rateType() = 'B';
      bsr->agentSine() = "FXR";
      if (primeCur == "GBP" && cur == "PLN")
      {
        bsr->rate() = 4.86;
        bsr->rateNodec() = 5;
      }
      else if (primeCur == "GBP" && cur == "USD")
      {
        bsr->rate() = 1.55;
        bsr->rateNodec() = 2;
      }
      else
      {
        bsr->rate() = 1;
        bsr->rateNodec() = 1;
      }

      ret->push_back(bsr);
      return *ret;
    }

    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      NUCInfo* nucInfo = _memHandle.create<NUCInfo>();
      return nucInfo;
    }

    const std::vector<Mileage*>& getMileage(const LocCode& origin,
                                            const LocCode& destination,
                                            const DateTime& dateTime,
                                            Indicator mileageType = 'T')
    {
      std::vector<Mileage*>* ret = _memHandle.create<std::vector<Mileage*> >();
      if (origin == "KRK" && destination == "DFW")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 1234;
        ret->push_back(m);
      }
      else if (origin == "KRK" && destination == "LAX")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 2345;
        ret->push_back(m);
      }
      else if (origin == "KRK" && destination == "WAW")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 2645;
        ret->push_back(m);
      }
      else if (origin == "AAA" && destination == "BBB")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 100;
        ret->push_back(m);
      }
      else if (origin == "AAA" && destination == "CCC")
      {
        Mileage* m;
        m = _memHandle.create<Mileage>();
        m->mileage() = 500;
        m->globaldir() = GlobalDirection::XX;
        ret->push_back(m);

        m = _memHandle.create<Mileage>();
        m->mileage() = 200;
        m->globaldir() = GlobalDirection::ZZ;
        ret->push_back(m);

        m = _memHandle.create<Mileage>();
        m->mileage() = 300;
        m->globaldir() = GlobalDirection::DO;
        ret->push_back(m);
      }
      else if (origin == "BBB" && destination == "CCC" && mileageType == MPM)
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 120;
        ret->push_back(m);
      }
      else if (origin == "BBB" && destination == "DDD" && mileageType == MPM)
      {
        Mileage* m;
        m = _memHandle.create<Mileage>();
        m->mileage() = 500;
        m->globaldir() = GlobalDirection::XX;
        ret->push_back(m);

        m = _memHandle.create<Mileage>();
        m->mileage() = 240;
        m->globaldir() = GlobalDirection::ZZ;
        ret->push_back(m);

        m = _memHandle.create<Mileage>();
        m->mileage() = 360;
        m->globaldir() = GlobalDirection::DO;
        ret->push_back(m);
      }
      return *ret;
    }

    const MileageSubstitution* getMileageSubstitution(const LocCode& key, const DateTime& date)
    {
      if (key == "AXX")
      {
        MileageSubstitution* mileageSubstitution = _memHandle.create<MileageSubstitution>();
        mileageSubstitution->publishedLoc() = "AAA";
        return mileageSubstitution;
      }
      else if (key == "BXX")
      {
        MileageSubstitution* mileageSubstitution = _memHandle.create<MileageSubstitution>();
        mileageSubstitution->publishedLoc() = "BBB";
        return mileageSubstitution;
      }
      else
        return 0;
    }
  };

  PoolFactory _poolFactory;
  TestMemHandle _memHandle;
  MyDataHandle* _mdh;
  PricingTrx* _trx;
  PricingRequest* _request;
  Itin* _itin;
  FarePath* _farePath;
  YQYRCalculator* _yq;
};

CPPUNIT_TEST_SUITE_REGISTRATION(YQYRCalculatorTest);

} // tse
