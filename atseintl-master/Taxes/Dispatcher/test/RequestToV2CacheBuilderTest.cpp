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

#include "test/include/CppUnitHelperMacros.h"

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include "DBAccess/DataHandle.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/XmlCache.h"
#include "Taxes/Dispatcher/RequestToV2CacheBuilder.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

namespace tse
{

class RequestToV2CacheBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RequestToV2CacheBuilderTest);
  CPPUNIT_TEST(testNoDate);
  CPPUNIT_SKIP_TEST(testBuildCache);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _builder.reset(new tax::RequestToV2CacheBuilder);
    _services.reset(new tax::DefaultServices);
    _xmlCache.reset(new tax::XmlCache);
    _request.reset(new tax::Request);
    _dataHandle.reset(new DataHandle);
  }

  void tearDown()
  {
  }

  void testNoDate()
  {
    CPPUNIT_ASSERT_THROW(_builder->buildCache(*_services, *_xmlCache, *_request, *_dataHandle),
                         boost::gregorian::bad_day_of_month);
  }

  void testBuildCache()
  {
    _request->ticketingOptions().ticketingDate() = tax::type::Date(2008, 1, 1);
    CPPUNIT_ASSERT_NO_THROW(_builder->buildCache(*_services, *_xmlCache, *_request, *_dataHandle));
    CPPUNIT_ASSERT_NO_THROW(_services->carrierFlightService());
  }

private:
  std::unique_ptr<tax::RequestToV2CacheBuilder> _builder;
  std::unique_ptr<tax::DefaultServices> _services;
  std::unique_ptr<tax::XmlCache> _xmlCache;
  std::unique_ptr<tax::Request> _request;
  std::unique_ptr<DataHandle> _dataHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RequestToV2CacheBuilderTest);
} // namespace tse
