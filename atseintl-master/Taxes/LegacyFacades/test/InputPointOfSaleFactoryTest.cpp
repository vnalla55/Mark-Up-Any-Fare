// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/TrxUtil.h"
#include "DBAccess/Loc.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputPointOfSale.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/LegacyFacades/InputPointOfSaleFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tax
{
class InputPointOfSaleFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(InputPointOfSaleFactoryTest);
  CPPUNIT_TEST(testCreateInputPointOfSale_SalePoint_Not_override);
  CPPUNIT_TEST(testCreateInputPointOfSale_SalePoint_override);
  CPPUNIT_TEST_SUITE_END();

private:
  ::tse::TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testCreateInputPointOfSale_SalePoint_Not_override()
  {
    tse::PricingTrx trx;

    tse::Agent* agent = _memHandle.create<tse::Agent>();
    agent->cxrCode() = "ABC";
    agent->airlineDept() = "DEF";
    agent->officeDesignator() = "GHI";
    tse::Loc agentLocation;
    tse::Loc::dummyData(agentLocation);
    agentLocation.loc() = "MNO"; // the default 8-letter LocCode is not valid airport code
    agentLocation.city() = "PQR";// the default 8-letter LocCode is not valid city code
    agent->agentLocation() = &agentLocation;

    tse::PricingRequest *request = _memHandle.create<tse::PricingRequest>();
    request->ticketingAgent() = agent;
    trx.setRequest(request);

    trx.billing() = _memHandle.create<tse::Billing>();
    trx.billing()->partitionID() = "LT";

    std::unique_ptr<InputPointOfSale> pointOfSale = InputPointOfSaleFactory::createInputPointOfSale(trx, *agent, 0);

    CPPUNIT_ASSERT(pointOfSale != 0);
    CPPUNIT_ASSERT_EQUAL(std::string(agent->agentLocation()->loc()), pointOfSale->agentCity().asString());
    CPPUNIT_ASSERT_EQUAL(std::string(agent->agentLocation()->loc()), pointOfSale->loc().asString());
    // GDS (called also CRS) code is always in agent->cxrCode()
    CPPUNIT_ASSERT_EQUAL(std::string(agent->cxrCode()), std::string(pointOfSale->vendorCrsCode()));
    CPPUNIT_ASSERT_EQUAL(trx.billing()->partitionID(), pointOfSale->carrierCode().asString());
    CPPUNIT_ASSERT_EQUAL(agent->airlineDept(), pointOfSale->agentAirlineDept());
    CPPUNIT_ASSERT_EQUAL(agent->officeDesignator(), pointOfSale->agentOfficeDesignator());
  }

  void testCreateInputPointOfSale_SalePoint_override()
  {
    tse::PricingTrx trx;

    tse::Agent* agent = _memHandle.create<tse::Agent>();
    agent->cxrCode() = "ABC";
    tse::Loc agentLocation;
    tse::Loc::dummyData(agentLocation);
    agent->agentLocation() = &agentLocation;

    tse::PricingRequest *request = _memHandle.create<tse::PricingRequest>();
    request->salePointOverride() = "AAA";

    request->ticketingAgent() = agent;
    trx.setRequest(request);
    trx.billing() = _memHandle.create<tse::Billing>();
    trx.billing()->partitionID() = "LT";

    std::unique_ptr<InputPointOfSale> pointOfSale = InputPointOfSaleFactory::createInputPointOfSale(trx, *agent, 0);

    CPPUNIT_ASSERT(pointOfSale != 0);
    CPPUNIT_ASSERT_EQUAL(tax::type::CityCode("AAA"), pointOfSale->agentCity());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("AAA"), pointOfSale->loc());
    // GDS (called also CRS) code is always in agent->cxrCode()
    CPPUNIT_ASSERT_EQUAL(std::string(agent->cxrCode()), std::string(pointOfSale->vendorCrsCode()));
    CPPUNIT_ASSERT_EQUAL(trx.billing()->partitionID(), pointOfSale->carrierCode().asString());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(InputPointOfSaleFactoryTest);
}
