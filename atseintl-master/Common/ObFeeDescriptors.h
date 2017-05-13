//  Description: Common functions required for ATSE shopping/pricing.
//
//  Updates:
//
//  Copyright Sabre 2014
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <map>
#include <string>

namespace tse
{

typedef std::map<ServiceSubTypeCode, std::string> DescriptorMap;

class ObFeeDescriptors
{
public:
  ObFeeDescriptors();
  std::string getDescription(const ServiceSubTypeCode subTypeCode);

private:
  DescriptorMap _descriptorTable;
};

}

