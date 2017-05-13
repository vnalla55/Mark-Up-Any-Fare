//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilder.h
//  Authors:        Piotr Lach
//  Created:        2/18/2008
//  Description:    PfcDisplayBuilder header file for ATSE V2 PFC Display Project.
//                  Interface to PfcDisplayBuilder functionality.
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
#ifndef PFC_DISPLAY_BUILDER_H
#define PFC_DISPLAY_BUILDER_H

#include "DataModel/TaxTrx.h"
#include "Taxes/Pfc/PfcDisplayData.h"
#include <string>

namespace tse
{

class PfcDisplayData;

class PfcDisplayBuilder
{
public:
  static const std::string MAIN_HEADER;
  static const std::string N_A_DATE;

  PfcDisplayBuilder(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilder();
  virtual std::string build();

protected:
  virtual std::string buildHeader();
  virtual std::string buildBody();
  virtual std::string buildFootnote();

  TaxTrx* trx() { return _trx; }
  const TaxTrx* trx() const { return _trx; }

  virtual PfcDisplayData* data() { return _data; }
  virtual const PfcDisplayData* data() const { return _data; }

protected:
  TaxTrx* _trx;
  PfcDisplayData* _data;
};

} // namespace tse
#endif
