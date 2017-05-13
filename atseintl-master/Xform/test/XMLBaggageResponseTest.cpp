#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <vector>

#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XMLBaggageResponse.h"
#include "Xform/XMLWriter.h"
#include "test/include/TestMemHandle.h"

using namespace boost::assign;

namespace tse
{
  class XMLBaggageResponseTest : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(XMLBaggageResponseTest);
    CPPUNIT_TEST(testGenerateAllowanceAttributes_no_allowance);
    CPPUNIT_TEST(testGenerateAllowanceAttributes_weight_only);
    CPPUNIT_TEST(testGenerateAllowanceAttributes_pcs_only);
    CPPUNIT_TEST(testGenerateAllowanceAttributes_pcs_weight_kg);
    CPPUNIT_TEST(testGenerateAllowanceAttributes_pcs_weight_lbs);
    CPPUNIT_TEST(testGenerateQ00);
    CPPUNIT_TEST_SUITE_END();

    void testGenerateAllowanceAttributes_no_allowance()
    {
      OptionalServicesInfo s7;

      s7.freeBaggagePcs() = 0;
      s7.baggageWeight() = 0;
      s7.baggageWeightUnit() = ' ';

      XMLWriter writer;
      {
        XMLWriter::Node nodeBDI(writer, xml2::BaggageBDI);
        XMLBaggageResponse response(writer);

        response.generateAllowanceAttributes(nodeBDI, s7);
      }
      CPPUNIT_ASSERT_EQUAL(
          std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><BDI BPC=\"0\"/>\n"),
          writer.result());
    }

    void testGenerateAllowanceAttributes_weight_only()
    {
      OptionalServicesInfo s7;

      s7.baggageWeight() = 20;
      s7.baggageWeightUnit() = 'K';

      XMLWriter writer;
      {
        XMLWriter::Node nodeBDI(writer, xml2::BaggageBDI);
        XMLBaggageResponse response(writer);

        response.generateAllowanceAttributes(nodeBDI, s7);
      }
      CPPUNIT_ASSERT_EQUAL(
          std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><BDI B20=\"20\" N0D=\"K\"/>\n"),
          writer.result());
    }

    void testGenerateAllowanceAttributes_pcs_only()
    {
      OptionalServicesInfo s7;

      s7.freeBaggagePcs() = 2;

      XMLWriter writer;
      {
        XMLWriter::Node nodeBDI(writer, xml2::BaggageBDI);
        XMLBaggageResponse response(writer);

        response.generateAllowanceAttributes(nodeBDI, s7);
      }
      CPPUNIT_ASSERT_EQUAL(
          std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><BDI BPC=\"2\"/>\n"),
          writer.result());
    }

    void testGenerateAllowanceAttributes_pcs_weight_kg()
    {
      OptionalServicesInfo s7;

      s7.freeBaggagePcs() = 2;
      s7.baggageWeight() = 10;
      s7.baggageWeightUnit() = 'K';

      XMLWriter writer;
      {
        XMLWriter::Node nodeBDI(writer, xml2::BaggageBDI);
        XMLBaggageResponse response(writer);

        response.generateAllowanceAttributes(nodeBDI, s7);
      }
      CPPUNIT_ASSERT_EQUAL(
          std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><BDI BPC=\"2\" B20=\"10\" N0D=\"K\"/>\n"),
          writer.result());
    }

    void testGenerateAllowanceAttributes_pcs_weight_lbs()
    {
      OptionalServicesInfo s7;

      s7.freeBaggagePcs() = 1;
      s7.baggageWeight() = 3;
      s7.baggageWeightUnit() = 'L';

      XMLWriter writer;
      {
        XMLWriter::Node nodeBDI(writer, xml2::BaggageBDI);
        XMLBaggageResponse response(writer);

        response.generateAllowanceAttributes(nodeBDI, s7);
      }
      CPPUNIT_ASSERT_EQUAL(
          std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><BDI BPC=\"1\" B20=\"3\" N0D=\"L\"/>\n"),
          writer.result());
    }

    void testGenerateQ00()
    {
      std::vector<TravelSeg*> segments;
      AirSeg seg1, seg2;

      seg1.pnrSegment() = 1;
      seg2.pnrSegment() = 2;

      segments += &seg1;
      segments += &seg2;

      BaggageTravel baggageTravel;
      baggageTravel.updateSegmentsRange(segments.begin(), segments.end());

      XMLWriter writer;
      XMLBaggageResponse response(writer);

      response.generateQ00(baggageTravel);

      CPPUNIT_ASSERT_EQUAL(
          std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><Q00>0</Q00>\n<Q00>1</Q00>\n"),
          response._writer.result());
    }
  };

  CPPUNIT_TEST_SUITE_REGISTRATION(XMLBaggageResponseTest);
}
