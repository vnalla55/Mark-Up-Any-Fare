#include <cppunit/TestFixture.h>
#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>
#include <iostream>
#include <memory>
#include "Common/Config/ConfigMan.h"
#include "Common/DateTime.h"
#include "Common/FareMarketUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DBAccess/CarrierPreference.h"
#include "Diagnostic/Diag270Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
class Diag270CollectorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(Diag270CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamingOperator_FlagDeactivated);
  CPPUNIT_TEST(testStreamingOperator_FlagActivated);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _airSeg.reset(new AirSeg);
    _airSeg->origAirport() = "ROA";
    _airSeg->carrier() = "US";
    _airSeg->destAirport() = "CLT";

    _fareMarket.reset(new FareMarket);
    _fareMarket->travelSeg().push_back(static_cast<TravelSeg*>(_airSeg.get()));
    _fareMarket->governingCarrier() = "US";
    _fareMarket->setGlobalDirection(GlobalDirection::WH);
    _fareMarket->direction() = FMDirection::OUTBOUND;

    _carrierPref.reset(new CarrierPreference);
    _carrierPref->carrier() = "US";
    _carrierPref->ovrideFreeBagFareCompLogic() = 'N';
    _carrierPref->freebaggageexempt() = 'N';
    _carrierPref->noApplyBagExceptOnCombArea1_3() = 'N';
    _carrierPref->availabilityApplyrul2st() = 'Y';
    _carrierPref->availabilityApplyrul3st() = 'Y';
    _carrierPref->bypassosc() = 'N';
    _carrierPref->bypassrsc() = 'N';
    _carrierPref->applysamenuctort() = 'N';
    _carrierPref->applyrtevaltoterminalpt() = 'N';
    _carrierPref->noApplydrvexceptus() = 'N';
    _carrierPref->applyleastRestrStOptopu() = 'N';
    _carrierPref->applyleastRestrtrnsftopu() = 'N';
    _carrierPref->noApplycombtag1and3() = 'Y';
    _carrierPref->applysingleaddonconstr() = 'Y';
    _carrierPref->applyspecoveraddon() = 'Y';
    _carrierPref->noApplynigeriaCuradj() = 'N';
    _carrierPref->noSurfaceAtFareBreak() = 'N';
    _carrierPref->carrierbasenation() = "US";
    _carrierPref->applyPremBusCabinDiffCalc() = 'N';
    _carrierPref->applyPremEconCabinDiffCalc() = 'N';
    _carrierPref->noApplySlideToNonPremium() = 'N';
    _carrierPref->applyNormalFareOJInDiffCntrys() = 'N';
    _carrierPref->applySingleTOJBetwAreasShorterFC() = 'N';
    _carrierPref->applySingleTOJBetwAreasLongerFC() = 'N';
    _carrierPref->applySpclDOJEurope() = 'N';
    _carrierPref->applyHigherRTOJ() = 'Y';
    _carrierPref->applyHigherRT() = 'Y';
    _carrierPref->applyFBCinFC() = 'N';
    _carrierPref->description() = "US AIRWAYS";

    _carrierPref->fbrPrefs().push_back("ATP");
    _carrierPref->fbrPrefs().push_back("SABR");
    _carrierPref->fbrPrefs().push_back("SITA");
    _carrierPref->fbrPrefs().push_back("SMFA");
    _carrierPref->fbrPrefs().push_back("SMFC");

    _carrierPref->activateSoloPricing() = 'N';
    _carrierPref->activateSoloShopping() = 'N';
    _carrierPref->activateJourneyPricing() = 'N';
    _carrierPref->activateJourneyShopping() = 'N';
    _carrierPref->applyUS2TaxOnFreeTkt() = 'N';
    _carrierPref->flowMktJourneyType() = 'N';
    _carrierPref->localMktJourneyType() = 'N';

    _fareMarket->governingCarrierPref() = _carrierPref.get();

    const std::string activateDate = "2013-06-16";
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", activateDate, "PRICING_SVC");
  }

  void tearDown()
  {
    _fareMarket.reset();
    _airSeg.reset();
    _memHandle.clear();
  }

  void testConstructor()
  {
    std::unique_ptr<Diagnostic> diagroot;
    CPPUNIT_ASSERT_NO_THROW(diagroot.reset(new Diagnostic(DiagnosticNone)));

    std::unique_ptr<Diag270Collector> diag;
    CPPUNIT_ASSERT_NO_THROW(diag.reset(new Diag270Collector(*diagroot)));

    CPPUNIT_ASSERT_EQUAL(std::string(""), diag->str());
  }

  void testStreamingOperator_FlagDeactivated()
  {
    Diagnostic diagroot(Diagnostic270);
    diagroot.activate();
    Diag270Collector diag(diagroot);
    diag.enable(Diagnostic270);

    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 5, 16);
    PricingTrx trx;
    trx.setRequest(&request);
    diag.trx() = &trx;

    diag << *_fareMarket;
    CPPUNIT_ASSERT_EQUAL(getExpectedString(*_fareMarket, false), diag.str());
  }

  void testStreamingOperator_FlagActivated()
  {
    Diagnostic diagroot(Diagnostic270);
    diagroot.activate();
    Diag270Collector diag(diagroot);
    diag.enable(Diagnostic270);

    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 7, 16);
    PricingTrx trx;
    trx.setRequest(&request);
    diag.trx() = &trx;

    diag << *_fareMarket;
    CPPUNIT_ASSERT_EQUAL(getExpectedString(*_fareMarket, true), diag.str());
  }

private:
  const std::string getExpectedString(const FareMarket& fm, bool flagActivated)
  {
    const CarrierPreference& carrierPref = *fm.governingCarrierPref();
    const std::vector<VendorCode>& vendorCodes = carrierPref.fbrPrefs();

    std::ostringstream expected;
    expected << " " << std::endl << FareMarketUtil::getFullDisplayString(fm) << std::endl << " "
             << std::endl << "CARRIER PREFERENCE DIAGNOSTIC" << std::endl << std::endl
             << "GOVERNING CARRIER                          : " << carrierPref.carrier()
             << std::endl << "OVERRIDE BY SECTOR BAGGAGE                 : "
             << carrierPref.ovrideFreeBagFareCompLogic() << std::endl
             << "BAG-FREE BAGGAGE EXEMPT                    : " << carrierPref.freebaggageexempt()
             << std::endl << "BAG-FREE BAGGAGE EXEMPT TC13               : "
             << carrierPref.noApplyBagExceptOnCombArea1_3() << std::endl
             << "AVL-APPLY RULE 2                           : "
             << carrierPref.availabilityApplyrul2st() << std::endl
             << "AVL-APPLY RULE 3                           : "
             << carrierPref.availabilityApplyrul3st() << std::endl
             << "MIN-DO NOT APPLY OSC                       : " << carrierPref.bypassosc()
             << std::endl
             << "MIN-DO NOT APPLY RSC                       : " << carrierPref.bypassrsc()
             << std::endl
             << "CUR-APPLY SAME NUC TO RT FARE              : " << carrierPref.applysamenuctort()
             << std::endl;

    if (!flagActivated)
    {
      expected << "RTG-TERMINAL ON/OFF POINTS ONLY            : "
               << carrierPref.applyrtevaltoterminalpt() << std::endl;
    }

    expected
        << "RTG-DO NOT APPLY DRV ON NON-US CITIES      : " << carrierPref.noApplydrvexceptus()
        << std::endl
        << "RUL-STOPOVER-LEAST RESTRICTIVE PROVISION   : " << carrierPref.applyleastRestrStOptopu()
        << std::endl
        << "RUL-TRANSFER-LEAST RESTRICTIVE PROVISION   : " << carrierPref.applyleastRestrtrnsftopu()
        << std::endl
        << "COM-DO NOT APPLY COMB OF TAG1 AND TAG3     : " << carrierPref.noApplycombtag1and3()
        << std::endl
        << "ADD-APPLY SINGLE ADD-ON OVER DOUBLE ADD-ON : " << carrierPref.applysingleaddonconstr()
        << std::endl
        << "ADD-APPLY SPECIFIED OVER CONSTRUCTED FARE  : " << carrierPref.applyspecoveraddon()
        << std::endl
        << "CUR-DO NOT APPLY NIGERIA CURR ADJ          : " << carrierPref.noApplynigeriaCuradj()
        << std::endl
        << "COM-NO SURFACE AT FARE BREAK               : " << carrierPref.noSurfaceAtFareBreak()
        << std::endl
        << "GEO-CARRIER BASE NATION                    : " << carrierPref.carrierbasenation()
        << std::endl << "APPLY-PREM-BUS-CABIN-DIFF-CALC             : "
        << carrierPref.applyPremBusCabinDiffCalc() << std::endl
        << "APPLY-PREM-ECO-CABIN-DIFF-CALC             : "
        << carrierPref.applyPremEconCabinDiffCalc() << std::endl
        << "NO-APPLY-NON-PREMIUM-CABIN-FARE            : " << carrierPref.noApplySlideToNonPremium()
        << std::endl << "APPLY-NORMAL-FARE-OJ-IN-DIFF-CNTRYS        : "
        << carrierPref.applyNormalFareOJInDiffCntrys() << std::endl
        << "APPLY-SINGLE-TOJ-BETW-AREAS-SHORTER-FC     : "
        << carrierPref.applySingleTOJBetwAreasShorterFC() << std::endl
        << "APPLY-SINGLE-TOJ-BETW-AREAS-LONGER-FC      : "
        << carrierPref.applySingleTOJBetwAreasLongerFC() << std::endl
        << "APPLY-SPCL-DOJ-EUROPE                      : " << carrierPref.applySpclDOJEurope()
        << std::endl
        << "APPLY-HIGHEST RT OPEN JAW CHECK            : " << carrierPref.applyHigherRTOJ()
        << std::endl
        << "APPLY-HIGHEST RT CHECK                     : " << carrierPref.applyHigherRT()
        << std::endl
        << "DISPLAY-FBC                                : " << carrierPref.applyFBCinFC()
        << std::endl << "DESCRIPTION : " << carrierPref.description() << std::endl << std::endl
        << "PERMITTED FBR VENDOR                       : " << vendorCodes[0] << " "
        << vendorCodes[1] << " " << vendorCodes[2] << " " << vendorCodes[3] << " " << std::endl
        << "                                             " << vendorCodes[4] << " " << std::endl
        << "ACTIVATE SOLO PRICING                      : " << carrierPref.activateSoloPricing()
        << std::endl << std::endl
        << "ACTIVATE SOLO SHOPPING                     : " << carrierPref.activateSoloShopping()
        << std::endl << std::endl
        << "ACTIVATE JOURNEY PRICING                   : " << carrierPref.activateJourneyPricing()
        << std::endl << std::endl
        << "ACTIVATE JOURNEY SHOPPING                  : " << carrierPref.activateJourneyShopping()
        << std::endl << std::endl
        << "APPLY US2 TAX ON FREE TICKET               : " << carrierPref.applyUS2TaxOnFreeTkt()
        << std::endl << std::endl
        << "FLOW MARKET JOURNEY TYPE                   : " << carrierPref.flowMktJourneyType()
        << std::endl << std::endl
        << "LOCAL MARKET JOURNEY TYPE                  : " << carrierPref.localMktJourneyType()
        << std::endl;

    return expected.str();
  }

  TestMemHandle _memHandle;
  std::unique_ptr<AirSeg> _airSeg;
  std::unique_ptr<FareMarket> _fareMarket;
  std::unique_ptr<CarrierPreference> _carrierPref;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag270CollectorTest);
}
