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
#include "Common/CurrencyConversionRequest.h"
#include "Common/Global.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryOptions/AncillaryPriceModifier.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/LegacyFacades/OptionalServicesBuilder.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/TaxSpecConfigReg.h"


namespace
{

class MyDataHandle : public tse::DataHandleMock
{
  tse::TestMemHandle _memHandle;

  tse::TaxSpecConfigReg::TaxSpecConfigRegSeq*
  createSeq(const std::string& paramName, const std::string& paramValue)
  {
    tse::TaxSpecConfigReg::TaxSpecConfigRegSeq *result =
        new tse::TaxSpecConfigReg::TaxSpecConfigRegSeq();
    result->paramName() = paramName;
    result->paramValue() = paramValue;

    return result;
  }
public:
  MyDataHandle()
  {
  }

  std::vector<tse::TaxSpecConfigReg*>& getTaxSpecConfig(const tse::TaxSpecConfigName& name)
  {
    std::vector<tse::TaxSpecConfigReg*>* result = _memHandle.create<std::vector<tse::TaxSpecConfigReg*> >();

    tse::TaxSpecConfigReg *taxSpecConfig = _memHandle.create<tse::TaxSpecConfigReg>();
    taxSpecConfig->effDate() = tse::DateTime(2000, 1, 20);
    taxSpecConfig->discDate() = tse::DateTime(2030, 1, 20);
    taxSpecConfig->taxSpecConfigName() = "ATPOCINC";
    result->push_back(taxSpecConfig);

    result->front()->seqs().push_back(createSeq("NATIONS", "AU"));
    result->front()->seqs().push_back(createSeq("NATIONS", "NZ"));
    return *result;
  }

  const tse::Loc* getLoc(const tse::LocCode& locCode, const tse::DateTime& date)
  {
    tse::Loc *result = _memHandle.create<tse::Loc>();
    tse::Loc::dummyData(*result);
    return result;
  }
};

} // end of anonymous namespace

namespace tax
{

typedef std::map<const tse::TravelSeg*, std::vector<tax::InputGeo*> > TravelSegGeoItems;
typedef std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > OptionalServicesRefs;

class OCFeesBuilder
{
  tse::OCFees* _ocFees;
  tse::SubCodeInfo* _s5;
  tse::OptionalServicesInfo* _s7;
  tse::TestMemHandle& _memHandle;

  tse::SubCodeInfo* createDummyS5()
  {
    tse::SubCodeInfo* s5 = _memHandle.create<tse::SubCodeInfo>();
    s5->serviceGroup() = "BG";
    s5->serviceSubGroup() = "OC";
    s5->serviceSubTypeCode() = "03F";
    s5->carrier() = "LH";
    return s5;
  }

public:
  OCFeesBuilder(tse::TestMemHandle& memHandle) : _memHandle(memHandle)
  {
    _ocFees = _memHandle.create<tse::OCFees>();
    _s5 = createDummyS5();
    _s7 = _memHandle.create<tse::OptionalServicesInfo>();
  }

  OCFeesBuilder& setTravelSegmentStart(tse::TravelSeg* travelSeg)
  {
    _ocFees->travelStart() = travelSeg;
    return *this;
  }

  OCFeesBuilder& setTravelSegmentEnd(tse::TravelSeg* travelSeg)
  {
    _ocFees->travelEnd() = travelSeg;
    return *this;
  }

  OCFeesBuilder& setTaxInclInd(const tse::Indicator& taxInclInd)
  {
    _s7->taxInclInd() = taxInclInd;
    return *this;
  }

  OCFeesBuilder& setTaxExemptInd(const tse::Indicator& taxExemptInd)
  {
    _s7->taxExemptInd() = taxExemptInd;
    return *this;
  }

  OCFeesBuilder& setFltTktMerchInd(const tse::Indicator& fltTktMerchInd)
  {
    _s7->fltTktMerchInd() = fltTktMerchInd;
    _s5->fltTktMerchInd() = fltTktMerchInd;
    return *this;
  }

  OCFeesBuilder& setNotAvailNoChargeInd(const tse::Indicator& noAvailNoChargeInd)
  {
    _s7->notAvailNoChargeInd() = noAvailNoChargeInd;
    return *this;
  }

  tse::OCFees* build()
  {
    _ocFees->subCodeInfo() = _s5;
    _ocFees->optFee() = _s7;
    return _ocFees;
  }
};

class ServiceFeesGroupBuilder
{
  tse::ServiceFeesGroup* _serviceFeesGroup;
  tse::TestMemHandle& _memHandle;

public:
  ServiceFeesGroupBuilder(tse::TestMemHandle& memHandle) : _memHandle(memHandle)
  {
    _serviceFeesGroup = _memHandle.create<tse::ServiceFeesGroup>();
  }

  ServiceFeesGroupBuilder&
  addFarePath(tse::FarePath* farePath, std::vector<tse::OCFees*>& ocFeesVector)
  {
    _serviceFeesGroup->ocFeesMap()[farePath].insert(
        _serviceFeesGroup->ocFeesMap()[farePath].end(), ocFeesVector.begin(), ocFeesVector.end());
    return *this;
  }

  tse::ServiceFeesGroup* build() { return _serviceFeesGroup; }
};

class OptionalServicesBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalServicesBuilderTest);
  CPPUNIT_TEST(testCreateOptionalServicesBuilder);
  CPPUNIT_TEST(testBuildMappings);
  CPPUNIT_TEST(testBuildMappingsManySegement);
  CPPUNIT_TEST(testAddOCFees);
  CPPUNIT_TEST(testAddOCFees_whenOcFeesAncillaryPriceModifierIsNotSet);
  CPPUNIT_TEST(testAddOCFees_whenOcFeesQuantityIsDifferentFromDefault);
  CPPUNIT_TEST(testAddServiceFeesGroup_NO_OCFees);
  CPPUNIT_TEST(testAddServiceFeesGroup_taxInclInd_TAXINCLIND);
  CPPUNIT_TEST(testAddServiceFeesGroup_taxExemptInd_TAXEXEMPTIND);
  CPPUNIT_TEST(testAddServiceFeesGroup_fltTktMerchInd_BAGGABE_CHARGES);
  CPPUNIT_TEST(testAddServiceFeesGroup_fltTktMerchInd_BAGGABE_ALLOWANCE);
  CPPUNIT_TEST(testAddServiceFeesGroup_fltTktMerchInd_BAGGABE_EMBARGOS);
  CPPUNIT_TEST(testAddServiceFeesGroup_notAvailNoChargeInd_F);
  CPPUNIT_TEST(testAddServiceFeesGroup_notAvailNoChargeInd_G);
  CPPUNIT_TEST(testAddServiceFeesGroup);

  CPPUNIT_TEST_SUITE_END();

  typedef std::map<const tse::TravelSeg*, std::vector<tax::InputGeo*> > TravelSegGeoItems;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _inputRequest = _memHandle.create<InputRequest>();
    _itin = _memHandle.create<InputItin>();
    _items = _memHandle.create<TravelSegGeoItems>();
    _optionalServicesMapping = _memHandle.create<OptionalServicesRefs>();
    _trx = _memHandle.create<tse::PricingTrx>();
    tse::PricingRequest* request = _memHandle.create<tse::PricingRequest>();
    request->ticketingDT() = tse::DateTime(2015, 1, 20);
    _trx->setRequest(request);
    _mdh = _memHandle.create<MyDataHandle>();
    _farePath = nullptr;
  }

  void tearDown() { _memHandle.clear(); }


  tse::FarePath* makeFarePath(tse::TravelSeg* travelSeg = 0, tse::TravelSeg* travelSeg2 = 0,
                              tse::TravelSeg* travelSeg3 = 0, tse::TravelSeg* travelSeg4 = 0)
  {
    tse::FarePath* farePath = _memHandle.create<tse::FarePath>();
    tse::Itin* itin = _memHandle.create<tse::Itin>();
    farePath->itin() = itin;
    if (travelSeg)
      itin->travelSeg().push_back(travelSeg);
    if (travelSeg2)
      itin->travelSeg().push_back(travelSeg2);
    if (travelSeg3)
      itin->travelSeg().push_back(travelSeg3);
    if (travelSeg4)
      itin->travelSeg().push_back(travelSeg4);
    return farePath;
  }

  tax::InputGeo* createInputGeo(int id)
  {
    tax::InputGeo* inputGeo = _memHandle.create<tax::InputGeo>();
    inputGeo->_id = id;
    return inputGeo;
  }

  tse::OCFees* createOcFees()
  {
    tse::TravelSeg* travelStart = _memHandle.create<tse::AirSeg>();
    std::vector<tax::InputGeo*> inputGeos;
    inputGeos.push_back(createInputGeo(1));
    (*_items)[travelStart] = inputGeos;
    _farePath = makeFarePath(travelStart);
    tse::OCFees* ocFees = OCFeesBuilder(_memHandle).setTravelSegmentStart(travelStart)
                                                   .setTravelSegmentEnd(travelStart)
                                                   .setFltTktMerchInd(tse::FLIGHT_RELATED_SERVICE).build();
    return ocFees;
  }

  void testCreateOptionalServicesBuilder()
  {
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    tse::FarePath* farePath = makeFarePath();
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    CPPUNIT_ASSERT(_inputRequest->optionalServicePaths().size() == 1);
    CPPUNIT_ASSERT(_inputRequest->optionalServicePaths()[0]._id == 0);
    CPPUNIT_ASSERT(_itin->_optionalServicePathRefId.has_value());
    CPPUNIT_ASSERT(_itin->_optionalServicePathRefId.value() == 0);
  }

  void testBuildMappings()
  {
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    tse::TravelSeg* travelStart = _memHandle.create<tse::AirSeg>();
    std::vector<tax::InputGeo*> inputGeos;
    inputGeos.push_back(createInputGeo(1));
    inputGeos.push_back(createInputGeo(2));
    inputGeos.push_back(createInputGeo(3));
    (*_items)[travelStart] = inputGeos;
    tse::FarePath* farePath = makeFarePath(travelStart);
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    tse::OCFees* ocFees = OCFeesBuilder(_memHandle).setTravelSegmentStart(travelStart)
                                                   .setTravelSegmentEnd(travelStart).build();
    builder.buildMappings(*ocFees);

    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         builder._inputRequest.geoPathMappings().back()._mappings.size());
    CPPUNIT_ASSERT_EQUAL(
        size_t(3), builder._inputRequest.geoPathMappings().back()._mappings.back().maps().size());
  }

  void testBuildMappingsManySegement()
  {
    tse::TravelSeg* travelStart = _memHandle.create<tse::AirSeg>();
    tse::TravelSeg* seg2 = _memHandle.create<tse::AirSeg>();
    tse::TravelSeg* seg3 = _memHandle.create<tse::AirSeg>();
    tse::TravelSeg* seg4 = _memHandle.create<tse::AirSeg>();
    std::vector<tax::InputGeo*> inputGeos;
    inputGeos.push_back(createInputGeo(1));
    inputGeos.push_back(createInputGeo(2));
    inputGeos.push_back(createInputGeo(3));
    (*_items)[travelStart] = inputGeos;
    inputGeos.clear();
    inputGeos.push_back(createInputGeo(4));
    inputGeos.push_back(createInputGeo(5));
    (*_items)[seg2] = inputGeos;
    inputGeos.clear();
    inputGeos.push_back(createInputGeo(6));
    inputGeos.push_back(createInputGeo(7));
    (*_items)[seg3] = inputGeos;
    inputGeos.clear();
    inputGeos.push_back(createInputGeo(8));
    inputGeos.push_back(createInputGeo(9));
    (*_items)[seg4] = inputGeos;

    tse::FarePath* farePath = makeFarePath(travelStart, seg2, seg3, seg4);
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    tse::OCFees* ocFees = OCFeesBuilder(_memHandle).setTravelSegmentStart(travelStart)
                                                   .setTravelSegmentEnd(seg3).build();
    builder.buildMappings(*ocFees);

    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         builder._inputRequest.geoPathMappings().back()._mappings.size());
    CPPUNIT_ASSERT_EQUAL(
        size_t(7), builder._inputRequest.geoPathMappings().back()._mappings.back().maps().size());
  }

  void testAddOCFees()
  {
    prepareTrxForServiceFeeUtil();
    tse::OCFees* ocFees = createOcFees();

    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *_farePath, *_optionalServicesMapping, ocTaxInclIndMap);

    builder.addGeoPathMappings();
    builder.buildMappings(*ocFees);
    builder.addOCFees(*ocFees);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _inputRequest->optionalServices().size());
    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._subCode == "03F");
    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._serviceGroup == "BG");
    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._serviceSubGroup == "OC");
    CPPUNIT_ASSERT_EQUAL(type::Index(0), _inputRequest->optionalServices()[0]._id);
    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         _inputRequest->optionalServicePaths()[0]._optionalServiceUsages.size());
    CPPUNIT_ASSERT_EQUAL(
        size_t(0),
        _inputRequest->optionalServicePaths()[0]._optionalServiceUsages[0]._optionalServiceRefId);
  }

  void testAddOCFees_whenOcFeesAncillaryPriceModifierIsNotSet()
  {
    prepareTrxForServiceFeeUtil();
    tse::OCFees* ocFees = createOcFees();

    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *_farePath, *_optionalServicesMapping, ocTaxInclIndMap);

    builder.addGeoPathMappings();
    builder.buildMappings(*ocFees);
    builder.addOCFees(*ocFees);

    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._quantity == 1);
  }

  void testAddOCFees_whenOcFeesQuantityIsDifferentFromDefault()
  {
    prepareTrxForServiceFeeUtil();
    tse::OCFees* ocFees = createOcFees();
    ocFees->getCurrentSeg()->_ancPriceModification = std::make_pair(tse::AncillaryIdentifier("1S|C|0DF|1.2|1000"), tse::AncillaryPriceModifier{boost::none, 5});

    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *_farePath, *_optionalServicesMapping, ocTaxInclIndMap);

    builder.addGeoPathMappings();
    builder.buildMappings(*ocFees);
    builder.addOCFees(*ocFees);

    CPPUNIT_ASSERT(_inputRequest->optionalServices()[0]._quantity == 5);
  }

  void testAddServiceFeesGroup_NO_OCFees()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();

    std::vector<tse::OCFees*> ocFeesVector;
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_taxInclInd_TAXINCLIND()
  {
    prepareTrxForServiceFeeUtil();

    _itin->_pointOfSaleRefId = 0;
    _inputRequest->pointsOfSale().push_back(new InputPointOfSale);

    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();

    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setTaxInclInd('X').build());
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_taxExemptInd_TAXEXEMPTIND()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();

    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setTaxExemptInd('Y').build());
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_fltTktMerchInd_BAGGABE_CHARGES()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setFltTktMerchInd(tse::BAGGAGE_CHARGE).build());
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_fltTktMerchInd_BAGGABE_ALLOWANCE()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setFltTktMerchInd(tse::BAGGAGE_ALLOWANCE).build());

    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_fltTktMerchInd_BAGGABE_EMBARGOS()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setFltTktMerchInd(tse::BAGGAGE_EMBARGO).build());

    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_notAvailNoChargeInd_F()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setNotAvailNoChargeInd('F').build());
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup_notAvailNoChargeInd_G()
  {
    tse::FarePath* farePath = makeFarePath();
    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(OCFeesBuilder(_memHandle).setNotAvailNoChargeInd('G').build());
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _inputRequest->optionalServices().size());
  }

  void testAddServiceFeesGroup()
  {
    prepareTrxForServiceFeeUtil();

    std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > ocTaxInclIndMap;
    tse::TravelSeg* travelStart = _memHandle.create<tse::AirSeg>();
    std::vector<tax::InputGeo*> inputGeos;
    inputGeos.push_back(createInputGeo(1));
    (*_items)[travelStart] = inputGeos;
    tse::FarePath* farePath = makeFarePath(travelStart);

    OptionalServicesBuilder builder(
        *_trx, *_inputRequest, *_itin, *_items, *farePath, *_optionalServicesMapping, ocTaxInclIndMap);
    builder.addGeoPathMappings();
    tse::OCFees* ocFees = OCFeesBuilder(_memHandle).setTravelSegmentStart(travelStart)
                                                   .setTravelSegmentEnd(travelStart).build();
    builder.buildMappings(*ocFees);

    std::vector<tse::OCFees*> ocFeesVector;
    ocFeesVector.push_back(ocFees);
    tse::ServiceFeesGroup* group =
        ServiceFeesGroupBuilder(_memHandle).addFarePath(farePath, ocFeesVector).build();
    builder.addServiceFeesGroup(*group);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _inputRequest->optionalServices().size());
  }

private:
  void prepareTrxForServiceFeeUtil()
  {
    _request = _memHandle.create<tse::PricingRequest>();
    _trx->setRequest(_request);

    _ticketingAgent = _memHandle.create<tse::Agent>();
    _ticketingAgent->currencyCodeAgent() = "USD";
    _trx->getRequest()->ticketingAgent() = _ticketingAgent;

    _pricingOptions = _memHandle.create<tse::PricingOptions>();
    _trx->setOptions(_pricingOptions);
  }

  tse::TestMemHandle _memHandle;
  InputRequest* _inputRequest;
  InputItin* _itin;
  TravelSegGeoItems* _items;
  OptionalServicesRefs* _optionalServicesMapping;
  tse::FarePath* _farePath;

  //for ServiceFeeUtil
  tse::PricingOptions* _pricingOptions;
  tse::Agent* _ticketingAgent;
  tse::PricingRequest* _request;
  tse::PricingTrx* _trx;
  tse::DataHandleMock* _mdh;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionalServicesBuilderTest);
}
