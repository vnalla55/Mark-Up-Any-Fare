
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordatance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"

using namespace tse;

const std::string PfcDisplayBuilder::MAIN_HEADER = "PASSENGER FACILITY CHARGES\n";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::PfcDisplayBuilder
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilder::PfcDisplayBuilder(TaxTrx* trx, PfcDisplayData* data) : _trx(trx), _data(data) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::~PfcDisplayBuilder
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilder::~PfcDisplayBuilder() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::build
//
// Description:  PFC Display entry build.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilder::build()
{
  return data()->getAxessPrefix() + buildHeader() + buildBody() + buildFootnote();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::buildHeader
//
// Description:  PFC Display entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilder::buildHeader()
{
  return std::string();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::buildBody
//
// Description:  PFC Display entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilder::buildBody()
{
  return std::string();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::buildFootnote
//
// Description:  PFC Display entry footnote.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilder::buildFootnote()
{
  return std::string();
}
