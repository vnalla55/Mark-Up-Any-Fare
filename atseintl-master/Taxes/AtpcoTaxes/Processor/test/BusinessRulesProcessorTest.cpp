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

#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/GeoPathProperties.h"
#include "DataModel/Services/RulesRecord.h"
#include "DomainDataObjects/DiagnosticCommand.h"
#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "Processor/BusinessRulesProcessor.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/ExemptTagRule.h" // to create dummy rule object
#include "Rules/TaxData.h"
#include "test/PaymentDetailMock.h"
#include "test/GeoPathBuilder.h"
#include "test/ItinBuilder.h"
#include "test/RequestBuilder.h"
#include "TestServer/Facades/RulesRecordsServiceServer.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <memory>
#include <vector>

namespace tax
{

class BusinessRulesProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BusinessRulesProcessorTest);

  CPPUNIT_TEST(testParseFilterParameters);
  CPPUNIT_TEST(testMatchesFilter);/*
  CPPUNIT_TEST(testApplyDepartureTax);
  CPPUNIT_TEST(testApplyArrivalTax);
  CPPUNIT_TEST(testApplySaleTax);*/

  CPPUNIT_TEST_SUITE_END();

  class ApplyTaxesArgs
  {
    type::Nation _nation;
    Geo _taxPoint;
    Geo _nextPrevTaxPoint;
    type::Index _itinId;

    ApplyTaxesArgs()
    : _itinId(0)
    {
    }

  public:
    ApplyTaxesArgs(type::Nation newNation, type::Index newItinId)
        : _nation(newNation), _itinId(newItinId)
    {
    }

    bool check(type::Nation nation, type::Index id)
    {
      return _nation == nation &&
          _itinId == id;
    }
  };

  class BusinessRulesProcessorMock : public BusinessRulesProcessor
  {
    int _applyTaxesCounter;
    boost::ptr_vector<ApplyTaxesArgs> _argsVector;
  public:
    BusinessRulesProcessorMock(Services& services) : BusinessRulesProcessor(services),
        _applyTaxesCounter(0)
    {
    }

    virtual void applyTax(const type::ProcessingGroup,
                          const TaxValue& tax,
                          const Geo& /*taxPoint*/,
                          const Geo& /*nextPrevTaxPoint*/,
                          const type::Index itinId,
                          const GeoPathProperties& /*geoPathProperties*/,
                          Request& /*request*/,
                          RawPayments& /*RawPayments*/)
    {
      _argsVector.push_back(new ApplyTaxesArgs(tax->getTaxName().nation(), itinId));
      ++_applyTaxesCounter;
    }

    int getApplyTaxesCounter()
    {
      return _applyTaxesCounter;
    }

    bool check(int index, type::Nation nation, type::Index id)
    {
      return _argsVector[index].check(nation, id);
    }
  };

public:
  void setUp()
  {
    _services.reset(new DefaultServices());
    _services->setRulesRecordsService(&_rulesRecordsServiceServer);
    _processor.reset(new BusinessRulesProcessor(*_services));
    _rawPayments = new RawPayments();
    _command.reset(new DiagnosticCommand());

    _command->parameters().push_back(new Parameter);
    _command->parameters().back().name() = "IV";
    _command->parameters().back().value() = "ATP";
    _command->parameters().push_back(new Parameter);
    _command->parameters().back().name() = "IN";
    _command->parameters().back().value() = "DE";
    _command->parameters().push_back(new Parameter);
    _command->parameters().back().name() = "IC";
    _command->parameters().back().value() = "AA";
    _command->parameters().push_back(new Parameter);
    _command->parameters().back().name() = "IT";
    _command->parameters().back().value() = "001";
    _command->parameters().push_back(new Parameter);
    _command->parameters().back().name() = "IS";
    _command->parameters().back().value() = "101-103";
  }

  void tearDown()
  {
    delete _rawPayments;
  }

  void testParseFilterParameters()
  {
    _processor->parseFilterParameters(*_command);
    CPPUNIT_ASSERT_EQUAL(type::Vendor("ATP"), _processor->_filter.vendor);
    CPPUNIT_ASSERT_EQUAL(type::Nation("DE"), _processor->_filter.nation);
    CPPUNIT_ASSERT_EQUAL(type::TaxCode("AA"), _processor->_filter.taxCode);
    CPPUNIT_ASSERT_EQUAL(type::TaxType("001"), _processor->_filter.taxType);
    CPPUNIT_ASSERT_EQUAL(type::SeqNo(101), _processor->_filter.seq);
    CPPUNIT_ASSERT_EQUAL(type::SeqNo(103), _processor->_filter.seqLimit);
    CPPUNIT_ASSERT_EQUAL(true, _processor->_filter.isSeqRange);
  }

  void testMatchesFilter()
  {
    _processor->parseFilterParameters(*_command);

    CPPUNIT_ASSERT_EQUAL(true,
                         _processor->matchesFilter(*createTaxName("DE", "AA", "001"),
                                                   *createRulesContainer(102, "ATP")));
    CPPUNIT_ASSERT_EQUAL(false,
                         _processor->matchesFilter(*createTaxName("GB", "AA", "001"),
                                                   *createRulesContainer(102, "ATP")));
    CPPUNIT_ASSERT_EQUAL(false,
                         _processor->matchesFilter(*createTaxName("DE", "AB", "001"),
                                                   *createRulesContainer(102, "ATP")));
    CPPUNIT_ASSERT_EQUAL(false,
                         _processor->matchesFilter(*createTaxName("DE", "AA", "002"),
                                                   *createRulesContainer(102, "ATP")));
    CPPUNIT_ASSERT_EQUAL(false,
                         _processor->matchesFilter(*createTaxName("DE", "AA", "001"),
                                                   *createRulesContainer(102, "SABR")));
    CPPUNIT_ASSERT_EQUAL(false,
                         _processor->matchesFilter(*createTaxName("DE", "AA", "001"),
                                                   *createRulesContainer(100, "ATP")));
  }

  std::shared_ptr<BusinessRulesContainer>
  createRulesContainer(type::SeqNo seqNo, type::Vendor vendor)
  {
    RulesRecord rulesRec;
    rulesRec.seqNo = seqNo;
    rulesRec.vendor = vendor;
    std::shared_ptr<BusinessRulesContainer> container(
        new BusinessRulesContainer(rulesRec, type::ProcessingGroup::Itinerary));
    return container;
  }

  std::shared_ptr<TaxName>
  createTaxName(const char (&nation)[3], type::TaxCode taxCode, type::TaxType taxType)
  {
    std::shared_ptr<TaxName> taxName(new TaxName());
    taxName->nation() = type::Nation(nation);
    taxName->taxCode() = taxCode;
    taxName->taxType() = taxType;
    return taxName;
  }

  void testApplyDepartureTax()
  {
    BusinessRulesProcessorMock processor(*_services);

    RawPayments itinRawPayment;

    std::shared_ptr<Itin> itin(ItinBuilder().setGeoPathRefId(0).build());

    GeoPath* geoPath = GeoPathBuilder()
        .addGeo("ES", type::TaxPointTag::Departure)
        .addGeo("ES", type::TaxPointTag::Arrival)
        .addGeo("ES", type::TaxPointTag::Departure)
        .addGeo("SV", type::TaxPointTag::Arrival)
        .addGeo("SV", type::TaxPointTag::Departure)
        .addGeo("US", type::TaxPointTag::Arrival)
        .build();

    std::shared_ptr<Request> request(RequestBuilder().addGeoPaths(geoPath).build());

    GeoPathProperties properties;

    RulesRecord rulesRec;
    std::vector<std::shared_ptr<BusinessRulesContainer>> containers;
    containers.push_back(
        std::make_shared<BusinessRulesContainer>(rulesRec, type::ProcessingGroup::Itinerary));
    TaxName esTax;
    esTax.nation() = "ES";
    esTax.taxCode() = "ES";
    esTax.taxType() = "001";
    esTax.taxPointTag() = type::TaxPointTag::Departure;

    TaxData tax(esTax, rulesRec.vendor);
    tax.get(type::ProcessingGroup::Itinerary).swap(containers);
    processor.applyDepartureTax(
        type::ProcessingGroup::Itinerary, &tax, *itin, properties, *request, itinRawPayment);

    CPPUNIT_ASSERT_EQUAL(2, processor.getApplyTaxesCounter());
  }

  void testApplyArrivalTax()
  {
    BusinessRulesProcessorMock processor(*_services);

    RawPayments itinRawPayment;

    std::shared_ptr<Itin> itin(ItinBuilder().setGeoPathRefId(0).build());

    GeoPath* geoPath = GeoPathBuilder()
        .addGeo("PL", type::TaxPointTag::Departure)
        .addGeo("ES", type::TaxPointTag::Arrival)
        .addGeo("ES", type::TaxPointTag::Departure)
        .addGeo("ES", type::TaxPointTag::Arrival)
        .addGeo("ES", type::TaxPointTag::Departure)
        .addGeo("ES", type::TaxPointTag::Arrival)
        .build();

    std::shared_ptr<Request> request(RequestBuilder().addGeoPaths(geoPath).build());

    GeoPathProperties properties;

    RulesRecord rulesRec;
    std::vector<std::shared_ptr<BusinessRulesContainer>> containers;
    containers.push_back(
        std::make_shared<BusinessRulesContainer>(rulesRec, type::ProcessingGroup::Itinerary));
    TaxName esTax;
    esTax.nation() = "ES";
    esTax.taxCode() = "ES";
    esTax.taxType() = "001";
    esTax.taxPointTag() = type::TaxPointTag::Arrival;

    TaxData tax(esTax, rulesRec.vendor);
    tax.get(type::ProcessingGroup::Itinerary).swap(containers);
    processor.applyArrivalTax(
        type::ProcessingGroup::Itinerary, &tax, *itin, properties, *request, itinRawPayment);

    CPPUNIT_ASSERT_EQUAL(3, processor.getApplyTaxesCounter());
  }

  void testApplySaleTax()
  {
    BusinessRulesProcessorMock processor(*_services);

    std::set<type::LocCode> nationSet;

    nationSet.insert("PL");
    nationSet.insert("DE");
    nationSet.insert("RU");

    RawPayments itinRawPayment;

    std::shared_ptr<Itin> itin(ItinBuilder().setGeoPathRefId(0).build());

    GeoPath* geoPath = GeoPathBuilder()
        .addGeo("PL", type::TaxPointTag::Departure)
        .addGeo("ES", type::TaxPointTag::Arrival)
        .addGeo("ES", type::TaxPointTag::Departure)
        .addGeo("SV", type::TaxPointTag::Arrival)
        .addGeo("SV", type::TaxPointTag::Departure)
        .addGeo("US", type::TaxPointTag::Arrival)
        .build();

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addGeoPaths(geoPath)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .addTaxPoint("ES", type::TaxPointTag::Sale)
                                         .addTaxPoint("SV", type::TaxPointTag::Sale)
                                         .addTaxPoint("US", type::TaxPointTag::Sale)
                                         .build());

    GeoPathProperties properties;

    RulesRecord rulesRec;
    rulesRec.applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    std::vector<std::shared_ptr<BusinessRulesContainer>> containers;
    containers.push_back(
        std::make_shared<BusinessRulesContainer>(rulesRec, type::ProcessingGroup::Itinerary));
    TaxName esTax;
    esTax.nation() = "ES";
    esTax.taxCode() = "ES";
    esTax.taxType() = "001";
    esTax.taxPointTag() = type::TaxPointTag::Sale;

    TaxData tax(esTax, rulesRec.vendor);
    tax.get(type::ProcessingGroup::Itinerary).swap(containers);
    processor.applySaleTax(
        type::ProcessingGroup::Itinerary, &tax, *itin, properties, *request, itinRawPayment);

    CPPUNIT_ASSERT_EQUAL(1, processor.getApplyTaxesCounter());
  }

private:
  std::unique_ptr<BusinessRulesProcessor> _processor;
  std::unique_ptr<DefaultServices> _services;
  std::unique_ptr<DiagnosticCommand> _command;
  RawPayments* _rawPayments;
  RulesRecordsServiceServer _rulesRecordsServiceServer;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BusinessRulesProcessorTest);
} // namespace tax
