//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilder.h
//  Authors:        Piotr Lach
//  Created:        4/17/2008
//  Description:    PfcDisplayDBProxy header file for ATSE V2 PFC Display Project.
//                  DB Proxy for PFC Display functionality.
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
#ifndef PFC_DISPLAY_DATA_PXC_H
#define PFC_DISPLAY_DATA_PXC_H

#include "Taxes/Pfc/PfcDisplayData.h"
#include "Common/TseConsts.h"

#include <string>
#include <vector>
#include <map>

namespace tse
{

class PfcDisplayDataPXC : public PfcDisplayData
{

public:
  static const LocTypeCode LOCTYPE_AIRPORT = 'A';
  static const LocTypeCode LOCTYPE_CITY = 'C';
  static const LocTypeCode LOCTYPE_AIRPORT_AND_CITY = 'B';

  static const uint32_t WRONG_SEGMENT_POS = 0xFFFFFFFF;

  typedef std::map<LocCode, std::string> WarningMap;

  PfcDisplayDataPXC(TaxTrx* trx, PfcDisplayDb* db);
  virtual ~PfcDisplayDataPXC();

  virtual void getPfcPFC(std::vector<PfcPFC*>& pfcV, WarningMap& warningMap) const;
  CurrencyCode getCustomerCurrency() const;
  LocCode getCustomerCity() const;
  virtual std::string getEquivalentAmount(MoneyAmount& amt) const;
  virtual bool isPfcAbsorb(LocCode& pfcAirport, uint32_t segmentPos = WRONG_SEGMENT_POS) const;
  virtual bool isPfcEssAirSvc(LocCode& pfcAirport, uint32_t segmentPos = WRONG_SEGMENT_POS) const;

  bool isPNR() { return trx()->pfcDisplayRequest()->isPNR(); }
  bool isPNR() const { return trx()->pfcDisplayRequest()->isPNR(); }

private:
  void updateWarningMap(LocCode& arpt, std::vector<PfcPFC*>& pfcV, WarningMap& warningMap) const;
  void pushBackPfcData(LocCode& arpt,
                       std::vector<PfcPFC*>& pfcRawV,
                       std::vector<PfcPFC*>& pfcV,
                       WarningMap& warningMap) const;
};

} // namespace tse
#endif
