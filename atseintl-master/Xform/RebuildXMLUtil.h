//
//      File: RebuildXMLUtil.h
//      Description: Class to convert XML string to format that parser can read
//      Created: June 18, 2014
//      Authors: 
//
#pragma once

#include "Common/TseStringTypes.h"

#include <map>
#include <string>
#include <vector>

namespace tse
{

class RebuildXMLUtil
{
  friend class RebuildXMLUtilTest;

public:
  RebuildXMLUtil(const char* originalXML)
    : _originalXML(originalXML)
  {
    _currentPtr = const_cast<char*>(_originalXML);
  }

  bool copyTo(std::string& key);
  bool exclusiveCopyTo(std::string& key, std::string& excludedTag);

  bool copyContentOf(std::string& tagName, 
                     const char* elementStr,
                     std::map<std::string, std::string>& attributeValues, 
                     const bool moveReadPointer);

  std::string& outputXML() { return _outputXML; }

  static void getAttributeValue(const char* attributesStr, const std::string& attrName, std::string& value);
  
private:
  const char* _originalXML;
  char* _currentPtr;
  std::string _outputXML;
};

}

