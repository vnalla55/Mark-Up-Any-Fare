#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/Templates/ElementField.h"
#include <sstream>

using namespace std;

namespace tse
{
class ElementFieldTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ElementFieldTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testRenderNoStream);
  CPPUNIT_TEST(testRenderBoolLeft);
  CPPUNIT_TEST(testRenderBoolRight);
  CPPUNIT_TEST(testRenderIntLeft);
  CPPUNIT_TEST(testRenderIntRight);
  CPPUNIT_TEST(testRenderStringLeft);
  CPPUNIT_TEST(testRenderStringRight);
  CPPUNIT_TEST(testRenderLabelNoStream);
  CPPUNIT_TEST(testRenderLabel);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    ElementField field;
    CPPUNIT_ASSERT_EQUAL(int16_t(0), field.labelPosition());
    CPPUNIT_ASSERT_EQUAL(int16_t(0), field.valuePosition());
    CPPUNIT_ASSERT_EQUAL(0, field.intValue());
    // This field is never initialized
    // CPPUNIT_ASSERT_EQUAL(int16_t(0), field.valueFieldSize());
    CPPUNIT_ASSERT_EQUAL(string(""), field.label());
    CPPUNIT_ASSERT_EQUAL(string(""), field.strValue());
    CPPUNIT_ASSERT_EQUAL(string(""), field.strValue2());
    CPPUNIT_ASSERT_EQUAL(false, field.boolValue());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(0), field.moneyValue());
    // This field is never initialized
    // CPPUNIT_ASSERT_EQUAL(JustificationType(RIGHT), field.justify());
  }

  void testRenderNoStream()
  {
    ElementField field;
    FieldValueType valueType;
    ostringstream* stream = 0;
    CPPUNIT_ASSERT_NO_THROW(field.render(stream, valueType));
  }

  void testRenderBoolLeft()
  {
    ElementField field;
    FieldValueType valueType = BOOL_VALUE;
    field.initialize(LINE_NUMBER, 1, 1, LEFT);
    _stream << std::setw(6) << std::setfill(' ') << "\n";
    field.boolValue() = true;
    field.render(&_stream, valueType);
    CPPUNIT_ASSERT_EQUAL(string("1    \n"), _stream.str());
  }

  void testRenderBoolRight()
  {
    ElementField field;
    FieldValueType valueType = BOOL_VALUE;
    field.initialize(LINE_NUMBER, 1, 1, RIGHT);
    _stream << std::setw(6) << std::setfill(' ') << "\n";
    field.boolValue() = false;
    field.render(&_stream, valueType);
    CPPUNIT_ASSERT_EQUAL(string("0    \n"), _stream.str());
  }

  void testRenderIntLeft()
  {
    ElementField field;
    FieldValueType valueType = INT_VALUE;
    field.valueFieldSize() = 8;
    field.valuePosition() = 1;
    field.justify() = JustificationType(LEFT);
    _stream << std::setw(6) << std::setfill(' ') << "\n";
    field.intValue() = 500;
    field.render(&_stream, valueType);
    CPPUNIT_ASSERT_EQUAL(string("500     "), _stream.str());
  }

  void testRenderIntRight()
  {
    ElementField field;
    FieldValueType valueType = INT_VALUE;
    field.valueFieldSize() = 8;
    field.valuePosition() = 1;
    field.justify() = JustificationType(RIGHT);
    _stream << std::setw(6) << std::setfill(' ') << "\n";
    field.intValue() = -500;
    field.render(&_stream, valueType);
    CPPUNIT_ASSERT_EQUAL(string("    -500"), _stream.str());
  }

  void testRenderStringLeft()
  {
    ElementField field;
    FieldValueType valueType = STRING_VALUE;
    field.valueFieldSize() = 19;
    field.valuePosition() = 1;
    _stream << std::setw(22) << std::setfill(' ') << "\n";
    field.justify() = JustificationType(LEFT);
    field.strValue() = " my value goes here ";
    field.render(&_stream, valueType);
    CPPUNIT_ASSERT_EQUAL(string(" my value goes here  \n"), _stream.str());
  }

  void testRenderStringRight()
  {
    ElementField field;
    FieldValueType valueType = STRING_VALUE;
    field.valueFieldSize() = 8;
    field.valuePosition() = 1;
    _stream << std::setw(22) << std::setfill(' ') << "\n";
    field.justify() = JustificationType(RIGHT);
    field.strValue() = "not here";
    field.render(&_stream, valueType);
    CPPUNIT_ASSERT_EQUAL(string("not here             \n"), _stream.str());
  }

  void testRenderLabelNoStream()
  {
    ElementField field;
    ostringstream* stream = 0;
    CPPUNIT_ASSERT_NO_THROW(field.renderLabel(stream));
  }

  void testRenderLabel()
  {
    ElementField field;
    field.valueFieldSize() = 8;
    field.valuePosition() = 1;
    _stream << std::setw(22) << std::setfill(' ') << "\n";
    field.justify() = JustificationType(RIGHT);
    field.strValue() = "not here";
    field.renderLabel(&_stream);
    CPPUNIT_ASSERT_EQUAL(string("not here             \n"), _stream.str());
  }

protected:
  ostringstream _stream;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ElementFieldTest);
} // namespace
