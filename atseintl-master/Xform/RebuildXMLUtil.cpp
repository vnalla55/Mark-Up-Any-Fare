
#include "Xform/RebuildXMLUtil.h"

#include <cstring>

namespace tse
{

// tagName should include "<"
bool
RebuildXMLUtil::copyTo(std::string& key)
{
  char* firstFind = strstr(_currentPtr, key.c_str());
  if (!firstFind)
    return false;

  char savedCh = *firstFind;
  *(firstFind) = '\0';

  _outputXML.append(_currentPtr);
  _currentPtr = firstFind;
  *_currentPtr = savedCh;

  return true;
}

bool
RebuildXMLUtil::exclusiveCopyTo(std::string& key, std::string& excludedTag)
{
  if (excludedTag.c_str()[0] != '<')
    return false;

  char* copyEnd = nullptr;
  if (!key.empty())
  {
    copyEnd = strstr(_currentPtr, key.c_str());

    if (!copyEnd)
      return false;
  }

  // when not closed tag by />, looking for </tag>
  std::string enclosingTag = "</" + excludedTag.substr(1, excludedTag.size() - 1) + ">";

  while (true)
  {
    char* excludedTagBegin = strstr(_currentPtr, excludedTag.c_str());
    if (!excludedTagBegin)
      break;

    *excludedTagBegin = '\0';
    _outputXML += _currentPtr;
    *excludedTagBegin = '<';
    _currentPtr = excludedTagBegin + excludedTag.size();

    char* closingCh = strstr(_currentPtr, ">");
    if (!closingCh) break;

    if (*(closingCh - 1) != '/') // not closed by />
    {
      closingCh = strstr(closingCh, enclosingTag.c_str() );
      if (!closingCh)
        break;
    }

    _currentPtr = closingCh + enclosingTag.size();
  }

  char savedCh = '\0';
  if (copyEnd)
  {
    savedCh = *copyEnd;
    *copyEnd = '\0';
  }
  _outputXML += _currentPtr;
  if (copyEnd)
    *copyEnd = savedCh;

  return true;
}

bool
RebuildXMLUtil::copyContentOf(std::string& excludedTag,
    const char* elementStr,
    std::map<std::string, std::string>& attributeValues,
    const bool moveReadPointer)
{
  char* savedCurrentReadPtr = _currentPtr;
  bool foundContent = false;

  while (true)
  {
    char* firstFind = strstr(_currentPtr, excludedTag.c_str());
    if (!firstFind)
      break;

    _currentPtr = firstFind + excludedTag.size();

    char* closingCh = strstr(firstFind, ">");
    if (!closingCh) break;

    const bool noValueOrElement = (*(closingCh - 1) == '/');

    if (noValueOrElement)
      continue;

    std::string enclosingTag = "</" + excludedTag.substr(1, excludedTag.size() - 1) + ">";

    if (elementStr)
    {
      firstFind = strstr(_currentPtr, elementStr);
      if (!firstFind)
      {
        closingCh = strstr(_currentPtr, enclosingTag.c_str());
        if (!closingCh)
          break;
        _currentPtr = closingCh + enclosingTag.size();
        continue;
      }
      else
      {
        _currentPtr = firstFind;
        char* closingCh = strstr(firstFind, ">");
        if (!closingCh)
          break;
      }
    }

    *closingCh = '\0';
    const char* attributesStr = _currentPtr;

    bool matchedAttribute = true;
    std::map<std::string, std::string>::const_iterator attrIt = attributeValues.begin();
    for(; attrIt != attributeValues.end(); ++attrIt)
    {
      std::string value;

      getAttributeValue(attributesStr, attrIt->first, value);

      if (attrIt->second != value)
      {
        matchedAttribute = false;
        break;
      }
    }

    *closingCh = '>';
    char* encloseFind = strstr(closingCh, enclosingTag.c_str() );
    if (!encloseFind)
      break;

    if (matchedAttribute)
    {
      foundContent = true;
      *encloseFind = '\0';
      _outputXML.append(closingCh + 1);
      *encloseFind = '<';

      if (moveReadPointer)
        _currentPtr = encloseFind + enclosingTag.size();

      break;
    }
    else
    {
      _currentPtr = encloseFind + enclosingTag.size();
    }
  }
  if (!moveReadPointer || !foundContent)
    _currentPtr = savedCurrentReadPtr;

  return foundContent;
}

void
RebuildXMLUtil::getAttributeValue(const char* attributesStr, const std::string& attrName, std::string& value)
{
  value = "";
  std::string attrTag = attrName + "=\"";
  const char* findAttr = strstr(attributesStr, attrTag.c_str() );
  if (!findAttr)
    return;

  const char* valueStrStart = findAttr + attrTag.size();
  char* valueStrEnd = const_cast<char*>(strstr(valueStrStart, "\""));
  if (!valueStrEnd)
    return;

  *valueStrEnd = '\0';
  value.append(valueStrStart);
  *valueStrEnd = '\"';
}

}
