//----------------------------------------------------------------------------
//
//      File: XformCacheMessageXML.h
//      Description: Class to transform client XML to/from Trx
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

#pragma once

#include "Xform/Xform.h"


namespace tse
{
class TseServer;
class XformCacheMessageXML;

class XformCacheMessageXML : public Xform
{
public:
  friend class XformCacheMessageXMLTest;

  XformCacheMessageXML(const std::string& name, ConfigMan& config);
  XformCacheMessageXML(const std::string& name, TseServer& srv);

  virtual ~XformCacheMessageXML();

  virtual bool convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled) override;
  virtual bool convert(Trx& trx, std::string& response) override;
  virtual bool convert(ErrorResponseException& ere, Trx& trx, std::string& response) override;
  virtual bool convert(ErrorResponseException& ere, std::string& response) override;

  virtual bool initialize(int argc, char** argv) override;

};
}

