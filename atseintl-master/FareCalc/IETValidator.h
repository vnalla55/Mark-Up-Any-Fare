
//-------------------------------------------------------------------
//
//  File:        IETValidator.h
//  Created:     March 01, 2012
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{

class PricingTrx;
class Itin;
class InterlineTicketCarrierInfo;
class Diag851Collector;

class IETValidator
{
public:
  IETValidator(PricingTrx& trx);
  virtual ~IETValidator();

  // interline ticketing carrier agreement data storage

  bool validate(const PricingTrx& trx, const Itin& itin, std::string& message);

private:
  bool validateInterlineTicketCarrierAgreement(
      const PricingTrx& trx,
      std::vector<CarrierCode>& cxrVec,
      const CarrierCode& validatingCarrier,
      const std::vector<InterlineTicketCarrierInfo*>& interlineInfoVec,
      bool participating,
      std::string& message);

  bool isDiagHeaderAdded() { return _diagHeaderAdded; }
  const bool isDiagHeaderAdded() const { return _diagHeaderAdded; }

private:
  PricingTrx& _trx;
  Diag851Collector* _diag;
  bool _diagHeaderAdded;
};
}

