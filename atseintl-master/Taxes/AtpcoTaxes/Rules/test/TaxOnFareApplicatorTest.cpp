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
#include "Common/RoundingUtil.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/SectorDetail.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Rules/LimitGroup.h"
#include "Rules/TaxOnFareApplicator.h"
#include "Rules/TaxOnFareRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "ServiceInterfaces/RepricingService.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"
#include "Taxes/TestServer/Facades/FallbackServiceServer.h"
#include "test/PaymentDetailMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>
#include <stdexcept>

#include <gmock/gmock.h>

using namespace std;
using testing::_;
using testing::DoAll;
using testing::StrictMock;
using testing::Return;
using testing::SetArgReferee;

namespace tax
{

namespace
{

class TaxRoundingInfoServiceMock : public tax::TaxRoundingInfoService
{
public:
  MOCK_CONST_METHOD3(getTrxRoundingInfo, void(const tax::type::Nation&,
                                              tax::type::MoneyAmount&,
                                              tax::type::TaxRoundingDir&));
  MOCK_CONST_METHOD3(getNationRoundingInfo, void(const tax::type::Nation&,
                                                 tax::type::MoneyAmount&,
                                                 tax::type::TaxRoundingDir&));
  MOCK_CONST_METHOD3(getFareRoundingInfo, void(const tax::type::CurrencyCode&,
                                               tax::type::MoneyAmount&,
                                               tax::type::TaxRoundingDir&));

  void doStandardRound(tax::type::MoneyAmount& amount,
                       tax::type::MoneyAmount& unit,
                       tax::type::TaxRoundingDir& dir,
                       tax::type::MoneyAmount /* currencyUnit */,
                       bool /* isOcFee */) const override
  {
    if (dir == tax::type::TaxRoundingDir::NoRounding)
    {
      dir = tax::type::TaxRoundingDir::RoundDown;
      unit = tax::type::MoneyAmount(1, 100);
    }

    amount = doRound(amount, unit, dir, true);
  }
};

class MileageGetterMock : public MileageGetter
{
public:
  MOCK_CONST_METHOD2(getSingleDistance, type::Miles(const type::Index&, const type::Index&));
  MOCK_CONST_METHOD2(getSingleGlobalDir,
                     type::GlobalDirection(const type::Index&, const type::Index&));
};

class RepricingServiceMock : public RepricingService
{
  type::MoneyAmount _amount;

public:
  type::MoneyAmount getFareUsingUSDeductMethod(const type::Index& /*begin*/,
                                               const type::Index& /*end*/,
                                               const type::Index& /*itinId*/) const
  {
    return _amount;
  }

  type::MoneyAmount getFareFromFareList(const type::Index& /*begin*/,
                                        const type::Index& /*end*/,
                                        const type::Index& /*itinId*/) const
  {
    return _amount;
  }

  type::MoneyAmount getSimilarDomesticFare(const type::Index& /*begin*/,
                                           const type::Index& /*end*/,
                                           const type::Index& /*itinId*/,
                                           bool& fareFound) const
  {
    fareFound = (_amount != 0);
    return _amount;
  }

  type::MoneyAmount getBahamasSDOMFare(const type::Index& /*begin*/,
                                       const type::Index& /*end*/,
                                       const type::Index& /*itinId*/) const
  {
    return _amount;
  }

  void setFareAmount(type::MoneyAmount amount) { _amount = amount; }
};
}

class TaxOnFareApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxOnFareApplicatorTest);

  CPPUNIT_TEST(testAllFares);
  CPPUNIT_TEST(testFailedItinNoApplication);
  CPPUNIT_TEST(testLowestFromFareList_bothFareBreaks);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_noApplication);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_noRepricing);
  CPPUNIT_TEST(testLowestFromFareList_bothFareBreaks_Tag06);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_Tag06_fareFound);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_Tag06_noFareFound_useProration);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_Tag07_searchIfThroughInternationalFare);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_Tag07_searchIfIDADFare);
  CPPUNIT_TEST(testLowestFromFareList_notBothFareBreaks_Tag07_searchIfNetOrFreeFare);
  CPPUNIT_TEST(testFaresWithFareBreaks);
  CPPUNIT_TEST(testAllFaresNetRemit);
  CPPUNIT_TEST(testFareBeforeDiscount);
  CPPUNIT_TEST(testFaresWithSectorDetail_noRestriction);
  CPPUNIT_TEST(testFaresWithSectorDetail_restrictSectors);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_bothFareBreaks);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_notBothFareBreaks);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_notBothFareBreaks_grouping);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_bothFareBreaks_Tag03);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_notBothFareBreaks_Tag03);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_notBothFareBreaks_Tag03_rounding);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_notBothFareBreaks_Tag03_withHidden);
  CPPUNIT_TEST(testFaresWithUSDeductMethod_notBothFareBreaks_noRepricing_Tag03);
  CPPUNIT_TEST(testBlankNoApplication);
  CPPUNIT_TEST(testFullyApplicableRoundTripFare_oneWay);
  CPPUNIT_TEST(testFullyApplicableRoundTripFare_roundTrip);
  CPPUNIT_TEST(testRoundTheWorldFare);
  CPPUNIT_TEST(testRoundTheWorldFare_rounding);
  CPPUNIT_TEST(testRoundTheWorldFare_withHidden);
  CPPUNIT_TEST(testSetPartialTax);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _services.reset(new DefaultServices());
    _services->setFallbackService(new FallbackServiceServer());
    _services->setRepricingService(new RepricingServiceMock());
    _repricingService = const_cast<RepricingServiceMock*>(
        dynamic_cast<const RepricingServiceMock*>(&_services->repricingService()));
    _services->setTaxRoundingInfoService(new TaxRoundingInfoServiceMock());
    _roundingService = const_cast<TaxRoundingInfoServiceMock*>(
        dynamic_cast<const TaxRoundingInfoServiceMock*>(&_services->taxRoundingInfoService()));
    _paymentDetail.reset(new PaymentDetailMock());
    _geoPath.reset(new GeoPath());
    for (uint16_t i = 0; i < 10; ++i)
    {
      _geoPath->geos().push_back(Geo());
      _geoPath->geos()[i].id() = i;
      _geoPath->geos()[i].loc().tag() =
          (i % 2) ? type::TaxPointTag::Arrival : type::TaxPointTag::Departure;
      _geoPath->geos()[i].loc().nation() = "JP";
    }
    _geoPath->geos()[9].loc().nation() = "US";

    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[7]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[2]);
    _paymentDetail->getMutableTaxPointsProperties().resize(10);
    _paymentDetail->getMutableTaxPointsProperties()[0].isFirst = true;
    _paymentDetail->getMutableTaxPointsProperties()[0].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[9].isLast = true;
    _paymentDetail->getMutableTaxPointsProperties()[9].isFareBreak = true;

    _fares.reset(new std::vector<Fare>());
    // amounts: 10, 20, 30, 40, 50, total: 150
    // sellAmounts: 5, 15, 25, 35, 45, total: 125
    for (uint16_t i = 0; i < 5; ++i)
    {
      _fares->push_back(Fare());
      _fares->back().amount() = 10 * (i + 1);
      _fares->back().sellAmount() = _fares->back().amount() - 5;
      _fares->back().isNetRemitAvailable() = true;
    }

    _netRemitApplTag.reset(new type::NetRemitApplTag(type::NetRemitApplTag::Blank));
    _taxAppliesToTagInd.reset(new type::TaxAppliesToTagInd(type::TaxAppliesToTagInd::Blank));
    _processingApplTag.reset(new type::TaxProcessingApplTag("  "));

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    fares.push_back(2);
    fares.push_back(3);
    fares.push_back(4);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 1));
    mappings.push_back(std::make_pair(2, 3));
    mappings.push_back(std::make_pair(4, 5));
    mappings.push_back(std::make_pair(6, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);

    _sectorDetail.reset(new SectorDetail);

    _flights.reset(new std::vector<Flight>());
    _flightUsages.reset(new std::vector<FlightUsage>());
    for (type::Index i = 0; i < 5; ++i)
    {
      _flights->push_back(Flight());
      _flights->back().equipment() = "D10";
      _flightUsages->push_back(FlightUsage());
      _flightUsages->back().flight() = &_flights->at(i);
      _flightUsages->back().flightRefId() = i;
    }
    _flights->at(1).equipment() = "777";
    _flights->at(2).equipment() = "777";
    _flights->at(4).equipment() = "777";

    _mileageGetter.reset(new StrictMock<MileageGetterMock>());

    _itemNo.reset(new type::Index(0));
    _itinId.reset(new type::Index(0));
    _vendor.reset(new type::Vendor("ATP"));
  }

  void fillFarePathAndGeoPathMapping(std::vector<type::Index>& fares,
                                     std::vector<std::pair<type::Index, type::Index> >& mappings)
  {
    CPPUNIT_ASSERT_EQUAL(fares.size(), mappings.size());

    _farePath.reset(new FarePath);
    _farePath->totalAmount() = 140;
    _farePath->totalAmountBeforeDiscount() = 200;
    for (type::Index i = 0; i < fares.size(); ++i)
    {
      _farePath->fareUsages().push_back(FareUsage());
      _farePath->fareUsages().back().index() = fares[i];
    }

    _geoPathMapping.reset(new GeoPathMapping);
    for (type::Index i = 0; i < mappings.size(); ++i)
    {
      _geoPathMapping->mappings().push_back(Mapping());
      for (type::Index j = mappings[i].first; j <= mappings[i].second; ++j)
      {
        _geoPathMapping->mappings().back().maps().push_back(Map());
        _geoPathMapping->mappings().back().maps().back().index() = j;
      }
    }
  }

  void tearDown() {}

  void testAllFares()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::AllBaseFare;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(140), *_paymentDetail->getTotalFareAmount());
  }

  void testFailedItinNoApplication()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::AllBaseFare;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    _paymentDetail->getMutableItineraryDetail().setFailedRule(_rule.get());
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_bothFareBreaks()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(90), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks()
  {
    initNotBothFareBreaks();
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(35), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_noApplication()
  {
    initNotBothFareBreaks();
    _geoPath->geos()[7].loc().nation() = "PL";
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_noRepricing()
  {
    initNotBothFareBreaks();
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   false,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_bothFareBreaks_Tag06()
  {
    TaxName taxName;
    taxName.taxCode() = "KH";
    *_processingApplTag = "06";
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxableUnit(type::TaxableUnit::Itinerary);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxName(taxName);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[2]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[7]);

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(90), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_Tag06_fareFound()
  {
    TaxName taxName;
    taxName.taxCode() = "KH";
    *_processingApplTag = "06";
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxableUnit(type::TaxableUnit::Itinerary);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxName(taxName);

    _fares.reset(new std::vector<Fare>());
    for (uint16_t i = 0; i < 3; ++i)
    {
      _fares->push_back(Fare());
      _fares->back().amount() = 10 * (i + 1);
      _fares->back().sellAmount() = _fares->back().amount() - 5;
      _fares->back().isNetRemitAvailable() = true;
    }

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    fares.push_back(2);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 1));
    mappings.push_back(std::make_pair(2, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _paymentDetail->getMutableTaxPointsProperties()[1].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[2]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[3]);
    _geoPath->geos()[2].loc().nation() = "KH";
    _geoPath->geos()[3].loc().nation() = "KH";
    _geoPath->geos()[4].loc().nation() = "KH";
    _geoPath->geos()[5].loc().nation() = "CN";
    _geoPath->geos()[6].loc().nation() = "CN";
    _geoPath->geos()[7].loc().nation() = "CN";
    _repricingService->setFareAmount(33);


    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(33), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_Tag06_noFareFound_useProration()
  {
    TaxName taxName;
    taxName.taxCode() = "KH";
    *_processingApplTag = "06";
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxableUnit(type::TaxableUnit::Itinerary);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxName(taxName);

    _fares.reset(new std::vector<Fare>());
    for (uint16_t i = 0; i < 3; ++i)
    {
      _fares->push_back(Fare());
      _fares->back().amount() = 10 * (i + 1);
      _fares->back().sellAmount() = _fares->back().amount() - 5;
      _fares->back().isNetRemitAvailable() = true;
    }

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    fares.push_back(2);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 1));
    mappings.push_back(std::make_pair(2, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _paymentDetail->getMutableTaxPointsProperties()[1].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[2]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[3]);
    _geoPath->geos()[2].loc().nation() = "KH";
    _geoPath->geos()[3].loc().nation() = "KH";
    _geoPath->geos()[4].loc().nation() = "KH";
    _geoPath->geos()[5].loc().nation() = "CN";
    _geoPath->geos()[6].loc().nation() = "CN";
    _geoPath->geos()[7].loc().nation() = "CN";
    _repricingService->setFareAmount(0);
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).Times(2).WillRepeatedly(
        DoAll(SetArgReferee<1>(0), SetArgReferee<2>(tax::type::TaxRoundingDir::NoRounding)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(100));


    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(5), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_Tag07_searchIfThroughInternationalFare()
  {
    initNotBothFareBreaksTag07();

    tax::LimitGroup limits;
    limits._limitType = type::TaxApplicationLimit::OnceForItin;
    _paymentDetail->setLimitGroup(&limits);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[2]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[9]);

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(84), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_Tag07_searchIfIDADFare()
  {
    initNotBothFareBreaksTag07();

    tax::LimitGroup limits;
    limits._limitType = type::TaxApplicationLimit::OnceForItin;
    _paymentDetail->setLimitGroup(&limits);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[6]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[9]);

    (*_fares)[1].basis() = "Y1R/AD";
    (*_fares)[2].basis() = "QAF/ID";

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(34), *_paymentDetail->getTotalFareAmount());
  }

  void testLowestFromFareList_notBothFareBreaks_Tag07_searchIfNetOrFreeFare()
  {
    initNotBothFareBreaksTag07();

    tax::LimitGroup limits;
    limits._limitType = type::TaxApplicationLimit::OnceForItin;
    _paymentDetail->setLimitGroup(&limits);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointBegin(_geoPath->geos()[6]);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxPointEnd(_geoPath->geos()[9]);

    (*_fares)[1].sellAmount() = 15;
    (*_fares)[1].isNetRemitAvailable() = true;
    (*_fares)[2].amount() = 0;

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(34), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithFareBreaks()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::BetweenFareBreaks;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getTotalFareAmount());

    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getTotalFareAmount());

    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(90), *_paymentDetail->getTotalFareAmount());
  }

  void testAllFaresNetRemit()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::AllBaseFare;
    *_netRemitApplTag = type::NetRemitApplTag::Applies;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(125), *_paymentDetail->getTotalFareAmount());
  }

  void testFareBeforeDiscount()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::BeforeDiscount;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(200), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithSectorDetail_noRestriction()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::MatchingSectorDetail;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   _sectorDetail,
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(150), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithSectorDetail_restrictSectors()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::MatchingSectorDetail;
    *_itemNo = 1;
    _sectorDetail->entries.push_back(new SectorDetailEntry);
    _sectorDetail->entries.back().equipmentCode = "777";
    _sectorDetail->entries.back().applTag = type::SectorDetailAppl::Positive;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   _sectorDetail,
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100), *_paymentDetail->getTotalFareAmount());

    _sectorDetail->entries.back().applTag = type::SectorDetailAppl::Negative;
    _sectorDetail->entries.push_back(new SectorDetailEntry);
    _sectorDetail->entries.back().equipmentCode = "D10";
    _sectorDetail->entries.back().applTag = type::SectorDetailAppl::Positive;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_bothFareBreaks()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    _repricingService->setFareAmount(50);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    _paymentDetail->getMutableTaxPointsProperties()[1].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_notBothFareBreaks()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    _repricingService->setFareAmount(50);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_notBothFareBreaks_grouping()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    _repricingService->setFareAmount(50);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   false,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(140), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_bothFareBreaks_Tag03()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    *_processingApplTag = "03";
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    _paymentDetail->getMutableTaxPointsProperties()[1].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    boost::optional<type::MoneyAmount> moneyAmount = _paymentDetail->getTotalFareAmount();
    CPPUNIT_ASSERT(moneyAmount.is_initialized());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(90), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_notBothFareBreaks_Tag03()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 80;
    _fares->push_back(Fare());
    _fares->back().amount() = 10;

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).Times(2).WillRepeatedly(
        DoAll(SetArgReferee<1>(0), SetArgReferee<2>(tax::type::TaxRoundingDir::NoRounding)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(960));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(240));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(300));

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    *_processingApplTag = "03";
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(32), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_notBothFareBreaks_Tag03_rounding()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 80;
    _fares->push_back(Fare());
    _fares->back().amount() = 10;

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    //_roundingService->unit() = 10;
    //_roundingService->dir() = type::TaxRoundingDir::RoundUp;
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).Times(2).WillRepeatedly(
        DoAll(SetArgReferee<1>(10), SetArgReferee<2>(tax::type::TaxRoundingDir::RoundUp)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(960));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(240));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(300));

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    *_processingApplTag = "03";
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(40), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_notBothFareBreaks_Tag03_withHidden()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 80;
    _fares->push_back(Fare());
    _fares->back().amount() = 10;

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).Times(2).WillRepeatedly(
        DoAll(SetArgReferee<1>(0), SetArgReferee<2>(tax::type::TaxRoundingDir::NoRounding)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(960));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 5)).Times(2).WillRepeatedly(Return(300));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(240));

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    *_processingApplTag = "03";
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(144, 5), *_paymentDetail->getTotalFareAmount());
  }

  void testFaresWithUSDeductMethod_notBothFareBreaks_noRepricing_Tag03()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 80;
    _fares->push_back(Fare());
    _fares->back().amount() = 10;

    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    *_processingApplTag = "03";

    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).Times(2).WillRepeatedly(
        DoAll(SetArgReferee<1>(0), SetArgReferee<2>(tax::type::TaxRoundingDir::NoRounding)));

    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   false,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(80), *_paymentDetail->getTotalFareAmount());
  }


  void testBlankNoApplication()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::Blank;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), *_paymentDetail->getTotalFareAmount());
  }

  void testFullyApplicableRoundTripFare_oneWay()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::FullyApplicableRoundTripFare;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(280), *_paymentDetail->getTotalFareAmount());
  }

  void testFullyApplicableRoundTripFare_roundTrip()
  {
    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::FullyApplicableRoundTripFare;
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    _paymentDetail->roundTripOrOpenJaw() = true;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(140), *_paymentDetail->getTotalFareAmount());
  }

  void testRoundTheWorldFare()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 1000;

    std::vector<type::Index> fares;
    fares.push_back(0);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).WillOnce(
        DoAll(SetArgReferee<1>(0), SetArgReferee<2>(tax::type::TaxRoundingDir::NoRounding)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(8, 9)).Times(2).WillRepeatedly(Return(100));

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    _repricingService->setFareAmount(1000);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   true,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(750), *_paymentDetail->getTotalFareAmount());
  }

  void testRoundTheWorldFare_rounding()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 100;

    std::vector<type::Index> fares;
    fares.push_back(0);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).WillOnce(
        DoAll(SetArgReferee<1>(10), SetArgReferee<2>(tax::type::TaxRoundingDir::RoundUp)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(8, 9)).Times(2).WillRepeatedly(Return(100));

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    _repricingService->setFareAmount(100);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   true,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(80), *_paymentDetail->getTotalFareAmount());
  }

  void testRoundTheWorldFare_withHidden()
  {
    _fares.reset(new std::vector<Fare>());
    _fares->push_back(Fare());
    _fares->back().amount() = 1000;
    _fares->back().sellAmount() = _fares->back().amount() - 5;
    _fares->back().isNetRemitAvailable() = true;

    std::vector<type::Index> fares;
    fares.push_back(0);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _geoPath->geos()[5].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[6].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_roundingService, getFareRoundingInfo(_, _, _)).WillOnce(
        DoAll(SetArgReferee<1>(0), SetArgReferee<2>(tax::type::TaxRoundingDir::NoRounding)));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 7)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(8, 9)).Times(2).WillRepeatedly(Return(100));

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::USDeduct;
    _repricingService->setFareAmount(1000);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   true,
                                   *_mileageGetter);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getTotalFareAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(600), *_paymentDetail->getTotalFareAmount());
  }

  void testSetPartialTax()
  {
    _farePath->totalAmount() = 140;

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::BeforeDiscount;
    _farePath->totalAmountBeforeDiscount() = 150;

    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));
    TaxOnFareApplicator applicator(*_rule,
                                   *_services,
                                   std::shared_ptr<SectorDetail>(),
                                   *_fares,
                                   *_farePath,
                                   *_flights,
                                   *_flightUsages,
                                   *_geoPath,
                                   *_geoPathMapping,
                                   *_itinId,
                                   true,
                                   false,
                                   *_mileageGetter);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->exchangeDetails().isPartialTax);
  }

private:
  void initNotBothFareBreaks()
  {
    std::vector<type::Index> fares;
    fares.push_back(0);
    fares.push_back(1);
    fares.push_back(2);
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 1));
    mappings.push_back(std::make_pair(2, 5));
    mappings.push_back(std::make_pair(6, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);

    *_taxAppliesToTagInd = type::TaxAppliesToTagInd::LowestFromFareList;
    _repricingService->setFareAmount(15);
    _rule.reset(new TaxOnFareRule(*_netRemitApplTag, *_taxAppliesToTagInd, *_itemNo, *_vendor, *_processingApplTag));

    _paymentDetail->getMutableTaxPointsProperties()[1].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[5].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isFareBreak = true;
  }

  void initNotBothFareBreaksTag07()
  {
    _repricingService->setFareAmount(17);
    *_processingApplTag = "07";
    TaxName taxName;
    taxName.taxCode() = "C9";
    taxName.percentFlatTag() = type::PercentFlatTag::Percent;

    ((PaymentDetailMock*)_paymentDetail.get())->setTaxableUnit(type::TaxableUnit::Itinerary);
    ((PaymentDetailMock*)_paymentDetail.get())->setTaxName(taxName);

    _fares.reset(new std::vector<Fare>());
    for (uint16_t i = 0; i < 3; ++i)
    {
      _fares->push_back(Fare());
      _fares->back().amount() = 10 * (i + 1);
      _fares->back().isNetRemitAvailable() = false;
    }

    std::vector<type::Index> fares{0, 1, 2};
    std::vector<std::pair<type::Index, type::Index> > mappings;
    mappings.push_back(std::make_pair(0, 5));
    mappings.push_back(std::make_pair(6, 7));
    mappings.push_back(std::make_pair(8, 9));
    fillFarePathAndGeoPathMapping(fares, mappings);
    _paymentDetail->getMutableTaxPointsProperties()[0].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[5].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[8].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[9].isFareBreak = true;

    _geoPath->geos()[0].loc().nation() = "US";
    _geoPath->geos()[9].loc().nation() = "US";
    for (int i = 1; i <= 8; ++i)
      _geoPath->geos()[i].loc().nation() = "BS";
  }

  std::shared_ptr<SectorDetail> _sectorDetail;
  std::unique_ptr<PaymentDetail> _paymentDetail;
  std::unique_ptr<StrictMock<MileageGetterMock>> _mileageGetter;
  std::unique_ptr<tax::DefaultServices> _services;
  RepricingServiceMock* _repricingService;
  TaxRoundingInfoServiceMock* _roundingService;
  std::unique_ptr<std::vector<Fare>> _fares;
  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<FarePath> _farePath;
  std::unique_ptr<std::vector<Flight>> _flights;
  std::unique_ptr<std::vector<FlightUsage>> _flightUsages;
  std::unique_ptr<GeoPathMapping> _geoPathMapping;
  std::unique_ptr<type::Index> _itemNo;
  std::unique_ptr<type::Index> _itinId;
  std::unique_ptr<type::Vendor> _vendor;
  std::unique_ptr<TaxOnFareRule> _rule;
  std::unique_ptr<type::NetRemitApplTag> _netRemitApplTag;
  std::unique_ptr<type::TaxAppliesToTagInd> _taxAppliesToTagInd;
  std::unique_ptr<type::TaxProcessingApplTag> _processingApplTag;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnFareApplicatorTest);
} // namespace tax
