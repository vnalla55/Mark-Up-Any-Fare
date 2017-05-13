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
#include "Rules/JourneyLoc1AsOriginApplicator.h"
#include "Rules/JourneyLoc1AsOriginRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "test/LocServiceMock.h"

// TODO: remove XmlParser from unit tests
#include "../TestServer/Xform/XmlParser.h"
#include "test/mainPath.h"
#include "TestServer/Xform/XmlTagsFactory.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/SelectTagsList.h"

namespace tax
{

class JourneyLoc1AsOriginApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(JourneyLoc1AsOriginApplicatorTest);

  CPPUNIT_TEST(testApplyJourneyLoc1AsOriginApplicator);
  CPPUNIT_TEST(testDontApplyJourneyLoc1AsOriginApplicator);

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

    _parent = new JourneyLoc1AsOriginRule(*_locZone, *_vendor);
    _locServiceMock = new LocServiceMock();
    _seqNo = new type::SeqNo();
    _geo = new Geo();
    _ticketedPointTag =
      new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    _taxName = new TaxName();
    _paymentDetail = new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        *_geo,
        *_geo,
        *_taxName);
  }

  void tearDown()
  {
    delete _locZone;
    delete _vendor;
    delete _parent;
    delete _locServiceMock;
    delete _seqNo;
    delete _geo;
    delete _ticketedPointTag;
    delete _paymentDetail;
    delete _taxName;
    delete _xmlTagsFactory;
    delete _request;
  }

  void testApplyJourneyLoc1AsOriginApplicator()
  {
    _locServiceMock->add(true);
    JourneyLoc1AsOriginApplicator applicator(*_parent, _request->geoPaths()[0].geos()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testDontApplyJourneyLoc1AsOriginApplicator()
  {
    _locServiceMock->add(false);
    JourneyLoc1AsOriginApplicator applicator(*_parent, _request->geoPaths()[0].geos()[0], *_locServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

private:
  InputRequest _inputRequest;
  XmlCache _xmlCache;
  Request* _request;
  LocServiceMock* _locServiceMock;
  JourneyLoc1AsOriginRule* _parent;
  LocZone* _locZone;
  type::Vendor* _vendor;
  type::SeqNo* _seqNo;
  Geo* _geo;
  PaymentDetail* _paymentDetail;
  type::TicketedPointTag* _ticketedPointTag;
  XmlTagsFactory* _xmlTagsFactory;
};

CPPUNIT_TEST_SUITE_REGISTRATION(JourneyLoc1AsOriginApplicatorTest);
} // namespace tax
