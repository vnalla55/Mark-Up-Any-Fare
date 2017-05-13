//-------------------------------------------------------------------
//
//  File:     FaresHeaderSection.cpp
//  Author:   Mike Carroll
//  Date:     July 26, 2005
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/FaresHeaderSection.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Templates.FaresHeaderSection");

void
FaresHeaderSection::buildDisplay()
{
  LOG4CXX_DEBUG(logger, "In buildDisplay");
  if (_templateSegRecs == nullptr || _templateSegRecs->empty())
    return;

  if (!headerBuilt)
  {
    displayLine().clear();
    displayLine() << std::setw(62) << std::setfill(' ') << " ";

    for (const auto fareDispTemplateSeg : *_templateSegRecs)
    {
      if (fareDispTemplateSeg->headerStart() == 0)
        continue;

      LOG4CXX_DEBUG(logger,
                    "Start: " << fareDispTemplateSeg->headerStart() << ", Header: '"
                              << fareDispTemplateSeg->header() << "'");
      displayLine().seekp(fareDispTemplateSeg->headerStart() - 1, std::ios_base::beg);
      displayLine() << fareDispTemplateSeg->header();
    }
    headerBuilt = true;
  }
  LOG4CXX_DEBUG(logger, "displayLine: '" << displayLine().str() << "'");
  _trx.response() << displayLine().str() << std::endl;
}
} // tse namespace
