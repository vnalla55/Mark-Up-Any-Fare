//----------------------------------------------------------------------------
//  File:        XMLShoppingResponseESV.h
//  Created:     2008-04-16
//
//  Description: ESV response formatter
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Common/CurrencyConversionFacade.h"
#include "Common/Message.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Xform/XMLWriter.h"

#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace tse
{

class ErrorResponseException;

class XMLShoppingResponseESV
{

public:
  static bool generateXML(PricingTrx& trx, std::string& res);

private:
  typedef XMLWriter::Node Node;

  explicit XMLShoppingResponseESV(PricingTrx& trx, ErrorResponseException* ere = nullptr);

  std::string getXML();
  void getXML(std::string& res);

  void generateResponse();

  void generateError();

  void generateOND();

  void checkDuplicateThruFM(std::vector<PricingTrx::OriginDestination>& odThruFM,
                            FareMarket& fareMarket);

  void generateITN(ESVSolution* esvSolution);

  void generateSID(ESVSolution* esvSolution);

  void generateBKK(ESVOption* esvOption);

  void generatePSG(ESVSolution* esvSolution);

  void generateBKC(ESVFareComponent* esvFareComponent);

  void generateTOT(ESVSolution* esvSolution);

  std::string formatDBInfo();

  std::string getSellingCurrency();

  XMLWriter _writer;
  PricingTrx& _trx;
  const ShoppingTrx* const _shoppingTrx;
  const ErrorResponseException* const _error;
};

} // End namespace tse

