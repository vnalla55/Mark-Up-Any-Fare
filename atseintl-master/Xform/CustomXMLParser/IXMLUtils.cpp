#include "Xform/CustomXMLParser/IXMLUtils.h"

#include <boost/algorithm/string/replace.hpp>

#include <algorithm>
#include <iterator>

namespace IXMLUtils
{
void
encode(const std::string& input, std::ostream& os)
{
  size_t pos(0), prvPos(0);
  bool modified(false);
  while ((pos = input.find_first_of(_specChars, prvPos)) != std::string::npos)
  {
    modified = true;
    std::ostream_iterator<char> it(os);
    std::copy(input.begin() + prvPos, input.begin() + pos, it);
    for (size_t i = 0; i < _encodingCount; ++i)
    {
      if (_specChars[i] == input[pos])
      {
        os << _encoding[i];
        break;
      }
    }
    prvPos = pos + 1;
  }
  if (modified)
  {
    std::ostream_iterator<char> it(os);
    std::copy(input.begin() + prvPos, input.end(), it);
  }
  else
  {
    os << input;
  }
}

bool
initLookupMaps(const char* const elementNames[],
               int numberOfElementNames,
               ILookupMap& elemLookupMap)
{
  int idx = elemLookupMap.size();
  const int mapSize = elemLookupMap.size();

  for (int i = 0; i < numberOfElementNames - mapSize; i++, idx++)
  {
    IKeyString nm(elementNames[i], strlen(elementNames[i]));
    elemLookupMap[nm] = idx;
  }
  return true;
}

bool
initLookupMaps(const char* const elementNames[],
               int numberOfElementNames,
               ILookupMap& elemLookupMap,
               const char* const attributeNames[],
               int numberOfAttributeNames,
               ILookupMap& attrLookupMap)
{
  initLookupMaps(elementNames, numberOfElementNames, elemLookupMap);
  initLookupMaps(attributeNames, numberOfAttributeNames, attrLookupMap);

  return true;
}

bool
initLookupMaps(const char* const elementNames[],
               int numberOfElementNames,
               const char* const additionalElementNames[],
               int maxAdditionalElementNames,
               ILookupMap& elemLookupMap,
               const char* const attributeNames[],
               int numberOfAttributeNames,
               const char* const additionalAttributeNames[],
               int maxAdditionlAttributeNames,
               ILookupMap& attrLookupMap)
{
  initLookupMaps(elementNames, numberOfElementNames, elemLookupMap);
  initLookupMaps(additionalElementNames, maxAdditionalElementNames, elemLookupMap);

  initLookupMaps(attributeNames, numberOfAttributeNames, attrLookupMap);
  initLookupMaps(additionalAttributeNames, maxAdditionlAttributeNames, attrLookupMap);

  return true;
}

void
stripnamespaces(std::string& content)
{
  size_t pos(0);
  std::vector<std::string> namespaces;
  static const std::string XMLNS("xmlns:");
  static const size_t XMLNS_SZ(XMLNS.size());
  while ((pos = content.find(XMLNS, pos)) != std::string::npos)
  {
    size_t end(content.find('=', pos));
    if (end != std::string::npos)
    {
      std::string ns(content, pos + XMLNS_SZ, end - pos - XMLNS_SZ);
      namespaces.push_back(ns + ':');
    }
    pos = end;
  }
  for (const std::string& ns : namespaces)
  {
    boost::replace_all(content, ns, "");
  }
}
}
