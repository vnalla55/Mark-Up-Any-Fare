//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXM.h
//  Authors:        Piotr Lach
//  Created:        7/09/2008
//  Description:    PfcDisplayBuilderPXM header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXM message.
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
#ifndef PFC_DISPLAY_BUILDER_PXM_H
#define PFC_DISPLAY_BUILDER_PXM_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXM.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"

namespace tse
{

class PfcDisplayBuilderPXM : public PfcDisplayBuilder
{
public:
  static const std::string MAIN_HEADER_PXM;
  static const std::string TABLE_HEADER;

  PfcDisplayBuilderPXM(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXM();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXM* data() override { return (PfcDisplayDataPXM*)_data; }
  const PfcDisplayDataPXM* data() const override { return (PfcDisplayDataPXM*)_data; }
  PfcDisplayFormatterPXM& fmt() { return _formatter; }

private:
  std::string locTypeDesc(LocTypeCode locTypeCode) const;

  PfcDisplayFormatterPXM _formatter;
};

} // namespace tse
#endif
