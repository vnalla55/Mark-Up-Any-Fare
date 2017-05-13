//----------------------------------------------------------------------------
//  File:           PfcDisplayData.h
//  Authors:        Piotr Lach
//  Created:        4/17/2008
//  Description:    PfcDisplaydata header file for ATSE V2 PFC Display Project.
//                  Common data for PFC Display functionality.
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
#ifndef PFC_DISPLAY_DATA_H
#define PFC_DISPLAY_DATA_H

#include "Taxes/Pfc/PfcDisplayDb.h"
#include "Common/TseConsts.h"

#include <string>
#include <vector>
#include <map>

namespace tse
{

class PfcDisplayData
{
public:
  static const std::string AXESS_PREFIX;
  static const std::string HDQ;

  PfcDisplayData(TaxTrx* trx, PfcDisplayDb* db);
  virtual ~PfcDisplayData();

  std::string getAxessPrefix() const;

protected:
  const TaxTrx* trx() const { return _trx; }
  const PfcDisplayDb* db() const { return _db; }

private:
  TaxTrx* _trx;
  PfcDisplayDb* _db;
};

} // namespace tse
#endif
