//----------------------------------------------------------------------------
//  File:        Diag914Collector.h
//  Created:     2006-07-17
//  Authors:     Kavya Katam
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class ShoppingTrx;

class Diag914Collector : public DiagCollector
{
public:
  Diag914Collector& operator<<(const ShoppingTrx& trx) override;

private:
  void outputAltDates(const ShoppingTrx& trx);
  void outputDurations(const ShoppingTrx& trx);
};
}

