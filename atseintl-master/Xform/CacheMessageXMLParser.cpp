//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#include "Xform/CacheMessageXMLParser.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/XMLChString.h"
#include "DBAccess/ObjectKey.h"

namespace tse
{
static Logger
logger("atseintl.Xform.CacheMessageXMLParser");

CacheMessageXMLParser::CacheMessageXMLParser(CacheTrx& trx, DataHandle& dataHandle)
  : _cacheWarmUpParser(*this),
    _cacheUpdateParser(*this),
    _objectParser(*this),
    _keyParser(*this),
    _nameParser(*this),
    _fieldParser(*this),
    _trx(trx),
    _dataHandle(dataHandle),
    _objectKey(),
    _fieldId()
{
}

CacheMessageXMLParser::~CacheMessageXMLParser() {}

void
CacheMessageXMLParser::startElement(const XMLCh* element, const xercesc::Attributes& attributes)
{
  if (_parsers.empty())
  {
    const XMLChString elname(element);

    if (elname == "CacheUpdate")
    {
      _parsers.push(&_cacheUpdateParser);
    }
    else if (elname == "CacheWarmUp")
    {
      _parsers.push(&_cacheWarmUpParser);
    }
    else
    {
      // unrecognized element
      _parsers.push(nullptr);
    }
  }
  else if (ElementParser* parser = _parsers.top())
  {
    _parsers.push(parser->newElement(element, attributes));
  }
  else
  {
    _parsers.push(nullptr);
  }

  if (_parsers.top() != nullptr)
  {
    _parsers.top()->startParsing(attributes);
  }
}

void
CacheMessageXMLParser::endElement(const XMLCh* element, const XMLCh* text)
{
  if (_parsers.top() != nullptr)
  {
    _parsers.top()->endParsing(text);
  }
  _parsers.pop();
}

// ElementParser --------------------------------------------------------------

CacheMessageXMLParser::ElementParser::ElementParser(CacheMessageXMLParser& parser) : _parser(parser)
{
}

CacheMessageXMLParser::ElementParser::~ElementParser() {}

CacheTrx&
CacheMessageXMLParser::ElementParser::trx()
{
  return _parser._trx;
}

DataHandle&
CacheMessageXMLParser::ElementParser::dataHandle()
{
  return _parser._dataHandle;
}

// CacheWarmUpParser -----------------------------------------------------------

CacheMessageXMLParser::CacheWarmUpParser::CacheWarmUpParser(CacheMessageXMLParser& parser)
  : CacheMessageXMLParser::ElementParser(parser)
{
}

CacheMessageXMLParser::CacheWarmUpParser::~CacheWarmUpParser() {}

CacheMessageXMLParser::ElementParser*
CacheMessageXMLParser::CacheWarmUpParser::newElement(const XMLCh* element,
                                                     const xercesc::Attributes& attributes)
{
  const XMLChString elname(element);
  if (elname == "Object")
  {
    return &parser()._objectParser;
  }
  return nullptr;
}

// CacheUpdateParser -----------------------------------------------------------

CacheMessageXMLParser::CacheUpdateParser::CacheUpdateParser(CacheMessageXMLParser& parser)
  : CacheMessageXMLParser::ElementParser(parser)
{
}

CacheMessageXMLParser::CacheUpdateParser::~CacheUpdateParser() {}

CacheMessageXMLParser::ElementParser*
CacheMessageXMLParser::CacheUpdateParser::newElement(const XMLCh* element,
                                                     const xercesc::Attributes& attributes)
{
  const XMLChString elname(element);
  if (elname == "Object")
  {
    return &parser()._objectParser;
  }
  return nullptr;
}

// ObjectParser ---------------------------------------------------------------

CacheMessageXMLParser::ObjectTypeParser::ObjectTypeParser(CacheMessageXMLParser& parser)
  : CacheMessageXMLParser::ElementParser(parser)
{
}

CacheMessageXMLParser::ObjectTypeParser::~ObjectTypeParser() {}

void
CacheMessageXMLParser::ObjectTypeParser::startParsing(const xercesc::Attributes& attributes)
{
  ObjectKey& objectKey = parser()._objectKey;

  objectKey.tableName().clear();
  objectKey.keyFields().clear();
}

void
CacheMessageXMLParser::ObjectTypeParser::endParsing(const XMLCh* text)
{
  ObjectKey& objectKey = parser()._objectKey;
  if (objectKey.tableName().empty())
  {

    LOG4CXX_WARN(logger,
                 "Cache update message parse error: "
                 "Object has no name; ignored");
    return;
  }
  if (objectKey.keyFields().empty())
  {
    LOG4CXX_WARN(logger,
                 "Cache update message parse error: "
                 "Object has no valid Keys; ignored");

    return;
  }
  trx().push_back(CACHE_UPDATE_INVALIDATE, objectKey);
}

CacheMessageXMLParser::ElementParser*
CacheMessageXMLParser::ObjectTypeParser::newElement(const XMLCh* element,
                                                    const xercesc::Attributes& attributes)
{
  const XMLChString elname(element);
  if (elname == "Name")
  {
    return &parser()._nameParser;
  }
  else if (elname == "Key")
  {
    return &parser()._keyParser;
  }
  return nullptr;
}

// NameTypeParser --------------------------------------------------------------

CacheMessageXMLParser::NameTypeParser::NameTypeParser(CacheMessageXMLParser& parser)
  : CacheMessageXMLParser::ElementParser(parser)
{
}

CacheMessageXMLParser::NameTypeParser::~NameTypeParser() {}

void
CacheMessageXMLParser::NameTypeParser::endParsing(const XMLCh* text)
{
  // Create text string object
  const XMLChString textStr(text);

  ObjectKey& objectKey = parser()._objectKey;
  if (!objectKey.tableName().empty())
    LOG4CXX_WARN(logger,
                 "Cache update message parse error:"
                 "Object has two Names"
                     << objectKey.tableName().c_str() << "&" << textStr.c_str());
  textStr.parse(objectKey.tableName());
}

// KeyTypeParser ---------------------------------------------------------------

CacheMessageXMLParser::KeyTypeParser::KeyTypeParser(CacheMessageXMLParser& parser)
  : CacheMessageXMLParser::ElementParser(parser)
{
}

CacheMessageXMLParser::KeyTypeParser::~KeyTypeParser() {}

CacheMessageXMLParser::ElementParser*
CacheMessageXMLParser::KeyTypeParser::newElement(const XMLCh* element,
                                                 const xercesc::Attributes& attributes)
{
  const XMLChString elname(element);
  if (elname == "Field")
  {
    return &parser()._fieldParser;
  }
  return nullptr;
}

// FieldTypeParser -------------------------------------------------------------

CacheMessageXMLParser::FieldTypeParser::FieldTypeParser(CacheMessageXMLParser& parser)
  : CacheMessageXMLParser::ElementParser(parser)
{
}

CacheMessageXMLParser::FieldTypeParser::~FieldTypeParser() {}

void
CacheMessageXMLParser::FieldTypeParser::startParsing(const xercesc::Attributes& attributes)
{
  const XMLAttributes attr(attributes);
  attr["id"].parse(parser()._fieldId);
}

void
CacheMessageXMLParser::FieldTypeParser::endParsing(const XMLCh* text)
{
  if (!parser()._fieldId.empty())
  {
    const XMLChString textStr(text);

    ObjectKey& objectKey = parser()._objectKey;
    objectKey.keyFields()[parser()._fieldId] = textStr.c_str();
  }
}
} // end namespace tse
