//-------------------------------------------------------------------
//
//  File:        InfoBase.h
//  Created:     March 10, 2005
//  Authors:     Doug Batchelor/LeAnn Perez
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
// InfoBase.h: interface for the InfoBase class.
//
//---------------------------------------------------------------------------

/**
 * @class InfoBase
 *
 * A class to provide an interface API for the Fare Display Template processing to use
 * to retrieve the data the Display Template must show.
 */

#pragma once

namespace tse
{

class FareDisplayTrx;
class PaxTypeFare;

class InfoBase
{
public:
  virtual ~InfoBase() = default;

  // Call this method after instancing an InfoBase object to
  // initialize it properly.
  virtual void initialize(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare) = 0;
};
} // namespace tse
