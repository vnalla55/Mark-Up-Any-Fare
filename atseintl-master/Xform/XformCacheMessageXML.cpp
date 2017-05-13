//----------------------------------------------------------------------------
//
//      File: XformCacheMessageXML.cpp
//      Description: Implementation of class to transform client XML to/from Trx
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
//----------------------------------------------------------------------------

#include "Xform/XformCacheMessageXML.h"

#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "DataModel/CacheTrx.h"
#include "Server/TseServer.h"
#include "Xform/CacheMessageContentHandler.h"
#include "Xform/CacheMessageXMLParser.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace tse;
using namespace XERCES_CPP_NAMESPACE;

static LoadableModuleRegister<Xform, XformCacheMessageXML>
_("libXformCacheMessage.so");

static Logger
logger("atseintl.Xform.XformCacheMessageXML");

XformCacheMessageXML::XformCacheMessageXML(const std::string& name, ConfigMan& config)
  : Xform(name, config)
{
}

XformCacheMessageXML::XformCacheMessageXML(const std::string& name, TseServer& srv)
  : Xform(name, srv.config())
{
}

XformCacheMessageXML::~XformCacheMessageXML() {}

bool
XformCacheMessageXML::convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool /*throttled*/)
{
  CacheTrx* cacheTrx;
  dataHandle.get(cacheTrx); // lint !e530
  trx = cacheTrx;

  CacheMessageXMLParser parser(*cacheTrx, dataHandle);
  CacheMessageContentHandler contentHandler(parser);

  return contentHandler.parse(request.c_str());
}

bool
XformCacheMessageXML::convert(Trx& trx, std::string& response)
{
  CacheTrx* const cacheTrx = dynamic_cast<CacheTrx*>(&trx);
  if (cacheTrx != nullptr)
  {
    // the cache messages are fire-and-forget; they don't expect a response
    response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    return true;
  }
  else
  {
    LOG4CXX_ERROR(logger, "Unexpected transaction type in convert");
    return false;
  }
}

bool
XformCacheMessageXML::convert(ErrorResponseException& ere, Trx& /*trx*/, std::string& response)
{
  return convert(ere, response);
}

bool
XformCacheMessageXML::convert(ErrorResponseException& /*ere*/, std::string& response)
{
  // the cache messages are fire-and-forget; they don't expect a response
  response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

  return true;
}

bool
XformCacheMessageXML::initialize(int argc, char** argv)
{
  return true;
}
