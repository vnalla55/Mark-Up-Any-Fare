#include "test/include/CppUnitHelperMacros.h"
#include "Xform/XMLWriter.h"

namespace tse
{
class XMLWriterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLWriterTest);
  CPPUNIT_TEST(testXMLWriter);
  CPPUNIT_TEST_SUITE_END();

public:
  void testXMLWriter()
  {
    XMLWriter writer;
    {
      XMLWriter::Node node(writer, "request");
      {
        XMLWriter::Node element(writer, "element");
        element.attr("a", "45");
        XMLWriter::Node(writer, "anon").convertAttr("b", 19).attr("c", "45");

        XMLWriter::Node text_element(writer, "text_element");
        text_element.addText("text\n");
        text_element.addText("needs&escaping\n");
        text_element.addData("<DoEscaping>");
        text_element.addRawText("<NoEscaping>");
      }

      {
        XMLWriter::Node element(writer, "element");
      }
    }

    const std::string expectedResult = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                       "<request>\n"
                                       "  <element a=\"45\">\n"
                                       "    <anon b=\"19\" c=\"45\"/>\n"
                                       "    <text_element>\n"
                                       "text\nneeds&amp;escaping\n"
                                       "<![CDATA[&lt;DoEscaping&gt;]]>"
                                       "<NoEscaping>"
                                       "    </text_element>\n"
                                       "  </element>\n"
                                       "  <element/>\n"
                                       "</request>\n";

    CPPUNIT_ASSERT_EQUAL(expectedResult, writer.result());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(XMLWriterTest);
}
