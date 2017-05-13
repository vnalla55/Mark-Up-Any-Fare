//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXT.h
//  Authors:        Jakub Kubica
//  Created:        July/03/2008
//  Description:    PfcDisplayBuilderPXT header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXT message.
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
#ifndef PFC_DISPLAY_BUILDER_PXT_H
#define PFC_DISPLAY_BUILDER_PXT_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXT.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"
#include "DBAccess/PfcCollectExcpt.h"

namespace tse
{

class PfcDisplayBuilderPXT : public PfcDisplayBuilder
{
public:
  static const std::string TABLE_HEADER;
  static const std::string TABLE_HEADER2;
  static const std::string TABLE_HEADER_FORMAT;

  virtual ~PfcDisplayBuilderPXT();
  PfcDisplayBuilderPXT(TaxTrx* trx, PfcDisplayData* data);

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXT* data() override { return (PfcDisplayDataPXT*)_data; }
  const PfcDisplayDataPXT* data() const override { return (PfcDisplayDataPXT*)_data; }

  PfcDisplayFormatterPXT& fmt() { return _formatter; }

private:
  PfcDisplayFormatterPXT _formatter;
};

} // tse

#endif
