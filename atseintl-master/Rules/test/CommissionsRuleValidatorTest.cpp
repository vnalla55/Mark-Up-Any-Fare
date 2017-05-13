#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/CommissionsRuleValidator.h"
#include "Rules/RuleUtil.cpp"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <vector>

//#define PERFORMANCE_TEST
#ifdef PERFORMANCE_TEST
#define PERFORMANCE_ITERATIONS 10000
#endif

namespace tse
{
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

namespace
{
class CommissionsRuleValidatorMock : public CommissionsRuleValidator
{
public:
  CommissionsRuleValidatorMock(PricingTrx& trx, FarePath& fp, FareUsage& fu, Diag867Collector* diag)
    : CommissionsRuleValidator(trx, fp, fu, diag) {}

  bool isInLoc(const LocCode& validatingLoc, LocTypeCode ruleLocType,
           const LocCode& ruleLoc, const PaxTypeFare& paxTypeFare) const
  {
    return validatingLoc == ruleLoc;
  }

  void getClassOfService()
  {
    BookingCode bkc = "A ";
    _rbd = bkc;
    return;
  }
  void getFareBasis()
  {
    std::string fareBasis = "FOCUS";
    _fareBasis = fareBasis;
    return;
  }
  void getPaxTypes()
  {
    PaxTypeCode pax = "ADT";
    _truePaxType = pax;
    return;
  }
};
}

class CommissionsRuleValidatorTest: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CommissionsRuleValidatorTest);
  CPPUNIT_TEST(testMatchPaxType_Required_Empty_Pass);
  CPPUNIT_TEST(testMatchPaxType_Required_NotEmpty_NotMatch_Fail);
  CPPUNIT_TEST(testMatchPaxType_Required_NotEmpty_Match_Pass);

  CPPUNIT_TEST(testMatchFareBasis_Excl_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchFareBasis_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchFareBasis_Excl_NoMatch_InclEmpty_Pass);
  CPPUNIT_TEST(testMatchFareBasis_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchFareBasis_Excl_Empty_Incl_No_Match_Fail);

  CPPUNIT_TEST(testMatchClassOfService_Excl_Empty_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchClassOfService_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchClassOfService_Excl_NoMatch_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchClassOfService_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchClassOfService_Excl_Empty_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchClassOfService_Excl_NoMatch_Incl_No_Match_Fail);
  CPPUNIT_TEST(testMatchClassOfService_Excl_NoMatch_Incl_Match_Pass);

  CPPUNIT_TEST(testMatchCabin_Excl_Empty_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchCabin_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchCabin_Excl_NoMatch_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchCabin_Excl_Empty_Req_Match_Pass);
  CPPUNIT_TEST(testMatchCabin_Excl_Empty_Req_NoMatch_Fail);
  CPPUNIT_TEST(testMatchCabin_Excl_NoMatch_Req_NoMatch_Fail);
  CPPUNIT_TEST(testMatchCabin_Excl_NoMatch_Req_Match_Pass);

  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_Empty_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_NoMatch_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_Empty_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_NoMatch_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchOperatingCarrier_Excl_NoMatch_Incl_Match_Pass);

  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_Empty_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_NoMatch_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_Empty_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_NoMatch_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchMarketingCarrier_Excl_NoMatch_Incl_Match_Pass);

  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_Empty_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_NoMatch_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_Empty_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_NoMatch_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchTicketingCarrier_Excl_NoMatch_Incl_Match_Pass);

  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_Empty_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_NoMatch_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_Empty_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_NoMatch_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchGovMarketingCarrier_Excl_NoMatch_Incl_Match_Pass);

  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_Empty_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_NoMatch_Incl_Empty_Pass);
  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_Empty_Incl_Match_Pass);
  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_Empty_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_NoMatch_Incl_NoMatch_Fail);
  CPPUNIT_TEST(testMatchGovOperatingCarrier_Excl_NoMatch_Incl_Match_Pass);

  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_Empty_TktDesig_Empty_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_Empty_TktDesig_NotEmpty_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_Empty_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_NotMatch_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_Match_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_Match_WildCard_Pass);

  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_Empty_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_Empty_Req_NotEmpty_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_Match_Req_Empty_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_Req_NotEmpty_NotMatch_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_ANY_TktDesig_NotEmpty_Req_NotEmpty_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_Req_NotEmpty_Match_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_Req_ANY_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Req_NotEmpty_NotMatch_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_Match_WildCard_Req_Empty_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Req_NotEmpty_NotMatch_WildCard_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Req_NotEmpty_Match_WildCard_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Percentage_TktDesig_Empty_Pass);
  CPPUNIT_TEST(testMatchTktDesignator_Excl_Percentage_TktDesig_NotEmpty__Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Req_Percentage_TktDesig_Empty_Fail);
  CPPUNIT_TEST(testMatchTktDesignator_Req_Percentage_TktDesig_NotEmpty_Pass);

  CPPUNIT_TEST(testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_Skip_Req_NotEmpty_ANY_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_ANY_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_One_TvlSeg_Excl_Empty_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_One_TvlSeg_Excl_Empty_Req_NotEmpty_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_Req_NotEmpty_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_Empty_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_ANY_Req_Empty_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_Match_Req_Empty_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_HasOne_TwoCONX_OneMatch_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_NotEmpty_PartiallyMatch_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_NotEmpty_NotMatch_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_NotEmpty_ANY_Match_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_Three_TvlSegs_Excl_Empty_Req_ANY_Pass);
  CPPUNIT_TEST(testMatchConnectionAirports_Three_TvlSegs_Excl_NotEmpty_Match_Req_ANY_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Three_TvlSegs_Excl_NotEmpty_NotMatch_ReqOne_Partially_Match_Fail);
  CPPUNIT_TEST(testMatchConnectionAirports_Three_TvlSegs_Excl_NotEmpty_NotMatch_Req_NotEmpty_All_Match_Pass);

  CPPUNIT_TEST(testMatchRoundTrip_Empty_Pass);
  CPPUNIT_TEST(testMatchRoundTrip_X_Itin_Not_RoundTrip_Pass);
  CPPUNIT_TEST(testMatchRoundTrip_X_Itin_Set_RoundTrip_Fail);
  CPPUNIT_TEST(testMatchRoundTrip_R_Itin_Not_Set_RoundTrip_Fail);
  CPPUNIT_TEST(testMatchRoundTrip_R_Itin_Set_RoundTrip_Pass);

  CPPUNIT_TEST(testMatchRequiredNonStop_Empty_Pass);
  CPPUNIT_TEST(testMatchRequiredNonStop_NotEmpty_AirPorts_Match_Pass);
  CPPUNIT_TEST(testMatchRequiredNonStop_NotEmpty_AirPorts_NoMatch_Fail);
  CPPUNIT_TEST(testMatchRequiredNonStop_NotEmpty_AirPortsOffCity_Match_Pass);
  CPPUNIT_TEST(testMatchRequiredNonStop_NotEmpty_BoardCityAirPorts_Match_Pass);
  CPPUNIT_TEST(testMatchRequiredNonStop_NotEmpty_OriginDestination_Match_Pass);

  CPPUNIT_TEST(testMatchInterlineConnection_Empty_Pass);
  CPPUNIT_TEST(testMatchInterlineConnection_N_Pass);
  CPPUNIT_TEST(testMatchInterlineConnection_Y_One_TravelSeg_Fail);
  CPPUNIT_TEST(testMatchInterlineConnection_Y_Two_TravelSegs_DifferentMarketing_Pass);
  CPPUNIT_TEST(testMatchInterlineConnection_Y_Two_TravelSegs_SameMarketing_Fail);

  CPPUNIT_TEST(testMatchFareBasisFragment);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_Empty_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_Match_Fail);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_NoMatch_Req_Empty_Pass);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_Empty_Req_Match_Pass);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_Empty_Req_NoMatch_Fail);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_NoMatch_Req_NoMatch_Fail);
  CPPUNIT_TEST(testMatchFareBasisFragment_Excl_NoMatch_Req_Match_Pass);

  CPPUNIT_TEST(testValidateCommissionRule_PASS);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_FareBasis_Excl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_FareBasis_Incl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_ClassOfService_Excl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_ClassOfService_Incl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_FBC_Fragment_Excl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_FBC_Fragment_Incl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_Cabin_Excl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_Cabin_Incl);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_TICKET_CARRIER_EXCL);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_TICKET_CARRIER_INCL);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_OPER_CARRIER_EXCL);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_OPER_CARRIER_INCL);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_MARKET_CARRIER_EXCL);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_MARKET_CARRIER_INCL);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_EXCL_MKT_GOV_CXR);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_REQ_MKT_GOV_CXR);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_EXCL_OPER_GOV_CXR);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_REQ_OPER_GOV_CXR);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_PSGR_TYPE);
// Deactivated for MVP phase 1, re-activate for phase 2
// Due to the requirement is not correct, remove this logic for AMC Phase2
//  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_EXCL_TKT_DESIGNATOR);
//  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_REQ_TKT_DESIGNATOR);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_ROUNDTRIP);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_REQ_NON_STOP);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_EXCL_CONN_AIRPORT);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_REQ_CONN_AIRPORT);
  CPPUNIT_TEST(testValidateCommissionRule_FAIL_CR_INTERLINE_CONNECTION);
  CPPUNIT_TEST_SUITE_END();


  void testMatchPaxType_Required_Empty_Pass()
  {
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchPassengerType(cri));
  }

  void testMatchPaxType_Required_NotEmpty_NotMatch_Fail()
  {
    PaxTypeCode pax = "PXX";
    _cri->requiredPaxType().push_back(pax);
    CPPUNIT_ASSERT(!_crv->matchPassengerType(*_cri));
  }

  void testMatchPaxType_Required_NotEmpty_Match_Pass()
  {
    PaxTypeCode pax = "ADT";
    _cri->requiredPaxType().push_back(pax);
    _crv->getPaxTypes();
    CPPUNIT_ASSERT(_crv->matchPassengerType(*_cri));
  }

  void testMatchFareBasis_Excl_Incl_Empty_Pass()
  {
    CommissionRuleInfo cri;
    CommissionValidationStatus rc = PASS_CR;
    CPPUNIT_ASSERT(_crv->matchFareBasis(cri, rc));
  }

  void testMatchFareBasis_Excl_Match_Fail()
  {
    char fbc = 'B';
    char fbc1 = 'F';
    _cri->fareBasisCodeExcl().push_back(fbc);
    _cri->fareBasisCodeExcl().push_back(fbc1);
    _crv->getFareBasis();
    CommissionValidationStatus rc = PASS_CR;
    CPPUNIT_ASSERT(!_crv->matchFareBasis(*_cri, rc));
  }

  void testMatchFareBasis_Excl_NoMatch_InclEmpty_Pass()
  {
    char fbc = 'F';
    _cri->fareBasisCodeExcl().push_back(fbc);
    _crv->_fareBasis = "YRT";
    CommissionValidationStatus rc = PASS_CR;
    CPPUNIT_ASSERT(_crv->matchFareBasis(*_cri, rc));
  }

  void testMatchFareBasis_Excl_Empty_Incl_Match_Pass()
  {
    char fbc = 'B';
    char fbc1 = 'F';
    _cri->fareBasisCodeIncl().push_back(fbc);
    _cri->fareBasisCodeIncl().push_back(fbc1);
    _crv->getFareBasis();
    CommissionValidationStatus rc = PASS_CR;
    CPPUNIT_ASSERT(_crv->matchFareBasis(*_cri, rc));
  }

  void testMatchFareBasis_Excl_Empty_Incl_No_Match_Fail()
  {
    char fbc = 'F';
    _cri->fareBasisCodeIncl().push_back(fbc);
    _crv->_fareBasis = "YRT";
    CommissionValidationStatus rc = PASS_CR;
    CPPUNIT_ASSERT(!_crv->matchFareBasis(*_cri, rc));
  }

  void testMatchClassOfService_Excl_Empty_Incl_Empty_Pass()
  {
    CommissionRuleInfo cri;
    CommissionValidationStatus rc = PASS_CR;
    CPPUNIT_ASSERT(_crv->matchClassOfService(cri, rc));
  }

  void testMatchClassOfService_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    BookingCode rbd1 = "B ";
    BookingCode rbd2 = "A ";
    _cri->classOfServiceExcl().push_back(rbd1);
    _cri->classOfServiceExcl().push_back(rbd2);
    _crv->getClassOfService();
    CPPUNIT_ASSERT(!_crv->matchClassOfService(*_cri, rc));
  }

  void testMatchClassOfService_Excl_NoMatch_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    BookingCode rbd1 = "F ";
    _cri->classOfServiceExcl().push_back(rbd1);
    _crv->getClassOfService();
    CPPUNIT_ASSERT(_crv->matchClassOfService(*_cri, rc));
  }

  void testMatchClassOfService_Excl_Empty_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    BookingCode rbd1 = "B ";
    BookingCode rbd2 = "A ";
    _cri->classOfServiceIncl().push_back(rbd1);
    _cri->classOfServiceIncl().push_back(rbd2);
    _crv->getClassOfService();
    CPPUNIT_ASSERT(_crv->matchClassOfService(*_cri, rc));
  }

  void testMatchClassOfService_Excl_Empty_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    BookingCode rbd1 = "F ";
    _cri->classOfServiceIncl().push_back(rbd1);
    _crv->getClassOfService();
    CPPUNIT_ASSERT(!_crv->matchClassOfService(*_cri, rc));
  }

  void testMatchClassOfService_Excl_NoMatch_Incl_No_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    BookingCode rbd1 = "F ";
    _cri->classOfServiceExcl().push_back(rbd1);
    _cri->classOfServiceIncl().push_back(rbd1);
    _crv->getClassOfService();
    CPPUNIT_ASSERT(!_crv->matchClassOfService(*_cri, rc));
  }

  void testMatchClassOfService_Excl_NoMatch_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    BookingCode rbd1 = "B ";
    BookingCode rbd2 = "A ";
    _cri->classOfServiceExcl().push_back(rbd1);
    _cri->classOfServiceIncl().push_back(rbd1);
    _cri->classOfServiceIncl().push_back(rbd2);
    _crv->getClassOfService();
    CPPUNIT_ASSERT(_crv->matchClassOfService(*_cri, rc));
  }

  void testMatchCabin_Excl_Empty_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchCabin(cri, rc));
  }

  void testMatchCabin_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    char cab1 = 'F';
    char cab2 = 'R';
    _cri->excludedCabinType().push_back(cab1);
    _cri->excludedCabinType().push_back(cab2);
    CPPUNIT_ASSERT(!_crv->matchCabin(*_cri, rc));
  }

  void testMatchCabin_Excl_NoMatch_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    char cab1 = 'F';
    _cri->excludedCabinType().push_back(cab1);
    CPPUNIT_ASSERT(_crv->matchCabin(*_cri, rc));
  }

  void testMatchCabin_Excl_Empty_Req_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    char cab1 = 'R';
    _cri->requiredCabinType().push_back(cab1);
    CPPUNIT_ASSERT(_crv->matchCabin(*_cri, rc));
  }

  void testMatchCabin_Excl_Empty_Req_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    char cab1 = 'F';
    _cri->requiredCabinType().push_back(cab1);
    CPPUNIT_ASSERT(!_crv->matchCabin(*_cri, rc));
  }

  void testMatchCabin_Excl_NoMatch_Req_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    char cab1 = 'F';
    _cri->excludedCabinType().push_back(cab1);
    _cri->requiredCabinType().push_back(cab1);
    CPPUNIT_ASSERT(!_crv->matchCabin(*_cri, rc));
  }

  void testMatchCabin_Excl_NoMatch_Req_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    char cab1 = 'F';
    char cab2 = 'R';
    _cri->excludedCabinType().push_back(cab1);
    _cri->requiredCabinType().push_back(cab2);
    CPPUNIT_ASSERT(_crv->matchCabin(*_cri, rc));
  }

  void testMatchOperatingCarrier_Excl_Empty_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchOperatingCarrier(cri, rc));
  }

  void testMatchOperatingCarrier_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->operatingCarrierExcl().push_back(cxr);
    CPPUNIT_ASSERT(!_crv->matchOperatingCarrier(*_cri, rc));
  }

  void testMatchOperatingCarrier_Excl_NoMatch_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BB";
    _cri->operatingCarrierExcl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchOperatingCarrier(*_cri, rc));
  }

  void testMatchOperatingCarrier_Excl_Empty_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->operatingCarrierIncl().push_back(cxr);
    _cri->operatingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchOperatingCarrier(*_cri, rc));
  }

  void testMatchOperatingCarrier_Excl_Empty_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BB";
    _cri->operatingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchOperatingCarrier(*_cri, rc));
  }

  void testMatchOperatingCarrier_Excl_NoMatch_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->operatingCarrierExcl().push_back(cxr1);
    _cri->operatingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchOperatingCarrier(*_cri, rc));
  }

  void testMatchOperatingCarrier_Excl_NoMatch_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    CarrierCode cxr2 = "BA";
    _cri->operatingCarrierExcl().push_back(cxr1);
    _cri->operatingCarrierIncl().push_back(cxr);
    _cri->operatingCarrierIncl().push_back(cxr2);
    CPPUNIT_ASSERT(_crv->matchOperatingCarrier(*_cri, rc));
  }

  void testMatchMarketingCarrier_Excl_Empty_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchMarketingCarrier(cri, rc));
  }

  void testMatchMarketingCarrier_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->marketingCarrierExcl().push_back(cxr);
    CPPUNIT_ASSERT(!_crv->matchMarketingCarrier(*_cri, rc));
  }

  void testMatchMarketingCarrier_Excl_NoMatch_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BB";
    _cri->marketingCarrierExcl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchMarketingCarrier(*_cri, rc));
  }

  void testMatchMarketingCarrier_Excl_Empty_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->marketingCarrierIncl().push_back(cxr);
    _cri->marketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchMarketingCarrier(*_cri, rc));
  }

  void testMatchMarketingCarrier_Excl_Empty_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BB";
    _cri->marketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchMarketingCarrier(*_cri, rc));
  }

  void testMatchMarketingCarrier_Excl_NoMatch_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->marketingCarrierExcl().push_back(cxr1);
    _cri->marketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchMarketingCarrier(*_cri, rc));
  }

  void testMatchMarketingCarrier_Excl_NoMatch_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    CarrierCode cxr2 = "BA";
    _cri->marketingCarrierExcl().push_back(cxr1);
    _cri->marketingCarrierIncl().push_back(cxr);
    _cri->marketingCarrierIncl().push_back(cxr2);
    CPPUNIT_ASSERT(_crv->matchMarketingCarrier(*_cri, rc));
  }

  void testMatchTicketingCarrier_Excl_Empty_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchTicketingCarrier(cri, rc, cxr));
  }

  void testMatchTicketingCarrier_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->ticketingCarrierExcl().push_back(cxr);
    CPPUNIT_ASSERT(!_crv->matchTicketingCarrier(*_cri, rc, cxr));
  }

  void testMatchTicketingCarrier_Excl_NoMatch_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->ticketingCarrierExcl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchTicketingCarrier(*_cri, rc, cxr));
  }

  void testMatchTicketingCarrier_Excl_Empty_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->ticketingCarrierIncl().push_back(cxr);
    CPPUNIT_ASSERT(_crv->matchTicketingCarrier(*_cri, rc, cxr));
  }

  void testMatchTicketingCarrier_Excl_Empty_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->ticketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchTicketingCarrier(*_cri, rc, cxr));
  }

  void testMatchTicketingCarrier_Excl_NoMatch_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->ticketingCarrierExcl().push_back(cxr1);
    _cri->ticketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchTicketingCarrier(*_cri, rc, cxr));
  }

  void testMatchTicketingCarrier_Excl_NoMatch_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->ticketingCarrierExcl().push_back(cxr1);
    _cri->ticketingCarrierIncl().push_back(cxr);
    CPPUNIT_ASSERT(_crv->matchTicketingCarrier(*_cri, rc, cxr));
  }

  void testMatchGovMarketingCarrier_Excl_Empty_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchMarketingGovCarrier(cri, rc));
  }

  void testMatchGovMarketingCarrier_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->excludedMktGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(!_crv->matchMarketingGovCarrier(*_cri, rc));
  }

  void testMatchGovMarketingCarrier_Excl_NoMatch_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->excludedMktGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchMarketingGovCarrier(*_cri, rc));
  }

  void testMatchGovMarketingCarrier_Excl_Empty_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->requiredMktGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(_crv->matchMarketingGovCarrier(*_cri, rc));
  }

  void testMatchGovMarketingCarrier_Excl_Empty_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->requiredMktGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchMarketingGovCarrier(*_cri, rc));
  }

  void testMatchGovMarketingCarrier_Excl_NoMatch_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->excludedMktGovCxr().push_back(cxr1);
    _cri->requiredMktGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchMarketingGovCarrier(*_cri, rc));
  }

  void testMatchGovMarketingCarrier_Excl_NoMatch_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->excludedMktGovCxr().push_back(cxr1);
    _cri->requiredMktGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(_crv->matchMarketingGovCarrier(*_cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_Empty_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchOperGovCarrier(cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->excludedOperGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(!_crv->matchOperGovCarrier(*_cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_NoMatch_Incl_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->excludedOperGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->matchOperGovCarrier(*_cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_Empty_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    _cri->requiredOperGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(_crv->matchOperGovCarrier(*_cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_Empty_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->requiredOperGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchOperGovCarrier(*_cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_NoMatch_Incl_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr1 = "BA";
    _cri->excludedOperGovCxr().push_back(cxr1);
    _cri->requiredOperGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(!_crv->matchOperGovCarrier(*_cri, rc));
  }

  void testMatchGovOperatingCarrier_Excl_NoMatch_Incl_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->excludedOperGovCxr().push_back(cxr1);
    _cri->requiredOperGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(_crv->matchOperGovCarrier(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_Empty_Req_Empty_TktDesig_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchTktDesignator(cri, rc));
  }

  void testMatchTktDesignator_Excl_Empty_Req_Empty_TktDesig_NotEmpty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    _crv->_tktDesignator = "TKT";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(cri, rc));
  }

  void testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_Empty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "ADD";
    _cri->requiredTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_NotMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "ADD";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TKT";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TKT";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_Empty_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    _cri->excludedTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_Empty_Req_NotEmpty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "ADD";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_Match_Req_Empty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TKT";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_Req_NotEmpty_NotMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    TktDesignator tktDesing1 = "TKTK";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "ADD";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_ANY_TktDesig_NotEmpty_Req_NotEmpty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "*ANY*";
    TktDesignator tktDesing1 = "TKTK";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "ADD";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_Req_NotEmpty_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKT";
    TktDesignator tktDesing1 = "ADD";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "ADD";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Req_NotEmpty_NotMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "TKTK";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "ADD";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_NotEmpty_Req_ANY_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "*ANY*";
    _cri->requiredTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_Match_WildCard_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "Y%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "1234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y1234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "1T345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "12T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "123T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "1234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "12345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "123456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "1234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "T2345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%78%P";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678P";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOP";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OP";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%78%P%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678POS";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOP";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOPU";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OP";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78%P%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678POS";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOP";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOPU";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OP";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "PYT345678OPO";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();
  }

  void testMatchTktDesignator_Excl_Empty_Req_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "Y%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "AY";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "123456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "YT1";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "12345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%78%P";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%78%P%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_Match_WildCard_Req_Empty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "Y%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%T";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "1234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y1234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "1T345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "12T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "123T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "1234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "12345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "123456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "1234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "T2345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));;
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%78";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%78%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%78%P";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678P";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOP";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OP";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%78%P%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678POS";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOP";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOPU";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OP";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%78%P%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678POS";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOP";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678QOPU";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OP";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "PYT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "Y%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "AY";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%T";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "123456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "YT1";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "12345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%78";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%78%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%78%P";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%78%P%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Req_NotEmpty_NotMatch_WildCard_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "Y%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "AY";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "123456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "YT1";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "12345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%78%P";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%78%P%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();
  }

  void testMatchTktDesignator_Excl_NotEmpty_TktDesig_NotEmpty_NotMatch_WildCard_Req_NotEmpty_Match_WildCard_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "Y%";
    TktDesignator tktDesing1 = "AY";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "AY";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T";
    tktDesing1 = "123456T8";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "123456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T";
    tktDesing1 = "YT1";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT1";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%T%";
    tktDesing1 = "12345678";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "12345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "Y%T%";
    tktDesing1 = "TY3456T8";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();

    tktDesing = "%Y%T%78";
    tktDesing1 = "Y23456T8";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "Y234567T";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "YT34567T";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "YT3456T8";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "TY3456T8";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();
    _cri->excludedTktDesig().pop_back();

    tktDesing = "%Y%T%78%";
    tktDesing1 = "Y23456T8";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "Y234567T";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "YT34567T";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "YT3456T8";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "TY3456T8";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%78%P";
    tktDesing1 = "Y2T45678";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "Y23T5678";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "Y234T678";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "Y2345T78";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "Y23456T8";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "YT345678";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT345678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();

    tktDesing1 = "YT345678OPO";
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "YT345678OPO";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->requiredTktDesig().pop_back();
    _cri->excludedTktDesig().pop_back();

    tktDesing = "Y%T%78%P%";
    tktDesing1 = "Y2T45678";
    _cri->excludedTktDesig().push_back(tktDesing);
    _cri->requiredTktDesig().push_back(tktDesing1);
    _crv->_tktDesignator = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
    _cri->excludedTktDesig().pop_back();
    _cri->requiredTktDesig().pop_back();
  }

  void testMatchTktDesignator_Excl_Percentage_TktDesig_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "%";
    _cri->excludedTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Excl_Percentage_TktDesig_NotEmpty__Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "%";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "AY";
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Req_Percentage_TktDesig_Empty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "%";
    _cri->requiredTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(!_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchTktDesignator_Req_Percentage_TktDesig_NotEmpty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    TktDesignator tktDesing = "%";
    _cri->requiredTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "AY";
    CPPUNIT_ASSERT(_crv->matchTktDesignator(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_Empty_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "MCK";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->excludedCnxAirPCodes().push_back(airp2);
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_ANY_Req_Empty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "*ANY*";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_Match_Req_Empty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "JFK";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->excludedCnxAirPCodes().push_back(airp2);
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_HasOne_TwoCONX_OneMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "JFK";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp2);
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_NotEmpty_PartiallyMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "JFK";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp2);
    _cri->requiredCnxAirPCodes().push_back(airp1);
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_NotEmpty_NotMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "MCP";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp2);
    _cri->requiredCnxAirPCodes().push_back(airp1);
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Excl_NotEmpty_NotMatch_Req_NotEmpty_ANY_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "*ANY*";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp2);
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_Skip_Req_NotEmpty_ANY_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    LocCode airp2 = "*ANY*";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp2);

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());

    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_ANY_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "*ANY*";
    _cri->excludedCnxAirPCodes().push_back(airp1);

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());

    //_crv->_fu.travelSeg().erase(_crv->_fu.travelSeg().begin());
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_One_TvlSeg_Excl_Empty_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());

    //_crv->_fu.travelSeg().erase(_crv->_fu.travelSeg().begin());
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    _cri->excludedCnxAirPCodes().push_back(airp1);

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());

    //_crv->_fu.travelSeg().erase(_crv->_fu.travelSeg().begin());
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_One_TvlSeg_Excl_Empty_Req_NotEmpty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    _cri->requiredCnxAirPCodes().push_back(airp1);

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());

    //_crv->_fu.travelSeg().erase(_crv->_fu.travelSeg().begin());
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_One_TvlSeg_Excl_NotEmpty_Req_NotEmpty_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "JFK";
    _cri->requiredCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp1);

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());

    //_crv->_fu.travelSeg().erase(_crv->_fu.travelSeg().begin());
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Three_TvlSegs_Excl_Empty_Req_ANY_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp2 = "*ANY*";
    _cri->requiredCnxAirPCodes().push_back(airp2);
    createAirSegs();
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Three_TvlSegs_Excl_NotEmpty_Match_Req_ANY_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "LHR"; // 2nd segment destination
    _cri->excludedCnxAirPCodes().push_back(airp1);
    LocCode airp2 = "*ANY*";
    _cri->requiredCnxAirPCodes().push_back(airp2);
    createAirSegs();
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Three_TvlSegs_Excl_NotEmpty_NotMatch_ReqOne_Partially_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    LocCode airp2 = "LHR";
    _cri->requiredCnxAirPCodes().push_back(airp2);
    createAirSegs();
    CPPUNIT_ASSERT(!_crv->matchConnectionAirports(*_cri, rc));
  }

  void testMatchConnectionAirports_Three_TvlSegs_Excl_NotEmpty_NotMatch_Req_NotEmpty_All_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    LocCode airp1 = "MKK";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    LocCode airp2 = "LGA";
    LocCode airp3 = "LGW";
    LocCode airp4 = "JFK";
    LocCode airp5 = "LHR";
    _cri->requiredCnxAirPCodes().push_back(airp2);
    _cri->requiredCnxAirPCodes().push_back(airp3);
    _cri->requiredCnxAirPCodes().push_back(airp4);
    _cri->requiredCnxAirPCodes().push_back(airp5);
    createAirSegs();
    CPPUNIT_ASSERT(_crv->matchConnectionAirports(*_cri, rc));
  }


  void testMatchRoundTrip_Empty_Pass()
  {
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchRoundTrip(cri));
  }

  void testMatchRoundTrip_X_Itin_Not_RoundTrip_Pass()
  {
    _cri->roundTrip() = 'X';
    CPPUNIT_ASSERT(_crv->matchRoundTrip(*_cri));
  }

  void testMatchRoundTrip_X_Itin_Set_RoundTrip_Fail()
  {
    _cri->roundTrip() = 'X';
    _airSeg1->origAirport() = "LHR";
    CPPUNIT_ASSERT(!_crv->matchRoundTrip(*_cri));
  }

  void testMatchRoundTrip_R_Itin_Not_Set_RoundTrip_Fail()
  {
    _cri->roundTrip() = 'R';
    CPPUNIT_ASSERT(!_crv->matchRoundTrip(*_cri));
  }

  void testMatchRoundTrip_R_Itin_Set_RoundTrip_Pass()
  {
    _cri->roundTrip() = 'R';
    _airSeg1->origAirport() = "LHR";
    CPPUNIT_ASSERT(_crv->matchRoundTrip(*_cri));
  }

  void testMatchRequiredNonStop_Empty_Pass()
  {
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchRequiredNonStop(cri));
  }

  void testMatchRequiredNonStop_NotEmpty_AirPorts_Match_Pass()
  {
    // DFWLGA
    _cri->requiredNonStop().emplace_back(_airSeg1->origAirport(), _airSeg1->destAirport());
    CPPUNIT_ASSERT(_crv->matchRequiredNonStop(*_cri));
  }

  void testMatchRequiredNonStop_NotEmpty_AirPorts_NoMatch_Fail()
  {
    // DFWLHR
    _cri->requiredNonStop().emplace_back(_airSeg1->origAirport(), _airSeg2->destAirport());
    CPPUNIT_ASSERT(!_crv->matchRequiredNonStop(*_cri));
  }

  void testMatchRequiredNonStop_NotEmpty_AirPortsOffCity_Match_Pass()
  {
    // DFWNYC
    _cri->requiredNonStop().emplace_back(_airSeg1->origAirport(), _airSeg1->offMultiCity());
    CPPUNIT_ASSERT(_crv->matchRequiredNonStop(*_cri));
  }

  void testMatchRequiredNonStop_NotEmpty_BoardCityAirPorts_Match_Pass()
  {
    // DFWNYC
    _cri->requiredNonStop().emplace_back(_airSeg1->boardMultiCity(), _airSeg1->destAirport());
    CPPUNIT_ASSERT(_crv->matchRequiredNonStop(*_cri));
  }

  void testMatchRequiredNonStop_NotEmpty_OriginDestination_Match_Pass()
  {
    // DFWNYC
    _cri->requiredNonStop().emplace_back(_airSeg1->origin()->loc(), _airSeg1->destination()->loc());
    CPPUNIT_ASSERT(_crv->matchRequiredNonStop(*_cri));
  }

  void testMatchInterlineConnection_Empty_Pass()
  {
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchInterlineConnection(cri));
  }

  void testMatchInterlineConnection_N_Pass()
  {
    _cri->interlineConnectionRequired() = 'N';
    CPPUNIT_ASSERT(_crv->matchInterlineConnection(*_cri));
  }

  void testMatchInterlineConnection_Y_One_TravelSeg_Fail()
  {
    _cri->interlineConnectionRequired() = 'Y';
    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().erase(fu->travelSeg().begin());
    CPPUNIT_ASSERT(!_crv->matchInterlineConnection(*_cri));
  }

  void testMatchInterlineConnection_Y_Two_TravelSegs_DifferentMarketing_Pass()
  {
    _cri->interlineConnectionRequired() = 'Y';
    CPPUNIT_ASSERT(_crv->matchInterlineConnection(*_cri));
  }

  void testMatchInterlineConnection_Y_Two_TravelSegs_SameMarketing_Fail()
  {
    _cri->interlineConnectionRequired() = 'Y';
    _airSeg2->setMarketingCarrierCode("AA");
    CPPUNIT_ASSERT(!_crv->matchInterlineConnection(*_cri));
  }


  void testMatchFareBasisFragment_Excl_Empty_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(cri, rc));
  }

  void testMatchFareBasisFragment_Excl_Match_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    std::string fragment = "%F%O%CU%S%";
    _cri->fbcFragmentExcl().push_back(fragment);
    _crv->getFareBasis();
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(*_cri, rc));
  }

  void testMatchFareBasisFragment_Excl_NoMatch_Req_Empty_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    std::string fragment = "%F%O%CO%S%";
    _cri->fbcFragmentExcl().push_back(fragment);
    _crv->getFareBasis();
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(*_cri, rc));
  }

  void testMatchFareBasisFragment_Excl_Empty_Req_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    std::string fragment = "%F%O%CU%S%";
    _cri->fbcFragmentIncl().push_back(fragment);
    _crv->getFareBasis();
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(*_cri, rc));
  }

  void testMatchFareBasisFragment_Excl_Empty_Req_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    std::string fragment = "%F%O%CO%S%";
    _cri->fbcFragmentIncl().push_back(fragment);
    _crv->getFareBasis();
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(*_cri, rc));
  }

  void testMatchFareBasisFragment_Excl_NoMatch_Req_NoMatch_Fail()
  {
    CommissionValidationStatus rc = PASS_CR;
    std::string fragment = "%F%O%CO%S%";
    _cri->fbcFragmentExcl().push_back(fragment);
    _cri->fbcFragmentIncl().push_back(fragment);
    _crv->getFareBasis();
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(*_cri, rc));
  }

  void testMatchFareBasisFragment_Excl_NoMatch_Req_Match_Pass()
  {
    CommissionValidationStatus rc = PASS_CR;
    std::string fragment = "%F%O%CO%S%";
    std::string fragment1 = "%F%O%CU%S%";
    _cri->fbcFragmentExcl().push_back(fragment);
    _cri->fbcFragmentIncl().push_back(fragment1);
    _crv->getFareBasis();
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(*_cri, rc));
  }

  void testMatchFareBasisFragment()
  {
    std::string s = "YRT";
    _crv->_fareBasis = "YRT";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YR";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YRTO";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "Y%";
    _crv->_fareBasis = "Y2345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "AY";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "%T";
    _crv->_fareBasis = "1234567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "123456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "Y%T";
    _crv->_fareBasis = "Y1234567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT1";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "%T%";
    _crv->_fareBasis = "1T345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "12T45678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "123T5678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "1234T678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "12345T78";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "123456T8";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "1234567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "T2345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "12345678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "Y%T%";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "%Y%T%";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT34567T";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT3456T8";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "TY3456T8";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));

    s = "%Y%T%78";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "%Y%T%78%";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234567T";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT34567T";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT3456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "TY3456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "Y%T%78%P";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2T45678P";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678QOP";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678OP";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "Y%T%78%P%";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2T45678POS";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678QOP";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678QOPU";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678OP";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678OPO";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "PYT345678OPO";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));

    s = "%Y%T%78%P%";
    _crv->_fareBasis = "Y2T45678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2T45678POS";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678QOP";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23T5678QOPU";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y234T678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y2345T78";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "Y23456T8";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678";
    CPPUNIT_ASSERT(!_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678OP";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "YT345678OPO";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
    _crv->_fareBasis = "PYT345678OPO";
    CPPUNIT_ASSERT(_crv->matchFareBasisFragment(s));
  }

  void testValidateCommissionRule_PASS()
  {
    CarrierCode cxr = "AA";
    CommissionRuleInfo cri;
    CPPUNIT_ASSERT_EQUAL(PASS_CR, _crv->validateCommissionRule(cri, cxr));
  }

  void testValidateCommissionRule_FAIL_FareBasis_Excl()
  {
    CarrierCode cxr = "AA";
    char fbc1 = 'F';
    _cri->fareBasisCodeExcl().push_back(fbc1);
    _crv->getFareBasis();

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_FARE_BASIS_EXCL);
  }

  void testValidateCommissionRule_FAIL_FareBasis_Incl()
  {
    CarrierCode cxr = "AA";
    char fbc = 'F';
    _cri->fareBasisCodeIncl().push_back(fbc);
    _crv->_fareBasis = "YRT";

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_FARE_BASIS_INCL);
  }

  void testValidateCommissionRule_FAIL_ClassOfService_Excl()
  {
    CarrierCode cxr = "AA";
    BookingCode rbd1 = "A ";
    _cri->classOfServiceExcl().push_back(rbd1);
    _crv->getClassOfService();

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_CLASS_OF_SERVICE_EXCL);
  }

  void testValidateCommissionRule_FAIL_ClassOfService_Incl()
  {
    CarrierCode cxr = "AA";
    BookingCode rbd1 = "B ";
    _cri->classOfServiceIncl().push_back(rbd1);
    _crv->getClassOfService();

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_CLASS_OF_SERVICE_INCL);
  }

  void testValidateCommissionRule_FAIL_FBC_Fragment_Excl()
  {
    CarrierCode cxr = "AA";
    std::string fragment = "%F%O%CU%S%";
    _cri->fbcFragmentExcl().push_back(fragment);
    _crv->getFareBasis();

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_FBC_FRAGMENT_EXCL);
  }

  void testValidateCommissionRule_FAIL_FBC_Fragment_Incl()
  {
    CarrierCode cxr = "AA";
    std::string fragment = "%F%O%CU%S1%";
    _cri->fbcFragmentIncl().push_back(fragment);
    _crv->getFareBasis();

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_FBC_FRAGMENT_INCL);
  }

  void testValidateCommissionRule_FAIL_Cabin_Excl()
  {
    CarrierCode cxr = "AA";
    char cab1 = 'R';
    _cri->excludedCabinType().push_back(cab1);

    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_EXCL_CABIN);
  }

  void testValidateCommissionRule_FAIL_Cabin_Incl()
  {
    CarrierCode cxr = "AA";
    char cab1 = 'F';
    _cri->requiredCabinType().push_back(cab1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_REQ_CABIN);
  }

  void testValidateCommissionRule_FAIL_CR_TICKET_CARRIER_EXCL()
  {
    CarrierCode cxr = "AA";
    _cri->ticketingCarrierExcl().push_back(cxr);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_TICKET_CARRIER_EXCL);
  }

  void testValidateCommissionRule_FAIL_CR_TICKET_CARRIER_INCL()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BA";
    _cri->ticketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_TICKET_CARRIER_INCL);
  }

  void testValidateCommissionRule_FAIL_CR_OPER_CARRIER_EXCL()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->operatingCarrierExcl().push_back(cxr1);
    _cri->operatingCarrierExcl().push_back(cxr);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_OPER_CARRIER_EXCL);
  }

  void testValidateCommissionRule_FAIL_CR_OPER_CARRIER_INCL()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->operatingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_OPER_CARRIER_INCL);
  }

  void testValidateCommissionRule_FAIL_CR_MARKET_CARRIER_EXCL()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->marketingCarrierExcl().push_back(cxr1);
    _cri->marketingCarrierExcl().push_back(cxr);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_MARKET_CARRIER_EXCL);
  }

  void testValidateCommissionRule_FAIL_CR_MARKET_CARRIER_INCL()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->marketingCarrierIncl().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_MARKET_CARRIER_INCL);
  }

  void testValidateCommissionRule_FAIL_CR_EXCL_MKT_GOV_CXR()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->excludedMktGovCxr().push_back(cxr1);
    _cri->excludedMktGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_EXCL_MKT_GOV_CXR);
  }

  void testValidateCommissionRule_FAIL_CR_REQ_MKT_GOV_CXR()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->requiredMktGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_REQ_MKT_GOV_CXR);
  }

  void testValidateCommissionRule_FAIL_CR_EXCL_OPER_GOV_CXR()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->excludedOperGovCxr().push_back(cxr1);
    _cri->excludedOperGovCxr().push_back(cxr);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_EXCL_OPER_GOV_CXR);
  }

  void testValidateCommissionRule_FAIL_CR_REQ_OPER_GOV_CXR()
  {
    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "BB";
    _cri->requiredOperGovCxr().push_back(cxr1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_REQ_OPER_GOV_CXR);
  }

  void testValidateCommissionRule_FAIL_CR_PSGR_TYPE()
  {
    PaxTypeCode pax = "ADT";
    _cri->requiredPaxType().push_back(pax);
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_PSGR_TYPE);
  }

  void testValidateCommissionRule_FAIL_CR_EXCL_TKT_DESIGNATOR()
  {
    CarrierCode cxr = "AA";
    TktDesignator tktDesing = "TKT";
    _cri->excludedTktDesig().push_back(tktDesing);
    _crv->_tktDesignator = "TKT";
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_EXCL_TKT_DESIGNATOR);
  }

  void testValidateCommissionRule_FAIL_CR_REQ_TKT_DESIGNATOR()
  {
    CarrierCode cxr = "AA";
    TktDesignator tktDesing = "ADD";
    _cri->requiredTktDesig().push_back(tktDesing);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_REQ_TKT_DESIGNATOR);
  }

  void testValidateCommissionRule_FAIL_CR_ROUNDTRIP()
  {
    CarrierCode cxr = "AA";
    _cri->roundTrip() = 'R';
    _itin->tripCharacteristics().set(Itin::RoundTrip, false);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_ROUNDTRIP);
  }

  void testValidateCommissionRule_FAIL_CR_REQ_NON_STOP()
  {
    CarrierCode cxr = "AA";
    _cri->requiredNonStop().emplace_back(_airSeg1->origAirport(), _airSeg2->destAirport());
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_REQ_NON_STOP);
  }

  void testValidateCommissionRule_FAIL_CR_EXCL_CONN_AIRPORT()
  {
    CarrierCode cxr = "AA";
    LocCode airp1 = "*ANY*";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_EXCL_CONN_AIRPORT);
  }

  void testValidateCommissionRule_FAIL_CR_REQ_CONN_AIRPORT()
  {
    CarrierCode cxr = "AA";
    LocCode airp1 = "MKK";
    LocCode airp2 = "MCP";
    _cri->excludedCnxAirPCodes().push_back(airp1);
    _cri->requiredCnxAirPCodes().push_back(airp2);
    _cri->requiredCnxAirPCodes().push_back(airp1);
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_REQ_CONN_AIRPORT);
  }

  void testValidateCommissionRule_FAIL_CR_INTERLINE_CONNECTION()
  {
    CarrierCode cxr = "AA";
    _cri->interlineConnectionRequired() = 'Y';
    _airSeg2->setMarketingCarrierCode("AA");
    CPPUNIT_ASSERT(_crv->validateCommissionRule(*_cri, cxr) == FAIL_CR_INTERLINE_CONNECTION);
  }

public:
  void setUp()
  {
    _memHandle.create<tse::SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _fp = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _fp->itin() = _itin;
    _fm = _memHandle.create<FareMarket>();
    _fu = _memHandle.create<FareUsage>();
    _loc1 = _memHandle.create<Loc>();
    _loc1->loc() = "DFW";
    _loc2 = _memHandle.create<Loc>();
    _loc2->loc() = "LON";

    _loc3 = _memHandle.create<Loc>();
    _loc3->loc() = "NYC";
    _loc4 = _memHandle.create<Loc>();
    _loc4->loc() = "FRA";

    _fm->boardMultiCity() = "DFW";
    _fm->offMultiCity() = "LON";
    _fm->origin() = _loc1;
    _fm->destination() = _loc2;
    _diag = _memHandle.create<Diag867Collector>();
    _crv = _memHandle.insert(new CommissionsRuleValidatorMock(*_trx, *_fp, *_fu, _diag));
    _cri = _memHandle.create<CommissionRuleInfo>();
    _cri = _memHandle.create<CommissionRuleInfo>();
    _fu->paxTypeFare() = createPaxTypeFare();
    _airSeg1 = _memHandle.create<AirSeg>();
    _airSeg2 = _memHandle.create<AirSeg>();
    _airSeg1->carrier() = "AA";
    _airSeg1->setOperatingCarrierCode("AA");
    _airSeg1->setMarketingCarrierCode("AA");
    _airSeg1->origAirport() = "DFW";
    _airSeg1->destAirport() = "LGA";
    _airSeg1->boardMultiCity() = "DFW";
    _airSeg1->offMultiCity() = "NYC";
    _airSeg1->origin() = _loc1;
    _airSeg1->destination() = _loc3;

    _airSeg2->carrier() = "BA";
    _airSeg2->setOperatingCarrierCode("BA");
    _airSeg2->setMarketingCarrierCode("BA");
    _airSeg2->origAirport() = "JFK";
    _airSeg2->destAirport() = "LHR";
    _airSeg2->boardMultiCity() = "NYC";
    _airSeg2->offMultiCity() = "LON";

    _airSeg2->origin() = _loc3;
    _airSeg2->destination() = _loc2;
    _fu->travelSeg().push_back(_airSeg1);
    _fu->travelSeg().push_back(_airSeg2);
    _itin->travelSeg().push_back(_airSeg1);
    _itin->travelSeg().push_back(_airSeg2);

  }
  void tearDown()
  {
    _memHandle.clear();
  }

  PaxTypeFare* createPaxTypeFare()
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_carrier = "AA";
    fi->_fareClass = "FOCUS";
    fi->_ruleNumber = "0100";
    fi->fareAmount() = 12.34;
    fi->_currency = "USD";
    fi->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    fi->vendor() = "ATP";
    Fare* fare = _memHandle.create<Fare>();
    fare->nucFareAmount() = 12.34;
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();

    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    fcasi->_bookingCode[0] = "A ";
    fcasi->_paxType = "PXX";
    ptf->fareClassAppSegInfo() = fcasi;

    FareClassAppInfo* fca = _memHandle.create<FareClassAppInfo>();
    fca->_fareType = "XPN";
    fca->_displayCatType = 'L';
    ptf->fareClassAppInfo() = fca;

    TariffCrossRefInfo* tcr = _memHandle.create<TariffCrossRefInfo>();
    tcr->ruleTariff() = 3;
    tcr->tariffCat() = 0;
    fare->setTariffCrossRefInfo(tcr);

    ptf->setFare(fare);
    ptf->cabin().setPremiumFirstClass();
    ptf->fareMarket() = _fm;
    return ptf;
  }

  void createAirSegs()
  {
    _airSeg3 = _memHandle.create<AirSeg>();
    _airSeg3->carrier() = "LH";
    _airSeg3->setOperatingCarrierCode("LH");
    _airSeg3->setMarketingCarrierCode("AB");
    _airSeg3->origAirport() = "LGW";
    _airSeg3->destAirport() = "FRA";
    _airSeg1->boardMultiCity() = "LON";
    _airSeg1->offMultiCity() = "FRA";
    _airSeg1->origin() = _loc2;
    _airSeg1->destination() = _loc4;

    FareUsage* fu = const_cast<FareUsage*>(&_crv->_fu);
    fu->travelSeg().push_back(_airSeg3);
  }

private:
  TestMemHandle       _memHandle;
  CommissionsRuleValidator* _crv;
  FareMarket*         _fm;
  PricingTrx*         _trx;
  Itin*               _itin;
  FarePath*           _fp;
  CommissionRuleInfo* _cri;
  FareUsage*          _fu;
  Diag867Collector*   _diag;
  Loc*                _loc1;
  Loc*                _loc2;
  Loc*                _loc3;
  Loc*                _loc4;
  AirSeg*             _airSeg1;
  AirSeg*             _airSeg2;
  AirSeg*             _airSeg3;
};
CPPUNIT_TEST_SUITE_REGISTRATION(CommissionsRuleValidatorTest);
}
