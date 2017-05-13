//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderHelp.h
//  Authors:        Piotr Lach
//  Created:        4/14/2008
//  Description:    PfcDisplayBuilder header file for ATSE V2 PFC Display Project.
//                  The object of this class build PFC Display help message.
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
#ifndef PFC_DISPLAY_BUILDER_HELP_H
#define PFC_DISPLAY_BUILDER_HELP_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"

namespace tse
{

class PfcDisplayBuilderHelp : public PfcDisplayBuilder
{
public:
  PfcDisplayBuilderHelp(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderHelp();
  std::string buildBody() override;
};

} // namespace tse
#endif
