// ----------------------------------------------------------------------------
//  File:         XMLConstruct.cpp
//  Author:       Jim Stoltenberg
//  Created:      01/23/2004
//  The XMLConstruct provides functionalities to build XML tags,
//                elements and attributes information for the response.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/XMLConstruct.h"

#include <string>

// Constants for XML special character replacement.
const char* XMLConstruct::startCharReplacement = "&lt;";
const char* XMLConstruct::endCharReplacement = "&gt;";
const char* XMLConstruct::ampersendCharReplacement = "&amp;";
const char* XMLConstruct::apostropheCharReplacement = "&apos;";
const char* XMLConstruct::quoteCharReplacement = "&quot;";

/*
 * Constructor
 *
 * @param blockFactor - initially allocate this much size for an XML string.
 */

XMLConstruct::XMLConstruct(size_t blockFactor) : size(blockFactor)
{
  data.reserve(size);
}

/*
 * Add an opening tag for the XML.
 *
 * @param tagName - the name of the XML opening tag.
 * @param tagSize - the size of this opening tag.
 * @return bool - whether the insertion is successful or not.
 */

bool
XMLConstruct::openElement(const char* tagName, size_t tagSize) // lint !e601
{
  if (history.size() > 0)
  {
    XMLRecord& rec = history.top();
    rec.hasElementData = true;
  }

  // close parent block which could still be open  "<TAG" to "<TAG>"

  if (openTag)
  {
    data += endChar;
  }

  data += startChar;

  history.push(XMLRecord(data.size(), tagSize));

  data += tagName;

  openTag = true;

  return !error;
}

/*
 * Close an XML tag.
 *
 * @return bool - whether the insertion of a closing tag is successful or not.
 */

bool
XMLConstruct::closeElement()
{
  openTag = false;

  if (LIKELY(history.size() > 0))
  {
    XMLRecord& rec = history.top();

    // need long form "</TAG>" ?

    if (rec.hasElementData)
    {
      data += startChar;

      data += terminateChar;

      data.append(data, rec.tagIndex, rec.tagLength);

      data += endChar;
    }

    // no just a simple close "/>"

    else
    {
      data += terminateChar;

      data += endChar;
    }

    history.pop();
  }
  else
  {
    error = true;
  }

  return !error;
}

/**
 * Add an attribute for an XML tag.
 *
 * @param attributeName - the name of the attribute to be added.
 * @param attributeSize - the size of the attribute.
 * @param value - the value for this attribute.
 * @param valueSize - the size of the value for this attribute.
 * @return bool - whether the insertion is successful or not.
 */

bool
XMLConstruct::addAttribute(const char* attributeName,
                           std::size_t attributeSize, // lint !e601
                           const char* value,
                           std::size_t valueSize, // lint !e601
                           uint16_t addEmptyValue)
{
  if (LIKELY(openTag))
  {

    if (addEmptyValue)
    {
      if (attributeSize <= 0 || valueSize < 0)
        return !error;
    }
    else
    {
      if (attributeSize <= 0 || valueSize <= 0)
        return !error;
    }

    data += whiteSpace;

    data.append(attributeName, attributeSize);

    data += assign;

    data += quoteChar;

    if (LIKELY(valueSize > 0))
    {
      data.append(value, valueSize);
    }

    data += quoteChar;
  }
  else
  {
    error = true;
  }

  return !error;
}

/*
 * Add data to an element.
 *
 * @param elementData - the data for this element.
 * @return bool - whether the insertion is successful or not.
 */
bool
XMLConstruct::addSpecialElement(const char* elementData)
{
  if (!elementData)
    return false;

  if (!history.empty())
  {
    XMLRecord& rec = history.top();
    rec.hasElementData = true;
  }

  data += startChar;

  data += specialChar;

  data.append(elementData);

  data += specialChar;

  data += endChar;

  return !error;
}

/*
 * Add data to an element (tag).
 *
 * @param elementData - the data for this element (tag).
 */

void
XMLConstruct::addElementData(const XMLConstruct& elementData)
{
  if (!history.empty())
  {
    XMLRecord& rec = history.top();
    rec.hasElementData = true;

    if (openTag)
    {
      openTag = false;
      data += endChar;
    }
  }

  data.append(elementData.getXMLData().data(), elementData.getXMLData().size());
}

/*
 * Add data to an element (tag).
 *
 * @param elementData - the data for this element (tag).
 * @param elementDataSize - the size of the data (tag).
 * @return bool - whether the insertion is successful or not.
 */

bool
XMLConstruct::addElementData(const char* elementData, size_t elementDataSize) // lint !e601
{
  if (!history.empty())
  {
    XMLRecord& rec = history.top();
    rec.hasElementData = true;

    if (openTag)
    {
      openTag = false;
      data += endChar;
    }

    data.append(elementData, elementDataSize);
  }
  else
  {
    error = true;
  }
  return !error;
}
