//-------------------------------------------------------------------
//
//  File:        Traveler.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"

namespace tse
{
class FrequentFlyerAccount;

class Traveler
{
public:
  uint16_t& referenceNumber() { return _referenceNumber; }
  const uint16_t& referenceNumber() const { return _referenceNumber; }

  std::string& lastNameQualifier() { return _lastNameQualifier; }
  const std::string& lastNameQualifier() const { return _lastNameQualifier; }

  std::string& lastName() { return _lastName; }
  const std::string& lastName() const { return _lastName; }

  std::string& firstNameQualifier() { return _firstNameQualifier; }
  const std::string& firstNameQualifier() const { return _firstNameQualifier; }

  std::string& firstName() { return _firstName; }
  const std::string& firstName() const { return _firstName; }

  std::string& otherName() { return _otherName; }
  const std::string& otherName() const { return _otherName; }

  bool& travelWithInfant() { return _travelWithInfant; }
  const bool& travelWithInfant() const { return _travelWithInfant; }

  FrequentFlyerAccount*& freqFlyerAccount() { return _freqFlyerAccount; }
  const FrequentFlyerAccount* freqFlyerAccount() const { return _freqFlyerAccount; }

private:
  // Fields populated from Reservation Data
  std::string _lastNameQualifier; // Last name Qualifier
  std::string _lastName; // Travelers Last Name
  std::string _firstNameQualifier; // First Name Qualifier
  std::string _firstName; // Traveler First Name
  std::string _otherName; // Traveler Other Name
  FrequentFlyerAccount* _freqFlyerAccount = nullptr; // FREQ Flyer Info
  uint16_t _referenceNumber = 0; // Travelers Reference number
  bool _travelWithInfant = false; // Traveler with Infant Ind
};
} // tse namespace
