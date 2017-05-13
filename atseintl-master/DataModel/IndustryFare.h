//-------------------------------------------------------------------
//
//  File:        IndustryFare.h
//  Created:     July 7, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: A class to represent the Industry Fare specific part
//               of a Fare.
//
//  Updates:
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

#include "DataModel/Fare.h"
#include "DBAccess/IndustryFareAppl.h"

namespace tse
{

class IndustryFare : public Fare
{
public:
  IndustryFare()
  {
    _status.set(FS_IndustryFare);
  }

  IndustryFare(const IndustryFare&) = delete;
  IndustryFare& operator=(const IndustryFare&) = delete;

  virtual Fare* clone(DataHandle& dataHandle, bool resetStatus = true) const override;

  void clone(DataHandle& dataHandle, IndustryFare& cloneObj, bool resetStatus = true) const;

  // initialization
  // ==============

  bool initialize(const Fare& fare,
                  const IndustryFareAppl& fareAppl,
                  const IndustryFareAppl::ExceptAppl* exceptAppl,
                  DataHandle& dataHandle,
                  bool validForPricing = true);

  bool isMultiLateral() const
  {
    return ((_fareAppl != nullptr) && (_fareAppl->selectionType() == MULTILATERAL));
  }

  CarrierCode& carrier() { return _carrier; }

  virtual const CarrierCode& carrier() const override { return _carrier; }

  FareClassCode& fareClass() { return _fareClass; }

  virtual const FareClassCode& fareClass() const override { return _fareClass; }

  const IndustryFareAppl* industryFareAppl() const { return _fareAppl; }
  const IndustryFareAppl::ExceptAppl* exceptAppl() const { return _exceptAppl; }

  const static Indicator MULTILATERAL;

  const bool& changeFareClass() const { return _changeFareClass; }
  bool& changeFareClass() { return _changeFareClass; }

  const bool& validForPricing() const { return _validForPricing; }
  bool& validForPricing() { return _validForPricing; }

  const bool& matchFareTypeOfTag2CxrFare() const { return _matchFareTypeOfTag2CxrFare; }
  bool& matchFareTypeOfTag2CxrFare() { return _matchFareTypeOfTag2CxrFare; }

private:
  // We have to alter these from the base fare values sometimes
  CarrierCode _carrier;
  FareClassCode _fareClass;

  const IndustryFareAppl* _fareAppl = nullptr;
  const IndustryFareAppl::ExceptAppl* _exceptAppl = nullptr;

  bool _changeFareClass = false;
  bool _validForPricing = true;
  bool _matchFareTypeOfTag2CxrFare = false;
};
} // tse namespace
