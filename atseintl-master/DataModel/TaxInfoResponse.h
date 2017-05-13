//----------------------------------------------------------------------------
//  File:           TaxInfoResponse.h
//  Authors:        Piotr Lach
//  Created:        12/12/2008
//  Description:    TaxInfoResponse tax file for ATSE V2 PFC Display Project.
//                  TaxInfo Response containers.
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

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace tse
{

class TaxInfoResponse
{
public:
  struct TAX
  {
    enum
    {
      CODE,
      ERROR,
      TYPE,
      AMOUNT,
      CURRENCY,
      NATION,
      DESCRIPTION,
      REFUNDABLE
    };
  };

  struct PFC
  {
    enum
    {
      AIRPORT,
      ERROR,
      AMOUNT,
      CURRENCY,
      EFF_DATE,
      DISC_DATE
    };
  };

  struct ZP
  {
    enum
    {
      AIRPORT,
      ERROR,
      AMOUNT,
      CURRENCY,
      IS_DOMESTIC,
      IS_RURAL
    };
  };

  TaxInfoResponse();
  ~TaxInfoResponse();

  void initPFC();

  void initZP();

  typedef std::tuple<std::string, // Tax Code
                     std::string, // Error
                     std::string, // Tax Type
                     std::string, // Amount
                     std::string, // Currency Code
                     std::string, // Tax Country Code
                     std::string, // Tax Description
                     std::string // Refundable tag
                     > Tax;

  typedef std::tuple<std::string, std::string, std::string, std::string, std::string, std::string>
    AptItem;

  typedef std::vector<AptItem> Apt;

  Tax& taxAttrNames() { return _taxAttrNames; }
  Tax& taxAttrValues() { return _taxAttrValues; }

  AptItem& aptAttrNames() { return _aptAttrNames; }
  Apt& aptAttrValues() { return _aptAttrValues; }

  Tax _taxAttrNames;
  Tax _taxAttrValues;

  AptItem _aptAttrNames;
  Apt _aptAttrValues;
};

} // namespace tse
