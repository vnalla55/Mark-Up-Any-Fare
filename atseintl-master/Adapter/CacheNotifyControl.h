//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Logger.h"

#include <map>
#include <string>
#include <vector>

#include <xercesc/sax2/DefaultHandler.hpp>

namespace tse
{
class CacheNotifyControl : public XERCES_CPP_NAMESPACE::DefaultHandler
{

public:
  CacheNotifyControl(const std::string& fileName,
                     std::map<std::string, std::vector<std::string>>& keyFields,
                     std::map<std::string, std::vector<std::string>>& cacheIds,
                     std::map<std::string, std::vector<std::string>>& hKeyFields,
                     std::map<std::string, std::vector<std::string>>& hCacheIds);

  virtual ~CacheNotifyControl();

  bool parse();

private:
  void startElement(const XMLCh* uri,
                    const XMLCh* localname,
                    const XMLCh* name,
                    const XERCES_CPP_NAMESPACE::Attributes& attributes) override;

  std::string _entityType;
  XMLCh* _fileName;
  XMLCh* _sNotification;
  XMLCh* _sHistoricalNotification;
  XMLCh* _sKeyField;
  XMLCh* _sCacheId;

  static log4cxx::LoggerPtr _logger;
  std::map<std::string, std::vector<std::string> >& _keyFields;
  std::map<std::string, std::vector<std::string> >& _cacheIds;
  std::map<std::string, std::vector<std::string> >& _historicalKeys;
  std::map<std::string, std::vector<std::string> >& _historicalIds;
  std::map<std::string, std::vector<std::string> >* _keys;
  std::map<std::string, std::vector<std::string> >* _ids;
};
}
