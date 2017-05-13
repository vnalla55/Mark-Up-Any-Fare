//----------------------------------------------------------------------------
//
//  File:        FareTypeMatrixUtil.h
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
#pragma once

#include "Common/FareTypeDesignator.h"
#include "Common/TseEnums.h"

#include <boost/thread/mutex.hpp>

#include <map>

namespace tse
{
class FareTypeMatrixUtil
{
public:
  static FareTypeDesignator convert(const std::string& fdDiagValue);
  static const std::string& convert(const FareTypeDesignator& fd);

private:
  static boost::mutex _mapMutex;
  static std::map<FareTypeDesignator::FareTypeDesignatorNew, std::string> _fdToStr;

  static std::map<std::string, FareTypeDesignator::FareTypeDesignatorNew> _strToFd;

  static void loadMaps();
};

} // end tse namespace


