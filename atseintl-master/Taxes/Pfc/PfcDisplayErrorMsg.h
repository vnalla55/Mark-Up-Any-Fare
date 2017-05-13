//----------------------------------------------------------------------------
//  File:           PfcDisplayErrorMsg.h
//  Description:    PfcDisplayErrorMsg header file for ATSE International Project
//  Created:        2/18/2008
//  Authors:        Piotr Lach
//
//  Description: Error/Warning messages for PFC Display functionality.
//
//
//  Copyright Sabre 2008
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

#ifndef PFC_DISPLAY_ERROR_MSG_H
#define PFC_DISPLAY_ERROR_MSG_H

#include <string>

class PfcDisplayErrorMsg
{
public:
  static const std::string INVALID_FORMAT;
  static const std::string INVALID_CODE;
  static const std::string INVALID_SEGMENT;
  static const std::string NO_ITINERARY;
  static const std::string AIRPORTS_EXCEEDED;
  static const std::string DATA_NOT_FOUND;
  static const std::string ABSORPTION_NOT_FOUND;
  static const std::string BEYOND_MAX_HISTORICAL_DATE;
  static const std::string PFC_NOT_APPLICABLE;
  static const std::string NO_MULTIAIRPORT_LOCATIONS;
  static const std::string NO_AIRPORT_EAS_LOCATIONS;
};

#endif
