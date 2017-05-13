//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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
#include "Diagnostic/Diag259Collector.h"

#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DiagRequest.h"
#include "AddonConstruction/GatewayPair.h"

#include <iomanip>

namespace tse
{
const char* GW_TO_RECONSTRUCT_TITLE_MSG = "RECONSTRUCTED GATEWAY PAIRS";
const char* GW_TO_RECONSTRUCT_TITLE2_MSG = "GATEWAY1  GATEWAY2  \n";

void
Diag259Collector::writeGWPairToReconstructHeader()
{
  if (!_active)
    return;

  writeCommonHeader(_cJob->vendorCode());

  (*this) << " \n" << GW_TO_RECONSTRUCT_TITLE_MSG << '\n' << SEPARATOR
          << GW_TO_RECONSTRUCT_TITLE2_MSG << SEPARATOR;
}

void
Diag259Collector::writeGWPairToReconstruct(const GatewayPair& gw)
{
  if (!_active)
    return;

  if (_numGWPairToReconstruct == 0)
    writeGWPairToReconstructHeader();

  (*this) << gw.gateway1();

  if (gw.gateway1() != gw.multiCity1())
    (*this) << '/' << gw.multiCity1() << '/';
  else
    (*this) << "     ";

  (*this) << "  " << gw.gateway2();

  if (gw.gateway2() != gw.multiCity2())
    (*this) << '/' << gw.multiCity2() << '/';

  (*this) << '\n';

  _numGWPairToReconstruct++;
}

void
Diag259Collector::writeGWPairToReconstructFooter()
{
  if (_active && _numGWPairToReconstruct != 0)
    (*this) << SEPARATOR;

  _numGWPairToReconstruct = 0;
}
}
