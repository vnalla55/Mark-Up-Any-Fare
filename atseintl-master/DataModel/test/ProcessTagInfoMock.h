//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#ifndef PROCESSTAGINFOMOCK_H
#define PROCESSTAGINFOMOCK_H

#include "DataModel/FareMarket.h"
#include "DataModel/ProcessTagInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"

namespace tse
{

class ProcessTagInfoMock : public ProcessTagInfo
{
public:
  ProcessTagInfoMock()
  {
    record3()->orig() = &_vci;
    reissueSequence()->orig() = &_rs;
    fareCompInfo() = &_fci;
    fareCompInfo()->fareMarket() = &_fm;
  }

  template <Indicator& (ReissueSequence::*set_method)()>
  ProcessTagInfoMock* setRS(Indicator byte)
  {
    (_rs.*set_method)() = byte;
    return this;
  }

  template <Indicator& (VoluntaryChangesInfo::*set_method)()>
  ProcessTagInfoMock* setVCI(Indicator byte)
  {
    (_vci.*set_method)() = byte;
    return this;
  }

protected:
  VoluntaryChangesInfo _vci;
  ReissueSequence _rs;
  FareCompInfo _fci;
  FareMarket _fm;
};

typedef ProcessTagInfoMock* (ProcessTagInfoMock::*Seter)(Indicator);

} // tse

#endif // PROCESSTAGINFOMOCK_H
