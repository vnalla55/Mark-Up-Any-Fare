// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <vector>
#include <map>

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/TseEnums.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/LegacyFacades/OptionalServicesForOtaTaxReqBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tax
{

class OptionalServicesForOtaTaxReqBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalServicesForOtaTaxReqBuilderTest);
  CPPUNIT_TEST(testCreateOptionalServicesForOtaTaxReqBuilder);
  CPPUNIT_TEST(testBuildMappings);
  CPPUNIT_TEST(testAddOCFees);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _inputRequest = _memHandle.create<InputRequest>();
    _itin = _memHandle.create<InputItin>();
    _trx = _memHandle.create<tse::PricingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testCreateOptionalServicesForOtaTaxReqBuilder()
  {
    tse::FarePath* farePath = _memHandle.create<tse::FarePath>();
    OptionalServicesForOtaTaxReqBuilder builder(
        *_trx, *_inputRequest, *_itin, *farePath);
    builder.addGeoPathMappings();
    CPPUNIT_ASSERT(_inputRequest->optionalServicePaths().size() == 1);
    CPPUNIT_ASSERT(_inputRequest->optionalServicePaths()[0]._id == 0);
    CPPUNIT_ASSERT(_itin->_optionalServicePathRefId.has_value());
    CPPUNIT_ASSERT(_itin->_optionalServicePathRefId.value() == 0);
  }

  void testBuildMappings()
  {
    tse::FarePath* farePath = _memHandle.create<tse::FarePath>();
    OptionalServicesForOtaTaxReqBuilder builder(
        *_trx, *_inputRequest, *_itin, *farePath);
    builder.addGeoPathMappings();
    builder.buildMappings();

    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         builder._inputRequest.geoPathMappings().back()._mappings.size());
    CPPUNIT_ASSERT_EQUAL(
        size_t(2), builder._inputRequest.geoPathMappings().back()._mappings.back().maps().size());
  }

  void testAddOCFees()
  {;
    tse::FarePath* farePath = _memHandle.create<tse::FarePath>();
    _request = _memHandle.create<tse::PricingRequest>();
    _request->validatingCarrier() = "LH";
    _trx->setRequest(_request);
    OptionalServicesForOtaTaxReqBuilder builder(
        *_trx, *_inputRequest, *_itin, *farePath);
    builder.addGeoPathMappings();
    tse::FareUsage* tseFareUsage = _memHandle.create<tse::FareUsage>();
    tseFareUsage->paxTypeFare() = _memHandle.create<tse::PaxTypeFare>();
    tseFareUsage->paxTypeFare()->nucFareAmount() = 100.0;
    builder.addOCFees(tseFareUsage);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _inputRequest->optionalServices().size());
    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._ownerCarrier == "LH");
    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._type == type::OptionalServiceTag::FlightRelated);

    CPPUNIT_ASSERT_EQUAL(type::Index(0), _inputRequest->optionalServices()[0]._id);
    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         _inputRequest->optionalServicePaths()[0]._optionalServiceUsages.size());
    CPPUNIT_ASSERT_EQUAL(
        size_t(0),
        _inputRequest->optionalServicePaths()[0]._optionalServiceUsages[0]._optionalServiceRefId);
  }

private:

  tse::TestMemHandle _memHandle;
  InputRequest* _inputRequest;
  InputItin* _itin;
  tse::PricingRequest* _request;
  tse::PricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionalServicesForOtaTaxReqBuilderTest);
}
