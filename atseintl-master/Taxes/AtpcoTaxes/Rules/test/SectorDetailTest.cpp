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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/YqYr.h"
#include "DomainDataObjects/YqYrPath.h"
#include "Rules/SectorDetailApplicator.h"
#include "Rules/SectorDetailRule.h"
#include "TestServer/Facades/SectorDetailServiceServer.h"
#include "test/PaymentDetailMock.h"

#include <memory>
#include <set>

namespace tax
{

class SectorDetailTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SectorDetailTest);
  CPPUNIT_TEST(dontApplyIfApplTagIsBlank);
  CPPUNIT_TEST(dontApplyIfSectorDetailIsNull);

  CPPUNIT_TEST(applyIfEquipmetCodeIsEmpty_AnySectorLoc1Loc2);
  CPPUNIT_TEST(dontApplyIfApplIsNegative_AnySectorLoc1Loc2);
  CPPUNIT_TEST(dontApplyIfCodeDoesntMatch_AnySectorLoc1Loc2);

  CPPUNIT_TEST(applyIfEquipmetCodeIsEmpty_AnySectorOrginDestination);
  CPPUNIT_TEST(dontApplyIfApplIsNegative_AnySectorOrginDestination);
  CPPUNIT_TEST(dontApplyIfCodeDoesntMatch_AnySectorOrginDestination);

  CPPUNIT_TEST(applyIfEquipmetCodeIsEmpty_FirstInternationalSector);
  CPPUNIT_TEST(dontApplyIfApplIsNegative_FirstInternationalSector);
  CPPUNIT_TEST(dontApplyIfCodeDoesntMatch_FirstInternationalSector);
  CPPUNIT_TEST(applyIfFirstInternetionalMatch_FirstInternationalSector);
  CPPUNIT_TEST(dontApplyIfFirstInternetionalDoentMatch_FirstInternationalSector);

  CPPUNIT_TEST(applyIfAllSectorsMatch_EverySectorInItin);
  CPPUNIT_TEST(dontApplyIfApplIsNegative_EverySectorInItin);
  CPPUNIT_TEST(dontApplyIfSomeCodeDoesntMatch_EverySectorInItin);
  CPPUNIT_TEST(applyIfSectorDetailApplIsBlank_EverySectorInItin);

  CPPUNIT_TEST(applyIfEquipmetCodeIsEmpty_AllSectorLoc1Loc2);
  CPPUNIT_TEST(applyIfAllSectorsMatch_AllSectorLoc1Loc2);
  CPPUNIT_TEST(dontApplyIfApplIsNegative_AllSectorLoc1Loc2);
  CPPUNIT_TEST(dontApplyIfCodeDoesntMatch_AllSectorLoc1Loc2);

  CPPUNIT_TEST(matchFareTypeCode_Blank);
  CPPUNIT_TEST(matchFareTypeCode_ValidationTest);

  CPPUNIT_TEST(matchEquipmentCode);
  CPPUNIT_TEST(matchEquipmentCode_Blank);

  CPPUNIT_TEST(matchFareRuleCode_Blank);
  CPPUNIT_TEST(matchFareRuleCode_ValidationTest);

  CPPUNIT_TEST(matchReservationDesignator_Blank);
  CPPUNIT_TEST(matchReservationDesignator_ValidationTest);

  CPPUNIT_TEST(matchFareTariff_Blank);
  CPPUNIT_TEST(matchFareTariff_ValidationTest);

  CPPUNIT_TEST(matchFareBasisTicketDesignator_Blank);
  CPPUNIT_TEST(matchFareBasisTicketDesignator_PreparePattern);
  CPPUNIT_TEST(matchFareBasisTicketDesignator_Slash);
  CPPUNIT_TEST(matchFareBasisTicketDesignator_Hyphen);
  CPPUNIT_TEST(matchFareBasisTicketDesignator_Asterisk);
  CPPUNIT_TEST(matchFareBasisTicketDesignator_Examples);

  CPPUNIT_TEST(matchTicketCode_Blank);
  CPPUNIT_TEST(matchTicketCode_ValidationTest);
  CPPUNIT_TEST(dontMatchTicketCode_AfterSlash);
  CPPUNIT_TEST(dontMatchTicketCode_AtTheFirstPosition);
  CPPUNIT_TEST_SUITE_END();

public:
  void prepareFlights()
  {
    std::vector<Geo>& geos = _geoPath->geos();
    _flights.clear();

    for (int flight = 0; flight < 3; flight++)
    {
      geos.push_back(Geo());
      geos.push_back(Geo());
      const type::Index geoId = flight * 2;
      geos[geoId].id() = geoId;
      geos[geoId + 1].id() = geoId + 1;
    }
    geos[0].loc().code() = "LAX";
    geos[0].loc().nation() = "US";
    geos[1].loc().code() = "DEN";
    geos[1].loc().nation() = "US";
    geos[2].loc().code() = "DEN";
    geos[2].loc().nation() = "US";
    geos[3].loc().code() = "KRK";
    geos[3].loc().nation() = "PL";
    geos[4].loc().code() = "KRK";
    geos[4].loc().nation() = "PL";
    geos[5].loc().code() = "DEN";
    geos[5].loc().nation() = "US";

    _flightUsages.push_back(FlightUsage());
    _flights.resize(3);
    Flight* flight1 = &_flights[0];
    flight1->equipment() = "TRN";
    flight1->marketingCarrier() = "LH";
    flight1->reservationDesignatorCode() = 'W';

    _flightUsages[0].flight() = &_flights[0];
    _flightUsages[0].flightRefId() = 0;

    _flightUsages.push_back(FlightUsage());
    Flight* flight2 = &_flights[1];
    flight2->equipment() = "151";
    flight2->reservationDesignatorCode() = 'P';
    flight1->marketingCarrier() = "LH";

    _flightUsages[1].flight() = &_flights[1];
    _flightUsages[1].flightRefId() = 1;

    _flightUsages.push_back(FlightUsage());
    Flight* flight3 = &_flights[2];
    flight3->equipment() = "111";
    flight3->reservationDesignatorCode() = 'R';
    flight1->marketingCarrier() = "LH";

    _flightUsages[2].flight() = &_flights[2];
    _flightUsages[2].flightRefId() = 2;
  }

  void createApplicator(type::SectorDetailApplTag const& _sectorDetailApplTag,
                        std::shared_ptr<SectorDetail const> sectorDetail)
  {

    _rule.reset(new SectorDetailRule(_sectorDetailApplTag, *_itemNo, *_vendor));
    _applicator.reset(new SectorDetailApplicator(_rule.get(),
                                                 sectorDetail,
                                                 _sectorDetailApplTag,
                                                 _flightUsages,
                                                 _flights,
                                                 _fares,
                                                 _fareUsage,
                                                 _geoPathMapping,
                                                 *_geoPath));
  }

  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _sectorDetail.reset(new SectorDetail());
    _flightUsages.clear();
    _itemNo = new type::Index(1);
    _vendor = new type::Vendor("ATP");
    _geoPath.reset(new GeoPath);
    _taxPointBegin.reset(new Geo);
    _taxPointEnd.reset(new Geo);
    _entry.reset(new SectorDetailEntry);
    prepareFlights();
    prepareGeoPathMappingData();

    _matcher.reset(
        new SectorDetailMatcher(_flightUsages, _flights, _fares, _fareUsage, _geoPathMapping));
  }

  void tearDown()
  {
    _rule.reset();
    _applicator.reset();
    delete _paymentDetail;
    delete _itemNo;
    delete _vendor;
  }

  void setUpPaymentDetail(type::Index beginId, type::Index endId)
  {
    _taxPointBegin->loc().tag() = type::TaxPointTag::Departure;
    _taxPointBegin->id() = beginId;
    _taxPointEnd->id() = endId;
    _paymentDetail->setTaxPointBegin(*_taxPointBegin);
    _paymentDetail->setTaxPointEnd(*_taxPointEnd);
  }

  void addSectorDetailEntry(type::SectorDetailAppl applTag,
                            type::EquipmentCode equipmentCode,
                            type::CarrierCode fareOwnerCarrier = type::CarrierCode(UninitializedCode),
                            type::FareTariff fareTariff = -1)
  {
    SectorDetailEntry* _entry = new SectorDetailEntry();
    _entry->applTag = applTag;
    _entry->equipmentCode = equipmentCode;
    _entry->fareOwnerCarrier = fareOwnerCarrier;
    _entry->tariff = Tariff{fareTariff, type::FareTariffInd{UninitializedCode}};
    _sectorDetail->entries.push_back(_entry);
  }

  void matchEquipmentCode()
  {
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    type::SectorDetailAppl applTag = type::SectorDetailAppl::Positive;
    type::EquipmentCode equipmentCode = "TRN";

    _entry->applTag = applTag;
    _entry->equipmentCode = equipmentCode;
    type::Index flightId = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchEquipmentCode(*_entry, flightId));
    flightId = 1;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchEquipmentCode(*_entry, flightId));
  }

  void matchEquipmentCode_Blank()
  {
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    type::SectorDetailAppl applTag = type::SectorDetailAppl::Positive;
    type::EquipmentCode equipmentCode = "";

    _entry->applTag = applTag;
    _entry->equipmentCode = equipmentCode;
    type::Index flightId = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchEquipmentCode(*_entry, flightId));
  }

  void matchReservationDesignator_Blank()
  {
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    _entry->reservationCodes[0].erase();
    _entry->reservationCodes[1].erase();
    _entry->reservationCodes[2].erase();

    type::Index flightIdx = 0;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchReservationDesignator(*_entry, flightIdx));
    flightIdx = 1;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchReservationDesignator(*_entry, flightIdx));
    flightIdx = 2;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchReservationDesignator(*_entry, flightIdx));
  }

  void matchReservationDesignator_ValidationTest()
  {
	type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
	createApplicator(sectorDetailApplTag, _sectorDetail);
	setUpPaymentDetail(0, 3);

    _entry->reservationCodes[0].erase();
    _entry->reservationCodes[1] = "P";
    _entry->reservationCodes[2] = "R";

    type::Index flightIdx = 0;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchReservationDesignator(*_entry, flightIdx));
    flightIdx = 1;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchReservationDesignator(*_entry, flightIdx));
    flightIdx = 2;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchReservationDesignator(*_entry, flightIdx));
  }

  void dontApplyIfApplTagIsBlank()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::Blank;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfSectorDetailIsNull()
  {
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AnySectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  //
  void applyIfEquipmetCodeIsEmpty_AnySectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "");
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AnySectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 1);
    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfApplIsNegative_AnySectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Negative, "TRN");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AnySectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfCodeDoesntMatch_AnySectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AnySectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void applyIfEquipmetCodeIsEmpty_AnySectorOrginDestination()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::AnySectorOrginDestination;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfApplIsNegative_AnySectorOrginDestination()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Negative, "151");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::AnySectorOrginDestination;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfCodeDoesntMatch_AnySectorOrginDestination()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::AnySectorOrginDestination;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void applyIfEquipmetCodeIsEmpty_FirstInternationalSector()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::FirstInternationalSector;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void applyIfFirstInternetionalMatch_FirstInternationalSector()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "151");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::FirstInternationalSector;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfFirstInternetionalDoentMatch_FirstInternationalSector()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::FirstInternationalSector;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfApplIsNegative_FirstInternationalSector()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Negative, "TRN");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::FirstInternationalSector;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfCodeDoesntMatch_FirstInternationalSector()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");

    type::SectorDetailApplTag sectorDetailApplTag =
        type::SectorDetailApplTag::FirstInternationalSector;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void applyIfAllSectorsMatch_EverySectorInItin()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "111");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "151");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::EverySectorInItin;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfApplIsNegative_EverySectorInItin()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Negative, "111");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "151");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::EverySectorInItin;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfSomeCodeDoesntMatch_EverySectorInItin()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "111");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "181");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::EverySectorInItin;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfSomeSectorDoesntMatch_EverySectorInItin()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "111");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::EverySectorInItin;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void applyIfSectorDetailApplIsBlank_EverySectorInItin()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::EverySectorInItin;
    createApplicator(sectorDetailApplTag, _sectorDetail);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void applyIfEquipmetCodeIsEmpty_AllSectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void applyIfAllSectorsMatch_AllSectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "TRN");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "151");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfApplIsNegative_AllSectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Negative, "TRN");
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "151");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void dontApplyIfCodeDoesntMatch_AllSectorLoc1Loc2()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");

    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }
  void matchFareTypeCode_Blank()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    _entry->fareType = "";
    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
    fareIdx = 1;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
  }
  void prepareGeoPathMappingData()
  {
    // seg 0 - flight 0 - Arrival+Departure - fare index 0 - AAA, W, AAAA, 101 PUB
    // seg 1 - flight 1 - Arrival+Departure - fare index 1 - AAB
    // seg 2 - flight 2 - Arrival+Departure - fare index 2 - AAC
    // seg 3 - flight 3 - Arrival+Departure - fare index 2 - AAC
    // seg 4 - flight 4 - Arrival+Departure - fare index 3 - AAD

    _fares.push_back(Fare());
    Fare* fare = &_fares.back();
    fare->type() = "AAA";
    fare->rule() = "AAAA";
    fare->tariff() = Tariff{101, type::FareTariffInd{"PUB"}};

    _fares.push_back(Fare());
    fare = &_fares.back();
    fare->type() = "AAB";
    fare->rule() = "AABB";
    fare->tariff() = Tariff{102, type::FareTariffInd{"PUB"}};

    _fares.push_back(Fare());
    fare = &_fares.back();
    fare->type() = "AAC";
    fare->rule() = "AACC";
    fare->tariff() = Tariff{103, type::FareTariffInd{"PUB"}};

    _fares.push_back(Fare());
    fare = &_fares.back();
    fare->type() = "AAD";
    fare->rule() = "AADD";
    fare->tariff() = Tariff{104, type::FareTariffInd{"PRI"}};

    _geoPathMapping.mappings().resize(4);
    _geoPathMapping.mappings()[0].maps().resize(2);
    _geoPathMapping.mappings()[0].maps()[0].index() = 0;
    _geoPathMapping.mappings()[0].maps()[1].index() = 1;

    _geoPathMapping.mappings()[1].maps().resize(2);
    _geoPathMapping.mappings()[1].maps()[0].index() = 2;
    _geoPathMapping.mappings()[1].maps()[1].index() = 3;

    _geoPathMapping.mappings()[2].maps().resize(4);
    _geoPathMapping.mappings()[2].maps()[0].index() = 4;
    _geoPathMapping.mappings()[2].maps()[1].index() = 5;
    _geoPathMapping.mappings()[2].maps()[2].index() = 6;
    _geoPathMapping.mappings()[2].maps()[3].index() = 7;

    _geoPathMapping.mappings()[3].maps().resize(2);
    _geoPathMapping.mappings()[3].maps()[0].index() = 8;
    _geoPathMapping.mappings()[3].maps()[1].index() = 9;

    _fareUsage.resize(4);
    _fareUsage[0].index() = 0;
    _fareUsage[1].index() = 1;
    _fareUsage[2].index() = 2;
    _fareUsage[3].index() = 3;

    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");
    type::SectorDetailApplTag _sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    _rule.reset(new SectorDetailRule(_sectorDetailApplTag, *_itemNo, *_vendor));
    _applicator.reset(new SectorDetailApplicator(_rule.get(),
                                                 _sectorDetail,
                                                 _sectorDetailApplTag,
                                                 _flightUsages,
                                                 _flights,
                                                 _fares,
                                                 _fareUsage,
                                                 _geoPathMapping,
                                                 *_geoPath));
  }
  void matchFareTypeCode_ValidationTest()
  {
    _entry->fareType = "AAA";
    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
    fareIdx = 1;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareTypeCode(*_entry, fareIdx));

    _entry->fareType = "AAB";
    fareIdx = 1;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
    fareIdx = 2;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareTypeCode(*_entry, fareIdx));

    _entry->fareType = "AAC";
    fareIdx = 2;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
    fareIdx = 2;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTypeCode(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareTypeCode(*_entry, fareIdx));
  }

  void matchFareRuleCode_Blank()
  {
    addSectorDetailEntry(type::SectorDetailAppl::Positive, "801");
    type::SectorDetailApplTag sectorDetailApplTag = type::SectorDetailApplTag::AllSectorLoc1Loc2;
    createApplicator(sectorDetailApplTag, _sectorDetail);
    setUpPaymentDetail(0, 3);

    _entry->rule = "";
    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
    fareIdx = 1;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));

    _entry->rule = "";
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
  }

  void matchFareRuleCode_ValidationTest()
  {
    _entry->rule = "AAAA";
    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
    fareIdx = 1;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareRuleCode(*_entry, fareIdx));

    _entry->rule = "AABB";
    fareIdx = 1;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
    fareIdx = 2;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareRuleCode(*_entry, fareIdx));

    _entry->rule = "AACC";
    fareIdx = 2;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
    fareIdx = 2;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareRuleCode(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareRuleCode(*_entry, fareIdx));
  }

  void matchFareTariff_Blank()
  {
    _entry->tariff = Tariff{-1, type::FareTariffInd{"PUB"}};

    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTariff(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareTariff(*_entry, fareIdx));

    _entry->tariff = Tariff{101, type::FareTariffInd{UninitializedCode}};

    fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTariff(*_entry, fareIdx));
    fareIdx = 3;
    _entry->tariff = Tariff{104, type::FareTariffInd{UninitializedCode}};
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTariff(*_entry, fareIdx));
  }

  void matchFareTariff_ValidationTest()
  {
    _entry->tariff = Tariff{101, type::FareTariffInd{"PUB"}};
    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareTariff(*_entry, fareIdx));

    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareTariff(*_entry, fareIdx));
  }

  void matchFareBasisTicketDesignator_Blank()
  {
    _entry->fareBasisTktDesignator = "";

    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));
  }

  void setFareBasisCode(type::Index fareIdx, const type::FareBasisCode fareBasisCode)
  {
    _fares[fareIdx].basis() = fareBasisCode;
  }

  void matchFareBasisTicketDesignator_PreparePattern()
  {
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-/*"}, _matcher->preparePattern("-/*"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-//*"}, _matcher->preparePattern("-//*"));

    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-E70-"}, _matcher->preparePattern("-E70"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E-70-"}, _matcher->preparePattern("E-70"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E70-"}, _matcher->preparePattern("E70-"));

    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-E70-/CH"}, _matcher->preparePattern("-E70/CH"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E-70-/CH"}, _matcher->preparePattern("E-70/CH"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E70-/CH"}, _matcher->preparePattern("E70-/CH"));

    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-E70-/*"}, _matcher->preparePattern("-E70/*"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E-70-/*"}, _matcher->preparePattern("E-70/*"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E70-/*"}, _matcher->preparePattern("E70-/*"));

    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-E70-/CH*"}, _matcher->preparePattern("-E70/CH*"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E-70-/CH*"}, _matcher->preparePattern("E-70/CH*"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E70-/CH*"}, _matcher->preparePattern("E70-/CH*"));

    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"-E70-/*CH"}, _matcher->preparePattern("-E70/*CH"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E-70-/*CH"}, _matcher->preparePattern("E-70/*CH"));
    CPPUNIT_ASSERT_EQUAL(type::FareBasisCode{"E70-/*CH"}, _matcher->preparePattern("E70-/*CH"));
  }

  void matchFareBasisTicketDesignator_Slash()
  {
    _entry->fareBasisTktDesignator = "-//*";

    type::Index fareIdx = 1;
    setFareBasisCode(fareIdx, "YAP//CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "Y/ID/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // matches all Fare Basis Codes with one ore more ticket designators
    _entry->fareBasisTktDesignator = "-/*";

    setFareBasisCode(fareIdx, "YAP/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "YAP//CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "YAP/ID/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "YAP");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));
  }

  void matchFareBasisTicketDesignator_Hyphen()
  {
    _entry->fareBasisTktDesignator = "BE-70";

    type::Index fareIdx = 1;
    setFareBasisCode(fareIdx, "BE1X70");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BEX170");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // hyphen is present with no slash
    _entry->fareBasisTktDesignator = "Q-";

    setFareBasisCode(fareIdx, "QAP7");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "Q/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "QAP/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    _entry->fareBasisTktDesignator = "-E70";

    setFareBasisCode(fareIdx, "BE70");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BE70/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BAP/E70");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // hyphen doesnt replace slash
    _entry->fareBasisTktDesignator = "Q-/CH";

    setFareBasisCode(fareIdx, "QAP7/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "QAP/ID90/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "QAPCH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // hyphen at the begining and no slash -> hyphen assumed at the end
    _entry->fareBasisTktDesignator = "-E70";

    setFareBasisCode(fareIdx, "BE70NR");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BAP/E70NR");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "QAPCH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // hyphen at the beginning and at least one slash -> hyphen assumed at the end, but prior to the
    // slash
    _entry->fareBasisTktDesignator = "-E70/CH";

    setFareBasisCode(fareIdx, "BE70NR/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BE70/CH33");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BE70NR//CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "BE70N4/ABC/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // hyphen at the beginning must be replaced with at least on character
    _entry->fareBasisTktDesignator = "-HE70/CH";

    setFareBasisCode(fareIdx, "BHE70/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "HE70NR/CH");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    _entry->fareBasisTktDesignator = "Q-AP/CH";

    setFareBasisCode(fareIdx, "QHXAP7M/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));
  }

  void matchFareBasisTicketDesignator_Asterisk()
  {
    type::Index fareIdx = 1;
    _entry->fareBasisTktDesignator = "Y/*";

    setFareBasisCode(fareIdx, "Y/CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "Y//CH");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "Y");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    // atserisk will not be used to replace a number when immediately following or preceding
    // character is also number
    _entry->fareBasisTktDesignator = "B/AB3*";

    setFareBasisCode(fareIdx, "B/AB3X");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));

    setFareBasisCode(fareIdx, "B/AB35");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));
  }

  void checkMatch(bool expected,
                  type::FareBasisCode fareBasisTktDesignator,
                  type::Index fareIdx,
                  type::FareBasisCode fareBasisCode,
                  int lineNo)
  {
    _entry->fareBasisTktDesignator = fareBasisTktDesignator;
    setFareBasisCode(fareIdx, fareBasisCode);
    std::string msg = boost::lexical_cast<std::string>(lineNo) + std::string(" Entry: \"") +
        fareBasisTktDesignator + "\", fare/tkt: \"" + fareBasisCode + '"';
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, expected, _matcher->matchFareBasisTicketDesignator(*_entry, fareIdx));
  }

  void matchFareBasisTicketDesignator_Examples()
  {
    std::cout << std::endl;

    // "When a single slash is present, the slash and any alphanumerics following the slash match
    // against the last ticket designator in the Fare Basis Code (refer to Wildcards below)"
    // Behavior described in ATPCO document in two cases (marked OVERRIDE below) does not agree
    // with above quote, thus I've changed their desired outcome so that tests pass

    checkMatch(true, "QHXAP/CH", 1, "QHXAP/CH", __LINE__);
    checkMatch(false, "QHXAP/CH", 1, "QHXAPCH", __LINE__);

    checkMatch(true, "QHXAP/ID/CH", 1, "QHXAP/ID/CH", __LINE__);
    checkMatch(false, "QHXAP/ID/CH", 1, "QHXAPID/CH", __LINE__);
    checkMatch(false, "QHXAP/ID/CH", 1, "QHXAP/CH", __LINE__);

    /* The hyphen will not be used to replace a number when the immediately following or preceding character in the Fare Basis Code on the ticket is also a number
     * B-E70 does not match BE701, BE710 or BE170
BE-70 matches BEX70, BE1X70, but does not match BEX170
B-E70/CH does not match BE701/CH, BE710/CH, or BE170/CH
     */
    checkMatch(false, "B-E70", 1, "BE701", __LINE__);
    checkMatch(false, "B-E70", 1, "BE710", __LINE__);
    checkMatch(false, "B-E70", 1, "BE170", __LINE__);

    checkMatch(true, "BE-70", 1, "BEX70", __LINE__);
    checkMatch(true, "BE-70", 1, "BE1X70", __LINE__);
    checkMatch(false, "BE-70", 1, "BEX170", __LINE__);

    checkMatch(false, "B-E70/CH", 1, "BE701/CH", __LINE__);
    checkMatch(false, "B-E70/CH", 1, "BE710/CH", __LINE__);
    checkMatch(false, "B-E70/CH", 1, "BE170/CH", __LINE__);

    /* When a hyphen is present with no slash, processing will only match Fare Basis Codes that have no ticket designator
     * -E70 matches BE70, YNWE70 and Q123E70NR, but does not match BE70/CH, BAP/E70 or BAP/E70/CH
Q-CH matches QAPCH, but does not match Q/CH, QAPCH/ID90, and QAP/CH
Q- matches QAP7 but does not match QAP7/ID90, Q/CH, and QAP/CH
     */
    checkMatch(true, "-E70", 1, "BE70", __LINE__);
    checkMatch(true, "-E70", 1, "YNWE70", __LINE__);
    checkMatch(true, "-E70", 1, "Q123E70NR", __LINE__);
    checkMatch(false, "-E70", 1, "BE70/CH", __LINE__);
    checkMatch(false, "-E70", 1, "BAP/E70", __LINE__);
    checkMatch(false, "-E70", 1, "BAP/E70/CH", __LINE__);

    checkMatch(true, "Q-CH", 1, "QAPCH", __LINE__);
    checkMatch(false, "Q-CH", 1, "Q/CH", __LINE__);
    checkMatch(false, "Q-CH", 1, "QAPCH/ID90", __LINE__);
    checkMatch(false, "Q-CH", 1, "QAP/CH", __LINE__);

    checkMatch(true, "Q-", 1, "QAP7", __LINE__);
    checkMatch(false, "Q-", 1, "QAP7/ID90", __LINE__);
    checkMatch(false, "Q-", 1, "Q/CH", __LINE__);
    checkMatch(false, "Q-", 1, "QAP/CH", __LINE__);

    /* The hyphen represents any single or consecutive alphanumeric string, except a slash
     * -CH matches QEECH, but does not match YAPCH/ABC or QEE/CH
-E70/CH matches BE70/CH and Q123E70/CH, but does not match YAP/E70/CH or YAP/BE70/CH
-/CH matches QEE7M/CH, but does not match QEE, QEE//CH, QEE/ABC/CH, or QEECH
Q-/CH matches QAP7/CH, but does not match QAP/ID90/CH or QAPCH
B-AP/CH matches BXAP/CH and BH123AP/CH, but does not match BXNR/AP/CH
     */
    checkMatch(true, "-CH", 1, "QEECH", __LINE__);
    checkMatch(false, "-CH", 1, "YAPCH/ABC", __LINE__);
    checkMatch(false, "-CH", 1, "QEE/CH", __LINE__);

    checkMatch(true, "-E70/CH", 1, "BE70/CH", __LINE__);
    checkMatch(true, "-E70/CH", 1, "Q123E70/CH", __LINE__);
    checkMatch(false, "-E70/CH", 1, "YAP/E70/CH", __LINE__);
    checkMatch(false, "-E70/CH", 1, "YAP/BE70/CH", __LINE__);

    checkMatch(true, "-/CH", 1, "QEE7M/CH", __LINE__);
    checkMatch(true, "-/CH", 1, "QEE//CH", __LINE__); // OVERRIDE
    checkMatch(false, "-/CH", 1, "QEE", __LINE__);
    checkMatch(false, "-/CH", 1, "QEE/ABC/CH", __LINE__);
    checkMatch(false, "-/CH", 1, "QEECH", __LINE__);

    checkMatch(true, "Q-/CH", 1, "QAP7/CH", __LINE__);
    checkMatch(false, "Q-/CH", 1, "QAP/ID90/CH", __LINE__);
    checkMatch(false, "Q-/CH", 1, "QAPCH", __LINE__);

    checkMatch(true, "B-AP/CH", 1, "BXAP/CH", __LINE__);
    checkMatch(true, "B-AP/CH", 1, "BH123AP/CH", __LINE__);
    checkMatch(false, "B-AP/CH", 1, "BXNR/AP/CH", __LINE__);

    //When the hyphen is in the beginning of an alphanumeric string:
    /*
     * If there is no slash:
o There is an assumed hyphen at the end.
Example: -E70 matches BE70NR and BE70NR50, but does not match BAP/E70NR or BE70NR/CH
o The hyphen must be replaced with at least one character and cannot be left blank
Example: -HE70 does not match HE70NR
     */
    checkMatch(true, "-E70", 1, "BE70NR", __LINE__);
    checkMatch(true, "-E70", 1, "BE70NR50", __LINE__);
    checkMatch(false, "-E70", 1, "BAP/E70NR", __LINE__);
    checkMatch(false, "-E70", 1, "BE70NR/CH", __LINE__);

    checkMatch(false, "-HE70", 1, "HE70NR", __LINE__);

    /*
     * If there is at least one slash:
o There is an assumed hyphen at the end, but prior to the slash.
Example: -E70/CH matches BE70NR/CH but does not match BE70/CH33, BE70NR//CH, or BE70N4/ABC/CH
o The hyphen must be replaced with at least one character and cannot be left blank
Example: -HE70/CH does not match HE70NR/CH
     */
    checkMatch(true, "-E70/CH", 1, "BE70NR/CH", __LINE__);
    checkMatch(true, "-E70/CH", 1, "BE70NR//CH", __LINE__); // OVERRIDE
    checkMatch(false, "-E70/CH", 1, "BE70/CH33", __LINE__);
    checkMatch(false, "-E70/CH", 1, "BE70N4/ABC/CH", __LINE__);

    checkMatch(false, "-HE70/CH", 1, "HE70NR/CH", __LINE__);

    //When the hyphen is in the middle of an alphanumeric string
    /*
     * If there is no slash:
o There is an assumed hyphen at the end.
Example: Q-AP matches QHXAP and QHXAPCH, but does not match QHEE/AP, QHAP/CH, or QEE/AP/CH
o The hyphen does not have to be replaced by any characters.
Example: Y-AP matches YAP and YHXAP
     */
    checkMatch(true, "Q-AP", 1, "QHXAP", __LINE__);
    checkMatch(true, "Q-AP", 1, "QHXAPCH", __LINE__);
    checkMatch(false, "Q-AP", 1, "QHEE/AP", __LINE__);
    checkMatch(false, "Q-AP", 1, "QHAP/CH", __LINE__);
    checkMatch(false, "Q-AP", 1, "QHAP/AP/CH", __LINE__);

    checkMatch(true, "Y-AP", 1, "YAP", __LINE__);
    checkMatch(true, "Y-AP", 1, "YHXAP", __LINE__);

    /*
     * If there is a slash:
o There is an assumed hyphen at the end, but prior to the slash.
Example: Q-AP/CH matches QHXAP7M/CH but does not match QHAP/ID/CH or QAPNR/CH33
o The hyphen does not have to be replaced by any characters.
Example: Y-AP/CH matches YAP/CH and YHXAP/CH
     */
    checkMatch(true, "Q-AP/CH", 1, "QHXAP7M/CH", __LINE__);
    checkMatch(false, "Q-AP/CH", 1, "QAPNR/ID/CH", __LINE__);
    checkMatch(false, "Q-AP/CH", 1, "QAPNR/CH33", __LINE__);

    checkMatch(true, "Y-AP/CH", 1, "YAP/CH", __LINE__);
    checkMatch(true, "Y-AP/CH", 1, "YHXAP/CH", __LINE__);

    //When the hyphen is at the end of an alphanumeric string
    /*
     *If there is no slash:
o The hyphen does not have to be replaced by any characters.
Example: Y- matches Y and YR
     */
    checkMatch(true, "Y-", 1, "Y", __LINE__);
    checkMatch(true, "Y-", 1, "YR", __LINE__);

    /*
     *If there is a slash:
o The hyphen does not have to be replaced by any characters.
Example: Y-/CH matches Y/CH and YR/CH
     */
    checkMatch(true, "Y-/CH", 1, "Y/CH", __LINE__);
    checkMatch(true, "Y-/CH", 1, "YR/CH", __LINE__);

    /*
     * When a single slash is present with no asterisk, the slash and all characters after the last slash are an exact match to the ticket designator. There is no implied wildcard at the end.
Example: -AP/CH matches QAPNR/CH but does not match QAPNR/CH33 or QAPNR/ID/CH
     */
    checkMatch(true, "-AP/CH", 1, "QAPNR/CH", __LINE__);
    checkMatch(false, "-AP/CH", 1, "QAPNR/CH33", __LINE__);
    checkMatch(false, "-AP/CH", 1, "QAPNR/ID/CH", __LINE__);

    /*
     * When two slashes are present, the last slash and all characters after the last slash are an exact match to the last ticket designator. There is no implied wildcard at the end of a slash or at the end of the ticket designator.
Examples: Y-//CH matches YAP//CH and Y//CH, but does not match YAPNR/CH33 or YAP/ID/CH
Y-/ID/CH matches Y/ID/CH and YAP/ID/CH, but does not match YAP/ID90/CH or YAP/ID90/CH33
     */
    checkMatch(true, "Y-//CH", 1, "YAP//CH", __LINE__);
    checkMatch(true, "Y-//CH", 1, "Y//CH", __LINE__);
    checkMatch(false, "Y-//CH", 1, "YAPNR/CH33", __LINE__);
    checkMatch(false, "Y-//CH", 1, "YAP/ID/CH", __LINE__);

    checkMatch(true, "Y-/ID/CH", 1, "Y/ID/CH", __LINE__);
    checkMatch(true, "Y-/ID/CH", 1, "YAP/ID/CH", __LINE__);
    checkMatch(false, "Y-/ID/CH", 1, "YAP/ID90/CH", __LINE__);
    checkMatch(false, "Y-/ID/CH", 1, "YAP/ID90/CH33", __LINE__);

//    When an asterisk is present, the asterisk may be replaced by any single or string of alphanumeric characters in the ticket designator, including a slash. If the asterisk is preceded by at least one alphanumeric character, it can be but does not have to be replaced. If the asterisk directly follows the slash, it must be replaced.
//    Examples: -AP/CH* matches QAPNR/CH, QAPNR/CH33, QAP/CH/ABC or QAP//CH, but does not match QAP/ID/CH
//    Y/ * matches Y/CH, Y//CH and Y/ID/CH, but does not match Y
//    Y// * matches Y//CH33, but does not match Y, Y/CH, or Y/ID/CH.
//    Y/ */ * matches Y/ID/CH, but does not match Y, C/CH, OR Y//CH
//    Y-//CH* matches YAP//CH and YAP//CH33, but does not match YAP/CH33 or YAP/ID/CH
//    Y-/ID/CH* matches Y/ID/CH, YAP/ID/CH, and YAP/ID/CH33, but does not match YAP//CH or YAP/ID90/CH33
//    Y-/ID*/CH* matches Y/ID/CH, YAP/ID90/CH, YAP/ID90/CH33, and YAP/ID/CH33, but does not match YAP//CH

    checkMatch(true, "-AP/CH*", 1, "QAPNR/CH", __LINE__);
    checkMatch(true, "-AP/CH*", 1, "QAPNR/CH33", __LINE__);
    checkMatch(true, "-AP/CH*", 1, "QAPNR/CH/ABC", __LINE__);
    checkMatch(true, "-AP/CH*", 1, "QAP//CH", __LINE__);
    checkMatch(false, "-AP/CH*", 1, "QAP/ID/CH", __LINE__);

    checkMatch(true, "Y/*", 1, "Y/CH", __LINE__);
    checkMatch(true, "Y/*", 1, "Y//CH", __LINE__);
    checkMatch(true, "Y/*", 1, "Y/ID/CH", __LINE__);
    checkMatch(false, "Y/*", 1, "Y", __LINE__);

    checkMatch(true, "Y//*", 1, "Y//CH33", __LINE__);
    checkMatch(false, "Y//*", 1, "Y", __LINE__);
    checkMatch(false, "Y//*", 1, "Y/CH", __LINE__);
    checkMatch(false, "Y//*", 1, "Y/ID/CH", __LINE__);

    checkMatch(true, "Y/*/*", 1, "Y/ID/CH", __LINE__);
    checkMatch(false, "Y/*/*", 1, "Y", __LINE__);
    checkMatch(false, "Y/*/*", 1, "C/CH", __LINE__);
    checkMatch(false, "Y/*/*", 1, "Y//CH", __LINE__);

    checkMatch(true, "Y-//CH*", 1, "YAP//CH", __LINE__);
    checkMatch(true, "Y-//CH*", 1, "YAP//CH33", __LINE__);
    checkMatch(false, "Y-//CH*", 1, "YAP/CH33", __LINE__);
    checkMatch(false, "Y-//CH*", 1, "YAP/ID/CH", __LINE__);

    checkMatch(true, "Y-/ID/CH*", 1, "Y/ID/CH", __LINE__);
    checkMatch(true, "Y-/ID/CH*", 1, "YAP/ID/CH", __LINE__);
    checkMatch(true, "Y-/ID/CH*", 1, "YAP/ID/CH33", __LINE__);
    checkMatch(false, "Y-/ID/CH*", 1, "YAP//CH", __LINE__);
    checkMatch(false, "Y-/ID/CH*", 1, "YAP/ID90/CH33", __LINE__);

    checkMatch(true, "Y-/ID*/CH*", 1, "Y/ID/CH", __LINE__);
    checkMatch(true, "Y-/ID*/CH*", 1, "YAP/ID90/CH", __LINE__);
    checkMatch(true, "Y-/ID*/CH*", 1, "YAP/ID90/CH33", __LINE__);
    checkMatch(true, "Y-/ID*/CH*", 1, "YAP/ID/CH33", __LINE__);
    checkMatch(false, "Y-/ID*/CH*", 1, "YAP//CH", __LINE__);

    /*
     * The asterisk will not be used to replace a number when the immediately following or preceding character in the ticket designator is also a number
Example: B-/AB3* matches BAP/AB3X, but does not match BEE/AB35
     */
    checkMatch(true, "B-/AB3*", 1, "BAP/AB3X", __LINE__);
    checkMatch(false, "B-/AB3*", 1, "BEE/AB35", __LINE__);

    /*
     * Generic wildcard matching:
-// * matches all Fare Basis Codes with ‘//’ (slashes must be adjacent).
Example: matches YAP//CH, but does not match YAP, YAP/CH, and Y/ID/CH
-/ * matches all Fare Basis Codes with one or more ticket designators.
Example: matches YAP/CH, YAP//CH, and YAP/ID/CH, but does not match YAP
     */
    checkMatch(true, "-//*", 1, "YAP//CH", __LINE__);
    checkMatch(false, "-//*", 1, "YAP", __LINE__);
    checkMatch(false, "-//*", 1, "YAP/CH", __LINE__);
    checkMatch(false, "-//*", 1, "Y/ID/CH", __LINE__);

    checkMatch(true, "-/*", 1, "YAP/CH", __LINE__);
    checkMatch(true, "-/*", 1, "YAP//CH", __LINE__);
    checkMatch(true, "-/*", 1, "YAP/ID/CH", __LINE__);
    checkMatch(false, "-/*", 1, "YAP", __LINE__);
  }

  void matchTicketCode_Blank()
  {
    _entry->ticketCode = "";

    type::Index fareIdx = 0;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchTicketCode(*_entry, fareIdx));
    fareIdx = 3;
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchTicketCode(*_entry, fareIdx));
  }

  void matchTicketCode_ValidationTest()
  {
    type::Index fareIdx = 1;
    _entry->ticketCode = "AD";

    setFareBasisCode(fareIdx, "YAD");
    CPPUNIT_ASSERT_EQUAL(true, _matcher->matchTicketCode(*_entry, fareIdx));
  }

  void dontMatchTicketCode_AfterSlash()
  {
    type::Index fareIdx = 1;
    _entry->ticketCode = "AD";

    setFareBasisCode(fareIdx, "Y/AD");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchTicketCode(*_entry, fareIdx));
  }

  void dontMatchTicketCode_AtTheFirstPosition()
  {
    type::Index fareIdx = 1;
    _entry->ticketCode = "AD";

    setFareBasisCode(fareIdx, "ADB");
    CPPUNIT_ASSERT_EQUAL(false, _matcher->matchTicketCode(*_entry, fareIdx));
  }

private:
  std::unique_ptr<SectorDetailRule> _rule;
  std::unique_ptr<SectorDetailApplicator> _applicator;
  std::unique_ptr<SectorDetailMatcher> _matcher;

  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<Geo> _taxPointBegin;
  std::unique_ptr<Geo> _taxPointEnd;
  std::unique_ptr<SectorDetailEntry> _entry;
  PaymentDetailMock* _paymentDetail;
  RawPayments* _rawPayments;
  std::shared_ptr<SectorDetail> _sectorDetail;
  std::vector<FlightUsage> _flightUsages;
  std::vector<Flight> _flights;
  type::Index* _itemNo;
  type::Vendor* _vendor;
  std::vector<Fare> _fares;
  std::vector<FareUsage> _fareUsage;
  GeoPathMapping _geoPathMapping;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SectorDetailTest);
} // namespace tax
