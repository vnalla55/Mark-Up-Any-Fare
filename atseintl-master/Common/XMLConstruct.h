//   File:         XMLConstruct.h
//   Author:       Jim Stoltenberg
//   Created:      01/23/2004
//    The XMLConstruct provides functionalities to build XML tags,
//                 elements and attributes information for the response.
//   Copyright Sabre 2004
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.

#pragma once

#include "Util/BranchPrediction.h"

#include <boost/container/string.hpp>

#include <cstdio>
#include <cstring>
#include <sstream>
#include <stack>
#include <string>

#include <cstdint>

/**
 *  Store XML tag information.
 */
struct XMLRecord
{
  XMLRecord(size_t tagLoc, size_t size) : tagIndex(tagLoc), tagLength(size) {}

  size_t tagIndex;
  size_t tagLength;
  bool hasAttributes = false;
  bool hasElementData = false;
};

/**
 *  The XMLConstruct provides functionalities to build XML tags,
 *  elements and attributes information for the response.
 */
class XMLConstruct final
{

public:
  explicit XMLConstruct(size_t blockFactor = 100);
  XMLConstruct(XMLConstruct&) = delete;
  XMLConstruct& operator=(const XMLConstruct&) = delete;

  bool openElement(const char* tagName, size_t tagSize);
  inline bool openElement(const char* tagName);
  inline bool openElement(const std::string& tagName);

  bool closeElement();

  bool addAttribute(const char* attributeName,
                    size_t attributeSize,
                    const char* value,
                    size_t valueSize,
                    uint16_t addEmptyValue = 0);

  inline bool addAttribute(const char* attributeName, const char* value);
  inline bool addAttribute(const std::string& attributeName, const char* value);
  inline bool addAttribute(const std::string& attributeName, const std::string& value);
  inline bool addAttribute(const std::string& attributeName, const boost::container::string& value);
  inline bool addAttributeNoNull(const char* attributeName, const char* value);
  inline bool addAttributeNoNull(const char* attributeName, const std::string& value);
  inline bool addAttributeNoNull(const std::string& attributeName, const std::string& value);

  inline bool addAttributeShort(const char* attributeName, int16_t value);
  inline bool addAttributeShort(const std::string& attributeName, int16_t value);

  inline bool addAttributeUShort(const char* attributeName, uint16_t value);
  inline bool addAttributeUShort(const std::string& attributeName, uint16_t value);

  inline bool addAttributeInteger(const char* attributeName, int32_t value);
  inline bool addAttributeInteger(const std::string& attributeName, int32_t value);

  inline bool addAttributeUInteger(const char* attributeName, uint32_t value);
  inline bool addAttributeUInteger(const std::string& attributeName, uint32_t value);

  inline bool addAttributeLong(const char* attributeName, int64_t value);
  inline bool addAttributeLong(const std::string& attributeName, int64_t value);

  inline bool addAttributeULong(const char* attributeName, uint64_t value);
  inline bool addAttributeULong(const std::string& attributeName, uint64_t value);

  inline bool addAttributeDouble(const char* attributeName, double value, int numDecimal);
  inline bool addAttributeDouble(const std::string& attributeName, double value, int numDecimal);
  inline bool addAttributeDouble(const char* attributeName, double value);
  inline bool addAttributeDouble(const std::string& attributeName, double value);

  inline bool addAttributeBoolean(const char* attributeName, bool value);
  inline bool addAttributeBoolean(const std::string& attributeName, bool value);

  inline bool addAttributeChar(const char* attributeName, char value);
  inline bool addAttributeChar(const std::string& attributeName, char value);

  inline bool addElementData(const char* elementData);

  bool addSpecialElement(const char* elementData);

  bool addElementData(const char* elementData, size_t elementDataSize);
  void addElementData(const XMLConstruct& elementData);

  const std::string& getXMLData() const;

  inline bool isWellFormed();

  inline bool addAlsoEmptyAttribute(const char* attributeName, const char* value);

  inline bool addAlsoEmptyAttribute(const std::string& attributeName, const std::string& value);

private:
  size_t offset = 0;

  size_t size;

  std::string data;

  std::stack<XMLRecord> history;

  bool error = false;
  bool openTag = false;

  // Constants for XML response construction.
  static constexpr char startChar = '<';
  static constexpr char endChar = '>';
  static constexpr char specialChar = '?';
  static constexpr char terminateChar = '/';
  static constexpr char quoteChar = '"';
  static constexpr char ampersendChar = '&';
  static constexpr char apostropheChar = '\'';
  static constexpr char whiteSpace = ' ';
  static constexpr char assign = '=';

  static const char* startCharReplacement;
  static const char* endCharReplacement;
  static const char* quoteCharReplacement;
  static const char* apostropheCharReplacement;
  static const char* ampersendCharReplacement;
};

/**
 *  Add an XML opening tag.
 *  @param tagName - the name of the opening tag.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::openElement(const char* tagName)
{
  return openElement(tagName, strlen(tagName));
}

inline bool
XMLConstruct::openElement(const std::string& tagName)
{
  return openElement(tagName.c_str());
}

/**
 *  Add an XML attribute for an element.
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in string format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttribute(const char* attributeName, const char* value)
{
  return addAttribute(attributeName, strlen(attributeName), value, strlen(value));
}

inline bool
XMLConstruct::addAttribute(const std::string& attributeName, const std::string& value)
{
  return addAttribute(attributeName.c_str(), value.c_str());
}

inline bool
XMLConstruct::addAttribute(const std::string& attributeName, const char* value)
{
  return addAttribute(attributeName.c_str(), value);
}

inline bool
XMLConstruct::addAttribute(const std::string& attributeName, const boost::container::string& value)
{
  return addAttribute(attributeName.c_str(), value.c_str());
}

inline bool
XMLConstruct::addAttributeNoNull(const char* attributeName, const char* value)
{
  if (*value == 0)
    return true;
  return addAttribute(attributeName, strlen(attributeName), value, strlen(value));
}

inline bool
XMLConstruct::addAttributeNoNull(const char* attributeName, const std::string& value)
{
  return addAttributeNoNull(attributeName, value.c_str());
}

inline bool
XMLConstruct::addAttributeNoNull(const std::string& attributeName, const std::string& value)
{
  return addAttributeNoNull(attributeName.c_str(), value.c_str());
}

/**
 *  Add an XML attribute for an element in short integer format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in short integer format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeShort(const char* attributeName, int16_t value)
{
  char tmp[6];

  memset(tmp, '\0', sizeof(tmp));

  sprintf(tmp, "%d", value);

  return addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp));
}

inline bool
XMLConstruct::addAttributeShort(const std::string& attributeName, int16_t value)
{
  return addAttributeShort(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in unsigned short integer format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in unsigned short integer format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeUShort(const char* attributeName, uint16_t value)
{
  char tmp[6];

  memset(tmp, '\0', sizeof(tmp));

  sprintf(tmp, "%u", value);

  return addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp));
}

inline bool
XMLConstruct::addAttributeUShort(const std::string& attributeName, uint16_t value)
{
  return addAttributeUShort(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in integer format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in integer format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeInteger(const char* attributeName, int32_t value)
{
  char tmp[20];

  memset(tmp, '\0', sizeof(tmp));

  sprintf(tmp, "%d", value);

  return addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp));
}

inline bool
XMLConstruct::addAttributeInteger(const std::string& attributeName, int32_t value)
{
  return addAttributeInteger(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in unsigned integer format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in unsigned integer format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeUInteger(const char* attributeName, uint32_t value)
{
  char tmp[20];

  memset(tmp, '\0', sizeof(tmp));

  sprintf(tmp, "%u", value);

  return addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp));
}

inline bool
XMLConstruct::addAttributeUInteger(const std::string& attributeName, uint32_t value)
{
  return addAttributeUInteger(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in long long integer format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in long long integer format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeLong(const char* attributeName, int64_t value)
{
  char tmp[21];

  memset(tmp, '\0', sizeof(tmp));

  sprintf(tmp, "%ld", value);

  return addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp));
}

inline bool
XMLConstruct::addAttributeLong(const std::string& attributeName, int64_t value)
{
  return addAttributeLong(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in unsigned long long integer format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in unsigned long long integer format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeULong(const char* attributeName, uint64_t value)
{
  char tmp[21];

  memset(tmp, '\0', sizeof(tmp));

  sprintf(tmp, "%lu", value);

  return addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp));
}

inline bool
XMLConstruct::addAttributeULong(const std::string& attributeName, uint64_t value)
{
  return addAttributeULong(attributeName.c_str(), value);
}

/**
 *   Add an XML attribute for an element in double format with specified decimal
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in double format.
 *  @param numDecimal - the number of decimal points
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeDouble(const char* attributeName, double value, int numDecimal)
{
  if (value == 0)
  {
    if (numDecimal == 0)
      return (addAttribute(attributeName, strlen(attributeName), "0", 1));
  }

  std::ostringstream stringStream;

  stringStream.setf(std::ios::fixed, std::ios::floatfield);
  stringStream.setf(std::ios::right, std::ios::adjustfield);

  stringStream.precision(numDecimal);

  stringStream << value;

  std::string formattedDouble = stringStream.str();

  return (addAttribute(
      attributeName, strlen(attributeName), formattedDouble.c_str(), formattedDouble.size()));
}

inline bool
XMLConstruct::addAttributeDouble(const std::string& attributeName, double value, int numDecimal)
{
  return addAttributeDouble(attributeName.c_str(), value, numDecimal);
}

/**
 *   Add an XML attribute for an element in double format of no decimal specified
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in double format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeDouble(const char* attributeName, double value)
{
  if (value == 0)
  {
    return (addAttribute(attributeName, strlen(attributeName), "0", 1));
  }

  std::ostringstream stringStream;

  stringStream.setf(std::ios::fixed, std::ios::floatfield);
  stringStream.setf(std::ios::right, std::ios::adjustfield);

  stringStream << value;

  std::string formattedDouble = stringStream.str();
  std::string zeroStr("0");
  std::string::size_type idx = formattedDouble.find_last_not_of(zeroStr);

  if (idx != std::string::npos)
    formattedDouble.erase((idx + 1), formattedDouble.size());

  return (addAttribute(
      attributeName, strlen(attributeName), formattedDouble.c_str(), formattedDouble.size()));
}

inline bool
XMLConstruct::addAttributeDouble(const std::string& attributeName, double value)
{
  return addAttributeDouble(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in boolean format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in boolean format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeBoolean(const char* attributeName, bool value)
{
  if (value)
  {
    return (addAttribute(attributeName, strlen(attributeName), "T", 1));
  }

  return (addAttribute(attributeName, strlen(attributeName), "F", 1));
}

inline bool
XMLConstruct::addAttributeBoolean(const std::string& attributeName, bool value)
{
  return addAttributeBoolean(attributeName.c_str(), value);
}

/**
 *  Add an XML attribute for an element in character format
 *  @param attributeName - the name of the attribute.
 *  @param value - the value of the attribute in character format.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addAttributeChar(const char* attributeName, char value)
{
  char tmp[2];

  memset(tmp, '\0', sizeof(tmp));

  tmp[0] = value;
  tmp[1] = '\0';

  return (addAttribute(attributeName, strlen(attributeName), tmp, strlen(tmp)));
}

inline bool
XMLConstruct::addAttributeChar(const std::string& attributeName, char value)
{
  return addAttributeChar(attributeName.c_str(), value);
}

/**
 *  Add data for an XML element.
 *  @param elementData - the data for the element.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::addElementData(const char* elementData)
{
  return addElementData(elementData, strlen(elementData));
}

/**
 *  Validate the constructed XML string.
 *  @return a string containing the tag.
 */
inline bool
XMLConstruct::isWellFormed()
{
  return (!error && !data.empty() && history.empty());
}

/**
 *   Get the constructed XML string.
 *  @return a string containing the XML.
 */
inline const std::string&
XMLConstruct::getXMLData() const
{
  return data;
}

inline bool
XMLConstruct::addAlsoEmptyAttribute(const char* attributeName, const char* value)
{
  uint16_t addEmptyValue = 1;
  return addAttribute(attributeName, strlen(attributeName), value, strlen(value), addEmptyValue);
}

inline bool
XMLConstruct::addAlsoEmptyAttribute(const std::string& attributeName, const std::string& value)
{
  return addAlsoEmptyAttribute(attributeName.c_str(), value.c_str());
}

