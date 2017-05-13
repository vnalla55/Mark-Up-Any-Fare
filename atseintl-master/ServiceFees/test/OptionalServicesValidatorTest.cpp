#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag877Collector.h"
#include "Rules/RuleConst.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OptionalServicesValidator.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <map>
#include <utility>

namespace tse
{
using namespace std;

namespace
{
class OptionalServicesValidatorMock : public OptionalServicesValidator
{
public:
  OptionalServicesValidatorMock(
      PricingTrx& trx,
      FarePath& fp,
      vector<TravelSeg*>::const_iterator segI,
      vector<TravelSeg*>::const_iterator segIE,
      const Ts2ss& ts2ss)
    : OptionalServicesValidator(OcValidationContext(trx, *fp.itin(), *fp.paxType(), &fp),
                                segI,
                                segIE,
                                segIE,
                                ts2ss,
                                false,
                                true,
                                true,
                                NULL),
      tempFlag(false)
  {
    _rbdInfos.push_back(&_rbdInfo1);
    _rbdInfos.push_back(&_rbdInfo2);
    _svcFeeCurInfos.push_back(&_currInfo);
    _svcResFCInfos.push_back(&_resFCInfo1);
    _svcResFCInfos.push_back(&_resFCInfo2);
    _cxrFlt1 = new CarrierFlightSeg; // memory will be deallocated in ~TaxCarrierFlightInfo
    _cxrFlt2 = new CarrierFlightSeg; // memory will be deallocated in ~TaxCarrierFlightInfo
    _cxrFlt1->marketingCarrier() = "CC";
    _cxrFlt1->operatingCarrier() = "BB";
    _cxrFlt1->flt1() = -1;
    _cxrFlt1->flt2() = 0;
    _cxrFlt2->marketingCarrier() = "DD";
    _cxrFlt2->operatingCarrier() = "";
    _cxrFlt2->flt1() = 18;
    _cxrFlt2->flt2() = 100;
    _cxrFlightT186.segs().push_back(_cxrFlt1);
    _cxrFlightT186.segs().push_back(_cxrFlt2);
  }

  const vector<OptionalServicesInfo*>& getOptionalServicesInfo(const SubCodeInfo& subCode) const
  {
    return _optSvcInfos;
  }

  const vector<SvcFeesCurrencyInfo*>&
  getSvcFeesCurrency(const OptionalServicesInfo& optSrvInfo) const
  {
    return _svcFeeCurInfos;
  }

  bool svcFeesAccountCodeValidate(const SvcFeesAccountCodeValidator& validator, int itemNo) const
  {
    return itemNo % 2;
  }

  bool svcFeesTktDesignatorValidate(const SvcFeesTktDesigValidator& validator, int itemNo) const
  {
    return itemNo % 2;
  }

  bool
  isInLoc(const VendorCode& vendor, const LocKey& locKey, const Loc& loc, CarrierCode carrier) const
  {
    if (vendor == SITA_VENDOR_CODE)
    {
      if (locKey.locType() == LOCTYPE_NATION)
        return locKey.loc() == loc.nation();
      else
        return locKey.loc() == loc.loc();
    }
    else
    {
      return !isdigit(loc.loc()[0]) && !isdigit(locKey.loc()[0]);
    }
  }

  bool
  isInZone(const VendorCode& vendor, const LocCode& zone, const Loc& loc, CarrierCode carrier) const
  {
    return isdigit(loc.loc()[0]);
  }

  virtual bool
  inputPtcValidate(const ServiceFeeUtil& util, const OptionalServicesInfo& optSrvInfo) const
  {
    return optSrvInfo.psgType().empty();
  }

  const vector<SvcFeesResBkgDesigInfo*>& getRBDInfo(const VendorCode& vendor, int itemNo) const
  {
    return _rbdInfos;
  }

  const std::vector<SvcFeesCxrResultingFCLInfo*>&
  getResFCInfo(const VendorCode& vendor, int itemNo) const
  {
    return _svcResFCInfos;
  }

  const TaxCarrierFlightInfo* getTaxCarrierFlight(const VendorCode& vendor, uint32_t itemNo) const
  {
    return &_cxrFlightT186;
  }

  const Loc* getLocation(const LocCode& locCode, const DateTime& refTime) const { return &_loc; }

  bool
  getUTCOffsetDifference(const Loc& loc1, const Loc& loc2, short& utcoffset, const DateTime& time)
      const
  {
    utcoffset = 0;
    if (tempFlag)
      utcoffset = 6;
    return true;
  }

  bool getFeeRounding(const CurrencyCode& currencyCode,
                      RoundingFactor& roundingFactor,
                      CurrencyNoDec& roundingNoDec,
                      RoundingRule& roundingRule) const
  {
    return false;
  }

  vector<OptionalServicesInfo*> _optSvcInfos;
  vector<SvcFeesCurrencyInfo*> _svcFeeCurInfos;
  vector<SvcFeesResBkgDesigInfo*> _rbdInfos;
  SvcFeesResBkgDesigInfo _rbdInfo1;
  SvcFeesResBkgDesigInfo _rbdInfo2;
  SvcFeesCurrencyInfo _currInfo;
  std::vector<SvcFeesCxrResultingFCLInfo*> _svcResFCInfos;
  SvcFeesCxrResultingFCLInfo _resFCInfo1;
  SvcFeesCxrResultingFCLInfo _resFCInfo2;
  TaxCarrierFlightInfo _cxrFlightT186;
  bool tempFlag;
  CarrierFlightSeg* _cxrFlt1;
  CarrierFlightSeg* _cxrFlt2;
  Loc _loc;
};

class LocKeyMock : public LocKey
{
public:
  LocKeyMock(const LocCode& locCode, LocTypeCode locTypeCode = LOCTYPE_CITY) : LocKey()
  {
    loc() = locCode;
    locType() = locTypeCode;
  }
};

class LocMock : public Loc
{
public:
  LocMock(const LocCode& loc) : Loc() { _loc = loc; }
};
}

SvcFeesResBkgDesigInfo*
getPadis(TestMemHandle& _memHandle,
         const VendorCode& vendor,
         int itemNo,
         int seqNo,
         const CarrierCode& carrier,
         const BookingCode& bookingCode1 = "",
         const BookingCode& bookingCode2 = "",
         const BookingCode& bookingCode3 = "",
         const BookingCode& bookingCode4 = "",
         const BookingCode& bookingCode5 = "")
{
  SvcFeesResBkgDesigInfo* padis = _memHandle.create<SvcFeesResBkgDesigInfo>();

  padis->vendor() = vendor;
  padis->itemNo() = itemNo;
  padis->carrier() = carrier;
  padis->bookingCode1() = bookingCode1;
  padis->bookingCode2() = bookingCode2;
  padis->bookingCode3() = bookingCode3;
  padis->bookingCode4() = bookingCode4;
  padis->bookingCode5() = bookingCode5;

  return padis;
}

class OptionalServicesValidatorDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;
  std::map<std::pair<VendorCode, int>, std::vector<SvcFeesResBkgDesigInfo*> > padisMap;

public:
  OptionalServicesValidatorDataHandleMock()
  {
    padisMap[std::make_pair(VendorCode("ATP"), 10)].push_back(
        getPadis(_memHandle, "ATP", 10, 1, "LH", "A", "B", "C", "D", "E"));

    padisMap[std::make_pair(VendorCode("ATP"), 22)].push_back(
        getPadis(_memHandle, "ATP", 22, 1, "LH", "A", "B", "C", "D", "E"));
    padisMap[std::make_pair(VendorCode("ATP"), 22)].push_back(
        getPadis(_memHandle, "ATP", 22, 2, "LH", "A", "C", "", "", ""));

    padisMap[std::make_pair(VendorCode("MMGR"), 11)].push_back(
        getPadis(_memHandle, "MMGR", 11, 1, "LH", "A", "B", "C", "D", "E"));

    padisMap[std::make_pair(VendorCode("MMGR"), 70)].push_back(
        getPadis(_memHandle, "MMGR", 70, 1, "LH", "A", "B", "C", "D", "E"));
    padisMap[std::make_pair(VendorCode("MMGR"), 70)].push_back(
        getPadis(_memHandle, "MMGR", 70, 2, "LH", "A", "B", "C", "D", "E"));
  }

  ~OptionalServicesValidatorDataHandleMock() { _memHandle.clear(); }

  const std::vector<SvcFeesResBkgDesigInfo*>&
  getSvcFeesResBkgDesig(const VendorCode& vendor, const int itemNo)
  {
    return padisMap[std::make_pair(vendor, itemNo)];
  }
};

class OptionalServicesValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalServicesValidatorTest);
  CPPUNIT_TEST(testValidate_Pass);
  CPPUNIT_TEST(testValidate_Fail);
  CPPUNIT_TEST(testValidateS7Data_Pass);
  CPPUNIT_TEST(testValidateS7Data_Fail_Sequence);
  CPPUNIT_TEST(testValidateS7Data_Fail_TravelDate);
  CPPUNIT_TEST(testValidateS7Data_Fail_InputTravelDate);
  CPPUNIT_TEST(testValidateS7Data_Fail_InputPtc);
  CPPUNIT_TEST(testValidateS7Data_Fail_FreqFlyer);
  CPPUNIT_TEST(testValidateS7Data_Fail_AccountCodes);
  CPPUNIT_TEST(testValidateS7Data_Fail_InterlineInd_N);
  CPPUNIT_TEST(testValidateS7Data_Fail_InterlineInd_Y);
  CPPUNIT_TEST(testValidateS7Data_Fail_InputTicketDesignator);
  CPPUNIT_TEST(testValidateS7Data_Fail_Security);
  CPPUNIT_TEST(testValidateS7Data_Fail_SectorPortion);
  CPPUNIT_TEST(testValidateS7Data_Fail_FtwInd);
  CPPUNIT_TEST(testValidateS7Data_Fail_IntermediatePoint);
  CPPUNIT_TEST(testValidateS7Data_Fail_StopCnxDest);
  CPPUNIT_TEST(testValidateS7Data_Fail_Cabin);
  CPPUNIT_TEST(testValidateS7Data_Fail_Rbd);
  CPPUNIT_TEST(testValidateS7Data_Fail_ResultingFare);
  CPPUNIT_TEST(testValidateS7Data_Fail_OutputTicketDesignator);
  CPPUNIT_TEST(testValidateS7Data_Fail_RuleTariffInd);
  CPPUNIT_TEST(testValidateS7Data_Fail_RuleTariff);
  CPPUNIT_TEST(testValidateS7Data_Fail_Rule);
  CPPUNIT_TEST(testValidateS7Data_Fail_FareInd);
  CPPUNIT_TEST(testValidateS7Data_Fail_TourCode);
  CPPUNIT_TEST(testValidateS7Data_Fail_Start_Stop_Time);
  CPPUNIT_TEST(testValidateS7Data_Fail_Start_Stop_Time_DOW);
  CPPUNIT_TEST(testValidateS7Data_Fail_Dow);
  CPPUNIT_TEST(testValidateS7Data_Fail_Equipment);
  CPPUNIT_TEST(testValidateS7Data_Fail_Mileage);
  CPPUNIT_TEST(testValidateS7Data_Fail_CarrierFlightApplT186);
  CPPUNIT_TEST(testValidateS7Data_Fail_AvdPur);
  CPPUNIT_TEST(testValidateS7Data_Fail_Sfc);

  CPPUNIT_TEST(testDisplaySvcFeeCurInfoHeader);
  CPPUNIT_TEST(testDisplaySvcFeeCurInfoHeader_NoDDInfo);
  CPPUNIT_TEST(testDisplaySvcFeeCurInfoDetail);
  CPPUNIT_TEST(testDisplaySvcFeeCurInfoDetail_NoDDInfo);
  CPPUNIT_TEST(testDisplaySvcFeeCurInfoHeaderAndDetail);
  CPPUNIT_TEST(testDisplaySvcFeeCurInfoHeaderAndDetail_NoDDInfo);
  CPPUNIT_TEST(testCheckDiagS7ForDetail);
  CPPUNIT_TEST(testCheckDiagS7ForDetail_NoDDInfo);
  CPPUNIT_TEST(testPrintDiagS7NotFound);

  CPPUNIT_TEST(testPrintDiagS7Info_DDInfo);
  CPPUNIT_TEST(testPrintDiagS7Info_NotDDInfo);

  CPPUNIT_TEST(testPrintDiagS7Info_DDPassed);
  CPPUNIT_TEST(testPrintDiagS7Info_DDPassed_Status_Fail);
  CPPUNIT_TEST(testPrintDiagS7Info_NotDDPassed);

  CPPUNIT_TEST(test_isDDPass_Pass);
  CPPUNIT_TEST(test_isDDPass_Fail);
  CPPUNIT_TEST(test_Validate_DDPass_False_Datanotfound);
  CPPUNIT_TEST(test_Validate_DDPass_True_Datanotfound);

  CPPUNIT_TEST(testCheckTravelDateWhenFailNoData);
  CPPUNIT_TEST(testCheckTravelDateWhenPassEffDisc);
  CPPUNIT_TEST(testCheckTravelDateWhenSameDateNoTimeOnEffDisc_Pass);
  CPPUNIT_TEST(testCheckTravelDateWhenDiffDateNoTimeOnEffDisc_Pass);
  CPPUNIT_TEST(testCheckTravelDateWhenPassTvlData);
  CPPUNIT_TEST(testCheckTravelDateWhenFailTvlStart);
  CPPUNIT_TEST(testCheckTravelDateWhenFailTvlStop);
  CPPUNIT_TEST(testCheckTravelDateWhenFailMonthZero);

  CPPUNIT_TEST(testCheckInputTravelDate_Pass);
  CPPUNIT_TEST(testCheckInputTravelDate_Fail_Create);
  CPPUNIT_TEST(testCheckInputTravelDate_Fail_Expire);
  CPPUNIT_TEST(testCheckInputTravelDate_Fail_Disc);
  CPPUNIT_TEST(testCheckInputTravelDate_Fail_Eff);

  CPPUNIT_TEST(testCheckInputPtc_Pass);
  CPPUNIT_TEST(testCheckInputPtc_Fail);

  CPPUNIT_TEST(testCheckAccountCodes_Pass_Zero);
  CPPUNIT_TEST(testCheckAccountCodes_Pass);
  CPPUNIT_TEST(testCheckAccountCodes_Fail);

  CPPUNIT_TEST(testCheckInputTicketDesignator_Pass_Zero);
  CPPUNIT_TEST(testCheckInputTicketDesignator_Pass);
  CPPUNIT_TEST(testCheckInputTicketDesignator_Fail);

  CPPUNIT_TEST(testCheckOutputTicketDesignator_Pass_Zero);
  CPPUNIT_TEST(testCheckOutputTicketDesignator_Fail_T171WithRuleTariffAndMultiCxr);
  CPPUNIT_TEST(testCheckOutputTicketDesignator_Fail_T171WithFareIndAndMultiCxr);
  CPPUNIT_TEST(testCheckOutputTicketDesignator_Pass);
  CPPUNIT_TEST(testCheckOutputTicketDesignator_Fail);

  CPPUNIT_TEST(testCheckGeoFtwInd_Pass_From);
  CPPUNIT_TEST(testCheckGeoFtwInd_Pass_To);
  CPPUNIT_TEST(testCheckGeoFtwInd_Pass_Between);
  CPPUNIT_TEST(testCheckGeoFtwInd_Pass_Between2);
  CPPUNIT_TEST(testCheckGeoFtwInd_Pass_Between_Empty);
  CPPUNIT_TEST(testCheckGeoFtwInd_Pass_Within);
  CPPUNIT_TEST(testCheckGeoFtwInd_Fail_RuleBusterA);
  CPPUNIT_TEST(testCheckGeoFtwInd_Fail_RuleBusterB);
  CPPUNIT_TEST(testCheckGeoFtwInd_Fail_Other);

  CPPUNIT_TEST(testValidateLocation_Pass_Empty);
  CPPUNIT_TEST(testValidateLocation_Fail_Empty);
  CPPUNIT_TEST(testValidateLocation_Pass_Zone);
  CPPUNIT_TEST(testValidateLocation_Fail_Zone);
  CPPUNIT_TEST(testValidateLocation_Pass_Loc);
  CPPUNIT_TEST(testValidateLocation_Fail_Loc);

  CPPUNIT_TEST(testValidateWithin_Fail_Orig);
  CPPUNIT_TEST(testValidateWithin_Fail_Dest);
  CPPUNIT_TEST(testValidateWithin_Pass);

  CPPUNIT_TEST(testCheckIntermediatePoint_Pass_ViaNull);
  CPPUNIT_TEST(testCheckIntermediatePoint_SameAS_Loc1_Fail);
  CPPUNIT_TEST(testCheckIntermediatePoint_SameAS_Loc2_Fail);
  CPPUNIT_TEST(testCheckIntermediatePoint_Pass_Loc2);
  CPPUNIT_TEST(testCheckIntermediatePoint_Pass_Stopover);
  CPPUNIT_TEST(testCheckIntermediatePoint_Fail);

  CPPUNIT_TEST(testIsStopover_Pass_Blank);
  CPPUNIT_TEST(testIsStopover_Fail_Blank);
  CPPUNIT_TEST(testIsStopover_Pass_SegArunk);
  CPPUNIT_TEST(testIsStopover_Fail_SegArunk);
  CPPUNIT_TEST(testIsStopover_Pass_NextArunk);
  CPPUNIT_TEST(testIsStopover_Fail_NextArunk);
  CPPUNIT_TEST(testIsStopover_Pass);
  CPPUNIT_TEST(testIsStopover_Pass_When_D_Blank);
  CPPUNIT_TEST(testIsStopover_Pass_When_D_Value_Fail);
  CPPUNIT_TEST(testIsStopover_Pass_When_D_Value_Pass);
  CPPUNIT_TEST(testIsStopover_Pass_When_M_Blank);
  CPPUNIT_TEST(testIsStopover_Pass_When_M_Value_Fail);
  CPPUNIT_TEST(testIsStopover_Pass_When_M_Value_Pass);
  CPPUNIT_TEST(testIsStopover_Fail);

  CPPUNIT_TEST(testCheckStopCnxDestInd_Fail_Cnx);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Fail_FareBreak);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Fail_FareBreakOrStopover_FareBreak);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Fail_FareBreakOrStopover_Stopover);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Fail_Stopover);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Fail_StopoverWithGeo);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Pass_OneSegmentPortion);
  CPPUNIT_TEST(testCheckStopCnxDestInd_Pass);

  CPPUNIT_TEST(testValidateCnxOrStopover_Fail_Sector);
  CPPUNIT_TEST(testValidateCnxOrStopover_Fail_Stopover);
  CPPUNIT_TEST(testValidateCnxOrStopover_Fail_StopoverPassedLoc3);
  CPPUNIT_TEST(testValidateCnxOrStopover_Pass);

  CPPUNIT_TEST(testValidateNoFareBreak_Fail_Sector);
  CPPUNIT_TEST(testValidateNoFareBreak_Fail_FareBreak);
  CPPUNIT_TEST(testValidateNoFareBreak_Pass_FareBreakPassedLoc3);
  CPPUNIT_TEST(testValidateNoFareBreak_Pass_FareBreakSideTrip);
  CPPUNIT_TEST(testValidateNoFareBreak_Pass);

  CPPUNIT_TEST(testValidateViaWithLoc2_Pass);
  CPPUNIT_TEST(testValidateViaWithLoc2_Fail_Via);

  CPPUNIT_TEST(testValidateViaWithStopover_Pass);
  CPPUNIT_TEST(testValidateViaWithStopover_Fail_Via);
  CPPUNIT_TEST(testValidateViaWithStopover_Fail_Stopover);

  CPPUNIT_TEST(testFindFirstStopover_Pass);
  CPPUNIT_TEST(testFindFirstStopover_Fail);

  CPPUNIT_TEST(testCheckSecurityWhenT183ItemIsZero);
  CPPUNIT_TEST(testCheckSecurityWhenT183ItemIsZeroAndPrivateIndicatorON);

  CPPUNIT_TEST(testRetrieveSpecifiedFee_Pass_Zero);
  CPPUNIT_TEST(testRetrieveSpecifiedFee_Fail_Empty);
  CPPUNIT_TEST(testRetrieveSpecifiedFee_Pass);

  CPPUNIT_TEST(testvalidateSfcLocation_Fail);
  CPPUNIT_TEST(testvalidateSfcLocation_Pass);

  CPPUNIT_TEST(testIsRBDValidWhenEmptyRBDInfo);
  CPPUNIT_TEST(testIsRBDValidWhenExistValidRBD);
  CPPUNIT_TEST(testIsRBDValidWhenOperPhaseAndDifftMktAndOperCxr);
  CPPUNIT_TEST(testIsRBDValidWhenMktPhaseAndDifftMktAndOperCxr);
  CPPUNIT_TEST(testIsRBDValidWhenDifftRBDCxr);
  CPPUNIT_TEST(testIsRBDValidWhenNoExistValidRBD);
  CPPUNIT_TEST(testIsRBDValidWhenExistValidRBDForRebook);
  CPPUNIT_TEST(testIsRBDValidWhenNoExistValidRBDForRebook);
  CPPUNIT_TEST(testCheckRBDWhenEmptyItemNo);
  CPPUNIT_TEST(test_skipUpgradeCheck_Match);
  CPPUNIT_TEST(test_skipUpgradeCheck_NoMatch1);
  CPPUNIT_TEST(test_skipUpgradeCheck_NoMatch2);
  CPPUNIT_TEST(test_skipUpgradeCheck_NoMatchButUPGroupForcesMatch);
  CPPUNIT_TEST(testCheckRBDWhenFailOnSectorPortionCheck);
  CPPUNIT_TEST(testCheckRBDWhenMktPhaseAndNoValidRecordsExists);
  CPPUNIT_TEST(testCheckRBDWhenMktPhaseAndNoExistsRecordsForSpecifiedCxr);
  CPPUNIT_TEST(testCheckRBDWhenMktPhaseAndNoExistsValidRBDs);
  CPPUNIT_TEST(testCheckRBDWhenMktPhaseAndExistsValidRBDs);
  CPPUNIT_TEST(testCheckRBDWhenOperPhaseAndUsedMktRecord);
  CPPUNIT_TEST(testCheckRBDWhenOperPhaseAndDifferentCarriers);
  CPPUNIT_TEST(testCheckRBDWhenOperPhaseAndNoExistsRecordsForSpecifiedCxr);
  CPPUNIT_TEST(testCheckRBDWhenOperPhaseAndNoExistsValidRBDs);
  CPPUNIT_TEST(testCheckRBDWhenOperPhaseAndExistsValidRBDs);

  CPPUNIT_TEST(testMapS7CabinTypeWhenPremiumFirstNew);
  CPPUNIT_TEST(testMapS7CabinTypeWhenPremiumEconomyNew);

  CPPUNIT_TEST(testMapS7CabinTypeWhenPremiumFirst);
  CPPUNIT_TEST(testMapS7CabinTypeWhenFirst);
  CPPUNIT_TEST(testMapS7CabinTypeWhenPremiumEconomy);
  CPPUNIT_TEST(testOptionalServicesValidatorWhen2AirSegs);
  CPPUNIT_TEST(testCheckCabinWhenS7CabinIsBlank);
  CPPUNIT_TEST(testCheckCabinWhenSegHasDifferentMarketCxrThenS7Record);
  CPPUNIT_TEST(testCheckCabinWhenSegHasDifferentOperCxrThenS7Record);
  CPPUNIT_TEST(testCheckCabinWhenAllSegmentsHaveSameMarketCxrAndSameCabinAsS7);
  CPPUNIT_TEST(testCheckCabinWhenAllSegmentsHaveSameOperCxrAndSameCabinAsS7);
  CPPUNIT_TEST(testCheckCabinWhenOneOfSegmentsHasSameMarketCxrAndDifferentCabinThenS7);
  CPPUNIT_TEST(testCheckCabinWhenOneOfSegmentsHasSameOperCxrAndDifferentCabinThenS7);

  CPPUNIT_TEST(testCheckFrequentFlyerStatus_noFFStat);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_lowerStatus_matchS7CXR);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_sameStatus_matchS7CXR);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_higherStatus_matchS7CXR);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_lowerStatus_noMatchS7CXR);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_sameStatus_noMatchS7CXR);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_higherStatus_noMatchS7CXR);

  CPPUNIT_TEST(testSetSrvInfo_vendor_ATP_Fee_IS_Guarantee);
  CPPUNIT_TEST(testSetSrvInfo_vendor_MMGR_Fee_NOT_Guarantee);

  CPPUNIT_TEST(testCheckEquipment_empty_MATCH);
  CPPUNIT_TEST(testCheckEquipment_BLANK_MATCH);
  CPPUNIT_TEST(testCheckEquipment_NotFLIGHT_NOMATCH);
  CPPUNIT_TEST(testCheckEquipment_PortionSameAsS7_MATCH);
  CPPUNIT_TEST(testCheckEquipment_PortionSomeNotSameAsS7_NoMATCH);

  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_BLANK_APPLY_SERVICE_FEE);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_X_SERVICE_NOT_AVAILABLE);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_F_SERVICE_FREE_NO_EMD_ISSUED);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_E_SERVICE_FREE_EMD_ISSUED);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_G_SERVICE_FREE_NO_BOOK_NO_EMD);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_H_SERVICE_FREE_NO_BOOK_EMD_ISSUED);

  CPPUNIT_TEST(testCheckResultingFareClassWhenEmptyItemNo);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenFailsOnCarrier);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenFailsOnFareClass);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenPassOnFareClassOverridedBySpecifiedFareClass);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenFailsOnFareType);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenPass);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenEmptyFareClass);
  CPPUNIT_TEST(testIsValidResultingFareClassWhenEmptyFareType);
  CPPUNIT_TEST(testCheckResultingFareClassWhenEmptyTS2SS);
  CPPUNIT_TEST(testCheckResultingFareClassWhenPassForOneFare);
  CPPUNIT_TEST(testCheckResultingFareClassWhenFailForOneFare);
  CPPUNIT_TEST(testCheckResultingFareClassWhenPassForTwoFares);
  CPPUNIT_TEST(testCheckResultingFareClassWhenFailForTwoFares);
  CPPUNIT_TEST(testIsValidFareClassFareTypeWhenPassOnFareClassOverridedBySpecifiedFareClass);

  CPPUNIT_TEST(test_checkStartStopTime_Empty);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsT_False);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsD_True_NoData);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsR_True_NoData);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsD_False_NoData);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsR_False_NoData);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsD_True_Data);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsR_True_Data);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsD_False_Data);
  CPPUNIT_TEST(test_checkStartStopTime_TAIsR_False_Data);
  CPPUNIT_TEST(test_convertMinutesSinceMidnightToActualTime_Empty);
  CPPUNIT_TEST(test_convertMinutesSinceMidnightToActualTime_Value);
  CPPUNIT_TEST(test_validateStartAndStopTime_False);
  CPPUNIT_TEST(test_validateStartAndStopTime_True);
  CPPUNIT_TEST(test_validateStartAndStopTime_True_Dup);
  CPPUNIT_TEST(test_validateStartAndStopTime_True_ExactTime);
  CPPUNIT_TEST(test_validateDOWAndStartAndStopTime_TAIsD_True);
  CPPUNIT_TEST(test_validateDOWAndStartAndStopTime_TAIsR_True);
  CPPUNIT_TEST(test_validateDOWAndStartAndStopTime_TAIsR_False);
  CPPUNIT_TEST(test_checkDOWRangeAndTime_False);
  CPPUNIT_TEST(test_checkDOWRangeAndTime_True);
  CPPUNIT_TEST(test_checkDOWRangeAndTime2_True);
  CPPUNIT_TEST(test_checkDOWRangeAndTime_RangeSingleValue_True);
  CPPUNIT_TEST(test_checkDOWRangeAndTime_RangeMultiValue_With_Space_True);
  CPPUNIT_TEST(test_checkDOWRange_False);
  CPPUNIT_TEST(test_checkDOWRange_True);
  CPPUNIT_TEST(test_checkDOWRange2_True);
  CPPUNIT_TEST(test_checkDOWRange_RangeSingleValue_Fail);
  CPPUNIT_TEST(test_checkDOWRange_RangeMultiValue_With_Space_True);
  CPPUNIT_TEST(test_displayStartStopTimeOrDOWFailDetail_False);
  CPPUNIT_TEST(test_displayStartStopTimeOrDOWFailDetail_True);

  CPPUNIT_TEST(testcheckDOW_empty_MATCH);
  CPPUNIT_TEST(testcheckDOW_BLANK_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISBLANK_NOMATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_NOMATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_MONDAY_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_TUESDAY_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_WEDNESDAY_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_THURSDAY_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_FRIDAY_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_SATURDAY_MATCH);
  CPPUNIT_TEST(testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_SUNDAY_MATCH);

  CPPUNIT_TEST(testcheckTourCode_empty_MATCH);
  CPPUNIT_TEST(testcheckTourCode_BLANK_MATCH);
  CPPUNIT_TEST(testcheckTourCode_WhenInS7AndNoneInCat35orCat27_NOMATCH);
  CPPUNIT_TEST(testcheckTourCode_NOMATCH_WhenDiffS7AndCat35);
  CPPUNIT_TEST(testcheckTourCode_MATCH_WhenSameS7AndCat35);
  CPPUNIT_TEST(testcheckTourCode_NOMATCH_WhenDiffS7AndCat35Blank);
  CPPUNIT_TEST(testcheckTourCode_NOMATCH_WhenDiffS7AndCat27Blank);
  CPPUNIT_TEST(testcheckTourCode_NOMATCH_WhenDiffS7AndCat27);
  CPPUNIT_TEST(testcheckTourCode_MATCH_WhenSameS7AndCat27);
  CPPUNIT_TEST(testcheckTourCode_MATCH_WhenDiffS7AndCat35SameCat27);
  CPPUNIT_TEST(testcheckTourCode_MATCH_WhenSameS7AndCat35DiffCat27);
  CPPUNIT_TEST(testcheckTourCode_NOMATCH_WhenDiffS7AndCat35DiffCat27);

  CPPUNIT_TEST(testcheckAdvPur_empty_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_BLANK_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Minute_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Minute_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_Hour_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Hour_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_Day_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Exact_Day_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Day_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_Month_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Exact_Month_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_Month_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_01_MON_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_01_FRI_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_01_TUE_SAMEDAY_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_01_FRI_SAMEDEP_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_10_MON_MATCH);
  CPPUNIT_TEST(testcheckAdvPur_10_FRI_NOMATCH);
  CPPUNIT_TEST(testcheckAdvPur_10_FRI_SAMEDEP_NOMATCH);
  CPPUNIT_TEST(test_isAdvPurUnitNHDM_ValueN);
  CPPUNIT_TEST(test_isAdvPurUnitNHDM_ValueH);
  CPPUNIT_TEST(test_isAdvPurUnitNHDM_ValueD);
  CPPUNIT_TEST(test_isAdvPurUnitNHDM_ValueM);
  CPPUNIT_TEST(test_isAdvPurUnitNHDM_Value20Tue);
  CPPUNIT_TEST(test_getDayOfWeek_SUN);
  CPPUNIT_TEST(test_getDayOfWeek_MON);
  CPPUNIT_TEST(test_getDayOfWeek_TUE);
  CPPUNIT_TEST(test_getDayOfWeek_WED);
  CPPUNIT_TEST(test_getDayOfWeek_THU);
  CPPUNIT_TEST(test_getDayOfWeek_FRI);
  CPPUNIT_TEST(test_getDayOfWeek_SAT);
  CPPUNIT_TEST(test_getDayOfWeek_ABC);
  CPPUNIT_TEST(test_getDayOfWeek_WED_NOMATCH);
  CPPUNIT_TEST(test_getAdvPurPeriod_TUE_MON);
  CPPUNIT_TEST(test_getAdvPurPeriod_WED_FRI);
  CPPUNIT_TEST(test_getAdvPurPeriod_WED_WED);
  CPPUNIT_TEST(test_getLocalTime_MATCH);
  CPPUNIT_TEST(test_getLocalTime_NOMATCH);
  CPPUNIT_TEST(test_getTimeDiff_Zero);
  CPPUNIT_TEST(test_getTimeDiff_Not_Zero);

  CPPUNIT_TEST(testCheckMileageFee_All_Empty_PASS);
  CPPUNIT_TEST(testCheckMileageFee_AND_BLANK_T170_Present_PASS);
  CPPUNIT_TEST(testCheckMileageFee_AND_BLANK_T170_Present_Mileage_Present_PASS);
  CPPUNIT_TEST(testCheckMileageFee_AND_BLANK_T170_NotPresent_Mileage_Present_FAIL);
  CPPUNIT_TEST(testCheckMileageFee_AND_A_T170_NotPresent_Mileage_Present_FAIL);
  CPPUNIT_TEST(testCheckMileageFee_AND_A_T170_Present_Mileage_Present_FAIL);
  CPPUNIT_TEST(testCheckMileageFee_AND_A_T170_Present_NO_Mileage_Present_PASS);
  CPPUNIT_TEST(testCheckMileageFee_AND_A_T170_NOT_Present_NO_Mileage_Present_PASS);

  CPPUNIT_TEST(testCheckRuleTariffIndWhenNoRestriction);
  CPPUNIT_TEST(testCheckRuleTariffIndWhenPassForPublic);
  CPPUNIT_TEST(testCheckRuleTariffIndWhenFailForPublic);
  CPPUNIT_TEST(testCheckRuleTariffIndWhenPassForPrivate);
  CPPUNIT_TEST(testCheckRuleTariffIndWhenFailForPrivate);
  CPPUNIT_TEST(testCheckRuleTariffWhenSpecifiedTariff);
  CPPUNIT_TEST(testCheckRuleTariffWhenSpecifiedTariffForR1);
  CPPUNIT_TEST(testCheckRuleTariffWhenFailForTwoFaresWhenSpecifiedTariff);
  CPPUNIT_TEST(test_CheckRuleTariff_Match);
  CPPUNIT_TEST(testCheckRuleTariffWhenPassForTwoFaresWhenSpecifiedTariff);

  CPPUNIT_TEST(testCheckRule_Pass_Empty);
  CPPUNIT_TEST(testCheckRule_Fail_NoMatch);
  CPPUNIT_TEST(testCheckRule_Pass_Match);

  CPPUNIT_TEST(testCheckFareInd_Pass_Blank);
  CPPUNIT_TEST(testCheckFareInd_Pass_Blank2);
  CPPUNIT_TEST(testCheckFareInd_Fail_19_22);
  CPPUNIT_TEST(testCheckFareInd_Fail_25);
  CPPUNIT_TEST(testCheckFareInd_Fail_35);
  CPPUNIT_TEST(testCheckFareInd_Fail_Other);
  CPPUNIT_TEST(testCheckFareInd_Pass_19);
  CPPUNIT_TEST(testCheckFareInd_Fail_20);
  CPPUNIT_TEST(testCheckFareInd_Fail_21);
  CPPUNIT_TEST(testCheckFareInd_Pass_22);
  CPPUNIT_TEST(testCheckFareInd_Pass_25);
  CPPUNIT_TEST(testCheckFareInd_Pass_35);

  CPPUNIT_TEST(testCheckIsValidCarrierFlightT186_MarketingCXR_Present_Any_Flight_PASS_FAIL);
  CPPUNIT_TEST(testCheckIsValidCarrierFlightT186_OperatingCXR_Present_Any_Flight_PASS_FAIL);
  CPPUNIT_TEST(testCheckIsValidCarrierFlightT186_MarketingCXR_Pass_Flight2_Zero_Flight1_FAIL_PASS);
  CPPUNIT_TEST(
      testCheckIsValidCarrierFlightT186_MarketingOperatingCXR_Pass_Flight2_Zero_Flight1_FAIL_PASS);
  CPPUNIT_TEST(testCheckIsValidCarrierFlightT186_MarketingOperatingCXR_Pass_Flight_Range_FAIL_PASS);

  CPPUNIT_TEST(testCheckCarrierFlightApplT186_NoTable_Return_True);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Table_Exists_No_SegCount_Return_False);
  CPPUNIT_TEST(
      testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_NoCxrFltSegment_Return_False);
  CPPUNIT_TEST(
      testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_MarketCxr_NotMatch_Return_False);
  CPPUNIT_TEST(
      testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_OperatCxr_NotMatch_Return_False);
  CPPUNIT_TEST(
      testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_MatchForSeg1_NoMatchForSeg2_Return_False);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_BothSegs_Match_Return_True);
  CPPUNIT_TEST(
      testCheckCarrierFlightApplT186_Table_Exists_Two_SegCount_1stMatch1st_2ndMatch2nd_Return_True);
  CPPUNIT_TEST(
      testCheckCarrierFlightApplT186_Table_Exists_Two_SegCount_1stMatch1st_2ndNotMatchFlt_Return_False);

  CPPUNIT_TEST(testCheckFeeApplication_Pass_Blank);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_Blank2);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_1_OW);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_2_RT);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_3_Item);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_4_Travel);
  CPPUNIT_TEST(testCheckFeeApplication_Pass_5_Yicket);
  CPPUNIT_TEST(testCheckFeeApplication_Fail_Unknown);

  CPPUNIT_TEST(testValidateS7Data_Fail_Collect_Subtract);
  CPPUNIT_TEST(testValidateS7Data_Pass_Collect_Subtract);
  CPPUNIT_TEST(testValidateS7Data_Fail_Net_Sell);
  CPPUNIT_TEST(testValidateS7Data_Pass_Net_Sell);
  CPPUNIT_TEST(test_setPBDate_True);

  CPPUNIT_TEST(testPrintStopAtFirstMatchMsg);

  CPPUNIT_TEST(testIsDOWstringValid_1234_Pass);
  CPPUNIT_TEST(testIsDOWstringValid_67123_Pass);
  CPPUNIT_TEST(testIsDOWstringValid_1567_Fail);
  CPPUNIT_TEST(testIsDOWstringValid_156_Fail);
  CPPUNIT_TEST(testIsDOWstringValid_672_Fail);
  CPPUNIT_TEST(testIsDOWstringValid_2356_Fail);
  CPPUNIT_TEST(testIsDOWstringValid_17_Fail);
  CPPUNIT_TEST(testIsDOWstringValid_71234567_Fail);
  CPPUNIT_TEST(testIsDOWstringValid_1_Fail);

  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_NotTkt_Pass);
  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_AdvPurchY_Pass);
  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_Tkt_AdvPurchX_Fail);

  CPPUNIT_TEST(testValidate_Pass_PrePaid_Journey);
  CPPUNIT_TEST(testValidate_Pass_PrePaid_Portion);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_LocNotBlank);
  CPPUNIT_TEST(testValidate_Pass_PrePaid_Blank);
  CPPUNIT_TEST(testValidate_Pass_PrePaid_Blank_With_Zero_Zones);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_Zone1Exist);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_Zone2Exist);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_Zone3Exist);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_StopConnection);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_StopoverUnit);
  CPPUNIT_TEST(testValidate_Fail_PrePaid_Blank_Loc2Notempty);
  CPPUNIT_TEST(testValidate_Pass_PrePaid_CheckJourneyForBG);
  CPPUNIT_TEST(testValidate_Pass_PrePaid_CheckPortionForBG);
  CPPUNIT_TEST(testValidate_Pass_PrePaid_CheckSectorForBG);

  CPPUNIT_TEST(testShouldProcessAdvPur_Pass);
  CPPUNIT_TEST(testShouldProcessAdvPur_Fail);

  CPPUNIT_TEST(tastGetLocForSfcValidation);

  CPPUNIT_TEST(testValidateS7Data_Padis_T198SetToZero_upgradeCabinNotZero);
  CPPUNIT_TEST(testValidateS7Data_Padis_T198SetToZero_upgradeCabinSetToZero);
  CPPUNIT_TEST(testValidateS7Data_Padis_T198NotZero_upgradeCabinNotZero);
  CPPUNIT_TEST(testValidateS7Data_Padis_T198NotZero_upgradeCabinSetToZero);
  CPPUNIT_TEST(testValidate_Padis_StopValidatingIfT198NotSet);
  CPPUNIT_TEST(testValidate_Padis_T198NotFoundInDB);
  CPPUNIT_TEST(testValidate_Padis_severalSequencesT198);
  CPPUNIT_TEST(testValidate_Padis_severalSequencesT198_identicalData);
  CPPUNIT_TEST(test_Padis_comparator);
  CPPUNIT_TEST(testValidateViaWithLoc2_FailOnOrigin);
  CPPUNIT_TEST(testValidateViaWithLoc2_FailOnDestination);
  CPPUNIT_TEST(testValidateViaWithLoc2_PassDespiteS7ViaPointMatchOrigin);
  CPPUNIT_TEST(testValidateViaWithLoc2_PassDespiteS7ViaPointMatchDestination);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  AncRequest _ancRequest;
  OptionalServicesValidator* _optSrvValidator;
  OptionalServicesInfo* _optSvcInfo;
  vector<TravelSeg*>* _travelSegs;
  AirSeg* _seg1;
  ArunkSeg* _seg2Arunk;
  AirSeg* _seg3;
  ArunkSeg* _seg4Arunk;
  vector<TravelSeg*>::const_iterator _segI;
  vector<TravelSeg*>::const_iterator _segIE;
  Ts2ss* _ts2ss;
  OCFees* _ocFee;
  SubCodeInfo* _subCodeInfo;
  SvcFeesResBkgDesigInfo* _rbdInfo1;
  SvcFeesResBkgDesigInfo* _rbdInfo2;
  vector<SvcFeesResBkgDesigInfo*>* _validRBDInfoForSeg;
  PaxType* _paxType;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  PaxTypeFare* _ptf1;
  PaxTypeFare* _ptf2;
  FareMarket* _fm1;
  FareMarket* _fm2;
  Fare* _fare1;
  Fare* _fare2;
  FareInfo* _fareInfo1;
  FareInfo* _fareInfo2;
  FareClassAppInfo* _fareClassAppInfo1;
  FareClassAppInfo* _fareClassAppInfo2;
  TariffCrossRefInfo* _tariffCrossRefInfo1;
  TariffCrossRefInfo* _tariffCrossRefInfo2;
  PaxTypeFare::SegmentStatus* _segmentStatus;
  OptionalServicesValidatorDataHandleMock* _dataHandleMock;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    //   _ancRequest  = _memHandle.create<AncRequest&>();
    _farePath = _memHandle.create<FarePath>();
    _travelSegs = _memHandle.create<vector<TravelSeg*>>();
    _seg1 = _memHandle.create<AirSeg>();
    _seg2Arunk = _memHandle.create<ArunkSeg>();
    _seg3 = _memHandle.create<AirSeg>();
    _seg4Arunk = _memHandle.create<ArunkSeg>();
    _travelSegs->push_back(_seg1);
    _travelSegs->push_back(_seg2Arunk);
    _travelSegs->push_back(_seg3);
    _farePath->itin() = _memHandle.create<Itin>();
    _farePath->itin()->travelSeg() = *_travelSegs;
    _segI = _travelSegs->begin();
    _segIE = _travelSegs->end();
    _ts2ss = _memHandle.create<Ts2ss>();
    _optSvcInfo = _memHandle.create<OptionalServicesInfo>();

    _rbdInfo1 = _memHandle.create<SvcFeesResBkgDesigInfo>();
    _rbdInfo2 = _memHandle.create<SvcFeesResBkgDesigInfo>();
    _validRBDInfoForSeg = _memHandle.create<vector<SvcFeesResBkgDesigInfo*>>();
    _ptf1 = _memHandle.create<PaxTypeFare>();
    _ptf2 = _memHandle.create<PaxTypeFare>();
    _fm1 = _memHandle.create<FareMarket>();
    _fm2 = _memHandle.create<FareMarket>();
    _fare1 = _memHandle.create<Fare>();
    _fare2 = _memHandle.create<Fare>();
    _fareInfo1 = _memHandle.create<FareInfo>();
    _fareInfo2 = _memHandle.create<FareInfo>();
    _fareClassAppInfo1 = _memHandle.create<FareClassAppInfo>();
    _fareClassAppInfo2 = _memHandle.create<FareClassAppInfo>();
    _tariffCrossRefInfo1 = _memHandle.create<TariffCrossRefInfo>();
    _tariffCrossRefInfo2 = _memHandle.create<TariffCrossRefInfo>();
    _segmentStatus = _memHandle.create<PaxTypeFare::SegmentStatus>();
    _subCodeInfo = _memHandle.create<SubCodeInfo>();

    _optSvcInfo->createDate() = DateTime(2010, 1, 18);
    _optSvcInfo->effDate() = DateTime(2010, 1, 18);
    _optSvcInfo->ticketEffDate() = DateTime(2010, 1, 18);
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_PORTION;
    _optSvcInfo->stopoverUnit() = OptionalServicesValidator::CHAR_BLANK;
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::FTW_FROM;
    _optSvcInfo->loc1().loc() = LOC_NYC;
    _optSvcInfo->loc1().locType() = LOCTYPE_CITY;
    _optSvcInfo->expireDate() = DateTime(2010, 1, 19);
    _optSvcInfo->discDate() = DateTime(2010, 1, 19);
    _optSvcInfo->ticketDiscDate() = DateTime(2010, 1, 19);

    _seg1->departureDT() = DateTime(2010, 1, 18);
    _seg1->arrivalDT() = DateTime(2010, 1, 18, 3, 0, 0);
    _seg2Arunk->segmentType() = Arunk;
    _seg2Arunk->departureDT() = DateTime(2010, 1, 18, 3, 0, 0);
    _seg2Arunk->arrivalDT() = DateTime(2010, 1, 19);
    _seg3->departureDT() = DateTime(2010, 1, 19);
    _seg3->arrivalDT() = DateTime(2010, 1, 19, 3, 0, 0);
    _seg4Arunk->segmentType() = Arunk;
    _seg4Arunk->departureDT() = DateTime(2010, 1, 19, 3, 0, 0);
    _seg4Arunk->arrivalDT() = DateTime(2010, 1, 19, 6, 42, 0);

    Loc* origin = _memHandle.insert(new LocMock(LOC_WAS));
    origin->nation() = "US";
    _seg1->origin() = origin;

    Loc* destination = _memHandle.insert(new LocMock(LOC_NYC));
    destination->nation() = "US";
    _seg1->destination() = destination;

    _seg2Arunk->origin() = _seg1->destination();
    _seg2Arunk->destination() = _seg1->origin();
    _seg3->origin() = _seg1->origin();
    _seg3->destination() = _seg1->destination();
    _seg4Arunk->origin() = _seg1->destination();
    _seg4Arunk->destination() = _seg1->origin();

    _seg1->segmentOrder() = 1;
    _seg2Arunk->segmentOrder() = 2;
    _seg3->segmentOrder() = 3;
    _seg4Arunk->segmentOrder() = 4;

    _seg1->setBookingCode(BookingCode('C'));

    _ocFee = _memHandle.create<OCFees>();
    _subCodeInfo->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    _ocFee->subCodeInfo() = _subCodeInfo;

    _validRBDInfoForSeg->push_back(_rbdInfo1);
    _validRBDInfoForSeg->push_back(_rbdInfo2);
    _paxType = _memHandle.create<PaxType>();
    _farePath->paxType() = _paxType;

    _pricingUnit = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(_pricingUnit);

    _fareUsage = _memHandle.create<FareUsage>();
    _pricingUnit->fareUsage().push_back(_fareUsage);

    _fareUsage->travelSeg().push_back(_seg1);
    _fareUsage->travelSeg().push_back(_seg2Arunk);
    _fareUsage->travelSeg().push_back(_seg3);

    _ptf1->fareClassAppInfo() = _fareClassAppInfo1;

    _ptf2->fareClassAppInfo() = _fareClassAppInfo2;
    _fare1->setTariffCrossRefInfo(_tariffCrossRefInfo1);
    _fare2->setTariffCrossRefInfo(_tariffCrossRefInfo2);
    _fareInfo1->ruleNumber() = "RULE";
    _fareInfo2->ruleNumber() = "RULE";
    _fare1->setFareInfo(_fareInfo1);
    _ptf1->setFare(_fare1);
    _fare2->setFareInfo(_fareInfo2);
    _ptf2->setFare(_fare2);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->isTicketingInd() = true;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->ticketingDate() = DateTime(2010, 1, 18);

    _dataHandleMock = _memHandle.create<OptionalServicesValidatorDataHandleMock>();
    _optSrvValidator = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));


    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->requestPath() = UNKNOWN_PATH;

    const std::string activationDate = "2000-01-01";
    TestConfigInitializer::setValue("EMD_VALIDATION_FLIGHT_RELATED_SERVICE_AND_PREPAID_BAGGAGE",
                                    activationDate,
                                    "EMD_ACTIVATION");
  }

  void tearDown() { _memHandle.clear(); }

  void setupValidCaseForCabin()
  {
    _seg1->setOperatingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("AA");
    _seg1->setMarketingCarrierCode("AA");
    _seg3->setMarketingCarrierCode("AA");
    CabinType cabin1;
    cabin1.setFirstClass();
    CabinType cabin2;
    cabin2.setFirstClass();
    _seg1->bookedCabin() = cabin1;
    _seg3->bookedCabin() = cabin2;
  }

  void setupFareAndResultingFareClassInfo(PaxTypeFare& ptf,
                                          FareInfo& fareInfo,
                                          FareMarket& fm,
                                          FareClassAppInfo& fareClassAppInfo,
                                          SvcFeesCxrResultingFCLInfo& fclInfo)
  {
    ptf.fareMarket() = &fm;
    fm.governingCarrier() = "AA";
    fclInfo.carrier() = "AA";
    fareInfo.fareClass() = "Y1CH";
    fclInfo.resultingFCL() = "Y-CH";
    fareClassAppInfo._fareType = "EU";
    fclInfo.fareType() = "EU";
  }

  void createDiagnostic(bool isDDInfo)
  {
    _optSrvValidator->_diag = _memHandle.insert(new Diag877Collector(_trx->diagnostic()));
    _optSrvValidator->_diag->activate();

    if (isDDInfo)
      _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DISPLAY_DETAIL, "INFO"));
  }

  void createDiagnosticDDPassed(bool isDDPass)
  {
    _optSrvValidator->_diag =
        _memHandle.insert(new Diag877Collector(*_memHandle.create<Diagnostic>()));
    _optSrvValidator->_diag->activate();

    if (isDDPass)
      _trx->diagnostic().diagParamMap().insert(
          std::make_pair(Diagnostic::DISPLAY_DETAIL, "PASSED"));
  }

  PaxType::FreqFlyerTierWithCarrier* createFFData(uint16_t PNRstatus,
                                                  CarrierCode PNRcxr,
                                                  uint16_t S7status = 3,
                                                  CarrierCode S7cxr = "AA")
  {
    PaxType::FreqFlyerTierWithCarrier* ffd = _memHandle.create<PaxType::FreqFlyerTierWithCarrier>();
    ffd->setFreqFlyerTierLevel(PNRstatus);
    ffd->setCxr(PNRcxr);
    _paxType->freqFlyerTierWithCarrier().push_back(ffd);

    _optSvcInfo->frequentFlyerStatus() = S7status;
    _optSvcInfo->carrier() = S7cxr;

    return ffd;
  }

  void setupT186CntZero(TaxCarrierFlightInfo& cxrFlightT186) { cxrFlightT186.segCnt() = 0; }

  void setupT186OneSegment(TaxCarrierFlightInfo& cxrFlightT186) { cxrFlightT186.segCnt() = 1; }

  void setupT186TwoSegments(TaxCarrierFlightInfo& cxrFlightT186) { cxrFlightT186.segCnt() = 2; }

  void
  createOptSvcInfWthDate(int32_t sy, int32_t sm, int32_t sd, int32_t ey, int32_t em, int32_t ed)
  {
    _optSvcInfo->ticketEffDate() = DateTime::emptyDate();
    _optSvcInfo->ticketDiscDate() = DateTime::emptyDate();

    _optSvcInfo->tvlStartYear() = sy;
    _optSvcInfo->tvlStartMonth() = sm;
    _optSvcInfo->tvlStartDay() = sd;
    _optSvcInfo->tvlStopYear() = ey;
    _optSvcInfo->tvlStopMonth() = em;
    _optSvcInfo->tvlStopDay() = ed;
  }

  void createTsMap(bool useTwoFares = false)
  {
    _ts2ss->insert(std::make_pair(_seg1, std::make_pair(_segmentStatus, _ptf1)));
    _optSrvValidator->_processedFares.insert(_ptf1);
    if (useTwoFares)
    {
      _ts2ss->insert(std::make_pair(_seg2Arunk, std::make_pair(_segmentStatus, _ptf2)));
      _ts2ss->insert(std::make_pair(_seg3, std::make_pair(_segmentStatus, _ptf2)));
      _optSrvValidator->_processedFares.insert(_ptf2);
    }
    else
    {
      _ts2ss->insert(std::make_pair(_seg2Arunk, std::make_pair(_segmentStatus, _ptf1)));
      _ts2ss->insert(std::make_pair(_seg3, std::make_pair(_segmentStatus, _ptf1)));
    }
  }

  void makeDiscounted(PaxTypeFare* fare, int category)
  {
    fare->status().set(PaxTypeFare::PTF_Discounted);
    DiscountInfo* info = _memHandle.create<DiscountInfo>();
    PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();

    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;

    ruleData->ruleItemInfo() = info;
    info->category() = category;
    (*fare->paxTypeFareRuleDataMap())[RuleConst::CHILDREN_DISCOUNT_RULE] = allRules;
  }

  void settestDataforAdvPur()
  {
    _trx->setRequest(_memHandle.create<PricingRequest>());
    DateTime tktDate = DateTime(2010, 5, 4, 8, 15, 0);
    _trx->getRequest()->ticketingDT() = tktDate;
    _optSvcInfo->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    setTicketingAgent();
  }

  void setupForFeeApplication()
  {
    _ocFee->feeAmount() = 100;
    _optSrvValidator->_processedFares.insert(_ptf1);
    _optSrvValidator->_processedFares.insert(_ptf2);
    _optSrvValidator->_processedFares.insert((PaxTypeFare*)NULL);
  }

  void settestDataforFailStartStopTime()
  {
    (*_segI)->pssDepartureTime() = "0630";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1015;
  }

  void settestDataforStartStopTime()
  {
    (*_segI)->pssDepartureTime() = "0630";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1250;
  }

  void settestDataforStartStopTime_Same()
  {
    (*_segI)->pssDepartureTime() = "0600";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1230;
  }

  void setTicketingAgent()
  {
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    //_trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
  }

  // TESTS
  void testValidate_Pass()
  {
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee));
  }

  void testValidate_Fail() { CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee)); }

  void testValidateS7Data_Pass()
  {
    CPPUNIT_ASSERT_EQUAL(PASS_S7, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Sequence()
  {
    createDiagnostic(false);
    _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::SEQ_NUMBER, "1"));
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_SEQUENCE, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_TravelDate()
  {
    _optSvcInfo->ticketDiscDate() = DateTime(2010, 1, 17);
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_TVL_DATE, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_InputTravelDate()
  {
    _optSvcInfo->expireDate() = DateTime(2010, 1, 17);
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_INPUT_TVL_DATE,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_InputPtc()
  {
    _optSvcInfo->psgType() = MIL;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_INPUT_PSG_TYPE,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_FreqFlyer()
  {
    createFFData(4, "AA", 3, "AA");
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_FREQ_FLYER_STATUS,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_AccountCodes()
  {
    _optSvcInfo->serviceFeesAccountCodeTblItemNo() = 1000;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_ACCOUNT_CODE,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_InterlineInd_N()
  {
    _optSvcInfo->interlineInd() = 'N';
    CPPUNIT_ASSERT_EQUAL(PASS_S7,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_InterlineInd_Y()
  {
    _optSvcInfo->interlineInd() = 'Y';
    CPPUNIT_ASSERT_EQUAL(PASS_S7,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }


  void testValidateS7Data_Fail_InputTicketDesignator()
  {
    _optSvcInfo->serviceFeesTktDesigTblItemNo() = 1000;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_INPUT_TKT_DESIGNATOR,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Security()
  {
    _optSvcInfo->publicPrivateInd() = OptionalServicesValidator::T183_SCURITY_PRIVATE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_SECUR_T183,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_SectorPortion()
  {
    _optSvcInfo->serviceFeesResBkgDesigTblItemNo() = 1;
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_SECTOR_PORTION,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_FtwInd()
  {
    _optSvcInfo->fromToWithinInd() = '~';
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_FROM_TO_WITHIN,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_IntermediatePoint()
  {
    _optSvcInfo->viaLoc() = _optSvcInfo->loc1();
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_INTERMEDIATE_POINT,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_StopCnxDest()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_CNX_POINT;

    CPPUNIT_ASSERT_EQUAL(FAIL_S7_STOP_CNX_DEST,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Cabin()
  {
    setupValidCaseForCabin();
    _optSvcInfo->cabin() = 'F';
    _optSvcInfo->carrier() = "UA";
    _seg1->setOperatingCarrierCode("UA");

    CPPUNIT_ASSERT_EQUAL(FAIL_S7_CABIN, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Rbd()
  {
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'O';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'O';
    _optSvcInfo->serviceFeesResBkgDesigTblItemNo() = 1;

    CPPUNIT_ASSERT_EQUAL(FAIL_S7_RBD_T198, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_ResultingFare()
  {
    _optSvcInfo->serviceFeesCxrResultingFclTblItemNo() = 1;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_RESULT_FC_T171,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_OutputTicketDesignator()
  {
    _optSvcInfo->resultServiceFeesTktDesigTblItemNo() = 1000;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_OUTPUT_TKT_DESIGNATOR,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_RuleTariffInd()
  {
    createTsMap();
    _optSvcInfo->ruleTariffInd() = OptionalServicesValidator::RULE_TARIFF_IND_PUBLIC;
    _tariffCrossRefInfo1->tariffCat() = 1;
    _ptf1->setFare(_fare1);
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_RULE_TARIFF_IND,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_RuleTariff()
  {
    createTsMap();
    _optSvcInfo->ruleTariff() = 10;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_RULE_TARIFF,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Rule()
  {
    createTsMap();
    _optSvcInfo->rule() = "FAIL";
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_RULE, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_FareInd()
  {
    createTsMap();
    _optSvcInfo->fareInd() = OptionalServicesValidator::FARE_IND_19_22;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_FARE_IND, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_TourCode()
  {
    _optSvcInfo->tourCode() = "FAIL";
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_TOURCODE, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Start_Stop_Time()
  {
    _optSvcInfo->timeApplication() = 'D';
    settestDataforFailStartStopTime();
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_START_STOP_TIME,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Start_Stop_Time_DOW()
  {
    _optSvcInfo->timeApplication() = 'D';
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->fltTktMerchInd() = MERCHANDISE_SERVICE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_START_STOP_TIME,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Dow()
  {
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->fltTktMerchInd() = MERCHANDISE_SERVICE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_DOW, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Equipment()
  {
    _optSvcInfo->equipmentCode() = "888";
    _optSvcInfo->fltTktMerchInd() = TICKET_RELATED_SERVICE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_EQUIPMENT,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Mileage()
  {
    _optSvcInfo->applicationFee() = 100;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_MILEAGE_FEE,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_CarrierFlightApplT186()
  {
    _optSvcInfo->carrierFltTblItemNo() = 100;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_CXR_FLT_T186,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_AvdPur()
  {
    _optSvcInfo->fltTktMerchInd() = TICKET_RELATED_SERVICE;
    _optSvcInfo->advPurchPeriod() = "MON";
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_ADVPUR, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Sfc()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 1;
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_svcFeeCurInfos.clear();
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_SFC_T170, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testDisplaySvcFeeCurInfoHeader()
  {
    createDiagnostic(true);
    _optSrvValidator->displaySvcFeeCurInfoHeader(9);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("ITEM NO : 9") != string::npos);
  }

  void testDisplaySvcFeeCurInfoHeader_NoDDInfo()
  {
    createDiagnostic(false);
    _optSrvValidator->displaySvcFeeCurInfoHeader(9);
    CPPUNIT_ASSERT_EQUAL(string::npos, _optSrvValidator->_diag->str().find("ITEM NO : 9"));
  }

  void testDisplaySvcFeeCurInfoDetail()
  {
    createDiagnostic(true);
    _optSrvValidator->displaySvcFeeCurInfoDetail(
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_currInfo, true);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("PASS") != string::npos);
  }

  void testDisplaySvcFeeCurInfoDetail_NoDDInfo()
  {
    createDiagnostic(false);
    _optSrvValidator->displaySvcFeeCurInfoDetail(
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_currInfo, true);
    CPPUNIT_ASSERT_EQUAL(string::npos, _optSrvValidator->_diag->str().find("PASS"));
  }

  void testDisplaySvcFeeCurInfoHeaderAndDetail()
  {
    createDiagnostic(true);
    _optSrvValidator->displaySvcFeeCurInfoHeaderAndDetail(
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_currInfo, true, true);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("DEFAULT POS EMPTY") != string::npos);
  }

  void testDisplaySvcFeeCurInfoHeaderAndDetail_NoDDInfo()
  {
    createDiagnostic(false);
    _optSrvValidator->displaySvcFeeCurInfoHeaderAndDetail(
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_currInfo, true, true);
    CPPUNIT_ASSERT_EQUAL(string::npos, _optSrvValidator->_diag->str().find("DEFAULT POS EMPTY"));
  }

  void testCheckDiagS7ForDetail()
  {
    createDiagnostic(true);
    _optSrvValidator->checkDiagS7ForDetail(_optSvcInfo);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("OPTIONAL SERVICE S7 DETAILED INFO") !=
                   string::npos);
  }

  void testCheckDiagS7ForDetail_NoDDInfo()
  {
    createDiagnostic(false);
    _optSrvValidator->checkDiagS7ForDetail(_optSvcInfo);
    CPPUNIT_ASSERT_EQUAL(string::npos,
                         _optSrvValidator->_diag->str().find(" OPTIONAL SERVICE S7 DETAILED INFO"));
  }

  void testPrintDiagS7NotFound()
  {
    createDiagnostic(false);
    _optSrvValidator->printDiagS7NotFound(SubCodeInfo());
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("DATA NOT FOUND") != string::npos);
  }

  void testPrintDiagS7Info_DDInfo()
  {
    createDiagnostic(true);
    _optSrvValidator->printDiagS7Info(_optSvcInfo, *_ocFee, PASS_S7);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("S7 STATUS : PASS") != string::npos);
  }

  void testPrintDiagS7Info_NotDDInfo()
  {
    createDiagnostic(false);
    _optSrvValidator->printDiagS7Info(_optSvcInfo, *_ocFee, PASS_S7);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("0                       PASS") !=
                   string::npos);
  }

  void testPrintDiagS7Info_DDPassed()
  {
    createDiagnosticDDPassed(true);
    _optSrvValidator->printDiagS7Info(_optSvcInfo, *_ocFee, PASS_S7);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("0                       PASS") !=
                   string::npos);
  }

  void testPrintDiagS7Info_DDPassed_Status_Fail()
  {
    createDiagnosticDDPassed(true);
    _optSrvValidator->printDiagS7Info(_optSvcInfo, *_ocFee, FAIL_S7_TVL_DATE);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().empty());
  }

  void testPrintDiagS7Info_NotDDPassed()
  {
    createDiagnosticDDPassed(false);
    _optSrvValidator->printDiagS7Info(_optSvcInfo, *_ocFee, FAIL_S7_TVL_DATE);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("0                       FAIL TVL DATE") !=
                   string::npos);
  }

  void test_isDDPass_Pass()
  {
    createDiagnosticDDPassed(true);
    CPPUNIT_ASSERT(_optSrvValidator->isDDPass() == true);
  }

  void test_isDDPass_Fail()
  {
    createDiagnosticDDPassed(false);
    CPPUNIT_ASSERT(_optSrvValidator->isDDPass() != true);
  }

  void test_Validate_DDPass_False_Datanotfound()
  {
    createDiagnosticDDPassed(false);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee) == false);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find(
                       "FLIGHT                            DATA NOT FOUND") != string::npos);
  }

  void test_Validate_DDPass_True_Datanotfound()
  {
    createDiagnosticDDPassed(true);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee) == false);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().empty());
  }

  void testCheckTravelDateWhenFailNoData()
  {
    _optSvcInfo->ticketEffDate() = DateTime::emptyDate();
    _optSvcInfo->ticketDiscDate() = DateTime::emptyDate();

    CPPUNIT_ASSERT(!_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenPassEffDisc()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 18, 13, 15, 0);
    _optSvcInfo->ticketEffDate() = DateTime(2010, 1, 18, 15, 15, 0);
    _optSvcInfo->ticketDiscDate() = DateTime(2010, 1, 18, 20, 15, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenSameDateNoTimeOnEffDisc_Pass()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 18, 14, 15, 0);
    _optSvcInfo->ticketEffDate() = DateTime(2010, 1, 18, 0, 0, 0);
    _optSvcInfo->ticketDiscDate() = DateTime(2010, 1, 18, 0, 0, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenDiffDateNoTimeOnEffDisc_Pass()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 28, 14, 15, 0);
    _optSvcInfo->ticketEffDate() = DateTime(2010, 1, 18, 0, 0, 0);
    _optSvcInfo->ticketDiscDate() = DateTime(2010, 2, 18, 0, 0, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenPassTvlData()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 18, 16, 15, 0);
    createOptSvcInfWthDate(2010, 1, 15, 2010, 1, 21);
    CPPUNIT_ASSERT(_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenFailTvlStart()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 18, 16, 15, 0);
    createOptSvcInfWthDate(2010, 1, 19, 0, 0, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenFailTvlStop()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 20, 16, 15, 0);
    createOptSvcInfWthDate(0, 0, 0, 2010, 1, 19);
    CPPUNIT_ASSERT(!_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckTravelDateWhenFailMonthZero()
  {
    (*_segI)->departureDT() = DateTime(2010, 1, 20, 16, 15, 0);
    createOptSvcInfWthDate(2010, 0, 15, 2010, 0, 21);
    CPPUNIT_ASSERT(!_optSrvValidator->checkTravelDate(*_optSvcInfo));
  }

  void testCheckInputTravelDate_Pass()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkInputTravelDate(*_optSvcInfo));
  }

  void testCheckInputTravelDate_Fail_Create()
  {
    _optSvcInfo->createDate() = DateTime(2010, 1, 19);
    CPPUNIT_ASSERT(!_optSrvValidator->checkInputTravelDate(*_optSvcInfo));
  }

  void testCheckInputTravelDate_Fail_Expire()
  {
    _optSvcInfo->expireDate() = DateTime(2010, 1, 17);
    CPPUNIT_ASSERT(!_optSrvValidator->checkInputTravelDate(*_optSvcInfo));
  }

  void testCheckInputTravelDate_Fail_Disc()
  {
    _optSvcInfo->discDate() = DateTime(2010, 1, 17);
    CPPUNIT_ASSERT(!_optSrvValidator->checkInputTravelDate(*_optSvcInfo));
  }

  void testCheckInputTravelDate_Fail_Eff()
  {
    _optSvcInfo->effDate() = DateTime(2010, 1, 19);
    CPPUNIT_ASSERT(!_optSrvValidator->checkInputTravelDate(*_optSvcInfo));
  }

  void testCheckInputPtc_Pass() { CPPUNIT_ASSERT(_optSrvValidator->checkInputPtc(*_optSvcInfo)); }

  void testCheckInputPtc_Fail()
  {
    _optSvcInfo->psgType() = ADULT;
    CPPUNIT_ASSERT(!_optSrvValidator->checkInputPtc(*_optSvcInfo));
  }

  void testCheckAccountCodes_Pass_Zero() { CPPUNIT_ASSERT(_optSrvValidator->checkAccountCodes(0)); }

  void testCheckAccountCodes_Pass() { CPPUNIT_ASSERT(_optSrvValidator->checkAccountCodes(1001)); }

  void testCheckAccountCodes_Fail() { CPPUNIT_ASSERT(!_optSrvValidator->checkAccountCodes(1000)); }

  void testCheckInputTicketDesignator_Pass_Zero()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkInputTicketDesignator(0));
  }

  void testCheckInputTicketDesignator_Pass()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkInputTicketDesignator(1001));
  }

  void testCheckInputTicketDesignator_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->checkInputTicketDesignator(1000));
  }

  void testCheckOutputTicketDesignator_Pass_Zero()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkOutputTicketDesignator(*_optSvcInfo));
  }

  void testCheckOutputTicketDesignator_Fail_T171WithRuleTariffAndMultiCxr()
  {
    _optSvcInfo->resultServiceFeesTktDesigTblItemNo() = 1001;
    _optSvcInfo->ruleTariff() = 9;
    _optSvcInfo->rule() = "XX";
    _optSvcInfo->ruleTariffInd() = "XX";
    _optSrvValidator->_isOneCarrier = false;

    CPPUNIT_ASSERT(!_optSrvValidator->checkOutputTicketDesignator(*_optSvcInfo));
  }

  void testCheckOutputTicketDesignator_Fail_T171WithFareIndAndMultiCxr()
  {
    _optSvcInfo->resultServiceFeesTktDesigTblItemNo() = 1001;
    _optSvcInfo->ruleTariff() = 9;
    _optSvcInfo->rule() = "XX";
    _optSvcInfo->fareInd() = 'X';
    _optSrvValidator->_isOneCarrier = false;

    CPPUNIT_ASSERT(!_optSrvValidator->checkOutputTicketDesignator(*_optSvcInfo));
  }

  void testCheckOutputTicketDesignator_Pass()
  {
    _optSvcInfo->resultServiceFeesTktDesigTblItemNo() = 1001;
    CPPUNIT_ASSERT(_optSrvValidator->checkOutputTicketDesignator(*_optSvcInfo));
  }

  void testCheckOutputTicketDesignator_Fail()
  {
    _optSvcInfo->resultServiceFeesTktDesigTblItemNo() = 1000;

    CPPUNIT_ASSERT(!_optSrvValidator->checkOutputTicketDesignator(*_optSvcInfo));
  }

  void testCheckGeoFtwInd_Pass_From()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::FTW_FROM;
    CPPUNIT_ASSERT(_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Pass_To()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::FTW_TO;
    CPPUNIT_ASSERT(_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Pass_Between()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::CHAR_BLANK;
    CPPUNIT_ASSERT(_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Pass_Between2()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::CHAR_BLANK2;
    CPPUNIT_ASSERT(_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Pass_Between_Empty()
  {
    _optSvcInfo->loc1().loc() = "";
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::CHAR_BLANK;
    CPPUNIT_ASSERT(_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Pass_Within()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::FTW_WITHIN;
    CPPUNIT_ASSERT(_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Fail_RuleBusterA()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::FTW_RULE_BUSTER_A;
    CPPUNIT_ASSERT(!_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Fail_RuleBusterB()
  {
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::FTW_RULE_BUSTER_B;
    CPPUNIT_ASSERT(!_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testCheckGeoFtwInd_Fail_Other()
  {
    _optSvcInfo->fromToWithinInd() = '~';
    CPPUNIT_ASSERT(!_optSrvValidator->checkGeoFtwInd(*_optSvcInfo, *_ocFee));
  }

  void testValidateLocation_Pass_Empty()
  {
    CPPUNIT_ASSERT(
        _optSrvValidator->validateLocation(VendorCode(), LocKey(), Loc(), LocCode(), true, "AA"));
  }

  void testValidateLocation_Fail_Empty()
  {
    CPPUNIT_ASSERT(
        !_optSrvValidator->validateLocation(VendorCode(), LocKey(), Loc(), LocCode(), false, "AA"));
  }

  void testValidateLocation_Pass_Zone()
  {
    CPPUNIT_ASSERT(_optSrvValidator->validateLocation(
        VendorCode(), LocKeyMock("A", LOCTYPE_USER), LocMock("1"), LocCode(), false, "AA"));
  }

  void testValidateLocation_Fail_Zone()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->validateLocation(
        VendorCode(), LocKeyMock("A", LOCTYPE_USER), LocMock("A"), LocCode(), false, "AA"));
  }

  void testValidateLocation_Pass_Loc()
  {
    CPPUNIT_ASSERT(_optSrvValidator->validateLocation(
        VendorCode(), LocKeyMock("A"), LocMock("A"), LocCode(), false, "AA"));
  }

  void testValidateLocation_Fail_Loc()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->validateLocation(
        VendorCode(), LocKeyMock("A"), LocMock("1"), LocCode(), false, "AA"));
  }

  void testValidateWithin_Fail_Orig()
  {
    LocMock loc("1");
    _seg1->origin() = &loc;
    CPPUNIT_ASSERT(!_optSrvValidator->validateWithin(*_optSvcInfo));
  }

  void testValidateWithin_Fail_Dest()
  {
    LocMock loc("1");
    _seg1->destination() = &loc;
    CPPUNIT_ASSERT(!_optSrvValidator->validateWithin(*_optSvcInfo));
  }

  void testValidateWithin_Pass() { CPPUNIT_ASSERT(_optSrvValidator->validateWithin(*_optSvcInfo)); }

  void testCheckIntermediatePoint_Pass_ViaNull()
  {
    vector<TravelSeg*> loc3passed;
    CPPUNIT_ASSERT(_optSrvValidator->checkIntermediatePoint(*_optSvcInfo, loc3passed, *_ocFee));
  }

  void testCheckIntermediatePoint_SameAS_Loc1_Fail()
  {
    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().loc() = LOC_WAS;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;

    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(!_optSrvValidator->checkIntermediatePoint(*_optSvcInfo, loc3passed, *_ocFee));
  }

  void testCheckIntermediatePoint_SameAS_Loc2_Fail()
  {
    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(!_optSrvValidator->checkIntermediatePoint(*_optSvcInfo, loc3passed, *_ocFee));
  }

  void testCheckIntermediatePoint_Pass_Loc2()
  {
    AirSeg* air = _memHandle.create<AirSeg>();
    _travelSegs->push_back(air);
    _segIE = _travelSegs->end();
    air->origin() = _seg2Arunk->origin();
    air->destination() = _seg2Arunk->destination();
    air->segmentOrder() = 4;

    OptionalServicesValidator* optSrvValidator1;
    optSrvValidator1 = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));

    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    _optSvcInfo->loc2().loc() = LOC_NRT;
    _optSvcInfo->loc2().locType() = LOCTYPE_CITY;

    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(optSrvValidator1->checkIntermediatePoint(*_optSvcInfo, loc3passed, *_ocFee));
  }

  void testCheckIntermediatePoint_Pass_Stopover()
  {
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    _optSvcInfo->loc1().loc() = "1";
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_STOPOVER_WITH_GEO;
    _seg2Arunk->departureDT() = _seg2Arunk->arrivalDT();
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(_optSrvValidator->checkIntermediatePoint(*_optSvcInfo, loc3passed, *_ocFee));
  }

  void testCheckIntermediatePoint_Fail()
  {
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(!_optSrvValidator->checkIntermediatePoint(*_optSvcInfo, loc3passed, *_ocFee));
  }

  void testIsStopover_Pass_Blank()
  {
    CPPUNIT_ASSERT(_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Fail_Blank()
  {
    _seg3->departureDT() = _seg1->arrivalDT();
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_SegArunk()
  {
    _optSvcInfo->stopoverUnit() = 'H';
    _optSvcInfo->stopoverTime() = "20";
    CPPUNIT_ASSERT(_optSrvValidator->isStopover(*_optSvcInfo, _seg2Arunk, _seg3));
  }

  void testIsStopover_Fail_SegArunk()
  {
    _optSvcInfo->stopoverUnit() = 'H';
    _optSvcInfo->stopoverTime() = "21";
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg2Arunk, _seg3));
  }

  void testIsStopover_Pass_NextArunk()
  {
    _optSvcInfo->stopoverUnit() = 'N';
    _optSvcInfo->stopoverTime() = "221";
    CPPUNIT_ASSERT(_optSrvValidator->isStopover(*_optSvcInfo, _seg3, _seg4Arunk));
  }

  void testIsStopover_Fail_NextArunk()
  {
    _optSvcInfo->stopoverUnit() = 'N';
    _optSvcInfo->stopoverTime() = "222";
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg3, _seg4Arunk));
  }

  void testIsStopover_Pass()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";
    CPPUNIT_ASSERT(_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_When_D_Blank()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "";
    _seg3->departureDT() = DateTime(2010, 1, 18, 15, 0, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_When_D_Value_Fail()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "2";
    _seg3->departureDT() = DateTime(2010, 1, 20);
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_When_D_Value_Pass()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "2";
    _seg3->departureDT() = DateTime(2010, 1, 21);
    CPPUNIT_ASSERT(_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_When_M_Blank()
  {
    _optSvcInfo->stopoverUnit() = 'M';
    _optSvcInfo->stopoverTime() = "";
    _seg3->departureDT() = DateTime(2010, 1, 18, 15, 0, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_When_M_Value_Fail()
  {
    _optSvcInfo->stopoverUnit() = 'M';
    _optSvcInfo->stopoverTime() = "1";
    _seg3->departureDT() = DateTime(2010, 2, 18);
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Pass_When_M_Value_Pass()
  {
    _optSvcInfo->stopoverUnit() = 'M';
    _optSvcInfo->stopoverTime() = "1";
    _seg3->departureDT() = DateTime(2010, 2, 19);
    CPPUNIT_ASSERT(_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testIsStopover_Fail()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "1";
    CPPUNIT_ASSERT(!_optSrvValidator->isStopover(*_optSvcInfo, _seg1, _seg3));
  }

  void testCheckStopCnxDestInd_Fail_Cnx()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_CNX_POINT;
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Fail_FareBreak()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_NO_FARE_BREAK;
    _fareUsage->travelSeg().push_back(_seg2Arunk);

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Fail_FareBreakOrStopover_FareBreak()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_NO_FARE_BREAK_OR_STOPOVER;
    _fareUsage->travelSeg().push_back(_seg2Arunk);

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Fail_FareBreakOrStopover_Stopover()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_NO_FARE_BREAK_OR_STOPOVER;
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Fail_Stopover()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_STOPOVER;
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "1";

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Fail_StopoverWithGeo()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_STOPOVER_WITH_GEO;
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "1";

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Pass_OneSegmentPortion()
  {
    _optSrvValidator = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segI + 1, *_ts2ss));

    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_CNX_POINT;
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";

    CPPUNIT_ASSERT(
        _optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testCheckStopCnxDestInd_Pass()
  {
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_CNX_POINT;
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "1";

    CPPUNIT_ASSERT(
        _optSrvValidator->checkStopCnxDestInd(*_optSvcInfo, vector<TravelSeg*>(), *_ocFee));
  }

  void testValidateCnxOrStopover_Fail_Sector()
  {
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    CPPUNIT_ASSERT(
        !_optSrvValidator->validateCnxOrStopover(*_optSvcInfo, vector<TravelSeg*>(), true));
  }

  void testValidateCnxOrStopover_Fail_Stopover()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";

    CPPUNIT_ASSERT(
        !_optSrvValidator->validateCnxOrStopover(*_optSvcInfo, vector<TravelSeg*>(), true));
  }

  void testValidateCnxOrStopover_Fail_StopoverPassedLoc3()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "000";
    vector<TravelSeg*> passedLoc3;
    passedLoc3.push_back(_seg2Arunk);

    CPPUNIT_ASSERT(!_optSrvValidator->validateCnxOrStopover(*_optSvcInfo, passedLoc3, true));
  }

  void testValidateCnxOrStopover_Pass()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "1";

    CPPUNIT_ASSERT(
        _optSrvValidator->validateCnxOrStopover(*_optSvcInfo, vector<TravelSeg*>(), true));
  }

  void testValidateNoFareBreak_Fail_Sector()
  {
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    CPPUNIT_ASSERT(!_optSrvValidator->validateNoFareBreak(*_optSvcInfo, vector<TravelSeg*>()));
  }

  void testValidateNoFareBreak_Fail_FareBreak()
  {
    _fareUsage->travelSeg().push_back(_seg2Arunk);
    CPPUNIT_ASSERT(!_optSrvValidator->validateNoFareBreak(*_optSvcInfo, vector<TravelSeg*>()));
  }

  void testValidateNoFareBreak_Pass_FareBreakPassedLoc3()
  {
    _fareUsage->travelSeg().push_back(_seg2Arunk);
    vector<TravelSeg*> passedLoc3;
    passedLoc3.push_back(_seg1);

    CPPUNIT_ASSERT(_optSrvValidator->validateNoFareBreak(*_optSvcInfo, passedLoc3));
  }

  void testValidateNoFareBreak_Pass_FareBreakSideTrip()
  {
    _fareUsage->travelSeg().push_back(_seg2Arunk);
    _pricingUnit->isSideTripPU() = true;

    CPPUNIT_ASSERT(_optSrvValidator->validateNoFareBreak(*_optSvcInfo, vector<TravelSeg*>()));
  }

  void testValidateNoFareBreak_Pass()
  {
    CPPUNIT_ASSERT(_optSrvValidator->validateNoFareBreak(*_optSvcInfo, vector<TravelSeg*>()));
  }

  void testValidateViaWithLoc2_FailOnOrigin()
  {
    // Point of travel matched with S7 VIA point which is the same location as origin of travel should be skipped in validation
    // WAS-SIN
    // SIN-WAS
    // WAS-NYC

    Loc* location = _memHandle.insert(new LocMock(LOC_SIN));
    (*_segI)->destination() = location;
    (*(_segI+1))->origin() = location;

    OptionalServicesValidator* optSrvValidator1;
    optSrvValidator1 = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));

    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    _optSvcInfo->viaLoc().loc() = "WAS";
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(!optSrvValidator1->validateViaWithLoc2(*_optSvcInfo, loc3passed));
  }

  void testValidateViaWithLoc2_FailOnDestination()
  {
    // Point of travel matched with S7 VIA point which is the same location as destination of travel should be skipped in validation
    // WAS NYC
    // NYC SIN
    // SIN NYC

    Loc* location = _memHandle.insert(new LocMock(LOC_SIN));
    (*(_segI+1))->destination() = location;
    (*(_segI+2))->origin() = location;

    OptionalServicesValidator* optSrvValidator1;
    optSrvValidator1 = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));

    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    _optSvcInfo->viaLoc().loc() = "NYC";
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(!optSrvValidator1->validateViaWithLoc2(*_optSvcInfo, loc3passed));
  }

  void testValidateViaWithLoc2_PassDespiteS7ViaPointMatchOrigin()
  {
    // Point of travel matched with S7 VIA point which is the same location as origin of travel should be skipped in validation
    // but validation should be continued
    // WAS EWR
    // EWR WAS
    // WAS SIN

    Loc* location = _memHandle.insert(new LocMock(LOC_EWR));
    location->nation() = "US";

    (*_segI)->destination() = location;
    (*(_segI+1))->origin() = location;
    (*(_segI+2))->destination() = _memHandle.insert(new LocMock(LOC_SIN));

    OptionalServicesValidator* optSrvValidator1;
    optSrvValidator1 = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));

    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_NATION;
    _optSvcInfo->viaLoc().loc() = "US";
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(optSrvValidator1->validateViaWithLoc2(*_optSvcInfo, loc3passed));
  }

  void testValidateViaWithLoc2_PassDespiteS7ViaPointMatchDestination()
  {
    // Point of travel matched with S7 VIA point which is the same location as destination of travel should be skipped in validation
    // but validation should be continued
    // SIN EWR
    // EWR WAS
    // WAS NYC

    Loc* location = _memHandle.insert(new LocMock(LOC_EWR));
    location->nation() = "US";

    (*_segI)->destination() = location;
    (*(_segI+1))->origin() = location;
    (*_segI)->origin() = _memHandle.insert(new LocMock(LOC_SIN));

    OptionalServicesValidator* optSrvValidator1;
    optSrvValidator1 = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));

    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_NATION;
    _optSvcInfo->viaLoc().loc() = "US";
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(optSrvValidator1->validateViaWithLoc2(*_optSvcInfo, loc3passed));
  }

  void testValidateViaWithLoc2_Pass()
  {
    AirSeg* air = _memHandle.create<AirSeg>();
    _travelSegs->push_back(air);
    _segIE = _travelSegs->end();
    air->origin() = _seg2Arunk->origin();
    air->destination() = _seg2Arunk->destination();
    air->segmentOrder() = 4;

    OptionalServicesValidator* optSrvValidator1;
    optSrvValidator1 = _memHandle.insert(
        new OptionalServicesValidatorMock(*_trx, *_farePath, _segI, _segIE, *_ts2ss));

    _optSvcInfo->vendor() = SITA_VENDOR_CODE;
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(optSrvValidator1->validateViaWithLoc2(*_optSvcInfo, loc3passed));
  }

  void testValidateViaWithLoc2_Fail_Via()
  {
    _optSvcInfo->viaLoc().loc() = "3";
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    vector<TravelSeg*> loc3passed;

    CPPUNIT_ASSERT(!_optSrvValidator->validateViaWithLoc2(*_optSvcInfo, loc3passed));
  }

  void testValidateViaWithStopover_Pass()
  {
    _optSvcInfo->loc1().loc() = "1";
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;
    _seg2Arunk->departureDT() = _seg2Arunk->arrivalDT();

    CPPUNIT_ASSERT(_optSrvValidator->validateViaWithStopover(*_optSvcInfo));
  }

  void testValidateViaWithStopover_Fail_Via()
  {
    _optSvcInfo->loc1().loc() = "1";
    _optSvcInfo->viaLoc().loc() = "3";
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;

    CPPUNIT_ASSERT(!_optSrvValidator->validateViaWithStopover(*_optSvcInfo));
  }

  void testValidateViaWithStopover_Fail_Stopover()
  {
    _optSvcInfo->loc1().loc() = "1";
    _optSvcInfo->viaLoc().loc() = LOC_NYC;
    _optSvcInfo->viaLoc().locType() = LOCTYPE_CITY;

    _seg2Arunk->departureDT() = DateTime(2010, 1, 19);
    _seg2Arunk->arrivalDT() = DateTime(2010, 2, 19);

    CPPUNIT_ASSERT(!_optSrvValidator->validateViaWithStopover(*_optSvcInfo));
  }

  void testFindFirstStopover_Pass()
  {
    _seg2Arunk->departureDT() = DateTime(2010, 1, 19);
    _seg2Arunk->arrivalDT() = DateTime(2010, 2, 19);
    CPPUNIT_ASSERT_EQUAL(3,
                         (int)distance(_optSrvValidator->findFirstStopover(*_optSvcInfo), _segIE));
  }

  void testFindFirstStopover_Fail()
  {
    _optSvcInfo->stopoverUnit() = 'D';
    _optSvcInfo->stopoverTime() = "1";

    CPPUNIT_ASSERT_EQUAL(0,
                         (int)distance(_optSrvValidator->findFirstStopover(*_optSvcInfo), _segIE));
  }

  void testCheckSecurityWhenT183ItemIsZero()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkSecurity(*_optSvcInfo, *_ocFee));
  }

  void testCheckSecurityWhenT183ItemIsZeroAndPrivateIndicatorON()
  {
    _optSvcInfo->publicPrivateInd() = OptionalServicesValidator::T183_SCURITY_PRIVATE;
    CPPUNIT_ASSERT(!_optSrvValidator->checkSecurity(*_optSvcInfo, *_ocFee));
  }

  void testRetrieveSpecifiedFee_Pass_Zero()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 0;
    CPPUNIT_ASSERT(_optSrvValidator->retrieveSpecifiedFee(*_optSvcInfo, *_ocFee));
  }

  void testRetrieveSpecifiedFee_Fail_Empty()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 1;
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_svcFeeCurInfos.clear();
    CPPUNIT_ASSERT(!_optSrvValidator->retrieveSpecifiedFee(*_optSvcInfo, *_ocFee));
  }

  void testRetrieveSpecifiedFee_Pass()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 1;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    CPPUNIT_ASSERT(_optSrvValidator->retrieveSpecifiedFee(*_optSvcInfo, *_ocFee));
  }

  void testvalidateSfcLocation_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->validateSfcLocation(VendorCode(), LocKey(), Loc()));
  }

  void testvalidateSfcLocation_Pass()
  {
    CPPUNIT_ASSERT(_optSrvValidator->validateSfcLocation(VendorCode(), LocKeyMock("A"), Loc()));
  }

  void testIsRBDValidWhenEmptyRBDInfo()
  {
    _validRBDInfoForSeg->clear();
    CPPUNIT_ASSERT(!_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenExistValidRBD()
  {
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _rbdInfo1->carrier() = "AA";
    _rbdInfo2->carrier() = "AA";
    _seg1->setBookingCode(BookingCode('V'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'B';
    _rbdInfo1->bookingCode2() = 'C';
    _rbdInfo2->bookingCode2() = 'V';

    CPPUNIT_ASSERT(_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenOperPhaseAndDifftMktAndOperCxr()
  {
    _optSrvValidator->_isMarketingCxr = false;
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("UA");
    _rbdInfo1->carrier() = "AA";
    _rbdInfo2->carrier() = "AA";
    _seg1->setBookingCode(BookingCode('V'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'B';
    _rbdInfo1->bookingCode2() = 'C';
    _rbdInfo2->bookingCode2() = 'V';

    CPPUNIT_ASSERT(!_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenMktPhaseAndDifftMktAndOperCxr()
  {
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("UA");
    _rbdInfo1->mkgOperInd() = OptionalServicesValidator::T198_MKT_OPER_IND_OPER;
    _rbdInfo2->mkgOperInd() = OptionalServicesValidator::T198_MKT_OPER_IND_OPER;
    _rbdInfo1->carrier() = "AA";
    _rbdInfo2->carrier() = "AA";
    _seg1->setBookingCode(BookingCode('V'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'B';
    _rbdInfo1->bookingCode2() = 'C';
    _rbdInfo2->bookingCode2() = 'V';

    CPPUNIT_ASSERT(!_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenDifftRBDCxr()
  {
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _rbdInfo1->carrier() = "UA";
    _rbdInfo2->carrier() = "UA";
    _seg1->setBookingCode(BookingCode('V'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'B';
    _rbdInfo1->bookingCode2() = 'C';
    _rbdInfo2->bookingCode2() = 'V';

    CPPUNIT_ASSERT(!_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenNoExistValidRBD()
  {
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _rbdInfo1->carrier() = "AA";
    _rbdInfo2->carrier() = "AA";
    _seg1->setBookingCode(BookingCode('V'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'B';
    _rbdInfo1->bookingCode2() = 'C';
    _rbdInfo2->bookingCode2() = 'Z';

    CPPUNIT_ASSERT(!_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenExistValidRBDForRebook()
  {
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _rbdInfo1->carrier() = "AA";
    _rbdInfo2->carrier() = "AA";
    _seg1->setBookingCode(BookingCode('X'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'V';
    _rbdInfo1->bookingCode2() = 'C';
    _rbdInfo2->bookingCode2() = 'Z';
    createTsMap();
    _segmentStatus->_bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _segmentStatus->_bkgCodeReBook = 'V';

    CPPUNIT_ASSERT(_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testIsRBDValidWhenNoExistValidRBDForRebook()
  {
    _seg1->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _rbdInfo1->carrier() = "AA";
    _rbdInfo2->carrier() = "AA";
    _seg1->setBookingCode(BookingCode('B'));
    _rbdInfo1->bookingCode1() = 'B';
    _rbdInfo2->bookingCode1() = 'V';
    _rbdInfo1->bookingCode2() = 'D';
    _rbdInfo2->bookingCode2() = 'Z';
    createTsMap();
    _segmentStatus->_bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _segmentStatus->_bkgCodeReBook = 'C';

    CPPUNIT_ASSERT(!_optSrvValidator->isRBDValid(_seg1, *_validRBDInfoForSeg, *_ocFee));
  }

  void testCheckRBDWhenEmptyItemNo()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkRBD("ATP", 0, *_ocFee));
  }

  void test_skipUpgradeCheck_Match()
  {
    CPPUNIT_ASSERT(_optSrvValidator->skipUpgradeCheck(*_optSvcInfo, *_ocFee));
  }

  void test_skipUpgradeCheck_NoMatch1()
  {
    _optSvcInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 1;
    CPPUNIT_ASSERT(!_optSrvValidator->skipUpgradeCheck(*_optSvcInfo, *_ocFee));
  }

  void test_skipUpgradeCheck_NoMatch2()
  {
    _optSvcInfo->upgradeCabin() = 'J';
    CPPUNIT_ASSERT(!_optSrvValidator->skipUpgradeCheck(*_optSvcInfo, *_ocFee));
  }

  void test_skipUpgradeCheck_NoMatchButUPGroupForcesMatch()
  {
    _optSvcInfo->upgradeCabin() = 'J';
    _subCodeInfo->serviceGroup() = "UP";
    _ocFee->subCodeInfo() = _subCodeInfo;
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _trx->billing()->partitionID() = "HDQ";
    _trx->modifiableActivationFlags().setAB240(true);

    CPPUNIT_ASSERT(_optSrvValidator->skipUpgradeCheck(*_optSvcInfo, *_ocFee));
  }

  void testCheckRBDWhenFailOnSectorPortionCheck()
  {
    CPPUNIT_ASSERT(
        !_optSrvValidator->checkSectorPortionInd(OptionalServicesValidator::SEC_POR_IND_SECTOR));
  }

  void testCheckRBDWhenMktPhaseAndNoValidRecordsExists()
  {
    _seg1->carrier() = "AA";
    _seg3->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("UA");
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() =
        OptionalServicesValidator::T198_MKT_OPER_IND_OPER;
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() =
        OptionalServicesValidator::T198_MKT_OPER_IND_OPER;

    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenMktPhaseAndNoExistsRecordsForSpecifiedCxr()
  {
    _seg1->carrier() = "AA";
    _seg3->carrier() = "CO";
    _seg1->setOperatingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("AA");
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'M';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "AA";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "UA";

    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenMktPhaseAndNoExistsValidRBDs()
  {
    _seg1->carrier() = "LH";
    _seg3->carrier() = "CO";
    _seg1->setOperatingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("AA");
    _seg1->setBookingCode(BookingCode('B'));
    _seg3->setBookingCode(BookingCode('Z'));
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'M';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "CO";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.bookingCode1() = 'K';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "LH";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode1() = 'B';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode2() = 'Z';

    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenMktPhaseAndExistsValidRBDs()
  {
    _seg1->carrier() = "LH";
    _seg3->carrier() = "CO";
    _seg1->setOperatingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("AA");
    _seg1->setBookingCode(BookingCode('B'));
    _seg3->setBookingCode(BookingCode('Z'));
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'M';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "CO";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.bookingCode1() = 'Z';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "LH";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode1() = 'B';

    CPPUNIT_ASSERT(_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenOperPhaseAndUsedMktRecord()
  {
    _optSrvValidator->_isMarketingCxr = false;
    _seg1->carrier() = "LH";
    _seg3->carrier() = "CO";
    _seg1->setOperatingCarrierCode("LH");
    _seg3->setOperatingCarrierCode("CO");
    _seg1->setBookingCode(BookingCode('B'));
    _seg3->setBookingCode(BookingCode('Z'));
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'M';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'M';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "CO";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.bookingCode1() = 'Z';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "LH";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode1() = 'B';

    CPPUNIT_ASSERT(_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenOperPhaseAndDifferentCarriers()
  {
    _optSrvValidator->_isMarketingCxr = false;
    _seg1->carrier() = "AA";
    _seg3->carrier() = "AA";
    _seg1->setOperatingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("UA");
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'O';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';

    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenOperPhaseAndNoExistsRecordsForSpecifiedCxr()
  {
    _optSrvValidator->_isMarketingCxr = false;
    _seg1->carrier() = "CO";
    _seg3->carrier() = "AA";
    _seg1->setOperatingCarrierCode("CO");
    _seg3->setOperatingCarrierCode("AA");
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'O';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "AA";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "UA";

    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenOperPhaseAndNoExistsValidRBDs()
  {
    _optSrvValidator->_isMarketingCxr = false;
    _seg1->carrier() = "LH";
    _seg3->carrier() = "CO";
    _seg1->setOperatingCarrierCode("LH");
    _seg3->setOperatingCarrierCode("CO");
    _seg1->setBookingCode(BookingCode('B'));
    _seg3->setBookingCode(BookingCode('Z'));
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'O';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "CO";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.bookingCode1() = 'K';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "LH";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode1() = 'B';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode2() = 'Z';

    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));

    createDiagnostic(true);
    CPPUNIT_ASSERT(!_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testCheckRBDWhenOperPhaseAndExistsValidRBDs()
  {
    _optSrvValidator->_isMarketingCxr = false;
    _seg1->carrier() = "LH";
    _seg3->carrier() = "CO";
    _seg1->setOperatingCarrierCode("LH");
    _seg3->setOperatingCarrierCode("CO");
    _seg1->setBookingCode(BookingCode('B'));
    _seg3->setBookingCode(BookingCode('Z'));
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.mkgOperInd() = 'O';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.mkgOperInd() = 'E';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.carrier() = "CO";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo1.bookingCode1() = 'Z';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.carrier() = "LH";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_rbdInfo2.bookingCode1() = 'B';

    CPPUNIT_ASSERT(_optSrvValidator->checkRBD("ATP", 1, *_ocFee));
  }

  void testMapS7CabinTypeWhenPremiumFirstNew()
  {
    Indicator cabin = 'R';
    CabinType cabinType;
    cabinType.setClassFromAlphaNumAnswer('R');
    CPPUNIT_ASSERT_EQUAL(cabinType, _optSrvValidator->mapS7CabinType(cabin));
  }

  void testMapS7CabinTypeWhenPremiumEconomyNew()
  {
    Indicator cabin = 'P';
    CabinType cabinType;
    cabinType.setClassFromAlphaNumAnswer('W');
    CPPUNIT_ASSERT_EQUAL(cabinType, _optSrvValidator->mapS7CabinType(cabin));
  }

  void testMapS7CabinTypeWhenPremiumFirst()
  {
    Indicator cabin = 'R';
    CabinType cabinType;
    cabinType.setClassFromAlphaNum('P');

    CPPUNIT_ASSERT_EQUAL(cabinType, _optSrvValidator->mapS7CabinType(cabin));
  }

  void testMapS7CabinTypeWhenFirst()
  {
    Indicator cabin = 'F';
    CabinType cabinType;
    cabinType.setClassFromAlphaNum('F');

    CPPUNIT_ASSERT_EQUAL(cabinType, _optSrvValidator->mapS7CabinType(cabin));
  }

  void testMapS7CabinTypeWhenPremiumEconomy()
  {
    Indicator cabin = 'P';
    CabinType cabinType;
    cabinType.setClassFromAlphaNum('S');

    CPPUNIT_ASSERT_EQUAL(cabinType, _optSrvValidator->mapS7CabinType(cabin));
  }

  void testOptionalServicesValidatorWhen2AirSegs()
  {
    CPPUNIT_ASSERT_EQUAL(size_t(2), _optSrvValidator->_airSegs.size());
    CPPUNIT_ASSERT_EQUAL(_seg1, _optSrvValidator->_airSegs[0]);
    CPPUNIT_ASSERT_EQUAL(_seg3, _optSrvValidator->_airSegs[1]);
  }

  void testCheckCabinWhenS7CabinIsBlank()
  {
    const Indicator cabin = BLANK;
    const CarrierCode carrier = "AA";

    CPPUNIT_ASSERT(_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckCabinWhenSegHasDifferentMarketCxrThenS7Record()
  {
    setupValidCaseForCabin();
    const Indicator cabin = 'F';
    const CarrierCode carrier = "UA";
    _seg1->setMarketingCarrierCode("UA");

    CPPUNIT_ASSERT(!_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckCabinWhenSegHasDifferentOperCxrThenS7Record()
  {
    setupValidCaseForCabin();
    const Indicator cabin = 'F';
    const CarrierCode carrier = "UA";
    _optSrvValidator->_isMarketingCxr = false;

    CPPUNIT_ASSERT(!_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckCabinWhenAllSegmentsHaveSameMarketCxrAndSameCabinAsS7()
  {
    setupValidCaseForCabin();
    const Indicator cabin = 'F';
    const CarrierCode carrier = "AA";

    CPPUNIT_ASSERT(_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckCabinWhenAllSegmentsHaveSameOperCxrAndSameCabinAsS7()
  {
    setupValidCaseForCabin();
    const Indicator cabin = 'F';
    const CarrierCode carrier = "AA";
    _optSrvValidator->_isMarketingCxr = false;

    CPPUNIT_ASSERT(_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckCabinWhenOneOfSegmentsHasSameMarketCxrAndDifferentCabinThenS7()
  {
    setupValidCaseForCabin();
    const Indicator cabin = 'F';
    const CarrierCode carrier = "AA";
    _seg3->bookedCabin().setEconomyClass();

    CPPUNIT_ASSERT(!_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckCabinWhenOneOfSegmentsHasSameOperCxrAndDifferentCabinThenS7()
  {
    setupValidCaseForCabin();
    const Indicator cabin = 'F';
    const CarrierCode carrier = "AA";
    _optSrvValidator->_isMarketingCxr = false;
    _seg3->bookedCabin().setEconomyClass();

    CPPUNIT_ASSERT(!_optSrvValidator->checkCabin(cabin, carrier, *_ocFee));
  }

  void testCheckFrequentFlyerStatus_noFFStat()
  {
    _optSvcInfo->frequentFlyerStatus() = 0;
    CPPUNIT_ASSERT(_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_ocFee->isFeeGuaranteed());
  }

  void testCheckFrequentFlyerStatus_lowerStatus_matchS7CXR()
  {
    createFFData(4, "AA", 3, "AA");
    CPPUNIT_ASSERT(!_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_ocFee->isFeeGuaranteed());
  }

  void testCheckFrequentFlyerStatus_sameStatus_matchS7CXR()
  {
    createFFData(3, "AA", 3, "AA");
    CPPUNIT_ASSERT(_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(!_ocFee->isFeeGuaranteed());
  }

  void testCheckFrequentFlyerStatus_higherStatus_matchS7CXR()
  {
    createFFData(2, "AA", 3, "AA");
    CPPUNIT_ASSERT(_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(!_ocFee->isFeeGuaranteed());
  }

  void testCheckFrequentFlyerStatus_lowerStatus_noMatchS7CXR()
  {
    createFFData(4, "AA", 3, "XX");
    CPPUNIT_ASSERT(!_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_ocFee->isFeeGuaranteed());
  }

  void testCheckFrequentFlyerStatus_sameStatus_noMatchS7CXR()
  {
    createFFData(3, "AA", 3, "XX");
    CPPUNIT_ASSERT(!_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_ocFee->isFeeGuaranteed());
  }

  void testCheckFrequentFlyerStatus_higherStatus_noMatchS7CXR()
  {
    createFFData(2, "AA", 3, "XX");
    CPPUNIT_ASSERT(!_optSrvValidator->checkFrequentFlyerStatus(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_ocFee->isFeeGuaranteed());
  }

  void testSetSrvInfo_vendor_ATP_Fee_IS_Guarantee()
  {
    _optSvcInfo->vendor() = ATPCO_VENDOR_CODE;
    _optSrvValidator->setSrvInfo(*_optSvcInfo, *_ocFee);
    CPPUNIT_ASSERT(_ocFee->isFeeGuaranteed());
  }

  void testSetSrvInfo_vendor_MMGR_Fee_NOT_Guarantee()
  {
    _optSvcInfo->vendor() = MERCH_MANAGER_VENDOR_CODE;
    _optSrvValidator->setSrvInfo(*_optSvcInfo, *_ocFee);
    CPPUNIT_ASSERT(!_ocFee->isFeeGuaranteed());
  }

  void testCheckEquipment_empty_MATCH()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkEquipmentType(*_optSvcInfo, *_ocFee));
  }

  void testCheckEquipment_BLANK_MATCH()
  {
    _optSvcInfo->equipmentCode() = "";
    CPPUNIT_ASSERT(_optSrvValidator->checkEquipmentType(*_optSvcInfo, *_ocFee));
  }

  void testCheckEquipment_NotFLIGHT_NOMATCH()
  {
    _optSvcInfo->equipmentCode() = "888";
    _optSvcInfo->fltTktMerchInd() = TICKET_RELATED_SERVICE;
    CPPUNIT_ASSERT(!_optSrvValidator->checkEquipmentType(*_optSvcInfo, *_ocFee));
  }

  void testCheckEquipment_PortionSameAsS7_MATCH()
  {
    _optSvcInfo->equipmentCode() = "777";
    _optSvcInfo->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE; i++)
    {
      (*i)->equipmentType() = _optSvcInfo->equipmentCode();
    }
    CPPUNIT_ASSERT(_optSrvValidator->checkEquipmentType(*_optSvcInfo, *_ocFee));
  }

  void testCheckEquipment_PortionSomeNotSameAsS7_NoMATCH()
  {
    _optSvcInfo->equipmentCode() = "777";
    _optSvcInfo->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    _seg1->equipmentType() = "313";
    _seg3->equipmentType() = "331";
    CPPUNIT_ASSERT(!_optSrvValidator->checkEquipmentType(*_optSvcInfo, *_ocFee));
  }

  void testCheckServiceNotAvailNoCharge_BLANK_APPLY_SERVICE_FEE()
  {
    _optSvcInfo->notAvailNoChargeInd() = BLANK;
    CPPUNIT_ASSERT(_optSrvValidator->checkServiceNotAvailNoCharge(*_optSvcInfo, *_ocFee) ==
                   PASS_S7);
  }

  void testCheckServiceNotAvailNoCharge_X_SERVICE_NOT_AVAILABLE()
  {
    _optSvcInfo->notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_NOT_AVAILABLE;
    CPPUNIT_ASSERT(_optSrvValidator->checkServiceNotAvailNoCharge(*_optSvcInfo, *_ocFee) ==
                   PASS_S7_NOT_AVAIL);
  }

  void testCheckServiceNotAvailNoCharge_F_SERVICE_FREE_NO_EMD_ISSUED()
  {
    _optSvcInfo->notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED;
    CPPUNIT_ASSERT(_optSrvValidator->checkServiceNotAvailNoCharge(*_optSvcInfo, *_ocFee) ==
                   PASS_S7_FREE_SERVICE);
  }

  void testCheckServiceNotAvailNoCharge_E_SERVICE_FREE_EMD_ISSUED()
  {
    _optSvcInfo->notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_FREE_EMD_ISSUED;
    CPPUNIT_ASSERT(_optSrvValidator->checkServiceNotAvailNoCharge(*_optSvcInfo, *_ocFee) ==
                   PASS_S7_FREE_SERVICE);
  }

  void testCheckServiceNotAvailNoCharge_G_SERVICE_FREE_NO_BOOK_NO_EMD()
  {
    _optSvcInfo->notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_FREE_NO_BOOK_NO_EMD;
    CPPUNIT_ASSERT(_optSrvValidator->checkServiceNotAvailNoCharge(*_optSvcInfo, *_ocFee) ==
                   PASS_S7_FREE_SERVICE);
  }

  void testCheckServiceNotAvailNoCharge_H_SERVICE_FREE_NO_BOOK_EMD_ISSUED()
  {
    _optSvcInfo->notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_FREE_NO_BOOK_EMD_ISSUED;
    CPPUNIT_ASSERT(_optSrvValidator->checkServiceNotAvailNoCharge(*_optSvcInfo, *_ocFee) ==
                   PASS_S7_FREE_SERVICE);
  }

  void testCheckResultingFareClassWhenEmptyItemNo()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkResultingFareClass("ATP", 0, *_ocFee));
  }

  void testIsValidResultingFareClassWhenFailsOnCarrier()
  {
    SvcFeesCxrResultingFCLInfo fclInfo;

    _ptf1->fareMarket() = _fm1;
    _fm1->governingCarrier() = "AA";
    fclInfo.carrier() = "UA";

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_CXR,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testIsValidResultingFareClassWhenFailsOnFareClass()
  {
    _trx->setRequest(_memHandle.create<AncRequest>());
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);
    fclInfo.resultingFCL() = "Y";

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_FARE_CLASS,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testIsValidResultingFareClassWhenPassOnFareClassOverridedBySpecifiedFareClass()
  {
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);
    fclInfo.resultingFCL() = "Y";
    _ptf1->fareMarket()->travelSeg().push_back(_seg1);
    _seg1->specifiedFbc() = "Y";

    CPPUNIT_ASSERT_EQUAL(PASS_T171,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testIsValidResultingFareClassWhenFailsOnFareType()
  {
    setTicketingAgent();
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);
    fclInfo.fareType() = "ER";

    CPPUNIT_ASSERT_EQUAL(FAIL_NO_FARE_TYPE,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testIsValidResultingFareClassWhenPass()
  {
    setTicketingAgent();
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);

    CPPUNIT_ASSERT_EQUAL(PASS_T171,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testIsValidResultingFareClassWhenEmptyFareClass()
  {
    setTicketingAgent();
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);
    fclInfo.resultingFCL() = "";

    CPPUNIT_ASSERT_EQUAL(PASS_T171,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testIsValidResultingFareClassWhenEmptyFareType()
  {
    setTicketingAgent();
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);
    fclInfo.fareType() = "";

    CPPUNIT_ASSERT_EQUAL(PASS_T171,
                         _optSrvValidator->isValidResultingFareClass(_ptf1, fclInfo, *_ocFee));
  }

  void testCheckResultingFareClassWhenEmptyTS2SS()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->checkResultingFareClass("ATP", 1, *_ocFee));
  }

  void testCheckResultingFareClassWhenPassForOneFare()
  {
    setTicketingAgent();
    createTsMap();
    setupFareAndResultingFareClassInfo(
        *_ptf1,
        *_fareInfo1,
        *_fm1,
        *_fareClassAppInfo1,
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo2);

    CPPUNIT_ASSERT(_optSrvValidator->checkResultingFareClass("ATP", 1, *_ocFee));
  }

  void testCheckResultingFareClassWhenFailForOneFare()
  {
    createTsMap();
    setupFareAndResultingFareClassInfo(
        *_ptf1,
        *_fareInfo1,
        *_fm1,
        *_fareClassAppInfo1,
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo2);
    _fm1->governingCarrier() = "UA";

    CPPUNIT_ASSERT(!_optSrvValidator->checkResultingFareClass("ATP", 1, *_ocFee));
  }

  void testCheckResultingFareClassWhenPassForTwoFares()
  {
    setTicketingAgent();
    createTsMap(true);
    setupFareAndResultingFareClassInfo(
        *_ptf1,
        *_fareInfo1,
        *_fm1,
        *_fareClassAppInfo1,
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo2);
    setupFareAndResultingFareClassInfo(
        *_ptf2,
        *_fareInfo2,
        *_fm2,
        *_fareClassAppInfo2,
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo1);
    _fm2->governingCarrier() = "UA";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo1.carrier() = "UA";

    CPPUNIT_ASSERT(_optSrvValidator->checkResultingFareClass("ATP", 1, *_ocFee));
  }

  void testCheckResultingFareClassWhenFailForTwoFares()
  {
    setTicketingAgent();
    createTsMap(true);
    setupFareAndResultingFareClassInfo(
        *_ptf1,
        *_fareInfo1,
        *_fm1,
        *_fareClassAppInfo1,
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo2);
    setupFareAndResultingFareClassInfo(
        *_ptf2,
        *_fareInfo2,
        *_fm2,
        *_fareClassAppInfo2,
        ((OptionalServicesValidatorMock*)_optSrvValidator)->_resFCInfo1);
    _fm2->governingCarrier() = "UA";

    CPPUNIT_ASSERT(!_optSrvValidator->checkResultingFareClass("ATP", 1, *_ocFee));
  }

  void testIsValidFareClassFareTypeWhenPassOnFareClassOverridedBySpecifiedFareClass()
  {
    SvcFeesCxrResultingFCLInfo fclInfo;

    setupFareAndResultingFareClassInfo(*_ptf1, *_fareInfo1, *_fm1, *_fareClassAppInfo1, fclInfo);
    fclInfo.resultingFCL() = "Y";
    _ptf1->fareMarket()->travelSeg().push_back(_seg1);
    _seg1->specifiedFbc() = "Y";

    CPPUNIT_ASSERT_EQUAL(PASS_T171,
                         _optSrvValidator->isValidFareClassFareType(_ptf1, fclInfo, *_ocFee));
  }

  // Start - Stop Time Unit Tests start
  void test_checkStartStopTime_Empty()
  {
    bool skipDOWCheck = false;
    CPPUNIT_ASSERT(_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
  }

  void test_checkStartStopTime_TAIsT_False()
  {
    bool skipDOWCheck = false;
    createDiagnostic(true);
    _optSvcInfo->timeApplication() = 'T';
    CPPUNIT_ASSERT(!_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO START TIME") != string::npos);
  }

  void test_checkStartStopTime_TAIsD_True_NoData()
  {
    bool skipDOWCheck = false;
    _optSvcInfo->timeApplication() = 'D';
    CPPUNIT_ASSERT(_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
  }

  void test_checkStartStopTime_TAIsR_True_NoData()
  {
    bool skipDOWCheck = false;
    _optSvcInfo->timeApplication() = 'R';
    CPPUNIT_ASSERT(_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
  }

  void test_checkStartStopTime_TAIsD_False_NoData()
  {
    bool skipDOWCheck = false;
    createDiagnostic(true);
    _optSvcInfo->timeApplication() = 'D';
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->fltTktMerchInd() = MERCHANDISE_SERVICE;
    _optSvcInfo->dayOfWeek() = "1";
    CPPUNIT_ASSERT(!_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
    CPPUNIT_ASSERT(skipDOWCheck);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO DOW") != string::npos);
  }

  void test_checkStartStopTime_TAIsR_False_NoData()
  {
    bool skipDOWCheck = false;
    createDiagnostic(true);
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->dayOfWeek() = "1";
    CPPUNIT_ASSERT(!_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
    CPPUNIT_ASSERT(skipDOWCheck);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO DOW") != string::npos);
  }

  void test_checkStartStopTime_TAIsD_True_Data()
  {
    bool skipDOWCheck = false;
    _optSvcInfo->timeApplication() = 'D';
    settestDataforStartStopTime();
    CPPUNIT_ASSERT(_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
  }

  void test_checkStartStopTime_TAIsR_True_Data()
  {
    bool skipDOWCheck = false;
    _optSvcInfo->timeApplication() = 'R';
    settestDataforStartStopTime();
    CPPUNIT_ASSERT(_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
  }

  void test_checkStartStopTime_TAIsD_False_Data()
  {
    bool skipDOWCheck = false;
    createDiagnostic(true);
    _optSvcInfo->timeApplication() = 'D';
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->fltTktMerchInd() = MERCHANDISE_SERVICE;
    settestDataforStartStopTime();
    CPPUNIT_ASSERT(!_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO DOW") != string::npos);
    CPPUNIT_ASSERT(skipDOWCheck);
  }

  void test_checkStartStopTime_TAIsR_False_Data()
  {
    bool skipDOWCheck = false;
    createDiagnostic(true);
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->dayOfWeek() = "1";
    settestDataforStartStopTime();
    CPPUNIT_ASSERT(!_optSrvValidator->checkStartStopTime(*_optSvcInfo, skipDOWCheck, *_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO DOW") != string::npos);
    CPPUNIT_ASSERT(skipDOWCheck);
  }

  void test_convertMinutesSinceMidnightToActualTime_Empty()
  {
    CPPUNIT_ASSERT(_optSrvValidator->convertMinutesSinceMidnightToActualTime("0") == 0);
  }

  void test_convertMinutesSinceMidnightToActualTime_Value()
  {
    CPPUNIT_ASSERT(_optSrvValidator->convertMinutesSinceMidnightToActualTime("0630") == 1030);
  }

  void test_validateStartAndStopTime_False()
  {
    (*_segI)->pssDepartureTime() = "0630";
    createDiagnostic(true);
    CPPUNIT_ASSERT(!_optSrvValidator->validateStartAndStopTime(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO START TIME") != string::npos);
  }

  void test_validateStartAndStopTime_True()
  {
    settestDataforStartStopTime();
    CPPUNIT_ASSERT(_optSrvValidator->validateStartAndStopTime(*_optSvcInfo, *_ocFee));
  }

  void test_validateStartAndStopTime_True_Dup()
  {
    CPPUNIT_ASSERT(_optSrvValidator->validateStartAndStopTime(*_ocFee));
  }

  void test_validateStartAndStopTime_True_ExactTime()
  {
    settestDataforStartStopTime_Same();
    CPPUNIT_ASSERT(_optSrvValidator->validateStartAndStopTime(*_optSvcInfo, *_ocFee));
  }

  void test_validateDOWAndStartAndStopTime_TAIsD_True()
  {
    settestDataforStartStopTime();
    _optSvcInfo->timeApplication() = 'D';
    _optSvcInfo->dayOfWeek() = "";

    CPPUNIT_ASSERT(_optSrvValidator->validateDOWAndStartAndStopTime(*_optSvcInfo, *_ocFee));
  }

  void test_validateDOWAndStartAndStopTime_TAIsR_True()
  {
    settestDataforStartStopTime();
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->dayOfWeek() = "";

    CPPUNIT_ASSERT(_optSrvValidator->validateDOWAndStartAndStopTime(*_optSvcInfo, *_ocFee));
  }

  void test_validateDOWAndStartAndStopTime_TAIsR_False()
  {
    settestDataforStartStopTime();
    createDiagnostic(true);
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->sectorPortionInd() = BLANK;

    CPPUNIT_ASSERT(!_optSrvValidator->validateDOWAndStartAndStopTime(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO DOW") != string::npos);
  }

  void test_checkDOWRangeAndTime_False()
  {
    _optSvcInfo->timeApplication() = 'R';
    createDiagnostic(true);
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    _optSvcInfo->dayOfWeek() = "345";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1215;
    (*_segI)->departureDT() = DateTime(2011, 6, 8, 9, 30, 0);
    (*_segI)->pssDepartureTime() = "570";
    CPPUNIT_ASSERT(!_optSrvValidator->checkDOWRangeAndTime(*_optSvcInfo, *_ocFee));
  }

  void test_checkDOWRangeAndTime_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    _optSvcInfo->dayOfWeek() = "123";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1215;
    (*_segI)->departureDT() = DateTime(2011, 4, 4, 10, 30, 0);
    (*_segI)->pssDepartureTime() = "630";

    CPPUNIT_ASSERT(_optSrvValidator->checkDOWRangeAndTime(*_optSvcInfo, *_ocFee));
  }

  void test_checkDOWRangeAndTime2_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    _optSvcInfo->dayOfWeek() = "71";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1215;
    (*_segI)->departureDT() = DateTime(2011, 5, 16, 10, 30, 0);
    (*_segI)->pssDepartureTime() = "630";

    CPPUNIT_ASSERT(_optSrvValidator->checkDOWRangeAndTime(*_optSvcInfo, *_ocFee));
  }

  void test_checkDOWRangeAndTime_In_Between_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    _optSvcInfo->dayOfWeek() = "123";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1215;
    (*_segI)->departureDT() = DateTime(2011, 4, 5, 9, 30, 0);

    CPPUNIT_ASSERT(!_optSrvValidator->checkDOWRangeAndTime(*_optSvcInfo, *_ocFee));
  }

  void test_checkDOWRangeAndTime_RangeSingleValue_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    _optSvcInfo->dayOfWeek() = "3";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1215;
    (*_segI)->departureDT() = DateTime(2010, 6, 8, 10, 30, 0);

    CPPUNIT_ASSERT(!_optSrvValidator->checkDOWRangeAndTime(*_optSvcInfo, *_ocFee));
  }

  void test_checkDOWRangeAndTime_RangeMultiValue_With_Space_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    _optSvcInfo->dayOfWeek() = " 1234";
    _optSvcInfo->startTime() = 1000;
    _optSvcInfo->stopTime() = 1215;
    (*_segI)->departureDT() = DateTime(2010, 5, 11, 8, 30, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkDOWRangeAndTime(*_optSvcInfo, *_ocFee));
  }

  void test_checkDOWRange_False()
  {
    _optSvcInfo->timeApplication() = 'R';
    createDiagnostic(true);
    _optSvcInfo->dayOfWeek() = "345";
    (*_segI)->departureDT() = DateTime(2010, 6, 8, 10, 30, 0);

    CPPUNIT_ASSERT(!_optSrvValidator->checkDOWRange(*_optSvcInfo));
  }

  void test_checkDOWRange_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->dayOfWeek() = "123";
    (*_segI)->departureDT() = DateTime(2010, 6, 8, 10, 30, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkDOWRange(*_optSvcInfo));
  }

  void test_checkDOWRange2_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->dayOfWeek() = "71";
    (*_segI)->departureDT() = DateTime(2010, 6, 7, 10, 30, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkDOWRange(*_optSvcInfo));
  }

  void test_checkDOWRange_RangeSingleValue_Fail()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->dayOfWeek() = "2";
    (*_segI)->departureDT() = DateTime(2010, 6, 15, 10, 30, 0);

    CPPUNIT_ASSERT(!_optSrvValidator->checkDOWRange(*_optSvcInfo));
  }

  void test_checkDOWRange_RangeMultiValue_With_Space_True()
  {
    _optSvcInfo->timeApplication() = 'R';
    _optSvcInfo->dayOfWeek() = " 1234";
    (*_segI)->departureDT() = DateTime(2010, 6, 15, 10, 30, 0);

    CPPUNIT_ASSERT(_optSrvValidator->checkDOWRange(*_optSvcInfo));
  }

  void test_displayStartStopTimeOrDOWFailDetail_False()
  {
    createDiagnostic(true);
    _optSrvValidator->displayStartStopTimeOrDOWFailDetail(false);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO START TIME") != string::npos);
  }

  void test_displayStartStopTimeOrDOWFailDetail_True()
  {
    createDiagnostic(true);
    _optSrvValidator->displayStartStopTimeOrDOWFailDetail(true);
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find("FAILED DUE TO DOW") != string::npos);
  }
  // Start - Stop Time Unit Tests End

  void testcheckDOW_empty_MATCH() { CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo)); }

  void testcheckDOW_BLANK_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "";
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISBLANK_NOMATCH()
  {
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->sectorPortionInd() = BLANK;
    _optSvcInfo->fltTktMerchInd() = MERCHANDISE_SERVICE;
    CPPUNIT_ASSERT(!_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 03, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_NOMATCH()
  {
    _optSvcInfo->dayOfWeek() = "5";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 03, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_MONDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "1";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 24, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_TUESDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "2";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 25, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_WEDNESDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "3";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 26, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_THURSDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "4";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 27, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_FRIDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "5";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 28, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_SATURDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "6";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 29, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  void testcheckDOW_FLIGHT_SECTORPORTIONINDISSECTOR_SUNDAY_MATCH()
  {
    _optSvcInfo->dayOfWeek() = "7";
    _optSvcInfo->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_SECTOR;
    (*_segI)->departureDT() = DateTime(2010, 5, 30, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkDOW(*_optSvcInfo));
  }

  // tour code tests

  void testcheckTourCode_empty_MATCH()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_BLANK_MATCH()
  {
    _optSvcInfo->tourCode() = "";
    CPPUNIT_ASSERT(_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_WhenInS7AndNoneInCat35orCat27_NOMATCH()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CPPUNIT_ASSERT(!_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_NOMATCH_WhenDiffS7AndCat35()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "PQRST";
    _farePath->collectedNegFareData() = &negFareData;
    CPPUNIT_ASSERT(!_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_MATCH_WhenSameS7AndCat35()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "ABCDE";
    _farePath->collectedNegFareData() = &negFareData;
    CPPUNIT_ASSERT(_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_NOMATCH_WhenDiffS7AndCat35Blank()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "";
    _farePath->collectedNegFareData() = &negFareData;
    CPPUNIT_ASSERT(!_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_NOMATCH_WhenDiffS7AndCat27Blank()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    _farePath->cat27TourCode() = "";
    CPPUNIT_ASSERT(!_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_NOMATCH_WhenDiffS7AndCat27()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    _farePath->cat27TourCode() = "PQRST";
    CPPUNIT_ASSERT(!_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_MATCH_WhenSameS7AndCat27()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    _farePath->cat27TourCode() = "ABCDE";
    CPPUNIT_ASSERT(_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_MATCH_WhenDiffS7AndCat35SameCat27()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "PQRST";
    _farePath->collectedNegFareData() = &negFareData;
    _farePath->cat27TourCode() = "ABCDE";
    CPPUNIT_ASSERT(_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_MATCH_WhenSameS7AndCat35DiffCat27()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "ABCDE";
    _farePath->collectedNegFareData() = &negFareData;
    _farePath->cat27TourCode() = "PQRST";
    CPPUNIT_ASSERT(_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  void testcheckTourCode_NOMATCH_WhenDiffS7AndCat35DiffCat27()
  {
    _optSvcInfo->tourCode() = "ABCDE";
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "PQRST";
    _farePath->collectedNegFareData() = &negFareData;
    _farePath->cat27TourCode() = "IJLKM";
    CPPUNIT_ASSERT(!_optSrvValidator->checkTourCode(*_optSvcInfo));
  }

  // Start Adv Pur Tests
  void testcheckAdvPur_empty_MATCH()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_BLANK_MATCH()
  {
    _optSvcInfo->advPurchPeriod() = "";
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Minute_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "30";
    _optSvcInfo->advPurchUnit() = "N";
    (*_segI)->departureDT() = DateTime(2010, 5, 4, 9, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Minute_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "30";
    _optSvcInfo->advPurchUnit() = "N";
    (*_segI)->departureDT() = DateTime(2010, 5, 4, 8, 30, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Hour_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "2";
    _optSvcInfo->advPurchUnit() = "H";
    (*_segI)->departureDT() = DateTime(2010, 5, 4, 11, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Hour_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "3";
    _optSvcInfo->advPurchUnit() = "H";
    (*_segI)->departureDT() = DateTime(2010, 5, 4, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Day_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "3";
    _optSvcInfo->advPurchUnit() = "D";
    (*_segI)->departureDT() = DateTime(2010, 5, 20, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Exact_Day_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "3";
    _optSvcInfo->advPurchUnit() = "D";
    (*_segI)->departureDT() = DateTime(2010, 5, 7, 7, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Day_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "30";
    _optSvcInfo->advPurchUnit() = "D";
    (*_segI)->departureDT() = DateTime(2010, 5, 20, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Month_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "2";
    _optSvcInfo->advPurchUnit() = "M";
    (*_segI)->departureDT() = DateTime(2010, 7, 20, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Exact_Month_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "2";
    _optSvcInfo->advPurchUnit() = "M";
    (*_segI)->departureDT() = DateTime(2010, 7, 4, 7, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_Month_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchPeriod() = "3";
    _optSvcInfo->advPurchUnit() = "M";
    (*_segI)->departureDT() = DateTime(2010, 7, 20, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_01_MON_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "MON";
    (*_segI)->departureDT() = DateTime(2010, 5, 12, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_01_FRI_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "FRI";
    (*_segI)->departureDT() = DateTime(2010, 5, 6, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_01_TUE_SAMEDAY_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "TUE";
    (*_segI)->departureDT() = DateTime(2010, 5, 6, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_01_FRI_SAMEDEP_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "FRI";
    (*_segI)->departureDT() = DateTime(2010, 5, 7, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_10_MON_MATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "10";
    _optSvcInfo->advPurchPeriod() = "MON";
    (*_segI)->departureDT() = DateTime(2010, 7, 15, 10, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_10_FRI_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "10";
    _optSvcInfo->advPurchPeriod() = "FRI";
    (*_segI)->departureDT() = DateTime(2010, 7, 7, 10, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void testcheckAdvPur_10_FRI_SAMEDEP_NOMATCH()
  {
    settestDataforAdvPur();
    _optSvcInfo->advPurchUnit() = "10";
    _optSvcInfo->advPurchPeriod() = "FRI";
    (*_segI)->departureDT() = DateTime(2010, 7, 9, 16, 15, 0);
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPur(*_optSvcInfo, *_ocFee));
  }

  void test_isAdvPurUnitNHDM_ValueN()
  {
    _optSvcInfo->advPurchUnit() = "N";
    _optSvcInfo->advPurchPeriod() = "30";
    CPPUNIT_ASSERT(_optSrvValidator->isAdvPurUnitNHDM(*_optSvcInfo));
  }

  void test_isAdvPurUnitNHDM_ValueH()
  {
    _optSvcInfo->advPurchUnit() = "H";
    _optSvcInfo->advPurchPeriod() = "3";
    CPPUNIT_ASSERT(_optSrvValidator->isAdvPurUnitNHDM(*_optSvcInfo));
  }

  void test_isAdvPurUnitNHDM_ValueD()
  {
    _optSvcInfo->advPurchUnit() = "D";
    _optSvcInfo->advPurchPeriod() = "3";
    CPPUNIT_ASSERT(_optSrvValidator->isAdvPurUnitNHDM(*_optSvcInfo));
  }

  void test_isAdvPurUnitNHDM_ValueM()
  {
    _optSvcInfo->advPurchUnit() = "M";
    _optSvcInfo->advPurchPeriod() = "2";
    CPPUNIT_ASSERT(_optSrvValidator->isAdvPurUnitNHDM(*_optSvcInfo));
  }

  void test_isAdvPurUnitNHDM_Value20Tue()
  {
    _optSvcInfo->advPurchUnit() = "20";
    _optSvcInfo->advPurchPeriod() = "TUE";
    CPPUNIT_ASSERT(!_optSrvValidator->isAdvPurUnitNHDM(*_optSvcInfo));
  }

  void test_getDayOfWeek_SUN() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("SUN") == 0); }

  void test_getDayOfWeek_MON() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("MON") == 1); }

  void test_getDayOfWeek_TUE() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("TUE") == 2); }

  void test_getDayOfWeek_WED() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("WED") == 3); }

  void test_getDayOfWeek_THU() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("THU") == 4); }

  void test_getDayOfWeek_FRI() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("FRI") == 5); }
  void test_getDayOfWeek_SAT() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("SAT") == 6); }

  void test_getDayOfWeek_ABC() { CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("ABC") == -1); }

  void test_getDayOfWeek_WED_NOMATCH()
  {
    CPPUNIT_ASSERT(_optSrvValidator->getDayOfWeek("WED") != 2);
  }

  void test_getAdvPurPeriod_TUE_MON()
  {
    uint32_t days = 0; // these two are for derived M70
    int advPinDays = 0;
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "MON";
    DateTime curDate = DateTime(2010, 5, 25, 8, 15, 0);
    string ret = _optSrvValidator->getAdvPurPeriod(*_optSvcInfo, curDate, days, advPinDays);
    CPPUNIT_ASSERT(ret == "6");
  }

  void test_getAdvPurPeriod_WED_FRI()
  {
    uint32_t days = 0; // these two are for derived M70
    int advPinDays = 0;
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "FRI";
    DateTime curDate = DateTime(2010, 5, 26, 8, 15, 0);
    string ret = _optSrvValidator->getAdvPurPeriod(*_optSvcInfo, curDate, days, advPinDays);
    CPPUNIT_ASSERT(ret == "2");
  }

  void test_getAdvPurPeriod_WED_WED()
  {
    uint32_t days = 0; // these two are for derived M70
    int advPinDays = 0;
    _optSvcInfo->advPurchUnit() = "01";
    _optSvcInfo->advPurchPeriod() = "WED";
    DateTime curDate = DateTime(2010, 5, 26, 8, 15, 0);
    string ret = _optSrvValidator->getAdvPurPeriod(*_optSvcInfo, curDate, days, advPinDays);
    CPPUNIT_ASSERT(ret == "0");
  }

  void test_getLocalTime_MATCH()
  {
    ((OptionalServicesValidatorMock*)_optSrvValidator)->tempFlag = false;
    settestDataforAdvPur();
    DateTime calcDate;
    DateTime tktDate = DateTime(2010, 5, 4, 8, 15, 0);
    _optSrvValidator->getLocalTime(calcDate);
    CPPUNIT_ASSERT(calcDate == tktDate);
  }

  void test_getLocalTime_NOMATCH()
  {
    settestDataforAdvPur();
    DateTime calcDate;
    DateTime tktDate = DateTime(2010, 6, 4, 8, 15, 0);
    _optSrvValidator->getLocalTime(calcDate);
    CPPUNIT_ASSERT(calcDate != tktDate);
  }

  void test_getTimeDiff_Zero()
  {
    settestDataforAdvPur();
    DateTime tktDate = DateTime(2010, 6, 4, 8, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->getTimeDiff(tktDate) == 0);
  }

  void test_getTimeDiff_Not_Zero()
  {
    ((OptionalServicesValidatorMock*)_optSrvValidator)->tempFlag = true;
    settestDataforAdvPur();
    DateTime tktDate = DateTime(2010, 6, 4, 8, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->getTimeDiff(tktDate) != 0);
  }
  // End Adv Pur tests

  void testCheckMileageFee_All_Empty_PASS()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_BLANK_T170_Present_PASS()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 10;
    CPPUNIT_ASSERT(_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_BLANK_T170_Present_Mileage_Present_PASS()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 10;
    _optSvcInfo->applicationFee() = 100;
    CPPUNIT_ASSERT(_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_BLANK_T170_NotPresent_Mileage_Present_FAIL()
  {
    _optSvcInfo->applicationFee() = 100;
    CPPUNIT_ASSERT(!_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_A_T170_NotPresent_Mileage_Present_FAIL()
  {
    _optSvcInfo->applicationFee() = 100;
    _optSvcInfo->andOrInd() = 'A';
    CPPUNIT_ASSERT(!_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_A_T170_Present_Mileage_Present_FAIL()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 10;
    _optSvcInfo->applicationFee() = 100;
    _optSvcInfo->andOrInd() = 'A';
    CPPUNIT_ASSERT(!_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_A_T170_Present_NO_Mileage_Present_PASS()
  {
    _optSvcInfo->serviceFeesCurrencyTblItemNo() = 10;
    _optSvcInfo->andOrInd() = 'A';
    CPPUNIT_ASSERT(!_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckMileageFee_AND_A_T170_NOT_Present_NO_Mileage_Present_PASS()
  {
    _optSvcInfo->andOrInd() = 'A';
    CPPUNIT_ASSERT(!_optSrvValidator->checkMileageFee(*_optSvcInfo));
  }

  void testCheckIsValidCarrierFlightT186_MarketingCXR_Present_Any_Flight_PASS_FAIL()
  {
    CarrierFlightSeg t186;
    _seg1->setMarketingCarrierCode("AZ");
    t186.marketingCarrier() = "AZ";
    t186.flt1() = -1;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == PASS_T186);
    _seg1->setMarketingCarrierCode("ZZ");
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == FAIL_ON_MARK_CXR);
  }

  void testCheckIsValidCarrierFlightT186_OperatingCXR_Present_Any_Flight_PASS_FAIL()
  {
    CarrierFlightSeg t186;
    _seg1->setMarketingCarrierCode("ZZ");
    t186.marketingCarrier() = "ZZ";
    _seg1->setOperatingCarrierCode("XX");
    t186.operatingCarrier() = "XX";
    t186.flt1() = -1;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == PASS_T186);
    t186.operatingCarrier() = "XY";
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == FAIL_ON_OPER_CXR);
  }

  void testCheckIsValidCarrierFlightT186_MarketingCXR_Pass_Flight2_Zero_Flight1_FAIL_PASS()
  {
    CarrierFlightSeg t186;
    _seg1->setMarketingCarrierCode("ZZ");
    _seg1->flightNumber() = 4321;
    t186.marketingCarrier() = "ZZ";
    t186.flt2() = 0;
    t186.flt1() = 1234;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == FAIL_ON_FLIGHT);
    t186.flt1() = 4321;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == PASS_T186);
  }

  void testCheckIsValidCarrierFlightT186_MarketingOperatingCXR_Pass_Flight2_Zero_Flight1_FAIL_PASS()
  {
    CarrierFlightSeg t186;
    _seg1->setMarketingCarrierCode("ZZ");
    _seg1->flightNumber() = 4321;
    t186.marketingCarrier() = "ZZ";
    _seg1->setOperatingCarrierCode("XX");
    t186.operatingCarrier() = "XX";
    t186.flt2() = 0;
    t186.flt1() = 1234;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == FAIL_ON_FLIGHT);
    t186.flt1() = 4321;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == PASS_T186);
  }

  void testCheckIsValidCarrierFlightT186_MarketingOperatingCXR_Pass_Flight_Range_FAIL_PASS()
  {
    CarrierFlightSeg t186;
    _seg1->setMarketingCarrierCode("ZZ");
    _seg1->flightNumber() = 4321;
    t186.marketingCarrier() = "ZZ";
    _seg1->setOperatingCarrierCode("XX");
    t186.operatingCarrier() = "XX";
    t186.flt1() = 1234;
    t186.flt2() = 3333;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == FAIL_ON_FLIGHT);
    _seg1->flightNumber() = 1111;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == FAIL_ON_FLIGHT);
    _seg1->flightNumber() = 2222;
    CPPUNIT_ASSERT(_optSrvValidator->isValidCarrierFlight(*_seg1, t186) == PASS_T186);
  }

  void testCheckCarrierFlightApplT186_NoTable_Return_True()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkCarrierFlightApplT186("ATP", 0, *_ocFee));
  }

  void testCheckCarrierFlightApplT186_Table_Exists_No_SegCount_Return_False()
  {
    setupT186CntZero(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_NoCxrFltSegment_Return_False()
  {
    setupT186OneSegment(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186.segs().clear();
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_MarketCxr_NotMatch_Return_False()
  {
    _seg1->setMarketingCarrierCode("UA");
    setupT186OneSegment(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_OperatCxr_NotMatch_Return_False()
  {
    _seg1->setMarketingCarrierCode("CC");
    _seg1->setOperatingCarrierCode("UA");
    setupT186OneSegment(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void
  testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_MatchForSeg1_NoMatchForSeg2_Return_False()
  {
    _seg1->setMarketingCarrierCode("CC");
    _seg1->setOperatingCarrierCode("BB");
    setupT186OneSegment(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void testCheckCarrierFlightApplT186_Table_Exists_One_SegCount_BothSegs_Match_Return_True()
  {
    _seg1->setMarketingCarrierCode("CC");
    _seg1->setOperatingCarrierCode("BB");
    _seg3->setMarketingCarrierCode("CC");
    _seg3->setOperatingCarrierCode("BB");
    setupT186OneSegment(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void
  testCheckCarrierFlightApplT186_Table_Exists_Two_SegCount_1stMatch1st_2ndMatch2nd_Return_True()
  {
    _seg1->setMarketingCarrierCode("CC");
    _seg1->setOperatingCarrierCode("BB");
    _seg3->setMarketingCarrierCode("DD");
    _seg3->flightNumber() = 33;
    setupT186TwoSegments(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void
  testCheckCarrierFlightApplT186_Table_Exists_Two_SegCount_1stMatch1st_2ndNotMatchFlt_Return_False()
  {
    _seg1->setMarketingCarrierCode("CC");
    _seg1->setOperatingCarrierCode("BB");
    _seg3->setMarketingCarrierCode("DD");
    _seg3->flightNumber() = 120;
    setupT186TwoSegments(((OptionalServicesValidatorMock*)_optSrvValidator)->_cxrFlightT186);
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
    _seg3->flightNumber() = 10;
    CPPUNIT_ASSERT(!_optSrvValidator->checkCarrierFlightApplT186("ATP", 1, *_ocFee));
  }

  void testCheckFeeApplication_Pass_Blank()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = OptionalServicesValidator::CHAR_BLANK;
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Pass_Blank2()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = OptionalServicesValidator::CHAR_BLANK2;
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Pass_1_OW()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = '1';
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(300, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Pass_2_RT()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = '2';
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(150, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Pass_3_Item()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = '3';
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Pass_4_Travel()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = '4';
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Pass_5_Yicket()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = '5';
    CPPUNIT_ASSERT(_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _ocFee->feeAmount(), 0.01);
  }

  void testCheckFeeApplication_Fail_Unknown()
  {
    setupForFeeApplication();
    _optSvcInfo->frequentFlyerMileageAppl() = 'X';
    CPPUNIT_ASSERT(!_optSrvValidator->checkFeeApplication(*_optSvcInfo, *_ocFee));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _ocFee->feeAmount(), 0.01);
  }

  void testCheckRule_Pass_Empty()
  {
    createTsMap(true);
    CPPUNIT_ASSERT(_optSrvValidator->checkRule("", *_ocFee));
  }

  void testCheckRule_Fail_NoMatch()
  {
    createTsMap(true);
    CPPUNIT_ASSERT(!_optSrvValidator->checkRule("FAIL", *_ocFee));
  }

  void testCheckRule_Pass_Match()
  {
    createTsMap(true);
    CPPUNIT_ASSERT(_optSrvValidator->checkRule("RULE", *_ocFee));
  }

  void testCheckFareInd_Pass_Blank()
  {
    createTsMap();
    CPPUNIT_ASSERT(_optSrvValidator->checkFareInd(OptionalServicesValidator::CHAR_BLANK));
  }

  void testCheckFareInd_Pass_Blank2()
  {
    createTsMap();
    CPPUNIT_ASSERT(_optSrvValidator->checkFareInd(OptionalServicesValidator::CHAR_BLANK2));
  }

  void testCheckFareInd_Fail_19_22()
  {
    createTsMap();
    CPPUNIT_ASSERT(!_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_19_22));
  }

  void testCheckFareInd_Fail_25()
  {
    createTsMap();
    CPPUNIT_ASSERT(!_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_25));
  }

  void testCheckFareInd_Fail_35()
  {
    createTsMap();
    CPPUNIT_ASSERT(!_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_35));
  }

  void testCheckFareInd_Fail_Other()
  {
    createTsMap();
    CPPUNIT_ASSERT(!_optSrvValidator->checkFareInd('~'));
  }

  void testCheckFareInd_Pass_19()
  {
    createTsMap();
    makeDiscounted(_ptf1, RuleConst::CHILDREN_DISCOUNT_RULE);
    CPPUNIT_ASSERT(_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_19_22));
  }

  void testCheckFareInd_Fail_20()
  {
    createTsMap();
    makeDiscounted(_ptf1, RuleConst::TOUR_DISCOUNT_RULE);
    CPPUNIT_ASSERT(!_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_19_22));
  }

  void testCheckFareInd_Fail_21()
  {
    createTsMap();
    makeDiscounted(_ptf1, RuleConst::AGENTS_DISCOUNT_RULE);
    CPPUNIT_ASSERT(!_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_19_22));
  }

  void testCheckFareInd_Pass_22()
  {
    createTsMap();
    makeDiscounted(_ptf1, RuleConst::OTHER_DISCOUNT_RULE);
    CPPUNIT_ASSERT(_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_19_22));
  }

  void testCheckFareInd_Pass_25()
  {
    createTsMap();
    _ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    CPPUNIT_ASSERT(_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_25));
  }

  void testCheckFareInd_Pass_35()
  {
    createTsMap();
    _ptf1->status().set(PaxTypeFare::PTF_Negotiated);
    CPPUNIT_ASSERT(_optSrvValidator->checkFareInd(OptionalServicesValidator::FARE_IND_35));
  }

  void testCheckRuleTariffIndWhenNoRestriction()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkRuleTariffInd(""));
  }

  void testCheckRuleTariffIndWhenEmptyTS2SS()
  {
    CPPUNIT_ASSERT(
        !_optSrvValidator->checkRuleTariffInd(OptionalServicesValidator::RULE_TARIFF_IND_PUBLIC));
  }

  void testCheckRuleTariffIndWhenPassForPublic()
  {
    createTsMap();
    _tariffCrossRefInfo1->tariffCat() = 0;
    _ptf1->setFare(_fare1);

    CPPUNIT_ASSERT(
        _optSrvValidator->checkRuleTariffInd(OptionalServicesValidator::RULE_TARIFF_IND_PUBLIC));
  }

  void testCheckRuleTariffIndWhenFailForPublic()
  {
    createTsMap();
    _tariffCrossRefInfo1->tariffCat() = RuleConst::PRIVATE_TARIFF;
    _ptf1->setFare(_fare1);

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkRuleTariffInd(OptionalServicesValidator::RULE_TARIFF_IND_PUBLIC));
  }

  void testCheckRuleTariffIndWhenPassForPrivate()
  {
    createTsMap();
    _tariffCrossRefInfo1->tariffCat() = RuleConst::PRIVATE_TARIFF;
    _ptf1->setFare(_fare1);

    CPPUNIT_ASSERT(
        _optSrvValidator->checkRuleTariffInd(OptionalServicesValidator::RULE_TARIFF_IND_PRIVATE));
  }

  void testCheckRuleTariffIndWhenFailForPrivate()
  {
    createTsMap();
    _tariffCrossRefInfo1->tariffCat() = 0;
    _ptf1->setFare(_fare1);

    CPPUNIT_ASSERT(
        !_optSrvValidator->checkRuleTariffInd(OptionalServicesValidator::RULE_TARIFF_IND_PRIVATE));
  }

  void testCheckRuleTariffWhenSpecifiedTariff()
  {
    createTsMap();
    _tariffCrossRefInfo1->ruleTariff() = 20;
    _ptf1->setFare(_fare1);

    CPPUNIT_ASSERT(_optSrvValidator->checkRuleTariff(20, *_ocFee));
  }

  void testCheckRuleTariffWhenSpecifiedTariffForR1()
  {
    createTsMap();
    _fareClassAppInfo1->_ruleTariff = 21;

    CPPUNIT_ASSERT(_optSrvValidator->checkRuleTariff(21, *_ocFee));
  }

  void testCheckRuleTariffWhenFailForTwoFaresWhenSpecifiedTariff()
  {
    createTsMap(true);
    _tariffCrossRefInfo1->ruleTariff() = 2;
    _ptf1->setFare(_fare1);
    _fareClassAppInfo1->_ruleTariff = 3;

    CPPUNIT_ASSERT(!_optSrvValidator->checkRuleTariff(2, *_ocFee));
  }

  void test_CheckRuleTariff_Match()
  {
    CPPUNIT_ASSERT(_optSrvValidator->checkRuleTariff(-1, *_ocFee));
  }

  void testCheckRuleTariffWhenPassForTwoFaresWhenSpecifiedTariff()
  {
    createTsMap(true);
    _tariffCrossRefInfo1->ruleTariff() = 2;
    _ptf1->setFare(_fare1);
    _fareClassAppInfo2->_ruleTariff = 2;

    CPPUNIT_ASSERT(_optSrvValidator->checkRuleTariff(2, *_ocFee));
  }

  void testValidateS7Data_Fail_Collect_Subtract()
  {
    _optSvcInfo->collectSubtractInd() = 'I';
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_COLLECT_SUBTRACT,
                         _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Pass_Collect_Subtract()
  {
    _optSvcInfo->collectSubtractInd() = ' ';
    CPPUNIT_ASSERT_EQUAL(PASS_S7, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Fail_Net_Sell()
  {
    _optSvcInfo->netSellingInd() = 'N';
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_NET_SELL, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Pass_Net_Sell()
  {
    _optSvcInfo->netSellingInd() = ' ';
    CPPUNIT_ASSERT_EQUAL(PASS_S7, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void test_setPBDate_True()
  {
    DateTime purByDate = DateTime(2010, 9, 30, 8, 15, 0);
    CPPUNIT_ASSERT(_optSrvValidator->setPBDate(*_optSvcInfo, *_ocFee, purByDate));
  }

  void testPrintStopAtFirstMatchMsg()
  {
    createDiagnostic(false);
    _optSrvValidator->printStopAtFirstMatchMsg();
    CPPUNIT_ASSERT(_optSrvValidator->_diag->str().find(
                       "STOP AFTER FIRST MATCH - NO GROUP CODE IN THE ENTRY") != string::npos);
  }

  void testIsDOWstringValid_1234_Pass()
  {
    CPPUNIT_ASSERT(_optSrvValidator->isDOWstringValid("1234"));
  }
  void testIsDOWstringValid_67123_Pass()
  {
    CPPUNIT_ASSERT(_optSrvValidator->isDOWstringValid("67123"));
  }
  void testIsDOWstringValid_1567_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("1567"));
  }
  void testIsDOWstringValid_156_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("156"));
  }
  void testIsDOWstringValid_672_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("672"));
  }
  void testIsDOWstringValid_2356_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("2356"));
  }
  void testIsDOWstringValid_17_Fail() { CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("17")); }
  void testIsDOWstringValid_71234567_Fail()
  {
    CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("71234567"));
  }
  void testIsDOWstringValid_1_Fail() { CPPUNIT_ASSERT(!_optSrvValidator->isDOWstringValid("1")); }
  void testCheckAdvPurchaseTktInd_NotTkt_Pass()
  {
    _trx->getOptions()->isTicketingInd() = false;
    _optSvcInfo->advPurchTktIssue() = 'X';
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPurchaseTktInd(*_optSvcInfo));
  }
  void testCheckAdvPurchaseTktInd_AdvPurchY_Pass()
  {
    _optSvcInfo->advPurchTktIssue() = 'Y';
    CPPUNIT_ASSERT(_optSrvValidator->checkAdvPurchaseTktInd(*_optSvcInfo));
  }
  void testCheckAdvPurchaseTktInd_Tkt_AdvPurchX_Fail()
  {
    _optSvcInfo->advPurchTktIssue() = 'X';
    CPPUNIT_ASSERT(!_optSrvValidator->checkAdvPurchaseTktInd(*_optSvcInfo));
  }

  void testValidate_Pass_PrePaid_Journey()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::SEC_POR_IND_JOURNEY);
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_portionBG);
  }

  void testValidate_Pass_PrePaid_Portion()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::SEC_POR_IND_PORTION);
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_LocNotBlank()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);

    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Pass_PrePaid_Blank()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_portionBG);
  }

  void testValidate_Pass_PrePaid_Blank_With_Zero_Zones()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    _optSvcInfo->loc1ZoneTblItemNo() = "0000000";
    _optSvcInfo->loc2ZoneTblItemNo() = "0000000";
    _optSvcInfo->viaLocZoneTblItemNo() = "0000000";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_Zone1Exist()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    _optSvcInfo->loc1ZoneTblItemNo() = "12345";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_Zone2Exist()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    _optSvcInfo->loc2ZoneTblItemNo() = "12345";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_Zone3Exist()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    _optSvcInfo->viaLocZoneTblItemNo() = "12345";
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_StopConnection()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::SCD_CNX_POINT;
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_StopoverUnit()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    _optSvcInfo->stopoverUnit() = 'D';
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Fail_PrePaid_Blank_Loc2Notempty()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::CHAR_BLANK);
    settestCleanDataforPrePaidBaggage();
    _optSvcInfo->loc2().loc() = LOC_NYC;
    _optSvcInfo->loc2().locType() = LOCTYPE_CITY;
    ((OptionalServicesValidatorMock*)_optSrvValidator)->_optSvcInfos.push_back(_optSvcInfo);
    CPPUNIT_ASSERT(!_optSrvValidator->validate(*_ocFee));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void testValidate_Pass_PrePaid_CheckJourneyForBG()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::SEC_POR_IND_JOURNEY);
    CPPUNIT_ASSERT(_optSrvValidator->checkSectorPortionIndForBG(*_optSvcInfo));
    CPPUNIT_ASSERT(_optSrvValidator->_portionBG);
  }

  void testValidate_Pass_PrePaid_CheckPortionForBG()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::SEC_POR_IND_PORTION);
    CPPUNIT_ASSERT(_optSrvValidator->checkSectorPortionIndForBG(*_optSvcInfo));
    CPPUNIT_ASSERT(_optSrvValidator->_portionBG);
  }

  void testValidate_Pass_PrePaid_CheckSectorForBG()
  {
    settestDataforPrePaidBaggage(OptionalServicesValidator::SEC_POR_IND_SECTOR);
    CPPUNIT_ASSERT(!_optSrvValidator->checkSectorPortionIndForBG(*_optSvcInfo));
    CPPUNIT_ASSERT(!_optSrvValidator->_portionBG);
  }

  void settestDataforPrePaidBaggage(const Indicator value)
  {
    _subCodeInfo->fltTktMerchInd() = PREPAID_BAGGAGE;
    _optSvcInfo->sectorPortionInd() = value;
  }

  void settestCleanDataforPrePaidBaggage()
  {
    _optSvcInfo->stopoverUnit() = OptionalServicesValidator::CHAR_BLANK;
    _optSvcInfo->fromToWithinInd() = OptionalServicesValidator::CHAR_BLANK;
    _optSvcInfo->loc1().loc() = "";
    _optSvcInfo->loc1().locType() = ' ';
    _optSvcInfo->loc1ZoneTblItemNo() = "";
    _optSvcInfo->loc2().loc() = "";
    _optSvcInfo->loc2().locType() = ' ';
    _optSvcInfo->loc2ZoneTblItemNo() = "";
    _optSvcInfo->viaLoc().loc() = "";
    _optSvcInfo->viaLoc().locType() = ' ';
    _optSvcInfo->viaLocZoneTblItemNo() = "";
    _optSvcInfo->stopCnxDestInd() = OptionalServicesValidator::CHAR_BLANK;
  }

  void testShouldProcessAdvPur_Pass()
  {
    _optSvcInfo->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    CPPUNIT_ASSERT(_optSrvValidator->shouldProcessAdvPur(*_optSvcInfo));
  }

  void testShouldProcessAdvPur_Fail()
  {
    _optSvcInfo->fltTktMerchInd() = TICKET_RELATED_SERVICE;
    CPPUNIT_ASSERT(!_optSrvValidator->shouldProcessAdvPur(*_optSvcInfo));
  }

  void tastGetLocForSfcValidation()
  {
    Loc loc;
    loc.loc() = "KRK";
    Agent agent;
    agent.agentLocation() = &loc;
    PricingRequest request;
    request.ticketingAgent() = &agent;
    _trx->setRequest(&request);

    CPPUNIT_ASSERT_EQUAL(std::string("KRK"),
                         std::string(_optSrvValidator->getLocForSfcValidation().loc()));
  }

  OptionalServicesInfo* getS7(VendorCode vendor,
                              CarrierCode carrier,
                              uint32_t padisItemNo,
                              const Indicator& upgradeCabin = BLANK)
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();

    s7->vendor() = vendor;
    s7->carrier() = carrier;
    s7->upgrdServiceFeesResBkgDesigTblItemNo() = padisItemNo;
    s7->upgradeCabin() = upgradeCabin;

    s7->createDate() = DateTime(2010, 1, 18);
    s7->effDate() = DateTime(2010, 1, 18);
    s7->ticketEffDate() = DateTime(2010, 1, 18);
    s7->sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_PORTION;
    s7->stopoverUnit() = OptionalServicesValidator::CHAR_BLANK;
    s7->fromToWithinInd() = OptionalServicesValidator::FTW_FROM;
    s7->loc1().loc() = LOC_NYC;
    s7->loc1().locType() = LOCTYPE_CITY;
    s7->expireDate() = DateTime(2010, 1, 19);
    s7->discDate() = DateTime(2010, 1, 19);
    s7->ticketDiscDate() = DateTime(2010, 1, 19);

    return s7;
  }

  void testValidateS7Data_Padis_T198SetToZero_upgradeCabinNotZero()
  {
    _subCodeInfo->serviceGroup() = "SA";

    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";
    _ocFee->segments().clear();

    _optSvcInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 0;
    _optSvcInfo->upgradeCabin() = 10;
    _optSvcInfo->vendor() = "MMGR";

    CPPUNIT_ASSERT_EQUAL(FAIL_S7_UPGRADE, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Padis_T198SetToZero_upgradeCabinSetToZero()
  {
    _subCodeInfo->serviceGroup() = "SA";

    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";
    _ocFee->segments().clear();

    _optSvcInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 0;
    _optSvcInfo->upgradeCabin() = BLANK;
    _optSvcInfo->vendor() = "MMGR";

    CPPUNIT_ASSERT_EQUAL(PASS_S7, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Padis_T198NotZero_upgradeCabinNotZero()
  {
    _subCodeInfo->serviceGroup() = "SA";
    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";
    _ocFee->segments().clear();

    _optSvcInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 70;
    _optSvcInfo->upgradeCabin() = 10;
    _optSvcInfo->vendor() = "MMGR";

    CPPUNIT_ASSERT_EQUAL(FAIL_S7_UPGRADE, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidateS7Data_Padis_T198NotZero_upgradeCabinSetToZero()
  {
    _subCodeInfo->serviceGroup() = "SA";
    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";
    _ocFee->segments().clear();

    _optSvcInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 70;
    _optSvcInfo->upgradeCabin() = BLANK;
    _optSvcInfo->vendor() = "MMGR";

    CPPUNIT_ASSERT_EQUAL(PASS_S7, _optSrvValidator->validateS7Data(*_optSvcInfo, *_ocFee));
  }

  void testValidate_Padis_StopValidatingIfT198NotSet()
  {
    _subCodeInfo->serviceGroup() = "SA";
    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";

    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("ATP", "LH", 10));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LH", 11));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("ATP", "LT", 0));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LO", 70));

    _optSrvValidator->validate(*_ocFee);

    CPPUNIT_ASSERT(_ocFee->segments().size() == 3);
  }

  void testValidate_Padis_T198NotFoundInDB()
  {
    createDiagnostic(true);

    _subCodeInfo->serviceGroup() = "SA";
    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";

    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("ATP", "LH", 10));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LH", 11));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("ATP", "LT", 100));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LO", 11));

    _optSrvValidator->validate(*_ocFee);

    CPPUNIT_ASSERT(_ocFee->segments().size() == 3);
  }

  void testValidate_Padis_severalSequencesT198()
  {
    createDiagnostic(true);

    _subCodeInfo->serviceGroup() = "SA";
    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";

    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("ATP", "LH", 22));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LH", 11));

    _optSrvValidator->validate(*_ocFee);

    CPPUNIT_ASSERT(_ocFee->segments().size() == 2);
    CPPUNIT_ASSERT(_ocFee->segments()[0]->_padisData.size() == 2);
    CPPUNIT_ASSERT(_ocFee->segments()[1]->_padisData.size() == 1);
  }

  void testValidate_Padis_severalSequencesT198_identicalData()
  {
    createDiagnostic(true);

    _subCodeInfo->serviceGroup() = "SA";
    _ocFee->matchedOriginAirport() = "KRK";
    _ocFee->matchedDestinationAirport() = "FRA";

    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("ATP", "LH", 10));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LH", 11));
    ((OptionalServicesValidatorMock*)_optSrvValidator)
        ->_optSvcInfos.push_back(getS7("MMGR", "LO", 70));

    _optSrvValidator->validate(*_ocFee);

    CPPUNIT_ASSERT(_ocFee->segments().size() == 3);
    CPPUNIT_ASSERT(_ocFee->segments()[0]->_padisData.size() == 1);
    CPPUNIT_ASSERT(_ocFee->segments()[1]->_padisData.size() == 1);
    CPPUNIT_ASSERT(_ocFee->segments()[2]->_padisData.size() == 1);
  }

  void test_Padis_comparator()
  {

    SvcFeesResBkgDesigInfo* padis_1 =
        getPadis(_memHandle, "ATP", 1, 1, "LH", "AL", "W", "LS", "BK");
    SvcFeesResBkgDesigInfo* padis_2 =
        getPadis(_memHandle, "ATP", 1, 1, "LH", "W", "KK", "TR", "SG");
    SvcFeesResBkgDesigInfo* padis_3 =
        getPadis(_memHandle, "ATP", 1, 1, "LH", "BK", "AL", "LS", "W");
    SvcFeesResBkgDesigInfo* padis_4 =
        getPadis(_memHandle, "ATP", 1, 1, "LH", "TR", "SG", "W", "KK");

    std::set<SvcFeesResBkgDesigInfo*, PadisComparator> padisSet_1;
    padisSet_1.insert(padis_1);
    padisSet_1.insert(padis_2);
    padisSet_1.insert(padis_3);
    padisSet_1.insert(padis_4);

    CPPUNIT_ASSERT(padisSet_1.size() == 2);

    SvcFeesResBkgDesigInfo* padis1 = getPadis(_memHandle, "ATP", 1, 1, "a", "b");
    SvcFeesResBkgDesigInfo* padis2 = getPadis(_memHandle, "ATP", 1, 1, "c", "d");
    SvcFeesResBkgDesigInfo* padis3 = getPadis(_memHandle, "ATP", 1, 1, "e", "f");

    std::set<SvcFeesResBkgDesigInfo*, PadisComparator> padisSet;
    padisSet.insert(padis1);
    padisSet.insert(padis2);
    padisSet.insert(padis3);

    PadisComparator padisComparator;

    // irreflexivity
    CPPUNIT_ASSERT(!padisComparator(padis1, padis1));
    CPPUNIT_ASSERT(!padisComparator(padis2, padis2));
    CPPUNIT_ASSERT(!padisComparator(padis3, padis3));

    // asymetric and transitive
    CPPUNIT_ASSERT(padisComparator(padis1, padis2));
    CPPUNIT_ASSERT(!padisComparator(padis2, padis1));

    CPPUNIT_ASSERT(padisComparator(padis2, padis3));
    CPPUNIT_ASSERT(!padisComparator(padis3, padis2));

    CPPUNIT_ASSERT(padisComparator(padis1, padis3));
    CPPUNIT_ASSERT(!padisComparator(padis3, padis1));

    CPPUNIT_ASSERT(padisSet.size() == 3);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionalServicesValidatorTest);
}
