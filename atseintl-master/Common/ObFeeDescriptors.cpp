//----------------------------------------------------------------------------
//
//  File:           ObFeeDescriptors.cpp
//  Created:        09/24/2014
//  Authors:
//
//  Description:    OB Fees sub type description table.
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

#include "Common/ObFeeDescriptors.h"

#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"

namespace tse
{

ObFeeDescriptors::ObFeeDescriptors()
{
  _descriptorTable["R01"] = "OVERNIGHT DELIVERY CHARGE";
  _descriptorTable["R02"] = "COURIER CHARGE";
  _descriptorTable["R03"] = "TOUR PACKAGE DELIVERY";
  _descriptorTable["R04"] = "TRAVEL VISA ACQUISITION CH";
  _descriptorTable["R05"] = "PAPER TICKET";
  _descriptorTable["R07"] = "UNACCOMPANIED MINOR";
  _descriptorTable["R08"] = "PRIVATE JET TICKETING 1";
  _descriptorTable["R09"] = "PRIVATE JET TICKETING 2";
  _descriptorTable["R10"] = "NON CREDIT CARD PAYMENT FEE";
  _descriptorTable["R11"] = "NON CREDIT CARD PAYMENT FEE";
  _descriptorTable["R12"] = "NON CREDIT CARD PAYMENT FEE";
}

std::string
ObFeeDescriptors::getDescription(const ServiceSubTypeCode subTypeCode)
{
  std::string description;

  switch (TrxUtil::getOBFeeSubType(subTypeCode))
  {
  case OBFeeSubType::OB_T_TYPE:
    description = "CARRIER TICKETING FEE" + subTypeCode.substr(1, 2);
    break;
  case OBFeeSubType::OB_R_TYPE:
    if (_descriptorTable.find(subTypeCode) != _descriptorTable.end())
      description = _descriptorTable[subTypeCode];
    else
      description = "OPTIONAL SERVICE FEE" + subTypeCode.substr(1, 2);
    break;
  default:
    break;
  }

  return description;
}

}
