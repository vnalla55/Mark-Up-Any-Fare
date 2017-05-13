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
#include "Pricing/Shopping/Swapper/AppraiserProxy.h"
#include "Pricing/Shopping/Swapper/AppraiserScoreBank.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/IAppraiserProxyParent.h"
#include "Pricing/Shopping/Swapper/IMapUpdater.h"

#include <boost/unordered_map.hpp>

#include <map>
#include <vector>

namespace tse
{

namespace swp
{

// Scores each new item using its appraisers and score builder
// Mediates between the following objects:
// - added appraisers that give their scores to newly arrived items
//   and update scores for any items on new item arrival/item removal
// - score builder that takes appraisers' scores and combine them
//   to the final score for an item
// - a score map that allows to update scores for selected items
//   when requested by appraisers
//
// ScoreBuilderType is expected to define three types:
// - ScoreBuilderType::AppraiserScore
//     for scores given by appraisers
// - ScoreBuilderType::ItemScore
//     for final item scores combined from appraisers' scores
// - ScoreBuilderType::AppraiserInfo
//     for info describing praticular appraiser to the builder
template <typename ItemType, typename ScoreBuilderType>
class ScoringMediator
    : public IAppraiserProxyParent<IAppraiser<ItemType, typename ScoreBuilderType::AppraiserScore> >
{
public:
  typedef IMapUpdater<ItemType, typename ScoreBuilderType::ItemScore> ScoreMap;
  typedef IAppraiser<ItemType, typename ScoreBuilderType::AppraiserScore> Appraiser;
  typedef AppraiserScoreBank<Appraiser> ImplAppraiserScoreBank;
  typedef typename ImplAppraiserScoreBank::Map AppraiserScoresMap;

  // - sets the reference to the used builder object
  // - sets the reference to the score map interface
  ScoringMediator(ScoreBuilderType& builder, ScoreMap& scoreMap)
    : _scoreBuilder(builder), _callbackMap(scoreMap)
  {
  }

  // Adds a new appraiser to this mediator object
  // info contains information that identifies
  // a particular appraiser from the builder's point of view
  // Throws if appraiser is zero
  // Throws if appraiser has been already added
  void addAppraiser(Appraiser* appraiser, const typename ScoreBuilderType::AppraiserInfo& info)
  {
    TSE_ASSERT(appraiser != nullptr);
    // Ensure there are no appraiser duplicates
    TSE_ASSERT(_proxyMap.find(appraiser) == _proxyMap.end());

    // Tell score builder about a new appraiser
    _scoreBuilder.addAppraiser(info);
    _infoMap[appraiser] = info;

    // Build proxy
    AppraiserProxy<Appraiser> proxy;
    proxy.setParent(this);
    proxy.setAppraiser(appraiser);
    _proxyMap[appraiser] = proxy;

    _appraisers.push_back(appraiser);
  }

  // Calls beforeItemAdded on all appraisers
  // They can update score blackboard using the proxy objects
  // Appraiser scores obtained from these calls
  // are combined into the total score for the item which is returned.
  typename ScoreBuilderType::ItemScore beforeItemAdded(const ItemType& item)
  {
    // Iterating over _appraisers (not map directly) to
    // obtain the order of appraisers' adding (easier testing)
    for (unsigned int i = 0; i < _appraisers.size(); ++i)
    {
      typename AppraisersProxyMap::iterator it = _proxyMap.find(_appraisers[i]);
      TSE_ASSERT(it != _proxyMap.end());
      _bank.setScore(item, _appraisers[i], it->second.beforeItemAdded(item));
    }
    return buildItemScoreUsingBank(item);
  }

  // Calls beforeItemRemoved on all appraisers
  // They can update score blackboard using the proxy objects
  void beforeItemRemoved(const ItemType& item)
  {
    // Iterating over _appraisers (not map directly) to
    // obtain the order of appraisers' adding (easier testing)
    for (unsigned int i = 0; i < _appraisers.size(); ++i)
    {
      typename AppraisersProxyMap::iterator it = _proxyMap.find(_appraisers[i]);
      TSE_ASSERT(it != _proxyMap.end());
      it->second.beforeItemRemoved(item);
    }
    _bank.removeItem(item);
  }

  // This callback function is executed as a result of call
  // to setScore() on a IMapUpdater interface
  // exposed to appraiser code.
  void setScoreFromAppraiser(const Appraiser* appraiser,
                             const typename Appraiser::Item& item,
                             const typename Appraiser::Score& score) override
  {
    _bank.setScore(item, appraiser, score);
    _callbackMap.updateValue(item, buildItemScoreUsingBank(item));
  }

  // Returns a vector of added appraisers
  const std::vector<const Appraiser*>& getAppraisers() const { return _appraisers; }

  // Returns info for given appraiser
  // Throws if no such appraiser
  typename ScoreBuilderType::AppraiserInfo getInfoForAppraiser(const Appraiser* appraiser) const
  {
    typename InfoMap::const_iterator infoIter = _infoMap.find(appraiser);
    TSE_ASSERT(infoIter != _infoMap.end());
    return infoIter->second;
  }

  // Returns mapping { appraiser* -> score } for given item
  // Throws if there are no scores for given item
  const AppraiserScoresMap& getScoresForItem(const ItemType& item) const
  {
    return _bank.getScoresForItem(item);
  }

  // Returns the number of items contained in the
  // appraiser score bank
  unsigned int getBankItemsCount() const { return _bank.getItemsCount(); }

  // Returns the total number of scores
  // contained in the appraiser score bank
  // (for all items, from all appraisers)
  unsigned int getBankScoresCount() const { return _bank.getScoresCount(); }

private:
  typedef boost::unordered_map<const Appraiser*, AppraiserProxy<Appraiser> > AppraisersProxyMap;
  typedef boost::unordered_map<const Appraiser*, typename ScoreBuilderType::AppraiserInfo> InfoMap;

  // Extracts appraiser scores for given item from bank
  // Then uses bulder to combine these scores to the final item score
  // which is returned
  typename ScoreBuilderType::ItemScore buildItemScoreUsingBank(const ItemType& item)
  {
    _scoreBuilder.newItem();
    const AppraiserScoresMap& scoreMap = getScoresForItem(item);

    for (typename AppraiserScoresMap::const_iterator it = scoreMap.begin(); it != scoreMap.end();
         ++it)
    {
      _scoreBuilder.addScoreFromAppraiser(getInfoForAppraiser(it->first), it->second);
    }

    return _scoreBuilder.getItemScore();
  }

  ImplAppraiserScoreBank _bank;
  ScoreBuilderType& _scoreBuilder;
  ScoreMap& _callbackMap;
  AppraisersProxyMap _proxyMap;
  InfoMap _infoMap;

  // Just to store appraisers in the order
  // of adding and return them in a convenient way
  std::vector<const Appraiser*> _appraisers;
};

} // namespace swp

} // namespace tse

