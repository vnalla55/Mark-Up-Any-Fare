//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXA.h
//  Authors:        Jakub Kubica
//  Created:        6/11/2008
//  Description:    PfcDisplayBuilderPXQ header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXQ message.
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
#ifndef PFC_DISPLAY_BUILDER_PXQ_H
#define PFC_DISPLAY_BUILDER_PXQ_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXQ.h"

#include "Taxes/Pfc/PfcDisplayFormatter.h"
#include "DBAccess/PfcEquipTypeExempt.h"

namespace tse
{

class PfcDisplayBuilderPXQ : public PfcDisplayBuilder
{
public:
  static const std::string TABLE_HEADER;
  static const std::string TABLE_HEADER2;
  static const std::string TABLE_FIELDPOS;

  virtual ~PfcDisplayBuilderPXQ();
  PfcDisplayBuilderPXQ(TaxTrx* trx, PfcDisplayData* data);

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXQ* data() override { return (PfcDisplayDataPXQ*)_data; }
  const PfcDisplayDataPXQ* data() const override { return (PfcDisplayDataPXQ*)_data; }

  PfcDisplayFormatterPXQ& fmt() { return _formatter; }

private:
  PfcDisplayFormatterPXQ _formatter;
};

} // tse

#endif
