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

#pragma once


#include <memory>
#include <set>

namespace tse
{
namespace flexFares
{

class ValidationStatus;

typedef uint16_t GroupId;
typedef std::set<GroupId> GroupsIds;
typedef std::shared_ptr<ValidationStatus> ValidationStatusPtr;
enum class JumpCabinLogic {ENABLED, ONLY_MIXED, DISABLED};

enum Attribute
{
  CORP_IDS = 0,
  ACC_CODES,
  PUBLIC_FARES,
  PRIVATE_FARES,
  PASSENGER_TYPE,
  NO_ADVANCE_PURCHASE,
  NO_PENALTIES,
  NO_MIN_MAX_STAY,
  NUM_ATTRS = NO_MIN_MAX_STAY + 1
};

const std::string DIAG_GROUP_ID = "FG";
}
}

