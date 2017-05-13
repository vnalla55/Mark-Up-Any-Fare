//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Adapter/CacheNotifyControl.h"

#include <cstdio>

#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace tse;
using namespace std;
using namespace XERCES_CPP_NAMESPACE;

CacheNotifyControl::CacheNotifyControl(const std::string& fileName,
                                       std::map<std::string, std::vector<std::string>>& keyFields,
                                       std::map<std::string, std::vector<std::string>>& cacheIds,
                                       std::map<std::string, std::vector<std::string>>& hKeyFields,
                                       std::map<std::string, std::vector<std::string>>& hCacheIds)
  : _fileName(nullptr),
    _sNotification(nullptr),
    _sHistoricalNotification(nullptr),
    _sKeyField(nullptr),
    _sCacheId(nullptr),
    _keyFields(keyFields),
    _cacheIds(cacheIds),
    _historicalKeys(hKeyFields),
    _historicalIds(hCacheIds),
    _keys(nullptr),
    _ids(nullptr)
{
  try
  {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& e)
  {
    LOG4CXX_ERROR(_logger, "Could not initialize XML utilities");
    return;
  }
  LOG4CXX_INFO(_logger, "Initialized XML utilities");

  _fileName = XMLString::transcode(fileName.c_str());
  _sNotification = XMLString::transcode("Notification");
  _sHistoricalNotification = XMLString::transcode("HistoricalNotification");
  _sKeyField = XMLString::transcode("KeyField");
  _sCacheId = XMLString::transcode("CacheId");
}

CacheNotifyControl::~CacheNotifyControl()
{
  if (_fileName)
    XMLString::release(&_fileName);
  if (_sNotification)
    XMLString::release(&_sNotification);
  if (_sHistoricalNotification)
    XMLString::release(&_sHistoricalNotification);
  if (_sKeyField)
    XMLString::release(&_sKeyField);
  if (_sCacheId)
    XMLString::release(&_sCacheId);
}

bool
CacheNotifyControl::parse()
{
  SAX2XMLReader* reader = nullptr;
  try
  {
    LocalFileInputSource source(_fileName);
    reader = XMLReaderFactory::createXMLReader();
    if (reader == nullptr)
    {
      LOG4CXX_ERROR(_logger, "Could not create XML reader");
      return false;
    }

    reader->setFeature(XMLUni::fgSAX2CoreValidation, false);
    reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

    reader->setContentHandler(this);
    reader->setErrorHandler(this);

    reader->parse(source);
  }
  catch (const XMLException& e)
  {
    delete reader;
    LOG4CXX_ERROR(_logger, "Could not initialize Cache Notification Adapter");
    return false;
  }
  catch (const SAXParseException& e)
  {
    delete reader;
    LOG4CXX_ERROR(_logger, "Sax Parsing exception while parsing request");
    return false;
  }

  delete reader;
  return true;
}

void
CacheNotifyControl::startElement(const XMLCh* uri,
                                 const XMLCh* localname,
                                 const XMLCh* name,
                                 const XERCES_CPP_NAMESPACE::Attributes& attributes)
{
  char buf[256];
  unsigned int idx = 0;

  if (XMLString::compareString(name, _sNotification) == 0)
  {
    XMLString::transcode(attributes.getValue(idx), buf, 255);
    _entityType = buf;
    _keys = &_keyFields;
    _ids = &_cacheIds;
    LOG4CXX_DEBUG(_logger, "Set entity type to " << _entityType);
  }
  else if (XMLString::compareString(name, _sHistoricalNotification) == 0)
  {
    XMLString::transcode(attributes.getValue(idx), buf, 255);
    _entityType = buf;
    _keys = &_historicalKeys;
    _ids = &_historicalIds;
    LOG4CXX_DEBUG(_logger, "Set historical entity type to " << _entityType);
  }
  else if (XMLString::compareString(name, _sKeyField) == 0)
  {
    XMLString::transcode(attributes.getValue(idx), buf, 255);
    (*_keys)[_entityType].push_back(buf);
    LOG4CXX_DEBUG(_logger, " pushed key " << buf << " into " << _entityType);
  }
  else if (XMLString::compareString(name, _sCacheId) == 0)
  {
    XMLString::transcode(attributes.getValue(idx), buf, 255);
    (*_ids)[_entityType].push_back(buf);
    LOG4CXX_DEBUG(_logger, " pushed id " << buf << " into " << _entityType);
  }
}

log4cxx::LoggerPtr
CacheNotifyControl::_logger(log4cxx::Logger::getLogger("atseintl.Adapter.CacheNotifyControl"));
