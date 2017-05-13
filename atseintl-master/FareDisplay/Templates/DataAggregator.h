//-------------------------------------------------------------------
//
//  File:        DataAggregator.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//
//  Updates:
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include <vector>
#include "FareDisplay/Templates/Field.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{

class DataAggregator
{
public:
  //--------------------------------------------------------------------------
  // @function DataAggregator::DataAggregator
  //
  // Description: A data aggregator
  //
  // @param none
  //--------------------------------------------------------------------------
  DataAggregator();

  //--------------------------------------------------------------------------
  // @function DataAggregator::~DataAggregator
  //
  // Description: Virtual destructor
  //
  // @param none
  //--------------------------------------------------------------------------
  virtual ~DataAggregator();

  //--------------------------------------------------------------------------
  // @function DataAggregator::surcharges
  //
  // Description: Aggregate surcharge data
  //
  // @param field - field to be filtered
  // @param paxTypeFare - a valid PaxTypeFare
  // @param trx - a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  static void surcharges(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function DataAggregator::taxes
  //
  // Description: Aggregate tax data
  //
  // @param field - field to be filtered
  // @param paxTypeFare - a valid PaxTypeFare
  // @param trx - a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  static void US1Taxes(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

private:
  DataAggregator(const DataAggregator& rhs);
  DataAggregator& operator=(const DataAggregator& rhs);

};

} // namespace tse

