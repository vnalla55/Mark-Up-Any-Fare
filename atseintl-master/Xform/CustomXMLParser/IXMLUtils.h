#pragma once

#include "Xform/CustomXMLParser/ICString.h"

#include <ostream>
#include <string>

#include <tr1/unordered_map>

typedef std::tr1::unordered_map<IKeyString, int, IKeyStringHasher> ILookupMap;

namespace IXMLUtils
{
const char* const
_specChars("&<>\'\"");
const size_t
_encodingCount(strlen(_specChars));
const char* const _encoding[] = { "&amp;", "&lt;", "&gt;", "&apos;", "&quot;" };

void
encode(const std::string& input, std::ostream& os);

bool
initLookupMaps(const char* const elementNames[],
               int numberOfElementNames,
               ILookupMap& elemLookupMap);

bool
initLookupMaps(const char* const elementNames[],
               int numberOfElementNames,
               ILookupMap& elemLookupMap,
               const char* const attributeNames[],
               int numberOfAttributeNames,
               ILookupMap& attrLookupMap);

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
               ILookupMap& attrLookupMap);

void
stripnamespaces(std::string& content);
}
