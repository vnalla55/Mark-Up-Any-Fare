//----------------------------------------------------------------------------
//  File:        TaxShoppingResponseFormatter.h
//  Created:     2012-05-28
//
//  Description: Shopping formatter for charger tax requests
//
//  Updates:
//
//  Copyright Sabre 2012
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

#include "Common/ErrorResponseException.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <map>
#include <string>
#include <vector>

namespace tse
{

class TaxShoppingResponseFormatter
{
public:
  TaxShoppingResponseFormatter(TaxTrx* taxTrx);
  virtual ~TaxShoppingResponseFormatter();
  void formatResponse();
  void formatResponse(ErrorResponseException& ere);
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

private:
  struct ItinInfo
  {
    ItinInfo() : _itin(nullptr), _totalTaxAmount(0.0), _taxIdVec() {}

    const Itin* _itin;
    MoneyAmount _totalTaxAmount;
    std::vector<int32_t> _taxIdVec;
  };

  struct TaxInfo
  {
    TaxInfo() : _taxRecord(nullptr), _id(-1) {}

    const TaxRecord* _taxRecord;
    int32_t _id;
  };

  void buildTaxMap();
  std::string buildTaxKey(const TaxRecord* taxRecord);
  void generateTAX();
  void generateCOM(const ItinInfo* itinInfo, const Itin* itin);
  void generatePXI(const ItinInfo* itinInfo);
  void generateDIA();
  void generateWAR();

  TaxTrx* _taxTrx;
  std::map<std::string, TaxInfo> _taxMap;
  std::vector<TaxRecord*> _taxVec;
  std::vector<ItinInfo*> _itinsVec;
};
}

