//----------------------------------------------------------------------------
//  File:        XMLRexShoppingResponse.h
//  Created:     2009-01-26
//
//  Description: RexShopping response formatter
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "Common/TseStringTypes.h"
#include "DataModel/RexShoppingTrx.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Xform/XMLWriter.h"

#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace tse
{

//--------------------------------------------------------------------
class ErrorResponseException;
class XMLRexShoppingResponse
{
public:
  static bool generateXML(PricingTrx& trx, std::string& res);

private:
  typedef XMLWriter::Node Node;

  explicit XMLRexShoppingResponse(PricingTrx& trx, ErrorResponseException* ere = nullptr);

  std::string getXML();
  void getXML(std::string& res);

  void generateResponse();

  void generateError();

  void generateOAD(const RexShoppingTrx::OADConsolidatedConstrainsVect& oadConsRest);
  void generateFCL();
  void generateFCI(const PaxTypeFare* ptf);
  void generateVCTR(const PaxTypeFare* ptf, const FareMarket* fareMarket);
  void generateR3I(const PaxTypeFare* ptf);

  std::string formatDBInfo();

  XMLWriter _writer;
  PricingTrx& _trx;
  RexShoppingTrx* _rexShoppingTrx;
  const ErrorResponseException* const _error;
};

} // End namespace tse

