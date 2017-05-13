#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "Common/TrxUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/ValidatingCxrConst.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "Diagnostic/Diag191Collector.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DataModel/ValidatingCxrGSAData.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DataModel/ExchangePricingTrx.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);

class ValidatingCxrUtilTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
  public:
    const Loc*
      getLoc(const LocCode& locCode, const DateTime& date)
      {
        Loc* loc = _memHandle.create<Loc>();
        loc->loc()=locCode;
        loc->nation()="VA";
        return loc;
      }
    const std::vector<CountrySettlementPlanInfo*>&
    getCountrySettlementPlans(const NationCode& nation)
    {
      std::vector<CountrySettlementPlanInfo*>* ret =
          _memHandle.create<std::vector<CountrySettlementPlanInfo*> >();

      if (nation == "US")
      {
        CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
        cspi->setSettlementPlanTypeCode("ARC");
        cspi->setCountryCode("US");
        cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi);
      }
      else if (nation == "AU")
      {
        CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
        cspi->setSettlementPlanTypeCode("BSP");
        cspi->setCountryCode(nation);
        cspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi);
      }
      else if (nation == "VA")
      {
        CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
        cspi->setSettlementPlanTypeCode("SAT");
        cspi->setCountryCode(nation);
        cspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi);
      }
      else if (nation == "SA")
      {
        CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
        cspi->setSettlementPlanTypeCode("GTC");
        cspi->setCountryCode("SA");
        cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi);

        CountrySettlementPlanInfo* cspi2 =
          _memHandle.create<CountrySettlementPlanInfo>();
        cspi2->setSettlementPlanTypeCode("BSP");
        cspi2->setCountryCode("SA");
        cspi2->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi2);
      }

      return *ret;
    }

    const std::vector<AirlineCountrySettlementPlanInfo*>&
    getAirlineCountrySettlementPlans(const NationCode& country,
                                     const CrsCode& gds,
                                     const CarrierCode& airline,
                                     const SettlementPlanType& spType)
    {
      std::vector<AirlineCountrySettlementPlanInfo*>* ret =
          _memHandle.create<std::vector<AirlineCountrySettlementPlanInfo*> >();

      if (country == "US")
      {
        AirlineCountrySettlementPlanInfo* acspi =
            _memHandle.create<AirlineCountrySettlementPlanInfo>();

        acspi->setRequiredTicketingMethod(vcx::TM_EMPTY);
        acspi->setPreferredTicketingMethod(vcx::TM_PAPER);

        ret->push_back(acspi);
      }
      else if (country == "VU" && airline != "AA")
      {
        AirlineCountrySettlementPlanInfo* acspi =
          _memHandle.create<AirlineCountrySettlementPlanInfo>();

        acspi->setRequiredTicketingMethod(vcx::TM_EMPTY);
        if (airline == "DL")
          acspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);

        acspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        if (airline != "DL")
          acspi->setPreferredTicketingMethod(vcx::TM_PAPER);

        ret->push_back(acspi);
      }
      else if (country == "KM" && airline != "AA")
      {
        AirlineCountrySettlementPlanInfo* acspi =
          _memHandle.create<AirlineCountrySettlementPlanInfo>();
        acspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(acspi);
      }
      else if (country == "NR" && airline != "AA" && airline != "LH")
      {
        AirlineCountrySettlementPlanInfo* acspi =
          _memHandle.create<AirlineCountrySettlementPlanInfo>();

        acspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
        acspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);

        ret->push_back(acspi);
      }
      else if ( ("JA" == country) && ("MA" == airline) )
      {
        AirlineCountrySettlementPlanInfo* acspi =
          _memHandle.create<AirlineCountrySettlementPlanInfo>();
        acspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
        acspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(acspi);
      }
      else if (country == "SA")
      {
        AirlineCountrySettlementPlanInfo* acspi =
            _memHandle.create<AirlineCountrySettlementPlanInfo>();

        acspi->setRequiredTicketingMethod(vcx::TM_EMPTY);
        acspi->setPreferredTicketingMethod(vcx::TM_PAPER);

        ret->push_back(acspi);
      }

      return *ret;
    }

    const std::vector<GenSalesAgentInfo*>&
    getGenSalesAgents(const CrsCode& gds,
                      const NationCode& country,
                      const SettlementPlanType& settlementPlan,
                      const CarrierCode& validatingCxr)
    {
      std::vector<GenSalesAgentInfo*>* ret = _memHandle.create<std::vector<GenSalesAgentInfo*> >();
      if (country == "US")
      {
        GenSalesAgentInfo* g = _memHandle.create<GenSalesAgentInfo>();
        g->setCxrCode("AA");

        ret->push_back(g);
      }
      else if (country == "CA" || country == "VU")
      {
        GenSalesAgentInfo* g = _memHandle.create<GenSalesAgentInfo>();
        g->setCxrCode("UA");

        GenSalesAgentInfo* g2 = _memHandle.create<GenSalesAgentInfo>();
        g2->setCxrCode("DL");

        ret->push_back(g);
        ret->push_back(g2);
      }
      else if (country == "NR")
      {
        GenSalesAgentInfo* g = _memHandle.create<GenSalesAgentInfo>();
        g->setCxrCode("UA");

        GenSalesAgentInfo* g2 = _memHandle.create<GenSalesAgentInfo>();
        g2->setCxrCode("LH");

        ret->push_back(g);
        ret->push_back(g2);
      }
      else if (country == "KM")
      {
        GenSalesAgentInfo* g = _memHandle.create<GenSalesAgentInfo>();
        g->setCxrCode("UA");
        ret->push_back(g);
      }

      if (country == "FR")
      {
        GenSalesAgentInfo* gsa = _memHandle.create<GenSalesAgentInfo>();
        gsa->setCxrCode("G1");
        ret->push_back(gsa);
        gsa = _memHandle.create<GenSalesAgentInfo>();
        gsa->setCxrCode("G2");
        ret->push_back(gsa);
      }

      return *ret;
    }

    const std::vector<NeutralValidatingAirlineInfo*>&
    getNeutralValidatingAirlines(const NationCode& country,
                                 const CrsCode& gds,
                                 const SettlementPlanType& spType)
    {
      std::vector<NeutralValidatingAirlineInfo*>* ret =
          _memHandle.create<std::vector<NeutralValidatingAirlineInfo*> >();

      if (country == "US")
      {
        NeutralValidatingAirlineInfo* n = _memHandle.create<NeutralValidatingAirlineInfo>();
        n->setAirline("N1");
        ret->push_back(n);
      }

      if (country == "JA")
      {
        NeutralValidatingAirlineInfo* n = _memHandle.create<NeutralValidatingAirlineInfo>();
        n->setAirline("MA");
        ret->push_back(n);
      }

      return *ret;
    }

    const std::vector<AirlineInterlineAgreementInfo*>&
    getAirlineInterlineAgreements(const NationCode& country,
                                  const CrsCode& gds,
                                  const CarrierCode& vcxr)
    {
      std::vector<AirlineInterlineAgreementInfo*>* aiaList =
          _memHandle.create<std::vector<AirlineInterlineAgreementInfo*> >();

      // All standard agreements
      if ( (country == "MA") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "PG");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "NA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "SA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "JA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "MA");
        aiaList->push_back(aia);
      }
      // All third party agreements
      else if ( (country == "JA") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "PG");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "NA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "SA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "JA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "MA");
        aiaList->push_back(aia);
      }
      // All paper agreements
      else if ( (country == "NA") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "PG");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "NA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "SA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "JA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "MA");
        aiaList->push_back(aia);
      }
      // Mixed agreements
      else if ( (country == "SA") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "PG");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "NA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "SA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "JA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "MA");
        aiaList->push_back(aia);
      }
      else if ( (country == "US") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "PG");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "NA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "SA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "JA");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "MA");
        aiaList->push_back(aia);
      }
      else if ( (country == "CA") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "DL");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "UA");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "AA");
        aiaList->push_back(aia);
      }
      else if ( (country == "VU") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
          createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "DL");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "UA");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "AA");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "PG");
        aiaList->push_back(aia);

        if (vcxr != "UA")
        {
          aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "PA");
          aiaList->push_back(aia);
        }
      }
      else if ( (country == "NR" || country == "KM") && (gds == "1S") )
      {
        AirlineInterlineAgreementInfo* aia =
          createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "LH");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "UA");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "AA");
        aiaList->push_back(aia);

        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "PG");
        aiaList->push_back(aia);
      }
      else if ( (country == vcx::ALL_COUNTRIES) && (gds == "1B") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "P1");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "P2");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "P3");
        aiaList->push_back(aia);
      }
      else if ( (country == "IR") && (gds == "1B") )
      {
        AirlineInterlineAgreementInfo* aia =
            createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_STANDARD, "P4");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_THIRD_PARTY, "P5");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "P6");
        aiaList->push_back(aia);
        aia = createAirlineInterlineAgreement(country, gds, vcxr, vcx::AGR_PAPER, "P7");
        aiaList->push_back(aia);
      }
      // No items
      else
      {
        ;
      }
      return *aiaList;
    }

    AirlineInterlineAgreementInfo* createAirlineInterlineAgreement(const NationCode& country,
                                                                   const CrsCode& gds,
                                                                   const CarrierCode& vcxr,
                                                                   const Alpha3Char& agreementType,
                                                                   const CarrierCode& pcxr)
    {
      AirlineInterlineAgreementInfo* aia = _memHandle.create<AirlineInterlineAgreementInfo>();
      aia->setCountryCode(country);
      aia->setGds(gds);
      aia->setValidatingCarrier(vcxr);
      aia->setAgreementTypeCode(agreementType);
      aia->setParticipatingCarrier(pcxr);
      return aia;
    }
    const std::vector<Customer*> & getCustomer( const PseudoCityCode &key)
    {
      std::vector<Customer*>& cus =  *_memHandle.create<std::vector<Customer*> >();
      Customer* customer = _memHandle.create<Customer>();
      customer->aaCity() = "MIA";
      cus.push_back(customer);
      return cus;
    }

    bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                                const DSTGrpCode& dstgrp2,
                                short& utcoffset,
                                const DateTime& dateTime1,
                                const DateTime& dateTime2)
    {
      utcoffset = 60;
      return true;
    }

  private:
    TestMemHandle _memHandle;
  };

  CPPUNIT_TEST_SUITE(ValidatingCxrUtilTest);
  CPPUNIT_TEST(testGetTicketingMethod);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanTicketingTrx);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanPricingTrx);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanPricingTrxForMultiSp);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanPricingTrx_ArcUser);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanPricingTrx_ArcUserForMultiSp);
  CPPUNIT_TEST(testDetermineCountrySettlementPlan_MultiSpTJROption1GTCWithOutSpOverride);
  CPPUNIT_TEST(testDetermineCountrySettlementPlan_MultiSpTJROption1GTCWithSpOverride);
  CPPUNIT_TEST(testDetermineCountrySettlementPlan_MultiSpTJROption2GTCUserWithoutSpOverride);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanPricingTrx_ArcUser_WithTicketPointOverride);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanPricingTrx_ArcUser_WithTicketPointOverrideForMultiSp);
  CPPUNIT_TEST(testDetermineCountrySettlementPlanBase);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfo);
  CPPUNIT_TEST(testGetRequestedPlan);
  CPPUNIT_TEST(testGetAirlineCountrySettlementPlanInfo);
  CPPUNIT_TEST(testDeterminePlanFromHierarchy);
  CPPUNIT_TEST(testGetValidatingCxrsFromMarketingCxr);
  CPPUNIT_TEST(testGetMarketingCxrsFromValidatingCxrs);
  CPPUNIT_TEST(testGetMarketingCxrFromSwapCxr);
  CPPUNIT_TEST(testGetAllItinCarriers);
  CPPUNIT_TEST(testGetAllItinCarriersForRexUnFlownSegments);
  CPPUNIT_TEST(testGetMarketingItinCxrs);
  CPPUNIT_TEST(testGetSwapCarriers);
  CPPUNIT_TEST(testGetSwapCarriers_Zz);
  CPPUNIT_TEST(testProcessNeutralCarriers);
  CPPUNIT_TEST(testProcessNeutralCarriers_NeutralCxrPartOfItin);
  CPPUNIT_TEST(testProcessNeutralCarriers_NeutralCxrNotPartOfItin);
  CPPUNIT_TEST(testProcessNeutralCarriers_SkipNeutral);
  CPPUNIT_TEST(testGetAlternateCxrInOrder_SwapCxrInValCxrList);

  CPPUNIT_TEST(testCheckMarketingCarrierMissing_NoOpenSegment);
  CPPUNIT_TEST(testCheckMarketingCarrierMissing_OpenSegmentWithAsterisks);
  CPPUNIT_TEST(testCheckMarketingCarrierMissing_OpenSegmentWithBLANK);
  CPPUNIT_TEST(testCheckMarketingCarrierMissing_OpenSegmentWithBLANKBLANK);

  CPPUNIT_TEST(testGetValidatingCxrList);
  CPPUNIT_TEST(testGetValidatingCxrListForMultiSp);
  CPPUNIT_TEST(testGetValidatingCxrList_Gsa);
  CPPUNIT_TEST(testGetValidatingCxrList_Neutral);
  CPPUNIT_TEST(testGetValidatingCxrList_SkipGsaAndNeutral);
  CPPUNIT_TEST(testGetValidatingCxrListOnRexTransaction);
  CPPUNIT_TEST(testGetValidatingCxrListForRexTrx_S96AM);
  CPPUNIT_TEST(testGetValidatingCxrListForRexTrx_S96AM_CxrDoesNotParticipateInSp);

  CPPUNIT_TEST(testGetValidatingCxrListForRexTrx_S96_NOT_AM);
  CPPUNIT_TEST(testGetValidatingCxrListForRexTrx_S96_NOT_AM_CxrDoesNotParticipateInSp);
  CPPUNIT_TEST(testGetValidatingCxrListForRexTrx_S96_NOT_AM_FailInterlineAgmt);
  CPPUNIT_TEST(testGetValidatingCxrListForRexTrx_S96_NOT_AM_PaperConflict);

  CPPUNIT_TEST(testGetValidatingCxrListForRex_AR);

  CPPUNIT_TEST(testCheckPaperAndInterlineAgmt_HasInterlineAgmt);
  CPPUNIT_TEST(testCheckPaperAndInterlineAgmt_NoInterlineAgmt);
  CPPUNIT_TEST(testCheckPaperAndInterlineAgmt_CxrSpecified_NoInterlineAgmt);
  CPPUNIT_TEST(testCheckPaperAndInterlineAgmt_PaperConflict);
  CPPUNIT_TEST(testCheckPaperAndInterlineAgmt_CxrSpecified_PaperConflict);

  CPPUNIT_TEST(testPaperConflict_MultiCarrierItin_G1_PaperConflict_G2_3PT_ERROR);
  CPPUNIT_TEST(testPaperConflict_MultiCarrierItin_G1_PaperConflict_G2_DOESNOT_PARTICIPATE);
  CPPUNIT_TEST(testMultiCarrierItin_G1_G2_3PT_ERROR);
  CPPUNIT_TEST(testMultiCarrierItin_No_GSA_ERROR);
  CPPUNIT_TEST(testMultiCarrierItin_One_GSA_NoPaperAgreement);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForSp);

  CPPUNIT_TEST(testFindAgreement_FoundFirst);
  CPPUNIT_TEST(testFindAgreement_FoundMiddle);
  CPPUNIT_TEST(testFindAgreement_FoundLast);
  CPPUNIT_TEST(testFindAgreement_NotFound);
  CPPUNIT_TEST(testIsElectronicTicketRequired);
  CPPUNIT_TEST(testIsTicketTypeElectronic);
  CPPUNIT_TEST(testIsTicketTypePaper);
  CPPUNIT_TEST(testIsPaperTicketOverrideConflict_Yes);
  CPPUNIT_TEST(testIsPaperTicketOverrideConflict_No);
  CPPUNIT_TEST(testIsPaperTicketConflict_Yes);
  CPPUNIT_TEST(testIsPaperTicketConflict_No);

  CPPUNIT_TEST(testIsPassInterlineAgreement_3PT);
  CPPUNIT_TEST(testIsPassInterlineAgreement_NoPcxAgreement);
  CPPUNIT_TEST(testIsPassInterlineAgreement_AnyAgreementIsOk);

  CPPUNIT_TEST(testValidateInterlineAgreements_NoParticipatingCarriers);
  CPPUNIT_TEST(testValidateInterlineAgreements_NoAgreements);
  CPPUNIT_TEST(testValidateInterlineAgreements_NoPcxAgreement);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicRequiredPaperRequested);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicPreferredPaperRequested);
  CPPUNIT_TEST(testValidateInterlineAgreements_PaperPreferredElectronicRequested_PPR);
  CPPUNIT_TEST(testValidateInterlineAgreements_PaperPreferredElectronicRequested_STD);
  CPPUNIT_TEST(testValidateInterlineAgreements_PaperPreferredElectronicRequested_3PT);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicRequiredElectronicRequested_PPR);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicRequiredElectronicRequested_STD);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicRequiredElectronicRequested_3PT);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicPreferredElectronicRequested_PPR);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicPreferredElectronicRequested_STD);
  CPPUNIT_TEST(testValidateInterlineAgreements_AnyAgreementIsOk);
  CPPUNIT_TEST(testValidateInterlineAgreements_ElectronicPreferredElectronicRequested_3PT);
  CPPUNIT_TEST(testValidateInterlineAgreements_ValidatingCxrOnParticipatingCxrList);
  CPPUNIT_TEST(testValidateInterlineAgreements_ValidatingCxrEqualsOnlyParticipatingCxr);
  CPPUNIT_TEST(testValidateInterlineAgreements_ZZ);

  CPPUNIT_TEST(testCheckInterlineAgreements_NoParticipatingCxrs);
  CPPUNIT_TEST(testCheckInterlineAgreements_VcxrEqualsPcxr);
  CPPUNIT_TEST(testCheckInterlineAgreements_NoAgreements);
  CPPUNIT_TEST(testCheckInterlineAgreements_NoMatchingAgreements);
  CPPUNIT_TEST(testCheckInterlineAgreements_STD);
  CPPUNIT_TEST(testCheckInterlineAgreements_AnyAgreementIsOk);

  CPPUNIT_TEST(testCheckAgreement_PaperRequested);
  CPPUNIT_TEST(testCheckAgreement_ElectronicRequested_ThirdPartyAgreement);
  CPPUNIT_TEST(testCheckAgreement_ElectronicRequested_StandardAgreement);
  CPPUNIT_TEST(testCheckAgreement_ElectronicRequested_PaperAgreement);
  CPPUNIT_TEST(testValidateInterlineAgreements_Diag_NoFailures);
  CPPUNIT_TEST(testValidateInterlineAgreements_Diag_Failure);
  CPPUNIT_TEST(testBuildValidationCxrMsg);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Empty_CXR);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Optional_CXR);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Optional_CXR_NoCXR);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_SingleCarrierItin);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SameGSA);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SameGSA_DifferOrder);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SameGSA_DifferOrder_HighFare);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_MiltiGSA);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SingleGSAPerCarrier);
  CPPUNIT_TEST(testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_NotFitInOneLine);
  CPPUNIT_TEST(testGetAirlineInterlineAgreements_None);
  CPPUNIT_TEST(testGetAirlineInterlineAgreements_OnlyZz);
  CPPUNIT_TEST(testGetAirlineInterlineAgreements_NoZz);
  CPPUNIT_TEST(testGetAirlineInterlineAgreements_ZzAndSpecific);

  CPPUNIT_TEST(testCreateHashString);
  CPPUNIT_TEST(testIsNationValidPass);
  CPPUNIT_TEST(testIsNationValidFail);
  CPPUNIT_TEST(testIsNationValidFailForZZ);
  CPPUNIT_TEST(testadjustTicketDate);
  CPPUNIT_TEST(testDetermineTicketDate_Rex);
  CPPUNIT_TEST(testDetermineTicketDate_PRC);
  CPPUNIT_TEST(testGetSettlementPlanName);

  CPPUNIT_SKIP_TEST(testIsGTCCarriersSingleMarketingCxr_true);
  CPPUNIT_TEST(testIsGTCCarriersSingleMarketingCxr_false);
  CPPUNIT_SKIP_TEST(testIsGTCCarriersMultipleMarketingCxr_true);
  CPPUNIT_TEST(testIsGTCCarriersMultipleMarketingCxr_false);
  CPPUNIT_TEST(testIsGTCCarriersMixedMarketingCxr);
  CPPUNIT_TEST(testSetErrorMsg_withGSA);
  CPPUNIT_TEST(testSetErrorMsg_withOutGSA);
  CPPUNIT_TEST(testSetErrorMsg_withGSA_PaperTktConflict);
  CPPUNIT_TEST(test_isGTCCarrier_false);
  CPPUNIT_TEST(test_isGTCCarrier_true);
  CPPUNIT_TEST(testSetErrorMsg_withGTCCarrier);
  CPPUNIT_TEST(testIsNonPreferredVCPass);
  CPPUNIT_TEST(testIsNonPreferredVCFail);
  CPPUNIT_TEST(teststdIncludesWrapper_Pass);
  CPPUNIT_TEST(teststdIncludesWrapper_Fail);
  CPPUNIT_TEST(testGetValidatingCxrListForMultiSp_noSMV_noIEV_withCRC);
  CPPUNIT_TEST(testGetValidatingCxrListForMultiSp_noSMV_noIEV_withoutCRC);
  CPPUNIT_TEST(testGetValidatingCxrListForMultiSp_noSMV_IEV_withCRC);
  CPPUNIT_TEST(testGetValidatingCxrListForMultiSp_noSMV_IEV_withoutCRC);

  CPPUNIT_TEST(testIsAddValidatingCxr_Pass);
  CPPUNIT_TEST(testIsAddValidatingCxr_Fail);
  CPPUNIT_TEST_SUITE_END();

private:
  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _pTrx;
  TicketingCxrTrx* _tTrx;
  ExchangePricingTrx* _rTrx;
  enum
  {
    Flown = false,
    Unflown = true
  };

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _pTrx = _memHandle.create<PricingTrx>();
    _tTrx = _memHandle.create<TicketingCxrTrx>();
    _rTrx = _memHandle.create<ExchangePricingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void setPricingRequest(const vcx::Pos& pos)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = pos.country;
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = loc;
    agent->cxrCode() = pos.primeHost;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->ticketingAgent() = agent;
    request->electronicTicket() = 'T';
    _pTrx->setRequest( request );
  }

  const std::vector<AirlineInterlineAgreementInfo*>& createAiaList()
  {
    std::vector<AirlineInterlineAgreementInfo*>* aiaList =
        _memHandle.create<std::vector<AirlineInterlineAgreementInfo*> >();
    AirlineInterlineAgreementInfo* aiaAA =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "AA");
    AirlineInterlineAgreementInfo* aiaBB =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "BB");
    AirlineInterlineAgreementInfo* aiaCC =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "CC");
    AirlineInterlineAgreementInfo* aiaDD =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "DD");
    AirlineInterlineAgreementInfo* aiaEE =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "EE");
    aiaList->push_back(aiaAA);
    aiaList->push_back(aiaBB);
    aiaList->push_back(aiaCC);
    aiaList->push_back(aiaDD);
    aiaList->push_back(aiaEE);

    return *aiaList;
  }

  void resetAgreements(std::vector<vcx::ParticipatingCxr>& pcxList)
  {
    for (vcx::ParticipatingCxr& pcx : pcxList)
    {
      pcx.agmtType = vcx::STANDARD;
    }
  }

  void testGetValidatingCxrList()
  {
    std::string errMsg;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    cspi.setSettlementPlanTypeCode("ARC");
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
        *_pTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(v);
    CPPUNIT_ASSERT(v->validatingCarriersData().size() == 1);

    cspi.setCountryCode("CA");
    CPPUNIT_ASSERT(!ValidatingCxrUtil::getValidatingCxrList(
          *_pTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers));
    CPPUNIT_ASSERT_EQUAL(std::string("NO VALID TICKETING AGREEMENTS FOUND"), errMsg);
  }

  void testGetValidatingCxrListForMultiSp()
  {
    Loc loc;
    loc.nation() = "SA";

    vcx::Pos pos( "SA", "1S" );
    setPricingRequest( pos );

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    ta.agentLocation() = &loc;
    Customer tjr;
    tjr.pseudoCity() = "9YMG";
    tjr.pricingApplTag2() = 'N'; // not an ARC user
    ta.agentTJR() = &tjr;
    ta.agentTJR()->settlementPlans()="BSPGEN";
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);

    Diag191Collector diag;
    diag.activate();
    SettlementPlanType* sp = 0;

    ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag, sp);
    CPPUNIT_ASSERT(2 == _pTrx->countrySettlementPlanInfos().size());

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    std::string errMsg;
    Itin itin;

    SpValidatingCxrGSADataMap spGsaDataMap;
    for (const CountrySettlementPlanInfo* cspInfo : _pTrx->countrySettlementPlanInfos())
    {
      const SettlementPlanType& spType = cspInfo->getSettlementPlanTypeCode();
      ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
          *_pTrx, &diag, *cspInfo, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

      CPPUNIT_ASSERT(v);
      CPPUNIT_ASSERT(v->validatingCarriersData().size() == 1);
      spGsaDataMap[spType] = v;
    }
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    CPPUNIT_ASSERT(2==itin.spValidatingCxrGsaDataMap()->size());
    diag.flushMsg();
  }

  void testGetValidatingCxrList_Gsa()
  {
    std::string errMsg;
    const vcx::Pos pos( "KM", "1S" );
    setPricingRequest( pos );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode( pos.country );
    cspi.setSettlementPlanTypeCode( "BSP" );
    //_pTrx->countrySettlementPlanInfo() = &cspi;

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back( "AA" );
    operatingCarriers.push_back( "AA" );

    ValidatingCxrGSAData* vcxrGsaData =
        ValidatingCxrUtil::getValidatingCxrList(
            *_pTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT( vcxrGsaData );
    CPPUNIT_ASSERT( 1 == vcxrGsaData->validatingCarriersData().size() );
    CPPUNIT_ASSERT( vcxrGsaData->hasCarrier( "UA" ) );
    CPPUNIT_ASSERT( vcxrGsaData->gsaSwapMap().count( "AA" ) );
  }

  void testGetValidatingCxrList_Neutral()
  {
    std::string errMsg;
    const vcx::Pos pos( "JA", "1S" );
    setPricingRequest( pos );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode( pos.country );
    cspi.setSettlementPlanTypeCode( "BSP" );
    //_pTrx->countrySettlementPlanInfo() = &cspi;

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back( "NA" );
    operatingCarriers.push_back( "NA" );

    ValidatingCxrGSAData* vcxrGsaData =
        ValidatingCxrUtil::getValidatingCxrList(
            *_pTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT( vcxrGsaData );
    CPPUNIT_ASSERT( 1 == vcxrGsaData->validatingCarriersData().size() );
    CPPUNIT_ASSERT( 0 == vcxrGsaData->gsaSwapMap().size() );
    CPPUNIT_ASSERT( vcxrGsaData->isNeutralValCxr() );
  }

  void testGetValidatingCxrList_SkipGsaAndNeutral()
  {
    std::string errMsg;
    const vcx::Pos pos( "KM", "1S" );
    setPricingRequest( pos );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode( pos.country );
    cspi.setSettlementPlanTypeCode( "BSP" );

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back( "AA" );
    operatingCarriers.push_back( "AA" );

    ValidatingCxrGSAData* vcxrGsaData = ValidatingCxrUtil::getValidatingCxrList(
        *_pTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT( vcxrGsaData );
    CPPUNIT_ASSERT( 1 == vcxrGsaData->validatingCarriersData().size() );
    CPPUNIT_ASSERT( vcxrGsaData->hasCarrier( "UA" ) );
    CPPUNIT_ASSERT( vcxrGsaData->gsaSwapMap().count( "AA" ) );

    _pTrx->setSkipGsa(true);
    _pTrx->setSkipNeutral(true);
    vcxrGsaData = ValidatingCxrUtil::getValidatingCxrList(
        *_pTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);
    CPPUNIT_ASSERT( !vcxrGsaData );
    CPPUNIT_ASSERT_EQUAL( std::string("NO VALID TICKETING AGREEMENTS FOUND"), errMsg );
  }

  void testSetErrorMsg_withOutGSA()
  {
    std::string errMsg;
    const vcx::Pos pos( "AA", "1S" );
    setPricingRequest( pos );

    bool anyCxrHasGsa = false;
    std::vector<vcx::ValidationStatus> gsaStatusList;

    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override
    ValidatingCxrUtil::setErrorMsg(*_pTrx, nullptr, anyCxrHasGsa, gsaStatusList, errMsg);
    CPPUNIT_ASSERT_EQUAL(std::string("AA NOT VALID FOR SETTLEMENT METHOD"), errMsg);
  }

  void testSetErrorMsg_withGSA()
  {
    std::string errMsg;
    const vcx::Pos pos( "AA", "1S" );
    setPricingRequest( pos );

    bool anyCxrHasGsa = true;
    std::vector<vcx::ValidationStatus> gsaStatusList;

    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override
    ValidatingCxrUtil::setErrorMsg(*_pTrx, nullptr, anyCxrHasGsa, gsaStatusList, errMsg);
    CPPUNIT_ASSERT_EQUAL(std::string("NO VALID TICKETING AGREEMENTS FOUND"), errMsg);
  }

  void testSetErrorMsg_withGSA_PaperTktConflict()
  {
    std::string errMsg;
    const vcx::Pos pos( "AA", "1S" );
    setPricingRequest( pos );

    bool anyCxrHasGsa = true;
    std::vector<vcx::ValidationStatus> gsaStatusList { vcx::PAPER_TKT_OVERRIDE_ERR };

    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override
    ValidatingCxrUtil::setErrorMsg(*_pTrx, nullptr, anyCxrHasGsa, gsaStatusList, errMsg);
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
  }

  void test_isGTCCarrier_false()
  {
    std::string errMsg;
    const vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );
    _pTrx->getRequest()->validatingCarrier() = "AA";
    _pTrx->getRequest()->ticketEntry() = 'T';

    Agent ta;
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = pos.country;
    ta.cxrCode() = pos.primeHost;
    ta.agentLocation() = loc;
    _pTrx->getRequest()->ticketingAgent() = &ta;

    CPPUNIT_ASSERT(!ValidatingCxrUtil::isGTCCarrier(
          _pTrx->dataHandle(),
          nullptr,
          pos.country,
          pos.primeHost,
          "AA"));
  }

  void test_isGTCCarrier_true()
  {
    std::string errMsg;
    const vcx::Pos pos( "SA", "1S" );
    setPricingRequest( pos );
    _pTrx->getRequest()->validatingCarrier() = "8P";
    _pTrx->getRequest()->ticketEntry() = 'T';

    Agent ta;
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = pos.country;
    ta.cxrCode() = pos.primeHost;
    ta.agentLocation() = loc;
    _pTrx->getRequest()->ticketingAgent() = &ta;

    CPPUNIT_ASSERT(ValidatingCxrUtil::isGTCCarrier(
          _pTrx->dataHandle(),
          nullptr,
          pos.country,
          pos.primeHost,
          "8P"));
  }

  void testSetErrorMsg_withGTCCarrier()
  {
    std::string errMsg;
    const vcx::Pos pos( "SA", "1S" );
    setPricingRequest( pos );
    _pTrx->getRequest()->validatingCarrier() = "8P";
    _pTrx->getRequest()->ticketEntry() = 'T';

    Agent ta;
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = pos.country;
    ta.cxrCode() = pos.primeHost;
    ta.agentLocation() = loc;
    _pTrx->getRequest()->ticketingAgent() = &ta;

    ValidatingCxrUtil::setErrorMsg(
        *_pTrx,
        nullptr,
        false,
        std::vector<vcx::ValidationStatus>(),
        errMsg);
    CPPUNIT_ASSERT_EQUAL(std::string("8P NOT VALID FOR SETTLEMENT METHOD - GTC CARRIER"), errMsg);
  }

  void testGetValidatingCxrListOnRexTransaction()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("PG");
    airSeg2.setOperatingCarrierCode("PG");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "US", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    //_rTrx->countrySettlementPlanInfo() = &cspi;
    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");
    std::string hashString = "AAPG|AAPG";

    ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
        *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(v);
    CPPUNIT_ASSERT(v->gsaSwapMap().empty());
    CPPUNIT_ASSERT(v->isNeutralValCxr() == false);
  }

  void testGetValidatingCxrListForRexTrx_S96AM()
  {
    _rTrx->reqType() = AGENT_PRICING_MASK;

    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("PG");
    airSeg2.setOperatingCarrierCode("PG");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "US", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    //_rTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(v);
    CPPUNIT_ASSERT(errMsg.empty());
  }

  void testGetValidatingCxrListForRexTrx_S96AM_CxrDoesNotParticipateInSp()
  {
    _rTrx->reqType() = AGENT_PRICING_MASK;

    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("PG");
    airSeg2.setOperatingCarrierCode("PG");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "CA", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country); // AA does not participates in ARC for CA
    //_rTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("AA NOT VALID FOR SETTLEMENT METHOD"), errMsg);
  }

  void testGetValidatingCxrListForRexTrx_S96_NOT_AM_PaperConflict()
  {
    vcx::Pos pos( "US", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );

    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("PG");
    airSeg2.setOperatingCarrierCode("PG");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    cspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    //_rTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);
    _rTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override
    _rTrx->getRequest()->electronicTicket() = 'F'; // XETR

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
  }

  void testGetValidatingCxrListForRexTrx_S96_NOT_AM_CxrDoesNotParticipateInSp()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("PG");
    airSeg2.setOperatingCarrierCode("PG");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("CA"); // AA does not participates in ARC for CA
    //_rTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("AA NOT VALID FOR SETTLEMENT METHOD"), errMsg);
  }

  void testGetValidatingCxrListForRexTrx_S96_NOT_AM()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("PG");
    airSeg2.setOperatingCarrierCode("PG");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "US", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    //_rTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(v);
    CPPUNIT_ASSERT(errMsg.empty());
  }

  void testGetValidatingCxrListForRexTrx_S96_NOT_AM_FailInterlineAgmt()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("XX");
    airSeg2.setOperatingCarrierCode("XX");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "US", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    //_rTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _rTrx->setRequest(&pRequest);
    _rTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("XX");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("XX");

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          *_rTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("AA HAS NO INTERLINE TICKETING AGREEMENT WITH XX"), errMsg);
  }

  void testGetValidatingCxrListForRex_AR()
  {
    std::string errMsg;
    const vcx::Pos pos( "US", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    RexPricingTrx rexTrx;
    //rexTrx.countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    rexTrx.setRequest(&pRequest);

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    marketingCarriers.push_back("PG");

    operatingCarriers.push_back("AA");
    operatingCarriers.push_back("PG");

    CPPUNIT_ASSERT( PricingTrx::AR_EXC_TRX == rexTrx.excTrxType() );

    ValidatingCxrGSAData* vcxrGsaData =
      ValidatingCxrUtil::getValidatingCxrListForRexTrx(
          rexTrx, 0, cspi, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

    CPPUNIT_ASSERT( vcxrGsaData );
    CPPUNIT_ASSERT( 1 == vcxrGsaData->validatingCarriersData().size() );
    CPPUNIT_ASSERT( vcxrGsaData->hasCarrier( "AA" ) );
    CPPUNIT_ASSERT( vcxrGsaData->gsaSwapMap().empty() );
    CPPUNIT_ASSERT( !vcxrGsaData->isNeutralValCxr() );
    CPPUNIT_ASSERT( errMsg.empty() );
  }

  void testCheckPaperAndInterlineAgmt_CxrSpecified_NoInterlineAgmt()
  {
    const CarrierCode vcxr = "JH";
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    _pTrx->getRequest()->validatingCarrier()=vcxr;
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "XX" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("JA");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    std::string errMsg;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;
    const bool success = ValidatingCxrUtil::checkPaperAndInterlineAgmt( *_pTrx,
        diag,
        cspi,
        vcxrData,
        validatingCxrGSAData,
        vcxr,
        pcxList,
        mcxList,
        errMsg);

    CPPUNIT_ASSERT( !success );
    CPPUNIT_ASSERT_EQUAL(std::string("JH HAS NO INTERLINE TICKETING AGREEMENT WITH XX"), errMsg);
  }

  void testCheckPaperAndInterlineAgmt_NoInterlineAgmt()
  {
    const CarrierCode vcxr = "JH";
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "XX" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("JA");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    std::string errMsg;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;
    const bool success = ValidatingCxrUtil::checkPaperAndInterlineAgmt( *_pTrx,
        diag,
        cspi,
        vcxrData,
        validatingCxrGSAData,
        vcxr,
        pcxList,
        mcxList,
        errMsg);

    CPPUNIT_ASSERT( !success );
    CPPUNIT_ASSERT(errMsg.empty());
  }

  void testCheckPaperAndInterlineAgmt_PaperConflict()
  {
    const CarrierCode vcxr          = "JH";
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR

    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "NA" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("JA");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    std::string errMsg;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;
    const bool success = ValidatingCxrUtil::checkPaperAndInterlineAgmt( *_pTrx,
        diag,
        cspi,
        vcxrData,
        validatingCxrGSAData,
        vcxr,
        pcxList,
        mcxList,
        errMsg);

    CPPUNIT_ASSERT( !success );
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
  }

  void testCheckPaperAndInterlineAgmt_CxrSpecified_PaperConflict()
  {
    const CarrierCode vcxr          = "JH";
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    _pTrx->getRequest()->validatingCarrier()=vcxr; // Carrier override
    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR

    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "NA" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("JA");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    std::string errMsg;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;
    const bool success = ValidatingCxrUtil::checkPaperAndInterlineAgmt( *_pTrx,
        diag,
        cspi,
        vcxrData,
        validatingCxrGSAData,
        vcxr,
        pcxList,
        mcxList,
        errMsg);

    CPPUNIT_ASSERT( !success );
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
  }

  // GSAs for AA are G1 and G2
  // G1 has paper conflict and G2 does not participates
  void testPaperConflict_MultiCarrierItin_G1_PaperConflict_G2_DOESNOT_PARTICIPATE()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("PA");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "NR", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setRequiredTicketingMethod(vcx::TM_EMPTY);

    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);
    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR
    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    operatingCarriers.push_back("PA");
    std::string hashString = "AA|PA";

    Diag191Collector diag;
    diag.activate();

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrList(*_pTrx,
          &diag,
          cspi,
          marketingCarriers,
          operatingCarriers,
          errMsg,
          marketingCarriers);
    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "LH DOES NOT PARTICIPATE IN SETTLEMENT METHOD"));
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "CARRIER UA IS NOT ADDED AS VALIDATING CARRIER"));
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "BECAUSE OF PAPER TICKET OVERRIDE CONFLICT"));
  }

  // GSAs for AA are G1 and G2
  // G1 has paper conflict and G2 does not have 3PT agmt
  void testPaperConflict_MultiCarrierItin_G1_PaperConflict_G2_3PT_ERROR()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("PA");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "VU", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setRequiredTicketingMethod(vcx::TM_EMPTY);

    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);
    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR
    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    operatingCarriers.push_back("PA");
    std::string hashString = "AA|PA";

    Diag191Collector diag;
    diag.activate();

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrList(*_pTrx,
          &diag,
          cspi,
          marketingCarriers,
          operatingCarriers,
          errMsg,
          marketingCarriers);
    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "CARRIER DL IS NOT ADDED AS VALIDATING CARRIER"));
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "BECAUSE OF PAPER TICKET OVERRIDE CONFLICT"));
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "UA HAS NO INTERLINE TICKETING AGREEMENT WITH PA"));
  }

  // GSAs for AA are G1 and G2
  // G1 and G2 do not have 3PT agmt
  void testMultiCarrierItin_G1_G2_3PT_ERROR()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("GA");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "VU", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setRequiredTicketingMethod(vcx::TM_EMPTY);

    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);
    _pTrx->getRequest()->electronicTicket() = 'T'; // ETR
    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    operatingCarriers.push_back("GA");
    std::string hashString = "AA|GA";

    Diag191Collector diag;
    diag.activate();

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrList(*_pTrx,
          &diag,
          cspi,
          marketingCarriers,
          operatingCarriers,
          errMsg,
          marketingCarriers);
    //std::cout<<diag.str()<<std::endl;
    CPPUNIT_ASSERT(!v);
    //CPPUNIT_ASSERT_EQUAL(std::string("AA NOT VALID FOR SETTLEMENT METHOD"), errMsg);
    CPPUNIT_ASSERT_EQUAL(std::string("NO VALID TICKETING AGREEMENTS FOUND"), errMsg);
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "DL HAS NO INTERLINE TICKETING AGREEMENT WITH GA"));
    CPPUNIT_ASSERT(strstr(diag.str().c_str(), "UA HAS NO INTERLINE TICKETING AGREEMENT WITH GA"));
  }

  // NO GSAs designated for AA
  void testMultiCarrierItin_No_GSA_ERROR()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("GA");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("TV");
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setRequiredTicketingMethod(vcx::TM_EMPTY);

    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);
    _pTrx->getRequest()->electronicTicket() = 'T'; // ETR
    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    operatingCarriers.push_back("GA");
    std::string hashString = "AA|GA";

    Diag191Collector diag;
    diag.activate();

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrList(*_pTrx,
          &diag,
          cspi,
          marketingCarriers,
          operatingCarriers,
          errMsg,
          marketingCarriers);
    //std::cout<<diag.str()<<std::endl;
    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("AA NOT VALID FOR SETTLEMENT METHOD"), errMsg);
  }

  // One GSA G1 designated for AA but has no apper agreement
  void testMultiCarrierItin_One_GSA_NoPaperAgreement()
  {
    std::string errMsg;
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("AA");
    airSeg1.setOperatingCarrierCode("PA");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    itin.travelSeg() = tSegs;

    const vcx::Pos pos( "KM", "1S" );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    cspi.setSettlementPlanTypeCode("BSP");
    cspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);

    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Agent ta;
    ta.cxrCode() = pos.primeHost;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);
    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR
    _pTrx->getRequest()->validatingCarrier() = "AA"; // Carrier override

    std::vector<CarrierCode> marketingCarriers, operatingCarriers;
    marketingCarriers.push_back("AA");
    operatingCarriers.push_back("PA");
    std::string hashString = "AA|PA";

    Diag191Collector diag;
    diag.activate();

    ValidatingCxrGSAData* v =
      ValidatingCxrUtil::getValidatingCxrList(*_pTrx,
          &diag,
          cspi,
          marketingCarriers,
          operatingCarriers,
          errMsg,
          marketingCarriers);
    CPPUNIT_ASSERT(!v);
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), errMsg);
  }

  void setCountrySettlementPlanInfos(std::initializer_list<std::string> list,
      std::vector<CountrySettlementPlanInfo*>& cspiCol)
  {
    for (auto sp : list)
    {
      CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
      if (cspi) {
        cspi->setSettlementPlanTypeCode(sp);
        cspiCol.push_back(cspi);
      }
    }
  }

  void testGetCountrySettlementPlanInfoForSp()
  {
    std::vector<CountrySettlementPlanInfo*> cspiCol;
    setCountrySettlementPlanInfos({"BSP", "GEN"}, cspiCol);

    CPPUNIT_ASSERT(nullptr == ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(cspiCol, "ARC"));
    CPPUNIT_ASSERT(ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(cspiCol, "BSP"));
    CPPUNIT_ASSERT(ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(cspiCol, "GEN"));

    cspiCol.clear();
    CPPUNIT_ASSERT(nullptr==ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(cspiCol, "BSP"));
    CPPUNIT_ASSERT(nullptr==ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(cspiCol, "GEN"));
  }

  void testCheckPaperAndInterlineAgmt_HasInterlineAgmt()
  {
    const CarrierCode vcxr          = "JH";
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "NA" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("JA");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    std::string errMsg;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;
    const bool success = ValidatingCxrUtil::checkPaperAndInterlineAgmt( *_pTrx,
        diag,
        cspi,
        vcxrData,
        validatingCxrGSAData,
        vcxr,
        pcxList,
        mcxList,
        errMsg);

    CPPUNIT_ASSERT( success );
    CPPUNIT_ASSERT( errMsg.empty() );
  }

  void testProcessNeutralCarriers()
  {
    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("CA");
    cspi.setSettlementPlanTypeCode("GEN"); // Since we do not do NVC for ARC and BSP

    Itin itin;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;

    std::vector<CarrierCode> p;
    std::vector<CarrierCode> m;
    ValidatingCxrUtil::processNeutralCarriers(*_pTrx, 0, cspi, validatingCxrGSAData, p, m);
    CPPUNIT_ASSERT(!validatingCxrGSAData);

    cspi.setCountryCode("US");
    cspi.setSettlementPlanTypeCode("ARC"); // Since we do not do NVC for ARC and BSP

    ValidatingCxrUtil::processNeutralCarriers(*_pTrx, 0, cspi, validatingCxrGSAData, p, m);
    CPPUNIT_ASSERT(validatingCxrGSAData);
    CPPUNIT_ASSERT(validatingCxrGSAData->hasCarrier("N1") == true);

    CPPUNIT_ASSERT(validatingCxrGSAData->isNeutralValCxr() == true);
    //CPPUNIT_ASSERT(validatingCxrGSAData->gsaSwapMap().empty());
  }

  // if neutral cxr is part of Itin then it is not selected.
  void testProcessNeutralCarriers_NeutralCxrPartOfItin()
  {
    std::string errMsg;
    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Itin itin;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;

    std::vector<CarrierCode> p;
    std::vector<CarrierCode> m;
    p.push_back("N1");
    m.push_back("N1");
    ValidatingCxrUtil::processNeutralCarriers(*_pTrx, 0, cspi, validatingCxrGSAData, p, m);

    // Since N1 is not part of Itin Marketing carrier list , we do add it to validating carrier list
    CPPUNIT_ASSERT(!validatingCxrGSAData);
  }

  void testProcessNeutralCarriers_NeutralCxrNotPartOfItin()
  {
    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    cspi.setSettlementPlanTypeCode("ARC"); // Since we do not do NVC for ARC and BSP

    Itin itin;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;

    std::vector<CarrierCode> p;
    std::vector<CarrierCode> m;
    p.push_back("N1");
    m.push_back("N2");
    ValidatingCxrUtil::processNeutralCarriers(*_pTrx, 0, cspi, validatingCxrGSAData, p, m);

    // Since N1 is not part of Itin Marketing carrier list , we do add it to validating carrier list
      CPPUNIT_ASSERT(validatingCxrGSAData);
  }

  void testProcessNeutralCarriers_SkipNeutral()
  {
    Agent ta;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    _pTrx->setRequest(&pRequest);

    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    cspi.setSettlementPlanTypeCode("GEN"); // Since we do not do NVC for ARC and BSP
    //_pTrx->countrySettlementPlanInfo() = &cspi;

    Itin itin;
    ValidatingCxrGSAData* validatingCxrGSAData = 0;

    std::vector<CarrierCode> participatingCxrs;
    std::vector<CarrierCode> marketingCxrs;
    participatingCxrs.push_back("N1"); // NVA for POS US
    marketingCxrs.push_back("M1");

    _pTrx->setSkipNeutral(true);
    ValidatingCxrUtil::processNeutralCarriers(
        *_pTrx, 0, cspi, validatingCxrGSAData, participatingCxrs, marketingCxrs);
    // No neutral because of skip
    CPPUNIT_ASSERT(!validatingCxrGSAData);

    _pTrx->setSkipNeutral(false);
    cspi.setSettlementPlanTypeCode("ARC"); // Since we do not do NVC for ARC and BSP
    ValidatingCxrUtil::processNeutralCarriers(
        *_pTrx, 0, cspi, validatingCxrGSAData, participatingCxrs, marketingCxrs);

    // N1 added to validating carrier list
    CPPUNIT_ASSERT(validatingCxrGSAData);
    CPPUNIT_ASSERT(validatingCxrGSAData->hasCarrier("N1") == true);
    CPPUNIT_ASSERT(validatingCxrGSAData->isNeutralValCxr() == true);
    CPPUNIT_ASSERT(validatingCxrGSAData->gsaSwapMap().empty());
  }

  void testGetSwapCarriers()
  {
    vcx::Pos pos( "XX", "1S" ); // no GSA for POS XX/1S
    setPricingRequest( pos );
    CountrySettlementPlanInfo cspi;
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    std::set<CarrierCode> swapCarriers;

    ValidatingCxrUtil::getGenSalesAgents(_pTrx->dataHandle(),
        0,
        "AA",
        cspi.getCountryCode(),
        _pTrx->getRequest()->ticketingAgent()->cxrCode(),
        cspi.getSettlementPlanTypeCode(),
        swapCarriers);
    CPPUNIT_ASSERT(swapCarriers.empty());

    cspi.setCountryCode("US");
    ValidatingCxrUtil::getGenSalesAgents(_pTrx->dataHandle(),
        0,
        "AA",
        cspi.getCountryCode(),
        _pTrx->getRequest()->ticketingAgent()->cxrCode(),
        cspi.getSettlementPlanTypeCode(),
        swapCarriers);

    // AA does not have 3PT agmt with self
    CPPUNIT_ASSERT(swapCarriers.size() == 0);
    CPPUNIT_ASSERT(swapCarriers.find("AA") == swapCarriers.end());

    swapCarriers.clear();
    cspi.setCountryCode("CA");
    ValidatingCxrUtil::getGenSalesAgents(_pTrx->dataHandle(),
        0,
        "AA",
        cspi.getCountryCode(),
        _pTrx->getRequest()->ticketingAgent()->cxrCode(),
        cspi.getSettlementPlanTypeCode(),
        swapCarriers);
    CPPUNIT_ASSERT(swapCarriers.size() == 2);
    CPPUNIT_ASSERT(swapCarriers.find("DL") != swapCarriers.end());
    CPPUNIT_ASSERT(swapCarriers.find("UA") != swapCarriers.end());
  }

  void testGetSwapCarriers_Zz()
  {
    vcx::Pos pos( "FR", "1B" ); // 1B => country ZZ for mocked interline data
    setPricingRequest( pos );
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("FR");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    std::set<CarrierCode> swapCarriers;

    ValidatingCxrUtil::getGenSalesAgents(_pTrx->dataHandle(),
        0,
        "P2", // Has third party agreement with G1 and G2
        cspi.getCountryCode(),
        _pTrx->getRequest()->ticketingAgent()->cxrCode(),
        cspi.getSettlementPlanTypeCode(),
        swapCarriers);

    size_t expectedSize = 2;
    CPPUNIT_ASSERT_EQUAL(expectedSize, swapCarriers.size());
    CPPUNIT_ASSERT(swapCarriers.find("G1") != swapCarriers.end());
    CPPUNIT_ASSERT(swapCarriers.find("G2") != swapCarriers.end());

    swapCarriers.clear();
    ValidatingCxrUtil::getGenSalesAgents(_pTrx->dataHandle(),
        0,
        "P1", // Does not have third party agreement with G1 or G2
        cspi.getCountryCode(),
        _pTrx->getRequest()->ticketingAgent()->cxrCode(),
        cspi.getSettlementPlanTypeCode(),
        swapCarriers);

    expectedSize = 0;
    CPPUNIT_ASSERT_EQUAL(expectedSize, swapCarriers.size());
  }

  void testGetTicketingMethod()
  {
    /*
    Country settlement	    Carrier                     Returned setting
    ------------------      -------                     ----------------
    Electronic preferred        Electronic preferred    Electronic preferred  1
    Electronic preferred        Electronic required     Electronic required   2
    Electronic preferred        Paper preferred         Paper preferred       3
    Electronic required         Electronic preferred    Electronic required   4
    Electronic required	        Electronic required     Electronic required   5
    Electronic required         Paper preferred         Electronic required   6
    Paper preferred             Electronic preferred    Electronic preferred  7
    Paper preferred             Electronic required     Electronic required   8
    Paper preferred             Paper preferred         Paper preferred       9
    */

    CountrySettlementPlanInfo cspi;
    AirlineCountrySettlementPlanInfo acspi;

    // Testing 1
    cspi.setRequiredTicketingMethod(vcx::TM_EMPTY);
    cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    acspi.setRequiredTicketingMethod(vcx::TM_EMPTY);
    acspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_PREF);

    // Testing 2
    acspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    acspi.setPreferredTicketingMethod(vcx::TM_EMPTY);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_REQ);

    // Testing 3
    acspi.setRequiredTicketingMethod(vcx::TM_EMPTY);
    acspi.setPreferredTicketingMethod(vcx::TM_PAPER);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::PAPER_TKT_PREF);

    // Testing 4
    cspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    cspi.setPreferredTicketingMethod(vcx::TM_EMPTY);
    acspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_REQ);

    // Testing 5
    acspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    acspi.setPreferredTicketingMethod(vcx::TM_EMPTY);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_REQ);

    // Testing 6
    acspi.setRequiredTicketingMethod(vcx::TM_EMPTY);
    acspi.setPreferredTicketingMethod(vcx::TM_PAPER);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_REQ);

    // Testing 7
    cspi.setRequiredTicketingMethod(vcx::TM_EMPTY);
    cspi.setPreferredTicketingMethod(vcx::TM_PAPER);
    acspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_PREF);

    // Testing 8
    acspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    acspi.setPreferredTicketingMethod(vcx::TM_EMPTY);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::ETKT_REQ);

    // Testing 9
    acspi.setRequiredTicketingMethod(vcx::TM_EMPTY);
    acspi.setPreferredTicketingMethod(vcx::TM_PAPER);
    CPPUNIT_ASSERT(ValidatingCxrUtil::getTicketingMethod(cspi, &acspi) == vcx::PAPER_TKT_PREF);
  }

  void testDeterminePlanFromHierarchy()
  {
    std::vector<CountrySettlementPlanInfo*> cspiList;
    CPPUNIT_ASSERT(!ValidatingCxrUtil::determinePlanFromHierarchy(cspiList));

    CountrySettlementPlanInfo cspi1, cspi2, cspi3, cspi4;

    cspi1.setSettlementPlanTypeCode("AAA");
    cspiList.push_back(&cspi1);

    CPPUNIT_ASSERT(!ValidatingCxrUtil::determinePlanFromHierarchy(cspiList));

    cspi2.setSettlementPlanTypeCode("KRY");
    cspiList.push_back(&cspi2);

    CountrySettlementPlanInfo* ret = ValidatingCxrUtil::determinePlanFromHierarchy(cspiList);

    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "KRY");

    cspi3.setSettlementPlanTypeCode("GEN");
    cspiList.push_back(&cspi3);

    ret = ValidatingCxrUtil::determinePlanFromHierarchy(cspiList);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "GEN");

    cspi4.setSettlementPlanTypeCode("RUT");
    cspiList.push_back(&cspi4);

    ret = ValidatingCxrUtil::determinePlanFromHierarchy(cspiList);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "GEN");
  }

void testGetCountrySettlementPlanInfo()
{
  try
  {
    const std::vector<CountrySettlementPlanInfo*>& col =
      ValidatingCxrUtil::getCountrySettlementPlanInfo(_pTrx->dataHandle(), 0, "CA", 0);
    CPPUNIT_ASSERT(col.empty());
  }
  catch (ErrorResponseException& ex)
  {
    CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
    CPPUNIT_ASSERT(ex.message() == "NO VALID SETTLEMENT METHOD FOUND FOR NATION CA");
  }

  const std::vector<CountrySettlementPlanInfo*>& col2 =
    ValidatingCxrUtil::getCountrySettlementPlanInfo(_pTrx->dataHandle(), 0, "US", 0);
  CPPUNIT_ASSERT(!col2.empty() && col2[0]->getSettlementPlanTypeCode() == "ARC");

  // now test with requested plan:
  SettlementPlanType spt = "BBB";
  try
  {
    ValidatingCxrUtil::getCountrySettlementPlanInfo(_pTrx->dataHandle(), 0, "US", &spt);
  }
  catch (ErrorResponseException& ex)
  {
    CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
    CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
  }

  spt = "ARC";
  const std::vector<CountrySettlementPlanInfo*>& col3 =
    ValidatingCxrUtil::getCountrySettlementPlanInfo(_pTrx->dataHandle(), 0, "US", &spt);
  CPPUNIT_ASSERT(!col3.empty() && col3[0]->getSettlementPlanTypeCode() == "ARC");

  try
  {
    const std::vector<CountrySettlementPlanInfo*>& col =
      ValidatingCxrUtil::getCountrySettlementPlanInfo(_pTrx->dataHandle(), 0, "CA", &spt);
    CPPUNIT_ASSERT(col.empty());
  }
  catch (ErrorResponseException& ex)
  {
    CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
    CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
  }
}

  void testDetermineCountrySettlementPlanBase()
  {
    try
    {
      CPPUNIT_ASSERT(
          !ValidatingCxrUtil::determineCountrySettlementPlanBase(_pTrx->dataHandle(), "CA", 0, 0));
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "NO VALID SETTLEMENT METHOD FOUND FOR NATION CA");
    }

    CountrySettlementPlanInfo* ret =
        ValidatingCxrUtil::determineCountrySettlementPlanBase(_pTrx->dataHandle(), "US", 0, 0);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "ARC");

    // now test with requested plan:
    /*
    SettlementPlanType spt = "";
    try
    {
      CPPUNIT_ASSERT(!ValidatingCxrUtil::determineCountrySettlementPlanBase(_pTrx->dataHandle(),
    "US", 0, &spt));
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
    }
    */

    SettlementPlanType spt = "BBB";
    try
    {
      CPPUNIT_ASSERT(!ValidatingCxrUtil::determineCountrySettlementPlanBase(
                         _pTrx->dataHandle(), "US", 0, &spt));
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
    }

    spt = "ARC";
    ret = ValidatingCxrUtil::determineCountrySettlementPlanBase(_pTrx->dataHandle(), "US", 0, &spt);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "ARC");

    try
    {
      CPPUNIT_ASSERT(!ValidatingCxrUtil::determineCountrySettlementPlanBase(
                         _pTrx->dataHandle(), "CA", 0, &spt));
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
    }
  }

  void testDetermineCountrySettlementPlanTicketingTrx()
  {
    TicketingCxrRequest tRequest;
    tRequest.setPosCountry("US");

    _tTrx->setRequest(&tRequest);

    CountrySettlementPlanInfo* ret =
        ValidatingCxrUtil::determineCountrySettlementPlan(*_tTrx, 0, 0);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "ARC");

    SettlementPlanType spt = "ARC";
    ret = ValidatingCxrUtil::determineCountrySettlementPlan(*_tTrx, 0, &spt);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "ARC");

    spt = "AAA";
    try { CPPUNIT_ASSERT(!ValidatingCxrUtil::determineCountrySettlementPlan(*_tTrx, 0, &spt)); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
    }

    tRequest.setPosCountry("CA");
    spt = "BSP";
    try { CPPUNIT_ASSERT(!ValidatingCxrUtil::determineCountrySettlementPlan(*_tTrx, 0, &spt)); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
    }
  }

void testDetermineCountrySettlementPlanPricingTrxForMultiSp()
{
  Loc loc;
  loc.nation() = "CA";

  Agent ta;
  ta.agentLocation() = &loc;

  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  _pTrx->setRequest(&pRequest);

  try { ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0); }
  catch (ErrorResponseException& ex)
  {
    CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
    CPPUNIT_ASSERT(ex.message() == "NO VALID SETTLEMENT METHOD FOUND FOR NATION CA");
  }

  loc.nation() = "US";

  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0, 0);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().front()->getSettlementPlanTypeCode() == "ARC");

  SettlementPlanType spt = "ARC";
  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0, &spt);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().front()->getSettlementPlanTypeCode() == "ARC");

  spt = "AAA";
  try { ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0, &spt); }
  catch (ErrorResponseException& ex)
  {
    CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
    CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
  }
}
  void testDetermineCountrySettlementPlanPricingTrx()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Loc loc;
    loc.nation() = "CA";

    Agent ta;
    ta.agentLocation() = &loc;

    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;

    _pTrx->setRequest(&pRequest);

    try { ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "NO VALID SETTLEMENT METHOD FOUND FOR NATION CA");
    }

    loc.nation() = "US";

    ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0, 0);
    CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfo()->getSettlementPlanTypeCode() == "ARC");

    SettlementPlanType spt = "ARC";
    ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0, &spt);
    CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfo()->getSettlementPlanTypeCode() == "ARC");

    spt = "AAA";
    try { ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, 0, &spt); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
      CPPUNIT_ASSERT(ex.message() == "INVALID SETTLEMENT METHOD FOR POINT OF SALE");
    }
  }

void testDetermineCountrySettlementPlan_MultiSpTJROption1GTCWithOutSpOverride()
{
  const vcx::Pos pos( "SA", "1S" );
  Diag191Collector diag191;
  diag191.activate();
  Loc loc;
  loc.nation() = pos.country;
  Agent ta;
  ta.cxrCode() = pos.primeHost;
  ta.agentLocation() = &loc;
  Customer tjr;
  tjr.pseudoCity() = "A0B0";
  tjr.pricingApplTag2() = 'N'; // not an ARC user
  ta.agentTJR() = &tjr;
  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  _pTrx->setRequest(&pRequest);

  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().size()==2);

  std::stringstream expectedDiag;
  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION SA **" << std::endl;
  expectedDiag << "POS: NATION: SA  PRIME HOST: 1S" << std::endl << std::endl;
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str() );
}

void testDetermineCountrySettlementPlan_MultiSpTJROption1GTCWithSpOverride()
{
  const vcx::Pos pos( "SA", "1S" );
  Diag191Collector diag191;
  diag191.activate();
  Loc loc;
  loc.nation() = pos.country;
  Agent ta;
  ta.cxrCode() = pos.primeHost;
  ta.agentLocation() = &loc;
  Customer tjr;
  tjr.pseudoCity() = "A0B0";
  ta.agentTJR() = &tjr;
  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  _pTrx->setRequest(&pRequest);

  SettlementPlanType spt = "BSP";
  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191, &spt);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().front()->getSettlementPlanTypeCode() == "BSP");

  std::stringstream expectedDiag;

  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION SA **" << std::endl;
  expectedDiag << "SPECIFIED SETTLEMENT METHOD: BSP" << std::endl;
  expectedDiag << "POS: NATION: SA  PRIME HOST: 1S" << std::endl << std::endl;
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str() );
}

void testDetermineCountrySettlementPlan_MultiSpTJROption2GTCUserWithoutSpOverride()
{
  const vcx::Pos pos( "SA", "1S" );
  Diag191Collector diag191;
  diag191.activate();
  Loc loc;
  loc.nation() = pos.country;
  Agent ta;
  ta.cxrCode() = pos.primeHost;
  ta.agentLocation() = &loc;
  Customer tjr;
  tjr.pseudoCity() = "A0B0";
  tjr.pricingApplTag2() = 'N'; // not an ARC user
  ta.agentTJR() = &tjr;
  ta.agentTJR()->settlementPlans()="ARCBSP";
  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  _pTrx->setRequest(&pRequest);

  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().size()==2);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos()[0]->getSettlementPlanTypeCode() == "BSP");
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos()[1]->getSettlementPlanTypeCode() == "GTC");

  std::stringstream expectedDiag;
  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION SA **" << std::endl;
  expectedDiag << "TJR SETTLEMENT METHOD/S: ARC BSP" << std::endl;
  expectedDiag << "ARC INVALID SETTLEMENT METHOD FOR POINT OF SALE" << std::endl;
  expectedDiag << "POS: NATION: SA  PRIME HOST: 1S" << std::endl << std::endl;
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str() );
}

void testDetermineCountrySettlementPlanPricingTrx_ArcUserForMultiSp()
{
  const vcx::Pos pos( "AU", "1S" );
  Diag191Collector diag191;
  diag191.activate();
  Loc loc;
  loc.nation() = pos.country;
  Agent ta;
  ta.cxrCode() = pos.primeHost;
  ta.agentLocation() = &loc;
  Customer tjr;
  tjr.pseudoCity() = "A0B0";
  tjr.pricingApplTag2() = 'N'; // not an ARC user
  ta.agentTJR() = &tjr;
  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  _pTrx->setRequest(&pRequest);

  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().front()->getSettlementPlanTypeCode() == "BSP");

  std::stringstream expectedDiag;

  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION AU **" <<std::endl;
  expectedDiag << " \n" << std::endl;
  expectedDiag << "PLAN TYPE: GTC NATION: AU" << std::endl;
  expectedDiag << "GTC INVALID SETTLEMENT METHOD FOR POINT OF SALE" << std::endl;
  expectedDiag << "POS: NATION: AU  PRIME HOST: 1S" << std::endl << std::endl;
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str() );

  tjr.pricingApplTag2() = 'Y'; // ARC user
  _pTrx = _memHandle.create<PricingTrx>();
  _pTrx->setRequest(&pRequest);
  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfos().size());
  CPPUNIT_ASSERT_EQUAL(std::string("ARC"),
      std::string(_pTrx->countrySettlementPlanInfos().front()->getSettlementPlanTypeCode()));

  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION US **" <<std::endl;
  expectedDiag << " \n" << std::endl;
  expectedDiag << "PLAN TYPE: GTC NATION: AU" << std::endl;
  expectedDiag << "GTC INVALID SETTLEMENT METHOD FOR POINT OF SALE" << std::endl;
  expectedDiag << "NATION US USED FOR ARC USER A0B0" << std::endl;
  expectedDiag << "POS: NATION: AU  PRIME HOST: 1S" << std::endl << std::endl;
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str());
}

void testDetermineCountrySettlementPlanPricingTrx_ArcUser()
{
  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  const vcx::Pos pos( "AU", "1S" );
  Diag191Collector diag191;
  diag191.activate();
  Loc loc;
  loc.nation() = pos.country;
  Agent ta;
  ta.cxrCode() = pos.primeHost;
  ta.agentLocation() = &loc;
  Customer tjr;
  tjr.pseudoCity() = "A0B0";
  tjr.pricingApplTag2() = 'N'; // not an ARC user
  ta.agentTJR() = &tjr;
  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  _pTrx->setRequest(&pRequest);

  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfo()->getSettlementPlanTypeCode() == "BSP");
  std::stringstream expectedDiag;
  std::string test1 = diag191.str();

  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION AU **" << std::endl
    << " " << std::endl
    << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  " << std::endl
    << "PLAN TYPE: BSP  NATION: AU" << std::endl
    << " " << std::endl
    << "POS: NATION: AU  PRIME HOST: 1S" << std::endl << std::endl;
  std::string test2 = expectedDiag.str();
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str() );

  tjr.pricingApplTag2() = 'Y'; // ARC user
  _pTrx->countrySettlementPlanInfo() = 0;
  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT(_pTrx->countrySettlementPlanInfo()->getSettlementPlanTypeCode() == "ARC");

  expectedDiag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION US **" << std::endl
    << " " << std::endl
    << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  " << std::endl
    << "PLAN TYPE: ARC  NATION: US" << std::endl
    << " " << std::endl
    << "NATION US USED FOR ARC USER A0B0" << std::endl
    << "POS: NATION: AU  PRIME HOST: 1S" << std::endl << std::endl;
  CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), diag191.str() );
}

void testDetermineCountrySettlementPlanPricingTrx_ArcUser_WithTicketPointOverrideForMultiSp()
{
  const vcx::Pos pos( "AU", "1S" );
  Diag191Collector diag191;
  diag191.activate();
  Loc loc;
  loc.nation() = pos.country;
  Agent ta;
  ta.cxrCode() = pos.primeHost;
  ta.agentLocation() = &loc;
  Customer tjr;
  tjr.pseudoCity() = "A0B0";
  tjr.pricingApplTag2() = 'Y'; // ARC user
  ta.agentTJR() = &tjr;
  PricingRequest pRequest;
  pRequest.ticketingAgent() = &ta;
  pRequest.ticketPointOverride()="VA";
  _pTrx->setRequest(&pRequest);

  ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
  CPPUNIT_ASSERT_EQUAL(std::string("SAT"),
      std::string(_pTrx->countrySettlementPlanInfos().front()->getSettlementPlanTypeCode()));
}

  void testDetermineCountrySettlementPlanPricingTrx_ArcUser_WithTicketPointOverride()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    const vcx::Pos pos( "AU", "1S" );
    Diag191Collector diag191;
    diag191.activate();
    Loc loc;
    loc.nation() = pos.country;
    Agent ta;
    ta.cxrCode() = pos.primeHost;
    ta.agentLocation() = &loc;
    Customer tjr;
    tjr.pseudoCity() = "A0B0";
    tjr.pricingApplTag2() = 'Y'; // ARC user
    ta.agentTJR() = &tjr;
    PricingRequest pRequest;
    pRequest.ticketingAgent() = &ta;
    pRequest.ticketPointOverride()="VA";
    _pTrx->setRequest(&pRequest);

    ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag191);
    CPPUNIT_ASSERT_EQUAL(std::string("SAT"),
        std::string(_pTrx->countrySettlementPlanInfo()->getSettlementPlanTypeCode()));
  }

  void testGetRequestedPlan()
  {
    std::vector<CountrySettlementPlanInfo*> cspiList;
    CPPUNIT_ASSERT(!ValidatingCxrUtil::getRequestedPlan("AAA", cspiList));

    CountrySettlementPlanInfo cspi1, cspi2;
    cspi1.setSettlementPlanTypeCode("AAA");

    cspiList.push_back(&cspi1);

    CountrySettlementPlanInfo* ret = ValidatingCxrUtil::getRequestedPlan("AAA", cspiList);

    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "AAA");

    CPPUNIT_ASSERT(!ValidatingCxrUtil::getRequestedPlan("GEN", cspiList));

    cspi2.setSettlementPlanTypeCode("GEN");
    cspiList.push_back(&cspi2);

    ret = ValidatingCxrUtil::getRequestedPlan("GEN", cspiList);
    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "GEN");

    ret = ValidatingCxrUtil::getRequestedPlan("AAA", cspiList);

    CPPUNIT_ASSERT(ret->getSettlementPlanTypeCode() == "AAA");

    CPPUNIT_ASSERT(!ValidatingCxrUtil::getRequestedPlan("", cspiList));
  }

  void testGetAirlineCountrySettlementPlanInfo()
  {
    CrsCode crs;
    Diag191Collector* diag = 0;
    CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
    cspi->setCountryCode("CA");

    CPPUNIT_ASSERT(!ValidatingCxrUtil::getAirlineCountrySettlementPlanInfo(
                       *cspi, diag, "AA", crs, _pTrx->dataHandle()));

    cspi->setCountryCode("US");
    CPPUNIT_ASSERT(ValidatingCxrUtil::getAirlineCountrySettlementPlanInfo(
        *cspi, diag, "AA", crs, _pTrx->dataHandle()));
  }

  void testGetMarketingCxrsFromValidatingCxrs()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );
    Itin itin;
    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;
    std::vector<CarrierCode> marketingCxrs, validatingCxrs;

    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(marketingCxrs.empty());

    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(marketingCxrs.empty());

    validatingCxrs.push_back("AA");
    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(marketingCxrs.empty());

    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1, airSeg2;

    airSeg1.setMarketingCarrierCode("AA");
    airSeg2.setMarketingCarrierCode("DL");

    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(marketingCxrs.size() == 1);
    CPPUNIT_ASSERT(marketingCxrs[0] == "AA");

    _pTrx->getRequest()->validatingCarrier() = "WS";
    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(validatingCxrs.size() == 1);

    _pTrx->getRequest()->validatingCarrier() = "";
    marketingCxrs.clear();
    validatingCxrs.push_back("DL");
    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);

    CPPUNIT_ASSERT(marketingCxrs.size() == 2);
    CPPUNIT_ASSERT(marketingCxrs[0] == "AA");
    CPPUNIT_ASSERT(marketingCxrs[1] == "DL");

    marketingCxrs.clear();
    validatingCxrs.clear();
    validatingCxrs.push_back("AA");

    v.gsaSwapMap()["DL"].insert("AA");

    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(marketingCxrs.size() == 2);
    CPPUNIT_ASSERT(marketingCxrs[0] == "AA");
    CPPUNIT_ASSERT(marketingCxrs[1] == "DL");

    validatingCxrs.clear();
    validatingCxrs.push_back("UA");

    marketingCxrs.clear();
    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);

    CPPUNIT_ASSERT(marketingCxrs.empty());

    v.gsaSwapMap()["DL"].insert("UA");

    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);
    CPPUNIT_ASSERT(marketingCxrs.size() == 1);
    CPPUNIT_ASSERT(marketingCxrs[0] == "DL");

    marketingCxrs.clear();
    v.gsaSwapMap()["AA"].insert("UA");
    ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(*_pTrx, itin, validatingCxrs, marketingCxrs);

    CPPUNIT_ASSERT(marketingCxrs.size() == 2);
    CPPUNIT_ASSERT(marketingCxrs[0] == "AA");
    CPPUNIT_ASSERT(marketingCxrs[1] == "DL");
  }

  void testGetMarketingCxrFromSwapCxr()
  {
    Itin itin;
    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;
    std::set<CarrierCode> ret;

    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "AA");
    CPPUNIT_ASSERT(ret.empty());

    v.gsaSwapMap()["AA"].insert("G1");

    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "AA");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "G1");
    CPPUNIT_ASSERT(ret.size() == 1);
    CPPUNIT_ASSERT(ret.count("AA") == 1);

    ret.clear();
    v.gsaSwapMap()["DL"].insert("G2");

    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "AA");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "G1");
    CPPUNIT_ASSERT(ret.size() == 1);
    CPPUNIT_ASSERT(ret.count("AA") == 1);
    ret.clear();
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "G2");
    CPPUNIT_ASSERT(ret.size() == 1);
    CPPUNIT_ASSERT(ret.count("DL") == 1);

    v.gsaSwapMap()["UA"].insert("G1");
    ret.clear();

    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "AA");
    CPPUNIT_ASSERT(ret.empty());
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "G1");
    CPPUNIT_ASSERT(ret.size() == 2);
    CPPUNIT_ASSERT(ret.count("AA") == 1);
    CPPUNIT_ASSERT(ret.count("UA") == 1);
    ret.clear();
    ret = ValidatingCxrUtil::getMarketingCxrFromSwapCxr(itin, "G2");
    CPPUNIT_ASSERT(ret.size() == 1);
    CPPUNIT_ASSERT(ret.count("DL") == 1);
  }

  void testGetAllItinCarriers()
  {
    Itin itin;
    std::vector<CarrierCode> marketingCarriers, participatingCarriers;

    ValidatingCxrUtil::getAllItinCarriers(*_pTrx, itin, marketingCarriers, participatingCarriers);
    CPPUNIT_ASSERT(marketingCarriers.empty());
    CPPUNIT_ASSERT(participatingCarriers.empty());

    std::vector<TravelSeg*> tSegs;
    ArunkSeg arunkSeg;
    AirSeg airSeg1, airSeg2, airSeg3;

    airSeg1.setMarketingCarrierCode("ZZ");
    airSeg1.setOperatingCarrierCode("ZZ");
    airSeg2.setMarketingCarrierCode("ZZ");
    airSeg2.setOperatingCarrierCode("ZZ");

    tSegs.push_back(&arunkSeg);
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);

    itin.travelSeg() = tSegs;
    ValidatingCxrUtil::getAllItinCarriers(*_pTrx, itin, marketingCarriers, participatingCarriers);

    CPPUNIT_ASSERT(marketingCarriers.size() == 1);
    CPPUNIT_ASSERT(marketingCarriers[0] == "ZZ");

    CPPUNIT_ASSERT(participatingCarriers.size() == 1);
    CPPUNIT_ASSERT(participatingCarriers[0] == "ZZ");

    airSeg3.setMarketingCarrierCode("ZZ");
    airSeg3.setOperatingCarrierCode("UU");
    tSegs.push_back(&airSeg3);
    itin.travelSeg() = tSegs;

    marketingCarriers.clear();
    participatingCarriers.clear();
    ValidatingCxrUtil::getAllItinCarriers(*_pTrx, itin, marketingCarriers, participatingCarriers);
    CPPUNIT_ASSERT(marketingCarriers.size() == 1);
    CPPUNIT_ASSERT(marketingCarriers[0] == "ZZ");

    CPPUNIT_ASSERT(participatingCarriers.size() == 2);
    CPPUNIT_ASSERT(participatingCarriers[0] == "ZZ");
    CPPUNIT_ASSERT(participatingCarriers[1] == "UU");

    airSeg3.setMarketingCarrierCode("CC");
    airSeg3.setOperatingCarrierCode("DD");

    marketingCarriers.clear();
    participatingCarriers.clear();
    ValidatingCxrUtil::getAllItinCarriers(*_pTrx, itin, marketingCarriers, participatingCarriers);

    CPPUNIT_ASSERT(marketingCarriers.size() == 2);
    CPPUNIT_ASSERT(marketingCarriers[0] == "ZZ");
    CPPUNIT_ASSERT(marketingCarriers[1] == "CC");

    CPPUNIT_ASSERT(participatingCarriers.size() == 3);
    CPPUNIT_ASSERT(participatingCarriers[0] == "ZZ");
    CPPUNIT_ASSERT(participatingCarriers[1] == "CC");
    CPPUNIT_ASSERT(participatingCarriers[2] == "DD");
  }

  void testGetAllItinCarriersForRexUnFlownSegments()
  {
    Itin itin;
    std::vector<CarrierCode> marketingCarriers, participatingCarriers;
    _rTrx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    ValidatingCxrUtil::getAllItinCarriers(*_rTrx, itin, marketingCarriers, participatingCarriers);
    CPPUNIT_ASSERT(marketingCarriers.empty());
    CPPUNIT_ASSERT(participatingCarriers.empty());

    std::vector<TravelSeg*> tSegs;
    ArunkSeg arunkSeg;
    AirSeg airSeg1, airSeg2, airSeg3;

    airSeg1.setMarketingCarrierCode("ZZ");
    airSeg1.setOperatingCarrierCode("ZZ");
    airSeg2.setMarketingCarrierCode("UA");
    airSeg2.setOperatingCarrierCode("UA");

    tSegs.push_back(&arunkSeg);
    tSegs.push_back(&airSeg1);
    tSegs.back()->unflown() = false;
    tSegs.push_back(&airSeg2);

    itin.travelSeg() = tSegs;
    ValidatingCxrUtil::getAllItinCarriers(*_rTrx, itin, marketingCarriers, participatingCarriers);

    CPPUNIT_ASSERT(marketingCarriers.size() == 1);
    CPPUNIT_ASSERT(marketingCarriers[0] == "UA");

    CPPUNIT_ASSERT(participatingCarriers.size() == 1);
    CPPUNIT_ASSERT(participatingCarriers[0] == "UA");

  }

  void testGetMarketingItinCxrs()
  {
    Itin itin;
    std::vector<CarrierCode> marketingCxrs;

    ValidatingCxrUtil::getMarketingItinCxrs( itin, marketingCxrs );
    CPPUNIT_ASSERT(marketingCxrs.empty());

    std::vector<TravelSeg*> tSegs;
    ArunkSeg arunkSeg;
    AirSeg airSeg1, airSeg2, airSeg3;

    airSeg1.setMarketingCarrierCode("M1");
    airSeg1.setOperatingCarrierCode("O1");
    airSeg2.setMarketingCarrierCode("M2");
    airSeg2.setOperatingCarrierCode("O2");
    airSeg3.setMarketingCarrierCode("M3");
    airSeg3.setOperatingCarrierCode("O3");
    airSeg3.unflown() = false; // flown segment

    tSegs.push_back( &arunkSeg );
    tSegs.push_back( &airSeg1 );
    tSegs.push_back( &airSeg2 );
    tSegs.push_back( &airSeg3 );

    itin.travelSeg() = tSegs;
    ValidatingCxrUtil::getMarketingItinCxrs( itin, marketingCxrs );

    CPPUNIT_ASSERT(marketingCxrs.size() == 3);
    CPPUNIT_ASSERT(marketingCxrs[0] == "M1");
    CPPUNIT_ASSERT(marketingCxrs[1] == "M2");
    CPPUNIT_ASSERT(marketingCxrs[2] == "M3");
  }

  void testGetValidatingCxrsFromMarketingCxr()
  {
    const SettlementPlanType sp("");
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Itin itin;
    std::set<CarrierCode> validatingCxrs;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData validatingCxrGSAData;
    itin.validatingCxrGsaData() = &validatingCxrGSAData;

    validatingCxrGSAData.validatingCarriersData()["AA"] = v;

    ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(itin, "DL", validatingCxrs);
    CPPUNIT_ASSERT(validatingCxrs.empty());

    ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(itin, "AA", validatingCxrs);
    CPPUNIT_ASSERT(validatingCxrs.size() == 1);
    CPPUNIT_ASSERT(validatingCxrs.count("AA") == 1);

    validatingCxrs.clear();
    validatingCxrGSAData.gsaSwapMap()["DL"].insert("G1");

    ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(itin, "DL", validatingCxrs);
    CPPUNIT_ASSERT(validatingCxrs.size() == 1);
    CPPUNIT_ASSERT(validatingCxrs.count("G1") == 1);

    validatingCxrs.clear();
    validatingCxrGSAData.gsaSwapMap()["DL"].insert("G2");

    ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(itin, "DL", validatingCxrs);
    CPPUNIT_ASSERT(validatingCxrs.size() == 2);
    CPPUNIT_ASSERT(validatingCxrs.count("G1") == 1);
    CPPUNIT_ASSERT(validatingCxrs.count("G2") == 1);
  }

  void testFindAgreement_FoundFirst()
  {
    const CarrierCode firstCarrier = "AA";
    std::vector<AirlineInterlineAgreementInfo*> aiaList = createAiaList();
    CPPUNIT_ASSERT(ValidatingCxrUtil::findAgreement(aiaList, firstCarrier));
  }

  void testFindAgreement_FoundMiddle()
  {
    const CarrierCode middleCarrier = "CC";
    std::vector<AirlineInterlineAgreementInfo*> aiaList = createAiaList();
    CPPUNIT_ASSERT(ValidatingCxrUtil::findAgreement(aiaList, middleCarrier));
  }

  void testFindAgreement_FoundLast()
  {
    const CarrierCode lastCarrier = "EE";
    std::vector<AirlineInterlineAgreementInfo*> aiaList = createAiaList();
    CPPUNIT_ASSERT(ValidatingCxrUtil::findAgreement(aiaList, lastCarrier));
  }

  void testFindAgreement_NotFound()
  {
    const CarrierCode dummyCarrier = "XX";
    std::vector<AirlineInterlineAgreementInfo*> aiaList = createAiaList();
    CPPUNIT_ASSERT(!ValidatingCxrUtil::findAgreement(aiaList, dummyCarrier));
  }

  void testIsElectronicTicketRequired()
  {
    CPPUNIT_ASSERT(ValidatingCxrUtil::isElectronicTicketRequired(vcx::ETKT_REQ));

    CPPUNIT_ASSERT(!ValidatingCxrUtil::isElectronicTicketRequired(vcx::PAPER_TKT_REQ));
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isElectronicTicketRequired(vcx::PAPER_TKT_PREF));
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isElectronicTicketRequired(vcx::ETKT_PREF));
  }

  void testIsTicketTypeElectronic()
  {
    CPPUNIT_ASSERT(ValidatingCxrUtil::isTicketTypeElectronic(vcx::ETKT_REQ));
    CPPUNIT_ASSERT(ValidatingCxrUtil::isTicketTypeElectronic(vcx::ETKT_PREF));

    CPPUNIT_ASSERT(!ValidatingCxrUtil::isTicketTypeElectronic(vcx::PAPER_TKT_REQ));
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isTicketTypeElectronic(vcx::PAPER_TKT_PREF));
  }

  void testIsTicketTypePaper()
  {
    CPPUNIT_ASSERT(ValidatingCxrUtil::isTicketTypePaper(vcx::PAPER_TKT_REQ));
    CPPUNIT_ASSERT(ValidatingCxrUtil::isTicketTypePaper(vcx::PAPER_TKT_PREF));

    CPPUNIT_ASSERT(!ValidatingCxrUtil::isTicketTypePaper(vcx::ETKT_REQ));
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isTicketTypePaper(vcx::ETKT_PREF));
  }

  void testIsPaperTicketOverrideConflict_Yes()
  {
    CPPUNIT_ASSERT(
        ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_REQ, vcx::PAPER_TKT_REQ));
    CPPUNIT_ASSERT(
        ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_REQ, vcx::PAPER_TKT_PREF));
  }

  void testIsPaperTicketOverrideConflict_No()
  {
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::PAPER_TKT_PREF, vcx::PAPER_TKT_REQ));
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::PAPER_TKT_PREF,
                                                                     vcx::PAPER_TKT_PREF));
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::PAPER_TKT_PREF, vcx::ETKT_REQ));
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::PAPER_TKT_PREF, vcx::ETKT_PREF));

    CPPUNIT_ASSERT(!ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_REQ, vcx::ETKT_REQ));
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_REQ, vcx::ETKT_PREF));

    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_PREF, vcx::PAPER_TKT_REQ));
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_PREF, vcx::PAPER_TKT_PREF));
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_PREF, vcx::ETKT_REQ));
    CPPUNIT_ASSERT(
        !ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::ETKT_PREF, vcx::ETKT_PREF));
  }

  void testIsPaperTicketConflict_Yes()
  {
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    // settlement plan
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    //_pTrx->countrySettlementPlanInfo() = &cspi;

    // airline and settlement plan
    AirlineCountrySettlementPlanInfo acspi;
    acspi.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    acspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);

    const CarrierCode vcxr = "AA";
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType =
      ValidatingCxrUtil::getTicketingMethod(cspi, &acspi);

    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR
    CPPUNIT_ASSERT(ValidatingCxrUtil::isPaperTicketConflict(*_pTrx,
          NULL,
          vcxrData,
          vcxr));
  }

  void testIsPaperTicketConflict_No()
  {
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    // settlement plan
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    //_pTrx->countrySettlementPlanInfo() = &cspi;

    // airline and settlement plan
    AirlineCountrySettlementPlanInfo acspi;
    acspi.setRequiredTicketingMethod(vcx::TM_PAPER);
    acspi.setPreferredTicketingMethod(vcx::TM_PAPER);

    const CarrierCode vcxr = "AA";
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType =
      ValidatingCxrUtil::getTicketingMethod(cspi, &acspi);

    _pTrx->getRequest()->electronicTicket() = 'F'; // XETR
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isPaperTicketConflict(*_pTrx,
          NULL,
          vcxrData,
          vcxr));
  }

  void testIsPassInterlineAgreement_3PT()
  {
    const CarrierCode vcxr          = "JH";
    const CarrierCode swappedForCxr = "";
    const bool        isNeutral     = false;
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "NA" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("JA");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    const bool success = ValidatingCxrUtil::isPassInterlineAgreement( *_pTrx,
                                                                     diag,
                                                                     cspi.getCountryCode(),
                                                                     vcxrData,
                                                                     vcxr,
                                                                     pcxList,
                                                                     mcxList,
                                                                     swappedForCxr,
                                                                     isNeutral);
    CPPUNIT_ASSERT( success );

    std::string errMsg;
    _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;
    _pTrx->getRequest()->spvCntyCode().push_back("US");
    _pTrx->getRequest()->spvCntyCode().push_back("JA");

    const bool nspSuccess = ValidatingCxrUtil::isPassNSPInterlineAgreement( *_pTrx,
        diag,
        errMsg,
        cspi.getCountryCode(),
        vcxrData,
        vcxr,
        pcxList,
        mcxList,
        swappedForCxr,
        isNeutral);

    CPPUNIT_ASSERT(nspSuccess );
    CPPUNIT_ASSERT(1 == vcxrData.interlineValidCountries.size());
    CPPUNIT_ASSERT_EQUAL(std::string("JA"), std::string(vcxrData.interlineValidCountries[0]));
  }

  void testIsPassInterlineAgreement_NoPcxAgreement()
  {
    const CarrierCode vcxr          = "JH";
    const CarrierCode swappedForCxr = "";
    const bool        isNeutral     = false;
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "MA" );
    pcxList.push_back( "JA" );
    pcxList.push_back( "XX" ); // No interline agreement for XX
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    Itin itin;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode("US");
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    const bool success = ValidatingCxrUtil::isPassInterlineAgreement( *_pTrx,
                                                                     diag,
                                                                     cspi.getCountryCode(),
                                                                     vcxrData,
                                                                     vcxr,
                                                                     pcxList,
                                                                     mcxList,
                                                                     swappedForCxr,
                                                                     isNeutral);
    CPPUNIT_ASSERT( !success );

    std::string errMsg;
    _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;
    _pTrx->getRequest()->spvCntyCode().push_back("US");
    _pTrx->getRequest()->spvCntyCode().push_back("TH");

    const bool nspSuccess = ValidatingCxrUtil::isPassNSPInterlineAgreement( *_pTrx,
                                                                          diag,
                                                                          errMsg,
                                                                          cspi.getCountryCode(),
                                                                          vcxrData,
                                                                          vcxr,
                                                                          pcxList,
                                                                          mcxList,
                                                                          swappedForCxr,
                                                                          isNeutral);

    CPPUNIT_ASSERT( !nspSuccess );
    CPPUNIT_ASSERT(0 == vcxrData.interlineValidCountries.size());
  }

  void testIsPassInterlineAgreement_AnyAgreementIsOk()
  {
    const CarrierCode vcxr          = "JH";
    const CarrierCode swappedForCxr = "";
    const bool        isNeutral     = false;
    const vcx::Pos pos( "MA", "1S" ); // Generates STD agreements with MA, JA, SA, NA, and PG
    setPricingRequest( pos );
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    std::vector<CarrierCode> pcxList;
    pcxList.push_back( "NA" );
    pcxList.push_back( "SA" );
    std::vector<CarrierCode> mcxList;
    mcxList = pcxList;
    CountrySettlementPlanInfo cspi;
    cspi.setCountryCode(pos.country);
    //_pTrx->countrySettlementPlanInfo() = &cspi;
    Diag191Collector* diag = 0;

    bool success = ValidatingCxrUtil::isPassInterlineAgreement(*_pTrx,
                                                               diag,
                                                               cspi.getCountryCode(),
                                                               vcxrData,
                                                               vcxr,
                                                               pcxList,
                                                               mcxList,
                                                               swappedForCxr,
                                                               isNeutral);
    // The validating carrier must be part of the itin for STD agreement.
    CPPUNIT_ASSERT( !success );

    _pTrx->setOnlyCheckAgreementExistence(true);
    success = ValidatingCxrUtil::isPassInterlineAgreement(*_pTrx,
                                                          diag,
                                                          cspi.getCountryCode(),
                                                          vcxrData,
                                                          vcxr,
                                                          pcxList,
                                                          mcxList,
                                                          swappedForCxr,
                                                          isNeutral);
    CPPUNIT_ASSERT( success );

    std::string errMsg;
    _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;
    _pTrx->getRequest()->spvCntyCode().push_back("US");
    _pTrx->getRequest()->spvCntyCode().push_back("TH");

    const bool nspSuccess = ValidatingCxrUtil::isPassNSPInterlineAgreement( *_pTrx,
        diag,
        errMsg,
        cspi.getCountryCode(),
        vcxrData,
        vcxr,
        pcxList,
        mcxList,
        swappedForCxr,
        isNeutral);

    CPPUNIT_ASSERT(nspSuccess );
    CPPUNIT_ASSERT(1 == vcxrData.interlineValidCountries.size());
    CPPUNIT_ASSERT_EQUAL(std::string("US"), std::string(vcxrData.interlineValidCountries[0]));
  }

  void testValidateInterlineAgreements_NoParticipatingCarriers()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("US", "1S");
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    const bool areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       0,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       isVcxrPartOfItin,
                                                       requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
  }

  void testValidateInterlineAgreements_NoAgreements()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("TH", "1S"); // No agreements returned for country TH
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx("MA");
    vcxrData.participatingCxrs.push_back(pcx);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    const bool areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       0,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       isVcxrPartOfItin,
                                                       requestedTicketType);
    const CarrierCode expectedFailedCarrier = "";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
  }

  void testValidateInterlineAgreements_NoPcxAgreement()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("MA", "1S");
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("XX");
    vcxrData.participatingCxrs.push_back(pcx2);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    const bool areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       0,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       isVcxrPartOfItin,
                                                       requestedTicketType);
    const CarrierCode expectedFailedCarrier = "XX";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);
  }

  // Electronic ticket is required but a paper ticket method is requested.
  // There are no valid agreements for this case.
  // This scenario should never make it to method validateInterlineAgreements.
  // If it does then all agreements will pass.
  void testValidateInterlineAgreements_ElectronicRequiredPaperRequested()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
   // const CarrierCode expectedFailedCarrier = vcxrData.participatingCxrs[0].cxrName;
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);

    // Still passes
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
  }

  // Electronic ticket is preferred and a paper ticket method is requested.
  // All combinations pass. All agreements are valid for this case.
  void testValidateInterlineAgreements_ElectronicPreferredPaperRequested()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::PAPER_ONLY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);
    // All pass, even when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::PAPER_ONLY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);
  }

  // Paper ticket is preferred and an electronic ticket method is requested.
  // PPR agreements fail.
  void testValidateInterlineAgreements_PaperPreferredElectronicRequested_PPR()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;
    vcx::ParticipatingCxr pcx1("PG");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("NA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("JA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("MA");
    vcxrData.participatingCxrs.push_back(pcx4);
    vcx::ParticipatingCxr pcx5("SA");
    vcxrData.participatingCxrs.push_back(pcx5);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const CarrierCode expectedFailedCarrier = "JA"; // First paper agreement

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);

    // PPR agreements fail, even when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);
  }

  // Paper ticket is preferred and an electronic ticket method is requested.
  // STD agreement passes, if the validating carrier is part of the itinerary.
  void testValidateInterlineAgreements_PaperPreferredElectronicRequested_STD()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("MA", "1S"); // Returns STD agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[3].agmtType);

    // STD does not pass, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    const CarrierCode expectedFailedCarrier = "MA";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);
  }

  // Paper ticket is preferred and an electronic ticket method is requested.
  // Third party agreements pass.
  void testValidateInterlineAgreements_PaperPreferredElectronicRequested_3PT()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("JA", "1S"); // Returns third party agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::PAPER_TKT_REQ;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);
    // Third party passes, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);
  }

  // Electronic ticket is required and an electronic ticket method is requested.
  // PPR agreements fail.
  void testValidateInterlineAgreements_ElectronicRequiredElectronicRequested_PPR()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    vcx::ParticipatingCxr pcx1("PG");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("MA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("NA");
    vcxrData.participatingCxrs.push_back(pcx4);
    vcx::ParticipatingCxr pcx5("SA");
    vcxrData.participatingCxrs.push_back(pcx5);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const CarrierCode expectedFailedCarrier = "JA"; // First paper agreement

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);

    // PPR agreements fail, even when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);
  }

  // Electronic ticket is required and an electronic ticket method is requested.
  // STD agreement passes, if the validating carrier is part of the itinerary.
  void testValidateInterlineAgreements_ElectronicRequiredElectronicRequested_STD()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("MA", "1S"); // Returns STD agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);

    // STD does not pass, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    const CarrierCode expectedFailedCarrier = "MA";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);
  }

  // Electronic ticket is required and an electronic ticket method is requested.
  // Third party agreements pass.
  void testValidateInterlineAgreements_ElectronicRequiredElectronicRequested_3PT()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("JA", "1S"); // Returns third party agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);

    // Third party passes, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);
  }

  // Electronic ticket is preferred and an electronic ticket method is requested.
  // PPR agreements fail.
  void testValidateInterlineAgreements_ElectronicPreferredElectronicRequested_PPR()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("PG");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("MA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("NA");
    vcxrData.participatingCxrs.push_back(pcx4);
    vcx::ParticipatingCxr pcx5("SA");
    vcxrData.participatingCxrs.push_back(pcx5);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const CarrierCode expectedFailedCarrier = "JA"; // First paper agreement

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);

    // PPR agreements fail, even when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);

  }

  // Electronic ticket is preferred and an electronic ticket method is requested.
  // STD agreement passes, if the validating carrier is part of the itinerary.
  void testValidateInterlineAgreements_ElectronicPreferredElectronicRequested_STD()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("MA", "1S"); // Returns STD agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);

    // STD does not pass, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    const CarrierCode expectedFailedCarrier = "MA";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);
  }

  // An agreement that would normally fail,
  // passes when we only want to check for the existence of any agreement.
  void testValidateInterlineAgreements_AnyAgreementIsOk()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("MA", "1S");  // generates STD agreements with MA, JA, NA, SA, and PG
    vcx::ValidatingCxrData vcxrData;
    vcx::ParticipatingCxr pcx1("NA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("SA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    bool areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       0,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       !isVcxrPartOfItin,
                                                       requestedTicketType,
                                                       !onlyCheckAgreementExistence);
    // STD is only valid when the validating carrier is part of the itinerary.
    const CarrierCode expectedFailedCarrier = "NA";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT(!vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT_EQUAL(expectedFailedCarrier, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == vcxrData.interlineStatusCode);

    vcxrData.interlineFailedCxr.clear();
    resetAgreements(vcxrData.participatingCxrs);

    // STD does not pass, when the validating carrier is not part of the itinerary.
    areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       0,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       !isVcxrPartOfItin,
                                                       requestedTicketType,
                                                       onlyCheckAgreementExistence);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[1].agmtType);
  }

  // Electronic ticket is preferred and an electronic ticket method is requested.
  // Third party agreements pass.
  void testValidateInterlineAgreements_ElectronicPreferredElectronicRequested_3PT()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("JA", "1S"); // Returns third party agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);

    // Third party passes, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);
  }

  // Electronic ticket is preferred and an electronic ticket method is requested.
  // Validating carrier is on the participating carrier list (should not affect result).
  // Third party agreements pass.
  void testValidateInterlineAgreements_ValidatingCxrOnParticipatingCxrList()
  {
    const CarrierCode validatingCarrier = "MA";
    vcx::Pos pos("JA", "1S"); // Returns third party agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::NO_AGMT, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);

    resetAgreements(vcxrData.participatingCxrs);

    // Third party passes, when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);
  }

  // Only one participating carrier and it is the validating carrier. Passes.
  void testValidateInterlineAgreements_ValidatingCxrEqualsOnlyParticipatingCxr()
  {
    const CarrierCode validatingCarrier = "VC";
    vcx::Pos pos("JA", "1S");
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_REQ;
    vcx::ParticipatingCxr pcx1("VC");
    vcxrData.participatingCxrs.push_back(pcx1);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
   // const CarrierCode expectedFailedCarrier = "VC";

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
  }

  // Electronic ticket is preferred and a paper ticket method is requested.
  // All combinations pass. All agreements are valid for this case.
  // ZZ (all countries) will return rows for P1, P2, and P3.
  void testValidateInterlineAgreements_ZZ()
  {
    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("FR", "1B"); // 1B => ZZ for mocked data
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("P1");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("P2");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("P3");
    vcxrData.participatingCxrs.push_back(pcx3);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;

    bool areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                             0,
                                                                             pos,
                                                                             validatingCarrier,
                                                                             vcxrData,
                                                                             isVcxrPartOfItin,
                                                                             requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::PAPER_ONLY, vcxrData.participatingCxrs[2].agmtType);

    resetAgreements(vcxrData.participatingCxrs);
    // All pass, even when the validating carrier is not part of the itinerary.
    areAgreementsValid = ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                                        0,
                                                                        pos,
                                                                        validatingCarrier,
                                                                        vcxrData,
                                                                        !isVcxrPartOfItin,
                                                                        requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::PAPER_ONLY, vcxrData.participatingCxrs[2].agmtType);
  }

  void testCheckInterlineAgreements_NoParticipatingCxrs()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("US", "1S");
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    const vcx::ValidationStatus status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    CPPUNIT_ASSERT( vcxrData.interlineFailedCxr.empty() );
    CPPUNIT_ASSERT( vcx::VALID_MSG == status );
  }

  void testCheckInterlineAgreements_VcxrEqualsPcxr()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("US", "1S");
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(vcxr); // Participating = validating
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    const vcx::ValidationStatus status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    CPPUNIT_ASSERT( vcxrData.interlineFailedCxr.empty() );
    CPPUNIT_ASSERT( vcx::VALID_MSG == status );
  }

  void testCheckInterlineAgreements_NoAgreements()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("XX", "1S"); // No agreements at all
    vcx::ValidatingCxrData vcxrData;
    vcx::ParticipatingCxr pcx1("P1");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("P2");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    const vcx::ValidationStatus status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    CPPUNIT_ASSERT( vcxrData.interlineFailedCxr.empty() );
    CPPUNIT_ASSERT( vcx::NO_VALID_TKT_AGMT_FOUND == status );
  }

  void testCheckInterlineAgreements_NoMatchingAgreements()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("MA", "1S");  // generates STD agreements with MA, JA, NA, SA, and PG
    vcx::ValidatingCxrData vcxrData;
    vcx::ParticipatingCxr pcx1("P1");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("P2");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    const vcx::ValidationStatus status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    const CarrierCode expectedFailedCarrier = "P1";
    CPPUNIT_ASSERT( expectedFailedCarrier == vcxrData.interlineFailedCxr );
    CPPUNIT_ASSERT( vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status );
  }

  void testCheckInterlineAgreements_STD()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("MA", "1S");  // generates STD agreements with MA, JA, NA, SA, and PG
    vcx::ValidatingCxrData vcxrData;
    vcx::ParticipatingCxr pcx1("NA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("SA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    vcx::ValidationStatus status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    CPPUNIT_ASSERT( vcxrData.interlineFailedCxr.empty() );
    CPPUNIT_ASSERT( vcx::VALID_MSG == status );

    status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    !isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    // For STD the validating carrier must be part of the itin
    const CarrierCode expectedFailedCarrier = "NA";
    CPPUNIT_ASSERT( expectedFailedCarrier == vcxrData.interlineFailedCxr );
    CPPUNIT_ASSERT( vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status );
  }

  void testCheckInterlineAgreements_AnyAgreementIsOk()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("MA", "1S");  // generates STD agreements with MA, JA, NA, SA, and PG
    vcx::ValidatingCxrData vcxrData;
    vcx::ParticipatingCxr pcx1("NA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("SA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcxrData.ticketType = vcx::ETKT_PREF;
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::ETKT_PREF;
    const bool onlyCheckAgreementExistence = true;

    vcx::ValidationStatus status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    !isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    !onlyCheckAgreementExistence);
    // For STD the validating carrier must be part of the itin
    const CarrierCode expectedFailedCarrier = "NA";
    CPPUNIT_ASSERT( expectedFailedCarrier == vcxrData.interlineFailedCxr );
    CPPUNIT_ASSERT( vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status );

    vcxrData.interlineFailedCxr.clear();
    status =
        ValidatingCxrUtil::checkInterlineAgreements(_tTrx->dataHandle(),
                                                    pos,
                                                    vcxr,
                                                    vcxrData,
                                                    !isVcxrPartOfItin,
                                                    requestedTicketType,
                                                    onlyCheckAgreementExistence);
    // Valid because we only checked that an agreement existed, not what kind of agreement.
    CPPUNIT_ASSERT( vcxrData.interlineFailedCxr.empty() );
    CPPUNIT_ASSERT( vcx::VALID_MSG == status );
  }

  void testCheckAgreement_PaperRequested()
  {
    AirlineInterlineAgreementInfo* aia =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "PC");
    vcx::TicketType vcxrTicketType = vcx::ETKT_PREF;
    vcx::TicketType requestedTicketType = vcx::PAPER_TKT_REQ;
    const bool isVcxrPartOfItin = true;

    vcx::ValidationStatus status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    requestedTicketType = vcx::PAPER_TKT_PREF;
    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);
  }

  void testCheckAgreement_ElectronicRequested_ThirdPartyAgreement()
  {
    AirlineInterlineAgreementInfo* aia =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_THIRD_PARTY, "PC");
    vcx::TicketType vcxrTicketType = vcx::ETKT_PREF;
    vcx::TicketType requestedTicketType = vcx::ETKT_REQ;
    const bool isVcxrPartOfItin = true;

    vcx::ValidationStatus status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    requestedTicketType = vcx::ETKT_PREF;
    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);
  }

  void testCheckAgreement_ElectronicRequested_StandardAgreement()
  {
    AirlineInterlineAgreementInfo* aia =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_STANDARD, "PC");
    vcx::TicketType vcxrTicketType = vcx::ETKT_PREF;
    vcx::TicketType requestedTicketType = vcx::ETKT_REQ;
    const bool isVcxrPartOfItin = true;

    vcx::ValidationStatus status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status);

    requestedTicketType = vcx::ETKT_PREF;
    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::VALID_MSG == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status);
  }

  void testCheckAgreement_ElectronicRequested_PaperAgreement()
  {
    AirlineInterlineAgreementInfo* aia =
        _mdh->createAirlineInterlineAgreement("US", "1S", "VC", vcx::AGR_PAPER, "PC");
    vcx::TicketType vcxrTicketType = vcx::ETKT_PREF;
    vcx::TicketType requestedTicketType = vcx::ETKT_REQ;
    const bool isVcxrPartOfItin = true;

    vcx::ValidationStatus status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status);

    requestedTicketType = vcx::ETKT_PREF;
    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status);

    status = ValidatingCxrUtil::checkAgreement(
        *aia, vcxrTicketType, requestedTicketType, !isVcxrPartOfItin);
    CPPUNIT_ASSERT(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == status);
  }

  // Ensure the DIAG doesn't affect the validation result and
  // the DIAG contents are as expected.
  void testValidateInterlineAgreements_Diag_NoFailures()
  {
    Diag191Collector diag;
    diag.activate();

    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("JA");
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("NA");
    vcxrData.participatingCxrs.push_back(pcx3);
    vcx::ParticipatingCxr pcx4("SA");
    vcxrData.participatingCxrs.push_back(pcx4);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;

    const bool areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       &diag,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       isVcxrPartOfItin,
                                                       requestedTicketType);
    CPPUNIT_ASSERT(areAgreementsValid);
    CPPUNIT_ASSERT(vcxrData.interlineFailedCxr.empty());
    CPPUNIT_ASSERT(vcx::VALID_MSG == vcxrData.interlineStatusCode);
    CPPUNIT_ASSERT_EQUAL(vcx::STANDARD, vcxrData.participatingCxrs[0].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::PAPER_ONLY, vcxrData.participatingCxrs[1].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[2].agmtType);
    CPPUNIT_ASSERT_EQUAL(vcx::THIRD_PARTY, vcxrData.participatingCxrs[3].agmtType);

    std::stringstream expectedDiag;
    expectedDiag << "\n  INTERLINE AGREEMENT INFO FOR JH IN:SA" << std::endl << "  PARTICIPATES IN ITIN: Y"
                 << std::endl << "  TICKET TYPE: ETKTPREF" << std::endl
                 << "  REQUESTED TICKET TYPE: PTKTPREF" << std::endl
                 << "  PARTICIPATING CARRIER: MA  AGREEMENT: STD" << std::endl
                 << "  PARTICIPATING CARRIER: JA  AGREEMENT: PPR" << std::endl
                 << "  PARTICIPATING CARRIER: NA  AGREEMENT: 3PT" << std::endl
                 << "  PARTICIPATING CARRIER: SA  AGREEMENT: 3PT" << std::endl << std::endl;

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), diag.str());
  }

  // Ensure the DIAG doesn't affect the validation result and
  // the DIAG contents are as expected.
  void testValidateInterlineAgreements_Diag_Failure()
  {
    Diag191Collector diag;
    diag.activate();

    const CarrierCode validatingCarrier = "JH";
    vcx::Pos pos("SA", "1S"); // Returns various agreement types
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("MA");
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("XX");
    vcxrData.participatingCxrs.push_back(pcx2);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;

    const bool areAgreementsValid =
        ValidatingCxrUtil::validateInterlineAgreements(_tTrx->dataHandle(),
                                                       &diag,
                                                       pos,
                                                       validatingCarrier,
                                                       vcxrData,
                                                       isVcxrPartOfItin,
                                                       requestedTicketType);
    const CarrierCode failedCxr = "XX";
    CPPUNIT_ASSERT(!areAgreementsValid);
    CPPUNIT_ASSERT_EQUAL(failedCxr, vcxrData.interlineFailedCxr);
    CPPUNIT_ASSERT(vcx::VALID_MSG != vcxrData.interlineStatusCode);

    std::stringstream expectedDiag;
    expectedDiag << "\n  INTERLINE AGREEMENT INFO FOR JH IN:SA" << std::endl << "  PARTICIPATES IN ITIN: Y"
                 << std::endl << "  TICKET TYPE: ETKTPREF" << std::endl
                 << "  REQUESTED TICKET TYPE: PTKTPREF" << std::endl
                 << "  PARTICIPATING CARRIER: MA  AGREEMENT: STD" << std::endl
                 << "  PARTICIPATING CARRIER: XX  AGREEMENT: " << std::endl
                 << "  JH HAS NO INTERLINE TICKETING AGREEMENT WITH XX" << std::endl << std::endl;

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), diag.str());
  }

  void testBuildValidationCxrMsg()
  {
    const CarrierCode vcxr = "JH";
    const CarrierCode cxr = "MA";
    const std::string expectedValidMsg = "VALIDATING CARRIER - JH";
    const std::string expectedOverrideMsg = "VALIDATING CARRIER SPECIFIED - JH";
    const std::string expectedSingleGsa = "VALIDATING CARRIER - JH PER GSA AGREEMENT WITH MA";
    const std::string expectedNoValidCarrier =
        "NO VALID TICKETING AGREEMENTS FOUND-TRY SEGMENT SELECT";
    const std::string expectedRexInterlineError = "JH HAS NO INTERLINE TICKETING AGREEMENT WITH MA";
    const std::string expectedRexNoValidCarrier =
        "NO VALID TICKETING AGREEMENTS FOUND"; // NO_VALID_TKT_AGMT_FOUND,
    const std::string expectedNoIet = "JH HAS NO INTERLINE TICKETING AGREEMENT WITH MA";
    const std::string expectedUnknown = "UNKNOWN STATUS";

    vcx::ValidationStatus status = vcx::VALID_MSG;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedValidMsg, gotMsg);

    status = vcx::VALID_OVERRIDE;
    gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedOverrideMsg, gotMsg);

    status = vcx::VALID_SINGLE_GSA;
    gotMsg = ValidatingCxrUtil::buildValidationCxrMsg( status, vcxr, cxr );
    CPPUNIT_ASSERT_EQUAL( expectedSingleGsa, gotMsg );

    status = vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH;
    gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedRexInterlineError, gotMsg);

    status = vcx::NO_VALID_TKT_AGMT_FOUND;
    gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedRexNoValidCarrier, gotMsg);

    status = vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH;
    gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedNoIet, gotMsg);

    status = vcx::MAX_VALIDATION_STATUS;
    gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedUnknown, gotMsg);
  }

  void testBuildValidationCxrMsg_Empty_CXR()
  {
    const CarrierCode vcxr = "";
    const CarrierCode cxr = "";
    const std::string expectedValidMsg = "VALIDATING CARRIER - ";

    vcx::ValidationStatus status = vcx::VALID_MSG;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(status, vcxr, cxr);
    CPPUNIT_ASSERT_EQUAL(expectedValidMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Optional_CXR()
  {
    Itin itin;
    std::vector<CarrierCode> vcxr;
    vcxr.push_back("AA");
    vcxr.push_back("CA");
    vcxr.push_back("BA");

    const std::string expectedOptCxrdMsg = "OPTIONAL VALIDATING CARRIERS - AA BA CA ";

    vcx::ValidationStatus status = vcx::OPTIONAL_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedOptCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Optional_CXR_NoCXR()
  {
    Itin itin;
    std::vector<CarrierCode> vcxr;
    const std::string expectedOptCxrdMsg = "OPTIONAL VALIDATING CARRIERS - ";

    vcx::ValidationStatus status = vcx::OPTIONAL_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedOptCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_SingleCarrierItin()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2;
    airSeg1.setMarketingCarrierCode("KL");
    airSeg2.setMarketingCarrierCode("KL");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["KL"].insert("DL");
    v.gsaSwapMap()["KL"].insert("AF");

    std::vector<CarrierCode> vcxr;
    vcxr.push_back("AF");
    vcxr.push_back("DL");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - AF DL ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3, airSeg4, airSeg5, airSeg6;
    airSeg1.setMarketingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("AA");
    airSeg3.setMarketingCarrierCode("AY");
    airSeg4.setMarketingCarrierCode("AY");
    airSeg5.setMarketingCarrierCode("IB");
    airSeg6.setMarketingCarrierCode("IB");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    tSegs.push_back(&airSeg5);
    tSegs.push_back(&airSeg6);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["AA"].insert("BA");
    v.gsaSwapMap()["AA"].insert("AB");

    std::vector<CarrierCode> vcxr;
    vcxr.push_back("IB");
    vcxr.push_back("BA");
    vcxr.push_back("AB");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - AB BA IB ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SameGSA()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3, airSeg4, airSeg5, airSeg6;
    airSeg1.setMarketingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("AA");
    airSeg3.setMarketingCarrierCode("AY");
    airSeg4.setMarketingCarrierCode("AY");
    airSeg5.setMarketingCarrierCode("IB");
    airSeg6.setMarketingCarrierCode("IB");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    tSegs.push_back(&airSeg5);
    tSegs.push_back(&airSeg6);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["AA"].insert("BA");
    v.gsaSwapMap()["AA"].insert("AB");
    v.gsaSwapMap()["AY"].insert("AB");

    std::vector<CarrierCode> vcxr;

    vcxr.push_back("BA");
    vcxr.push_back("AB");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - AB BA ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SameGSA_DifferOrder_HighFare()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3, airSeg4, airSeg5, airSeg6;
    airSeg1.setMarketingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("AA");
    airSeg3.setMarketingCarrierCode("AY");
    airSeg4.setMarketingCarrierCode("AY");
    airSeg5.setMarketingCarrierCode("IB");
    airSeg6.setMarketingCarrierCode("IB");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    tSegs.push_back(&airSeg5);
    tSegs.push_back(&airSeg6);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["AA"].insert("BA");
    v.gsaSwapMap()["AA"].insert("AB");
    v.gsaSwapMap()["AY"].insert("AB");

    std::vector<CarrierCode> vcxr;

    vcxr.push_back("BA");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - BA ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SameGSA_DifferOrder()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3, airSeg4, airSeg5, airSeg6;
    airSeg1.setMarketingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("AA");
    airSeg3.setMarketingCarrierCode("AY");
    airSeg4.setMarketingCarrierCode("AY");
    airSeg5.setMarketingCarrierCode("IB");
    airSeg6.setMarketingCarrierCode("IB");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    tSegs.push_back(&airSeg5);
    tSegs.push_back(&airSeg6);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["AA"].insert("BA");
    v.gsaSwapMap()["AY"].insert("BA");
    v.gsaSwapMap()["AY"].insert("AB");

    std::vector<CarrierCode> vcxr;

    vcxr.push_back("BA");
    vcxr.push_back("AB");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - BA AB ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_MiltiGSA()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3;
    airSeg1.setMarketingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("AA");
    airSeg3.setMarketingCarrierCode("AY");


    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);

    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["AA"].insert("BA");
    v.gsaSwapMap()["AA"].insert("AZ");

    std::vector<CarrierCode> vcxr;
    vcxr.push_back("IB");
    vcxr.push_back("BA");
    vcxr.push_back("AZ");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - AZ BA ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_SingleGSAPerCarrier()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3, airSeg4;
    airSeg1.setMarketingCarrierCode("EI");
    airSeg2.setMarketingCarrierCode("BA");
    airSeg3.setMarketingCarrierCode("LH");
    airSeg4.setMarketingCarrierCode("EK");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["EI"].insert("IB");
    v.gsaSwapMap()["BA"].insert("VY");

    std::vector<CarrierCode> vcxr;
    vcxr.push_back("LH");
    vcxr.push_back("VY");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - VY LH ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testBuildValidationCxrMsg_Alternate_CXR_MultiCarrierItin_NotFitInOneLine()
  {
    Itin itin;

    AirSeg airSeg1, airSeg2, airSeg3, airSeg4, airSeg5, airSeg6;
    airSeg1.setMarketingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("AA");
    airSeg3.setMarketingCarrierCode("AY");
    airSeg4.setMarketingCarrierCode("KL");
    airSeg5.setMarketingCarrierCode("PP");
    airSeg6.setMarketingCarrierCode("IB");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    tSegs.push_back(&airSeg5);
    tSegs.push_back(&airSeg6);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["AA"].insert("BA");
    v.gsaSwapMap()["AA"].insert("AB");
    v.gsaSwapMap()["AY"].insert("BB");
    v.gsaSwapMap()["AY"].insert("CC");
    v.gsaSwapMap()["LA"].insert("LL");
    v.gsaSwapMap()["LA"].insert("MM");
    v.gsaSwapMap()["KL"].insert("MH");
    v.gsaSwapMap()["KL"].insert("MU");
    v.gsaSwapMap()["IB"].insert("BI");
    v.gsaSwapMap()["IB"].insert("II");

    std::vector<CarrierCode> vcxr;
    vcxr.push_back("PP");
    vcxr.push_back("BA");
    vcxr.push_back("AB");
    vcxr.push_back("BB");
    vcxr.push_back("CC");
    vcxr.push_back("LL");
    vcxr.push_back("MM");
    vcxr.push_back("MH");
    vcxr.push_back("MU");
    vcxr.push_back("BI");
    vcxr.push_back("II");
    vcxr.push_back("AB");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - LL MM AB BA BB CC MH MU PP BI II ";

    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  void testGetAirlineInterlineAgreements_None()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("TH", "1S"); // No agreements returned for POS TH/1S
    std::vector<AirlineInterlineAgreementInfo*> aiaList;
    ValidatingCxrUtil::getAirlineInterlineAgreements(
        _tTrx->dataHandle(), vcxr, pos.country, pos.primeHost, aiaList);
    CPPUNIT_ASSERT( aiaList.empty() );
  }

  void testGetAirlineInterlineAgreements_OnlyZz()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("FR", "1B"); // 1B => ZZ for mocked data
    std::vector<AirlineInterlineAgreementInfo*> aiaList;
    ValidatingCxrUtil::getAirlineInterlineAgreements(
        _tTrx->dataHandle(), vcxr, pos.country, pos.primeHost, aiaList);
    CPPUNIT_ASSERT( 3 == aiaList.size() );
  }

  void testGetAirlineInterlineAgreements_NoZz()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("US", "1S"); // No ZZ for POS US/1S
    std::vector<AirlineInterlineAgreementInfo*> aiaList;
    ValidatingCxrUtil::getAirlineInterlineAgreements(
        _tTrx->dataHandle(), vcxr, pos.country, pos.primeHost, aiaList);
    CPPUNIT_ASSERT( 5 == aiaList.size() );
  }

  void testGetAirlineInterlineAgreements_ZzAndSpecific()
  {
    const CarrierCode vcxr = "VC";
    vcx::Pos pos("IR", "1B"); // 1B => ZZ for mocked data
    std::vector<AirlineInterlineAgreementInfo*> aiaList;
    ValidatingCxrUtil::getAirlineInterlineAgreements(
        _tTrx->dataHandle(), vcxr, pos.country, pos.primeHost, aiaList);
    CPPUNIT_ASSERT( 7 == aiaList.size() );
  }

  void testCreateHashString()
  {
    std::vector<CarrierCode> m, p;

    CPPUNIT_ASSERT(ValidatingCxrUtil::createHashString(m, p) == "|");

    m.push_back("ZZ");
    CPPUNIT_ASSERT(ValidatingCxrUtil::createHashString(m, p) == "ZZ|");

    p.push_back("ZZ");
    CPPUNIT_ASSERT(ValidatingCxrUtil::createHashString(m, p) == "ZZ|");

    p.push_back("AA");
    CPPUNIT_ASSERT(ValidatingCxrUtil::createHashString(m, p) == "ZZ|AA");

    m.push_back("BB");
    CPPUNIT_ASSERT(ValidatingCxrUtil::createHashString(m, p) == "BBZZ|AA");

    m.push_back("DD");
    p.push_back("DD");

    CPPUNIT_ASSERT(ValidatingCxrUtil::createHashString(m, p) == "BBDDZZ|AA");
  }

  void testIsNationValidPass()
  {
    const NationCode country = "US";
    CPPUNIT_ASSERT(ValidatingCxrUtil::isNationValid(*_tTrx, country, 0));
  }

  void testIsNationValidFail()
  {
    const NationCode country = "ABC";
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isNationValid(*_tTrx, country, 0));
  }
  void testIsNationValidFailForZZ()
  {
    const NationCode country = vcx::ALL_COUNTRIES;
    CPPUNIT_ASSERT(!ValidatingCxrUtil::isNationValid(*_tTrx, country, 0));
  }
  void testadjustTicketDate()
  {
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    short utcOffset = ValidatingCxrUtil::adjustTicketDate(*_pTrx);
    CPPUNIT_ASSERT(utcOffset = 60);
  }

  void testDetermineTicketDate_Rex()
  {
    vcx::Pos pos( "JA", "1S" );
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = pos.country;
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = loc;
    agent->cxrCode() = pos.primeHost;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->ticketingAgent() = agent;
    request->electronicTicket() = 'T';
    _rTrx->setRequest( request );
    _rTrx->currentTicketingDT() = _rTrx->dataHandle().ticketDate().addDays(10);
    const DateTime expectedDate = DateTime::localTime();
    const DateTime actualDate = ValidatingCxrUtil::determineTicketDate(*_rTrx);
    CPPUNIT_ASSERT( expectedDate == actualDate );
  }

  void testDetermineTicketDate_PRC()
  {
    vcx::Pos pos( "JA", "1S" ); // Generates 3PT agreements
    setPricingRequest( pos );
    const DateTime expectedDate = _pTrx->dataHandle().ticketDate().addSeconds(60 * 60);
    const DateTime actualDate = ValidatingCxrUtil::determineTicketDate(*_pTrx);
    CPPUNIT_ASSERT( expectedDate == actualDate );
  }

  void testGetSettlementPlanName()
  {
    const std::string expectedPlanNameArc = "AIRLINE REPORTING CORPORATION";
    const std::string expectedPlanNameBsp = "BILLING AND SETTLEMENT PLAN";
    const std::string expectedPlanNameSat = "STUDENT AIRLINE TICKETING AGREEMENT";
    const std::string expectedPlanNameKry = "STUDENT TICKETING KILROY";
    const std::string expectedPlanNamePrt = "PHILIPPINES TRANSITIONAL AIRLINE TICKET";
    const std::string expectedPlanNameRut = "RUSSIAN TRANSITIONAL AIRLINE TICKET";
    const std::string expectedPlanNameGen = "GENERIC TRANSITIONAL AIRLINE TICKET";
    const std::string expectedPlanNameTch = "TRANSPORT CLEARING HOUSE";
    const std::string expectedPlanNameGtc = "GUARANTEED TICKETING CARRIERS";
    const std::string expectedPlanNameIpc = "INSTANT PURCHASE CARRIERS";
    const std::string expectedPlanNameNone = "";
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameArc, vcx::getSettlementPlanName( "ARC" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameBsp, vcx::getSettlementPlanName( "BSP" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameSat, vcx::getSettlementPlanName( "SAT" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameKry, vcx::getSettlementPlanName( "KRY" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNamePrt, vcx::getSettlementPlanName( "PRT" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameRut, vcx::getSettlementPlanName( "RUT" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameGen, vcx::getSettlementPlanName( "GEN" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameTch, vcx::getSettlementPlanName( "TCH" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameGtc, vcx::getSettlementPlanName( "GTC" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameIpc, vcx::getSettlementPlanName( "IPC" ) );
    CPPUNIT_ASSERT_EQUAL( expectedPlanNameNone, vcx::getSettlementPlanName( "XYZ" ) );
  }

  void testIsGTCCarriersSingleMarketingCxr_true()
  {
    std::string cfgGtc = "4N|MO|P6|WJ";
    TestConfigInitializer::setValue("GTC_CARRIERS", cfgGtc, "PRICING_SVC");

    std::vector<CarrierCode> marketingCxr;
    marketingCxr.push_back("P6");
    CPPUNIT_ASSERT_EQUAL(true, ValidatingCxrUtil::isGTCCarriers(*_pTrx, marketingCxr));
  }

  void testIsGTCCarriersSingleMarketingCxr_false()
  {
    std::string cfgGtc = "4N|MO|P6|WJ";
    TestConfigInitializer::setValue("GTC_CARRIERS", cfgGtc, "PRICING_SVC");

    std::vector<CarrierCode> marketingCxr;
    marketingCxr.push_back("AA");
    CPPUNIT_ASSERT_EQUAL(false, ValidatingCxrUtil::isGTCCarriers(*_pTrx, marketingCxr));
  }

  void testIsGTCCarriersMultipleMarketingCxr_true()
  {
    std::string cfgGtc = "4N|MO|P6|WJ";
    TestConfigInitializer::setValue("GTC_CARRIERS", cfgGtc, "PRICING_SVC");

    std::vector<CarrierCode> marketingCxr;
    marketingCxr.push_back("P6");
    marketingCxr.push_back("4N");

    bool isMultipleGtc = marketingCxr.size() > 1 &&
      ValidatingCxrUtil::isGTCCarriers(*_pTrx, marketingCxr);
    CPPUNIT_ASSERT(isMultipleGtc==true);
  }

  void testIsGTCCarriersMultipleMarketingCxr_false()
  {
    std::string cfgGtc = "4N|MO|P6|WJ";
    TestConfigInitializer::setValue("GTC_CARRIERS", cfgGtc, "PRICING_SVC");

    std::vector<CarrierCode> marketingCxr;
    marketingCxr.push_back("AA");
    marketingCxr.push_back("UA");
    CPPUNIT_ASSERT_EQUAL(false, ValidatingCxrUtil::isGTCCarriers(*_pTrx, marketingCxr));
  }

  // Mixed GTC Itin is treated as normal Itin
  void testIsGTCCarriersMixedMarketingCxr()
  {
    std::string cfgGtc = "4N|MO|P6|WJ";
    TestConfigInitializer::setValue("GTC_CARRIERS", cfgGtc, "PRICING_SVC");

    std::vector<CarrierCode> marketingCxr;
    marketingCxr.push_back("AA");
    marketingCxr.push_back("P6");
    CPPUNIT_ASSERT_EQUAL(false, ValidatingCxrUtil::isGTCCarriers(*_pTrx, marketingCxr));
  }

  // When we have swap that is also part of validating carrier list.
  // In such case, swap should take its marketing place and duplicating validating
  // carrier should not be displayed in the alt list of validating carriers.
  void testGetAlternateCxrInOrder_SwapCxrInValCxrList()
  {
    //ValCxr: BA SQ
    //GSA: LH=>SQ
    //GSA: IB=>BA
    //ALT: SQ CX

    Itin itin;
    AirSeg airSeg1, airSeg2, airSeg3, airSeg4, airSeg5;
    airSeg1.setMarketingCarrierCode("LH");
    airSeg2.setMarketingCarrierCode("CX");
    airSeg3.setMarketingCarrierCode("CX");
    airSeg4.setMarketingCarrierCode("IB");
    airSeg5.setMarketingCarrierCode("SQ");

    std::vector<TravelSeg*> tSegs;
    tSegs.push_back(&airSeg1);
    tSegs.push_back(&airSeg2);
    tSegs.push_back(&airSeg3);
    tSegs.push_back(&airSeg4);
    tSegs.push_back(&airSeg5);
    itin.travelSeg() = tSegs;

    ValidatingCxrGSAData v;
    itin.validatingCxrGsaData() = &v;

    v.gsaSwapMap()["LH"].insert("SQ");
    v.gsaSwapMap()["IB"].insert("BA");

    std::vector<CarrierCode> vcxr;
    vcxr.push_back("BA");
    vcxr.push_back("SQ");

    const std::string expectedAltCxrdMsg = "ALTERNATE VALIDATING CARRIER/S - SQ BA ";
    vcx::ValidationStatus status = vcx::ALTERNATE_CXR;
    std::string gotMsg = ValidatingCxrUtil::buildValidationCxrMsg(itin, status, vcxr);
    CPPUNIT_ASSERT_EQUAL(expectedAltCxrdMsg, gotMsg);
  }

  //{ OPEN SEGMENT ERROR VALIDATING CXR
  // Create a vector of CarrierCode with or without open segment
  void setOpenSegment(std::vector<CarrierCode>& marketingCxrs,
      const std::string& carrierCode /*open segment indicator*/)
  {
    Itin itin;
    std::vector<TravelSeg*> tSegs;
    AirSeg airSeg1, airSeg2, airSeg3;

    airSeg1.setMarketingCarrierCode("M1");
    airSeg1.setOperatingCarrierCode("O1");
    airSeg2.setMarketingCarrierCode("M2");
    airSeg2.setOperatingCarrierCode("O2");

    if (carrierCode.empty())
      airSeg3.setMarketingCarrierCode("M3");
    else
      airSeg3.setMarketingCarrierCode(carrierCode);

    airSeg3.setOperatingCarrierCode("O3");
    tSegs.push_back( &airSeg1 );
    tSegs.push_back( &airSeg2 );
    tSegs.push_back( &airSeg3 );

    itin.travelSeg() = tSegs;
    ValidatingCxrUtil::getMarketingItinCxrs( itin, marketingCxrs );
  }

  //@todo 3/24/15: Rewrite this test case of multi SP
  // Return "OPEN SEG ** NOT ALLOWED - CANCEL/REBOOK" for open segment Itin
std::string getOpenSegmentErrorMsg(std::vector<CarrierCode>& marketingCxrs)
{
  std::string errMsg = "";
  ValidatingCxrUtil::getValidatingCxrList( *_pTrx,
      0,
      *_pTrx->countrySettlementPlanInfo(),
      marketingCxrs,
      marketingCxrs,
      errMsg,
      marketingCxrs);
  return errMsg;
}

  // Test open segment in an Itin
  void testCheckMarketingCarrierMissing_NoOpenSegment()
  {
    std::vector<CarrierCode> marketingCxrs;
    setOpenSegment(marketingCxrs, ""); // no open segment
    CPPUNIT_ASSERT(!ValidatingCxrUtil::checkMarketingCarrierMissing(marketingCxrs));
  }

  void testCheckMarketingCarrierMissing_OpenSegmentWithAsterisks()
  {
    std::vector<CarrierCode> marketingCxrs;
    setOpenSegment(marketingCxrs, "**"); //open segment
    std::string errMsg = getOpenSegmentErrorMsg(marketingCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("OPEN SEG ** NOT ALLOWED - CANCEL/REBOOK"), errMsg);
  }

  void testCheckMarketingCarrierMissing_OpenSegmentWithBLANK()
  {
    std::vector<CarrierCode> marketingCxrs;
    setOpenSegment(marketingCxrs, " "); //open segment
    std::string errMsg = getOpenSegmentErrorMsg(marketingCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("OPEN SEG ** NOT ALLOWED - CANCEL/REBOOK"), errMsg);
  }

  void testCheckMarketingCarrierMissing_OpenSegmentWithBLANKBLANK()
  {
    std::vector<CarrierCode> marketingCxrs;
    setOpenSegment(marketingCxrs, "  "); //open segment
    std::string errMsg = getOpenSegmentErrorMsg(marketingCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("OPEN SEG ** NOT ALLOWED - CANCEL/REBOOK"), errMsg);
  }

  void testIsNonPreferredVCPass()
  {
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    _pTrx->getRequest()->nonPreferredVCs().push_back("AA");
    _pTrx->getRequest()->nonPreferredVCs().push_back("AF");
    _pTrx->getRequest()->nonPreferredVCs().push_back("BA");
    _pTrx->getRequest()->nonPreferredVCs().push_back("LH");

    CPPUNIT_ASSERT(ValidatingCxrUtil::isNonPreferredVC(*_pTrx, "BA"));

  }
  void testIsNonPreferredVCFail()
  {
    vcx::Pos pos( "US", "1S" );
    setPricingRequest( pos );

    _pTrx->getRequest()->nonPreferredVCs().push_back("AA");
    _pTrx->getRequest()->nonPreferredVCs().push_back("AF");
    _pTrx->getRequest()->nonPreferredVCs().push_back("BA");
    _pTrx->getRequest()->nonPreferredVCs().push_back("LH");

    CPPUNIT_ASSERT(!ValidatingCxrUtil::isNonPreferredVC(*_pTrx, "KL"));
  }

  void teststdIncludesWrapper_Pass()
   {
     std::vector<CarrierCode> v1, v2;
     v1.push_back("AA");
     v1.push_back("AF");
     v1.push_back("BA");
     v1.push_back("UA");
     v1.push_back("LH");

     v2.push_back("AF");
     v2.push_back("LH");

     CPPUNIT_ASSERT(ValidatingCxrUtil::stdIncludesWrapper(v1, v2));

   }

   void teststdIncludesWrapper_Fail()
   {
     std::vector<CarrierCode> v1, v2;
     v1.push_back("AA");
     v1.push_back("AF");
     v1.push_back("BA");
     v1.push_back("UA");
     v1.push_back("LH");

     v2.push_back("AF");
     v2.push_back("JA");

     CPPUNIT_ASSERT(!ValidatingCxrUtil::stdIncludesWrapper(v1, v2));

   }

   void testGetValidatingCxrListForMultiSp_noSMV_noIEV_withCRC()
   {
     Loc loc;
     loc.nation() = "SA";

     vcx::Pos pos( "SA", "1S" );
     setPricingRequest( pos );

     Agent ta;
     ta.cxrCode() = pos.primeHost;
     ta.agentLocation() = &loc;
     Customer tjr;
     tjr.pseudoCity() = "9YMG";
     tjr.pricingApplTag2() = 'N'; // not an ARC user
     ta.agentTJR() = &tjr;
     ta.agentTJR()->settlementPlans()="BSPGEN";
     PricingRequest pRequest;
     pRequest.ticketingAgent() = &ta;
     _pTrx->setRequest(&pRequest);

     Diag191Collector diag;
     diag.activate();
     SettlementPlanType* sp = 0;

     ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag, sp);


     const std::vector<CountrySettlementPlanInfo*>& cspInfos = _pTrx->countrySettlementPlanInfos();
     const Loc* posLoc = ValidatingCxrUtil::getPOSLoc(*_pTrx, 0);
     CountrySettlementPlanInfo* nspSP = _pTrx->dataHandle().create<CountrySettlementPlanInfo>();
     nspSP->setCountryCode(ValidatingCxrUtil::getNation(*_pTrx, posLoc));
     nspSP->setSettlementPlanTypeCode(NO_SETTLEMENTPLAN);
     const_cast<std::vector<CountrySettlementPlanInfo*>&>(cspInfos).push_back(nspSP);

     CPPUNIT_ASSERT(3 == _pTrx->countrySettlementPlanInfos().size());

     _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;
     _pTrx->getRequest()->spvCxrsCode().push_back("PG");

     std::vector<CarrierCode> marketingCarriers, operatingCarriers;
     marketingCarriers.push_back("AA");
     marketingCarriers.push_back("PG");

     operatingCarriers.push_back("AA");
     operatingCarriers.push_back("PG");

     std::string errMsg;
     Itin itin;

     SpValidatingCxrGSADataMap spGsaDataMap;
     for (const CountrySettlementPlanInfo* cspInfo : _pTrx->countrySettlementPlanInfos())
     {
       const SettlementPlanType& spType = cspInfo->getSettlementPlanTypeCode();
       ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
           *_pTrx, &diag, *cspInfo, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

       CPPUNIT_ASSERT(v);
       CPPUNIT_ASSERT(v->validatingCarriersData().size() == 1);
       spGsaDataMap[spType] = v;
     }
     itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
     CPPUNIT_ASSERT(3==itin.spValidatingCxrGsaDataMap()->size());
     for(const auto& itr : *(itin.spValidatingCxrGsaDataMap()))
     {
       if(itr.first == "BSP")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("BSP"), std::string(itr.first) );
         CPPUNIT_ASSERT(1 == itr.second->validatingCarriersData().size());
         CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(itr.second->validatingCarriersData().begin()->first));
       }
       else if(itr.first == "GTC")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("GTC"), std::string(itr.first) );
         CPPUNIT_ASSERT(1 == itr.second->validatingCarriersData().size());
         CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(itr.second->validatingCarriersData().begin()->first));
       }
       else if(itr.first == "NSP")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("NSP"), std::string(itr.first) );
         CPPUNIT_ASSERT(1 == itr.second->validatingCarriersData().size());
         CPPUNIT_ASSERT_EQUAL(std::string("PG"), std::string(itr.second->validatingCarriersData().begin()->first));
       }
     }

     diag.flushMsg();
   }
   void testGetValidatingCxrListForMultiSp_noSMV_noIEV_withoutCRC()
   {
     Loc loc;
     loc.nation() = "SA";

     vcx::Pos pos( "SA", "1S" );
     setPricingRequest( pos );

     Agent ta;
     ta.cxrCode() = pos.primeHost;
     ta.agentLocation() = &loc;
     Customer tjr;
     tjr.pseudoCity() = "9YMG";
     tjr.pricingApplTag2() = 'N'; // not an ARC user
     ta.agentTJR() = &tjr;
     ta.agentTJR()->settlementPlans()="BSPGEN";
     PricingRequest pRequest;
     pRequest.ticketingAgent() = &ta;
     _pTrx->setRequest(&pRequest);

     Diag191Collector diag;
     diag.activate();
     SettlementPlanType* sp = 0;

     ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag, sp);


     const std::vector<CountrySettlementPlanInfo*>& cspInfos = _pTrx->countrySettlementPlanInfos();
     const Loc* posLoc = ValidatingCxrUtil::getPOSLoc(*_pTrx, 0);
     CountrySettlementPlanInfo* nspSP = _pTrx->dataHandle().create<CountrySettlementPlanInfo>();
     nspSP->setCountryCode(ValidatingCxrUtil::getNation(*_pTrx, posLoc));
     nspSP->setSettlementPlanTypeCode(NO_SETTLEMENTPLAN);
     const_cast<std::vector<CountrySettlementPlanInfo*>&>(cspInfos).push_back(nspSP);

     CPPUNIT_ASSERT(3 == _pTrx->countrySettlementPlanInfos().size());

     _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;

     std::vector<CarrierCode> marketingCarriers, operatingCarriers;
     marketingCarriers.push_back("AA");
     marketingCarriers.push_back("PG");

     operatingCarriers.push_back("AA");
     operatingCarriers.push_back("PG");

     std::string errMsg;
     Itin itin;

     SpValidatingCxrGSADataMap spGsaDataMap;
     for (const CountrySettlementPlanInfo* cspInfo : _pTrx->countrySettlementPlanInfos())
     {
       const SettlementPlanType& spType = cspInfo->getSettlementPlanTypeCode();
       ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
           *_pTrx, &diag, *cspInfo, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

       spGsaDataMap[spType] = v;
     }
     itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
     CPPUNIT_ASSERT(3==itin.spValidatingCxrGsaDataMap()->size());
     for(const auto& itr : *(itin.spValidatingCxrGsaDataMap()))
     {
       if(itr.first == "BSP")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("BSP"), std::string(itr.first) );
         CPPUNIT_ASSERT(!itr.second);
       }
       else if(itr.first == "GTC")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("GTC"), std::string(itr.first) );
         CPPUNIT_ASSERT(!itr.second);
       }
       else if(itr.first == "NSP")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("NSP"), std::string(itr.first) );
         CPPUNIT_ASSERT(2 == itr.second->validatingCarriersData().size());

         auto itrVcxr = itr.second->validatingCarriersData().begin();
         CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(itrVcxr->first));
         itrVcxr++;
         CPPUNIT_ASSERT_EQUAL(std::string("PG"), std::string(itrVcxr->first));
       }
     }
     diag.flushMsg();
   }

   void testGetValidatingCxrListForMultiSp_noSMV_IEV_withCRC()
   {
     Loc loc;
     loc.nation() = "SA";

     vcx::Pos pos( "SA", "1S" );
     setPricingRequest( pos );

     Agent ta;
     ta.cxrCode() = pos.primeHost;
     ta.agentLocation() = &loc;
     Customer tjr;
     tjr.pseudoCity() = "9YMG";
     tjr.pricingApplTag2() = 'N'; // not an ARC user
     ta.agentTJR() = &tjr;
     ta.agentTJR()->settlementPlans()="BSPGEN";
     PricingRequest pRequest;
     pRequest.ticketingAgent() = &ta;
     _pTrx->setRequest(&pRequest);

     Diag191Collector diag;
     diag.activate();
     SettlementPlanType* sp = 0;

     ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag, sp);


     const std::vector<CountrySettlementPlanInfo*>& cspInfos = _pTrx->countrySettlementPlanInfos();
     const Loc* posLoc = ValidatingCxrUtil::getPOSLoc(*_pTrx, 0);
     CountrySettlementPlanInfo* nspSP = _pTrx->dataHandle().create<CountrySettlementPlanInfo>();
     nspSP->setCountryCode(ValidatingCxrUtil::getNation(*_pTrx, posLoc));
     nspSP->setSettlementPlanTypeCode(NO_SETTLEMENTPLAN);
     const_cast<std::vector<CountrySettlementPlanInfo*>&>(cspInfos).push_back(nspSP);

     CPPUNIT_ASSERT(3 == _pTrx->countrySettlementPlanInfos().size());

     _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_IEV;
     _pTrx->getRequest()->spvCxrsCode().push_back("PG");
     _pTrx->getRequest()->spvCntyCode().push_back("US");

     std::vector<CarrierCode> marketingCarriers, operatingCarriers;
     marketingCarriers.push_back("AA");
     marketingCarriers.push_back("PG");

     operatingCarriers.push_back("AA");
     operatingCarriers.push_back("PG");

     std::string errMsg;
     Itin itin;

     SpValidatingCxrGSADataMap spGsaDataMap;
     for (const CountrySettlementPlanInfo* cspInfo : _pTrx->countrySettlementPlanInfos())
     {
       const SettlementPlanType& spType = cspInfo->getSettlementPlanTypeCode();
       ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
           *_pTrx, &diag, *cspInfo, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

       if(spType == "NSP")
         CPPUNIT_ASSERT(!v);
       else
       {
         CPPUNIT_ASSERT(v);
         CPPUNIT_ASSERT(v->validatingCarriersData().size() == 1);
       }
       spGsaDataMap[spType] = v;
     }
     itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
     CPPUNIT_ASSERT(3==itin.spValidatingCxrGsaDataMap()->size());
     for(const auto& itr : *(itin.spValidatingCxrGsaDataMap()))
     {
       if(itr.first == "BSP")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("BSP"), std::string(itr.first) );
         CPPUNIT_ASSERT(1 == itr.second->validatingCarriersData().size());
         CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(itr.second->validatingCarriersData().begin()->first));
       }
       else if(itr.first == "GTC")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("GTC"), std::string(itr.first) );
         CPPUNIT_ASSERT(1 == itr.second->validatingCarriersData().size());
         CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(itr.second->validatingCarriersData().begin()->first));
       }
       else if(itr.first == "NSP")
       {
         CPPUNIT_ASSERT_EQUAL(std::string("NSP"), std::string(itr.first) );
         CPPUNIT_ASSERT(!itr.second);
       }
     }

     diag.flushMsg();
   }
     void testGetValidatingCxrListForMultiSp_noSMV_IEV_withoutCRC()
     {
       Loc loc;
       loc.nation() = "SA";

       vcx::Pos pos( "SA", "1S" );
       setPricingRequest( pos );

       Agent ta;
       ta.cxrCode() = pos.primeHost;
       ta.agentLocation() = &loc;
       Customer tjr;
       tjr.pseudoCity() = "9YMG";
       tjr.pricingApplTag2() = 'N'; // not an ARC user
       ta.agentTJR() = &tjr;
       ta.agentTJR()->settlementPlans()="BSPGEN";
       PricingRequest pRequest;
       pRequest.ticketingAgent() = &ta;
       _pTrx->setRequest(&pRequest);

       Diag191Collector diag;
       diag.activate();
       SettlementPlanType* sp = 0;

       ValidatingCxrUtil::determineCountrySettlementPlan(*_pTrx, &diag, sp);


       const std::vector<CountrySettlementPlanInfo*>& cspInfos = _pTrx->countrySettlementPlanInfos();
       const Loc* posLoc = ValidatingCxrUtil::getPOSLoc(*_pTrx, 0);
       CountrySettlementPlanInfo* nspSP = _pTrx->dataHandle().create<CountrySettlementPlanInfo>();
       nspSP->setCountryCode(ValidatingCxrUtil::getNation(*_pTrx, posLoc));
       nspSP->setSettlementPlanTypeCode(NO_SETTLEMENTPLAN);
       const_cast<std::vector<CountrySettlementPlanInfo*>&>(cspInfos).push_back(nspSP);

       CPPUNIT_ASSERT(3 == _pTrx->countrySettlementPlanInfos().size());

       _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_IEV;
       _pTrx->getRequest()->spvCntyCode().push_back("US");

       std::vector<CarrierCode> marketingCarriers, operatingCarriers;
       marketingCarriers.push_back("AA");
       marketingCarriers.push_back("PG");

       operatingCarriers.push_back("AA");
       operatingCarriers.push_back("PG");

       std::string errMsg;
       Itin itin;

       SpValidatingCxrGSADataMap spGsaDataMap;
       for (const CountrySettlementPlanInfo* cspInfo : _pTrx->countrySettlementPlanInfos())
       {
         const SettlementPlanType& spType = cspInfo->getSettlementPlanTypeCode();
         ValidatingCxrGSAData* v = ValidatingCxrUtil::getValidatingCxrList(
             *_pTrx, &diag, *cspInfo, marketingCarriers, operatingCarriers, errMsg, marketingCarriers);

         spGsaDataMap[spType] = v;
       }
       itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
       CPPUNIT_ASSERT(3==itin.spValidatingCxrGsaDataMap()->size());
       for(const auto& itr : *(itin.spValidatingCxrGsaDataMap()))
       {
         if(itr.first == "BSP")
         {
           CPPUNIT_ASSERT_EQUAL(std::string("BSP"), std::string(itr.first) );
           CPPUNIT_ASSERT(!itr.second);
         }
         else if(itr.first == "GTC")
         {
           CPPUNIT_ASSERT_EQUAL(std::string("GTC"), std::string(itr.first) );
           CPPUNIT_ASSERT(!itr.second);
         }
         else if(itr.first == "NSP")
         {
           CPPUNIT_ASSERT_EQUAL(std::string("NSP"), std::string(itr.first) );
           CPPUNIT_ASSERT(1 == itr.second->validatingCarriersData().size());
           CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(itr.second->validatingCarriersData().begin()->first));
           CPPUNIT_ASSERT_EQUAL(std::string("US"),
               std::string(itr.second->validatingCarriersData().begin()->second.interlineValidCountries[0]));
         }
       }
       diag.flushMsg();
     }

     void testIsAddValidatingCxr_Pass()
     {
       const CarrierCode vcxr          = "JH";
       const bool        isNeutral     = false;
       vcx::ValidatingCxrData vcxrData;
       ValidatingCxrGSAData* vcxrGSAData = nullptr;

       PricingRequest pRequest;
       _pTrx->setRequest(&pRequest);

       Diag191Collector diag;
       diag.activate();

       CountrySettlementPlanInfo cspi;
       cspi.setCountryCode("US");
       cspi.setSettlementPlanTypeCode("NSP");

        _pTrx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;

        const bool bResult = ValidatingCxrUtil::isAddNSPValidatingCxr(*_pTrx,
                                                                      &diag,
                                                                      cspi,
                                                                      vcxrData,
                                                                      vcxrGSAData,
                                                                      vcxr,
                                                                      isNeutral);
       CPPUNIT_ASSERT(bResult);
       CPPUNIT_ASSERT(!vcxrGSAData->isNeutralValCxr());
       CPPUNIT_ASSERT_EQUAL(std::string("JH"), std::string(vcxrGSAData->validatingCarriersData().begin()->first));

     }

     void testIsAddValidatingCxr_Fail()
     {
       const CarrierCode vcxr          = "JH";
       const bool        isNeutral     = false;
       vcx::ValidatingCxrData vcxrData;
       ValidatingCxrGSAData* vcxrGSAData = nullptr;

       PricingRequest pRequest;
       _pTrx->setRequest(&pRequest);

       Diag191Collector diag;
       diag.activate();

       CountrySettlementPlanInfo cspi;
       cspi.setCountryCode("US");
       cspi.setSettlementPlanTypeCode("NSP");

       const bool bResult = ValidatingCxrUtil::isAddNSPValidatingCxr(*_pTrx,
                                                                     &diag,
                                                                     cspi,
                                                                     vcxrData,
                                                                     vcxrGSAData,
                                                                     vcxr,
                                                                     isNeutral);
       CPPUNIT_ASSERT(!bResult);
     }




  //} OPEN SEGMENT ERROR VALIDATING CXR

}; // class ValidatingCxrUtilTest

CPPUNIT_TEST_SUITE_REGISTRATION(ValidatingCxrUtilTest);

} // namespace tse
