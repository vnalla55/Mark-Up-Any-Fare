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

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxOverride.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/Processor/BusinessRulesProcessor.h"
#include "Taxes/Dispatcher/AtpcoTaxesDriverV2.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

namespace tse
{

class AtpcoTaxesDriverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AtpcoTaxesDriverTest);
  CPPUNIT_TEST(testUninitialized);
  CPPUNIT_TEST(testBuildRequest);
  CPPUNIT_TEST(testConvertResponse);
  CPPUNIT_TEST(testConvertDiagnosticResponse);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

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

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx.reset(new PricingTrx());

    _itin.reset(new Itin());
    _trx->itin().push_back(_itin.get());

    _pricingRequest.reset(new PricingRequest());
    _trx->setRequest(_pricingRequest.get());

    _agentLocation.reset(new Loc());
    _agentLocation->loc() = std::string("DFW");
    _agentLocation->nation() = std::string("US");

    _agent.reset(new Agent());
    _trx->getRequest()->ticketingAgent() = _agent.get();
    _trx->getRequest()->ticketingAgent()->agentLocation() = _agentLocation.get();
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = "A123";
    _trx->getRequest()->ticketingAgent()->agentCity() = "DFW";
    _trx->getRequest()->ticketingAgent()->vendorCrsCode() = "X";
    _trx->getRequest()->ticketingAgent()->cxrCode() = "XX";

    _options.reset(new PricingOptions());
    _trx->setOptions(_options.get());

    _billing.reset(new Billing());
    _trx->billing() = _billing.get();
  }

  void tearDown()
  {
    _trx.reset();
    _itin.reset();
    _pricingRequest.reset();
    _agent.reset();
    _options.reset();
    _billing.reset();
    _memHandle.clear();
  }

  void testUninitialized()
  {
    const tax::ServicesFeesMap servicesFees;
    AtpcoTaxesDriverV2 atpcoTaxesDriverV2(*_trx, servicesFees);

    CPPUNIT_ASSERT(!atpcoTaxesDriverV2._request);
    CPPUNIT_ASSERT(atpcoTaxesDriverV2._xmlRequest.empty());
  }

  void testBuildRequest()
  {
    const tax::ServicesFeesMap servicesFees;
    AtpcoTaxesDriverV2 atpcoTaxesDriverV2(*_trx, servicesFees);

    CPPUNIT_ASSERT(atpcoTaxesDriverV2.buildRequest());
    CPPUNIT_ASSERT(atpcoTaxesDriverV2._request);
  }

  void testConvertResponse()
  {
    const tax::ServicesFeesMap servicesFees;
    AtpcoTaxesDriverV2 atpcoTaxesDriverV2(*_trx, servicesFees);

    CPPUNIT_ASSERT(atpcoTaxesDriverV2.buildRequest());
    atpcoTaxesDriverV2.setServices();

    tax::Request* request = atpcoTaxesDriverV2._request.get();
    CPPUNIT_ASSERT(request);
    request->ticketingOptions().paymentCurrency() = "USD";
    CPPUNIT_ASSERT(atpcoTaxesDriverV2.processTaxes());

    createItin(atpcoTaxesDriverV2);
    createItin(atpcoTaxesDriverV2);

    CPPUNIT_ASSERT(atpcoTaxesDriverV2.convertResponse());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _trx->taxResponse().size());
  }

  void testConvertDiagnosticResponse()
  {
    _trx->diagnostic().diagnosticType() = tse::Diagnostic817;

    const tax::ServicesFeesMap servicesFees;
    AtpcoTaxesDriverV2 atpcoTaxesDriverV2(*_trx, servicesFees);

    CPPUNIT_ASSERT(atpcoTaxesDriverV2.buildRequest());
    atpcoTaxesDriverV2.setServices();

    tax::Request* request = atpcoTaxesDriverV2._request.get();
    CPPUNIT_ASSERT(request);
    request->ticketingOptions().paymentCurrency() = "USD";
    CPPUNIT_ASSERT(atpcoTaxesDriverV2.processTaxes());

    createItin(atpcoTaxesDriverV2);
    createItin(atpcoTaxesDriverV2);

    CPPUNIT_ASSERT(atpcoTaxesDriverV2.convertResponse());
    CPPUNIT_ASSERT(
        !atpcoTaxesDriverV2._processor->response()._diagnosticResponse->_messages.empty());
  }

private:
  void createItin(AtpcoTaxesDriverV2& driver)
  {
    tax::Request& request = *driver._request.get();
    request.allItins().push_back(tax::Itin());
    tax::Itin& itin = request.allItins().back();
    request.itins().push_back(&itin);
    itin.id() = request.itins().size() - 1;

    Itin* tseItin = new tse::Itin;
    FarePath* tseFarePath = new tse::FarePath;
    tax::type::CarrierCode cc{"CC"};
    const tax::ItinFarePathKey key = std::make_tuple(tseItin, tseFarePath, cc, 0);
    driver._v2TrxMappingDetails._itinFarePathMapping.push_back(key);
  }

  std::unique_ptr<PricingTrx> _trx;
  std::unique_ptr<Itin> _itin;
  std::unique_ptr<PricingRequest> _pricingRequest;
  std::unique_ptr<Agent> _agent;
  std::unique_ptr<Loc> _agentLocation;
  std::unique_ptr<PricingOptions> _options;
  std::unique_ptr<Billing> _billing;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AtpcoTaxesDriverTest);
} // namespace tse
