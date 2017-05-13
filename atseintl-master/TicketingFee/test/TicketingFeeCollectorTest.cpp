#include <boost/assign/std/vector.hpp>

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/TicketingFeesInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTicketingFeesInfoFactory.h"
#include "TicketingFee/TicketingFeeCollector.h"

using namespace boost::assign;

namespace tse
{

const std::string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const std::string LOC_DEN = "/vobs/atseintl/test/testdata/data/LocDEN.xml";
const std::string LOC_CHI = "/vobs/atseintl/test/testdata/data/LocCHI.xml";
const std::string LOC_NYC1 = "/vobs/atseintl/test/testdata/data/LocNYC.xml";
const std::string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
const std::string LOC_FRA = "/vobs/atseintl/test/testdata/data/LocFRA.xml";

const std::string
AIR_FRANCE("AF");
const std::string
LUFTHANSA("LH");

const std::string
CLASS_Y("Y");
const std::string
CLASS_M("M");
const std::string
CLASS_F("F");
const std::string
CLASS_C("C");
const std::string
CLASS_YOW("YOW");
const std::string
FARE_BASIS_M("M");
const std::string
FARE_BASIS_F("F");

class TicketingFeeCollectorStub : public TicketingFeeCollector
{
public:
  TicketingFeeCollectorStub(PricingTrx* trx, FarePath* farePath)
    : TicketingFeeCollector(trx, farePath),
      _globalDir(GlobalDirection::XX),
      _isRTJourneyType(false),
      _isIntJourneyType(false),
      _setMlgTurnAround(false),
      _isInLoc(false),
      _validateGeoLoc(false),
      _S1FareBasis(""),
      _isCurrencyNoDecSet(false)
  {
  }

  ~TicketingFeeCollectorStub() {}
  const std::vector<TicketingFeesInfo*>& getTicketingFee() const { return _tktInfo; }

  GlobalDirection getGlobalDirection(std::vector<TravelSeg*>& travelSegs) const
  {
    return _globalDir;
  }

  void getValidSegs() {}

  uint32_t getTPM(const Loc& market1,
                  const Loc& market2,
                  const GlobalDirection& glbDir,
                  const DateTime& tvlDate)
  {
    return 100;
  }
  bool roundTripJouneyType() { return _isRTJourneyType; }
  bool internationalJouneyType() { return _isIntJourneyType; }
  bool setMlgTurnAround()
  {
    if (_setMlgTurnAround)
      return true;
    return TicketingFeeCollector::setMlgTurnAround();
  }
  // simplified version of LocUtil::isInLoc() without hitting DB
  bool isInLoc(const LocKey& locKey, const Loc& loc, const LocCode& zone) const
  {
    switch (locKey.locType())
    {
    case LOCTYPE_AREA:
      return locKey.loc() == loc.area();
    case LOCTYPE_NATION:
      return locKey.loc() == loc.nation();
    default:
      return false;
    }
  }

  // mock method for db retrieval original method
  const std::vector<const PaxTypeMatrix*>& getSabrePaxTypes(const PaxTypeCode& farePathPtc) const
  {
    PaxTypeMatrix* pax1 = _memHandle.create<PaxTypeMatrix>();
    PaxTypeMatrix* pax2 = _memHandle.create<PaxTypeMatrix>();

    pax1->sabrePaxType() = "TC1";
    pax1->atpPaxType() = JCB;

    pax2->sabrePaxType() = "TC1";
    pax2->atpPaxType() = "SPA";

    std::vector<const PaxTypeMatrix*>* paxV = _memHandle.create<std::vector<const PaxTypeMatrix*> >();
    (*paxV) += pax1, pax2;

    return *paxV;
  }
  bool validateGeoLoc1Loc2(const TicketingFeesInfo* feesInfo) const
  {
    if (!_validateGeoLoc)
      return true;
    return TicketingFeeCollector::validateGeoLoc1Loc2(feesInfo);
  }
  bool validateGeoVia(const TicketingFeesInfo* feesInfo) const
  {
    if (!_validateGeoLoc)
      return true;
    return TicketingFeeCollector::validateGeoVia(feesInfo);
  }
  bool validateGeoWhlWithin(const TicketingFeesInfo* feesInfo) const
  {
    if (!_validateGeoLoc)
      return true;
    return TicketingFeeCollector::validateGeoWhlWithin(feesInfo);
  }

  // mock method for createFareBasis original method
  //(for test purposes store farebasis in matchedAccCode member)
  std::string getFareBasis(const PaxTypeFare& paxTypeFare) const { return paxTypeFare.matchedAccCode(); }

  // mock method for matchFareClass original method
  bool matchFareClass(const std::string ptfFareBasis, const std::string tktFareBasis) const
  {
    // return (ptfFareBasis==tktFareBasis);
    return RuleUtil::matchFareClass(ptfFareBasis.c_str(), tktFareBasis.c_str());
  }

  // mock method for getS1FareBasisCode original method
  std::string getS1FareBasisCode(const PaxTypeFare& paxTypeFare, const TravelSeg* lastAirSeg) const
  {
    return _S1FareBasis;
  }

  // mock method for isIndustryFare original method
  bool isIndustryFare(const PaxTypeFare& paxTypeFare) const { return false; }

  // mock method for matchDiffFareBasisTktIndAll original method
  bool matchDiffFareBasisTktIndAll(const FareUsage* fu,
                                   std::string tktFareBasis,
                                   const CarrierCode tktPrimCxr) const
  {
    return true;
  }

  // mock method for matchDiffFareBasisTktIndAny original method
  bool matchDiffFareBasisTktIndAny(const FareUsage* fu,
                                   std::string tktFareBasis,
                                   const CarrierCode tktPrimCxr) const
  {
    return false;
  }
  void setCurrencyNoDec(TicketingFeesInfo& fee) const { _isCurrencyNoDecSet = true; }

  std::vector<TicketingFeesInfo*> _tktInfo;
  GlobalDirection _globalDir;
  bool _isRTJourneyType;
  bool _isIntJourneyType;
  bool _setMlgTurnAround;
  bool _isInLoc;
  bool _validateGeoLoc;
  std::string _S1FareBasis;
  mutable TestMemHandle _memHandle;
  mutable bool _isCurrencyNoDecSet;
};

namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  bool isCxrActivatedForServiceFee(const CarrierCode& validatingCarrier, const DateTime& date)
  {
    if (validatingCarrier == "")
      return false;
    else if (validatingCarrier == "AA")
      return date > DateTime(2009, 4, 22);
    else if (validatingCarrier == "CC")
      return false;
    return DataHandleMock::isCxrActivatedForServiceFee(validatingCarrier, date);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "NYC")
      return "NYC";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (origin == "NYC" && dest == "DEN" && globalDir != GlobalDirection::WH)
      return 0;
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};
}

class TicketingFeeCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketingFeeCollectorTest);
  CPPUNIT_TEST(testConstructor);

  CPPUNIT_SKIP_TEST(testServiceFeeActiveForCarrierReturnsTrue);
  CPPUNIT_TEST(testServiceFeeActiveForCarrierReturnsFalse);
  CPPUNIT_TEST(testServiceFeeActiveForCarrierEmptyValidatingCXRReturnsFalse);
  CPPUNIT_TEST(testServiceFeeActiveForCarrierTravelDateBeforeActivationReturnsFalse);

  CPPUNIT_TEST(testIsValidTrxINTERNAL_ERRORWhenTrxZero);
  CPPUNIT_TEST(testIsValidTrxINTERNAL_ERRORWhenFarePathZero);
  CPPUNIT_TEST(testIsValidTrxTJR_NOT_APPLY_TKTFEEWhenDoNotCollectTicketingFeeEqualYES);
  CPPUNIT_TEST(testIsValidTrxVALIDATING_CXR_EMPTYWhenValidatingCarrierEmpty);
  CPPUNIT_TEST(testIsValidTrxALL_SEGS_OPENWhenAllSegsOpen);
  CPPUNIT_TEST(testIsValidTrxNOT_ALL_SEGS_CONFIRMWhenNoSegsConfirm);
  CPPUNIT_TEST(testIsValidTrxPASS_S4WhenTjrZero);
  CPPUNIT_TEST(testIsValidTrxPASS_S4);
  CPPUNIT_TEST(testIsValidTrxNON_CREDIT_CARD_FOPWhenNonCreditCardFOP);

  CPPUNIT_TEST(testValidatingCarrierReturnsFromTrxWhenNotEmpty);
  CPPUNIT_TEST(testValidatingCarrierReturnsFromItinWhenEmptyInTrx);
  CPPUNIT_TEST(testCheckTicketingDatesReturnTrueWhenTicketingDateEqualEffDate);
  CPPUNIT_TEST(testCheckTicketingDatesReturnTrueWhenTicketingDateBetweenEffAndDisc);
  CPPUNIT_TEST(testCheckTicketingDatesReturnTrueWhenTicketingDateEqualDiscDate);
  CPPUNIT_TEST(testCheckTicketingDatesReturnTrueWhenTicketingDateBeforeEffDateOnlyTime);
  CPPUNIT_TEST(testCheckTicketingDatesReturnTrueWhenTicketingDateAfterDiscDateOnlyTime);
  CPPUNIT_TEST(testCheckTicketingDatesReturnFalseWhenTicketingDateBeforeEffDate);
  CPPUNIT_TEST(testCheckTicketingDatesReturnFalseWhenTicketingDateAfterDiscDate);
  CPPUNIT_TEST(testCheckServiceTypeReturnTrueWhenServiceOBFCA);
  CPPUNIT_TEST(testCheckServiceSubTypeReturnTrueWhenServiceOBR01);
  CPPUNIT_TEST(testCheckServiceSubTypeReturnTrueWhenServiceOBT01);
  CPPUNIT_TEST(testCheckServiceSubTypeReturnFalseWhenServiceNotFCA);
  CPPUNIT_TEST(testCheckPaxTypeReturnTrueWhenSabrePtcMatchFeePtc);
  CPPUNIT_TEST(testCheckPaxTypeReturnFalseWhenSabrePtcNoMatchFeePtc);
  CPPUNIT_TEST(testCheckPaxTypeReturnTrueWhenFeePtcEmpty);
  CPPUNIT_TEST(testCheckPaxTypeReturnFalseWhenInputPtcNoMatchFeePtc);
  CPPUNIT_TEST(testCheckPaxTypeReturnTrueWhenInputPtcExactMatchFeePtc);
  CPPUNIT_TEST(testCheckPaxTypeReturnFalseWhenInputPtcNotMapToFeePtcYTH);
  CPPUNIT_TEST(testCheckPaxTypeReturnFalseWhenInputPtcNotMapToFeePtcINS1);
  CPPUNIT_TEST(testCheckFareBasisReturnTrueWhenAllFareBasisAndCxrMatchAInd);
  CPPUNIT_TEST(testCheckFareBasisReturnFalseWhenAllFareBasisMatchAndCxrNotMatchAInd);
  CPPUNIT_TEST(testCheckFareBasisReturnFalseWhenAllFareBasisNotMatchAndCxrMatchAInd);
  CPPUNIT_TEST(testCheckFareBasisReturnTrueWhenSomeFareBasisNotMatchAndCxrMatchSInd);
  CPPUNIT_TEST(testCheckFareBasisReturnTrueWhenFareBasisMatchAndSomeCxrNotMatchSInd);
  CPPUNIT_TEST(testCheckFareBasisReturnFalseWhenAllMatchFInd);
  CPPUNIT_TEST(testCheckFareBasisCheckFrequentFlyer);
  CPPUNIT_TEST(testMatchFareBasisWildcards1);
  CPPUNIT_TEST(testMatchFareBasisWildcards2);
  CPPUNIT_TEST(testMatchFareBasisWildcards3);
  CPPUNIT_TEST(testMatchFareBasisWildcards4);
  CPPUNIT_TEST(testMatchFareBasisWildcards5);
  CPPUNIT_TEST(testMatchFareBasisWildcards6);
  CPPUNIT_TEST(testMatchFareBasisWildcards7);
  CPPUNIT_TEST(testMatchFareBasisWildcards8);
  CPPUNIT_TEST(testMatchFareBasisWildcards9);
  CPPUNIT_TEST(testMatchFareBasisWildcards10);
  CPPUNIT_TEST(testMatchFareBasisWildcards11);
  CPPUNIT_TEST(testMatchFareBasisWildcards12);
  CPPUNIT_TEST(testMatchFareBasisWildcardsGroup);

  CPPUNIT_TEST(testCheckSecurityReturnTrueWhenT183Zero);
  CPPUNIT_TEST(testCheckSecurityReturnFalseWhenT183ZeroAndPrivateIndicatorON);

  CPPUNIT_TEST(testValidateSequenceReturnFAIL_TKT_DATEwhenTktDateFails);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_SVC_TYPEwhenServiceTypeFails);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_PAX_TYPEwhenPaxTypeFails);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_SECUR_T183whenSecurityFails);
  CPPUNIT_TEST(testValidateSequenceReturnPASS_S4whenSequencePass);

  CPPUNIT_TEST(testCreateDiagnosticNoDiagnosticRequested);
  CPPUNIT_TEST(testCreateDiagnosticWhenDiagnosticRequested);
  CPPUNIT_TEST(testPrintDiagHeaderWhenNoDiagnosticRequested);
  CPPUNIT_TEST(testPrintDiagHeaderWhenDiagnosticRequested);
  CPPUNIT_TEST(testIsDiagSequenceMatchtExpectedMatchWhenSequenceRequested);
  CPPUNIT_TEST(testIsDiagSequenceMatchtExpectedNoMatchWhenSequenceRequested);
  CPPUNIT_TEST(testIsDiagSequenceMatchtExpectedMatchWhenServiceCodeRequested);
  CPPUNIT_TEST(testIsDiagSequenceMatchtExpectedNoMatchWhenServiceCodeRequested);
  CPPUNIT_TEST(testCheckDiagDetailedRequestExpectedNoMatchWhenDDINFOisNotRequested);
  CPPUNIT_TEST(testCheckDiagDetailedRequestExpectedMatchWhenDDINFOisRequested);

  CPPUNIT_TEST(testJouneyTypeWhenNationSameReturnFalse);
  CPPUNIT_TEST(testJouneyTypeWhenNationDifferentReturnTrue);
  CPPUNIT_TEST(testJouneyTypeWithArunkWhenNationDifferentReturnTrue);
  CPPUNIT_TEST(testRoundTripJouneyTypeWhenNationSameReturnFalse);
  CPPUNIT_TEST(testRoundTripJouneyTypeWhenNationDifferentReturnTrue);
  CPPUNIT_TEST(testRoundTripJouneyTypeWithArunkWhenNationDifferentReturnTrue);

  CPPUNIT_TEST(testCollectZeroWhenNoTicketingInfoRetrieved);
  CPPUNIT_TEST(testCollectZeroWhenNoValidTicketingInfo);
  CPPUNIT_TEST(testCollectTwoWhenTwoValidTicketingInfo);
  CPPUNIT_TEST(testCollectTwoWhenOneofTwoValidTicketingInfo);
  CPPUNIT_TEST(testCollectOneWhenTwoValidTicketingInfoButFirstHasFopBinNumberBlank);

  CPPUNIT_TEST(testGetValidSegsReturnFareBreaks);
  CPPUNIT_TEST(testGetValidSegsReturnEmptyVector);
  CPPUNIT_TEST(testGetValidSegsReturnFareBreaksAndStopovers);
  CPPUNIT_TEST(testGetValidSegsSkipEmbeddedSideTrip);
  CPPUNIT_TEST(testGetValidSegsStopoverIsFareBreak);
  CPPUNIT_TEST(testGetValidSegsStopoverOnSideTrip);
  CPPUNIT_TEST(testSetMlgTurnAroundNotEmptyMapForGlobalDirectionXX);
  CPPUNIT_TEST(testSetMlgTurnAroundPass);

  CPPUNIT_TEST(testValidateGeoLoc1Loc2PassWhenJourneyIndIsALoc1MatchOriginLoc2IsNull);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2PassWhenJourneyIndIsALoc1MatchOriginLoc2MatchDestination);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1MatchOriginLoc2NoMatchDestination);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginLoc1NoMatchDestination);
  CPPUNIT_TEST(
      testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginAndDestinationLoc2NoMatchOrigniNorDestination);
  CPPUNIT_TEST(
      testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginAndDestinationLoc2NoMatchOrigniMatchDestination);
  CPPUNIT_TEST(
      testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginAndDestinationLoc2MatchOrigniNoMatchDestination);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1IsNull);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1IsNotNullLoc2IsNull);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1MatchOriginLoc2MatchDestination);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1MatchDestinationLoc2MatchOrigin);
  CPPUNIT_TEST(testValidateGeoLoc1Loc2FailWhenJourneyIndIsBlankLoc1IsNotNullLoc2IsNotNull);
  CPPUNIT_TEST(testValidateGeoViaFailWhenViaNotFound);
  CPPUNIT_TEST(testValidateGeoViaPassWhenViaLocFound);
  CPPUNIT_TEST(testValidateGeoViaFailWhenOriginAndDestinationMatchVia);
  CPPUNIT_TEST(testValidateGeoViaFailWhenTurnAroundPointMatchVia);
  CPPUNIT_TEST(testValidateGeoViaPassWhenTurnAroundPointMatchViaAndThereIsDifferentViaPoint);
  CPPUNIT_TEST(testValidateGeoViaPassWhenTurnAroundPointMatchViaNation);
  CPPUNIT_TEST(testValidateGeoViaFailWhenTurnAroundPointMatchViaAndLastPoint);
  CPPUNIT_TEST(testValidateGeoWhlWithinPassArea1);
  CPPUNIT_TEST(testValidateGeoWhlWithinPassArea2);
  CPPUNIT_TEST(testValidateGeoWhlWithinPassCountryUS);

  CPPUNIT_TEST(testCheckAccountCodeReturnTrueWhenT172Zero);
  CPPUNIT_TEST(testValidateGeoWhlWithinPassWhenBlank);
  CPPUNIT_TEST(testValidateGeoViaPassWhenBlank);

  CPPUNIT_TEST(testCheckTktDesignatorWithNoItemNumber);

  CPPUNIT_TEST(testCompareFopBinsTrueWhenFopEqual);
  CPPUNIT_TEST(testCompareFopBinsTrueWhenFopNotEqualButMatch);
  CPPUNIT_TEST(testCompareFopBinsFalseFopNotEqualAndNotCover);

  CPPUNIT_TEST(testCheckFopBinNumberSetStopProcessingWhenFopBinNumberEmpty);
  CPPUNIT_TEST(testCheckFopBinNumberNotSetStopProcessingWhenFopBinNumberNotEmpty);
  CPPUNIT_TEST(testCheckDuplicatedRecordsPassWhenExist);
  CPPUNIT_TEST(testCheckDuplicatedRecordsFailWhenEmptyVector);
  CPPUNIT_TEST(testCheckDuplicatedRecordsFailWhenNoDuplicateFound);
  CPPUNIT_TEST(testCheckDuplicatedRecordsPassWhenMultipleRecordsInVectorFirstMatch);
  CPPUNIT_TEST(testCheckDuplicatedRecordsFailWhenMultipleRecordsInVectorNoMatch);
  CPPUNIT_TEST(testCheckDuplicatedRecordsPassWhenDifferentSubtypes);

  CPPUNIT_TEST(testSetJourneyDestinationWhenJourneyIsOneWayInternational);
  CPPUNIT_TEST(testSetJourneyDestinationWhenJourneyIsOneWayDomestic);
  CPPUNIT_TEST(testSetJourneyDestinationWhenJourneyIsRoundTripDomestic);
  CPPUNIT_TEST(testSetJourneyDestinationJourneyIsRoundTripReturnWhenBadData);
  CPPUNIT_TEST(
      testSetJourneyDestinationWhenJourneyIsRoundTripInternationalTurnAroundPointIsInDifferentCountryThanJourneyOrigin);
  CPPUNIT_TEST(
      testSetJourneyDestinationWhenJourneyIsRoundTripInternationalTurnAroundPointIsInTheSameCountryAsJourneyOrigin);
  CPPUNIT_TEST(testGetS1FareBasisCodeReturnEmptyStringForArunk);

  CPPUNIT_TEST(testIsStopOverPointPassWhenIsStopOver);
  CPPUNIT_TEST(testIsStopOverPointPassWhenTravelSegIsInStooversVector);
  CPPUNIT_TEST(testIsStopOverPointFail);
  CPPUNIT_TEST(testIsStopOverPassWhenStopOverPoint);
  CPPUNIT_TEST(testIsStopOverPassWhenIsNotStopOverPoint);
  CPPUNIT_TEST(testIsStopOverFailWhenIsNotStopOverPoint);
  CPPUNIT_TEST(testCollectCallSetCurrencyNoDec);
  CPPUNIT_TEST(testisArunkPartOfSideTrip);

  CPPUNIT_TEST(testIsProcessFOPFailWhenNoFop);
  CPPUNIT_TEST(testIsProcessFOPFailWhenSingleFopIsNot6Digits);
  CPPUNIT_TEST(testIsProcessFOPFailWhenSingleFopIs6Mixed);
  CPPUNIT_TEST(testIsProcessFOPPassWhenSingleFopIs6Digits);
  CPPUNIT_TEST(testIsFopBinMatchFailWhenNoMatchSingleFop);
  CPPUNIT_TEST(testIsFopBinMatchTrueWhenMatchSingleFop);

  CPPUNIT_TEST(testIsAnyNonCreditCardFOPFalseWhenCreditCard);
  CPPUNIT_TEST(testIsAnyNonCreditCardFOPTrueWhenNonCreditCardAndCash);

  CPPUNIT_TEST(testCheckDuplicatedRTypeRecordsPassWhenExist);
  CPPUNIT_TEST(testCheckDuplicatedRTypeRecordsFalseWhenDoNotExist);
  CPPUNIT_TEST(testCheckDuplicatedTTypeRecordsPassWhenExist);
  CPPUNIT_TEST(testCheckDuplicatedTTypeRecordsFalseWhenDoNotExist);

  CPPUNIT_TEST(testCheckSortRTypeOBFees);

  CPPUNIT_TEST(testInitializeFor2Cc);
  CPPUNIT_TEST(testInitializeFor2CcReturnIfNot2CC);
  CPPUNIT_TEST(testIsSvcTypeMatchCardTypePassCredit);
  CPPUNIT_TEST(testIsSvcTypeMatchCardTypePassDebit);
  CPPUNIT_TEST(testIsSvcTypeMatchCardTypeFailCredit);
  CPPUNIT_TEST(testIsSvcTypeMatchCardTypeFailDebit);
  CPPUNIT_TEST(testAddFopBinIfMatchAddOneForEmpty);
  CPPUNIT_TEST(testAddFopBinIfMatchAddTwoForEmpty);
  CPPUNIT_TEST(testAddFopBinIfMatchAddTwoForEmptyFda);
  CPPUNIT_TEST(testAddFopBinIfMatchFail);
  CPPUNIT_TEST(testAdjustAnyCreditCardTypeNoChangeForDebit);
  CPPUNIT_TEST(testAdjustAnyCreditCardTypeNoChangeForCredit);
  CPPUNIT_TEST(testAdjustAnyCreditCardTypeChangeForG);
  CPPUNIT_TEST(testAdjustAnyCreditCardTypeChangeForBlank);
  CPPUNIT_TEST(testSwapObFeesVecFro2CC);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _request->collectOBFee() = 'Y';
    _request->validatingCarrier() = "AA";
    const DateTime tktDate = DateTime(2009, 3, 23, 8, 15, 0);
    _request->ticketingDT() = tktDate;
    _trx->setRequest(_request);
    _customer.doNotApplyObTktFees() = NO;
    _agent.agentTJR() = &_customer;
    _request->ticketingAgent() = &_agent;

    _farePath = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;
    _paxType = _memHandle.create<PaxType>();
    _farePath->paxType() = _paxType;

    _memHandle.insert(_tfcs = new TicketingFeeCollectorStub(_trx, _farePath));
    _memHandle.insert(_tfc = new TicketingFeeCollector(_trx, _farePath));

    _feeInfo = _memHandle.create<TicketingFeesInfo>();
    const DateTime effDate = DateTime(2009, 3, 10, 11, 59, 0);
    _feeInfo->ticketEffDate() = effDate;
    const DateTime disDate = DateTime(2009, 3, 12, 11, 59, 0);
    _feeInfo->ticketDiscDate() = disDate;
    _feeInfo->journeyInd() = RuleConst::BLANK;
    _feeInfo->locVia().locType() = LOCTYPE_AREA;
    _feeInfo->locVia().loc() = IATA_AREA2;
    _feeInfo->locWhlWithin().locType() = LOCTYPE_AREA;
    _feeInfo->locWhlWithin().loc() = IATA_AREA2;

    _airSeg1.resStatus() = CONFIRM_RES_STATUS;

    _airSeg1.resStatus() = CONFIRM_RES_STATUS;
    _airSeg1.origin() = TestLocFactory::create(LOC_DFW);
    _airSeg1.destination() = TestLocFactory::create(LOC_DEN);
    _airSeg2.resStatus() = CONFIRM_RES_STATUS;
    _airSeg2.origin() = TestLocFactory::create(LOC_CHI);
    _airSeg2.destination() = TestLocFactory::create(LOC_NYC1);

    TestConfigInitializer::setValue("ATPCO_BIN_ANSWER_TABLE_ACTIVATION_DATE", "2009-06-01", "FARE_CALC_SVC", true);
    TestConfigInitializer::setValue(
        "FF_TIER_OB_ACTIVATION_DATE", DateTime(2009, 1, 1), "PRICING_SVC");
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    CPPUNIT_ASSERT_EQUAL(_trx, _tfc->_trx);
    CPPUNIT_ASSERT_EQUAL(_farePath, _tfc->_farePath);
  }

  void testServiceFeeActiveForCarrierReturnsTrue()
  {
    _trx->ticketingDate() = DateTime::localTime();
    CPPUNIT_ASSERT(_tfc->serviceFeeActiveForCarrier());
  }

  void testServiceFeeActiveForCarrierReturnsFalse()
  {
    _trx->ticketingDate() = DateTime::localTime();
    _request->validatingCarrier() = "CC";
    CPPUNIT_ASSERT(!_tfc->serviceFeeActiveForCarrier());
  }

  void testServiceFeeActiveForCarrierEmptyValidatingCXRReturnsFalse()
  {
    _trx->ticketingDate() = DateTime::localTime();
    _request->validatingCarrier().clear();
    _itin->validatingCarrier().clear();
    CPPUNIT_ASSERT(!_tfc->serviceFeeActiveForCarrier());
  }

  void testServiceFeeActiveForCarrierTravelDateBeforeActivationReturnsFalse()
  {
    const DateTime travelDate = DateTime(2009, 3, 23, 8, 15, 0);
    _trx->ticketingDate() = travelDate;
    CPPUNIT_ASSERT(!_tfc->serviceFeeActiveForCarrier());
  }

  void testIsValidTrxINTERNAL_ERRORWhenTrxZero()
  {
    PricingTrx* savedTrxPointer = _tfc->_trx;
    _tfc->_trx = 0;
    CPPUNIT_ASSERT_EQUAL(INTERNAL_ERROR, _tfc->isValidTrx());
    _tfc->_trx = savedTrxPointer;
  }

  void testIsValidTrxINTERNAL_ERRORWhenFarePathZero()
  {
    FarePath* savedFarePathPointer = _tfc->_farePath;
    _tfc->_farePath = 0;
    CPPUNIT_ASSERT_EQUAL(INTERNAL_ERROR, _tfc->isValidTrx());
    _tfc->_farePath = savedFarePathPointer;
  }

  void testIsValidTrxTJR_NOT_APPLY_TKTFEEWhenDoNotCollectTicketingFeeEqualYES()
  {
    _customer.doNotApplyObTktFees() = YES;
    CPPUNIT_ASSERT_EQUAL(TJR_NOT_APPLY_TKTFEE, _tfc->isValidTrx());
  }

  void testIsValidTrxVALIDATING_CXR_EMPTYWhenValidatingCarrierEmpty()
  {
    _trx->getRequest()->validatingCarrier().clear();
    _itin->validatingCarrier().clear();
    CPPUNIT_ASSERT_EQUAL(VALIDATING_CXR_EMPTY, _tfc->isValidTrx());
  }

  void testIsValidTrxALL_SEGS_OPENWhenAllSegsOpen()
  {
    _airSeg1.segmentType() = Open;
    _airSeg2.segmentType() = Open;
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    CPPUNIT_ASSERT_EQUAL(ALL_SEGS_OPEN, _tfc->isValidTrx());
  }

  void testIsValidTrxNOT_ALL_SEGS_CONFIRMWhenNoSegsConfirm()
  {
    _airSeg1.resStatus() = QF_RES_STATUS;
    _airSeg2.resStatus() = QF_RES_STATUS;
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    CPPUNIT_ASSERT_EQUAL(NOT_ALL_SEGS_CONFIRM, _tfc->isValidTrx());
  }

  void testIsValidTrxPASS_S4WhenTjrZero()
  {
    _agent.agentTJR() = 0;
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    CPPUNIT_ASSERT_EQUAL(PASS_S4, _tfc->isValidTrx());
  }

  void testIsValidTrxPASS_S4()
  {
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    CPPUNIT_ASSERT_EQUAL(PASS_S4, _tfc->isValidTrx());
  }

  void testIsValidTrxNON_CREDIT_CARD_FOPWhenNonCreditCardFOP()
  {
    _trx->getRequest()->formOfPaymentCard() = 'F';
    _trx->getRequest()->formOfPaymentCash() = 'T';
    _itin->travelSeg() += &_airSeg1, &_airSeg2;

    CPPUNIT_ASSERT(_trx->getRequest()->isFormOfPaymentCash());
    CPPUNIT_ASSERT(!_trx->getRequest()->isFormOfPaymentCard());
    CPPUNIT_ASSERT(_tfc->isAnyNonCreditCardFOP());
    CPPUNIT_ASSERT_EQUAL(NON_CREDIT_CARD_FOP, _tfc->isValidTrx());
  }

  void testValidatingCarrierReturnsFromTrxWhenNotEmpty()
  {
    CPPUNIT_ASSERT_EQUAL(_trx->getRequest()->validatingCarrier(), _tfc->validatingCarrier());
  }

  void testValidatingCarrierReturnsFromItinWhenEmptyInTrx()
  {
    _trx->getRequest()->validatingCarrier().clear();
    _itin->validatingCarrier() = "CO";
    CPPUNIT_ASSERT_EQUAL(_itin->validatingCarrier(), _tfc->validatingCarrier());
  }

  void testCheckTicketingDatesReturnTrueWhenTicketingDateEqualEffDate()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate();
    CPPUNIT_ASSERT(_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckTicketingDatesReturnTrueWhenTicketingDateBetweenEffAndDisc()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().addDays(1);
    CPPUNIT_ASSERT(_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckTicketingDatesReturnTrueWhenTicketingDateEqualDiscDate()
  {
    _request->ticketingDT() = _feeInfo->ticketDiscDate();
    CPPUNIT_ASSERT(_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckTicketingDatesReturnTrueWhenTicketingDateBeforeEffDateOnlyTime()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().subtractSeconds(1);
    CPPUNIT_ASSERT(_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckTicketingDatesReturnTrueWhenTicketingDateAfterDiscDateOnlyTime()
  {
    _request->ticketingDT() = _feeInfo->ticketDiscDate().addSeconds(1);
    CPPUNIT_ASSERT(_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckTicketingDatesReturnFalseWhenTicketingDateBeforeEffDate()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().subtractDays(1);
    CPPUNIT_ASSERT(!_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckTicketingDatesReturnFalseWhenTicketingDateAfterDiscDate()
  {
    _request->ticketingDT() = _feeInfo->ticketDiscDate().addDays(1);
    CPPUNIT_ASSERT(!_tfc->checkTicketingDates(_feeInfo));
  }

  void testCheckServiceTypeReturnTrueWhenServiceOBFCA()
  {
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = ANY_CREDIT;
    CPPUNIT_ASSERT(_tfc->checkServiceType(_feeInfo));
  }

  void testCheckServiceSubTypeReturnTrueWhenServiceOBR01()
  {
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = "R01";
    _trx->getRequest()->setCollectRTypeOBFee(true);
    CPPUNIT_ASSERT(_tfc->checkServiceSubType(_feeInfo));
  }

  void testCheckServiceSubTypeReturnTrueWhenServiceOBT01()
  {
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = "T01";
    _trx->getRequest()->setCollectTTypeOBFee(true);
    CPPUNIT_ASSERT(_tfc->checkServiceSubType(_feeInfo));
  }

  void testCheckServiceSubTypeReturnFalseWhenServiceNotFCA()
  {
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = "FCI";
    CPPUNIT_ASSERT(!_tfc->checkServiceSubType(_feeInfo));
  }

  void testCheckPaxTypeReturnTrueWhenSabrePtcMatchFeePtc()
  {
    _farePath->paxType()->paxType() = "TC1";
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = JCB;
    CPPUNIT_ASSERT(_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckPaxTypeReturnFalseWhenSabrePtcNoMatchFeePtc()
  {
    _farePath->paxType()->paxType() = "TC1";
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = SRC;
    CPPUNIT_ASSERT(!_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckPaxTypeReturnTrueWhenFeePtcEmpty()
  {
    _farePath->paxType()->paxType() = CHILD;
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = "";
    CPPUNIT_ASSERT(_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckPaxTypeReturnFalseWhenInputPtcNoMatchFeePtc()
  {
    _farePath->paxType()->paxType() = ADULT;
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = "ITF";
    CPPUNIT_ASSERT(!_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckPaxTypeReturnTrueWhenInputPtcExactMatchFeePtc()
  {
    _farePath->paxType()->paxType() = INE;
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = INE;
    CPPUNIT_ASSERT(_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckPaxTypeReturnFalseWhenInputPtcNotMapToFeePtcYTH()
  {
    _farePath->paxType()->paxType() = ADULT;
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = "YTH";
    CPPUNIT_ASSERT(!_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckPaxTypeReturnFalseWhenInputPtcNotMapToFeePtcINS1()
  {
    _farePath->paxType()->paxType() = INS;
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = INFANT;
    CPPUNIT_ASSERT(!_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisReturnTrueWhenAllFareBasisAndCxrMatchAInd()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    _farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _farePath->pricingUnit()[0]->fareUsage()[2]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _tfcs->_tktInfo[0]->fareInd() = 'A';
    _tfcs->_tktInfo[0]->fareBasis() = FARE_BASIS_M;
    _tfcs->_tktInfo[0]->primaryFareCarrier() = AIR_FRANCE;
    _tfcs->_S1FareBasis = FARE_BASIS_M;

    CPPUNIT_ASSERT(_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisReturnFalseWhenAllFareBasisMatchAndCxrNotMatchAInd()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    _farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(LUFTHANSA, CLASS_M, FARE_BASIS_M);
    _farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _tfcs->_tktInfo[0]->fareInd() = 'A';
    _tfcs->_tktInfo[0]->fareBasis() = FARE_BASIS_M;
    _tfcs->_tktInfo[0]->primaryFareCarrier() = AIR_FRANCE;
    _tfcs->_S1FareBasis = FARE_BASIS_M;

    CPPUNIT_ASSERT(!_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisReturnFalseWhenAllFareBasisNotMatchAndCxrMatchAInd()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    _farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_F, FARE_BASIS_F);
    _farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _tfcs->_tktInfo[0]->fareInd() = 'A';
    _tfcs->_tktInfo[0]->fareBasis() = FARE_BASIS_M;
    _tfcs->_tktInfo[0]->primaryFareCarrier() = AIR_FRANCE;
    _tfcs->_S1FareBasis = FARE_BASIS_F;

    CPPUNIT_ASSERT(!_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisReturnTrueWhenSomeFareBasisNotMatchAndCxrMatchSInd()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    _farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_F, FARE_BASIS_F);
    _farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _tfcs->_tktInfo[0]->fareInd() = 'S';
    _tfcs->_tktInfo[0]->fareBasis() = FARE_BASIS_M;
    _tfcs->_tktInfo[0]->primaryFareCarrier() = AIR_FRANCE;
    _tfcs->_S1FareBasis = FARE_BASIS_M;

    CPPUNIT_ASSERT(_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisReturnTrueWhenFareBasisMatchAndSomeCxrNotMatchSInd()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    _farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(LUFTHANSA, CLASS_M, FARE_BASIS_M);
    _farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _tfcs->_tktInfo[0]->fareInd() = 'S';
    _tfcs->_tktInfo[0]->fareBasis() = FARE_BASIS_M;
    _tfcs->_tktInfo[0]->primaryFareCarrier() = AIR_FRANCE;
    _tfcs->_S1FareBasis = FARE_BASIS_M;

    CPPUNIT_ASSERT(_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisReturnFalseWhenAllMatchFInd()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    _farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare() =
        createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_M, FARE_BASIS_M);
    _tfcs->_tktInfo[0]->fareInd() = TKT_FREQ_FLYER;
    _tfcs->_tktInfo[0]->fareBasis() = FARE_BASIS_M;
    _tfcs->_tktInfo[0]->primaryFareCarrier() = AIR_FRANCE;
    _tfcs->_tktInfo[0]->serviceSubTypeCode() = "";
    _tfcs->_S1FareBasis = FARE_BASIS_M;

    CPPUNIT_ASSERT(!_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testCheckFareBasisCheckFrequentFlyer()
  {
    createValidTicketingInfoForPaxTypeCheck();
    createBasicPricingUnitAndItin();
    PaxType::FreqFlyerTierWithCarrier* data =
        _memHandle.create<PaxType::FreqFlyerTierWithCarrier>();
    CarrierCode carrier = CarrierCode("LH");
    data->setCxr(carrier);
    data->setFreqFlyerTierLevel(1);
    _paxType->freqFlyerTierWithCarrier().push_back(data);
    _tfcs->_tktInfo[0]->fareInd() = TKT_FREQ_FLYER;
    _tfcs->_tktInfo[0]->fareBasis() = "5";
    _tfcs->_tktInfo[0]->serviceSubTypeCode() = "FCA";
    _request->validatingCarrier() = carrier;
    CPPUNIT_ASSERT(_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));

    _tfcs->_frequentFlyerTierLevel.reset();
    _paxType->freqFlyerTierWithCarrier().front()->setFreqFlyerTierLevel(6);
    CPPUNIT_ASSERT(!_tfcs->checkFareBasis(_tfcs->_tktInfo[0]));
  }

  void testMatchFareBasisWildcards1()
  {
    std::string matchedAccCode("YOW");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_YOW, matchedAccCode);
    std::string tktFareBasis = "-/*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards2()
  {
    std::string matchedAccCode("YOW/UAC");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_YOW, matchedAccCode);
    const_cast<TktDesignator&>(ptf->fcasTktDesignator()) = "UAC";
    std::string tktFareBasis = "-/*";
    _tfcs->_S1FareBasis = "YOW/UAC";
    std::vector<TravelSeg*> ts;
    ts.push_back(_memHandle.create<AirSeg>());
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards3()
  {
    std::string matchedAccCode("Y");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "Y//*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards4()
  {
    std::string matchedAccCode("Y/CH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "Y//*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards5()
  {
    std::string matchedAccCode("Y/ID/CH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "Y//*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards6()
  {
    std::string matchedAccCode("Y");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "Y/*/*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards7()
  {
    std::string matchedAccCode("Y/CH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_C, matchedAccCode);
    std::string tktFareBasis = "Y/*/*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards8()
  {
    std::string matchedAccCode("Y//CH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "Y/*/*";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards9()
  {
    std::string matchedAccCode("QEE");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "-/CH";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards10()
  {
    std::string matchedAccCode("QEE//CH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "-/CH";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards11()
  {
    std::string matchedAccCode("QEE/ABC/CH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "-/CH";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcards12()
  {
    std::string matchedAccCode("QEECH");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_Y, matchedAccCode);
    std::string tktFareBasis = "-/CH";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testMatchFareBasisWildcardsGroup()
  {
    std::string matchedAccCode("");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_YOW, matchedAccCode);

    std::string tktFareBasis = "B-E70";
    _tfcs->_S1FareBasis = "BE701";
    std::vector<TravelSeg*> ts;
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BE710";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BE170";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "BE-70";
    _tfcs->_S1FareBasis = "BEX70";
    ts.push_back(_memHandle.create<AirSeg>());
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BE1X70";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BEX170";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "B-E70/CH";
    _tfcs->_S1FareBasis = "BE701/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BE710/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BE170/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-/*";
    _tfcs->_S1FareBasis = "J77OW/TEST";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-E70";
    _tfcs->_S1FareBasis = "BE70";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YNWE70";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "Q123E70NR";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BE70/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BAP/E70";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "BAP/E70/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "Q-CH";
    _tfcs->_S1FareBasis = "QAPCH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "Q/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAPCH/ID90";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAP/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "Q-";
    _tfcs->_S1FareBasis = "QAP7";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAP7/ID90";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "Q/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAP/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-CH";
    _tfcs->_S1FareBasis = "QEECH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAPCH/ABC";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QEE/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-E70/CH";
    _tfcs->_S1FareBasis = "BE70/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "Q123E70/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/E70/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/BE70/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-/CH";
    _tfcs->_S1FareBasis = "QEE7M/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QEE";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QEE//CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QEE/ABC/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QEECH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-AP/CH";
    _tfcs->_S1FareBasis = "QAPNR/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAPNR/CH33";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-/*";
    _tfcs->_S1FareBasis = "YAP/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP//CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "-AP/CH*";
    _tfcs->_S1FareBasis = "QAPNR/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAPNR/CH33";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAP/CH/ABC";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAP/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "QAP/ID/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "Y-//CH*";
    _tfcs->_S1FareBasis = "YAP//CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP//CH33";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/CH33";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID/CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "Y-/ID/CH*";
    _tfcs->_S1FareBasis = "Y/ID/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID/CH33";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP//CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID90/CH33";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    tktFareBasis = "Y-/ID*/CH*";
    _tfcs->_S1FareBasis = "Y/ID/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID90/CH";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID90/CH33";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP/ID/CH33";
    CPPUNIT_ASSERT(_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));

    _tfcs->_S1FareBasis = "YAP//CH";
    CPPUNIT_ASSERT(!_tfcs->matchFareBasis(tktFareBasis, *ptf, ts));
  }

  void testCheckSecurityReturnTrueWhenT183Zero()
  {
    _feeInfo->svcFeesSecurityTblItemNo() = 0;
    CPPUNIT_ASSERT(_tfc->checkSecurity(_feeInfo));
  }

  void testCheckSecurityReturnFalseWhenT183ZeroAndPrivateIndicatorON()
  {
    _feeInfo->svcFeesSecurityTblItemNo() = 0;
    _feeInfo->publicPrivateInd() = TicketingFeeCollector::T183_SCURITY_PRIVATE;
    CPPUNIT_ASSERT(!_tfc->checkSecurity(_feeInfo));
  }

  void testValidateSequenceReturnFAIL_TKT_DATEwhenTktDateFails()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().subtractDays(1);
    StatusS4Validation status = PASS_S4;
    if (!_tfc->checkTicketingDates(_feeInfo))
    {
      status = FAIL_TKT_DATE;
    }
    // CPPUNIT_ASSERT(!_tfc->checkTicketingDates(_feeInfo, status));
    CPPUNIT_ASSERT_EQUAL(FAIL_TKT_DATE, status);
  }

  void testValidateSequenceReturnFAIL_SVC_TYPEwhenServiceTypeFails()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().addDays(1);
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = "FCI";
    StatusS4Validation status = PASS_S4;
    CPPUNIT_ASSERT(!_tfc->validateSequence(_feeInfo, status));
    CPPUNIT_ASSERT_EQUAL(FAIL_SVC_TYPE, status);
  }

  void testValidateSequenceReturnFAIL_PAX_TYPEwhenPaxTypeFails()
  {
    _farePath->paxType()->paxType() = "TC1";
    createValidTicketingInfoForPaxTypeCheck();
    _tfcs->_tktInfo[0]->paxType() = SRC;
    CPPUNIT_ASSERT(!_tfcs->checkPaxType(_tfcs->_tktInfo[0]));
    StatusS4Validation status = PASS_S4;
    CPPUNIT_ASSERT(!_tfcs->validateSequence(_tfcs->_tktInfo[0], status));
    CPPUNIT_ASSERT_EQUAL(FAIL_PAX_TYPE, status);
  }

  void testValidateSequenceReturnFAIL_SECUR_T183whenSecurityFails()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().addDays(1);
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = ANY_CREDIT;
    _feeInfo->svcFeesSecurityTblItemNo() = 0;
    _feeInfo->publicPrivateInd() = TicketingFeeCollector::T183_SCURITY_PRIVATE;
    StatusS4Validation status = PASS_S4;
    CPPUNIT_ASSERT(!_tfc->validateSequence(_feeInfo, status));
    CPPUNIT_ASSERT_EQUAL(FAIL_SECUR_T183, status);
  }

  void testValidateSequenceReturnPASS_S4whenSequencePass()
  {
    _request->ticketingDT() = _feeInfo->ticketEffDate().addDays(1);
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = ANY_CREDIT;
    _feeInfo->svcFeesSecurityTblItemNo() = 0;
    StatusS4Validation status = PASS_S4;
    addTravelSegmentsToItin(LOC_LON, LOC_LON, LOC_LON, LOC_LON);
    CPPUNIT_ASSERT(_tfc->validateSequence(_feeInfo, status));
    CPPUNIT_ASSERT_EQUAL(PASS_S4, status);
  }

  void testCollectZeroWhenNoTicketingInfoRetrieved()
  {
    // Run with Diags on to verify there are no issues
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _tfc->createDiag();

    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    CPPUNIT_ASSERT_EQUAL((size_t)0, _farePath->collectedTktOBFees().size());
  }

  void testCollectZeroWhenNoValidTicketingInfo()
  {
    createTwoInvalidTicketingInfo();
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    _tfcs->collect();
    CPPUNIT_ASSERT_EQUAL((size_t)0, _farePath->collectedTktOBFees().size());
  }

  void testCollectTwoWhenTwoValidTicketingInfo()
  {
    createTwoValidTicketingInfo();
    _tfcs->_tktInfo[1]->fopBinNumber() = "2*****";
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    _tfcs->collect();
    CPPUNIT_ASSERT_EQUAL((size_t)2, _farePath->collectedTktOBFees().size());
  }

  void testCollectTwoWhenOneofTwoValidTicketingInfo()
  {
    createTwoValidTicketingInfo();
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    _tfcs->_tktInfo[0]->serviceTypeCode() = "XX"; // INVALID
    _tfcs->collect();
    CPPUNIT_ASSERT_EQUAL((size_t)1, _farePath->collectedTktOBFees().size());
  }

  void testCollectOneWhenTwoValidTicketingInfoButFirstHasFopBinNumberBlank()
  {
    createTwoValidTicketingInfo();
    _tfcs->_tktInfo[0]->fopBinNumber() = "";
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    _farePath->paxType()->paxType() = INFANT;
    _feeInfo->paxType() = INFANT;
    PaxType pt1;
    _trx->paxType() += &pt1;
    _tfcs->collect();
    CPPUNIT_ASSERT_EQUAL((size_t)1, _farePath->collectedTktOBFees().size());
  }

  void testCollectCallSetCurrencyNoDec()
  {
    createTwoValidTicketingInfo();
    _tfcs->_tktInfo[1]->fopBinNumber() = "2*****";
    _itin->travelSeg() += &_airSeg1, &_airSeg2;
    _tfcs->collect();
    CPPUNIT_ASSERT(_tfcs->_isCurrencyNoDecSet);
  }
  void testCreateDiagnosticNoDiagnosticRequested()
  {
    _trx->diagnostic().diagnosticType() = DiagnosticNone;
    _tfc->createDiag();
    CPPUNIT_ASSERT(!_tfc->diagActive());
  }

  void testCreateDiagnosticWhenDiagnosticRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _tfc->createDiag();
    CPPUNIT_ASSERT(_tfc->diagActive());
  }

  void testPrintDiagHeaderWhenNoDiagnosticRequested()
  {
    _trx->diagnostic().diagnosticType() = DiagnosticNone;
    _tfc->printDiagHeader();
    CPPUNIT_ASSERT(!_tfc->diagActive());
  }

  void testPrintDiagHeaderWhenDiagnosticRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _tfc->printDiagHeader();
    CPPUNIT_ASSERT(!_tfc->diagActive());
  }

  void testIsDiagSequenceMatchtExpectedMatchWhenSequenceRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "1002"));
    const TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    CPPUNIT_ASSERT(_tfc->isDiagSequenceMatch(tInfo));
  }

  void testIsDiagSequenceMatchtExpectedNoMatchWhenSequenceRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "1001"));
    const TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    CPPUNIT_ASSERT(!_tfc->isDiagSequenceMatch(tInfo));
  }

  void testIsDiagSequenceMatchtExpectedMatchWhenServiceCodeRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SC", "OBFCA"));
    const TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    CPPUNIT_ASSERT(_tfc->isDiagServiceCodeMatch(tInfo));
  }

  void testIsDiagSequenceMatchtExpectedNoMatchWhenServiceCodeRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SC", "OBFDA"));
    const TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    CPPUNIT_ASSERT(!_tfc->isDiagServiceCodeMatch(tInfo));
  }

  void testCheckDiagDetailedRequestExpectedNoMatchWhenDDINFOisNotRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "1002"));
    const TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    _tfc->createDiag();
    _tfc->checkDiagDetailedRequest(tInfo);
    CPPUNIT_ASSERT(!_tfc->_diagInfo);
  }

  void testCheckDiagDetailedRequestExpectedMatchWhenDDINFOisRequested()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    const TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    _tfc->createDiag();
    _tfc->checkDiagDetailedRequest(tInfo);
    CPPUNIT_ASSERT(_tfc->_diagInfo);
  }

  void testJouneyTypeWhenNationSameReturnFalse()
  {
    addTravelSegmentsToItin(LOC_DFW, LOC_DEN, LOC_CHI, LOC_NYC1);
    CPPUNIT_ASSERT(!_tfc->internationalJouneyType());
  }

  void testJouneyTypeWhenNationDifferentReturnTrue()
  {
    addTravelSegmentsToItin(LOC_DFW, LOC_LON, LOC_FRA, LOC_NYC1);
    CPPUNIT_ASSERT(_tfc->internationalJouneyType());
  }

  void testJouneyTypeWithArunkWhenNationDifferentReturnTrue()
  {
    AirSeg ts1;
    AirSeg tsA;
    AirSeg ts2;

    ts1.origin() = TestLocFactory::create(LOC_DFW);
    ts1.destination() = TestLocFactory::create(LOC_LON);
    ts2.origin() = TestLocFactory::create(LOC_FRA);
    ts2.destination() = TestLocFactory::create(LOC_NYC1);

    ts1.segmentType() = Air;
    ts2.segmentType() = Air;
    tsA.segmentType() = Arunk;
    _farePath->itin()->travelSeg() += &ts1, &tsA, &ts2;

    CPPUNIT_ASSERT(_tfc->internationalJouneyType());
  }

  void testRoundTripJouneyTypeWhenNationDifferentReturnTrue()
  {
    addTravelSegmentsToItin(LOC_DFW, LOC_LON, LOC_FRA, LOC_NYC1);
    CPPUNIT_ASSERT(_tfc->roundTripJouneyType());
  }

  void testRoundTripJouneyTypeWithArunkWhenNationDifferentReturnTrue()
  {
    AirSeg ts1;
    AirSeg tsA;
    AirSeg ts2;

    ts1.origin() = TestLocFactory::create(LOC_DFW);
    ts1.destination() = TestLocFactory::create(LOC_LON);
    ts2.origin() = TestLocFactory::create(LOC_FRA);
    ts2.destination() = TestLocFactory::create(LOC_NYC1);

    ts1.segmentType() = Air;
    ts2.segmentType() = Air;
    tsA.segmentType() = Arunk;
    _farePath->itin()->travelSeg() += &ts1, &tsA, &ts2;

    CPPUNIT_ASSERT(_tfc->roundTripJouneyType());
  }

  void testRoundTripJouneyTypeWhenNationSameReturnFalse()
  {
    addTravelSegmentsToItin(LOC_DFW, LOC_NYC1, LOC_CHI, LOC_NYC1);
    CPPUNIT_ASSERT(!_tfc->roundTripJouneyType());
  }

  void testGetValidSegsReturnFareBreaks()
  {
    createBasicPricingUnitAndItin();

    _tfc->getValidSegs();

    CPPUNIT_ASSERT_EQUAL((size_t)3, _tfc->_validSegs.size());
    CPPUNIT_ASSERT(
        _tfc->_validSegs.find(
            _tfc->_farePath->pricingUnit().front()->fareUsage().front()->travelSeg().front()) !=
        _tfc->_validSegs.end());
    CPPUNIT_ASSERT(
        _tfc->_validSegs.find(
            _tfc->_farePath->pricingUnit().front()->fareUsage().back()->travelSeg().front()) !=
        _tfc->_validSegs.end());
  }

  void testGetValidSegsReturnEmptyVector()
  {
    _tfc->getValidSegs();
    CPPUNIT_ASSERT_EQUAL((size_t)0, _tfc->_validSegs.size());
  }

  void testGetValidSegsReturnFareBreaksAndStopovers()
  {
    createBasicPricingUnitAndItin();
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfc->getValidSegs();

    CPPUNIT_ASSERT_EQUAL((size_t)3, _tfc->_validSegs.size());
    CPPUNIT_ASSERT(
        _tfc->_validSegs.find(
            _tfc->_farePath->pricingUnit().front()->fareUsage().front()->travelSeg().front()) !=
        _tfc->_validSegs.end());
    CPPUNIT_ASSERT(
        _tfc->_validSegs.find(
            _tfc->_farePath->pricingUnit().front()->fareUsage().back()->travelSeg().front()) !=
        _tfc->_validSegs.end());
  }

  void testGetValidSegsSkipEmbeddedSideTrip()
  {
    createBasicPricingUnitAndItin();
    _tfc->_farePath->pricingUnit().front()->isSideTripPU() = true;
    _tfc->getValidSegs();

    CPPUNIT_ASSERT_EQUAL((size_t)0, _tfc->_validSegs.size());
  }

  void testGetValidSegsStopoverIsFareBreak()
  {
    createBasicPricingUnitAndItin();
    _tfc->_farePath->pricingUnit()
        .front()
        ->fareUsage()
        .front()
        ->travelSeg()
        .front()
        ->forcedStopOver() = 'Y';
    _tfc->getValidSegs();

    CPPUNIT_ASSERT_EQUAL((size_t)3, _tfc->_validSegs.size());
  }

  void testGetValidSegsStopoverOnSideTrip()
  {
    createBasicPricingUnitAndItin();
    _tfc->_farePath->pricingUnit().front()->isSideTripPU() = true;
    _tfc->_farePath->pricingUnit()
        .front()
        ->fareUsage()
        .front()
        ->travelSeg()
        .front()
        ->forcedStopOver() = 'Y';
    _tfc->getValidSegs();

    CPPUNIT_ASSERT_EQUAL((size_t)0, _tfc->_validSegs.size());
  }

  void testSetMlgTurnAroundNotEmptyMapForGlobalDirectionXX()
  {
    populateValidSegs();
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_globalDir = GlobalDirection::XX;
    CPPUNIT_ASSERT(_tfcs->setMlgTurnAround());
    CPPUNIT_ASSERT(!_tfcs->_mlgMap.empty());
  }

  void testSetMlgTurnAroundPass()
  {
    populateValidSegs();
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_globalDir = GlobalDirection::ZZ;
    CPPUNIT_ASSERT(_tfcs->setMlgTurnAround());
    CPPUNIT_ASSERT(!_tfcs->_mlgMap.empty());
    CPPUNIT_ASSERT_EQUAL(1, (*_tfcs->_mlgMap.begin()).second);
  }

  void testValidateGeoLoc1Loc2PassWhenJourneyIndIsALoc1MatchOriginLoc2IsNull()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2PassWhenJourneyIndIsALoc1MatchOriginLoc2MatchDestination()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA1;
    _feeInfo->loc2().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_NYC1);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1MatchOriginLoc2NoMatchDestination()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA1;
    _feeInfo->loc2().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_NYC1);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginLoc1NoMatchDestination()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_NYC1);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void
  testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginAndDestinationLoc2NoMatchOrigniNorDestination()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA1;
    _feeInfo->loc2().loc() = IATA_AREA3;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_LON); // area 2
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void
  testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginAndDestinationLoc2NoMatchOrigniMatchDestination()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA3;
    _feeInfo->loc2().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_LON); // area 2
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void
  testValidateGeoLoc1Loc2FailWhenJourneyIndIsALoc1NoMatchOriginAndDestinationLoc2MatchOrigniNoMatchDestination()
  {
    _feeInfo->journeyInd() = 'A';
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA3;
    _feeInfo->loc2().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_LON); // area 2
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1IsNull()
  {
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1IsNotNullLoc2IsNull()
  {
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1MatchOriginLoc2MatchDestination()
  {
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA1;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_LON); // area 2
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2PassWhenJourneyIndIsBlankLoc1MatchDestinationLoc2MatchOrigin()
  {
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA2;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_LON); // area 2
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoLoc1Loc2FailWhenJourneyIndIsBlankLoc1IsNotNullLoc2IsNotNull()
  {
    _feeInfo->loc1().locType() = LOCTYPE_AREA;
    _feeInfo->loc1().loc() = IATA_AREA2;
    _feeInfo->loc2().locType() = LOCTYPE_AREA;
    _feeInfo->loc2().loc() = IATA_AREA3;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_journeyDestination = TestLocFactory::create(LOC_LON); // area 2
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoLoc1Loc2(_feeInfo));
  }

  void testValidateGeoViaFailWhenViaNotFound()
  {
    _feeInfo->locVia().locType() = LOCTYPE_AREA;
    _feeInfo->locVia().loc() = IATA_AREA3;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoVia(_feeInfo));
  }

  void testValidateGeoViaPassWhenViaLocFound()
  {
    _feeInfo->locVia().locType() = LOCTYPE_AREA;
    _feeInfo->locVia().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_NYC1, LOC_LON, LOC_NYC1, LOC_CHI);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoVia(_feeInfo));
  }

  void testValidateGeoViaFailWhenOriginAndDestinationMatchVia()
  {
    _feeInfo->locVia().locType() = LOCTYPE_AREA;
    _feeInfo->locVia().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_FRA, LOC_NYC1, LOC_DFW, LOC_FRA);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoVia(_feeInfo));
  }

  void testValidateGeoViaFailWhenTurnAroundPointMatchVia()
  {
    _feeInfo->locVia().locType() = LOCTYPE_AREA;
    _feeInfo->locVia().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_FRA, LOC_NYC1, LOC_NYC1, LOC_FRA);
    _tfcs->_validateGeoLoc = true;
    _tfcs->_journeyTurnAroundTs = _itin->travelSeg().front();
    _tfcs->_journeyTurnAroundPoint = _itin->travelSeg().front()->destination();
    CPPUNIT_ASSERT(!_tfcs->validateGeoVia(_feeInfo));
  }

  void testValidateGeoViaPassWhenTurnAroundPointMatchViaAndThereIsDifferentViaPoint()
  {
    _feeInfo->locVia().locType() = LOCTYPE_AREA;
    _feeInfo->locVia().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_FRA, LOC_NYC1, LOC_NYC1, LOC_FRA);
    addTravelSegmentsToItin(LOC_FRA, LOC_NYC1, LOC_NYC1, LOC_LON);
    _tfcs->_validateGeoLoc = true;
    _tfcs->_journeyTurnAroundTs = _itin->travelSeg().front();
    _tfcs->_journeyTurnAroundPoint = _itin->travelSeg().front()->destination();
    CPPUNIT_ASSERT(_tfcs->validateGeoVia(_feeInfo));
  }

  void testValidateGeoViaPassWhenTurnAroundPointMatchViaNation()
  {
    _feeInfo->locVia().locType() = LOCTYPE_NATION;
    _feeInfo->locVia().loc() = NATION_US;
    addTravelSegmentsToItin(LOC_FRA, LOC_CHI, LOC_CHI, LOC_LON);
    addTravelSegmentsToItin(LOC_LON, LOC_CHI, LOC_CHI, LOC_FRA);
    _tfcs->_journeyTurnAroundTs = _itin->travelSeg().front();
    _tfcs->_journeyTurnAroundPoint = _itin->travelSeg().front()->destination();
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoVia(_feeInfo));
  }
  void testValidateGeoViaFailWhenTurnAroundPointMatchViaAndLastPoint()
  {
    _feeInfo->locVia().locType() = LOCTYPE_NATION;
    _feeInfo->locVia().loc() = NATION_US;
    addTravelSegmentsToItin(LOC_FRA, LOC_NYC1, LOC_NYC1, LOC_FRA);
    addTravelSegmentsToItin(LOC_LON, LOC_FRA, LOC_FRA, LOC_NYC1);
    _tfcs->_journeyTurnAroundTs = _itin->travelSeg().front();
    _tfcs->_journeyTurnAroundPoint = _itin->travelSeg().front()->destination();
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(!_tfcs->validateGeoVia(_feeInfo));
  }

  void testValidateGeoWhlWithinPassArea1()
  {
    _feeInfo->locWhlWithin().locType() = LOCTYPE_AREA;
    _feeInfo->locWhlWithin().loc() = IATA_AREA1;
    addTravelSegmentsToItin(LOC_DFW, LOC_NYC1, LOC_NYC1, LOC_CHI);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoWhlWithin(_feeInfo));
  }

  void testValidateGeoWhlWithinPassArea2()
  {
    _feeInfo->locWhlWithin().locType() = LOCTYPE_AREA;
    _feeInfo->locWhlWithin().loc() = IATA_AREA2;
    addTravelSegmentsToItin(LOC_LON, LOC_FRA, LOC_FRA, LOC_LON);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoWhlWithin(_feeInfo));
  }

  void testValidateGeoWhlWithinPassCountryUS()
  {
    _feeInfo->locWhlWithin().locType() = LOCTYPE_NATION;
    _feeInfo->locWhlWithin().loc() = NATION_US;
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoWhlWithin(_feeInfo));
  }

  void testValidateGeoWhlWithinPassWhenBlank()
  {
    _feeInfo->locWhlWithin().locType() = RuleConst::BLANK;
    _feeInfo->locWhlWithin().loc() = EMPTY_STRING();
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoWhlWithin(_feeInfo));
  }

  void testValidateGeoViaPassWhenBlank()
  {
    _feeInfo->locVia().locType() = RuleConst::BLANK;
    _feeInfo->locVia().loc() = EMPTY_STRING();
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->_validateGeoLoc = true;
    CPPUNIT_ASSERT(_tfcs->validateGeoVia(_feeInfo));
  }

  void testCompareFopBinsTrueWhenFopEqual()
  {
    TicketingFeesInfo fi1, fi2;
    fi1.fopBinNumber() = "12****";
    fi2.fopBinNumber() = "12****";
    CPPUNIT_ASSERT(fi1.compareFopBin(fi2.fopBinNumber()));
  }

  void testCompareFopBinsTrueWhenFopNotEqualButMatch()
  {
    TicketingFeesInfo fi1, fi2;
    fi1.fopBinNumber() = "12****";
    fi2.fopBinNumber() = "123***";
    CPPUNIT_ASSERT(fi1.compareFopBin(fi2.fopBinNumber()));
  }

  void testCompareFopBinsFalseFopNotEqualAndNotCover()
  {
    TicketingFeesInfo fi1, fi2;
    fi1.fopBinNumber() = "123***";
    fi2.fopBinNumber() = "1256**";
    CPPUNIT_ASSERT(!fi1.compareFopBin(fi2.fopBinNumber()));
  }

  void testCheckFopBinNumberSetStopProcessingWhenFopBinNumberEmpty()
  {
    _feeInfo->fopBinNumber() = EMPTY_STRING();
    _tfc->_stopS4Processing = false;
    _tfc->_stopS4ProcessingFDA = true;
    _tfc->_stopS4ProcessingFCA = true;
    _tfc->checkFopBinNumber(_feeInfo);
    CPPUNIT_ASSERT(_tfc->_stopS4Processing);
  }

  void testCheckFopBinNumberNotSetStopProcessingWhenFopBinNumberNotEmpty()
  {
    _feeInfo->fopBinNumber() = "622***";
    _tfc->_stopS4Processing = false;
    _tfc->checkFopBinNumber(_feeInfo);
    CPPUNIT_ASSERT(!_tfc->_stopS4Processing);
  }

  void testCheckDuplicatedRecordsPassWhenExist()
  {
    TicketingFeesInfo* tfiPass = createTicketingFeesInfoWithBinNumber("12****");
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfoWithBinNumber("123***");
    _farePath->collectedTktOBFees().push_back(tfiPass);
    CPPUNIT_ASSERT(_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedRecordsFailWhenEmptyVector()
  {
    TicketingFeesInfo* tfi = createTicketingFeesInfoWithBinNumber("123***");
    CPPUNIT_ASSERT(!_tfc->checkDuplicatedRecords(tfi));
  }

  void testCheckDuplicatedRecordsFailWhenNoDuplicateFound()
  {
    TicketingFeesInfo* tfiPass = createTicketingFeesInfoWithBinNumber("124***");
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfoWithBinNumber("123***");
    _farePath->collectedTktOBFees().push_back(tfiPass);
    CPPUNIT_ASSERT(!_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedRecordsPassWhenMultipleRecordsInVectorFirstMatch()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfoWithBinNumber("123***");
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("12****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("2*****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("3*****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("456***"));
    CPPUNIT_ASSERT(_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedRecordsPassWhenMultipleRecordsInVectorLastMatch()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfoWithBinNumber("456789");
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("12****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("2*****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("3*****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("456***"));
    CPPUNIT_ASSERT(_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedRecordsFailWhenMultipleRecordsInVectorNoMatch()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfoWithBinNumber("5*****");
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("12****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("2*****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("3*****"));
    _farePath->collectedTktOBFees().push_back(createTicketingFeesInfoWithBinNumber("456***"));
    CPPUNIT_ASSERT(!_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedRecordsPassWhenDifferentSubtypes()
  {
    TicketingFeesInfo* tfiPass = createTicketingFeesInfoWithBinNumber("12****");
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfoWithBinNumber("123***");
    tfiPass->serviceSubTypeCode() = ANY_CREDIT;
    tfiCheck->serviceSubTypeCode() = ANY_DEBIT;
    _farePath->collectedTktOBFees().push_back(tfiPass);
    CPPUNIT_ASSERT(!_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckAccountCodeReturnTrueWhenT172Zero()
  {
    _feeInfo->svcFeesAccCodeTblItemNo() = 0;
    CPPUNIT_ASSERT(_tfc->checkAccountCode(_feeInfo));
  }

  void testCheckTktDesignatorWithNoItemNumber()
  {
    _feeInfo->svcFeesTktDsgnTblItemNo() = 0;
    CPPUNIT_ASSERT(_tfc->checkTktDesignator(_feeInfo));
  }
  void testSetJourneyDestinationWhenJourneyIsOneWayInternational()
  {
    _tfcs->_isRTJourneyType = false;
    _tfcs->_isIntJourneyType = true;
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->setJourneyDestination();
    CPPUNIT_ASSERT_EQUAL(_farePath->itin()->travelSeg().back()->destination(),
                         _tfcs->_journeyDestination);
    CPPUNIT_ASSERT(!_tfcs->_journeyTurnAroundPoint);
    CPPUNIT_ASSERT(_tfcs->_journeyDestination);
  }

  void testSetJourneyDestinationWhenJourneyIsOneWayDomestic()
  {
    _tfcs->_isRTJourneyType = false;
    _tfcs->_isIntJourneyType = false;
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->setJourneyDestination();
    CPPUNIT_ASSERT_EQUAL(_farePath->itin()->travelSeg().back()->destination(),
                         _tfcs->_journeyDestination);
    CPPUNIT_ASSERT(!_tfcs->_journeyTurnAroundPoint);
  }

  void testSetJourneyDestinationWhenJourneyIsRoundTripDomestic()
  {
    _tfcs->_isRTJourneyType = true;
    _tfcs->_isIntJourneyType = false;
    _tfcs->_setMlgTurnAround = true;
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->_journeyTurnAroundPoint = TestLocFactory::create(LOC_NYC1);
    _tfcs->setJourneyDestination();
    CPPUNIT_ASSERT_EQUAL(_tfcs->_journeyTurnAroundPoint, _tfcs->_journeyDestination);
    CPPUNIT_ASSERT(_tfcs->_journeyTurnAroundPoint);
  }

  void testSetJourneyDestinationJourneyIsRoundTripReturnWhenBadData()
  {
    _tfcs->_isRTJourneyType = true;
    _tfcs->_setMlgTurnAround = false;
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW);
    _tfcs->setJourneyDestination();
    CPPUNIT_ASSERT(!_tfcs->_journeyDestination);
    CPPUNIT_ASSERT(!_tfcs->_journeyTurnAroundPoint);
  }

  void
  testSetJourneyDestinationWhenJourneyIsRoundTripInternationalTurnAroundPointIsInDifferentCountryThanJourneyOrigin()
  {
    _tfcs->_isRTJourneyType = true;
    _tfcs->_isIntJourneyType = true;
    _tfcs->_setMlgTurnAround = true;
    addTravelSegmentsToItin(LOC_DEN, LOC_NYC1, LOC_NYC1, LOC_DFW); // origin in US
    _tfcs->_journeyTurnAroundPoint = TestLocFactory::create(LOC_LON); // turn around in UK
    _tfcs->setJourneyDestination();
    CPPUNIT_ASSERT_EQUAL(_tfcs->_journeyTurnAroundPoint, _tfcs->_journeyDestination);
    CPPUNIT_ASSERT(_tfcs->_journeyDestination);
  }

  void
  testSetJourneyDestinationWhenJourneyIsRoundTripInternationalTurnAroundPointIsInTheSameCountryAsJourneyOrigin()
  {
    _tfcs->_isRTJourneyType = true;
    _tfcs->_isIntJourneyType = true;
    _tfcs->_setMlgTurnAround = true;
    _tfcs->_mlgMap.insert(std::map<int, int, std::greater<int> >::value_type(100, 1));
    _tfcs->_mlgMap.insert(std::map<int, int, std::greater<int> >::value_type(200, 2));

    addTravelSegmentsToItin(LOC_DEN, LOC_LON, LOC_LON, LOC_DFW); // origin in US
    _tfcs->_journeyTurnAroundPoint = TestLocFactory::create(LOC_DFW); // turn around in US
    _tfcs->setJourneyDestination();
    CPPUNIT_ASSERT_EQUAL(_itin->travelSeg().front()->destination(), _tfcs->_journeyDestination);
    CPPUNIT_ASSERT(_tfcs->_journeyDestination);
  }

  void testGetS1FareBasisCodeReturnEmptyStringForArunk()
  {
    ArunkSeg* arunk = _memHandle.create<ArunkSeg>();
    arunk->origin() = TestLocFactory::create(LOC_DFW);
    arunk->destination() = TestLocFactory::create(LOC_LON);
    arunk->segmentType() = Arunk;
    _itin->travelSeg() += arunk;
    createBasicPricingUnitAndItin();
    std::string str1 = EMPTY_STRING();
    std::string matchedAccCode("");
    PaxTypeFare* ptf = createPaxTypeFareWithFareInfo(AIR_FRANCE, CLASS_YOW, matchedAccCode);
    _tfcs->_S1FareBasis = "";
    std::string str2 = _tfcs->getS1FareBasisCode(*ptf, 0);
    CPPUNIT_ASSERT_EQUAL(str1, str2);
  }

  void testIsStopOverPointPassWhenIsStopOver()
  {
    FareUsage* fu = createFareUsageWithGeoTravelType(GeoTravelType::International);
    _airSeg1.arrivalDT() = DateTime(2009, 3, 10, 11, 30, 0);
    _airSeg2.departureDT() = DateTime(2009, 3, 11, 11, 31, 0);
    _airSeg1.segmentType() = Air;
    _airSeg2.segmentType() = Air;
    _airSeg1.forcedConx() = 'N';
    CPPUNIT_ASSERT(_tfc->isStopOverPoint(&_airSeg1, &_airSeg2, fu));
  }

  void testIsStopOverPointPassWhenTravelSegIsInStooversVector()
  {
    FareUsage* fu = createFareUsageWithGeoTravelType(GeoTravelType::International);
    fu->stopovers().insert(&_airSeg1);
    _airSeg1.arrivalDT() = DateTime(2009, 3, 10, 11, 30, 0);
    _airSeg2.departureDT() = DateTime(2009, 3, 10, 11, 31, 0);
    CPPUNIT_ASSERT(_tfc->isStopOverPoint(&_airSeg1, &_airSeg2, fu));
  }

  void testIsStopOverPointFail()
  {
    FareUsage* fu = createFareUsageWithGeoTravelType(GeoTravelType::International);
    _airSeg1.arrivalDT() = DateTime(2009, 3, 10, 11, 30, 0);
    _airSeg2.departureDT() = DateTime(2009, 3, 10, 11, 31, 0);
    CPPUNIT_ASSERT(!_tfc->isStopOverPoint(&_airSeg1, &_airSeg2, fu));
  }

  void testIsStopOverPassWhenStopOverPoint()
  {
    FareUsage* fu = createFareUsageWithGeoTravelType(GeoTravelType::International);
    _airSeg1.arrivalDT() = DateTime(2009, 3, 10, 11, 30, 0);
    _airSeg2.departureDT() = DateTime(2009, 3, 11, 11, 31, 0);
    CPPUNIT_ASSERT(_tfc->isStopOver(&_airSeg1, &_airSeg2, fu));
  }

  void testIsStopOverPassWhenIsNotStopOverPoint()
  {
    FareUsage* fu = createFareUsageWithGeoTravelType(GeoTravelType::International);
    ArunkSeg arunkSeg;
    arunkSeg.segmentType() = Arunk;
    fu->travelSeg() += &_airSeg1, &arunkSeg;

    _airSeg1.arrivalDT() = DateTime(2009, 3, 10, 11, 30, 0);
    _airSeg2.departureDT() = DateTime(2009, 3, 10, 11, 31, 0);
    CPPUNIT_ASSERT(_tfc->isStopOver(&_airSeg1, &_airSeg2, fu));
  }

  void testIsStopOverFailWhenIsNotStopOverPoint()
  {
    FareUsage* fu = createFareUsageWithGeoTravelType(GeoTravelType::International);
    fu->travelSeg() += &_airSeg1;
    _airSeg1.arrivalDT() = DateTime(2009, 3, 10, 11, 30, 0);
    _airSeg2.departureDT() = DateTime(2009, 3, 10, 11, 31, 0);
    CPPUNIT_ASSERT(!_tfc->isStopOver(&_airSeg1, &_airSeg2, fu));
  }
  void testisArunkPartOfSideTrip()
  {
    createBasicPricingUnitAndItin();
    TravelSeg* tv = _itin->travelSeg().front();
    CPPUNIT_ASSERT(!_tfc->isArunkPartOfSideTrip(tv));
  }

  void testIsProcessFOPFailWhenNoFop()
  {
    _trx->getRequest()->formOfPayment().clear();
    std::vector<FopBinNumber> fopBin;
    CPPUNIT_ASSERT(!_tfcs->isProcessFOP(fopBin));
    CPPUNIT_ASSERT(fopBin.empty());
  }

  void testIsProcessFOPFailWhenSingleFopIsNot6Digits()
  {
    _trx->getRequest()->formOfPayment() = "12345";
    std::vector<FopBinNumber> fopBin;
    CPPUNIT_ASSERT(!_tfcs->isProcessFOP(fopBin));
    CPPUNIT_ASSERT(fopBin.empty());
  }

  void testIsProcessFOPFailWhenSingleFopIs6Mixed()
  {
    _trx->getRequest()->formOfPayment() = "12A456";
    std::vector<FopBinNumber> fopBin;
    CPPUNIT_ASSERT(!_tfcs->isProcessFOP(fopBin));
    CPPUNIT_ASSERT(fopBin.empty());
  }

  void testIsProcessFOPPassWhenSingleFopIs6Digits()
  {
    _trx->getRequest()->formOfPayment() = "123456";
    std::vector<FopBinNumber> fopBin;
    CPPUNIT_ASSERT(_tfcs->isProcessFOP(fopBin));
    CPPUNIT_ASSERT_EQUAL(_trx->getRequest()->formOfPayment(), fopBin[0]);
  }

  void testIsFopBinMatchFailWhenNoMatchSingleFop()
  {
    TicketingFeesInfo* tfi = createTicketingFeesInfoWithBinNumber("123456");
    std::vector<FopBinNumber> fopBin;
    fopBin.push_back("123457");

    CPPUNIT_ASSERT(!_tfc->isFopBinMatch(*tfi, fopBin));
    CPPUNIT_ASSERT(!fopBin.empty());
  }

  void testIsFopBinMatchTrueWhenMatchSingleFop()
  {
    TicketingFeesInfo* tfi = createTicketingFeesInfoWithBinNumber("123456");
    std::vector<FopBinNumber> fopBin;
    fopBin.push_back("123456");

    CPPUNIT_ASSERT(_tfc->isFopBinMatch(*tfi, fopBin));
    CPPUNIT_ASSERT(fopBin.empty());
  }

  void testIsAnyNonCreditCardFOPFalseWhenCreditCard()
  {
    _trx->getRequest()->formOfPaymentCard() = 'T';
    _trx->getRequest()->formOfPaymentCash() = 'T';
    CPPUNIT_ASSERT(!_tfc->isAnyNonCreditCardFOP());
  }

  void testIsAnyNonCreditCardFOPTrueWhenNonCreditCardAndCash()
  {
    _trx->getRequest()->formOfPaymentCard() = 'F';
    _trx->getRequest()->formOfPaymentCash() = 'T';
    CPPUNIT_ASSERT(_tfc->isAnyNonCreditCardFOP());
  }

  void testCheckDuplicatedRTypeRecordsPassWhenExist()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R11";
    _tfc->ticketingFeesInfoFromFP(tfiCheck).push_back(tfiCheck);
    CPPUNIT_ASSERT(_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedRTypeRecordsFalseWhenDoNotExist()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R11";
    CPPUNIT_ASSERT(!_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedTTypeRecordsPassWhenExist()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "T01";
    _tfc->ticketingFeesInfoFromFP(tfiCheck).push_back(tfiCheck);
    CPPUNIT_ASSERT(_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckDuplicatedTTypeRecordsFalseWhenDoNotExist()
  {
    TicketingFeesInfo* tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "T01";
    CPPUNIT_ASSERT(!_tfc->checkDuplicatedRecords(tfiCheck));
  }

  void testCheckSortRTypeOBFees()
  {
    TicketingFeesInfo* tfiCheck;

    tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R41";
    _farePath->collectedRTypeOBFee().push_back(tfiCheck);
    tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R03";
    _farePath->collectedRTypeOBFee().push_back(tfiCheck);
    tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R01";
    _farePath->collectedRTypeOBFee().push_back(tfiCheck);
    tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R02";
    _farePath->collectedRTypeOBFee().push_back(tfiCheck);
    tfiCheck = createTicketingFeesInfo();
    tfiCheck->serviceTypeCode() = "OB";
    tfiCheck->serviceSubTypeCode() = "R22";
    _farePath->collectedRTypeOBFee().push_back(tfiCheck);

    _tfc->sortOBFeeVectors(_farePath->collectedRTypeOBFee());

    CPPUNIT_ASSERT(_farePath->collectedRTypeOBFee()[0]->serviceSubTypeCode() == "R01");
    CPPUNIT_ASSERT(_farePath->collectedRTypeOBFee()[2]->serviceSubTypeCode() == "R03");
    CPPUNIT_ASSERT(_farePath->collectedRTypeOBFee()[4]->serviceSubTypeCode() == "R41");
  }

  void testInitializeFor2Cc()
  {
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = true;
    _tfc->initializeFor2CreditCards(fopMatched);

    CPPUNIT_ASSERT_EQUAL(size_t(2), fopMatched.size());
  }

  void testInitializeFor2CcReturnIfNot2CC()
  {
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = false;
    _tfc->initializeFor2CreditCards(fopMatched);

    CPPUNIT_ASSERT_EQUAL(fopMatched.size(), size_t(0));
  }

  void testIsSvcTypeMatchCardTypePassCredit()
  {
    ServiceSubTypeCode svcSubCode = ANY_CREDIT;
    Indicator cardType = TicketingFeeCollector::CREDIT;

    CPPUNIT_ASSERT(_tfc->isSvcTypeMatchCardType(svcSubCode, cardType));
  }

  void testIsSvcTypeMatchCardTypePassDebit()
  {
    ServiceSubTypeCode svcSubCode = ANY_DEBIT;
    Indicator cardType = TicketingFeeCollector::DEBIT;

    CPPUNIT_ASSERT(_tfc->isSvcTypeMatchCardType(svcSubCode, cardType));
  }

  void testIsSvcTypeMatchCardTypeFailCredit()
  {
    ServiceSubTypeCode svcSubCode = ANY_DEBIT;
    Indicator cardType = TicketingFeeCollector::CREDIT;

    CPPUNIT_ASSERT(!_tfc->isSvcTypeMatchCardType(svcSubCode, cardType));
  }

  void testIsSvcTypeMatchCardTypeFailDebit()
  {
    ServiceSubTypeCode svcSubCode = ANY_CREDIT;
    Indicator cardType = TicketingFeeCollector::DEBIT;

    CPPUNIT_ASSERT(!_tfc->isSvcTypeMatchCardType(svcSubCode, cardType));
  }

  void testAddFopBinIfMatchAddOneForEmpty()
  {
    prepareFeeInfo("", "", "", ANY_CREDIT,
                   TicketingFeeCollector::CREDIT,
                   TicketingFeeCollector::DEBIT);
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = true;
    _tfc->initializeFor2CreditCards(fopMatched);

    CPPUNIT_ASSERT(_tfc->addFopBinIfMatch(*_feeInfo, fopMatched));
    CPPUNIT_ASSERT(fopMatched.front());
    CPPUNIT_ASSERT(!fopMatched.back());
  }

  void testAddFopBinIfMatchAddTwoForEmpty()
  {
    prepareFeeInfo("", "", "", ANY_CREDIT,
                   TicketingFeeCollector::CREDIT,
                   TicketingFeeCollector::CREDIT);
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = true;
    _tfc->initializeFor2CreditCards(fopMatched);

    CPPUNIT_ASSERT(_tfc->addFopBinIfMatch(*_feeInfo, fopMatched));
    CPPUNIT_ASSERT(fopMatched.front());
    CPPUNIT_ASSERT(fopMatched.back());
  }

  void testAddFopBinIfMatchAddTwoForEmptyFda()
  {
    prepareFeeInfo("", "", "", ANY_DEBIT,
                   TicketingFeeCollector::DEBIT,
                   TicketingFeeCollector::DEBIT);
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = true;
    _tfc->initializeFor2CreditCards(fopMatched);

    CPPUNIT_ASSERT(_tfc->addFopBinIfMatch(*_feeInfo, fopMatched));
    CPPUNIT_ASSERT(fopMatched.front());
    CPPUNIT_ASSERT(fopMatched.back());
  }

  void testAddFopBinIfMatchFail()
  {
    prepareFeeInfo("", "", "", ANY_CREDIT,
                   TicketingFeeCollector::DEBIT,
                   TicketingFeeCollector::DEBIT);
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = true;
    _tfc->initializeFor2CreditCards(fopMatched);

    CPPUNIT_ASSERT(!_tfc->addFopBinIfMatch(*_feeInfo, fopMatched));
    CPPUNIT_ASSERT(!fopMatched.front());
    CPPUNIT_ASSERT(!fopMatched.back());
  }

  void testAdjustAnyCreditCardTypeNoChangeForDebit()
  {
    Indicator cardType(TicketingFeeCollector::DEBIT);
    _tfc->adjustAnyCreditCardType(cardType);
    CPPUNIT_ASSERT_EQUAL(TicketingFeeCollector::DEBIT, cardType);
  }

  void testAdjustAnyCreditCardTypeNoChangeForCredit()
  {
    Indicator cardType(TicketingFeeCollector::CREDIT);
    _tfc->adjustAnyCreditCardType(cardType);
    CPPUNIT_ASSERT_EQUAL(TicketingFeeCollector::CREDIT, cardType);
  }

  void testAdjustAnyCreditCardTypeChangeForG()
  {
    Indicator cardType('G');
    _tfc->adjustAnyCreditCardType(cardType);
    CPPUNIT_ASSERT_EQUAL(TicketingFeeCollector::CREDIT, cardType);
  }

  void testAdjustAnyCreditCardTypeChangeForBlank()
  {
    Indicator cardType(TicketingFeeCollector::BLANK);
    _tfc->adjustAnyCreditCardType(cardType);
    CPPUNIT_ASSERT_EQUAL(TicketingFeeCollector::CREDIT, cardType);
  }

  void testSwapObFeesVecFro2CC()
  {
    std::vector<TicketingFeesInfo*> fopMatched;
    _tfc->_process2CC = true;
    _tfc->initializeFor2CreditCards(fopMatched);
    _tfc->swapObFeesVecFor2CC(fopMatched);
    CPPUNIT_ASSERT_EQUAL(size_t(2), _farePath->collectedTktOBFees().size());
  }

protected:
  PricingTrx* _trx;
  PricingRequest* _request;
  FarePath* _farePath;
  Itin* _itin;
  PaxType* _paxType;
  TicketingFeeCollector* _tfc;
  TicketingFeeCollectorStub* _tfcs;
  TicketingFeesInfo* _feeInfo;
  AirSeg _airSeg1;
  AirSeg _airSeg2;
  TestMemHandle _memHandle;
  Customer _customer;
  Agent _agent;

  void addTravelSegmentsToItin(const std::string loc1Orig,
                               const std::string loc1Dest,
                               const std::string loc2Orig,
                               const std::string loc2Dest)
  {
    AirSeg* as1 = _memHandle.create<AirSeg>();
    as1->origin() = TestLocFactory::create(loc1Orig);
    as1->destination() = TestLocFactory::create(loc1Dest);
    as1->segmentType() = Air;
    as1->forcedStopOver() = 'Y';

    AirSeg* as2 = _memHandle.create<AirSeg>();
    as2->origin() = TestLocFactory::create(loc2Orig);
    as2->destination() = TestLocFactory::create(loc2Dest);
    as2->segmentType() = Air;
    _itin->travelSeg() += as1, as2;
  }

  void createBasicPricingUnitAndItin()
  {
    AirSeg* ts1 = _memHandle.create<AirSeg>();
    AirSeg* ts2 = _memHandle.create<AirSeg>();
    AirSeg* ts3 = _memHandle.create<AirSeg>();

    ts1->origin() = TestLocFactory::create(LOC_DFW);
    ts1->destination() = TestLocFactory::create(LOC_DEN);
    ts2->origin() = TestLocFactory::create(LOC_CHI);
    ts2->destination() = TestLocFactory::create(LOC_NYC1);

    ts1->segmentType() = Air;
    ts2->segmentType() = Air;
    ts3->segmentType() = Arunk;

    _itin->travelSeg() += ts1, ts2, ts3;
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    fu1->travelSeg() += ts1;
    fu2->travelSeg() += ts2;
    fu3->travelSeg() += ts3;
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage() += fu1;
    pu->fareUsage() += fu2;
    pu->fareUsage() += fu3;
    _farePath->pricingUnit() += pu;
  }

  void createTwoInvalidTicketingInfo()
  {
    for (int i = 0; i < 2; ++i)
      _tfcs->_tktInfo.push_back(_memHandle.create<TicketingFeesInfo>());
  }

  void createTwoValidTicketingInfo()
  {
    DateTime effDate = DateTime::localTime();
    _request->ticketingDT() = effDate.addDays(1);

    for (int i = 0; i < 2; ++i)
    {
      _tfcs->_tktInfo.push_back(_memHandle.create<TicketingFeesInfo>());
      _tfcs->_tktInfo[i]->ticketEffDate() = effDate;
      _tfcs->_tktInfo[i]->ticketDiscDate() = effDate.addDays(2);
      _tfcs->_tktInfo[i]->serviceTypeCode() = "OB";
      _tfcs->_tktInfo[i]->serviceSubTypeCode() = ANY_CREDIT;
      _tfcs->_tktInfo[i]->fopBinNumber() = "1*****";
    }
  }

  void createValidTicketingInfoForPaxTypeCheck()
  {
    const DateTime effDate = DateTime(2009, 3, 10, 11, 59, 0);
    const DateTime disDate = DateTime(2009, 3, 12, 11, 59, 0);
    _request->ticketingDT() = effDate.addDays(1);

    _tfcs->_tktInfo.push_back(_memHandle.create<TicketingFeesInfo>());
    _tfcs->_tktInfo[0]->ticketEffDate() = effDate;
    _tfcs->_tktInfo[0]->ticketDiscDate() = disDate;
    _tfcs->_tktInfo[0]->serviceTypeCode() = "OB";
    _tfcs->_tktInfo[0]->serviceSubTypeCode() = ANY_CREDIT;
  }

  void populateValidSegs()
  {
    AirSeg* ts1 = _memHandle.create<AirSeg>();
    AirSeg* ts2 = _memHandle.create<AirSeg>();

    ts1->origin() = TestLocFactory::create(LOC_DFW);
    ts1->destination() = TestLocFactory::create(LOC_DEN);
    ts2->origin() = TestLocFactory::create(LOC_CHI);
    ts2->destination() = TestLocFactory::create(LOC_NYC1);

    ts1->segmentType() = Air;
    ts2->segmentType() = Air;

    _tfcs->_validSegs.clear();
    _tfcs->_validSegs.insert(ts1);
    _tfcs->_validSegs.insert(ts2);
  }

  TicketingFeesInfo* createTicketingFeesInfo() { return _memHandle.create<TicketingFeesInfo>(); }

  TicketingFeesInfo* createTicketingFeesInfoWithBinNumber(const std::string& binNumber)
  {
    TicketingFeesInfo* info = createTicketingFeesInfo();
    info->fopBinNumber() = binNumber;
    info->serviceTypeCode() = "OB";
    info->serviceSubTypeCode() = ANY_CREDIT;
    return info;
  }

  PaxTypeFare* createPaxTypeFareWithFareInfo(const std::string& carrier,
                                             const std::string& fareClass,
                                             const std::string& matchedAccCode = "")
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_carrier = carrier;
    fi->_fareClass = fareClass;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    fcasi->_tktDesignator = "";
    ptf->fareClassAppSegInfo() = fcasi;
    ptf->matchedAccCode() = matchedAccCode;

    return ptf;
  }
  FareUsage* createFareUsageWithGeoTravelType(const GeoTravelType& geoTvlType)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = _memHandle.create<PaxTypeFare>();
    fu->paxTypeFare()->fareMarket() = _memHandle.create<FareMarket>();
    fu->paxTypeFare()->fareMarket()->geoTravelType() = geoTvlType;
    return fu;
  }

  void prepareFeeInfo(FopBinNumber fop1, FopBinNumber fop2,
                                    FopBinNumber fopS4,
                                    ServiceSubTypeCode svcSubCode,
                                    Indicator cardType,
                                    Indicator cardType2)
  {
    _request->formOfPayment() = fop1;
    _request->secondFormOfPayment() = fop2;
    _trx->setRequest(_request);
    _feeInfo->serviceSubTypeCode() = svcSubCode;
    _feeInfo->fopBinNumber() = fopS4;
    _tfc->_cardType = cardType;
    _tfc->_secondCardType = cardType2;
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(TicketingFeeCollectorTest);
}
