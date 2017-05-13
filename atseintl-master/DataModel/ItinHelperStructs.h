//-------------------------------------------------------------------
//  Copyright Sabre 2011
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


#include <map>

namespace tse
{
class Itin;

class ItinBoolMap : public std::map<const Itin*, bool>
{
public:
  bool getValForKey(const Itin* key) const;
};

} // tse namespace

