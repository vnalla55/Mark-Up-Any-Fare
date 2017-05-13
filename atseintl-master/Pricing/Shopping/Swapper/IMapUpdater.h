//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
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

namespace swp
{

// Allows to update value for a key in a mapping object
template <typename KeyType, typename ValueType>
class IMapUpdater
{
public:
  // Sets a new value for the given key
  // If the key is not found, an error is raised
  virtual void updateValue(const KeyType& key, const ValueType& value) = 0;
  virtual ~IMapUpdater() {}
};

} // namespace swp

} // namespace tse

