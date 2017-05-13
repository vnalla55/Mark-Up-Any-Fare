//----------------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "test/DBAccessMock/DataHandleMock.h"

#include "Common/ErrorResponseException.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/TaxText.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingOptions.h"
#include "FareCalc/FareCalcConsts.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/AncillaryPricingResponseFormatter.h"
#include "Xform/PricingResponseXMLTags.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace tse
{
using namespace ::testing;
using ::testing::_;

using GroupFeesVector = AncillaryPricingResponseFormatter::GroupFeesVector;
using GroupFeesUsagesVector = AncillaryPricingResponseFormatter::GroupFeesUsagesVector;

const std::string TAXTEXT_1 = "BAGGAGE IS NOT ALLOWED";
const std::string TAXTEXT_2 = "//";
const std::string TAXTEXT_3 = "//03/0F3";
const std::string TAXTEXT_4 = "//03/0F7";

const std::string TAXTEXT_3055_1 = "LIVE ANIMALS-DOUBLE CHARGE IS APPLICABLE";
const std::string TAXTEXT_3055_2 = "SPECIAL CHARGES APPLY FOR THE 1ST ITEM OF SPECIAL";
const std::string TAXTEXT_3055_3 = "BAGGAGE-";
const std::string TAXTEXT_3055_4 = "SKI EQUIPMENT - RATE FOR 3KG OF EXCESS BAGGAGE";
const std::string TAXTEXT_3055_5 = "GOLF EQUIPMENT - FOR THE FIRST 15KG OF EQUIPMENT -";
const std::string TAXTEXT_3055_6 = "RATE FOR 6KG OF EXCESS BAGGAGE";

class AncillaryPricingResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPricingResponseFormatterTest);

  CPPUNIT_SKIP_TEST(test_readConfigXMLNamespace_empty);
  CPPUNIT_TEST(test_readConfigXMLNamespace_Val);
  CPPUNIT_TEST(test_formatResponse_Empty);
  CPPUNIT_TEST(test_formatResponse_Data);
  CPPUNIT_TEST(test_prepareResponseText_Empty);
  CPPUNIT_TEST(test_prepareResponseText_Data);
  CPPUNIT_TEST(test_prepareMessage_Empty);
  CPPUNIT_TEST(test_prepareMessage_notEmpty);
  CPPUNIT_TEST(test_formatAEFeesResponse_NoItin);
  CPPUNIT_TEST(test_formatAEFeesResponse_Itin_SG);
  CPPUNIT_TEST(test_formatAEFeesResponse_Itin_SG_WPDispAE);
  CPPUNIT_TEST(test_formatAEFeesResponse_Itin_SG_WPDispAE_DP_true);
  CPPUNIT_TEST(test_formatAEFeesResponse_whenHandlingSecondCallForMonetaryDiscount_shouldCallBuildOcFeesSecondResponseForMonetaryDiscount);
  CPPUNIT_TEST(test_formatAEFeesResponse_whenNotHandlingSecondCallForMonetaryDiscount_shouldCallBuildOCFeesResponse);
  CPPUNIT_TEST(test_buildOcFeesSecondResponseForMonetaryDiscount_whenProcessingIsNotStopped_shouldCreateItnTagWithQ00AttributeAndCallFormatOcFees);
  CPPUNIT_TEST(test_buildOcFeesSecondResponseForMonetaryDiscount_whenProcessingIsStopped_shouldDoNothing);
  CPPUNIT_TEST(test_isGenericTrailer_checkIfAnyGroupValid_True);
  CPPUNIT_TEST(test_isGenericTrailer_checkIfAnyGroupValid_False);
  CPPUNIT_TEST(test_createOCGSection);
  CPPUNIT_TEST(test_createOCGSection_Data);
  CPPUNIT_TEST(test_checkIfAnyGroupValid_false);
  CPPUNIT_TEST(test_checkIfAnyGroupValid_true);
  CPPUNIT_TEST(test_formatOCFeesLineForWPDispAE);
  CPPUNIT_TEST(test_formatOCFeesLineForWPDispAE_noIndex);
  CPPUNIT_TEST(test_formatOCFeesLineForWPDispAE_noIndex_NotAvail);
  CPPUNIT_TEST(test_buildOSCData_whenShouldSkipCreatingOscElementReturnsTrue_shouldDoNothing);
  CPPUNIT_TEST(test_buildOSCData_whenShouldSkipCreatingOscElementReturnsFalse_shouldProceedWithCreatingOscElement);
  CPPUNIT_TEST(test_shouldSkipCreatingOscElement_whenNotHandlingSecondCallForMonetaryDiscount_shouldReturnFalse);
  CPPUNIT_TEST(test_shouldSkipCreatingOscElement_givenSecondCallForMonetaryDiscount_whenPaxOcFeesHasUsages_shouldReturnFalse);
  CPPUNIT_TEST(test_shouldSkipCreatingOscElement_givenSecondCallForMonetaryDiscount_whenPaxOcFeesHasNoUsages_shouldReturnTrue);
  CPPUNIT_TEST(test_buildOSCOptionalData);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_WPDispAE_DO_True);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_WPDispAE_DO_False);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_WPDispAE_DO_False_AB240);
  CPPUNIT_TEST(test_buildS7OOSData);
  CPPUNIT_TEST(test_buildS7OOSData_S7);
  CPPUNIT_TEST(test_buildS7OOSData_S7_SP);
  CPPUNIT_TEST(test_buildS7OOSData_S7_SP_AX1);
  CPPUNIT_TEST(test_buildS7OOSData_S7_SP_MOA);
  CPPUNIT_TEST(test_buildS7OOSData_S7_SP_MOA_MDA);
  CPPUNIT_TEST(test_buildS7OOSData_S7_givenPriceModificationForOcFeesSegmentExists_whenBuildingS7OosData_shouldAddPmiAsAttribute);
  CPPUNIT_TEST(test_buildSegments);
  CPPUNIT_TEST(test_buildSegments_multiple);
  CPPUNIT_TEST(test_buildSegments_multiple_Seg_Reverse);
  CPPUNIT_TEST(test_buildSegmentElement);
  CPPUNIT_TEST(test_buildSegmentElement_Arunk);
  CPPUNIT_TEST(test_buildS7OOSOptionalData);
  CPPUNIT_TEST(test_buildS7OOSOptionalData_RBD_Soft);
  CPPUNIT_TEST(test_buildS7OOSOptionalData_RF_Soft);
  CPPUNIT_TEST(test_buildS7OOSOptionalData_CF_Soft);
  CPPUNIT_TEST(test_buildSUMData);
  CPPUNIT_TEST(test_buildSUMData_BC);
  CPPUNIT_TEST(test_buildSUMData_SFI);
  CPPUNIT_TEST(test_buildSUMData_SFI_WPDispAE);
  CPPUNIT_TEST(test_buildSUMData_givenOcFee_whenAncillaryIdMatchingThisOcFeeExistsAndMonetaryDiscountIsEnabled_shouldAddQtyWithValueFromItinToSum);
  CPPUNIT_TEST(test_buildSFQData);
  CPPUNIT_TEST(test_buildSFQData_isWPDispAE);
  CPPUNIT_TEST(test_buildSFQData_C);
  CPPUNIT_TEST(test_buildSFQData_C_TC);
  CPPUNIT_TEST(test_buildSFQData_C_TC_RT);
  CPPUNIT_TEST(test_buildSFQData_C_TC_RT_R);
  CPPUNIT_TEST(test_buildSFQData_C_TC_RT_R_DOW);
  CPPUNIT_TEST(test_buildSFQData_C_TC_RT_R_DOW_EC);
  CPPUNIT_TEST(test_buildSFQData_C_TC_RT_R_DOW_EC_TS);
  CPPUNIT_TEST(test_buildSFQData_TC_DOW_TS);
  CPPUNIT_TEST(test_buildSFQData_WPDispAE);
  CPPUNIT_TEST(test_buildSFQData_WPDispAE_TC);
  CPPUNIT_TEST(test_buildSTOData);
  CPPUNIT_TEST(test_buildSTOData_SI);
  CPPUNIT_TEST(test_buildSTOData_SI_ST);
  CPPUNIT_TEST(test_buildSTOData_SI_ST_SU);
  CPPUNIT_TEST(test_buildDTEData);
  CPPUNIT_TEST(test_buildDTEData_D01);
  CPPUNIT_TEST(test_buildDTEData_D01_D02);
  CPPUNIT_TEST(test_buildDTEData_D01_D02_D03NotValid);
  CPPUNIT_TEST(test_buildDTEData_D01_D02_D03Valid);
  CPPUNIT_TEST(test_buildDTEData_D01_D02_D03Valid_TS);
  CPPUNIT_TEST(test_buildDTEData_D01_D02_D03Valid_TS_Q11);
  CPPUNIT_TEST(test_buildDTEData_D01_D02_D03Valid_TS_Q11_Q12);
  CPPUNIT_TEST(test_buildDTEData_D01_D02_D03Valid_TS_Q11_Q12_WPDispAE);
  CPPUNIT_TEST(test_buildFATData);
  CPPUNIT_TEST(test_buildFATData_FF);
  CPPUNIT_TEST(test_buildFATData_FF_SP);
  CPPUNIT_TEST(test_buildFATData_FF_FG);
  CPPUNIT_TEST(test_buildFATData_P01_WhenPdisplayAE_ANG_True);
  CPPUNIT_TEST(test_buildFATData_FF_FG_AP);
  CPPUNIT_TEST(test_buildFATData_FF_FG_AP_NCNA);
  CPPUNIT_TEST(test_buildFATData_FF_FG_AP_NCNA_FR);
  CPPUNIT_TEST(test_buildFATData_FF_SP_AP_NCNA_FR_FFS);
  CPPUNIT_TEST(test_buildFATData_FF_SP_AP_NCNA_FR_FFS_RR);
  CPPUNIT_TEST(test_buildFATData_FF_SP_AP_NCNA_FR_FFS_RR_CI);
  CPPUNIT_TEST(test_buildFATData_FF_SP_AP_NCNA_FR_FFS_RR_CI_II);
  CPPUNIT_TEST(test_buildRBDData);
  CPPUNIT_TEST(test_buildRBDData_empty);
  CPPUNIT_TEST(test_buildRBDData_data);
  CPPUNIT_TEST(test_buildRBDData_allData);
  CPPUNIT_TEST(test_buildBKCData);
  CPPUNIT_TEST(test_buildBKCData_data);
  CPPUNIT_TEST(test_buildFCLData);
  CPPUNIT_TEST(test_buildFCLData_empty);
  CPPUNIT_TEST(test_buildFCLData_data);
  CPPUNIT_TEST(test_buildCFTData);
  CPPUNIT_TEST(test_buildCFTData_empty);
  CPPUNIT_TEST(test_buildCFTData_data);
  CPPUNIT_TEST(test_buildCFTData_data1);
  CPPUNIT_TEST(test_buildCFTData_data2);
  CPPUNIT_TEST(test_processTicketEffDate_validDate);
  CPPUNIT_TEST(test_processTicketEffDate_validDate_WPDispAE);
  CPPUNIT_TEST(test_processTicketEffDate_startValues);
  CPPUNIT_TEST(test_processTicketEffDate_startValues_WPDispAE);
  CPPUNIT_TEST(test_processTicketEffDate_openDate);
  CPPUNIT_TEST(test_processTicketEffDate_openDate_WPDispAE);
  CPPUNIT_TEST(test_processTicketDiscDate_validDate);
  CPPUNIT_TEST(test_processTicketDiscDate_validDate_WPDispAE);
  CPPUNIT_TEST(test_processTicketDiscDate_infinity);
  CPPUNIT_TEST(test_processTicketDiscDate_infinity_WPDispAE);
  CPPUNIT_TEST(test_processTicketDiscDate_openDate);
  CPPUNIT_TEST(test_processTicketDiscDate_openDate_WPDispAE);
  CPPUNIT_TEST(test_processTicketDiscDate_openDate_stopValues);
  CPPUNIT_TEST(test_processTicketDiscDate_openDate_stopValues_WPDispAE);
  CPPUNIT_TEST(test_processPurchaseByDate_validDate);
  CPPUNIT_TEST(test_formatOCFeesForR7);
  CPPUNIT_TEST(test_formatOCFeesForR7_Group_Empty);
  CPPUNIT_TEST(test_formatOCFeesForR7_Group_NOT_AVAILABLE);
  CPPUNIT_TEST(test_formatOCFeesGroups_NotAvailable_NoPax);
  CPPUNIT_TEST(test_formatOCFeesGroups_Empty_NoPax);
  CPPUNIT_TEST(test_formatOCFeesGroups_Invalid_NoPax);
  CPPUNIT_TEST(test_formatOCFeesGroups_Valid_NoPax);
  CPPUNIT_TEST(test_formatOCFeesGroups_SA_Padis);
  CPPUNIT_TEST(test_formatOCFeesGroups_whenMonetaryDiscountIsEnabled_shouldSurroundOosWithOccAndAddAidAttributeToOcc);
  CPPUNIT_TEST(test_formatOCFeesGroups_givenMonetaryDiscountIsEnabled_whenOscHasMultipleOosElements_shouldGroupOosElementsInOccElementsByAid);
  CPPUNIT_TEST(test_formatOCFeesGroups_whenShouldSkipCreatingOcgElementMethodReturnsTrue_shouldSkipCreatingOcgElement);
  CPPUNIT_TEST(test_formatOCFeesGroups_whenShouldSkipCreatingOcgElementMethodReturnsFalse_shouldNotSkipCreatingOcgElement);
  CPPUNIT_TEST(test_shouldSkipCreatingOcgElement_whenNotHandlingSecondCallForMonetaryDiscount_shouldReturnFalse);
  CPPUNIT_TEST(test_shouldSkipCreatingOcgElement_givenPaxOcFeesUsagesArePresent_whenHandlingSecondCallForMonetaryDiscount_shouldReturnFalse);
  CPPUNIT_TEST(test_shouldSkipCreatingOcgElement_givenNoPaxOcFeesUsages_whenHandlingSecondCallForMonetaryDiscount_shouldReturnTrue);
  CPPUNIT_TEST(test_formatOCFees_SA_Padis);
  CPPUNIT_TEST(test_formatOCFees_NotAvailable);
  CPPUNIT_TEST(test_timeOutMaxCharCountNoOCFeesReturned_BG_Data_SWS_PATH);
  CPPUNIT_TEST(test_timeOutMaxCharCountNoOCFeesReturned_BG_Data);
  CPPUNIT_TEST(test_timeOutMaxCharCountNoOCFeesReturned_No_Data_SWS_PATH);
  CPPUNIT_TEST(test_timeOutMaxCharCountNoOCFeesReturned_No_Data);

  CPPUNIT_TEST(test_timeOutMaxCharCountRequestedOCFeesReturned_No_Data);
  CPPUNIT_TEST(test_timeOutMaxCharCountRequestedOCFeesReturned_No_Data_SWS_PATH);
  CPPUNIT_TEST(test_anyTimeOutMaxCharCountIssue_timeOutForExceeded_True);
  CPPUNIT_TEST(test_anyTimeOutMaxCharCountIssue_timeOutOCFForWP_True);
  CPPUNIT_TEST(test_anyTimeOutMaxCharCountIssue_timeOutForExceededSFGpresent_True);

  CPPUNIT_TEST(test_timeOutMaxCharCountNoOCFeesReturned_BG_Data_R7Post_OneItins);
  CPPUNIT_TEST(test_timeOutMaxCharCountNoOCFeesReturned_BG_Data_R7Post_TwoItins);
  CPPUNIT_TEST(test_isTimeOutBeforeStartOCFees_True);
  CPPUNIT_TEST(test_isTimeOutBeforeStartOCFees_False);
  CPPUNIT_TEST(test_samePaxType_True);
  CPPUNIT_TEST(test_samePaxType_False);
  // EMD-S phase 2 project
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_M70_PR0_Y);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_M70_PR0_N);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_M70_PR0_Blank);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_WPAE_PR0_Y);
  CPPUNIT_TEST(test_buildOSCOptionalData_Data_WPAE_PR0_Y_AB240);
  CPPUNIT_TEST(test_buildS7OOSData_S7_M70_PY0_Y);
  CPPUNIT_TEST(test_buildS7OOSData_S7_M70_PY0_N);
  CPPUNIT_TEST(test_buildS7OOSData_S7_M70_PY0_Blank);
  CPPUNIT_TEST(test_buildS7OOSData_S7_WPAE_PY0_Y);
  CPPUNIT_TEST(test_buildOSCOptionalData_MSG);
  CPPUNIT_TEST(test_buildOSCOptionalData_BIR_SDC);
  CPPUNIT_TEST(test_buildBGA);

  CPPUNIT_TEST(test_getPadisCodeString);
  CPPUNIT_TEST(test_getPadisCodeString_NoMatch);
  CPPUNIT_TEST(test_buildS7OOSData_Padis);
  CPPUNIT_TEST(test_buildS7OOSData_SA_NoPadis);
  CPPUNIT_TEST(test_buildS7OOSData_SA_NoPadis_ACS);
  CPPUNIT_TEST(test_buildPds);
  CPPUNIT_TEST(test_buildPds_NoAbbreviation);
  CPPUNIT_TEST(test_buildPds_NoDescription);
  CPPUNIT_TEST(test_buildPds_NoAbbreviation_NoDescription);
  CPPUNIT_TEST(test_buildPds_NoPadisCode);
  CPPUNIT_TEST(test_buildUpc);
  CPPUNIT_TEST(test_buildUpc_DuplicatePadisCodes);
  CPPUNIT_TEST(test_buildUpc_NoPadisCode);
  CPPUNIT_TEST(test_buildUpc_NoFarePath);
  CPPUNIT_TEST(test_buildUpc_NoItin);
  CPPUNIT_TEST(test_buildPadis_OnePadis_OneCode);
  CPPUNIT_TEST(test_buildPadis_OnePadis_FiveCodes);
  CPPUNIT_TEST(test_buildPadis_MultiplePadis_SingleCodes);
  CPPUNIT_TEST(test_buildPadis_MultiplePadis_MultipleCodes);

  CPPUNIT_TEST(test_convertWeightUnit_Kilo);
  CPPUNIT_TEST(test_convertWeightUnit_Pound);

  CPPUNIT_TEST(test_isOCSData_Allowance_S7);
  CPPUNIT_TEST(test_isOCSData_No_Allowance_S7);

  CPPUNIT_TEST(test_buildDisclosureText_Found);
  CPPUNIT_TEST(test_buildDisclosureText_NotFound);
  CPPUNIT_TEST(test_buildS7Oos_AB240_Padis_1);
  CPPUNIT_TEST(test_buildS7Oos_NoAB240_NoPadis);
  CPPUNIT_TEST(test_buildS7Oos_AB240_Padis_2);
  CPPUNIT_TEST(test_buildS7Oos_ACS_NoPadis);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Flight_Related_Service_M70);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Flight_Related_Service_WPAE);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Flight_Related_Service_NonDisplayOnly_M70);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Flight_Related_Service_NonDisplayOnly_WPAE);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_M70);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_WPAE);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_NonDisplayOnly_M70);
  CPPUNIT_TEST(test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_NonDisplayOnly_WPAE);
  CPPUNIT_TEST(test_AM_Apply_Charges_Tax);
  CPPUNIT_TEST(test_AM_Do_Not_Apply_Charges_Tax);

  CPPUNIT_TEST(test_buildOperatingCarrierInfo);

  CPPUNIT_TEST_SUITE_END();

  class AncillaryPricingResponseFormatterDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memH;

  public:
    AncillaryPricingResponseFormatterDataHandleMock() {}

    ~AncillaryPricingResponseFormatterDataHandleMock() { _memH.clear(); }

    const ServicesDescription* getServicesDescription(const ServiceGroupDescription& value)
    {
      ServicesDescription* ret = _memH.create<ServicesDescription>();

      if (value == "10")
      {
        ret->description() = "TEST10";
        return ret;
      }
      else if (value == "12")
      {
        ret->description() = "TEST12";
        return ret;
      }
      else if (value == "100")
      {
        ret->description() = "TEST1TEST1TEST1";
        return ret;
      }
      else if (value == "200")
      {
        ret->description() = "TEST2TEST2TEST2";
        return ret;
      }
      else if (value == "")
        return 0;

      return DataHandleMock::getServicesDescription(value);
    }

    const TaxText* getTaxText(const VendorCode& vendor, int itemNo)
    {
      TaxText* taxText = _memH.create<TaxText>();

      switch (itemNo)
      {
      case 1:
        taxText->txtMsgs().push_back(TAXTEXT_1);
        break;
      case 2:
        taxText->txtMsgs().push_back(TAXTEXT_1);
        taxText->txtMsgs().push_back(TAXTEXT_2);
        break;
      case 3:
        taxText->txtMsgs().push_back(TAXTEXT_3);
        taxText->txtMsgs().push_back(TAXTEXT_2);
        break;
      case 3055:
        taxText->txtMsgs().push_back(TAXTEXT_3055_1);
        taxText->txtMsgs().push_back(TAXTEXT_3055_2);
        taxText->txtMsgs().push_back(TAXTEXT_3055_3);
        taxText->txtMsgs().push_back(TAXTEXT_3055_4);
        taxText->txtMsgs().push_back(TAXTEXT_3055_5);
        taxText->txtMsgs().push_back(TAXTEXT_3055_6);
        break;
      }

      return taxText;
    }

    const ServicesSubGroup*
    getServicesSubGroup(const ServiceGroup& serviceGroup, const ServiceGroup& serviceSubGroup)
    {
      ServicesSubGroup* servicesSubGroup = _memH.create<ServicesSubGroup>();
      servicesSubGroup->serviceGroup() = serviceGroup;
      servicesSubGroup->serviceSubGroup() = serviceSubGroup;
      servicesSubGroup->definition() = "SPORTING EQUIPMENT";

      return servicesSubGroup;
    }

    SeatCabinCharacteristicInfo*
    getSeatCabinCharacteristic(const CarrierCode& carrier,
                               const Indicator& codeType,
                               const SeatCabinCode& padisCode,
                               const std::string& deprecatedDescription,
                               const std::string& displayDescription,
                               const std::string& abbreviatedDescription)
    {
      SeatCabinCharacteristicInfo* seatCabinCharacteristic =
          _memH.create<SeatCabinCharacteristicInfo>();
      seatCabinCharacteristic->carrier() = carrier;
      seatCabinCharacteristic->codeType() = codeType;
      seatCabinCharacteristic->seatCabinCode() = padisCode;
      seatCabinCharacteristic->codeDescription() = deprecatedDescription;
      seatCabinCharacteristic->displayDescription() = displayDescription;
      seatCabinCharacteristic->abbreviatedDescription() = abbreviatedDescription;
      return seatCabinCharacteristic;
    }

    const std::vector<SeatCabinCharacteristicInfo*>&
    getSeatCabinCharacteristicInfo(const CarrierCode& carrier,
                                   const Indicator& codeType,
                                   const DateTime& travelDate)
    {
      std::vector<SeatCabinCharacteristicInfo*>* result =
          _memH.create<std::vector<SeatCabinCharacteristicInfo*>>();

      result->push_back(getSeatCabinCharacteristic("**", 'S', "E", "ExitRow", "EXIT", "EXTRW"));
      result->push_back(getSeatCabinCharacteristic("**", 'S', "W", "Window", "WINDOW", "W"));
      result->push_back(
          getSeatCabinCharacteristic("**", 'S', "K", "BulkHeadSeat", "BULK HEAD", "BKHDS"));
      result->push_back(
          getSeatCabinCharacteristic("**", 'S', "AB", "AdjacentToBar", "ADJACENT TO BAR", "BAR"));
      result->push_back(getSeatCabinCharacteristic("**", 'S', "A", "Aisle Seat", "AISLE", "AISLE"));
      result->push_back(getSeatCabinCharacteristic(
          "**", 'S', "CC", "CenterSectionSeat", "CENTER SECTION", "CTSEC"));
      result->push_back(getSeatCabinCharacteristic(
          "**", 'S', "Q", "SeatInQuietZone", "SEAT IN QUIET ZONE", "QUIET"));
      result->push_back(
          getSeatCabinCharacteristic("**", 'S', "AR", "NoAirPhone", "NO AIRPHONE", "NOAPH"));
      result->push_back(
          getSeatCabinCharacteristic("**", 'S', "LS", "LeftSide", "LEFT SIDE", "LEFT"));

      return *result;
    }
  };

  class AncillaryPricingResponseFormatterMock : public AncillaryPricingResponseFormatter
  {
  public:
    DateTime calculatePurchaseByDate(const PricingTrx& pricingTrx) { return DateTime(2011, 3, 1); }
    MOCK_CONST_METHOD2(shouldSkipCreatingOcgElement, bool(AncillaryPricingTrx&, const std::vector<PaxOCFees>&));
    MOCK_CONST_METHOD2(shouldSkipCreatingOscElement, bool(AncillaryPricingTrx&, const PaxOCFees&));
    MOCK_METHOD3(buildOCFeesResponse, void(AncillaryPricingTrx&, Itin*, XMLConstruct&));
    MOCK_METHOD3(buildOcFeesSecondResponseForMonetaryDiscount, void(AncillaryPricingTrx&, Itin*, XMLConstruct&));
    MOCK_METHOD3(formatOCFees, void(AncillaryPricingTrx&, XMLConstruct&, const bool));
    MOCK_METHOD4(buildOscContent, void(AncillaryPricingTrx&, XMLConstruct&, const PaxOCFees&, const uint16_t));

    void doBuildOcFeesSecondResponseForMonetaryDiscount(AncillaryPricingTrx& ancTrx, Itin* itin, XMLConstruct& construct)
    {
      AncillaryPricingResponseFormatter::buildOcFeesSecondResponseForMonetaryDiscount(ancTrx, itin, construct);
    }
  };

private:
    AncillaryPricingResponseFormatterDataHandleMock* _dataHandleMock;
    AncillaryPricingResponseFormatter* _ancFormatter;
    AncillaryPricingResponseFormatterMock* _ancFormatterMock;
    XMLConstruct* _construct;
    AncillaryPricingTrx* _ancTrx;
    Itin* _itin;
    AncRequest* _ancRequest;
    PricingOptions* _pOptions;
    FareCalcConfig* _fareCalcConfig;
    Agent* _agent;
    TestMemHandle _memH;
    Billing* _billing;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    populateConfig();
    _dataHandleMock = _memH.create<AncillaryPricingResponseFormatterDataHandleMock>();
    _ancFormatter = _memH.create<AncillaryPricingResponseFormatter>();
    _construct = _memH.create<XMLConstruct>();
    _ancFormatterMock = _memH.create<AncillaryPricingResponseFormatterMock>();
    _ancTrx = _memH.create<AncillaryPricingTrx>();
    _ancRequest = _memH.create<AncRequest>();
    _pOptions = _memH.create<PricingOptions>();
    _itin = _memH.create<Itin>();
    _ancFormatter->currentItin() = _itin;
    _fareCalcConfig = _memH.create<FareCalcConfig>();
    _agent = _memH.create<Agent>();
    _billing = _memH.create<Billing>();
    _agent->currencyCodeAgent() = "USD";
    _fareCalcConfig->warningMessages() = FareCalcConsts::FC_YES;
    _ancTrx->setRequest(_ancRequest);
    _ancTrx->getRequest()->ticketingAgent() = _agent;
    _ancTrx->setOptions(_pOptions);
    _ancTrx->itin().push_back(_itin);
    _ancTrx->fareCalcConfig() = _fareCalcConfig;
    _ancTrx->billing() = _billing;
    populateSizeWeightDescription();
  }

  void tearDown() { _memH.clear(); }

  CarrierFlightSeg* createCxrFltSeg()
  {
    CarrierFlightSeg* cfs = _memH.insert(new CarrierFlightSeg);
    return cfs;
  }

  FarePath* createFarePath()
  {
    FarePath* farePath = _memH.insert(new FarePath);
    PaxType* paxType = _memH.insert(new PaxType);
    paxType->paxType() = "ADT";
    farePath->paxType() = paxType;
    return farePath;
  }

  OCFees* createOCFees(bool displayOnly = false, ServiceGroup serviceGroup = "BG", bool isS7 = true)
  {
    OCFees* ocFees = _memH.insert(new OCFees);
    ocFees->setDisplayOnly(displayOnly);
    ocFees->carrierCode() = "LH";
    ocFees->feeNoDec() = 0;

    SubCodeInfo* subCodeInfo = _memH.insert(new SubCodeInfo);
    subCodeInfo->commercialName() = "BAGGAGE";
    subCodeInfo->serviceGroup() = serviceGroup;
    ocFees->subCodeInfo() = subCodeInfo;

    if (isS7)
    {
      OptionalServicesInfo* optServicesInfo = _memH.insert(new OptionalServicesInfo);
      ocFees->optFee() = optServicesInfo;
    }

    Loc* originLoc = _memH.insert(new Loc);
    Loc* destLoc = _memH.insert(new Loc);
    originLoc->loc() = "DEL";
    destLoc->loc() = "FRA";
    AirSeg* travelStartSeg = _memH.insert(new AirSeg);
    AirSeg* travelEndSeg = _memH.insert(new AirSeg);
    travelStartSeg->origin() = originLoc;
    travelStartSeg->destination() = destLoc;
    travelStartSeg->pnrSegment() = 1;
    travelStartSeg->setCheckedPortionOfTravelInd('T');
    ocFees->travelStart() = travelStartSeg;
    travelEndSeg->origin() = destLoc;
    travelEndSeg->destination() = originLoc;
    travelEndSeg->pnrSegment() = 2;
    travelEndSeg->setCheckedPortionOfTravelInd('T');
    ocFees->travelEnd() = travelEndSeg;

    OCFeesUsage* ocFeeUsage = _memH.insert(new OCFeesUsage);
    ocFeeUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeeUsage);

    return ocFees;
  }

  OCFees* createOCFeesOneTravelSeg(bool displayOnly = false, ServiceGroup serviceGroup = "BG", bool shouldGenerateAid = false)
  {
    OCFees* ocFees = _memH.insert(new OCFees);
    ocFees->carrierCode() = "LH";
    ocFees->feeNoDec() = 0;

    SubCodeInfo* subCodeInfo = _memH.insert(new SubCodeInfo);
    subCodeInfo->commercialName() = "BAGGAGE";
    subCodeInfo->serviceGroup() = serviceGroup;
    if (shouldGenerateAid)
    {
      subCodeInfo->fltTktMerchInd() = 'A';
      subCodeInfo->serviceSubTypeCode() = ServiceSubTypeCode("0DF");
    }
    ocFees->subCodeInfo() = subCodeInfo;

    OptionalServicesInfo* optServicesInfo = _memH.insert(new OptionalServicesInfo);
    optServicesInfo->seqNo() = 1234;
    if (shouldGenerateAid)
      optServicesInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 3000;
    ocFees->optFee() = optServicesInfo;

    Loc* originLoc = _memH.insert(new Loc);
    originLoc->loc() = "DEL";
    AirSeg* travelStartSeg = _memH.insert(new AirSeg);
    travelStartSeg->origin() = originLoc;
    travelStartSeg->pnrSegment() = 1;
    Loc* destLoc = _memH.insert(new Loc);
    destLoc->loc() = "FRA";
    travelStartSeg->destination() = destLoc;
    ocFees->travelStart() = travelStartSeg;
    ocFees->travelEnd() = travelStartSeg;

    OCFeesUsage* ocFeeUsage = _memH.insert(new OCFeesUsage);
    ocFeeUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeeUsage);
    ocFees->setDisplayOnly(displayOnly);

    if (shouldGenerateAid)
    {
      AncillaryIdentifier ancId{*ocFees};
      ocFees->getCurrentSeg()->_ancPriceModification = std::make_pair(ancId, AncillaryPriceModifier{std::string("pmi"), 2});
    }

    return ocFees;
  }

  PaxOCFees*
  createPaxOCFees(bool displayOnly = false, ServiceGroup serviceGroup = "BG", bool isS7 = true)
  {
    FPOCFees* fpOcFees =
        _memH.insert(new FPOCFees(createFarePath(), createOCFees(displayOnly, serviceGroup, isS7)));
    PaxOCFees* paxOcFees = _memH.insert(new PaxOCFees(*fpOcFees));
    return paxOcFees;
  }

  PaxOCFees* createPaxOCFeesOneTravelSeg(bool displayOnly = false, ServiceGroup serviceGroup = "BG", bool shouldGenerateAid = false)
  {
    FPOCFees* fpOcFees = _memH.insert(
        new FPOCFees(createFarePath(), createOCFeesOneTravelSeg(displayOnly, serviceGroup, shouldGenerateAid)));
    PaxOCFees* paxOcFees = _memH.insert(new PaxOCFees(*fpOcFees));
    return paxOcFees;
  }

  PaxOCFees* createPaxOcFeesWithManyOcFeesUsagesThatHaveDifferentAid(int seqNo = 4321)
  {
    FPOCFees* fpOcFees = _memH.insert(
        new FPOCFees(createFarePath(), createOcFeesWithManyOcFeesUsagesThatHaveDifferentAid(seqNo)));
    PaxOCFees* paxOcFees = _memH.insert(new PaxOCFees(*fpOcFees));
    return paxOcFees;
  }

  OCFees* createOcFeesWithManyOcFeesUsagesThatHaveDifferentAid(int seqNo = 4321)
  {
    OCFees* ocFees = createOCFeesOneTravelSeg(false, "BG", true);

    OCFeesUsage* repeatedOcFeeUsage = _memH.insert(new OCFeesUsage);
    repeatedOcFeeUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(repeatedOcFeeUsage);

    ocFees->addSeg(_ancTrx->dataHandle());

    OptionalServicesInfo* optServicesInfoForDifferentAid = _memH.insert(new OptionalServicesInfo);
    optServicesInfoForDifferentAid->seqNo() = seqNo;
    ocFees->optFee() = optServicesInfoForDifferentAid;

    OCFeesUsage* ocFeeUsageForDifferentAid = _memH.insert(new OCFeesUsage);
    ocFeeUsageForDifferentAid->oCFees() = ocFees;
    ocFeeUsageForDifferentAid->setSegIndex(1);
    ocFees->ocfeeUsage().push_back(ocFeeUsageForDifferentAid);

    AncillaryIdentifier ancId{*ocFees};
    ocFees->getCurrentSeg()->_ancPriceModification = std::make_pair(ancId, AncillaryPriceModifier());

    return ocFees;
  }

  OCFees* createOCFeesSubCodeInfo(bool displayOnly = false, ServiceGroup serviceGroup = "BG")
  {
    OCFees* ocFees = _memH.insert(new OCFees);
    ocFees->setDisplayOnly(displayOnly);
    ocFees->carrierCode() = "LH";
    ocFees->feeNoDec() = 0;

    SubCodeInfo* subCodeInfo = _memH.insert(new SubCodeInfo);
    subCodeInfo->commercialName() = "BAGGAGE";
    subCodeInfo->serviceGroup() = serviceGroup;
    subCodeInfo->rfiCode() = 'Y';
    subCodeInfo->ssrCode() = "ABCD";
    subCodeInfo->emdType() = 'G';
    subCodeInfo->bookingInd() = "A";
    subCodeInfo->ssimCode() = 'F';
    ocFees->subCodeInfo() = subCodeInfo;

    OptionalServicesInfo* optServicesInfo = _memH.insert(new OptionalServicesInfo);
    ocFees->optFee() = optServicesInfo;
    OCFeesUsage* ocFeeUsage = _memH.insert(new OCFeesUsage);
    ocFeeUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeeUsage);

    return ocFees;
  }

  PaxOCFees* createPaxOCFeesSubCodeInfo(bool displayOnly = false, ServiceGroup serviceGroup = "BG")
  {
    FPOCFees* fpOcFees = _memH.insert(
        new FPOCFees(createFarePath(), createOCFeesSubCodeInfo(displayOnly, serviceGroup)));
    PaxOCFees* paxOcFees = _memH.insert(new PaxOCFees(*fpOcFees));
    return paxOcFees;
  }

  SvcFeesCxrResultingFCLInfo* createSvcFeesCxrResultingFCLInfo()
  {
    SvcFeesCxrResultingFCLInfo* sfcResFCLInfo = _memH.insert(new SvcFeesCxrResultingFCLInfo);
    return sfcResFCLInfo;
  }

  SvcFeesResBkgDesigInfo* createSvcFeesResBkgDesigInfo()
  {
    SvcFeesResBkgDesigInfo* svcFeesRBDInfo = _memH.insert(new SvcFeesResBkgDesigInfo);
    svcFeesRBDInfo->mkgOperInd() = ' ';
    return svcFeesRBDInfo;
  }

  ServiceFeesGroup* createBasicServiceFeesGroup()
  {
    ServiceFeesGroup* serviceFeesGroup = _memH.insert(new ServiceFeesGroup);
    serviceFeesGroup->groupCode() = "SA";
    serviceFeesGroup->state() = ServiceFeesGroup::VALID;
    return serviceFeesGroup;
  }

  void fillRBDData(OCFeesUsage& ocFeeUsage)
  {
    std::vector<SvcFeesResBkgDesigInfo*> svcFeeCxrResBkgDesInfoVec;
    SvcFeesResBkgDesigInfo* svcFeesResBkgDesigInfoData = createSvcFeesResBkgDesigInfo();
    svcFeesResBkgDesigInfoData->mkgOperInd() = 'E';
    svcFeesResBkgDesigInfoData->carrier() = "LH";
    svcFeesResBkgDesigInfoData->bookingCode1() = "Y";
    svcFeesResBkgDesigInfoData->bookingCode2() = "H";
    svcFeesResBkgDesigInfoData->bookingCode3() = "E";
    svcFeesResBkgDesigInfoData->bookingCode4() = "F";
    svcFeesResBkgDesigInfoData->bookingCode5() = "N";
    svcFeeCxrResBkgDesInfoVec.push_back(svcFeesResBkgDesigInfoData);
    ocFeeUsage.softMatchRBDT198() = svcFeeCxrResBkgDesInfoVec;
  }

  SvcFeesResBkgDesigInfo* createPadis(BookingCode padisCode1,
                                      BookingCode padisCode2,
                                      BookingCode padisCode3,
                                      BookingCode padisCode4,
                                      BookingCode padisCode5,
                                      CarrierCode carrierCode,
                                      int seqNo = 1)
  {
    SvcFeesResBkgDesigInfo* padis = createSvcFeesResBkgDesigInfo();

    padis->seqNo() = seqNo;
    padis->carrier() = carrierCode;

    if (!padisCode1.empty())
    {
      padis->bookingCode1() = padisCode1;
    }
    if (!padisCode2.empty())
    {
      padis->bookingCode2() = padisCode2;
    }
    if (!padisCode3.empty())
    {
      padis->bookingCode3() = padisCode3;
    }
    if (!padisCode4.empty())
    {
      padis->bookingCode4() = padisCode4;
    }
    if (!padisCode5.empty())
    {
      padis->bookingCode5() = padisCode5;
    }

    return padis;
  }

  void fillFCLData(OCFeesUsage& ocFeesUsage)
  {
    std::vector<SvcFeesCxrResultingFCLInfo*> svcFeeCxrResFCLInfoVec;
    SvcFeesCxrResultingFCLInfo* svcFeeCxrResFCLInfoData = createSvcFeesCxrResultingFCLInfo();
    svcFeeCxrResFCLInfoData->carrier() = "AA";
    svcFeeCxrResFCLInfoData->resultingFCL() = "Y26";
    svcFeeCxrResFCLInfoData->fareType() = "ER";
    svcFeeCxrResFCLInfoVec.push_back(svcFeeCxrResFCLInfoData);
    ocFeesUsage.softMatchResultingFareClassT171() = svcFeeCxrResFCLInfoVec;
  }

  void fillCFData(OCFeesUsage& ocFeesUsage)
  {
    std::vector<CarrierFlightSeg*> cfsVec;
    CarrierFlightSeg* cfsData = createCxrFltSeg();
    cfsData->marketingCarrier() = "AA";
    cfsData->operatingCarrier() = "BA";
    cfsData->flt1() = 142;
    cfsData->flt2() = 10;
    cfsVec.push_back(cfsData);
    ocFeesUsage.softMatchCarrierFlightT186() = cfsVec;
  }

  void makeAncTrxSecondCallForMonetaryPricing()
  {
    _ancTrx->modifiableActivationFlags().setMonetaryDiscount(true);
    std::string validAid = "1S|C|0DF|1.2|1000";
    _itin->addAncillaryPriceModifier(AncillaryIdentifier(validAid), AncillaryPriceModifier());
  }

  void test_readConfigXMLNamespace_empty()
  {
    std::string val = _ancFormatter->readConfigXMLNamespace("");
    std::string test = "";
    CPPUNIT_ASSERT_EQUAL(test, val);
  }

  static const std::string XMLNS;

  void populateConfig()
  {
    TestConfigInitializer::setValue("ANC_PRC_XML_NAMESPACE", XMLNS, "SERVICE_FEES_SVC");
    TestConfigInitializer::setValue("ANCILLARY_EMDS_PHASE_2_ENABLED", "Y", "SERVICE_FEES_SVC");
  }

  void populateSizeWeightDescription()
  {
    _ancFormatter->_baggageSizeWeightRestrictions.push_back(
        AncillaryPricingResponseFormatter::BaggageSizeWeightDescription("10", 3.4, 'K', 'O'));
    _ancFormatter->_baggageSizeWeightRestrictions.push_back(
        AncillaryPricingResponseFormatter::BaggageSizeWeightDescription("12", 22.5, 'L', 'U'));
    _ancFormatter->_baggageSizeWeightRestrictions.push_back(
        AncillaryPricingResponseFormatter::BaggageSizeWeightDescription("12", 10, 'L', 'O'));
    _ancFormatter->_baggageSizeWeightRestrictions.push_back(
        AncillaryPricingResponseFormatter::BaggageSizeWeightDescription("12", 3., 'K', 'O'));
    _ancFormatter->_baggageSizeWeightRestrictions.push_back(
        AncillaryPricingResponseFormatter::BaggageSizeWeightDescription("12", 22, 'C', 'U'));
    _ancFormatter->_baggageSizeWeightRestrictions.push_back(
        AncillaryPricingResponseFormatter::BaggageSizeWeightDescription("12", 5, 'I', 'O'));
  }

  void test_readConfigXMLNamespace_Val()
  {
    std::string configValue = "TEST";

    Global::config().getValue("ANC_PRC_XML_NAMESPACE", configValue, "SERVICE_FEES_SVC");
    std::string val = _ancFormatter->readConfigXMLNamespace("ANC_PRC_XML_NAMESPACE");
    CPPUNIT_ASSERT_EQUAL(configValue, val);
    CPPUNIT_ASSERT_EQUAL(XMLNS, val);
    CPPUNIT_ASSERT_EQUAL(XMLNS, configValue);
  }

  void test_formatResponse_Empty()
  {
    const std::string test = "";
    ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::SYSTEM_EXCEPTION;
    const std::string expectedXML("<AncillaryPricingResponse xmlns=\"" + XMLNS +
                                  "\">"
                                  "<MSG N06=\"E\" Q0K=\"0\"/></AncillaryPricingResponse>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _ancFormatter->formatResponse(test, *_ancTrx, errCode));
  }

  void test_formatResponse_Data()
  {
    const std::string test = "";
    ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR;
    const std::string expectedXML("<AncillaryPricingResponse xmlns=\"" + XMLNS + "\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _ancFormatter->formatResponse(test, *_ancTrx, errCode));
  }

  void test_prepareResponseText_Empty()
  {
    const std::string test = "";
    _ancFormatter->prepareResponseText(test, *_construct, false);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_prepareResponseText_Data()
  {
    const std::string test("<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                           "SFN=\"A\" N03=\"F\" SFD=\"N\"/>");
    _ancFormatter->prepareResponseText(test, *_construct, false);
    const std::string expectedXML("<MSG N06=\"X\" Q0K=\"3\" S18=\""
                                  "<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" SFD=\"N\"/>\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_prepareMessage_Empty()
  {
    _ancFormatter->prepareMessage(*_construct, ' ', 0, "");
    const std::string expectedXML = "<MSG N06=\" \" Q0K=\"0\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_prepareMessage_notEmpty()
  {
    _ancFormatter->prepareMessage(*_construct, 'A', 1, "HELLO");
    const std::string expectedXML = "<MSG N06=\"A\" Q0K=\"1\" S18=\"HELLO\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatAEFeesResponse_NoItin()
  {
    _ancFormatter->formatAEFeesResponse(*_construct, *_ancTrx);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // SG - Service Group
  void test_formatAEFeesResponse_Itin_SG()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _itin->setOcFeesFound(true);
    _ancFormatter->formatAEFeesResponse(*_construct, *_ancTrx);
    const std::string expectedXML = "<ITN Q00=\"0\"><OCG Q00=\"0\"/></ITN>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // SG - Service Group
  void test_formatAEFeesResponse_Itin_SG_WPDispAE()
  {
    _ancFormatter->_isWPDispAE = true;
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _itin->setOcFeesFound(true);
    _ancFormatter->formatAEFeesResponse(*_construct, *_ancTrx);
    const std::string expectedXML("<ITN Q00=\"0\"><OCH><MSG S18=\"AIR EXTRAS\"/>"
                                  "</OCH><OCG Q00=\"0\"/></ITN>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // DP - Do Not Process
  void test_formatAEFeesResponse_Itin_SG_WPDispAE_DP_true()
  {
    _ancFormatter->_isWPDispAE = true;
    _ancFormatter->_doNotProcess = true;
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->formatAEFeesResponse(*_construct, *_ancTrx);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatAEFeesResponse_whenHandlingSecondCallForMonetaryDiscount_shouldCallBuildOcFeesSecondResponseForMonetaryDiscount()
  {
    _itin->ocFeesGroup().push_back(nullptr);
    makeAncTrxSecondCallForMonetaryPricing();

    EXPECT_CALL(*_ancFormatterMock, buildOcFeesSecondResponseForMonetaryDiscount(_, _, _)).Times(1);
    EXPECT_CALL(*_ancFormatterMock, buildOCFeesResponse(_, _, _)).Times(0);
    _ancFormatterMock->formatAEFeesResponse(*_construct, *_ancTrx);
  }

  void test_formatAEFeesResponse_whenNotHandlingSecondCallForMonetaryDiscount_shouldCallBuildOCFeesResponse()
  {
    _itin->ocFeesGroup().push_back(nullptr);

    EXPECT_CALL(*_ancFormatterMock, buildOcFeesSecondResponseForMonetaryDiscount(_, _, _)).Times(0);
    EXPECT_CALL(*_ancFormatterMock, buildOCFeesResponse(_, _, _)).Times(1);
    _ancFormatterMock->formatAEFeesResponse(*_construct, *_ancTrx);
  }

  void test_buildOcFeesSecondResponseForMonetaryDiscount_whenProcessingIsNotStopped_shouldCreateItnTagWithQ00AttributeAndCallFormatOcFees()
  {
    _itin->setItinOrderNum(1);
    _ancFormatterMock->_doNotProcess = false;

    EXPECT_CALL(*_ancFormatterMock, formatOCFees(_, _, _)).Times(1);
    _ancFormatterMock->doBuildOcFeesSecondResponseForMonetaryDiscount(*_ancTrx, _itin, *_construct);
    EXPECT_STREQ("<ITN Q00=\"1\"/>", _construct->getXMLData().c_str());
  }

  void test_buildOcFeesSecondResponseForMonetaryDiscount_whenProcessingIsStopped_shouldDoNothing()
  {
    _ancFormatterMock->_doNotProcess = true;

    EXPECT_CALL(*_ancFormatterMock, formatOCFees(_, _, _)).Times(0);
    _ancFormatterMock->doBuildOcFeesSecondResponseForMonetaryDiscount(*_ancTrx, _itin, *_construct);
    EXPECT_TRUE(_construct->getXMLData().empty());
  }

  void test_isGenericTrailer_checkIfAnyGroupValid_True()
  {
    _ancFormatter->currentItin() = _itin;
    CPPUNIT_ASSERT_EQUAL(true, _ancFormatter->isGenericTrailer(*_ancTrx, *_construct));
    const std::string expectedXML("<OCM><OCF><MSG S18=\"AIR EXTRAS NOT FOUND\"/></OCF></OCM>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isGenericTrailer_checkIfAnyGroupValid_False()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    CPPUNIT_ASSERT_EQUAL(false, _ancFormatter->isGenericTrailer(*_ancTrx, *_construct));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_createOCGSection()
  {
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->createOCGSection(*_ancTrx, *_construct);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_createOCGSection_Data()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    ServiceFeesGroup* srvFeesGroup1 = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup1->groupCode() = "ML";
    _itin->ocFeesGroup().push_back(srvFeesGroup1);
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->createOCGSection(*_ancTrx, *_construct);
    const std::string expectedXML = "<OCG SF0=\"BG\"/><OCG SF0=\"ML\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_checkIfAnyGroupValid_false()
  {
    _ancFormatter->currentItin() = _itin;
    CPPUNIT_ASSERT_EQUAL(false, _ancFormatter->checkIfAnyGroupValid(*_ancTrx));
  }

  void test_checkIfAnyGroupValid_true()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    CPPUNIT_ASSERT_EQUAL(true, _ancFormatter->checkIfAnyGroupValid(*_ancTrx));
  }

  void test_formatOCFeesLineForWPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelStart()));
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelEnd()));
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->formatOCFeesLineForR7(*_ancTrx, *_construct, *paxFee, 1, ' ');
    const std::string expectedXML("<MSG N06=\"X\" Q0K=\"3\" S18=\"1  ADT-BAGGAGE"
                                  "                        LH  1-DELFRA       0.00  \"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesLineForWPDispAE_noIndex()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelStart()));
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelEnd()));
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->formatOCFeesLineForR7(*_ancTrx, *_construct, *paxFee, 0, ' ');
    const std::string expectedXML("<MSG N06=\"X\" Q0K=\"3\" S18=\"-- ADT-BAGGAGE"
                                  "                        LH  1-DELFRA       0.00  \"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesLineForWPDispAE_noIndex_NotAvail()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelStart()));
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelEnd()));
    (const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee()))->notAvailNoChargeInd() = 'X';
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->formatOCFeesLineForR7(*_ancTrx, *_construct, *paxFee, 0, ' ');
    const std::string expectedXML("<MSG N06=\"X\" Q0K=\"3\" S18=\"-- ADT-BAGGAGE"
                                  "                        LH  1-DELFRA  NOT AVAIL  \"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCData_whenShouldSkipCreatingOscElementReturnsTrue_shouldDoNothing()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesSubCodeInfo();
    uint16_t index = 0;
    EXPECT_CALL(*_ancFormatterMock, shouldSkipCreatingOscElement(_, _)).Times(1).WillOnce(Return(true));
    _ancFormatterMock->buildOSCData(*_ancTrx, *_construct, *paxOcFees, index);
    EXPECT_TRUE(_construct->getXMLData().empty());
  }

  void test_buildOSCData_whenShouldSkipCreatingOscElementReturnsFalse_shouldProceedWithCreatingOscElement()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesSubCodeInfo();
    uint16_t index = 0;
    EXPECT_CALL(*_ancFormatterMock, shouldSkipCreatingOscElement(_, _)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*_ancFormatterMock, buildOscContent(_, _, _, _)).Times(1);
    _ancFormatterMock->buildOSCData(*_ancTrx, *_construct, *paxOcFees, index);
    EXPECT_FALSE(_construct->getXMLData().empty());
  }

  void test_shouldSkipCreatingOscElement_whenNotHandlingSecondCallForMonetaryDiscount_shouldReturnFalse()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesSubCodeInfo();
    CPPUNIT_ASSERT(!_ancFormatter->shouldSkipCreatingOscElement(*_ancTrx, *paxOcFees));
  }

  void test_shouldSkipCreatingOscElement_givenSecondCallForMonetaryDiscount_whenPaxOcFeesHasUsages_shouldReturnFalse()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesSubCodeInfo();
    makeAncTrxSecondCallForMonetaryPricing();
    CPPUNIT_ASSERT(!_ancFormatter->shouldSkipCreatingOscElement(*_ancTrx, *paxOcFees));
  }

  void test_shouldSkipCreatingOscElement_givenSecondCallForMonetaryDiscount_whenPaxOcFeesHasNoUsages_shouldReturnTrue()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesSubCodeInfo();
    auto& usages = const_cast<std::vector<OCFeesUsage*>&>(paxOcFees->fees()->ocfeeUsage());
    usages.clear();
    makeAncTrxSecondCallForMonetaryPricing();
    CPPUNIT_ASSERT(_ancFormatter->shouldSkipCreatingOscElement(*_ancTrx, *paxOcFees));
  }

  void test_buildOSCOptionalData()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_Data()
  {
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    (const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee()))->advPurchTktIssue() = 'X';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML(
        "<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" SFN=\"A\" N03=\"F\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // DO - Display Only
  void test_buildOSCOptionalData_Data_WPDispAE_DO_True()
  {
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo(true);
    (const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee()))->advPurchTktIssue() = 'X';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->_isWPDispAE = true;
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC SFD=\"Y\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_Data_WPDispAE_DO_False()
  {
    _ancTrx->modifiableActivationFlags().setAB240(false);
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee()))->advPurchTktIssue() = 'X';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->_isWPDispAE = true;
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" SFD=\"N\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_Data_WPDispAE_DO_False_AB240()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee()))->advPurchTktIssue() = 'X';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->_isWPDispAE = true;
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC SFD=\"N\" N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFees oc;
    OCFeesUsage ocFeesUsage;
    ocFeesUsage.oCFees() = &oc;
    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, *paxFee, ocFeesUsage);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData_S7()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML(
        "<OOS SEQ=\"1234\"><Q00>01</Q00><SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
        "<SFQ/><DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // SP - Sector Portion Indicator
  void test_buildS7OOSData_S7_SP()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->sectorPortionInd() = 'P';

    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"P\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // AX1 - Avail Service Tag
  void test_buildS7OOSData_S7_SP_AX1()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->sectorPortionInd() = 'P';
    optI->availabilityInd() = 'Y';
    optI->notAvailNoChargeInd() = 'F';
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"P\" AX1=\"T\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT N43=\"F\"/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // MOA - Matched Origin Airport
  void test_buildS7OOSData_S7_SP_MOA()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.matchedOriginAirport() = "DFW";
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->sectorPortionInd() = 'P';

    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"P\" A01=\"DFW\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // MDA - Matched Destination Airport
  void test_buildS7OOSData_S7_SP_MOA_MDA()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.matchedOriginAirport() = "DFW";
    oc.matchedDestinationAirport() = "ORD";
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->sectorPortionInd() = 'P';
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"P\" A01=\"DFW\" A02=\"ORD\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData_S7_givenPriceModificationForOcFeesSegmentExists_whenBuildingS7OosData_shouldAddPmiAsAttribute()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(false, "BG", true);

    _ancTrx->modifiableActivationFlags().setMonetaryDiscount(true);
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));

    const std::string expectedXML(
        "<OOS SEQ=\"1234\" PMI=\"pmi\"><Q00>01</Q00><SFQ/>"
        "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/><BGA/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSegments()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    _ancFormatter->buildSegments(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<Q00>01</Q00>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSegments_multiple()
  {
    PaxOCFees* paxFee = createPaxOCFees();

    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelStart()));
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelEnd()));

    _ancFormatter->currentItin() = _itin;
    _ancFormatter->buildSegments(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<Q00>01</Q00><Q00>02</Q00>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSegments_multiple_Seg_Reverse()
  {
    PaxOCFees* paxFee = createPaxOCFees();

    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelEnd()));
    _itin->travelSeg().push_back(const_cast<TravelSeg*>(paxFee->fees()->travelStart()));

    _ancFormatter->currentItin() = _itin;
    _ancFormatter->buildSegments(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSegmentElement()
  {
    int16_t pnrSeg = 1;
    _ancFormatter->buildSegmentElement(*_construct, pnrSeg);
    const std::string expectedXML = "<Q00>01</Q00>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSegmentElement_Arunk()
  {
    int16_t pnrSeg = 255;
    _ancFormatter->buildSegmentElement(*_construct, pnrSeg);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSOptionalData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildS7OOSOptionalData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSOptionalData_RBD_Soft()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_RBD_SOFT;
    fillRBDData(*(paxFee->fees()->ocfeeUsage()[0]));
    _ancFormatter->buildS7OOSOptionalData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<RBD N51=\"E\" B51=\"LH\"><BKC>Y</BKC><BKC>H</BKC>"
                                  "<BKC>E</BKC><BKC>F</BKC><BKC>N</BKC></RBD>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSOptionalData_RF_Soft()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_RESULTING_FARE_SOFT;
    fillFCLData(*(paxFee->fees()->ocfeeUsage()[0]));
    _ancFormatter->buildS7OOSOptionalData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FCL B61=\"AA\" BJ0=\"Y26\" S53=\"ER\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSOptionalData_CF_Soft()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_CARRIER_FLIGHT_SOFT;
    fillCFData(*(paxFee->fees()->ocfeeUsage()[0]));
    _ancFormatter->buildS7OOSOptionalData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<CFT B71=\"AA\" B72=\"BA\" Q0B=\"142\" Q0C=\"10\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_getPadisCodeString()
  {
    const BookingCode padisCode = "L";
    std::map<BookingCode, std::string> padisCodeMap;
    const std::string expected = "LEGSP";
    padisCodeMap[padisCode] = expected;

    const std::string result = _ancFormatter->getPadisCodeString(padisCode, padisCodeMap);
    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void test_getPadisCodeString_NoMatch()
  {
    const BookingCode padisCode = "L";
    std::map<BookingCode, std::string> padisCodeMap;
    padisCodeMap[padisCode] = "LEGSP";

    const std::string empty;
    const std::string result = _ancFormatter->getPadisCodeString("", padisCodeMap);
    CPPUNIT_ASSERT_EQUAL(empty, result);
  }

  void test_buildPds()
  {
    const BookingCode padisCode = "L";
    std::map<BookingCode, std::string> padisCodeAbbreviationMap;
    std::map<BookingCode, std::string> padisCodeDescriptionMap;
    padisCodeAbbreviationMap[padisCode] = "LEGSP";
    padisCodeDescriptionMap[padisCode] = "LEG SPACE";

    _ancFormatter->buildPds(
        *_construct, padisCode, padisCodeAbbreviationMap, padisCodeDescriptionMap);
    const std::string expectedXML("<PDS PDI=\"L\" PCA=\"LEGSP\" SED=\"LEG SPACE\"/>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPds_NoDescription()
  {
    const BookingCode padisCode = "L";
    std::map<BookingCode, std::string> padisCodeAbbreviationMap;
    std::map<BookingCode, std::string> padisCodeDescriptionMap;
    padisCodeAbbreviationMap[padisCode] = "LEGSP";

    _ancFormatter->buildPds(
        *_construct, padisCode, padisCodeAbbreviationMap, padisCodeDescriptionMap);
    const std::string expectedXML("<PDS PDI=\"L\" PCA=\"LEGSP\"/>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPds_NoAbbreviation()
  {
    const BookingCode padisCode = "L";
    std::map<BookingCode, std::string> padisCodeAbbreviationMap;
    std::map<BookingCode, std::string> padisCodeDescriptionMap;
    padisCodeDescriptionMap[padisCode] = "LEG SPACE";

    _ancFormatter->buildPds(
        *_construct, padisCode, padisCodeAbbreviationMap, padisCodeDescriptionMap);
    const std::string expectedXML("<PDS PDI=\"L\" SED=\"LEG SPACE\"/>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPds_NoAbbreviation_NoDescription()
  {
    const BookingCode padisCode = "L";
    std::map<BookingCode, std::string> padisCodeAbbreviationMap;
    std::map<BookingCode, std::string> padisCodeDescriptionMap;

    _ancFormatter->buildPds(
        *_construct, padisCode, padisCodeAbbreviationMap, padisCodeDescriptionMap);
    const std::string expectedXML("<PDS PDI=\"L\"/>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPds_NoPadisCode()
  {
    const BookingCode padisCode;
    std::map<BookingCode, std::string> padisCodeAbbreviationMap;
    std::map<BookingCode, std::string> padisCodeDescriptionMap;

    _ancFormatter->buildPds(
        *_construct, padisCode, padisCodeAbbreviationMap, padisCodeDescriptionMap);
    const std::string expectedXML("<PDS/>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildUpc()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SvcFeesResBkgDesigInfo* padis = createPadis("AB", "", "", "", "", "**", 9);

    _ancFormatter->buildUpc(*_ancTrx, *_construct, *padis, *ocFeeUsage);
    const std::string expectedXML("<UPC SEU=\"9\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildUpc_DuplicatePadisCodes()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SvcFeesResBkgDesigInfo* padis = createPadis("AB", "AB", "AB", "AB", "AB", "**", 9);

    _ancFormatter->buildUpc(*_ancTrx, *_construct, *padis, *ocFeeUsage);
    const std::string expectedXML("<UPC SEU=\"9\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildUpc_NoPadisCode()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SvcFeesResBkgDesigInfo* padis = createPadis("", "", "", "", "", "**", 1);

    _ancFormatter->buildUpc(*_ancTrx, *_construct, *padis, *ocFeeUsage);
    const std::string expectedXML("<UPC SEU=\"1\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // Without the fare path (travel date) we can't get the PADIS abbreviation and description.
  // Note the missing PCA and SED attributes.
  void test_buildUpc_NoFarePath()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    SvcFeesResBkgDesigInfo* padis = createPadis("AB", "", "", "", "", "**", 1);

    _ancFormatter->buildUpc(*_ancTrx, *_construct, *padis, *ocFeeUsage);
    const std::string expectedXML("<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\"/>"
                                  "</UPC>");
    CPPUNIT_ASSERT(0 == ocFeeUsage->farePath());
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // Without the itinerary (travel date) we can't get the PADIS abbreviation and description.
  // Note the missing PCA and SED attributes.
  void test_buildUpc_NoItin()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    ocFeeUsage->farePath() = createFarePath();
    SvcFeesResBkgDesigInfo* padis = createPadis("AB", "", "", "", "", "**", 1);

    _ancFormatter->buildUpc(*_ancTrx, *_construct, *padis, *ocFeeUsage);
    const std::string expectedXML("<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\"/>"
                                  "</UPC>");
    CPPUNIT_ASSERT(0 == ocFeeUsage->farePath()->itin());
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPadis_OnePadis_OneCode()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("A", "", "", "", "", "**", 1));
    ocFeeUsage->padisData() = padisData;

    _ancFormatter->buildPadisData(*_ancTrx, *_construct, *ocFeeUsage);
    const std::string expectedXML("<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"A\" PCA=\"AISLE\" SED=\"AISLE\"/>"
                                  "</UPC>"
                                  "</PSP>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPadis_OnePadis_FiveCodes()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("A", "K", "AR", "Q", "E", "**", 1));
    ocFeeUsage->padisData() = padisData;

    _ancFormatter->buildPadisData(*_ancTrx, *_construct, *ocFeeUsage);
    const std::string expectedXML("<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"A\" PCA=\"AISLE\" SED=\"AISLE\"/>"
                                  "<PDS PDI=\"AR\" PCA=\"NOAPH\" SED=\"NO AIRPHONE\"/>"
                                  "<PDS PDI=\"E\" PCA=\"EXTRW\" SED=\"EXIT\"/>"
                                  "<PDS PDI=\"K\" PCA=\"BKHDS\" SED=\"BULK HEAD\"/>"
                                  "<PDS PDI=\"Q\" PCA=\"QUIET\" SED=\"SEAT IN QUIET ZONE\"/>"
                                  "</UPC>"
                                  "</PSP>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPadis_MultiplePadis_SingleCodes()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("A", "", "", "", "", "**", 1));
    padisData.push_back(createPadis("Q", "", "", "", "", "**", 2));
    ocFeeUsage->padisData() = padisData;

    _ancFormatter->buildPadisData(*_ancTrx, *_construct, *ocFeeUsage);
    const std::string expectedXML("<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"A\" PCA=\"AISLE\" SED=\"AISLE\"/>"
                                  "</UPC>"
                                  "<UPC SEU=\"2\">"
                                  "<PDS PDI=\"Q\" PCA=\"QUIET\" SED=\"SEAT IN QUIET ZONE\"/>"
                                  "</UPC>"
                                  "</PSP>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPadis_MultiplePadis_MultipleCodes()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("A", "Q", "", "", "", "**", 1));
    padisData.push_back(createPadis("K", "AR", "", "", "", "**", 2));
    ocFeeUsage->padisData() = padisData;

    _ancFormatter->buildPadisData(*_ancTrx, *_construct, *ocFeeUsage);
    const std::string expectedXML("<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"A\" PCA=\"AISLE\" SED=\"AISLE\"/>"
                                  "<PDS PDI=\"Q\" PCA=\"QUIET\" SED=\"SEAT IN QUIET ZONE\"/>"
                                  "</UPC>"
                                  "<UPC SEU=\"2\">"
                                  "<PDS PDI=\"AR\" PCA=\"NOAPH\" SED=\"NO AIRPHONE\"/>"
                                  "<PDS PDI=\"K\" PCA=\"BKHDS\" SED=\"BULK HEAD\"/>"
                                  "</UPC>"
                                  "</PSP>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData_Padis()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 771;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("AB", "", "", "", "", "**", 1));
    ocFeeUsage->padisData() = padisData;

    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);
    const std::string expectedXML("<OOS SEQ=\"1234\" URT=\"771\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/>"
                                  "<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>"
                                  "</PSP>"
                                  "</OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData_SA_NoPadis()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 0;

    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);

    const std::string expectedXML(
        "<OOS SEQ=\"1234\"><Q00>01</Q00><SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
        "<SFQ/><DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData_SA_NoPadis_ACS()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 0;
    _ancTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;

    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);

    const std::string expectedXML(
        "<OOS SEQ=\"1234\"><Q00>01</Q00><SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
        "<SFQ/><DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSUMData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.displayCurrency() = "EUR";
    oc.feeAmount() = 10;
    _ancTrx->ticketingDate() = DateTime(2011, 5, 1);
    _ancFormatter->buildSUMData(*_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SUM B70=\"ADT\" C51=\"10.00\" C52=\"10.00\" C5B=\"USD\" C50=\"10.00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // BC - Base Currency
  void test_buildSUMData_BC()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.displayCurrency() = "EUR";
    oc.feeCurrency() = "USD";
    oc.feeNoDec() = 2;
    oc.feeAmount() = 10;
    _ancTrx->ticketingDate() = DateTime(2011, 5, 1);
    _ancFormatter->buildSUMData(*_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SUM B70=\"ADT\" C51=\"10.00\" C5A=\"USD\" C50=\"10.00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSUMData_SFI()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.displayCurrency() = "EUR";
    oc.feeAmount() = 10;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->taxInclInd() = 'X';
    _ancTrx->ticketingDate() = DateTime(2011, 5, 1);
    _ancFormatter->buildSUMData(*_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML =
        "<SUM B70=\"ADT\" C51=\"10.00\" C52=\"10.00\" C5B=\"USD\" N21=\"X\" C50=\"10.00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSUMData_SFI_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.displayCurrency() = "EUR";
    oc.feeAmount() = 10;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->taxInclInd() = 'X';
    _ancTrx->ticketingDate() = DateTime(2011, 5, 1);
    _ancFormatter->_isWPDispAE = true;
    _ancFormatter->buildSUMData(*_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML =
        "<SUM B70=\"ADT\" C51=\"10.00\" C52=\"10.00\" C5B=\"USD\" N21=\"X\" C50=\"10.00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSUMData_givenOcFee_whenAncillaryIdMatchingThisOcFeeExistsAndMonetaryDiscountIsEnabled_shouldAddQtyWithValueFromItinToSum()
  {
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    PaxOCFees* paxOcFee = createPaxOCFeesOneTravelSeg(false, "BG", true);
    OCFeesUsage* ocFeeUsage = paxOcFee->fees()->ocfeeUsage()[0];
    ocFeeUsage->farePath() = fp;
    AncillaryIdentifier ancId(*(paxOcFee->fees()));
    unsigned int quantity = 2;
    AncillaryPriceModifier ancPriceModifier{std::string("id"), quantity};
    _itin->addAncillaryPriceModifier(ancId, ancPriceModifier);
    const_cast<OCFees*>(paxOcFee->fees())->getCurrentSeg()->_ancPriceModification = std::make_pair(ancId, ancPriceModifier);

    _ancTrx->modifiableActivationFlags().setMonetaryDiscount(true);
    _ancFormatter->buildSUMData(*_ancTrx, *_construct, *paxOcFee, *(ocFeeUsage));

    const std::string expectedXML =
        "<SUM B70=\"ADT\" QTY=\"" + std::to_string(quantity) + "\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_isWPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->_isWPDispAE = true;
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_C()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_CABIN_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->cabin() = 'J';
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ N1A=\"J\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_C_TC()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_CABIN_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->cabin() = 'J';
    optI->tourCode() = "TRAVEL";
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ N1A=\"J\" SHC=\"TRAVEL\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_C_TC_RT()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_RULETARIFF_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->tourCode() = "TRAVEL";
    optI->ruleTariff() = 10;
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\" Q0W=\"10\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_C_TC_RT_R()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_RULE_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->tourCode() = "TRAVEL";
    optI->rule() = "1234";
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\" SHP=\"1234\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_C_TC_RT_R_DOW()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_RULE_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->tourCode() = "TRAVEL";
    optI->rule() = "1234";
    optI->dayOfWeek() = "4567";

    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\" SHP=\"1234\" SHQ=\"4567\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_C_TC_RT_R_DOW_EC()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_EQUIPMENT_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->tourCode() = "TRAVEL";
    optI->rule() = "1234";
    optI->dayOfWeek() = "4567";
    optI->equipmentCode() = "CRB";

    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\" SHQ=\"4567\" SHR=\"CRB\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // TS - Time Soft
  void test_buildSFQData_C_TC_RT_R_DOW_EC_TS()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_TIME_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->tourCode() = "TRAVEL";
    optI->rule() = "1234";
    optI->dayOfWeek() = "4567";
    optI->equipmentCode() = "CRB";

    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\" SHQ=\"4567\"><STO/></SFQ>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_TC_DOW_TS()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_TIME_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->tourCode() = "TRAVEL";
    optI->dayOfWeek() = "4567";
    _ancFormatter->buildSFQData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\" SHQ=\"4567\"><STO/></SFQ>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildSFQDataR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSFQData_WPDispAE_TC()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    uint16_t index = 1;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>((paxFee->fees())->optFee());
    optI->tourCode() = "TRAVEL";
    _ancFormatter->buildSFQDataR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]), index);
    const std::string expectedXML = "<SFQ SHC=\"TRAVEL\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSTOData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildSTOData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<STO/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // SI - Stop Connection Destination Indicator
  void test_buildSTOData_SI()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>((paxFee->fees())->optFee());
    optI->stopCnxDestInd() = 'S';
    _ancFormatter->buildSTOData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<STO N12=\"S\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // ST - Stop Over Time
  void test_buildSTOData_SI_ST()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>((paxFee->fees())->optFee());
    optI->stopCnxDestInd() = 'S';
    optI->stopoverTime() = "360";
    _ancFormatter->buildSTOData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<STO N12=\"S\" Q01=\"360\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // SU - Stop Over Unit
  void test_buildSTOData_SI_ST_SU()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>((paxFee->fees())->optFee());
    optI->stopCnxDestInd() = 'S';
    optI->stopoverTime() = "360";
    optI->stopoverUnit() = 'N';
    _ancFormatter->buildSTOData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<STO N12=\"S\" Q01=\"360\" N13=\"N\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>((paxFee->fees())->optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<DTE D01=\"2011-02-01\" D02=\"0000-00-00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01_D02()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>((paxFee->fees())->optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<DTE D01=\"2011-02-01\" D02=\"2011-10-01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01_D02_D03NotValid()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";

    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    oc.purchaseByDate() = emptyDateValue;
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<DTE D01=\"2011-02-01\" "
                                  "D02=\"2011-10-01\" D03=\"1980-Jan-01\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01_D02_D03Valid()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";
    oc.purchaseByDate() = DateTime(2011, 3, 1);
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<DTE D01=\"2011-02-01\" "
                                  "D02=\"2011-10-01\" D03=\"2011-03-01\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // TS - Time Soft
  void test_buildDTEData_D01_D02_D03Valid_TS()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_TIME_SOFT;
    oc.purchaseByDate() = DateTime(2011, 3, 1);
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<DTE D01=\"2011-02-01\" D02=\"2011-10-01\" "
                                  "D03=\"2011-03-01\" Q11=\"0\" Q12=\"0\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01_D02_D03Valid_TS_Q11()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_TIME_SOFT;
    oc.purchaseByDate() = DateTime(2011, 3, 1);
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";
    optI->startTime() = 800;
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<DTE D01=\"2011-02-01\" D02=\"2011-10-01\" "
                                  "D03=\"2011-03-01\" Q11=\"800\" Q12=\"0\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01_D02_D03Valid_TS_Q11_Q12()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_TIME_SOFT;
    oc.purchaseByDate() = DateTime(2011, 3, 1);
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";
    optI->startTime() = 800;
    optI->stopTime() = 1900;
    _ancFormatter->buildDTEData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<DTE D01=\"2011-02-01\" D02=\"2011-10-01\" "
                                  "D03=\"2011-03-01\" Q11=\"800\" Q12=\"1900\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDTEData_D01_D02_D03Valid_TS_Q11_Q12_WPDispAE()
  {
    _ancFormatterMock->_isWPDispAE = true;
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.purchaseByDate() = DateTime(2011, 3, 1);
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";
    uint16_t index = 1;
    _ancFormatterMock->buildDTEData(
        *_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]), index);
    const std::string expectedXML =
        "<DTE D01=\"2011-02-01\" D02=\"2011-10-01\" D03=\"2011-03-01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildFATData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // FF - Frequest Flyer
  void test_buildFATData_FF()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->frequentFlyerMileageAppl() = 'Q';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // SP - Soft Pass
  void test_buildFATData_FF_SP()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.softMatchS7Status() = OCFees::S7_CABIN_SOFT;
    optI->frequentFlyerMileageAppl() = 'Q';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\" P01=\"T\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // FG - Fee Guarantee
  void test_buildFATData_FF_FG()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.isFeeGuaranteed() = false;
    optI->frequentFlyerMileageAppl() = 'Q';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\" P01=\"T\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // FG - Fee Guarantee when non-guarantee 'True' came from the WP*AE request
  void test_buildFATData_P01_WhenPdisplayAE_ANG_True()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->frequentFlyerMileageAppl() = 'Q';
    _ancFormatter->_ancillariesNonGuarantee = true;
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\" P01=\"T\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // AP - Fee Guarantee
  void test_buildFATData_FF_FG_AP()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.isFeeGuaranteed() = false;
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    _ancFormatter->_ancillariesNonGuarantee = false;
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\" P01=\"T\" N42=\"I\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // NCNA - No Charge Not Avail
  void test_buildFATData_FF_FG_AP_NCNA()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.isFeeGuaranteed() = false;
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    optI->notAvailNoChargeInd() = 'N';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\" P01=\"T\" N42=\"I\" N43=\"N\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // FR - Form of Refund
  void test_buildFATData_FF_FG_AP_NCNA_FR()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.isFeeGuaranteed() = false;
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    optI->notAvailNoChargeInd() = 'N';
    optI->formOfFeeRefundInd() = 'M';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FAT N41=\"Q\" P01=\"T\" N42=\"I\" N43=\"N\" N44=\"M\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // FFS - Frequest Flyer Status
  void test_buildFATData_FF_SP_AP_NCNA_FR_FFS()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    oc.softMatchS7Status() = OCFees::S7_FREQFLYER_SOFT;
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    optI->notAvailNoChargeInd() = 'N';
    optI->formOfFeeRefundInd() = 'M';
    optI->frequentFlyerStatus() = 7;
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<FAT N41=\"Q\" P01=\"T\" N42=\"I\" N43=\"N\" "
                                  "N44=\"M\" Q7D=\"7\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // RR - Refund ReIssue
  void test_buildFATData_FF_SP_AP_NCNA_FR_FFS_RR()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.softMatchS7Status() = OCFees::S7_FREQFLYER_SOFT;
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    optI->notAvailNoChargeInd() = 'N';
    optI->formOfFeeRefundInd() = 'M';
    optI->frequentFlyerStatus() = 7;
    optI->refundReissueInd() = 'J';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<FAT N41=\"Q\" P01=\"T\" N42=\"I\" N43=\"N\" "
                                  "N44=\"M\" Q7D=\"7\" N45=\"J\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // CI - Comission Indicator
  void test_buildFATData_FF_SP_AP_NCNA_FR_FFS_RR_CI()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.softMatchS7Status() = OCFees::S7_FREQFLYER_SOFT;
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    optI->notAvailNoChargeInd() = 'N';
    optI->formOfFeeRefundInd() = 'M';
    optI->frequentFlyerStatus() = 7;
    optI->refundReissueInd() = 'J';
    optI->commissionInd() = 'K';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<FAT N41=\"Q\" P01=\"T\" N42=\"I\" N43=\"N\" "
                                  "N44=\"M\" Q7D=\"7\" N45=\"J\" P03=\"K\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // II - Interline Indicator
  void test_buildFATData_FF_SP_AP_NCNA_FR_FFS_RR_CI_II()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    oc.softMatchS7Status() = OCFees::S7_FREQFLYER_SOFT;
    optI->frequentFlyerMileageAppl() = 'Q';
    optI->advPurchTktIssue() = 'I';
    optI->notAvailNoChargeInd() = 'N';
    optI->formOfFeeRefundInd() = 'M';
    optI->frequentFlyerStatus() = 7;
    optI->refundReissueInd() = 'J';
    optI->commissionInd() = 'K';
    optI->interlineInd() = 'L';
    _ancFormatter->buildFATData(*_ancTrx, *_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<FAT N41=\"Q\" P01=\"T\" N42=\"I\" N43=\"N\" "
                                  "N44=\"M\" Q7D=\"7\" N45=\"J\" P03=\"K\" P04=\"L\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildRBDData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildRBDData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildRBDData_empty()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    std::vector<SvcFeesResBkgDesigInfo*> svcFeeCxrResBkgDesInfoVec;
    svcFeeCxrResBkgDesInfoVec.push_back(createSvcFeesResBkgDesigInfo());
    oc.softMatchRBDT198() = svcFeeCxrResBkgDesInfoVec;
    _ancFormatter->buildRBDData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<RBD N51=\" \"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildRBDData_data()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    std::vector<SvcFeesResBkgDesigInfo*> svcFeeCxrResBkgDesInfoVec;
    SvcFeesResBkgDesigInfo* svcFeesResBkgDesigInfoData = createSvcFeesResBkgDesigInfo();
    svcFeesResBkgDesigInfoData->mkgOperInd() = 'E';
    svcFeesResBkgDesigInfoData->carrier() = "LH";
    svcFeesResBkgDesigInfoData->bookingCode1() = "Y";
    svcFeesResBkgDesigInfoData->bookingCode2() = "R";
    svcFeeCxrResBkgDesInfoVec.push_back(svcFeesResBkgDesigInfoData);
    oc.softMatchRBDT198() = svcFeeCxrResBkgDesInfoVec;
    _ancFormatter->buildRBDData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<RBD N51=\"E\" B51=\"LH\"><BKC>Y</BKC><BKC>R</BKC></RBD>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildRBDData_allData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    fillRBDData(*(paxFee->fees()->ocfeeUsage()[0]));
    _ancFormatter->buildRBDData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<RBD N51=\"E\" B51=\"LH\"><BKC>Y</BKC><BKC>H</BKC>"
                                  "<BKC>E</BKC><BKC>F</BKC><BKC>N</BKC></RBD>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildBKCData()
  {
    BookingCode bk;
    _ancFormatter->buildBKCData(*_construct, bk);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildBKCData_data()
  {
    BookingCode bk = "A";
    _ancFormatter->buildBKCData(*_construct, bk);
    const std::string expectedXML = "<BKC>A</BKC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildFCLData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildFCLData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildFCLData_empty()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    std::vector<SvcFeesCxrResultingFCLInfo*> svcFeeCxrResFCLInfoVec;
    svcFeeCxrResFCLInfoVec.push_back(createSvcFeesCxrResultingFCLInfo());
    oc.softMatchResultingFareClassT171() = svcFeeCxrResFCLInfoVec;
    _ancFormatter->buildFCLData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FCL/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildFCLData_data()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    fillFCLData(*(paxFee->fees()->ocfeeUsage()[0]));
    _ancFormatter->buildFCLData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<FCL B61=\"AA\" BJ0=\"Y26\" S53=\"ER\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildCFTData()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    _ancFormatter->buildCFTData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildCFTData_empty()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    std::vector<CarrierFlightSeg*> cfsVec;
    cfsVec.push_back(createCxrFltSeg());
    oc.softMatchCarrierFlightT186() = cfsVec;
    _ancFormatter->buildCFTData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<CFT Q0B=\"0\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildCFTData_data()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    std::vector<CarrierFlightSeg*> cfsVec;
    CarrierFlightSeg* cfsData = createCxrFltSeg();
    cfsData->marketingCarrier() = "AA";
    cfsData->operatingCarrier() = "BA";
    cfsData->flt1() = 0;
    cfsData->flt2() = 142;
    cfsVec.push_back(cfsData);
    oc.softMatchCarrierFlightT186() = cfsVec;
    _ancFormatter->buildCFTData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<CFT B71=\"AA\" B72=\"BA\" Q0B=\"0\" Q0C=\"142\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildCFTData_data1()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    std::vector<CarrierFlightSeg*> cfsVec;
    CarrierFlightSeg* cfsData = createCxrFltSeg();
    cfsData->marketingCarrier() = "AA";
    cfsData->operatingCarrier() = "BA";
    cfsData->flt1() = 142;
    cfsData->flt2() = 0;
    cfsVec.push_back(cfsData);
    oc.softMatchCarrierFlightT186() = cfsVec;
    _ancFormatter->buildCFTData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<CFT B71=\"AA\" B72=\"BA\" Q0B=\"142\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildCFTData_data2()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    fillCFData(*(paxFee->fees()->ocfeeUsage()[0]));
    _ancFormatter->buildCFTData(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML = "<CFT B71=\"AA\" B72=\"BA\" Q0B=\"142\" Q0C=\"10\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketEffDate_validDate()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketEffDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D01=\"2011-02-01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketEffDate_validDate_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketEffDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D01=\"11/02/01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketEffDate_startValues()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->tvlStartYear() = 2011;
    optI->tvlStartMonth() = 12;
    optI->tvlStartDay() = 15;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketEffDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D01=\"2011-12-15\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketEffDate_startValues_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    boost::gregorian::date d1(neg_infin);
    optI->ticketEffDate() = DateTime(d1);
    optI->tvlStartYear() = 11;
    optI->tvlStartMonth() = 12;
    optI->tvlStartDay() = 15;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketEffDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D01=\"11/12/15\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketEffDate_openDate()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    optI->ticketEffDate() = emptyDateValue;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketEffDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D01=\"1980-01-01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketEffDate_openDate_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    optI->ticketEffDate() = emptyDateValue;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketEffDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D01=\"80/01/01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_validDate()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->ticketDiscDate() = DateTime(2011, 1, 1);
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"2011-01-01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_validDate_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->ticketDiscDate() = DateTime(2011, 1, 1);
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"11/01/01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_infinity()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    boost::gregorian::date d1(neg_infin);
    optI->ticketDiscDate() = DateTime(d1);
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"9999-12-31\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_infinity_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    boost::gregorian::date d1(neg_infin);
    optI->ticketDiscDate() = DateTime(d1);
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"99/12/31\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_openDate()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    optI->ticketDiscDate() = emptyDateValue;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"0000-00-00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_openDate_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    optI->ticketDiscDate() = emptyDateValue;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"00/00/00\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_openDate_stopValues()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    optI->ticketDiscDate() = emptyDateValue;
    optI->tvlStopYear() = 2011;
    optI->tvlStopMonth() = 2;
    optI->tvlStopDay() = 25;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"2011-02-25\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processTicketDiscDate_openDate_stopValues_WPDispAE()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    optI->ticketDiscDate() = emptyDateValue;
    optI->tvlStopYear() = 2011;
    optI->tvlStopMonth() = 2;
    optI->tvlStopDay() = 25;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processTicketDiscDateForR7(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D02=\"2011/02/25\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_processPurchaseByDate_validDate()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
                            boost::posix_time::time_duration(0, 0, 0));
    // optServicesInfo->ticketDiscDate() = emptyDateValue;
    oc.purchaseByDate() = emptyDateValue;
    _construct->openElement(xml2::DateTimeInfo); // DTE Open
    _ancFormatter->processPurchaseByDate(*_construct, *(paxFee->fees()->ocfeeUsage()[0]));
    _construct->closeElement(); // DTE Close
    const std::string expectedXML = "<DTE D03=\"1980-Jan-01\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesForR7()
  {
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->formatOCFeesForR7(*_ancTrx, *_construct, false);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesForR7_Group_Empty()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    srvFeesGroup->state() = ServiceFeesGroup::EMPTY;
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->formatOCFeesForR7(*_ancTrx, *_construct, false);
    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"BG\" ST0=\"NF\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesForR7_Group_NOT_AVAILABLE()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    srvFeesGroup->state() = ServiceFeesGroup::NOT_AVAILABLE;
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->formatOCFeesForR7(*_ancTrx, *_construct, false);
    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"BG\" ST0=\"NA\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_NotAvailable_NoPax()
  {
    ServiceFeesGroup srvFeesGroup;
    srvFeesGroup.groupCode() = "SA";
    srvFeesGroup.state() = ServiceFeesGroup::NOT_AVAILABLE;

    std::vector<PaxOCFees> paxOcFees;
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFees));

    const bool timeOutMax = false;
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"SA\">"
                                    "<MSG N06=\"X\" Q0K=\"3\" S18=\"NOT AVAILABLE\"/>"
                                    "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_Empty_NoPax()
  {
    ServiceFeesGroup srvFeesGroup;
    srvFeesGroup.groupCode() = "SA";
    srvFeesGroup.state() = ServiceFeesGroup::EMPTY;

    std::vector<PaxOCFees> paxOcFees;
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFees));

    const bool timeOutMax = false;
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"SA\">"
                                    "<MSG N06=\"X\" Q0K=\"3\" S18=\"NOT FOUND\"/>"
                                    "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_Invalid_NoPax()
  {
    ServiceFeesGroup srvFeesGroup;
    srvFeesGroup.groupCode() = "SA";
    srvFeesGroup.state() = ServiceFeesGroup::INVALID;

    std::vector<PaxOCFees> paxOcFees;
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFees));

    const bool timeOutMax = false;
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"SA\">"
                                    "<MSG N06=\"X\" Q0K=\"3\" S18=\"NOT VALID\"/>"
                                    "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_Valid_NoPax()
  {
    ServiceFeesGroup srvFeesGroup;
    srvFeesGroup.groupCode() = "SA";
    srvFeesGroup.state() = ServiceFeesGroup::VALID;

    std::vector<PaxOCFees> paxOcFees;
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFees));

    const bool timeOutMax = false;
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"SA\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_whenMonetaryDiscountIsEnabled_shouldSurroundOosWithOccAndAddAidAttributeToOcc()
  {
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    PaxOCFees* paxOcFee = createPaxOCFeesOneTravelSeg(false, "BG", true);
    ServiceFeesGroup* srvFeesGroup = createBasicServiceFeesGroup();
    std::vector<PaxOCFees> paxOcFees{*paxOcFee};
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(srvFeesGroup, paxOcFees));

    const bool timeOutMax = false;
    _ancTrx->modifiableActivationFlags().setMonetaryDiscount(true);
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    AncillaryIdentifier ancId(*(paxOcFee->fees()));
    std::string expectedXML  = "<OCG Q00=\"1\" SF0=\"SA\">"
                                 "<OSC SHK=\"0DF\" SFF=\"BAGGAGE\" N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"A\">";
                expectedXML +=     "<OCC AID=\"" + ancId.getIdentifier() + "\">";
                expectedXML +=       "<OOS SEQ=\"1234\" PMI=\"pmi\">"
                                       "<Q00>01</Q00>"
                                       "<SFQ/>"
                                       "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                       "<FAT/>"
                                       "<BGA/>"
                                     "</OOS>"
                                   "</OCC>"
                                 "</OSC>"
                               "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_givenMonetaryDiscountIsEnabled_whenOscHasMultipleOosElements_shouldGroupOosElementsInOccElementsByAid()
  {
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    PaxOCFees* paxOcFee1 = createPaxOcFeesWithManyOcFeesUsagesThatHaveDifferentAid();
    PaxOCFees* paxOcFee2 = createPaxOCFeesOneTravelSeg(false, "BG", true);
    ServiceFeesGroup* srvFeesGroup = createBasicServiceFeesGroup();
    std::vector<PaxOCFees> paxOcFees{*paxOcFee1, *paxOcFee2};
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(srvFeesGroup, paxOcFees));

    const bool timeOutMax = false;
    _ancTrx->modifiableActivationFlags().setMonetaryDiscount(true);
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    OCFees* ocFees1 = const_cast<OCFees*>(paxOcFee1->fees());
    ocFees1->setSeg(0);
    AncillaryIdentifier ancId1(*ocFees1);
    ocFees1->setSeg(1);
    AncillaryIdentifier ancId2(*ocFees1);
    AncillaryIdentifier ancId3(*(paxOcFee2->fees()));
    std::string expectedXML  = "<OCG Q00=\"2\" SF0=\"SA\">"
                                 "<OSC SHK=\"0DF\" SFF=\"BAGGAGE\" N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"A\">";
                expectedXML +=     "<OCC AID=\"" + ancId1.getIdentifier() + "\">";
                expectedXML +=       "<OOS SEQ=\"1234\" PMI=\"pmi\">"
                                       "<Q00>01</Q00>"
                                       "<SFQ/>"
                                       "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                       "<FAT/>"
                                       "<BGA/>"
                                     "</OOS>"
                                     "<OOS SEQ=\"1234\" PMI=\"pmi\">"
                                       "<Q00>01</Q00>"
                                       "<SFQ/>"
                                       "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                       "<FAT/>"
                                       "<BGA/>"
                                     "</OOS>"
                                   "</OCC>";
                expectedXML +=     "<OCC AID=\"" + ancId2.getIdentifier() + "\">";
                expectedXML +=       "<OOS SEQ=\"4321\">"
                                       "<Q00>01</Q00>"
                                       "<SFQ/>"
                                       "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                       "<FAT/>"
                                       "<BGA/>"
                                     "</OOS>"
                                   "</OCC>"
                                 "</OSC>"
                                 "<OSC SHK=\"0DF\" SFF=\"BAGGAGE\" N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"A\">";
                expectedXML +=     "<OCC AID=\"" + ancId3.getIdentifier() + "\">";
                expectedXML +=       "<OOS SEQ=\"1234\" PMI=\"pmi\">"
                                       "<Q00>01</Q00>"
                                       "<SFQ/>"
                                       "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                       "<FAT/>"
                                       "<BGA/>"
                                     "</OOS>"
                                   "</OCC>"
                                 "</OSC>"
                               "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_SA_Padis()
  {
    const bool displayOnly = false;
    const bool isS7 = true;
    PaxOCFees* paxFee = createPaxOCFees(displayOnly, "SA", isS7);

    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1111;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 123;

    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("A", "", "", "", "", "**", 1));
    padisData.push_back(createPadis("Q", "", "", "", "", "**", 2));
    ocFeeUsage->padisData() = padisData;

    ServiceFeesGroup srvFeesGroup;
    srvFeesGroup.groupCode() = "SA";
    srvFeesGroup.state() = ServiceFeesGroup::VALID;

    std::vector<PaxOCFees> paxOcFees;
    paxOcFees.push_back(*paxFee);

    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFees));

    SubCodeInfo& subCodeInfo = const_cast<SubCodeInfo&>(*(paxFee->fees()->subCodeInfo()));
    subCodeInfo.serviceSubTypeCode() = "0B5";
    subCodeInfo.commercialName() = "SEAT ASSIGNMENT";
    subCodeInfo.vendor() = "MMGR";
    subCodeInfo.carrier() = "CC";
    subCodeInfo.rfiCode() = 'A';
    subCodeInfo.emdType() = '2';
    _ancFormatter->currentItin() = _itin;

    const bool timeOutMax = false;
    _ancFormatter->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, timeOutMax);

    const std::string expectedXML = "<OCG Q00=\"1\" SF0=\"SA\">"
                                    "<OSC SHK=\"0B5\" SFF=\"SEAT ASSIGNMENT\" SFV=\"MMGR\" "
                                    "B01=\"CC\" N01=\"A\" N02=\"2\" ASD=\"SPORTING EQUIPMENT\">"
                                    "<OOS SEQ=\"1111\" URT=\"123\">"
                                    "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                    "<SFQ/>"
                                    "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                    "<FAT/>"
                                    "<PSP>"
                                    "<UPC SEU=\"1\">"
                                    "<PDS PDI=\"A\" PCA=\"AISLE\" SED=\"AISLE\"/>"
                                    "</UPC>"
                                    "<UPC SEU=\"2\">"
                                    "<PDS PDI=\"Q\" PCA=\"QUIET\" SED=\"SEAT IN QUIET ZONE\"/>"
                                    "</UPC>"
                                    "</PSP>"
                                    "</OOS>"
                                    "</OSC>"
                                    "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesGroups_whenShouldSkipCreatingOcgElementMethodReturnsTrue_shouldSkipCreatingOcgElement()
  {
    bool irrelevantArg = false;
    PaxOCFees* paxOcFees = createPaxOcFeesWithManyOcFeesUsagesThatHaveDifferentAid();
    ServiceFeesGroup* srvFeesGroup = createBasicServiceFeesGroup();
    std::vector<PaxOCFees> paxOcFeesVector{*paxOcFees};
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(srvFeesGroup, paxOcFeesVector));

    EXPECT_CALL(*_ancFormatterMock, shouldSkipCreatingOcgElement(_, _)).Times(1).WillOnce(Return(true));
    _ancFormatterMock->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, irrelevantArg);

    EXPECT_TRUE(_construct->getXMLData().empty());
  }

  void test_formatOCFeesGroups_whenShouldSkipCreatingOcgElementMethodReturnsFalse_shouldNotSkipCreatingOcgElement()
  {
    bool irrelevantArg = false;
    PaxOCFees* paxOcFees = createPaxOcFeesWithManyOcFeesUsagesThatHaveDifferentAid();
    ServiceFeesGroup* srvFeesGroup = createBasicServiceFeesGroup();
    std::vector<PaxOCFees> paxOcFeesVector{*paxOcFees};
    GroupFeesVector groupFeesVector;
    groupFeesVector.push_back(std::make_pair(srvFeesGroup, paxOcFeesVector));

    EXPECT_CALL(*_ancFormatterMock, shouldSkipCreatingOcgElement(_, _)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*_ancFormatterMock, shouldSkipCreatingOscElement(_, _)).Times(1).WillOnce(Return(true));
    _ancFormatterMock->formatOCFeesGroups(*_ancTrx, *_construct, groupFeesVector, irrelevantArg);

    EXPECT_FALSE(_construct->getXMLData().empty());
  }

  void test_shouldSkipCreatingOcgElement_whenNotHandlingSecondCallForMonetaryDiscount_shouldReturnFalse()
  {
    std::vector<PaxOCFees> paxOcFeesVector;
    CPPUNIT_ASSERT(!_ancFormatter->shouldSkipCreatingOcgElement(*_ancTrx, paxOcFeesVector));
  }

  void test_shouldSkipCreatingOcgElement_givenPaxOcFeesUsagesArePresent_whenHandlingSecondCallForMonetaryDiscount_shouldReturnFalse()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesOneTravelSeg(false, "BG", true);
    std::vector<PaxOCFees> paxOcFeesVector{*paxOcFees};

    makeAncTrxSecondCallForMonetaryPricing();
    CPPUNIT_ASSERT(!_ancFormatter->shouldSkipCreatingOcgElement(*_ancTrx, paxOcFeesVector));
  }

  void test_shouldSkipCreatingOcgElement_givenNoPaxOcFeesUsages_whenHandlingSecondCallForMonetaryDiscount_shouldReturnTrue()
  {
    PaxOCFees* paxOcFees = createPaxOCFeesOneTravelSeg(false, "BG", true);
    auto& usages = const_cast<std::vector<OCFeesUsage*>&>(paxOcFees->fees()->ocfeeUsage());
    usages.clear();
    std::vector<PaxOCFees> paxOcFeesVector{*paxOcFees};

    makeAncTrxSecondCallForMonetaryPricing();
    CPPUNIT_ASSERT(_ancFormatter->shouldSkipCreatingOcgElement(*_ancTrx, paxOcFeesVector));
  }

  void test_formatOCFees_SA_Padis()
  {
    const bool displayOnly = false;
    const bool isS7 = true;
    PaxOCFees* paxFee = createPaxOCFees(displayOnly, "SA", isS7);
    OCFees* ocFees = const_cast<OCFees*>(paxFee->fees());

    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(ocFees->optFee());
    osi->seqNo() = 1111;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 1234;

    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* farePath = createFarePath();
    farePath->itin() = _itin;
    ocFeeUsage->farePath() = farePath;

    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("A", "", "", "", "", "**", 1));
    padisData.push_back(createPadis("Q", "", "", "", "", "**", 2));
    ocFeeUsage->padisData() = padisData;

    ServiceFeesGroup srvFeesGroup;
    srvFeesGroup.groupCode() = "SA";
    srvFeesGroup.state() = ServiceFeesGroup::VALID;
    srvFeesGroup.ocFeesMap()[farePath].push_back(ocFees);

    _itin->ocFeesGroup().push_back(&srvFeesGroup);

    SubCodeInfo& subCodeInfo = const_cast<SubCodeInfo&>(*(paxFee->fees()->subCodeInfo()));
    SubCodeInfo::dummyData(subCodeInfo);
    subCodeInfo.serviceGroup() = "SA";

    _ancFormatter->currentItin() = _itin;

    PaxType paxType;
    paxType.paxType() = ADULT;
    paxType.requestedPaxType() = ADULT;
    paxType.number() = 1;
    _ancRequest->paxTypesPerItin()[_ancFormatter->currentItin()].push_back(&paxType);

    _ancFormatter->_doNotProcess = false;
    const bool timeOutMax = false;
    _ancFormatter->formatOCFees(*_ancTrx, *_construct, timeOutMax);

    const std::string expectedXML =
        "<OCG Q00=\"1\" SF0=\"SA\">"
        "<OSC SHK=\"IJK\" SFF=\"123456789012345678901234567890\" SFV=\"ABCD\" "
        "B01=\"EF\" N01=\"Y\" SHL=\"ZABC\" N02=\"G\" SFN=\"HI\" N03=\"F\" PR0=\"T\" "
        "ASG=\"QRS\" ASD=\"SPORTING EQUIPMENT\" AST=\"L\">"
        "<OOS SEQ=\"1111\" URT=\"1234\">"
        "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
        "<SFQ/>"
        "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
        "<FAT/>"
        "<PSP>"
        "<UPC SEU=\"1\">"
        "<PDS PDI=\"A\" PCA=\"AISLE\" SED=\"AISLE\"/>"
        "</UPC>"
        "<UPC SEU=\"2\">"
        "<PDS PDI=\"Q\" PCA=\"QUIET\" SED=\"SEAT IN QUIET ZONE\"/>"
        "</UPC>"
        "</PSP>"
        "</OOS>"
        "</OSC>"
        "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
    _itin->ocFeesGroup().clear();
  }

  void test_formatOCFees_NotAvailable()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "SA";
    srvFeesGroup->state() = ServiceFeesGroup::NOT_AVAILABLE;
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    const bool timeOutMax = false;
    _ancFormatter->formatOCFees(*_ancTrx, *_construct, timeOutMax);
    const std::string expectedXML = "<OCG Q00=\"0\" SF0=\"SA\">"
                                    "<MSG N06=\"X\" Q0K=\"3\" S18=\"NOT AVAILABLE\"/>"
                                    "</OCG>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
    _itin->ocFeesGroup().clear();
  }

  void test_timeOutMaxCharCountNoOCFeesReturned_BG_Data_SWS_PATH()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    _ancTrx->billing()->requestPath() = SWS_PO_ATSE_PATH;
    _ancFormatter->timeOutMaxCharCountNoOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML = "<OCG SF0=\"BG\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_timeOutMaxCharCountNoOCFeesReturned_BG_Data()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->timeOutMaxCharCountNoOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML("<OCF><MSG S18=\"AIR EXTRAS MAY APPLY - "
                                  "USE WPAE WITH SERVICE QUALIFIER\"/></OCF><OCG SF0=\"BG\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_timeOutMaxCharCountNoOCFeesReturned_No_Data_SWS_PATH()
  {
    _ancFormatter->currentItin() = _itin;
    _ancTrx->billing()->requestPath() = SWS_PO_ATSE_PATH;
    _ancFormatter->timeOutMaxCharCountNoOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_timeOutMaxCharCountNoOCFeesReturned_No_Data()
  {
    _ancFormatter->currentItin() = _itin;
    _ancFormatter->timeOutMaxCharCountNoOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML("<OCF><MSG S18=\"AIR EXTRAS MAY APPLY - "
                                  "USE WPAE WITH SERVICE QUALIFIER\"/></OCF>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_timeOutMaxCharCountRequestedOCFeesReturned_No_Data()
  {
    _ancFormatter->currentItin() = _itin;

    _ancFormatter->timeOutMaxCharCountRequestedOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_timeOutMaxCharCountRequestedOCFeesReturned_No_Data_SWS_PATH()
  {
    _ancFormatter->currentItin() = _itin;
    _ancTrx->billing()->requestPath() = SWS_PO_ATSE_PATH;

    _ancFormatter->timeOutMaxCharCountRequestedOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_anyTimeOutMaxCharCountIssue_timeOutForExceeded_True()
  {
    _itin->timeOutForExceeded() = true;
    _ancFormatter->currentItin() = _itin;

    _ancFormatter->anyTimeOutMaxCharCountIssue(*_ancTrx, *_construct);
    const std::string expectedXML("<OCF><MSG S18=\"AIR EXTRAS MAY APPLY - "
                                  "USE WPAE WITH SERVICE QUALIFIER\"/></OCF>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_anyTimeOutMaxCharCountIssue_timeOutOCFForWP_True()
  {
    _itin->timeOutOCFForWP() = true;
    _ancFormatter->currentItin() = _itin;

    _ancFormatter->anyTimeOutMaxCharCountIssue(*_ancTrx, *_construct);
    const std::string expectedXML("<OCF><MSG S18=\"AIR EXTRAS MAY APPLY - "
                                  "USE WPAE WITH SERVICE QUALIFIER\"/></OCF>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_anyTimeOutMaxCharCountIssue_timeOutForExceededSFGpresent_True()
  {
    _itin->timeOutForExceededSFGpresent() = true;
    _ancFormatter->currentItin() = _itin;

    _ancFormatter->anyTimeOutMaxCharCountIssue(*_ancTrx, *_construct);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_timeOutMaxCharCountNoOCFeesReturned_BG_Data_R7Post_OneItins()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _itin->timeOutForExceeded() = true;
    PaxType paxT;
    paxT.paxType() = ADULT;
    _ancRequest->paxTypesPerItin()[_itin].push_back(&paxT);

    _ancFormatter->currentItin() = _itin;
    _ancFormatter->_isWpAePostTicket = true;
    _ancFormatter->timeOutMaxCharCountNoOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML("<OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE "
                                  "SVC QUALIFIER\"/></OCF><OCG SF0=\"BG\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }
  void test_timeOutMaxCharCountNoOCFeesReturned_BG_Data_R7Post_TwoItins()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    _itin->timeOutForExceeded() = true;
    Itin itin1;
    _ancTrx->itin().push_back(&itin1);

    _ancFormatter->currentItin() = _itin;
    _ancFormatter->_isWpAePostTicket = true;
    _ancFormatter->timeOutMaxCharCountNoOCFeesReturned(*_ancTrx, *_construct);
    const std::string expectedXML("<OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE "
                                  "SVC QUALIFIER OR TKT SELECT\"/></OCF><OCG SF0=\"BG\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isTimeOutBeforeStartOCFees_True()
  {
    _itin->timeOutForExceeded() = true;
    _ancFormatter->currentItin() = _itin;
    _itin->setItinOrderNum(1);
    _ancFormatter->_isWpAePostTicket = true;
    CPPUNIT_ASSERT(_ancFormatter->isTimeOutBeforeStartOCFees(*_ancTrx, *_construct));

    const std::string expectedXML("<ITN Q00=\"1\"><OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE "
                                  "SVC QUALIFIER OR SEG SELECT\"/></OCF></ITN>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isTimeOutBeforeStartOCFees_False()
  {
    _itin->timeOutForExceeded() = false;
    _ancFormatter->currentItin() = _itin;
    CPPUNIT_ASSERT(!_ancFormatter->isTimeOutBeforeStartOCFees(*_ancTrx, *_construct));
  }

  void test_samePaxType_True()
  {
    _ancFormatter->currentItin() = _itin;
    PaxType paxT;
    paxT.paxType() = ADULT;
    _ancRequest->paxTypesPerItin()[_itin].push_back(&paxT);
    CPPUNIT_ASSERT(_ancFormatter->samePaxType(*_ancTrx));
  }
  void test_samePaxType_False()
  {
    _ancFormatter->currentItin() = _itin;
    PaxType paxT;
    paxT.paxType() = ADULT;
    PaxType paxT1;
    paxT1.paxType() = "CNN";
    _ancRequest->paxTypesPerItin()[_itin].push_back(&paxT);
    _ancRequest->paxTypesPerItin()[_itin].push_back(&paxT1);
    CPPUNIT_ASSERT(!_ancFormatter->samePaxType(*_ancTrx));
  }

  // EMD-S 2 test OSC/PR0 and OOS/PY0

  // PR0 - S5 consumption at issuance indicator
  void test_buildOSCOptionalData_Data_M70_PR0_Y()
  {
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->consumptionInd() = 'Y';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" PR0=\"Y\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }
  void test_buildOSCOptionalData_Data_M70_PR0_N()
  {
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->consumptionInd() = 'N';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" PR0=\"N\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }
  void test_buildOSCOptionalData_Data_M70_PR0_Blank()
  {
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->consumptionInd() = ' ';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_Data_WPAE_PR0_Y()
  {
    _ancTrx->modifiableActivationFlags().setAB240(false);
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->consumptionInd() = 'Y';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->_isWPDispAE = true;
    _ancRequest->ancRequestType() = AncRequest::WPAERequest;
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" SFD=\"N\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_Data_WPAE_PR0_Y_AB240()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    PaxOCFees* paxFee = createPaxOCFeesSubCodeInfo();
    uint16_t index = 1;
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->consumptionInd() = 'Y';
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->_isWPDispAE = true;
    _ancRequest->ancRequestType() = AncRequest::WPAERequest;
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, index);
    _construct->closeElement(); // OSC close
    const std::string expectedXML("<OSC SFD=\"N\" N01=\"Y\" SHL=\"ABCD\" N02=\"G\" "
                                  "SFN=\"A\" N03=\"F\" ASD=\"SPORTING EQUIPMENT\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // PY0 - Tax Exempt Indicator Tag
  void test_buildS7OOSData_S7_M70_PY0_Y()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->sectorPortionInd() = 'S';
    optI->taxExemptInd() = 'Y';
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"S\" PY0=\"Y\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }
  void test_buildS7OOSData_S7_M70_PY0_N()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->sectorPortionInd() = 'S';
    optI->taxExemptInd() = 'N';
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"S\" PY0=\"N\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }
  void test_buildS7OOSData_S7_M70_PY0_Blank()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->sectorPortionInd() = 'S';
    optI->taxExemptInd() = ' ';
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"S\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7OOSData_S7_WPAE_PY0_Y()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    optI->sectorPortionInd() = 'S';
    optI->taxExemptInd() = 'Y';
    _ancRequest->ancRequestType() = AncRequest::WPAERequest;
    _ancFormatter->buildS7OOSData(
        *_ancTrx, *_construct, *paxFee, *(paxFee->fees()->ocfeeUsage()[0]));
    const std::string expectedXML("<OOS N11=\"S\" SEQ=\"1234\"><Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/><SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/></OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  SubCodeInfo* createSubCodeInfo()
  {
    SubCodeInfo* obj = _memH.insert(new SubCodeInfo);
    SubCodeInfo::dummyData(*obj);
    obj->description1() = "10";
    obj->description2() = "12";
    obj->taxTextTblItemNo() = 2;
    return obj;
  }

  void test_buildOSCOptionalData_MSG()
  {
    static_cast<AncRequest*>(_ancTrx->getRequest())->majorSchemaVersion() = 2;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->vendor() = "ABCD";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->taxTextTblItemNo() = 2;
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\"><MSG N06=\"X\" "
                                    "Q0K=\"0\" S18=\"BAGGAGE IS NOT ALLOWED\"/>"
                                    "<MSG N06=\"X\" Q0K=\"0\" S18=\"//\"/></OSC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_BIR_SDC()
  {
    static_cast<AncRequest*>(_ancTrx->getRequest())->majorSchemaVersion() = 2;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\">"
                                    "<BIR B20=\"3.4\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22.5\" N0D=\"L\" L01=\"U\"/>"
                                    "<BIR B20=\"10\" N0D=\"L\" L01=\"O\"/>"
                                    "<BIR B20=\"3\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22\" N0D=\"C\" L01=\"U\"/>"
                                    "<BIR B20=\"5\" N0D=\"I\" L01=\"O\"/>"
                                    "<SDC DC1=\"10\" D00=\"TEST10\"/>"
                                    "<SDC DC1=\"12\" D00=\"TEST12\"/></OSC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildBGA()
  {
    PaxOCFees* paxFee = createPaxOCFees();
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());

    OptionalServicesInfo::dummyData(*optI);

    OCFeesUsage ocFeesUsage = *(paxFee->fees()->ocfeeUsage()[0]);

    ocFeesUsage.getBaggageProperty().push_back(
        OCFees::BaggageItemProperty("NO CHECKED BAG ALLOWANCE FOR LAP INFANT WITHOUT SEAT"));
    ocFeesUsage.getBaggageProperty().push_back(OCFees::BaggageItemProperty("//"));
    ocFeesUsage.getBaggageProperty().push_back(OCFees::BaggageItemProperty(3, createSubCodeInfo()));
    ocFeesUsage.getBaggageProperty().push_back(OCFees::BaggageItemProperty(4, createSubCodeInfo()));
    ocFeesUsage.getBaggageProperty().push_back(OCFees::BaggageItemProperty(6, createSubCodeInfo()));

    CPPUNIT_ASSERT(_ancFormatter->_baggageSizeWeightRestrictions.size() == 6);

    _ancRequest->ancRequestType() = AncRequest::WPAERequest;
    _ancFormatter->buildBGAData(*_ancTrx, *_construct, ocFeesUsage);

    const std::string expectedITT =
        "<ITT SHK=\"IJK\" SFF=\"123456789012345678901234567890\" SFV=\"ABCD\" "
        "B01=\"EF\" N01=\"Y\" SHL=\"ZABC\" N02=\"G\" SFN=\"HI\" N03=\"F\" ASG=\"QRS\" "
        "ASD=\"SPORTING EQUIPMENT\" AST=\"L\">";

    const std::string expectedBIR = "<BIR B20=\"3.4\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22.5\" N0D=\"L\" L01=\"U\"/>"
                                    "<BIR B20=\"10\" N0D=\"L\" L01=\"O\"/>"
                                    "<BIR B20=\"3\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22\" N0D=\"C\" L01=\"U\"/>"
                                    "<BIR B20=\"5\" N0D=\"I\" L01=\"O\"/>";

    const std::string expectedSDC =
        "<SDC DC1=\"10\" D00=\"TEST10\"/><SDC DC1=\"12\" D00=\"TEST12\"/>";

    const std::string expectedMSG_ITT = "<MSG N06=\"X\" Q0K=\"0\" S18=\"BAGGAGE IS NOT "
                                        "ALLOWED\"/><MSG N06=\"X\" Q0K=\"0\" S18=\"//\"/>";

    const std::string expectedMSG =
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"NO CHECKED BAG ALLOWANCE FOR "
        "LAP INFANT WITHOUT SEAT\"/><MSG N06=\"X\" Q0K=\"0\" S18=\"//\"/>";

    const std::string expectedXML =
        "<BGA B20=\"33\" N0D=\"L\" BPC=\"30\"><ITR Q00=\"3\">" + expectedITT + expectedBIR +
        expectedSDC + expectedMSG_ITT + "</ITT></ITR><ITR Q00=\"4\">" + expectedITT + expectedBIR +
        expectedSDC + expectedMSG_ITT + "</ITT></ITR><ITR Q00=\"6\">" + expectedITT + expectedBIR +
        expectedSDC + expectedMSG_ITT + "</ITT></ITR>" + expectedMSG + "</BGA>";

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_convertWeightUnit_Kilo()
  {
    CPPUNIT_ASSERT_EQUAL('K', _ancFormatter->convertWeightUnit('K'));
  }

  void test_convertWeightUnit_Pound()
  {
    CPPUNIT_ASSERT_EQUAL('L', _ancFormatter->convertWeightUnit('P'));
  }

  void test_isOCSData_Allowance_S7()
  {
    std::vector<PaxOCFees> paxOCFeesVector;
    PaxOCFees* paxOcFees_1 = createPaxOCFees(false, "BG", true);
    paxOCFeesVector.push_back(*paxOcFees_1);
    PaxOCFees* paxOcFees_2 = createPaxOCFees(false, "BG", true);
    paxOCFeesVector.push_back(*paxOcFees_2);
    PaxOCFees* paxOcFees_3 = createPaxOCFees(false, "BG", true);
    paxOCFeesVector.push_back(*paxOcFees_3);

    CPPUNIT_ASSERT(_ancFormatter->isOCSData(paxOCFeesVector));
  }

  void test_isOCSData_No_Allowance_S7()
  {
    std::vector<PaxOCFees> paxOCFeesVector;
    PaxOCFees* paxOcFees_1 = createPaxOCFees(false, "BG", false);
    paxOCFeesVector.push_back(*paxOcFees_1);
    PaxOCFees* paxOcFees_2 = createPaxOCFees(false, "BG", false);
    paxOCFeesVector.push_back(*paxOcFees_2);
    PaxOCFees* paxOcFees_3 = createPaxOCFees(false, "BG", false);
    paxOCFeesVector.push_back(*paxOcFees_3);

    CPPUNIT_ASSERT(!_ancFormatter->isOCSData(paxOCFeesVector));
  }

  void test_buildDisclosureText_Found()
  {
    static const char fp0_baggageResponse[] = "Dummy baggageResponse text from FarePath 0";
    static const char fp0_baggageEmbargoesResponse[] =
        "Dummy baggageEmbargoesResponse from FarePath0";
    static const char fp1_baggageEmbargoesResponse[] =
        "Dummy baggageEmbargoesResponse from FarePath1. "
        "This is a very long string just to test the splitting. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisqu"
        "e suscipit id erat sit amet suscipit. Sed sollicitudin odio eu "
        "nisl eleifend semper. Morbi efficitur mattis sem, volutpat frin"
        "gilla felis lacinia vitae. Nullam sit amet urna pretium massa n"
        "unc.";
    static const char expectedOutput_Found[] =
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"ADT-02\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"Dummy baggageResponse text from FarePath 0\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"Dummy baggageEmbargoesResponse from FarePath0\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"ADT-01\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"Dummy baggageEmbargoesResponse from FarePath1. This is a "
        "very l\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"ong string just to test the splitting. Lorem ipsum dolor "
        "sit am\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"et, consectetur adipiscing elit. Quisque suscipit id erat "
        "sit a\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"met suscipit. Sed sollicitudin odio eu nisl eleifend "
        "semper. Mo\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"rbi efficitur mattis sem, volutpat fringilla felis lacinia "
        "vita\"/>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"e. Nullam sit amet urna pretium massa nunc.\"/>";

    std::unique_ptr<FarePath> fp0(new FarePath());
    std::unique_ptr<PaxType> paxType0(new PaxType());
    paxType0->paxType() = "ADT";
    paxType0->number() = 2;
    fp0->paxType() = paxType0.get();
    fp0->baggageResponse() = fp0_baggageResponse;
    fp0->baggageEmbargoesResponse() = fp0_baggageEmbargoesResponse;

    std::unique_ptr<FarePath> fp1(new FarePath());
    std::unique_ptr<PaxType> paxType1(new PaxType());
    paxType1->paxType() = "ADT";
    paxType1->number() = 1;
    fp1->paxType() = paxType1.get();
    fp1->baggageEmbargoesResponse() = fp1_baggageEmbargoesResponse;

    _itin->farePath().push_back(fp0.get());
    _itin->farePath().push_back(fp1.get());

    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    srvFeesGroup->state() = ServiceFeesGroup::VALID;
    _itin->ocFeesGroupsFreeBag().push_back(srvFeesGroup);

    _ancFormatter->buildDisclosureText(*_ancTrx, _itin, *_construct);

    CPPUNIT_ASSERT_EQUAL(std::string(expectedOutput_Found), _construct->getXMLData());
  }

  void test_buildDisclosureText_NotFound()
  {
    static const char expectedOutput_NotFound[] =
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"DISCLOSURE TEXT NOT FOUND\"/>";

    _ancFormatter->buildDisclosureText(*_ancTrx, _itin, *_construct);

    CPPUNIT_ASSERT_EQUAL(std::string(expectedOutput_NotFound), _construct->getXMLData());
  }

  PaxOCFeesUsages* createPaxOCFeesUsages(bool displayOnly = false, ServiceGroup serviceGroup = "BG")
  {
    OCFees* ocFees = createOCFeesOneTravelSeg(displayOnly, serviceGroup);
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("AB", "", "", "", "", "**", 1));
    ocFees->padisData() = padisData;
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFees->farePath() = fp;
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(ocFees->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 771;
    OCFeesUsage* ocFeesUsage = _memH.create<OCFeesUsage>();
    ocFeesUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeesUsage);
    FPOCFeesUsages* fpOcFeesUsages =
        _memH.insert(new FPOCFeesUsages(createFarePath(), ocFeesUsage));
    PaxOCFeesUsages* paxOcFeesUsages = _memH.insert(new PaxOCFeesUsages(*fpOcFeesUsages));
    return paxOcFeesUsages;
  }

  void test_buildS7Oos_AB240_Padis_1()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFees& oc = const_cast<OCFees&>(*(paxFee->fees()));
    OptionalServicesInfo* optI = const_cast<OptionalServicesInfo*>(oc.optFee());
    optI->ticketEffDate() = DateTime(2011, 2, 1);
    optI->ticketDiscDate() = DateTime(2011, 10, 1);
    optI->advPurchPeriod() = "10";
    oc.purchaseByDate() = DateTime(2011, 3, 1);
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 771;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("AB", "", "", "", "", "**", 1));
    ocFeeUsage->padisData() = padisData;
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancRequest->hardMatchIndicator() = true;
    RequestedOcFeeGroup requestedOcFeeGroupSA;
    requestedOcFeeGroupSA.groupCode() = "SA";
    requestedOcFeeGroupSA.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);

    _pOptions->serviceGroupsVec().push_back(requestedOcFeeGroupSA);

    _ancFormatterMock->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);
    const std::string expectedXML("<OOS SEQ=\"1234\" URT=\"771\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<DTE D01=\"2011-02-01\" D02=\"2011-10-01\" D03=\"2011-03-01\"/>"
                                  "<FAT/>"
                                  "<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>"
                                  "</PSP>"
                                  "</OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7Oos_NoAB240_NoPadis()
  {
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 771;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("AB", "", "", "", "", "**", 1));
    ocFeeUsage->padisData() = padisData;
    static_cast<AncRequest*>(_ancTrx->getRequest())->ancRequestType() = AncRequest::WPAERequest;
    RequestedOcFeeGroup requestedOcFeeGroupBG;
    requestedOcFeeGroupBG.groupCode() = "SA";
    requestedOcFeeGroupBG.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);

    _pOptions->serviceGroupsVec().push_back(requestedOcFeeGroupBG);

    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);
    const std::string expectedXML("<OOS SEQ=\"1234\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/>"
                                  "</OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7Oos_AB240_Padis_2()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    GroupFeesUsagesVector groupFeesVector;

    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancRequest->hardMatchIndicator() = true;
    RequestedOcFeeGroup requestedOcFeeGroupBG;
    requestedOcFeeGroupBG.groupCode() = "SA";
    requestedOcFeeGroupBG.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);
    _pOptions->serviceGroupsVec().push_back(requestedOcFeeGroupBG);

    srvFeesGroup.groupCode() = "SA";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(false, srvFeesGroup.groupCode())));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));
    _ancTrx->billing()->requestPath() = ANCS_PO_ATSE_PATH;

    _ancFormatterMock->buildS7OOSData(*_ancTrx, *_construct, paxOcFeesUsages[0]);
    const std::string expectedXML("<OOS SEQ=\"1234\" URT=\"771\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\" D03=\"2011-03-01\"/>"
                                  "<FAT/>"
                                  "<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>"
                                  "</PSP>"
                                  "</OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildS7Oos_ACS_NoPadis()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    GroupFeesUsagesVector groupFeesVector;

    RequestedOcFeeGroup requestedOcFeeGroupBG;
    requestedOcFeeGroupBG.groupCode() = "SA";
    requestedOcFeeGroupBG.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);
    _pOptions->serviceGroupsVec().push_back(requestedOcFeeGroupBG);

    srvFeesGroup.groupCode() = "SA";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(false, srvFeesGroup.groupCode())));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));
    _ancTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;

    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, paxOcFeesUsages[0]);
    const std::string expectedXML("<OOS SEQ=\"1234\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/>"
                                  "</OOS>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Flight_Related_Service_M70()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = false;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ true);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->fltTktMerchInd() =
        FLIGHT_RELATED_SERVICE;
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Flight_Related_Service_WPAE()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = true;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ true);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->fltTktMerchInd() =
        FLIGHT_RELATED_SERVICE;
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML =
        "<OSC SFD=\"Y\" N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Flight_Related_Service_NonDisplayOnly_M70()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = false;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ false);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->fltTktMerchInd() =
        FLIGHT_RELATED_SERVICE;
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, 1);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Flight_Related_Service_NonDisplayOnly_WPAE()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = true;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ false);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->fltTktMerchInd() =
        FLIGHT_RELATED_SERVICE;
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, 1);
    _construct->closeElement(); // OSC close
    const std::string expectedXML =
        "<OSC SFD=\"N\" N02=\" \" ASD=\"SPORTING EQUIPMENT\" AST=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_M70()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = false;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ true);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\">"
                                    "<BIR B20=\"3.4\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22.5\" N0D=\"L\" L01=\"U\"/>"
                                    "<BIR B20=\"10\" N0D=\"L\" L01=\"O\"/>"
                                    "<BIR B20=\"3\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22\" N0D=\"C\" L01=\"U\"/>"
                                    "<BIR B20=\"5\" N0D=\"I\" L01=\"O\"/>"
                                    "<SDC DC1=\"10\" D00=\"TEST10\"/>"
                                    "<SDC DC1=\"12\" D00=\"TEST12\"/></OSC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_WPAE()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = true;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ true);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC SFD=\"Y\" N02=\" \" ASD=\"SPORTING EQUIPMENT\">"
                                    "<BIR B20=\"3.4\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22.5\" N0D=\"L\" L01=\"U\"/>"
                                    "<BIR B20=\"10\" N0D=\"L\" L01=\"O\"/>"
                                    "<BIR B20=\"3\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22\" N0D=\"C\" L01=\"U\"/>"
                                    "<BIR B20=\"5\" N0D=\"I\" L01=\"O\"/>"
                                    "<SDC DC1=\"10\" D00=\"TEST10\"/>"
                                    "<SDC DC1=\"12\" D00=\"TEST12\"/></OSC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_NonDisplayOnly_M70()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = false;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ false);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, 2);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC N02=\" \" ASD=\"SPORTING EQUIPMENT\">"
                                    "<BIR B20=\"3.4\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22.5\" N0D=\"L\" L01=\"U\"/>"
                                    "<BIR B20=\"10\" N0D=\"L\" L01=\"O\"/>"
                                    "<BIR B20=\"3\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22\" N0D=\"C\" L01=\"U\"/>"
                                    "<BIR B20=\"5\" N0D=\"I\" L01=\"O\"/>"
                                    "<SDC DC1=\"10\" D00=\"TEST10\"/>"
                                    "<SDC DC1=\"12\" D00=\"TEST12\"/></OSC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOSCOptionalData_AB240_Non_Flight_Related_Service_NonDisplayOnly_WPAE()
  {
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancFormatter->_isWPDispAE = true;
    _ancTrx->getRequest()->majorSchemaVersion() = 3;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg(/* displayOnly= */ false);
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description1() = "10";
    (const_cast<SubCodeInfo*>(paxFee->fees()->subCodeInfo()))->description2() = "12";
    _construct->openElement(xml2::OCS5Info); // OSC Open
    _ancFormatter->buildOSCOptionalData(*_ancTrx, *_construct, *paxFee, 2);
    _construct->closeElement(); // OSC close
    const std::string expectedXML = "<OSC SFD=\"N\" N02=\" \" ASD=\"SPORTING EQUIPMENT\">"
                                    "<BIR B20=\"3.4\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22.5\" N0D=\"L\" L01=\"U\"/>"
                                    "<BIR B20=\"10\" N0D=\"L\" L01=\"O\"/>"
                                    "<BIR B20=\"3\" N0D=\"K\" L01=\"O\"/>"
                                    "<BIR B20=\"22\" N0D=\"C\" L01=\"U\"/>"
                                    "<BIR B20=\"5\" N0D=\"I\" L01=\"O\"/>"
                                    "<SDC DC1=\"10\" D00=\"TEST10\"/>"
                                    "<SDC DC1=\"12\" D00=\"TEST12\"/></OSC>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_AM_Apply_Charges_Tax()
  {
    std::string amChargesTaxesData = "MX/IVA/16|BO/BOA/13";
    TestConfigInitializer::setValue("AM_CHARGES_TAXES", amChargesTaxesData, "SERVICE_FEES_SVC");
    _ancTrx->modifiableActivationFlags().setAB240(true);
    _ancRequest->hardMatchIndicator() = true;
    _ancTrx->loadAmVatTaxRatesOnCharges();
    _ancTrx->billing()->partitionID() = CARRIER_AM;
    _ancFormatterMock->_nation = MEXICO;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    subCodeInfo->fltTktMerchInd() = BAGGAGE_CHARGE;
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 771;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("AB", "", "", "", "", "**", 1));
    ocFeeUsage->padisData() = padisData;
    RequestedOcFeeGroup requestedOcFeeGroupSA;
    requestedOcFeeGroupSA.groupCode() = "SA";
    requestedOcFeeGroupSA.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);

    _pOptions->serviceGroupsVec().push_back(requestedOcFeeGroupSA);

    _ancFormatterMock->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);
    const std::string expectedXML("<OOS SEQ=\"1234\" URT=\"771\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\" D03=\"2011-03-01\"/>"
                                  "<FAT/>"
                                  "<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>"
                                  "</PSP>"
                                  "<TAX BC0=\"IVA\" C6B=\"0.00\"/>"
                                  "<BGD OC1=\"-1\" OC2=\"-1\"/>"
                                  "</OOS>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_AM_Do_Not_Apply_Charges_Tax()
  {
    _ancTrx->billing()->partitionID() = SPECIAL_CARRIER_AA;
    _ancFormatter->_nation = MEXICO;
    PaxOCFees* paxFee = createPaxOCFeesOneTravelSeg();
    OCFeesUsage* ocFeeUsage = paxFee->fees()->ocfeeUsage()[0];
    FarePath* fp = createFarePath();
    fp->itin() = _itin;
    ocFeeUsage->farePath() = fp;
    SubCodeInfo* subCodeInfo = const_cast<SubCodeInfo*>(ocFeeUsage->subCodeInfo());
    subCodeInfo->serviceGroup() = "SA";
    subCodeInfo->fltTktMerchInd() = BAGGAGE_CHARGE;
    OptionalServicesInfo* osi = const_cast<OptionalServicesInfo*>(paxFee->fees()->optFee());
    osi->seqNo() = 1234;
    osi->upgrdServiceFeesResBkgDesigTblItemNo() = 771;
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    padisData.push_back(createPadis("AB", "", "", "", "", "**", 1));
    ocFeeUsage->padisData() = padisData;
    RequestedOcFeeGroup requestedOcFeeGroupSA;
    requestedOcFeeGroupSA.groupCode() = "SA";
    requestedOcFeeGroupSA.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);

    _pOptions->serviceGroupsVec().push_back(requestedOcFeeGroupSA);

    _ancFormatter->buildS7OOSData(*_ancTrx, *_construct, *paxFee, *ocFeeUsage);
    const std::string expectedXML("<OOS SEQ=\"1234\" URT=\"771\">"
                                  "<Q00>01</Q00>"
                                  "<SUM B70=\"ADT\" C51=\"0.00\" C52=\"0.00\" C5B=\"USD\" C50=\"0.00\"/>"
                                  "<SFQ/>"
                                  "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/>"
                                  "<FAT/>"
                                  "<PSP>"
                                  "<UPC SEU=\"1\">"
                                  "<PDS PDI=\"AB\" PCA=\"BAR\" SED=\"ADJACENT TO BAR\"/>"
                                  "</UPC>"
                                  "</PSP>"
                                  "<BGD OC1=\"-1\" OC2=\"-1\"/>"
                                  "</OOS>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildOperatingCarrierInfo()
  {
    AirSeg* airSeg = _memH.insert(new AirSeg);
    airSeg->setOperatingCarrierCode("AB");
    airSeg->operatingFlightNumber() = 123;
    airSeg->bookedCabin().setEconomyClass();
    _itin->travelSeg().push_back(airSeg);

    airSeg = _memH.insert(new AirSeg);
    airSeg->setOperatingCarrierCode("LH");
    airSeg->operatingFlightNumber() = 666;
    airSeg->bookedCabin().setBusinessClass();
    _itin->travelSeg().push_back(airSeg);

    _ancFormatter->buildOperatingCarrierInfo(*_ancTrx, _itin, *_construct);

    const std::string expectedXML("<SEG Q00=\"1\" B01=\"AB\" Q0B=\"123\" N00=\"Y\"/>"
                                  "<SEG Q00=\"2\" B01=\"LH\" Q0B=\"666\" N00=\"C\"/>");

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }
};

const std::string
AncillaryPricingResponseFormatterTest::XMLNS("http://www.atse.sabre.com"
                                             "/AncillaryPricing/Response");

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPricingResponseFormatterTest);

} // tse
