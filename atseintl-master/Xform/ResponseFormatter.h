//----------------------------------------------------------------------------
//
//      File: ResponseFormatter.h
//      Description: Base class for formatting any kind of response (Pricing, FareDispaly, ...)
//      Created: Nov 9, 2006
//      Authors: Jeff Hoffman
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

#include "Common/Message.h"
#include "DataModel/Trx.h"
#include "Diagnostic/Diagnostic.h"

class XMLConstruct;

namespace tse
{
class ResponseFormatter
{
public:
  virtual ~ResponseFormatter() = default;

protected:
  virtual bool hostDiagString(std::vector<std::string>& hostMsg) const;
  virtual void buildDiagString(std::vector<std::string>& buildMsg) const;
  bool configDiagString(std::vector<std::string>& configMsgVec, Trx& trx, bool force = false) const;
  virtual void dbDiagString(std::vector<std::string>& dbMsg) const;
  void buildDiag854(XMLConstruct& construct, int& recNum) const;
  static void addMessageLine(const std::string& line,
                             XMLConstruct& construct,
                             const std::string& msgType,
                             int recNum);
}; // End class ResponseFormatter
} // End namespace tse
