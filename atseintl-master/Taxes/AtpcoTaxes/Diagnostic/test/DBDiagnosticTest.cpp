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
#include "Diagnostic/DBDiagnostic.h"
#include "test/include/CppUnitHelperMacros.h"

#include <boost/lexical_cast.hpp>
#include "DataModel/Services/RulesRecord.h"
#include "ServiceInterfaces/Services.h"
#include "TestServer/Facades/RulesRecordsServiceServer.h"

#include <memory>

// TODO AK UNCOMMENT
/*
namespace tax
{

class DBDiagnosticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DBDiagnosticTest);

  CPPUNIT_TEST(testHelp);
  CPPUNIT_TEST(testNaOutput);
  CPPUNIT_TEST(testNaBrOutput_noFillerDesctiption);
  CPPUNIT_TEST(testTxOutput);
  // CPPUNIT_TEST(testRawData);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    RulesRecordsServiceServer* rr(new RulesRecordsServiceServer);
    rr->rulesRecords().push_back(new RulesRecord);
    createRecord(rr->rulesRecords().back());
    rr->updateKeys();

    _services.reset(new DefaultServices);
    _services->setRulesRecordsService(rr);

    _parameters.clear();

    type::Timestamp ticketingDate;
    _diagnostic.reset(new DBDiagnostic(*_services, _parameters, ticketingDate));
  }

  void tearDown()
  {
    _services.reset();
  }

  void testHelp()
  {
    _parameters.push_back(new Parameter);
    Parameter& parameter = _parameters.back();
    parameter.name() = "HELP";
    _diagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    CPPUNIT_ASSERT(std::string::npos != messages.find("DIAGNOSTIC 801 - DB DATA"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("HELP -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("BR -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("NANN -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("NANN/PTX -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("TXNNTCTT -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("TXNNTCTT/SQNNN -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("TBTTT/NANN -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("TBTTT/NANN/PTX -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("TBTTT/TXNNTCTT -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("TBTTT/TXNNTCTT/SQNNN -"));
  }

  void testNaOutput()
  {
    _parameters.push_back(new Parameter);
    Parameter& firstParameter = _parameters.back();
    firstParameter.name() = "NA";
    firstParameter.value() = "US";

    _parameters.push_back(new Parameter);
    Parameter& secondParameter = _parameters.back();
    secondParameter.name() = "PT";
    secondParameter.value() = "D";

    _diagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    CPPUNIT_ASSERT(std::string::npos != messages.find("DIAGNOSTIC 801 - DB DATA"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("- TAX INFO -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("NATION CODE TYPE TXPT SEQ"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("  US    PL   001   D  100"));
  }

  void testNaBrOutput_noFillerDesctiption()
  {
    _parameters.push_back(new Parameter);
    Parameter& firstParameter = _parameters.back();
    firstParameter.name() = "NA";
    firstParameter.value() = "US";

    _parameters.push_back(new Parameter);
    Parameter& secondParameter = _parameters.back();
    secondParameter.name() = "BR";

    _diagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    CPPUNIT_ASSERT(std::string::npos != messages.find("DIAGNOSTIC 801 - DB DATA"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("- TAX INFO -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("NATION CODE TYPE TXPT SEQ"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("  US    PL   001   D  100"));

    CPPUNIT_ASSERT(std::string::npos == messages.find("FILLER"));
  }

  void testTxOutput()
  {
    _parameters.push_back(new Parameter);
    Parameter& firstParameter = _parameters.back();
    firstParameter.name() = "TX";
    firstParameter.value() = "USPL001";

    _diagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    CPPUNIT_ASSERT(std::string::npos != messages.find("DIAGNOSTIC 801 - DB DATA"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("- TAX INFO -"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("NATION CODE TYPE TXPT SEQ"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("  US    PL   001   D  100"));
  }

  void testRawData()
  {
    _parameters.push_back(new Parameter);
    Parameter& firstParameter = _parameters.back();
    firstParameter.name() = "RD";
    _parameters.push_back(new Parameter);
    Parameter& secondParameter = _parameters.back();
    secondParameter.name() = "TX";
    secondParameter.value() = "USPL001";
    _parameters.push_back(new Parameter);
    Parameter& thirdParameter = _parameters.back();
    thirdParameter.name() = "SQ";
    thirdParameter.value() = "100";

    _diagnostic->createMessages(_messages);

    std::string expected[] = { "************************************************************",
                               "          ----    DIAGNOSTIC 801 - DB DATA    ----          ",
                               "************************************************************",
                               "----------------- TAX RULES RECORD RAW DATA ----------------",
                               "SABRE",
                               "US",
                               "PL",
                               "001",
                               "D",
                               "F",
                               "100",
                               "200",
                               "Y",
                               "X",
                               "U",
                               "2010-01-01",
                               "2011-02-02",
                               "2012-03-03",
                               "A",
                               "A",
                               "12321",
                               "C",
                               "KRK",
                               "P",
                               "DFW",
                               "N",
                               "US",
                               "2013-04-04 14:44",
                               "2014",
                               "5",
                               "6",
                               "2015",
                               "7",
                               "8",
                               "J",
                               "S",
                               "S01",
                               "M",
                               "EXT",
                               "A",
                               "2004-12-12",
                               "2008-11-11",
                               "2010-10-10",
                               "2012-09-09",
                               "C",
                               "B",
                               "C",
                               "D",
                               "E",
                               "F",
                               "G",
                               "H",
                               "I",
                               "E",
                               "D",
                               "N",
                               "C",
                               "X",
                               "------------------------------------------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < _messages.size(); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i], _messages[i]._content);
  }

private:
  std::unique_ptr<DBDiagnostic> _diagnostic;
  std::unique_ptr<Services> _services;
  std::unique_ptr<RulesRecordsServiceServer> _rulesRecordsServiceServer;

  boost::ptr_vector<Parameter> _parameters;
  boost::ptr_vector<Message> _messages;

  void createRecord(RulesRecord& record)
  {
    record.vendor = "SABRE";
    record.taxName.nation() = "US";
    record.taxName.taxCode() = "PL";
    record.taxName.taxType() = "001";
    record.taxName.taxPointTag() = type::TaxPointTag::Departure;
    record.taxName.percentFlatTag() = type::PercentFlatTag::Flat;
    record.seqNo = 100;
    record.taxAmt = 200;
    record.rtnToOrig = type::RtnToOrig::ReturnToOrigin;
    record.exemptTag = type::ExemptTag::Exempt;
    record.ticketedPointTag = type::TicketedPointTag::MatchTicketedAndUnticketedPoints;
    record.createDate = type::Date(2010, 1, 1);
    record.effDate = type::Date(2011, 2, 2);
    record.discDate = type::Date(2012, 3, 3);
    record.jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    record.jrnyLocZone1.type() = type::LocType::Area;
    record.jrnyLocZone1.code() = "12321";
    record.jrnyLocZone2.type() = type::LocType::City;
    record.jrnyLocZone2.code() = "KRK";
    record.trvlWhollyWithin.type() = type::LocType::Airport;
    record.trvlWhollyWithin.code() = "DFW";
    record.jrnyIncludes.type() = type::LocType::Nation;
    record.jrnyIncludes.code() = "US";
    record.expiredDate = type::Timestamp(type::Date(2013, 4, 4), type::Time(14, 44));
    record.firstTravelYear = 2014;
    record.firstTravelMonth = 5;
    record.firstTravelDay = 6;
    record.lastTravelYear = 2015;
    record.lastTravelMonth = 7;
    record.lastTravelDay = 8;
    record.travelDateTag = type::TravelDateAppTag::Journey;
    record.taxPointLocZone1.type() = type::LocType::StateProvince;
    record.taxPointLocZone1.code() = "S01";
    record.taxPointLocZone2.type() = type::LocType::Miscellaneous;
    record.taxPointLocZone2.code() = "EXT";
    record.taxPointLoc1TransferType = type::TransferTypeTag::Interline;
    record.histSaleEffDate = type::Date(2004, 12, 12);
    record.histSaleDiscDate = type::Date(2008, 11, 11);
    record.histTrvlEffDate = type::Date(2010, 10, 10);
    record.histTrvlDiscDate = type::Date(2012, 9, 9);
    record.taxPointLoc1StopoverTag = type::StopoverTag::Connection;
    record.connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    record.connectionsTags.insert(type::ConnectionsTag::FareBreak);
    record.connectionsTags.insert(type::ConnectionsTag::FurthestFareBreak);
    record.connectionsTags.insert(type::ConnectionsTag::GroundTransport);
    record.connectionsTags.insert(type::ConnectionsTag::DifferentMarketingCarrier);
    record.connectionsTags.insert(type::ConnectionsTag::Multiairport);
    record.connectionsTags.insert(type::ConnectionsTag::DomesticToInternational);
    record.connectionsTags.insert(type::ConnectionsTag::InternationalToDomestic);
    record.taxPointLoc1IntlDomInd = type::AdjacentIntlDomInd::AdjacentDomestic;
    record.taxPointLoc2IntlDomInd = type::IntlDomInd::Domestic;
    record.stopoverTimeUnit = type::StopoverTimeUnit::Minutes;
    record.stopoverTimeTag = type::StopoverTag::Connection;
    record.taxPointLoc2Compare = type::TaxPointLoc2Compare::Point;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DBDiagnosticTest);

} // namespace tse
*/
