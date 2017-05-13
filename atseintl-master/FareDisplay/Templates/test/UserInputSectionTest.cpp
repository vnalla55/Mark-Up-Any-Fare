#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/Templates/UserInputSection.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareDisplayOptionsFactory.h"
#include "test/testdata/TestFareDisplayRequestFactory.h"

namespace tse
{

class UserInputSectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(UserInputSectionTest);
  CPPUNIT_TEST(testBuildDisplay);
  CPPUNIT_TEST_SUITE_END();

public:
protected:
  void testBuildDisplay()
  {
    std::string response;
    FareDisplayTrx trx;
    populateTrx(trx);
    UserInputSection section(trx);
    CPPUNIT_ASSERT_NO_THROW(section.buildDisplay());
    CPPUNIT_ASSERT_NO_THROW(response = trx.response().str());

    // Not testing every field, but test a sub set of each.
    this->findKeyInText("LINE NUMBER", response);
    this->findKeyInText("100", response);
    this->findKeyInText("BOOKING CODE", response);
    this->findKeyInText("Yx", response);
    this->findKeyInText("G13", response);
    this->findKeyInText("G14", response);
    this->findKeyInText("G15", response);
    this->findKeyInText("G16", response);
    this->findKeyInText("GBP", response);
    this->findKeyInText("EUR", response);
    this->findKeyInText("TAK", response);
    this->findKeyInText("AGENTDUTY", response);
    this->findKeyInText("2007-Dec-14", response);
    this->findKeyInText("2007-Dec-17", response);
    this->findKeyInText("2007-Dec-18", response);
    this->findKeyInText("BASE FARE CURRENCY", response);
    this->findKeyInText("SELLING CURRENCY", response);
    this->findKeyInText("81", response);
    this->findKeyInText("543", response);
  }

  size_t findKeyInText(std::string key, std::string& text)
  {
    size_t found = text.find(key);
    if (found == std::string::npos)
    {
      CPPUNIT_ASSERT_MESSAGE(" Unable to find " + key + " in text below\n" + text, false);
    }
    return found;
  }

  void populateTrx(FareDisplayTrx& trx)
  {
    if (trx.getRequest() == 0)
      trx.setRequest(TestFareDisplayRequestFactory::create(
          "/vobs/atseintl/test/testdata/data/fareDisplayRequest.xml"));
    if (trx.getOptions() == 0)
      trx.setOptions(TestFareDisplayOptionsFactory::create(
          "/vobs/atseintl/test/testdata/data/fareDisplayOptions.xml"));

    // Build agent
    Agent* agent = new Agent();
    // FIXME -- man need to add city to locaton
    agent->agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    agent->tvlAgencyPCC() = "ABCD";
    agent->mainTvlAgencyPCC() = "EFGH";
    agent->tvlAgencyIATA() = "KAT";
    agent->homeAgencyIATA() = "TAK";
    agent->airlineDept() = "AAAA";
    agent->agentDuty() = "AGENTDUTY";
    agent->agentFunctions() = "FUNCT";
    agent->cxrCode() = "CXR";
    agent->coHostID() = 321;
    agent->currencyCodeAgent() = "USD";
    trx.getRequest()->ticketingAgent() = agent;

    // build up request
    trx.getRequest()->displayCurrency() = "EUR";
    trx.getRequest()->alternateDisplayCurrency() = "GBP";
    trx.getRequest()->returnDate() = DateTime(2007, 12, 18, 0, 0, 0);
    trx.getRequest()->dateRangeLower() = DateTime(2007, 12, 14, 0, 0, 0);
    trx.getRequest()->dateRangeUpper() = DateTime(2007, 12, 17, 0, 0, 0);
    trx.getRequest()->preferredTravelDate() = DateTime(2007, 12, 15, 0, 0, 0);
    trx.getRequest()->bookingCode() = "Yx";
    trx.getRequest()->numberOfFareLevels() = 2;
    trx.getRequest()->fareBasisCode() = "G13";
    trx.getRequest()->ticketDesignator() = "G14";
    trx.getRequest()->inclusionCode() = "G15";
    trx.getRequest()->requestedInclusionCode() = "G16";
    trx.getRequest()->addSubLineNumber() = 543;
    trx.getRequest()->addSubPercentage() = 81;
    trx.getRequest()->carrierNotEntered() = 'N';

    // Add travel seg
    trx.travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_ORD.xml"));
    trx.travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_X_HNL.xml"));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(UserInputSectionTest);

}; // namespace
