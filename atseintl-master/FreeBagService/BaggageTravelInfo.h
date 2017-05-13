//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

namespace tse
{

class BaggageTravelInfo
{
public:
  BaggageTravelInfo(uint32_t bagIndex, uint32_t fareIndex)
    : _bagIndex(bagIndex), _fareIndex(fareIndex)
  {
  }

  uint32_t bagIndex() const { return _bagIndex; }

  uint32_t fareIndex() const { return _fareIndex; }

private:
  uint32_t _bagIndex;
  uint32_t _fareIndex;
};

} // tse

