//----------------------------------------------------------------------------
//
// Copyright Sabre 2013
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"

namespace tse
{

class AncillaryPricingTrx;
class Itin;

class AncillaryTrxScopeDateSetter
{
  friend class AncillaryTrxScopeDateSetterTest;

public:
  explicit AncillaryTrxScopeDateSetter(AncillaryPricingTrx& ancTrx);
  ~AncillaryTrxScopeDateSetter();

  void updateDate(const Itin* itin);

private:
  AncillaryTrxScopeDateSetter(const AncillaryTrxScopeDateSetter&);
  AncillaryTrxScopeDateSetter& operator=(const AncillaryTrxScopeDateSetter&);

  void update(const DateTime& newDate, const DateTime& ticketingDate);

  AncillaryPricingTrx& _ancTrx;
  DateTime _handleDate;
  DateTime _ticketingDate;
};
}

