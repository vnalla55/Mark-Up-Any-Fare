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
#include "test/include/CppUnitHelperMacros.h"

#include "Taxes/AtpcoTaxes/rapidxml/rapidxml_wrapper.hpp"
#include "Taxes/TestServer/Xform/XmlWriter.h"
#include "Taxes/TestServer/Xform/NaturalXmlTagsList.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/BCHOutputResponse.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"

using namespace rapidxml;

namespace tax
{

class XmlWriterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XmlWriterTest);
  CPPUNIT_TEST(perTutDetailsDirectlyBelowTaxDetails);
  CPPUNIT_TEST(attributeTUT);
  CPPUNIT_TEST(attributesInGeoDetails);
  CPPUNIT_TEST(attributesInCalcDetails);
  CPPUNIT_TEST(taxDetailsRef);
  CPPUNIT_TEST(taxBchResponse);
  CPPUNIT_TEST_SUITE_END();

  OutputResponse buildResponseWith5TaxDetails()
  {
    OutputResponse response;
    response.itins() = OutputItins();
    response.itins()->taxDetailsSeq().push_back(std::make_shared<OutputTaxDetails>());
    response.itins()->taxDetailsSeq().push_back(std::make_shared<OutputTaxDetails>());
    response.itins()->taxDetailsSeq().push_back(std::make_shared<OutputTaxDetails>());
    response.itins()->taxDetailsSeq().push_back(std::make_shared<OutputTaxDetails>());
    response.itins()->taxDetailsSeq().push_back(std::make_shared<OutputTaxDetails>());
    response.itins()->taxDetailsSeq()[0]->id() = 1;
    response.itins()->taxDetailsSeq()[1]->id() = 2;
    response.itins()->taxDetailsSeq()[2]->id() = 3;
    response.itins()->taxDetailsSeq()[3]->id() = 4;
    response.itins()->taxDetailsSeq()[4]->id() = 5;
    return response;
  }

  BCHOutputResponse buildBCHResponseWith5Taxes()
  {
    static std::string dummyCodes[] = {"AA1", "AA2", "AA3", "AA4", "AA5"};
    static std::string dummyNames[] = {"NAME 1", "NAME 2", "NAME 3", "NAME 4", "NAME 5"};
    BCHOutputResponse response;
    response.mutableTaxDetails().push_back(new BCHOutputTaxDetail);
    response.mutableTaxDetails().push_back(new BCHOutputTaxDetail);
    response.mutableTaxDetails().push_back(new BCHOutputTaxDetail);
    response.mutableTaxDetails().push_back(new BCHOutputTaxDetail);
    response.mutableTaxDetails().push_back(new BCHOutputTaxDetail);

    for (int i = 0; i < 5; ++i)
    {
      response.mutableTaxDetails()[i].setId(i);
      response.mutableTaxDetails()[i].setPaymentAmt(101.0*(i+1)/100.0);
      response.mutableTaxDetails()[i].setSabreCode(dummyCodes[i]);
      response.mutableTaxDetails()[i].setName(dummyNames[i]);
    }

    response.mutableItins().push_back(new BCHOutputItin);
    response.mutableItins().back().setId(0);
    response.mutableItins().back().mutablePaxDetail().setPtc("ADT");
    response.mutableItins().back().mutablePaxDetail().setPtcNumber(1);
    response.mutableItins().back().mutablePaxDetail().setTotalAmount(7.07);
    response.mutableItins().back().mutablePaxDetail().mutableTaxIds() = {0, 1, 3};
    response.mutableItins().push_back(new BCHOutputItin);
    response.mutableItins().back().setId(1);
    response.mutableItins().back().mutablePaxDetail().setPtc("ADT");
    response.mutableItins().back().mutablePaxDetail().setPtcNumber(1);
    response.mutableItins().back().mutablePaxDetail().setTotalAmount(9.09);
    response.mutableItins().back().mutablePaxDetail().mutableTaxIds() = {0, 2, 4};
    return response;
  }

  template<class ResponseClass, class Tags>
  class ParsedResponse
  {
    std::string _output;
    xml_document<> _doc;

  public:
    ParsedResponse(const ResponseClass& response)
    {
      std::stringstream output;
      Tags tags;
      writeXmlResponse(output, response, tags);
      _output = output.str();
      _doc.parse<0>(&_output[0]);
    }

    xml_document<>& doc() { return _doc; }
    std::string string() const { return _output; }
  };

public:

  void perTutDetailsDirectlyBelowTaxDetails()
  {
    OutputResponse response = buildResponseWith5TaxDetails();
    response.itins()->taxDetailsSeq()[0]->taxOnYqYrDetails = OutputYQYRDetails();
    response.itins()->taxDetailsSeq()[0]->taxOnYqYrDetails->totalAmt = type::MoneyAmount(100, 1);
    response.itins()->taxDetailsSeq()[0]->taxOnTaxDetails = OutputTaxOnTaxDetails();
    response.itins()->taxDetailsSeq()[0]->taxOnTaxDetails->totalAmt = type::MoneyAmount(200, 1);
    response.itins()->taxDetailsSeq()[0]->taxOnFaresDetails = OutputFaresDetails();
    response.itins()->taxDetailsSeq()[0]->taxOnFaresDetails->totalAmt = type::MoneyAmount(300, 1);
    response.itins()->taxDetailsSeq()[0]->taxOnOcDetails = OutputOCDetails();
    response.itins()->taxDetailsSeq()[0]->taxOnOcDetails->totalAmt = type::MoneyAmount(400, 1);
    response.itins()->taxDetailsSeq()[0]->taxOnBaggageDetails = OutputBaggageDetails();
    response.itins()->taxDetailsSeq()[0]->taxOnBaggageDetails->totalAmt = type::MoneyAmount(500, 1);

    ParsedResponse<OutputResponse, NaturalXmlTagsList> parsedResponse(response);
    xml_node<>* root = parsedResponse.doc().first_node("TaxRs");
    CPPUNIT_ASSERT(root);
    xml_node<>* itins = root->first_node("Itins");
    CPPUNIT_ASSERT(itins);
    xml_node<>* taxDetails = itins->first_node("TaxDetails");
    CPPUNIT_ASSERT(taxDetails);

    xml_node<>* yqyrDetails = taxDetails->first_node("TaxOnYQYRDetails");
    CPPUNIT_ASSERT(yqyrDetails);
    xml_attribute<>* totalAmt = yqyrDetails->first_attribute("TotalAmt");
    CPPUNIT_ASSERT(totalAmt);
    CPPUNIT_ASSERT_EQUAL(std::string("100"), std::string(totalAmt->value()));

    xml_node<>* taxOnTaxDetails = taxDetails->first_node("TaxOnTaxDetails");
    CPPUNIT_ASSERT(taxOnTaxDetails);
    totalAmt = taxOnTaxDetails->first_attribute("TotalAmt");
    CPPUNIT_ASSERT(totalAmt);
    CPPUNIT_ASSERT_EQUAL(std::string("200"), std::string(totalAmt->value()));

    xml_node<>* taxOnFaresDetails = taxDetails->first_node("TaxOnFaresDetails");
    CPPUNIT_ASSERT(taxOnFaresDetails);
    totalAmt = taxOnFaresDetails->first_attribute("TotalAmt");
    CPPUNIT_ASSERT(totalAmt);
    CPPUNIT_ASSERT_EQUAL(std::string("300"), std::string(totalAmt->value()));

    xml_node<>* taxOnOcDetails = taxDetails->first_node("TaxOnOptionalServiceDetails");
    CPPUNIT_ASSERT(taxOnOcDetails);
    totalAmt = taxOnOcDetails->first_attribute("TotalAmt");
    CPPUNIT_ASSERT(totalAmt);
    CPPUNIT_ASSERT_EQUAL(std::string("400"), std::string(totalAmt->value()));

    xml_node<>* taxOnBaggageDetails = taxDetails->first_node("TaxOnBaggageDetails");
    CPPUNIT_ASSERT(taxOnBaggageDetails);
    totalAmt = taxOnBaggageDetails->first_attribute("TotalAmt");
    CPPUNIT_ASSERT(totalAmt);
    CPPUNIT_ASSERT_EQUAL(std::string("500"), std::string(totalAmt->value()));
  }

  void attributeTUT()
  {
    OutputResponse response = buildResponseWith5TaxDetails();
    response.itins()->taxDetailsSeq()[0]->taxableUnitTags().push_back(type::TaxableUnit::YqYr);
    response.itins()->taxDetailsSeq()[0]->taxableUnitTags().push_back(type::TaxableUnit::TaxOnTax);
    response.itins()->taxDetailsSeq()[0]->taxableUnitTags().push_back(type::TaxableUnit::Itinerary);
    response.itins()->taxDetailsSeq()[1]->taxableUnitTags().push_back(type::TaxableUnit::OCFlightRelated);

    ParsedResponse<OutputResponse, NaturalXmlTagsList> parsedResponse(response);
    xml_node<>* root = parsedResponse.doc().first_node("TaxRs");
    CPPUNIT_ASSERT(root);
    xml_node<>* itins = root->first_node("Itins");
    CPPUNIT_ASSERT(itins);

    xml_node<>* taxDetails = itins->first_node("TaxDetails");
    CPPUNIT_ASSERT(taxDetails);
    xml_attribute<>* tut = taxDetails->first_attribute("TaxableUnitTags");
    CPPUNIT_ASSERT(tut);
    CPPUNIT_ASSERT_EQUAL(std::string("1 8 9"), std::string(tut->value()));

    taxDetails = taxDetails->next_sibling();
    CPPUNIT_ASSERT(taxDetails);
    tut = taxDetails->first_attribute("TaxableUnitTags");
    CPPUNIT_ASSERT(tut);
    CPPUNIT_ASSERT_EQUAL(std::string("3"), std::string(tut->value()));

    taxDetails = taxDetails->next_sibling();
    CPPUNIT_ASSERT(taxDetails);
    tut = taxDetails->first_attribute("TaxableUnitTags");
    CPPUNIT_ASSERT(tut);
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(tut->value()));
  }

  void attributesInGeoDetails()
  {
    OutputResponse response = buildResponseWith5TaxDetails();
    response.itins()->taxDetailsSeq()[0]->geoDetails() = OutputGeoDetails();
    OutputGeoDetails& geoDetails = *response.itins()->taxDetailsSeq()[0]->geoDetails();

    geoDetails.unticketedPoint() = true;
    geoDetails.taxPointIndexBegin() = 1;
    geoDetails.taxPointIndexEnd() = 5;
    geoDetails.taxPointLoc1() = "AMS";
    geoDetails.taxPointLoc2() = "BRU";
    geoDetails.taxPointLoc3() = "CPH";
    geoDetails.journeyLoc1() = boost::none;
    geoDetails.journeyLoc2() = "JNY2";
    geoDetails.pointOfSaleLoc() = "DUS";
    geoDetails.pointOfTicketingLoc() = "EDI";

    ParsedResponse<OutputResponse, NaturalXmlTagsList> parsedResponse(response);
    xml_node<>* root = parsedResponse.doc().first_node("TaxRs");
    CPPUNIT_ASSERT(root);
    xml_node<>* itins = root->first_node("Itins");
    CPPUNIT_ASSERT(itins);

    xml_node<>* taxDetails = itins->first_node("TaxDetails");
    CPPUNIT_ASSERT(taxDetails);

    xml_node<>* geoDetail = taxDetails->first_node("GeoDetails");
    CPPUNIT_ASSERT(geoDetail);
    xml_attribute<>* attr = geoDetail->first_attribute("UnticketedPoint");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("true"), std::string(attr->value()));

    attr = geoDetail->first_attribute("TaxPointIndexBegin");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), std::string(attr->value()));

    attr = geoDetail->first_attribute("TaxPointIndexEnd");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("5"), std::string(attr->value()));

    attr = geoDetail->first_attribute("TaxPointLoc1");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("AMS"), std::string(attr->value()));

    attr = geoDetail->first_attribute("TaxPointLoc2");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("BRU"), std::string(attr->value()));

    attr = geoDetail->first_attribute("TaxPointLoc3");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("CPH"), std::string(attr->value()));

    attr = geoDetail->first_attribute("JourneyLoc1");
    CPPUNIT_ASSERT(!attr);

    attr = geoDetail->first_attribute("JourneyLoc2");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("JNY2"), std::string(attr->value()));

    attr = geoDetail->first_attribute("PointOfSaleLoc");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("DUS"), std::string(attr->value()));

    attr = geoDetail->first_attribute("PointOfTicketingLoc");
    CPPUNIT_ASSERT(attr);
    CPPUNIT_ASSERT_EQUAL(std::string("EDI"), std::string(attr->value()));
  }

  void attributesInCalcDetails()
  {
    OutputResponse response = buildResponseWith5TaxDetails();

    response.itins()->taxDetailsSeq()[0]->calcDetails() = OutputCalcDetails();

    OutputCalcDetails detail1;
    detail1.taxCurToPaymentCurBSR = type::BSRValue(5, 2);
    detail1.roundingUnit = type::MoneyAmount(1000, 1);
    detail1.roundingDir = type::TaxRoundingDir::Nearest;
    response.itins()->taxDetailsSeq()[1]->calcDetails() = detail1;

    OutputCalcDetails detail2;
    detail2.taxCurToPaymentCurBSR = type::BSRValue(1, 4);
    detail2.roundingUnit = type::MoneyAmount(1, 100);
    detail2.roundingDir = type::TaxRoundingDir::RoundDown;
    response.itins()->taxDetailsSeq()[2]->calcDetails() = detail2;

    OutputCalcDetails detail3;
    detail3.taxCurToPaymentCurBSR = boost::none;
    detail3.roundingUnit = boost::none;
    detail3.roundingDir = type::TaxRoundingDir::NoRounding;
    response.itins()->taxDetailsSeq()[3]->calcDetails() = detail3;

    ParsedResponse<OutputResponse, NaturalXmlTagsList> parsedResponse(response);
    xml_node<>* root = parsedResponse.doc().first_node("TaxRs");
    CPPUNIT_ASSERT(root);
    xml_node<>* itins = root->first_node("Itins");
    CPPUNIT_ASSERT(itins);
    xml_node<>* taxDetails = itins->first_node("TaxDetails");
    CPPUNIT_ASSERT(taxDetails);

    xml_node<>* calcDetails = taxDetails->first_node("CalcDetails");
    CPPUNIT_ASSERT(calcDetails);
    xml_attribute<>* attr = calcDetails->first_attribute("TaxCurToPaymentCurBSR");
    CPPUNIT_ASSERT(!attr);
    attr = calcDetails->first_attribute("TaxRoundingUnit");
    CPPUNIT_ASSERT(!attr);
    attr = calcDetails->first_attribute("TaxRoundingDir");
    CPPUNIT_ASSERT(!attr);

    taxDetails = taxDetails->next_sibling();
    CPPUNIT_ASSERT(taxDetails);
    calcDetails = taxDetails->first_node("CalcDetails");
    CPPUNIT_ASSERT(calcDetails);
    attr = calcDetails->first_attribute("TaxCurToPaymentCurBSR");
    CPPUNIT_ASSERT_EQUAL(std::string("2.5"), std::string(attr->value()));
    attr = calcDetails->first_attribute("TaxRoundingUnit");
    CPPUNIT_ASSERT_EQUAL(std::string("1000"), std::string(attr->value()));
    attr = calcDetails->first_attribute("TaxRoundingDir");
    CPPUNIT_ASSERT_EQUAL(std::string("N"), std::string(attr->value()));

    taxDetails = taxDetails->next_sibling();
    CPPUNIT_ASSERT(taxDetails);
    calcDetails = taxDetails->first_node("CalcDetails");
    CPPUNIT_ASSERT(calcDetails);
    attr = calcDetails->first_attribute("TaxCurToPaymentCurBSR");
    CPPUNIT_ASSERT_EQUAL(std::string("0.25"), std::string(attr->value()));
    attr = calcDetails->first_attribute("TaxRoundingUnit");
    CPPUNIT_ASSERT_EQUAL(std::string("0.01"), std::string(attr->value()));
    attr = calcDetails->first_attribute("TaxRoundingDir");
    CPPUNIT_ASSERT_EQUAL(std::string("D"), std::string(attr->value()));

    taxDetails = taxDetails->next_sibling();
    CPPUNIT_ASSERT(taxDetails);
    calcDetails = taxDetails->first_node("CalcDetails");
    CPPUNIT_ASSERT(calcDetails);
    attr = calcDetails->first_attribute("TaxCurToPaymentCurBSR");
    CPPUNIT_ASSERT(!attr);
    attr = calcDetails->first_attribute("TaxRoundingUnit");
    CPPUNIT_ASSERT(!attr);
    attr = calcDetails->first_attribute("TaxRoundingDir");
    CPPUNIT_ASSERT_EQUAL(std::string("S"), std::string(attr->value()));
  }

  void taxDetailsRef()
  {
    OutputResponse response = buildResponseWith5TaxDetails();
    {
      OutputTaxOnTaxDetails details;
      details.taxDetailsRef.push_back(OutputTaxDetailsRef(3));
      details.taxDetailsRef.push_back(OutputTaxDetailsRef(4));
      response.itins()->taxDetailsSeq()[0]->taxOnTaxDetails = details;
    }

    ParsedResponse<OutputResponse, NaturalXmlTagsList> parsedResponse(response);
    xml_node<>* root = parsedResponse.doc().first_node("TaxRs");
    CPPUNIT_ASSERT(root);
    xml_node<>* itins = root->first_node("Itins");
    CPPUNIT_ASSERT(itins);
    xml_node<>* taxDetails = itins->first_node("TaxDetails");
    CPPUNIT_ASSERT(taxDetails);

    xml_node<>* details = taxDetails->first_node("TaxOnTaxDetails");
    CPPUNIT_ASSERT(details);
    xml_node<>* ref = details->first_node("ElementRef");
    CPPUNIT_ASSERT(ref);
    xml_attribute<>* refId = ref->first_attribute("ElementRefId");
    CPPUNIT_ASSERT_EQUAL(std::string("3"), std::string(refId->value()));

    ref = ref->next_sibling();
    CPPUNIT_ASSERT(ref);
    refId = ref->first_attribute("ElementRefId");
    CPPUNIT_ASSERT_EQUAL(std::string("4"), std::string(refId->value()));

    ref = ref->next_sibling();
    CPPUNIT_ASSERT(!ref);
  }

  void taxBchResponse()
  {
    BCHOutputResponse response = buildBCHResponseWith5Taxes();
    ParsedResponse<BCHOutputResponse, Xml2TagsList> parsedResponse(response);

    xml_node<>* root = parsedResponse.doc().first_node("TRS");
    CPPUNIT_ASSERT(root);

    // Check tax nodes
    xml_node<>* tax = root->first_node("TAX");
    CPPUNIT_ASSERT(tax);
    xml_attribute<>* id = tax->first_attribute("Q1B");
    CPPUNIT_ASSERT(id);
    CPPUNIT_ASSERT_EQUAL(std::string("0"), std::string(id->value()));
    xml_attribute<>* code = tax->first_attribute("BC0");
    CPPUNIT_ASSERT(code);
    CPPUNIT_ASSERT_EQUAL(std::string("AA1"), std::string(code->value()));
    xml_attribute<>* amount = tax->first_attribute("C6B");
    CPPUNIT_ASSERT(amount);
    CPPUNIT_ASSERT_EQUAL(std::string("1.01"), std::string(amount->value()));
    xml_attribute<>* name = tax->first_attribute("S04");
    CPPUNIT_ASSERT(name);
    CPPUNIT_ASSERT_EQUAL(std::string("NAME 1"), std::string(name->value()));
    for (int i = 0 ; i < 4; ++i)
    {
      tax = tax->next_sibling("TAX");
      CPPUNIT_ASSERT(tax);
    }
    CPPUNIT_ASSERT(!tax);

    // Check itin nodes
    xml_node<>* itin = root->first_node("COM");
    CPPUNIT_ASSERT(itin);
    xml_attribute<>* cid = tax->first_attribute("Q1D");
    CPPUNIT_ASSERT(cid);
    CPPUNIT_ASSERT_EQUAL(std::string("0"), std::string(cid->value()));
    xml_node<>* pax = itin->first_node("PXI");
    CPPUNIT_ASSERT(pax);
    xml_attribute<>* ptc = pax->first_attribute("B70");
    CPPUNIT_ASSERT(ptc);
    CPPUNIT_ASSERT_EQUAL(std::string("ADT"), std::string(ptc->value()));
    xml_attribute<>* ptcNum = pax->first_attribute("Q0W");
    CPPUNIT_ASSERT(ptcNum);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), std::string(ptcNum->value()));
    xml_attribute<>* total = pax->first_attribute("C65");
    CPPUNIT_ASSERT(total);
    CPPUNIT_ASSERT_EQUAL(std::string("7.07"), std::string(total->value()));
    xml_attribute<>* tid = pax->first_attribute("TID");
    CPPUNIT_ASSERT(tid);
    CPPUNIT_ASSERT_EQUAL(std::string("0|1|3"), std::string(tid->value()));
    pax = pax->next_sibling("PXI");
    CPPUNIT_ASSERT(!pax);
    itin = itin->next_sibling("COM");
    CPPUNIT_ASSERT(itin);
    itin = itin->next_sibling("COM");
    CPPUNIT_ASSERT(!itin);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(XmlWriterTest);

} // namespace tax

