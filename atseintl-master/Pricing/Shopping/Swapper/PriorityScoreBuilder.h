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
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScoreVectorizer.h"
#include "Pricing/Shopping/Swapper/PositionalScore.h"
#include "Pricing/Shopping/Swapper/RankingSet.h"
#include "Pricing/Shopping/Swapper/ScoreVectorSummator.h"

#include <iostream>
#include <string>
#include <vector>

namespace tse
{

namespace swp
{

// Builds a PositionalScore from a number of BasicAppraiserScores
//
// Each BasicAppraiserScore is transformed into a
// three-element vector, using BasicAppraiserScoreVectorizer
//
// The resulting vector is added with a calculated offset
// to this builder's PositionalScore which represents the final item score
//
// Appraisers are seen by this builder through associated priorities
// The priority of an appraiser is used to calculate the rank which is
// a number >= 0 assigned to all appraisers sharing a particular priority
// The greater the priority, the lesser the rank, e.g. imagine four
// appraisers with priorities 10, 25, 5 and again 25. They will be assigned
// the following ranks:
//
// Appraiser | Priority | Rank
//         A |       10 |    1
//         B |       25 |    0
//         C |        5 |    2
//         D |       25 |    0
//
// Rank is also an offset with witch a vector produced from
// the appraiser score is placed in the resulting PositionalScore
// (see ScoreVectorSummator). For this example, offsets will be
// assigned as follows:
// Offset         0     1     2
// Appraisers  B, D     A     C
//
// Note: this class only cares about unique priorities
// of appraisers that were added. It does need the total appraisers
// number and does not store it.
class PriorityScoreBuilder
{
public:
  typedef int AppraiserPriority;
  typedef unsigned int AppraiserRank;

  typedef BasicAppraiserScore AppraiserScore;
  typedef PositionalScore ItemScore;
  typedef AppraiserPriority AppraiserInfo;
  typedef AppraiserRank DerivedInfo;

  PriorityScoreBuilder() : _summator(WORDS_NUMBER) {}

  // Add a new appraiser represented by its priority
  void addAppraiser(const AppraiserPriority& priority)
  {
    _rankingSet.insert(priority);
    // Set number of offsets equal to the number
    // of priority groups
    _summator.newScore(_rankingSet.getSize());
  }

  // Clears the internal score to start accumulate
  // appraiser scores for a new item
  // If no appraisers are added yet, an exception is raised
  void newItem()
  {
    TSE_ASSERT(_rankingSet.getSize() > 0);
    _summator.newScore(_rankingSet.getSize());
  }

  // Accepts a score from an appraiser with given priority
  // updating the total item score
  // (score is transformed to a vector and added
  // to the item score with offset calculated from priority)
  //
  // If priority was never added to this builder, an exception
  // is raised
  void addScoreFromAppraiser(const AppraiserPriority& priority, const BasicAppraiserScore& score)
  {
    const BasicAppraiserScoreVectorizer::ScoreVector v = _vectorizer.vectorize(score);
    _summator.addScoreVector(getDerivedInfo(priority), v);
  }

  // Returns the contained score
  // Throws if no appraisers yet
  const PositionalScore& getItemScore() const
  {
    // Check if we have any appraisers
    TSE_ASSERT(_rankingSet.getSize() > 0);
    return _summator.getScore();
  }

  // Returns rank for given appraiser priority
  // (see class description for rank explanation)
  // Throws on unrecognized priority
  AppraiserRank getDerivedInfo(const AppraiserPriority& priority) const
  {
    // getRank will throw if no such priority
    const AppraiserRank rank = _rankingSet.getRank(priority);
    TSE_ASSERT(rank <= (_rankingSet.getSize() - 1));
    // "Reverse" rank here: the higher the priority, the smaller the rank
    // Rank here will be a non-negative integer
    return _rankingSet.getSize() - 1 - rank;
  }

private:
  static const unsigned int WORDS_NUMBER = 3;
  RankingSet<AppraiserPriority> _rankingSet;
  BasicAppraiserScoreVectorizer _vectorizer;
  ScoreVectorSummator _summator;
};

} // namespace swp

} // namespace tse

