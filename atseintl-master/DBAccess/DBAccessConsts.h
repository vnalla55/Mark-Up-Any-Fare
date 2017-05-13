//----------------------------------------------------------------------------
//
//     File:           DBAccessConsts.h
//     Description:    Shared DBAccess constants.
//     Created:        04/28/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.  This software/documentation
//         is the confidential and proprietary product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

namespace DBAccess
{
enum ServerType
{
  UNKNOWN_SERVER = 0,
  ORACLE_SERVER = 3,
  // This value is under consideration! It might not be used!
  NUM_SERVERS = 5
};
}

