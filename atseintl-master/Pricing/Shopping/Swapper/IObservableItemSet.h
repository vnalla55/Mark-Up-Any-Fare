//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

template <typename ItemType>
class IItemSetObserver
{
public:
  virtual void itemAdded(const ItemType& item) = 0;
  virtual void itemRemoved(const ItemType& item) = 0;
  virtual ~IItemSetObserver() {}
};

template <typename ItemType>
class IObservableItemSet
{
public:
  virtual void addItemSetObserver(IItemSetObserver<ItemType>* observer) = 0;
  virtual void removeItemSetObserver(IItemSetObserver<ItemType>* observer) = 0;
  virtual ~IObservableItemSet() {}
};

class ISwapperInfo
{
public:
  // Tells how many times an item has been added to the swapper
  virtual unsigned int getTotalIterationsCount() const = 0;
  virtual ~ISwapperInfo() {}
};

} // namespace swp

} // namespace tse

