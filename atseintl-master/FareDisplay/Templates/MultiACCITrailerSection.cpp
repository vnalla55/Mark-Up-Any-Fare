//-------------------------------------------------------------------
//
//  File:        MultiACCITrailerSection.cpp
//  Description: Builds the trailer message section for multi account
//               code and/or corporate ID's FareDisplay response
//
//
//  Copyright Sabre 2008
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/MultiACCITrailerSection.h"

#include "DataModel/FareDisplayRequest.h"

#include <string>
#include <vector>

namespace tse
{

void
MultiACCITrailerSection::buildDisplay()
{
  const std::vector<std::string>& vec = _trx.getRequest()->incorrectCorpIdVec();
  if (!vec.empty())
  {
    _trx.response() << " " << std::endl << "INVALID CORPORATE ID ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(_trx.response(), " "));
    _trx.response() << std::endl;
  }
}
}
