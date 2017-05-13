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

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

namespace swp
{

// This class translates a single BasicAppraiserScore
// into a vector of three integers: [A, B, C]
// where:
// A: 1 for category must have, -1 for category want to remove, else zero
// B: 1 for category nice to have, else zero
// C: minor sort rank (taken from BasicAppraiserScore::getMinorRank())
//    or zero for category ignored
class BasicAppraiserScoreVectorizer : boost::noncopyable
{
public:
  typedef std::vector<int> ScoreVector;

  // Returns a vector produced from a score, as in class description
  static ScoreVector vectorize(const BasicAppraiserScore& score)
  {
    if (score.getCategory() == BasicAppraiserScore::IGNORE)
    {
      return createEmptyVector();
    }

    ScoreVector v = createEmptyVector();
    v[RANK_INDEX] = score.getMinorRank();

    if (score.getCategory() == BasicAppraiserScore::MUST_HAVE)
    {
      v[MUST_INDEX] = 1;
    }
    else if (score.getCategory() == BasicAppraiserScore::WANT_TO_REMOVE)
    {
      v[MUST_INDEX] = -1;
    }
    else if (score.getCategory() == BasicAppraiserScore::NICE_TO_HAVE)
    {
      v[NICE_INDEX] = 1;
    }
    else
    {
      TSE_ASSERT(!"Bad score category");
    }
    return v;
  }

  // Returns an empty vector = [0, 0, 0]
  static ScoreVector createEmptyVector() { return ScoreVector(WORDS_NUMBER, 0); }

private:
  static const unsigned int WORDS_NUMBER = 3;
  static const unsigned int MUST_INDEX = 0;
  static const unsigned int NICE_INDEX = 1;
  static const unsigned int RANK_INDEX = 2;
};

} // namespace swp

} // namespace tse

