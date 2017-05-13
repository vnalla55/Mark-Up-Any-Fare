//----------------------------------------------------------------------------
//
//  File:        FareTypeMatrixUtil.cpp
//  Created:     12/02/2004
//  Authors:     Mark Kasprowicz
//
//  Description: Common functions required for FareTypeMatrix
//
//  Copyright Sabre 2004
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

#include "Common/FareTypeMatrixUtil.h"

#include "Common/TseConsts.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace tse
{
boost::mutex FareTypeMatrixUtil::_mapMutex;
std::map<FareTypeDesignator::FareTypeDesignatorNew, std::string> FareTypeMatrixUtil::_fdToStr;

std::map<std::string, FareTypeDesignator::FareTypeDesignatorNew> FareTypeMatrixUtil::_strToFd;

FareTypeDesignator
FareTypeMatrixUtil::convert(const std::string& fdDiagValue)
{
  loadMaps();
  FareTypeDesignator ret;
  std::map<std::string, FareTypeDesignator::FareTypeDesignatorNew>::const_iterator i =
      _strToFd.find(fdDiagValue);
  if (i == _strToFd.end())
    ret.setFareTypeDesignator(FareTypeDesignator::FTD_UNKNOWN);
  else
    ret.setFareTypeDesignator(i->second);
  return ret;
}

const std::string&
FareTypeMatrixUtil::convert(const FareTypeDesignator& fd)
{
  loadMaps();

  FareTypeDesignator::FareTypeDesignatorNew des =
      (FareTypeDesignator::FareTypeDesignatorNew)fd.fareTypeDesig();
  std::map<FareTypeDesignator::FareTypeDesignatorNew, std::string>::const_iterator i =
      _fdToStr.find(des);
  if (i == _fdToStr.end())
    return EMPTY_STRING();

  return i->second;
}

void
FareTypeMatrixUtil::loadMaps()
{
  boost::lock_guard<boost::mutex> g(_mapMutex);
  if (_fdToStr.empty())
  {
    // FTD_UNKNOWN -- Dont need
    _fdToStr[FareTypeDesignator::FTD_FIRST] = FD_FIRST;
    _fdToStr[FareTypeDesignator::FTD_BUSINESS] = FD_BUSINESS;
    _fdToStr[FareTypeDesignator::FTD_ECONOMY] = FD_ECONOMY;
    _fdToStr[FareTypeDesignator::FTD_EXCURSION] = FD_EXCURSION;
    _fdToStr[FareTypeDesignator::FTD_ONEWAY_ADVANCE_PURCHASE] = FD_ONEWAY_ADVANCE_PURCHASE;
    _fdToStr[FareTypeDesignator::FTD_ROUNDTRIP_ADVANCE_PURCHASE] = FD_ROUNDTRIP_ADVANCE_PURCHASE;
    _fdToStr[FareTypeDesignator::FTD_ONEWAY_INSTANT_PURCHASE] = FD_ONEWAY_INSTANT_PURCHASE;
    _fdToStr[FareTypeDesignator::FTD_ROUNDTRIP_INSTANT_PURCHASE] = FD_ROUNDTRIP_INSTANT_PURCHASE;
    _fdToStr[FareTypeDesignator::FTD_SPECIAL] = FD_SPECIAL;
    _fdToStr[FareTypeDesignator::FTD_ADDON] = FD_ADDON;
    _fdToStr[FareTypeDesignator::FTD_PROMOTIONAL] = FD_PROMOTIONAL;
    _fdToStr[FareTypeDesignator::FTD_PREMIUM_FIRST] = FD_PREMIUM_FIRST;
    _fdToStr[FareTypeDesignator::FTD_PREMIUM_ECONOMY] = FD_PREMIUM_ECONOMY;

    // FTD_UNKNOWN -- Dont need
    _strToFd[FD_FIRST] = FareTypeDesignator::FTD_FIRST;
    _strToFd[FD_BUSINESS] = FareTypeDesignator::FTD_BUSINESS;
    _strToFd[FD_ECONOMY] = FareTypeDesignator::FTD_ECONOMY;
    _strToFd[FD_EXCURSION] = FareTypeDesignator::FTD_EXCURSION;
    _strToFd[FD_ONEWAY_ADVANCE_PURCHASE] = FareTypeDesignator::FTD_ONEWAY_ADVANCE_PURCHASE;
    _strToFd[FD_ROUNDTRIP_ADVANCE_PURCHASE] = FareTypeDesignator::FTD_ROUNDTRIP_ADVANCE_PURCHASE;
    _strToFd[FD_ONEWAY_INSTANT_PURCHASE] = FareTypeDesignator::FTD_ONEWAY_INSTANT_PURCHASE;
    _strToFd[FD_ROUNDTRIP_INSTANT_PURCHASE] = FareTypeDesignator::FTD_ROUNDTRIP_INSTANT_PURCHASE;
    _strToFd[FD_SPECIAL] = FareTypeDesignator::FTD_SPECIAL;
    _strToFd[FD_ADDON] = FareTypeDesignator::FTD_ADDON;
    _strToFd[FD_PROMOTIONAL] = FareTypeDesignator::FTD_PROMOTIONAL;
    _strToFd[FD_PREMIUM_FIRST] = FareTypeDesignator::FTD_PREMIUM_FIRST;
    _strToFd[FD_PREMIUM_ECONOMY] = FareTypeDesignator::FTD_PREMIUM_ECONOMY;
  }
}
}
