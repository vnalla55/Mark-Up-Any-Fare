//-------------------------------------------------------------------
//
//  File:        SITAConstructedFareInfo.cpp
//  Created:     Feb 14, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents common data and members of
//               one SITA constructed fare (result of an add-on
//               construction process)
//
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
//-------------------------------------------------------------------

#include "DBAccess/SITAConstructedFareInfo.h"

#include "DBAccess/DataHandle.h"

namespace tse
{
SITAConstructedFareInfo::SITAConstructedFareInfo() : ConstructedFareInfo(false), _throughMPMInd(' ')
{
  initialize();
}

void
SITAConstructedFareInfo::initialize()
{
  _fareInfo = new SITAFareInfo;
}

ConstructedFareInfo*
SITAConstructedFareInfo::clone(DataHandle& dataHandle) const
{
  SITAConstructedFareInfo* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(*cloneObj); // lint !e413

  return dynamic_cast<ConstructedFareInfo*>(cloneObj);
}

void
SITAConstructedFareInfo::clone(SITAConstructedFareInfo& cloneObj) const
{
  cloneObj._throughFareRouting = _throughFareRouting;
  cloneObj._throughMPMInd = _throughMPMInd;
  cloneObj._throughRule = _throughRule;

  ConstructedFareInfo::clone(dynamic_cast<ConstructedFareInfo&>(cloneObj));
}

void
SITAConstructedFareInfo::dumpObject(std::ostream& os) const
{
  dumpConstructedFareInfo(os);

  os << "|" << _throughFareRouting << "|" << _throughMPMInd << "|" << _throughRule << "]";
}

void
SITAConstructedFareInfo::dummyData()
{
  ConstructedFareInfo::dummyData();
  _throughFareRouting = "PQRS";
  _throughMPMInd = 'M';
  _throughRule = "2001";
}
}
