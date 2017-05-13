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

#include "Common/Assert.h"
#include "Pricing/Shopping/Swapper/IAppraiserProxyParent.h"
#include "Pricing/Shopping/Swapper/IMapUpdater.h"

namespace tse
{

namespace swp
{

// Exposes callback interface IMapUpdater for a single Appraiser
// which has been set using setAppraiser()
// Forwards score modification calls from Appraiser on this interface
// to the parent object which has been set using setParent()
template <typename AppraiserType>
class AppraiserProxy
    : public IMapUpdater<typename AppraiserType::Item, typename AppraiserType::Score>
{
public:
  typedef IAppraiserProxyParent<AppraiserType> AppraiserProxyParent;

  AppraiserProxy() : _parent(nullptr), _appraiser(nullptr) {}

  // Sets the parent object
  // Throws no zero pointer
  void setParent(AppraiserProxyParent* parent)
  {
    TSE_ASSERT(parent != nullptr);
    _parent = parent;
  }

  // Sets the appraiser object
  // Throws no zero pointer
  void setAppraiser(AppraiserType* appraiser)
  {
    TSE_ASSERT(appraiser != nullptr);
    _appraiser = appraiser;
  }

  // Calls beforeItemAdded on its Appraiser,
  // passing this as the callback object
  // Throws if appraiser not initialized
  // Returns appraiser's score for the item
  typename AppraiserType::Score beforeItemAdded(const typename AppraiserType::Item& item)
  {
    TSE_ASSERT(_appraiser != nullptr);
    return _appraiser->beforeItemAdded(item, *this);
  }

  // Calls beforeItemRemoved on its Appraiser,
  // passing this as the callback object
  // Throws if appraiser not initialized
  void beforeItemRemoved(const typename AppraiserType::Item& item)
  {
    TSE_ASSERT(_appraiser != nullptr);
    _appraiser->beforeItemRemoved(item, *this);
  }

  // Calls the parent object to update score for given item
  // (callback method to be invoked by Appraiser)
  // Throws if parent not initialized
  void updateValue(const typename AppraiserType::Item& key,
                   const typename AppraiserType::Score& value) override
  {
    TSE_ASSERT(_appraiser != nullptr);
    TSE_ASSERT(_parent != nullptr);
    _parent->setScoreFromAppraiser(_appraiser, key, value);
  }

private:
  AppraiserProxyParent* _parent;
  AppraiserType* _appraiser;
};

} // namespace swp

} // namespace tse

