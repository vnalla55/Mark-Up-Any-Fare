#ifndef FARE_DISPLAY_COOKING_CODE_EXCEPTION_TEST
#define FARE_DISPLAY_COOKING_CODE_EXCEPTION_TEST

#include <string>
#include <functional>

#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "test/testdata/TestBookingCodeExceptionListFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/include/TestMemHandle.h"

#include "BookingCode/FareDisplayBookingCodeException.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "BookingCode/RBData.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"

namespace tse
{

class FareDisplayTrx;
class PaxTypeFare;
class RBData;

// this class is used by FareDisplayBookingCodeTest.
class FareDisplayBookingCodeExceptionMock : public FareDisplayBookingCodeException
{
public:
  FareDisplayBookingCodeExceptionMock(FareDisplayTrx* trx, PaxTypeFare* ptf, RBData* rbData = 0)
    : FareDisplayBookingCodeException(trx, ptf, rbData),
      _requestedItemNo(-1),
      _requestedTariffNumber(-1)
  {
  }
  int& requestedItemNo() { return _requestedItemNo; }
  VendorCode& requestedVendorCode() { return _requestedVendorCode; }
  CarrierCode& requestedCarrierCode() { return _requestedCarrierCode; }
  TariffNumber& requestedTariffNumber() { return _requestedTariffNumber; }
  RuleNumber& requestedRuleNumber() { return _requestedRuleNumber; }
  Indicator& requestedConventionNo() { return _requestedConventionNo; }
  bool& isT999validation() { return _isT999validation; }

protected:
  BookingCodeExceptionSequenceList _emptyList;
  BookingCodeExceptionSequenceList* _list;

  virtual BookingCodeExceptionSequenceList&
  getBookingCodeExceptionSequence(const VendorCode& vendor, const int itemNo)
  {
    _requestedVendorCode = vendor;
    _requestedItemNo = itemNo;
    _isT999validation = true;
    if (itemNo == 626236)
      _list = TestBookingCodeExceptionListFactory::create("testdata/BCE1.xml");
    else if (itemNo == 546232)
      _list = TestBookingCodeExceptionListFactory::create("testdata/BCE1/BCE_546232.xml");
    else
      _list = &_emptyList;
    return *_list;
  }
  virtual BookingCodeExceptionSequenceList&
  getBookingCodeExceptionSequence(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  Indicator conventionNo,
                                  const DateTime& date)
  {
    _isT999validation = false;
    _requestedVendorCode = vendor;
    _requestedCarrierCode = carrier;
    _requestedTariffNumber = ruleTariff;
    _requestedRuleNumber = rule;
    _requestedConventionNo = conventionNo;
    if (vendor == "ATP" && carrier == "JJ" && ruleTariff == 27 && rule == "FRAR" &&
        conventionNo == '2')
      _list = TestBookingCodeExceptionListFactory::create("testdata/BCE1.xml");
    else if (vendor == "ATP" && carrier == "LG" && ruleTariff == 0 && rule == "0000" &&
             conventionNo == '1')
      _list = TestBookingCodeExceptionListFactory::create("testdata/BCE1/BCE_LG_C1.xml");
    else
      _list = &_emptyList;
    return *_list;
  }
  virtual bool scopeTSIGeo(const TSICode tsi,
                           const LocKey& locKey1,
                           const LocKey& locKey2,
                           const RuleConst::TSIScopeParamType& defaultScope,
                           PricingTrx& trx,
                           const FarePath* farePath,
                           const PricingUnit* pricingUnit,
                           const FareMarket* fareMarket,
                           const DateTime& ticketingDate,
                           RuleUtil::TravelSegWrapperVector& applTravelSegment,
                           const DiagnosticTypes& callerDiag = DiagnosticNone,
                           const VendorCode& vendorCode = "ATP")
  {
    if (tsi == 1)
      return true;
    if (tsi == 2)
      return false;
    if (tsi == 3)
    {
      applTravelSegment.push_back(&_tvw);
      return true;
    }
    return false;
  }
  virtual bool isInLoc(const LocCode& loc,
                       const LocTypeCode& locTypeCode,
                       const LocCode& locCode,
                       const VendorCode& vendorCode = EMPTY_VENDOR,
                       const ZoneType zoneType = RESERVED,
                       GeoTravelType geoTvlType = GeoTravelType::International,
                       LocUtil::ApplicationType applType = LocUtil::OTHER,
                       const DateTime& ticketingDate = DateTime::localTime())
  {
    if (locTypeCode == 'A')
    {
      if (loc == "LON")
        return locCode == "2";
      else if (loc == "MAD")
        return locCode == "2";
      else if (loc == "LUX")
        return locCode == "2";
      else if (loc == "AUH")
        return locCode == "2";
    }
    return loc == locCode;
  }
  virtual bool isInLoc(const Loc& loc,
                       const LocTypeCode& locTypeCode,
                       const LocCode& locCode,
                       const VendorCode& vendorCode = EMPTY_VENDOR,
                       const ZoneType zoneType = RESERVED,
                       LocUtil::ApplicationType applType = LocUtil::OTHER,
                       GeoTravelType geoTvlType = GeoTravelType::International,
                       const CarrierCode& carrier = "",
                       const DateTime& ticketingDate = DateTime::localTime())
  {
    // only city and nation comparsion
    if (locTypeCode == 'N')
      return loc.nation() == locCode;
    if (locTypeCode == 'A')
      return loc.area() == locCode;
    return loc.loc() == locCode;
  }

private:
  int _requestedItemNo;
  VendorCode _requestedVendorCode;
  CarrierCode _requestedCarrierCode;
  TariffNumber _requestedTariffNumber;
  RuleNumber _requestedRuleNumber;
  Indicator _requestedConventionNo;
  bool _isT999validation;
  RuleUtil::TravelSegWrapper _tvw;
};

class FareDisplayBookingCodeExceptionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayBookingCodeExceptionTest);
  CPPUNIT_TEST(testcanRemoveSegmentCondMatch);
  CPPUNIT_TEST(testcanRemoveSegmentCondNotMatch);
  CPPUNIT_TEST(testcanRemoveSegmentRestrictionNotMatch);
  CPPUNIT_TEST(testisValidSequenceSpecifiedBlank);
  CPPUNIT_TEST(testisValidSequenceSpecifiedSpecified);
  CPPUNIT_TEST(testisValidSequenceSpecifiedConstructed);
  CPPUNIT_TEST(testisValidSequenceConstructedBlank);
  CPPUNIT_TEST(testisValidSequenceConstructedSpecified);
  CPPUNIT_TEST(testisValidSequenceConstructedConstructed);
  CPPUNIT_TEST(testisRBSegmentConditionalfltRangeAppl);
  CPPUNIT_TEST(testisRBSegmentConditionalflight1);
  CPPUNIT_TEST(testisRBSegmentConditionalflight2);
  CPPUNIT_TEST(testisRBSegmentConditionalposTsi);
  CPPUNIT_TEST(testisRBSegmentConditionalposLocType);
  CPPUNIT_TEST(testisRBSegmentConditionalsellTktInd);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlEffYear);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlEffMonth);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlEffDay);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlDiscYear);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlDiscMonth);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlDiscDay);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlStartTime);
  CPPUNIT_TEST(testisRBSegmentConditionaltvlEndTime);
  CPPUNIT_TEST(testisRBSegmentConditionalrestrictionTag);
  CPPUNIT_TEST(testisRBSegmentConditionalrestrictionTagB_R6C1);
  CPPUNIT_TEST(testisRBSegmentConditionalrestrictionTagB_R6C2);
  CPPUNIT_TEST(testvalidateCarrierBlank);
  CPPUNIT_TEST(testvalidateCarrierAnyCarrier);
  CPPUNIT_TEST(testvalidateCarrierCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierCarrierYY);
  CPPUNIT_TEST(testvalidateCarrierCarrierXDollarCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierCarrierXDollarCarrierYY);
  CPPUNIT_TEST(testvalidateCarrierAirSegCarrierRB);
  CPPUNIT_TEST(testvalidateCarrierAirSegCarrierFQ);
  CPPUNIT_TEST(testvalidateCarrierConditXDollarCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierConditXDollarCarrierIB);
  CPPUNIT_TEST(testvalidateCarrierConditCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierConditCarrierIB);
  CPPUNIT_TEST(testvalidateCarrierConditAirSegCarrier);
  CPPUNIT_TEST(testvalidateCarrierIndustryBlank);
  CPPUNIT_TEST(testvalidateCarrierIndustryAnyCarrier);
  CPPUNIT_TEST(testvalidateCarrierIndustryCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierIndustryCarrierYY);
  CPPUNIT_TEST(testvalidateCarrierIndustryXDollarCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierIndustryXDollarCarrierYY);
  CPPUNIT_TEST(testvalidateCarrierIndustryAirSegCarrierRB);
  CPPUNIT_TEST(testvalidateCarrierIndustryAirSegCarrierFQ);
  CPPUNIT_TEST(testvalidateCarrierIndustryConditXDollarCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierIndustryConditXDollarCarrierIB);
  CPPUNIT_TEST(testvalidateCarrierIndustryConditCarrierEY);
  CPPUNIT_TEST(testvalidateCarrierIndustryConditCarrierIB);
  CPPUNIT_TEST(testvalidateCarrierIndustryConditAirSegCarrier);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim11);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim12);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim13);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim21);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim22);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim23);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim31);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim32);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Prim33);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec11);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec12);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec13);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec21);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec22);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec23);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec31);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec32);
  CPPUNIT_TEST(testvalidatePortionOfTravelAT_Sec33);
  CPPUNIT_TEST(testvalidatePortionOfTravelCA_PLCA);
  CPPUNIT_TEST(testvalidatePortionOfTravelCA_CAPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelCA_CACA);
  CPPUNIT_TEST(testvalidatePortionOfTravelCO_11);
  CPPUNIT_TEST(testvalidatePortionOfTravelCO_12);
  CPPUNIT_TEST(testvalidatePortionOfTravelCO_21);
  CPPUNIT_TEST(testvalidatePortionOfTravelCO_CACA);
  CPPUNIT_TEST(testvalidatePortionOfTravelCO_CAPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelCO_PLCA);
  CPPUNIT_TEST(testvalidatePortionOfTravelDO_USUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelDO_USCA);
  CPPUNIT_TEST(testvalidatePortionOfTravelDO_CAUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelDO_PLPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelDO_PLUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelDO_USPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_11);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_12);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_13);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_21);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_22);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_23);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_31);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_32);
  CPPUNIT_TEST(testvalidatePortionOfTravelEH_33);
  CPPUNIT_TEST(testvalidatePortionOfTravelFD_USUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelFD_USPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelFD_PLUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelFD_PLPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_11);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_12);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_13);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_21);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_22);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_23);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_31);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_32);
  CPPUNIT_TEST(testvalidatePortionOfTravelFE_33);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri11);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri12);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri13);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri21);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri22);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri23);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri31);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri32);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Pri33);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec11);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec12);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec12);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec21);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec22);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec22);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec31);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec32);
  CPPUNIT_TEST(testvalidatePortionOfTravelPA_Sec32);
  CPPUNIT_TEST(testvalidatePortionOfTravelTB_USUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelTB_USCA);
  CPPUNIT_TEST(testvalidatePortionOfTravelTB_CAUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelTB_PLCA);
  CPPUNIT_TEST(testvalidatePortionOfTravelTB_CAPL);
  CPPUNIT_TEST(testvalidatePortionOfTravelUS_USUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelUS_USCA);
  CPPUNIT_TEST(testvalidatePortionOfTravelUS_CAUS);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_11);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_12);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_13);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_21);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_22);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_23);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_31);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_32);
  CPPUNIT_TEST(testvalidatePortionOfTravelWH_33);
  CPPUNIT_TEST(testvalidatePrimarySecondaryDomesticPrimary);
  CPPUNIT_TEST(testvalidatePrimarySecondaryDomesticSecondary);
  CPPUNIT_TEST(testvalidatePrimarySecondaryDomesticFromTo);
  CPPUNIT_TEST(testvalidatePrimarySecondaryTransborderPrimary);
  CPPUNIT_TEST(testvalidatePrimarySecondaryTransborderSecondary);
  CPPUNIT_TEST(testvalidatePrimarySecondaryTransborderFromTo);
  CPPUNIT_TEST(testvalidatePrimarySecondaryForeignDomesticPrimary);
  CPPUNIT_TEST(testvalidatePrimarySecondaryForeignDomesticSecondary);
  CPPUNIT_TEST(testvalidatePrimarySecondaryForeignDomesticFromTo);
  CPPUNIT_TEST(testvalidatePrimarySecondaryBlank);
  CPPUNIT_TEST(testvalidatePrimarySecondaryPrimary);
  CPPUNIT_TEST(testvalidatePrimarySecondarySecondary);
  CPPUNIT_TEST(testvalidateTSIFail);
  CPPUNIT_TEST(testvalidateTSIPassEmpty);
  CPPUNIT_TEST(testvalidateTSIPassNotEmpty);
  CPPUNIT_TEST(testvalidateLocationDir1_PriBlank);
  CPPUNIT_TEST(testvalidateLocationDir1_PriLUXABC);
  CPPUNIT_TEST(testvalidateLocationDir1_PriABCAUH);
  CPPUNIT_TEST(testvalidateLocationDir1_PriLUXAUH);
  CPPUNIT_TEST(testvalidateLocationDir1_PriAUHLUX);
  CPPUNIT_TEST(testvalidateLocationDir1_PriAUHLUX_RT);
  CPPUNIT_TEST(testvalidateLocationDir1_SecBlank);
  CPPUNIT_TEST(testvalidateLocationDir1_SecLONABC);
  CPPUNIT_TEST(testvalidateLocationDir1_SecABCMAD);
  CPPUNIT_TEST(testvalidateLocationDir1_SecLONMAD);
  CPPUNIT_TEST(testvalidateLocationDir1_SecMADLON);
  CPPUNIT_TEST(testvalidateLocationDir2_PriBlank);
  CPPUNIT_TEST(testvalidateLocationDir2_PriLUXABC);
  CPPUNIT_TEST(testvalidateLocationDir2_PriABCAUH);
  CPPUNIT_TEST(testvalidateLocationDir2_PriLUXAUH);
  CPPUNIT_TEST(testvalidateLocationDir2_PriLUXAUH_RT);
  CPPUNIT_TEST(testvalidateLocationDir2_PriAUHLUX);
  CPPUNIT_TEST(testvalidateLocationDir2_SecBlank);
  CPPUNIT_TEST(testvalidateLocationDir2_SecLONABC);
  CPPUNIT_TEST(testvalidateLocationDir2_SecABCMAD);
  CPPUNIT_TEST(testvalidateLocationDir2_SecLONMAD);
  CPPUNIT_TEST(testvalidateLocationDir2_SecMADLON);
  CPPUNIT_TEST(testvalidateLocationDir3_PriBlank);
  CPPUNIT_TEST(testvalidateLocationDir3_PriLUXABC);
  CPPUNIT_TEST(testvalidateLocationDir3_PriABCAUH);
  CPPUNIT_TEST(testvalidateLocationDir3_PriLUXAUH);
  CPPUNIT_TEST(testvalidateLocationDir3_PriAUHLUX);
  CPPUNIT_TEST(testvalidateLocationDir3_PriAUHLUX_INBOUND);
  CPPUNIT_TEST(testvalidateLocationDir3_SecBlank);
  CPPUNIT_TEST(testvalidateLocationDir3_SecLONMAD);
  CPPUNIT_TEST(testvalidateLocationDir3_SecMADLON);
  CPPUNIT_TEST(testvalidateLocationDir4_PriBlank);
  CPPUNIT_TEST(testvalidateLocationDir4_PriLUXABC);
  CPPUNIT_TEST(testvalidateLocationDir4_PriABCAUH);
  CPPUNIT_TEST(testvalidateLocationDir4_PriLUXAUH);
  CPPUNIT_TEST(testvalidateLocationDir4_PriLUXAUH_INBOUND);
  CPPUNIT_TEST(testvalidateLocationDir4_PriAUHLUX);
  CPPUNIT_TEST(testvalidateLocationDir4_SecBlank);
  CPPUNIT_TEST(testvalidateLocationDir4_SecLONMAD);
  CPPUNIT_TEST(testvalidateLocationDir4_SecMADLON);
  CPPUNIT_TEST(testvalidateLocationDirBlank_PriBlank);
  CPPUNIT_TEST(testvalidateLocationDirBlank_Pri12);
  CPPUNIT_TEST(testvalidateLocationDirBlank_Pri21);
  CPPUNIT_TEST(testvalidateLocationDirBlank_Pri22);
  CPPUNIT_TEST(testvalidateLocationDirBlank_PriLUXABC);
  CPPUNIT_TEST(testvalidateLocationDirBlank_PriABCAUH);
  CPPUNIT_TEST(testvalidateLocationDirBlank_PriLUXAUH);
  CPPUNIT_TEST(testvalidateLocationDirBlank_PriAUHLUX);
  CPPUNIT_TEST(testvalidateLocationDirBlank_SecBlank);
  CPPUNIT_TEST(testvalidateLocationDirBlank_Sec12);
  CPPUNIT_TEST(testvalidateLocationDirBlank_Sec21);
  CPPUNIT_TEST(testvalidateLocationDirBlank_Sec22);
  CPPUNIT_TEST(testvalidateLocationDirBlank_SecLONABC);
  CPPUNIT_TEST(testvalidateLocationDirBlank_SecABCMAD);
  CPPUNIT_TEST(testvalidateLocationDirBlank_SecLONMAD);
  CPPUNIT_TEST(testvalidateLocationDirBlank_SecMADLON);
  CPPUNIT_TEST(testvalidateLocationDirBlankConditional_KRKWAW_WAWLON);
  CPPUNIT_TEST(testvalidateLocationDirBlankConditional_WAWLON_LONMAD);
  CPPUNIT_TEST(testvalidateLocationDirBlankConditional_LONMAD_MADWAW);
  CPPUNIT_TEST(testvalidatePointOfSale);
  CPPUNIT_TEST(testvalidateSoldTag);
  CPPUNIT_TEST(testvalidateDateTimeDOW);
  CPPUNIT_TEST(testvalidateFareClassType_T_FR);
  CPPUNIT_TEST(testvalidateFareClassType_T_PIT);
  CPPUNIT_TEST(testvalidateFareClassType_F_Y);
  CPPUNIT_TEST(testvalidateFareClassType_Y_AOWLU);
  CPPUNIT_TEST(testvalidateFareClassType_M_ABA);
  CPPUNIT_TEST(testvalidateFareClassType_M_AWLU);
  CPPUNIT_TEST(testvalidateFareClassType_A_B);
  CPPUNIT_TEST(testvalidateFareClassType_A_A);
  CPPUNIT_TEST(testchangeFareBasisCodeFailNotPermitted);
  CPPUNIT_TEST(testchangeFareBasisCodeFailConditional);
  CPPUNIT_TEST(testchangeFareBasisCodeFailRestrictionTag);
  CPPUNIT_TEST(testchangeFareBasisCodePass);

  CPPUNIT_TEST_SUITE_END();

public:
  void testcanRemoveSegmentCondMatch();
  void testcanRemoveSegmentCondNotMatch();
  void testcanRemoveSegmentRestrictionNotMatch();
  void testisValidSequenceSpecifiedBlank();
  void testisValidSequenceSpecifiedSpecified();
  void testisValidSequenceSpecifiedConstructed();
  void testisValidSequenceConstructedBlank();
  void testisValidSequenceConstructedSpecified();
  void testisValidSequenceConstructedConstructed();
  void testisRBSegmentConditionalfltRangeAppl();
  void testisRBSegmentConditionalflight1();
  void testisRBSegmentConditionalflight2();
  void testisRBSegmentConditionalposTsi();
  void testisRBSegmentConditionalposLocType();
  void testisRBSegmentConditionalsellTktInd();
  void testisRBSegmentConditionaltvlEffYear();
  void testisRBSegmentConditionaltvlEffMonth();
  void testisRBSegmentConditionaltvlEffDay();
  void testisRBSegmentConditionaltvlDiscYear();
  void testisRBSegmentConditionaltvlDiscMonth();
  void testisRBSegmentConditionaltvlDiscDay();
  void testisRBSegmentConditionaltvlStartTime();
  void testisRBSegmentConditionaltvlEndTime();
  void testisRBSegmentConditionalrestrictionTag();
  void testisRBSegmentConditionalrestrictionTagB_R6C1();
  void testisRBSegmentConditionalrestrictionTagB_R6C2();
  void testvalidateCarrierBlank();
  void testvalidateCarrierAnyCarrier();
  void testvalidateCarrierCarrierEY();
  void testvalidateCarrierCarrierYY();
  void testvalidateCarrierCarrierXDollarCarrierEY();
  void testvalidateCarrierCarrierXDollarCarrierYY();
  void testvalidateCarrierAirSegCarrierRB();
  void testvalidateCarrierAirSegCarrierFQ();
  void testvalidateCarrierConditXDollarCarrierEY();
  void testvalidateCarrierConditXDollarCarrierIB();
  void testvalidateCarrierConditCarrierEY();
  void testvalidateCarrierConditCarrierIB();
  void testvalidateCarrierConditAirSegCarrier();
  void testvalidateCarrierIndustryBlank();
  void testvalidateCarrierIndustryAnyCarrier();
  void testvalidateCarrierIndustryCarrierEY();
  void testvalidateCarrierIndustryCarrierYY();
  void testvalidateCarrierIndustryXDollarCarrierEY();
  void testvalidateCarrierIndustryXDollarCarrierYY();
  void testvalidateCarrierIndustryAirSegCarrierRB();
  void testvalidateCarrierIndustryAirSegCarrierFQ();
  void testvalidateCarrierIndustryConditXDollarCarrierEY();
  void testvalidateCarrierIndustryConditXDollarCarrierIB();
  void testvalidateCarrierIndustryConditCarrierEY();
  void testvalidateCarrierIndustryConditCarrierIB();
  void testvalidateCarrierIndustryConditAirSegCarrier();
  void testvalidatePortionOfTravelAT_Prim11();
  void testvalidatePortionOfTravelAT_Prim12();
  void testvalidatePortionOfTravelAT_Prim13();
  void testvalidatePortionOfTravelAT_Prim21();
  void testvalidatePortionOfTravelAT_Prim22();
  void testvalidatePortionOfTravelAT_Prim23();
  void testvalidatePortionOfTravelAT_Prim31();
  void testvalidatePortionOfTravelAT_Prim32();
  void testvalidatePortionOfTravelAT_Prim33();
  void testvalidatePortionOfTravelAT_Sec11();
  void testvalidatePortionOfTravelAT_Sec12();
  void testvalidatePortionOfTravelAT_Sec13();
  void testvalidatePortionOfTravelAT_Sec21();
  void testvalidatePortionOfTravelAT_Sec22();
  void testvalidatePortionOfTravelAT_Sec23();
  void testvalidatePortionOfTravelAT_Sec31();
  void testvalidatePortionOfTravelAT_Sec32();
  void testvalidatePortionOfTravelAT_Sec33();
  void testvalidatePortionOfTravelCA_PLCA();
  void testvalidatePortionOfTravelCA_CAPL();
  void testvalidatePortionOfTravelCA_CACA();
  void testvalidatePortionOfTravelCO_11();
  void testvalidatePortionOfTravelCO_12();
  void testvalidatePortionOfTravelCO_21();
  void testvalidatePortionOfTravelCO_CACA();
  void testvalidatePortionOfTravelCO_CAPL();
  void testvalidatePortionOfTravelCO_PLCA();
  void testvalidatePortionOfTravelDO_USUS();
  void testvalidatePortionOfTravelDO_USCA();
  void testvalidatePortionOfTravelDO_CAUS();
  void testvalidatePortionOfTravelDO_PLPL();
  void testvalidatePortionOfTravelDO_PLUS();
  void testvalidatePortionOfTravelDO_USPL();
  void testvalidatePortionOfTravelEH_11();
  void testvalidatePortionOfTravelEH_12();
  void testvalidatePortionOfTravelEH_13();
  void testvalidatePortionOfTravelEH_21();
  void testvalidatePortionOfTravelEH_22();
  void testvalidatePortionOfTravelEH_23();
  void testvalidatePortionOfTravelEH_31();
  void testvalidatePortionOfTravelEH_32();
  void testvalidatePortionOfTravelEH_33();
  void testvalidatePortionOfTravelFD_USUS();
  void testvalidatePortionOfTravelFD_USPL();
  void testvalidatePortionOfTravelFD_PLUS();
  void testvalidatePortionOfTravelFD_PLPL();
  void testvalidatePortionOfTravelFE_11();
  void testvalidatePortionOfTravelFE_12();
  void testvalidatePortionOfTravelFE_13();
  void testvalidatePortionOfTravelFE_21();
  void testvalidatePortionOfTravelFE_22();
  void testvalidatePortionOfTravelFE_23();
  void testvalidatePortionOfTravelFE_31();
  void testvalidatePortionOfTravelFE_32();
  void testvalidatePortionOfTravelFE_33();
  void testvalidatePortionOfTravelPA_Pri11();
  void testvalidatePortionOfTravelPA_Pri12();
  void testvalidatePortionOfTravelPA_Pri13();
  void testvalidatePortionOfTravelPA_Pri21();
  void testvalidatePortionOfTravelPA_Pri22();
  void testvalidatePortionOfTravelPA_Pri23();
  void testvalidatePortionOfTravelPA_Pri31();
  void testvalidatePortionOfTravelPA_Pri32();
  void testvalidatePortionOfTravelPA_Pri33();
  void testvalidatePortionOfTravelPA_Sec11();
  void testvalidatePortionOfTravelPA_Sec12();
  void testvalidatePortionOfTravelPA_Sec13();
  void testvalidatePortionOfTravelPA_Sec21();
  void testvalidatePortionOfTravelPA_Sec22();
  void testvalidatePortionOfTravelPA_Sec23();
  void testvalidatePortionOfTravelPA_Sec31();
  void testvalidatePortionOfTravelPA_Sec32();
  void testvalidatePortionOfTravelPA_Sec33();
  void testvalidatePortionOfTravelTB_USUS();
  void testvalidatePortionOfTravelTB_USCA();
  void testvalidatePortionOfTravelTB_CAUS();
  void testvalidatePortionOfTravelTB_PLCA();
  void testvalidatePortionOfTravelTB_CAPL();
  void testvalidatePortionOfTravelUS_USUS();
  void testvalidatePortionOfTravelUS_USCA();
  void testvalidatePortionOfTravelUS_CAUS();
  void testvalidatePortionOfTravelWH_11();
  void testvalidatePortionOfTravelWH_12();
  void testvalidatePortionOfTravelWH_13();
  void testvalidatePortionOfTravelWH_21();
  void testvalidatePortionOfTravelWH_22();
  void testvalidatePortionOfTravelWH_23();
  void testvalidatePortionOfTravelWH_31();
  void testvalidatePortionOfTravelWH_32();
  void testvalidatePortionOfTravelWH_33();
  void testvalidatePrimarySecondaryDomesticPrimary();
  void testvalidatePrimarySecondaryDomesticSecondary();
  void testvalidatePrimarySecondaryDomesticFromTo();
  void testvalidatePrimarySecondaryTransborderPrimary();
  void testvalidatePrimarySecondaryTransborderSecondary();
  void testvalidatePrimarySecondaryTransborderFromTo();
  void testvalidatePrimarySecondaryForeignDomesticPrimary();
  void testvalidatePrimarySecondaryForeignDomesticSecondary();
  void testvalidatePrimarySecondaryForeignDomesticFromTo();
  void testvalidatePrimarySecondaryBlank();
  void testvalidatePrimarySecondaryPrimary();
  void testvalidatePrimarySecondarySecondary();
  void testvalidateTSIFail();
  void testvalidateTSIPassEmpty();
  void testvalidateTSIPassNotEmpty();
  void testvalidateLocationDir1_PriBlank();
  void testvalidateLocationDir1_PriLUXABC();
  void testvalidateLocationDir1_PriABCAUH();
  void testvalidateLocationDir1_PriLUXAUH();
  void testvalidateLocationDir1_PriAUHLUX();
  void testvalidateLocationDir1_PriAUHLUX_RT();
  void testvalidateLocationDir1_SecBlank();
  void testvalidateLocationDir1_SecLONABC();
  void testvalidateLocationDir1_SecABCMAD();
  void testvalidateLocationDir1_SecLONMAD();
  void testvalidateLocationDir1_SecMADLON();
  void testvalidateLocationDir2_PriBlank();
  void testvalidateLocationDir2_PriLUXABC();
  void testvalidateLocationDir2_PriABCAUH();
  void testvalidateLocationDir2_PriLUXAUH();
  void testvalidateLocationDir2_PriLUXAUH_RT();
  void testvalidateLocationDir2_PriAUHLUX();
  void testvalidateLocationDir2_SecBlank();
  void testvalidateLocationDir2_SecLONABC();
  void testvalidateLocationDir2_SecABCMAD();
  void testvalidateLocationDir2_SecLONMAD();
  void testvalidateLocationDir2_SecMADLON();
  void testvalidateLocationDir3_PriBlank();
  void testvalidateLocationDir3_PriLUXABC();
  void testvalidateLocationDir3_PriABCAUH();
  void testvalidateLocationDir3_PriLUXAUH();
  void testvalidateLocationDir3_PriAUHLUX();
  void testvalidateLocationDir3_PriAUHLUX_INBOUND();
  void testvalidateLocationDir3_SecBlank();
  void testvalidateLocationDir3_SecLONMAD();
  void testvalidateLocationDir3_SecMADLON();
  void testvalidateLocationDir4_PriBlank();
  void testvalidateLocationDir4_PriLUXABC();
  void testvalidateLocationDir4_PriABCAUH();
  void testvalidateLocationDir4_PriLUXAUH();
  void testvalidateLocationDir4_PriLUXAUH_INBOUND();
  void testvalidateLocationDir4_PriAUHLUX();
  void testvalidateLocationDir4_SecBlank();
  void testvalidateLocationDir4_SecLONMAD();
  void testvalidateLocationDir4_SecMADLON();
  void testvalidateLocationDirBlank_PriBlank();
  void testvalidateLocationDirBlank_Pri12();
  void testvalidateLocationDirBlank_Pri21();
  void testvalidateLocationDirBlank_Pri22();
  void testvalidateLocationDirBlank_PriLUXABC();
  void testvalidateLocationDirBlank_PriABCAUH();
  void testvalidateLocationDirBlank_PriLUXAUH();
  void testvalidateLocationDirBlank_PriAUHLUX();
  void testvalidateLocationDirBlank_SecBlank();
  void testvalidateLocationDirBlank_Sec12();
  void testvalidateLocationDirBlank_Sec21();
  void testvalidateLocationDirBlank_Sec22();
  void testvalidateLocationDirBlank_SecLONABC();
  void testvalidateLocationDirBlank_SecABCMAD();
  void testvalidateLocationDirBlank_SecLONMAD();
  void testvalidateLocationDirBlank_SecMADLON();
  void testvalidateLocationDirBlankConditional_KRKWAW_WAWLON();
  void testvalidateLocationDirBlankConditional_WAWLON_LONMAD();
  void testvalidateLocationDirBlankConditional_LONMAD_MADWAW();
  void testvalidatePointOfSale();
  void testvalidateSoldTag();
  void testvalidateDateTimeDOW();
  void testvalidateFareClassType_T_FR();
  void testvalidateFareClassType_T_PIT();
  void testvalidateFareClassType_F_Y();
  void testvalidateFareClassType_Y_AOWLU();
  void testvalidateFareClassType_M_ABA();
  void testvalidateFareClassType_M_AWLU();
  void testvalidateFareClassType_A_B();
  void testvalidateFareClassType_A_A();
  void testchangeFareBasisCodeFailNotPermitted();
  void testchangeFareBasisCodeFailConditional();
  void testchangeFareBasisCodeFailRestrictionTag();
  void testchangeFareBasisCodePass();

  void setUp();
  void tearDown();

protected:
  void initSegment(BookingCodeExceptionSegment* seg);
  void initSequence(BookingCodeExceptionSequence* seq);

  void restoreDirections();
  void setDirectionality(const PaxTypeFare* ptf, GlobalDirection gd);
  void setIATA(const Loc* loc, std::string iata);
  void setNation(const Loc* loc, NationCode nation);
  void setOWRT(PaxTypeFare*, Indicator owrt);
  void setMarketDirection(PaxTypeFare*, FMDirection);

private:
  PaxTypeFare* _ptfFBR;
  PaxTypeFare* _ptfInd;
  FareDisplayBookingCodeExceptionMock* _fbExc;
  FareDisplayBookingCodeExceptionMock* _fbExcInd;
  FareDisplayTrx* _trx;
  FareDisplayRequest* _req;
  RBData* _rb;
  AirSeg* _airSeg;
  BookingCodeExceptionSequence* _seq;
  BookingCodeExceptionSegment* _seg;
  TestMemHandle _memHandle;
};

}; // namespace

#endif
