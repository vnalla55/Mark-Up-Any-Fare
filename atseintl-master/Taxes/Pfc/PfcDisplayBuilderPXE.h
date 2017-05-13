//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXE.h
//  Authors:        Piotr Lach
//  Created:        6/23/2008
//  Description:    PfcDisplayBuilderPXE header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXE message.
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
#ifndef PFC_DISPLAY_BUILDER_PXE_H
#define PFC_DISPLAY_BUILDER_PXE_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXE.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"

namespace tse
{

class PfcDisplayBuilderPXE : public PfcDisplayBuilder
{
public:
  static const std::string MAIN_HEADER_PXE;
  static const std::string TABLE_HEADER;
  static const std::string IATA_SPECIAL_CARRIER_SIGN_DB;
  static const std::string IATA_SPECIAL_CARRIER_SIGN_DISPLAY;
  static const std::string ALL_FLIGHTS_AVAILABLE;

  PfcDisplayBuilderPXE(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXE();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXE* data() override { return (PfcDisplayDataPXE*)_data; }
  const PfcDisplayDataPXE* data() const override { return (PfcDisplayDataPXE*)_data; }
  PfcDisplayFormatterPXE& fmt() { return _formatter; }

private:
  PfcDisplayFormatterPXE _formatter;
};

} // namespace tse
#endif
