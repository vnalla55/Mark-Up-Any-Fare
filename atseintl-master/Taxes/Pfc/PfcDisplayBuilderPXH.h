//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXH.h
//  Authors:        Piotr Lach
//  Created:        6/23/2008
//  Description:    PfcDisplayBuilderPXH header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXH message.
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
#ifndef PFC_DISPLAY_BUILDER_PXH_H
#define PFC_DISPLAY_BUILDER_PXH_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXH.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"

namespace tse
{

class PfcDisplayBuilderPXH : public PfcDisplayBuilder
{
public:
  static const std::string MAIN_HEADER_PXH;
  static const std::string TABLE_HEADER;

  PfcDisplayBuilderPXH(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXH();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXH* data() override { return (PfcDisplayDataPXH*)_data; }
  const PfcDisplayDataPXH* data() const override { return (PfcDisplayDataPXH*)_data; }
  PfcDisplayFormatterPXH& fmt() { return _formatter; }

private:
  PfcDisplayFormatterPXH _formatter;
};

} // namespace tse
#endif
