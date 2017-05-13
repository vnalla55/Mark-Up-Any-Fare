//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/Shopping/IBF/IbfIsStatusWriter.h"

#include "Common/Assert.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/Shopping/IBF/V2IbfManager.h"

#include <sstream>

using namespace std;

namespace tse
{

IbfIsStatusWriter::IbfIsStatusWriter(ShoppingTrx& trx, const V2IbfManager& mgr)
  : _trx(trx), _mgr(mgr)
{
}

void
IbfIsStatusWriter::writeIsStatus(DiagnosticTypes diagType, const std::string& header)
{
  DiagManager diag(_trx, diagType);
  if (diag.isActive())
  {
    ostringstream out;
    formatAndWriteMsg(out, header);
    diag << out.str();
  }
}

void
IbfIsStatusWriter::formatAndWriteMsg(std::ostream& out, const std::string& header)
{
  out << endl;

  out << "Itinerary Selector: " << header << endl;
  out << "-------------------------------------------------" << endl;
  out << "Number of requested solutions:            " << _mgr.getRequestedNbrOfSolutions() << endl;
  out << "Number of generated direct FOS solutions: " << _mgr.getDirectFosSolutions().size()
      << endl;
  out << "SOP usage (unused/total):                 " << _mgr.getNbrOfUnusedSops() << "/"
      << _mgr.getNbrOfSops() << endl;
  out << "-------------------------------------------------" << endl;

  if (areDetailsEnabled())
  {
    out << "Detailed SOP usage: " << endl;
    out << _mgr.sopUsageToString() << endl;

    out << endl;
    out << "SWAPPER STATUS" << endl;
    out << "Each flight option is represented in the Item column" << endl;
    out << "as a sequence of SOP ids, one SOP id per leg." << endl;
    out << endl;
    out << "Appraiser scores format:" << endl;
    out << endl;
    swp::printBasShortFormatDescription(out);
    out << endl;
    out << _mgr.swapperToString() << endl;
  }
}

bool
IbfIsStatusWriter::areDetailsEnabled()
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "DETAILS";
}

} // namespace tse
