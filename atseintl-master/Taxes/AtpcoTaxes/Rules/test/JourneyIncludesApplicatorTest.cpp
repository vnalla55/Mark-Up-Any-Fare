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
#include <fstream>
#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/LocZone.h"
#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DataModel/RequestResponse/InputRequest.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/XmlCache.h"
#include "Factories/RequestFactory.h"
#include "Rules/JourneyIncludesApplicator.h"
#include "Rules/JourneyIncludesRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "ServiceInterfaces/LocService.h"

// TODO: remove XmlParser from unit tests
#include "../TestServer/Xform/XmlParser.h"
#include "test/mainPath.h"
#include "TestServer/Xform/XmlTagsFactory.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/SelectTagsList.h"

#include <gmock/gmock.h>

using testing::_;
using testing::DoAll;
using testing::StrictMock;
using testing::Return;
using testing::SetArgReferee;

namespace tax
{

namespace
{
class LocServiceMock : public tax::LocService
{
public:
  MOCK_CONST_METHOD1(getNation, type::Nation(const type::AirportCode&));
  MOCK_CONST_METHOD1(getNationByName, type::Nation(const type::NationName&));
  MOCK_CONST_METHOD1(getNationName, type::NationName(const type::Nation&));
  MOCK_CONST_METHOD1(getCityCode, type::CityCode(const type::AirportCode&));
  MOCK_CONST_METHOD1(getAlaskaZone, type::AlaskaZone(const type::AirportCode&));
  MOCK_CONST_METHOD1(getState, type::StateProvinceCode(const type::AirportCode&));
  MOCK_CONST_METHOD1(getCurrency, type::CurrencyCode(const type::AirportCode&));
  MOCK_CONST_METHOD3(matchPassengerLocation, bool(const type::LocCode&, const LocZone&, const type::Vendor&));

  MOCK_CONST_METHOD3(isInLoc, bool(const type::AirportOrCityCode&, const LocZone&, const type::Vendor&));
};

}

class JourneyIncludesApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(JourneyIncludesApplicatorTest);

  CPPUNIT_TEST(testApplyJourneyIncludesApplicator);
  CPPUNIT_TEST(testDontApplyJourneyIncludesApplicator);
  CPPUNIT_TEST(testApplyJourneyIncludesApplicatorWhenLastApplies);

  CPPUNIT_TEST(testApplyJourneyIncludesApplicator_mustBeStopAndNotOrigDest);
  CPPUNIT_TEST(testDontApplyJourneyIncludesApplicator_mustBeStopAndNotOrigDest_notInZone);
  CPPUNIT_TEST(testDontApplyJourneyIncludesApplicator_mustBeStopAndNotOrigDest_connection);
  CPPUNIT_TEST(testApplyJourneyIncludesApplicatorWhenLastApplies_mustBeStopAndNotOrigDest);

  CPPUNIT_TEST_SUITE_END();

  TaxName* _taxName;

public:
  void setUp()
  {
    std::ifstream xmlFile((mainPath + "Rules/test/data/jrnygeospec.xml").c_str());
    std::vector<char> xmlBuf((std::istreambuf_iterator<char>(xmlFile)),
                             std::istreambuf_iterator<char>());
    xmlBuf.push_back('\0');

    _xmlTagsFactory = new XmlTagsFactory();
    _xmlTagsFactory->registerList(new NaturalXmlTagsList);

    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xmlBuf[0]);
    const XmlTagsList& xmlTagsList = selectTagsList(parsedRequest, *_xmlTagsFactory);

    tax::XmlParser().parse(_inputRequest, _xmlCache, parsedRequest, xmlTagsList);
    RequestFactory factory;
    _request = new Request();
    factory.createFromInput(_inputRequest, *_request);

    _locZone = new LocZone();
    _vendor = new type::Vendor();

    _ticketedPointTag =
      new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);

    _parent = new JourneyIncludesRule(*_ticketedPointTag, *_locZone, *_vendor, false);
    _locServiceMock = new StrictMock<LocServiceMock>();
    _seqNo = new type::SeqNo();
    _geo = new Geo();
    _taxName = new TaxName();
    _paymentDetail = new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        *_geo,
        *_geo,
        *_taxName);

    _paymentDetail->getMutableTaxPointsProperties().resize(6);
    _paymentDetail->getMutableTaxPointsProperties()[0].isFirst = true;
    _paymentDetail->getMutableTaxPointsProperties()[5].isLast = true;
  }

  void tearDown()
  {
    delete _xmlTagsFactory;
    delete _locZone;
    delete _vendor;
    delete _ticketedPointTag;
    delete _parent;
    delete _locServiceMock;
    delete _seqNo;
    delete _geo;
    delete _paymentDetail;
    delete _taxName;
    delete _request;
  }

  void testApplyJourneyIncludesApplicator()
  {
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).WillOnce(Return(true));
    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testApplyJourneyIncludesApplicatorWhenLastApplies()
  {
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).Times(5).WillRepeatedly(Return(false)).RetiresOnSaturation();
    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testDontApplyJourneyIncludesApplicator()
  {
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).Times(6).WillRepeatedly(Return(false));
    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

  void testApplyJourneyIncludesApplicator_mustBeStopAndNotOrigDest()
  {
    delete _parent;
    _parent = new JourneyIncludesRule(*_ticketedPointTag, *_locZone, *_vendor, true);

    _paymentDetail->getMutableTaxPointsProperties()[3].isTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isTimeStopover = true;
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).WillOnce(Return(true));
    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testApplyJourneyIncludesApplicatorWhenLastApplies_mustBeStopAndNotOrigDest()
  {
    delete _parent;
    _parent = new JourneyIncludesRule(*_ticketedPointTag, *_locZone, *_vendor, true);

    _paymentDetail->getMutableTaxPointsProperties()[3].isTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isTimeStopover = true;
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).Times(2).WillOnce(Return(false)).WillOnce(Return(true));
    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testDontApplyJourneyIncludesApplicator_mustBeStopAndNotOrigDest_notInZone()
  {
    delete _parent;
    _parent = new JourneyIncludesRule(*_ticketedPointTag, *_locZone, *_vendor, true);

    _paymentDetail->getMutableTaxPointsProperties()[3].isTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isTimeStopover = true;
    EXPECT_CALL(*_locServiceMock, isInLoc(_, _, _)).Times(2).WillRepeatedly(Return(false));
    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

  void testDontApplyJourneyIncludesApplicator_mustBeStopAndNotOrigDest_connection()
  {
    delete _parent;
    _parent = new JourneyIncludesRule(*_ticketedPointTag, *_locZone, *_vendor, true);

    JourneyIncludesApplicator applicator(*_parent, _request->geoPaths()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

private:
  InputRequest _inputRequest;
  XmlCache _xmlCache;
  Request* _request;
  StrictMock<LocServiceMock>* _locServiceMock;
  JourneyIncludesRule* _parent;
  LocZone* _locZone;
  type::Vendor* _vendor;
  type::SeqNo* _seqNo;
  Geo* _geo;
  PaymentDetail* _paymentDetail;
  type::TicketedPointTag* _ticketedPointTag;
  XmlTagsFactory* _xmlTagsFactory;
};

CPPUNIT_TEST_SUITE_REGISTRATION(JourneyIncludesApplicatorTest);
} // namespace tax
