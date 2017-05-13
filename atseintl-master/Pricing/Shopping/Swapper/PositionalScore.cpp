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

#include "Pricing/Shopping/Swapper/PositionalScore.h"

#include "Common/Assert.h"

#include <boost/regex.hpp>

#include <sstream>

namespace tse
{

namespace swp
{

void
PositionalScore::setSize(unsigned int words, unsigned int wordWidth, int defaultValue)
{
  TSE_ASSERT(words > 0);
  TSE_ASSERT(wordWidth > 0);

  // If the object has been initialized in the past,
  // no second resize is allowed
  if (getWordsNbr() > 0)
  {
    TSE_ASSERT(!"No second initialization of a score is possible");
    return;
  }

  _words.resize(words);
  for (auto& elem : _words)
  {
    elem.resize(wordWidth, defaultValue);
  }
}

const char* PositionalScoreFormatter::EMPTY_REPR = "(score empty)";

void
PositionalScoreFormatter::formatScoreWord(std::ostream& out, const PositionalScore::ScoreWord& bsw)
{
  out << "[";
  for (unsigned int i = 0; i < bsw.size(); ++i)
  {
    out << bsw[i];
    if (i != (bsw.size() - 1))
    {
      out << " ";
    }
  }
  out << "]";
}

void
PositionalScoreFormatter::formatPositionalScore(std::ostream& out, const PositionalScore& ps)
{
  if (ps.getWordsNbr() == 0)
  {
    out << EMPTY_REPR;
    return;
  }

  for (unsigned int i = 0; i < ps.getWordsNbr(); ++i)
  {
    out << ps.getWord(i);
  }
}

PositionalScore
PositionalScoreFormatter::scoreFromString(const std::string& s)
{
  if (s == EMPTY_REPR)
  {
    return PositionalScore();
  }

  // extract tokens of form [*]
  boost::regex re("\\[.+?\\]");
  boost::sregex_token_iterator i(s.begin(), s.end(), re);
  boost::sregex_token_iterator j;

  std::vector<PositionalScore::ScoreWord> words;
  while (i != j)
  {
    words.push_back(wordFromToken(*i));
    i++;
  }

  if (words.empty())
  {
    return PositionalScore();
  }

  PositionalScore p(words.size(), words[0].size());
  for (unsigned int k = 0; k < words.size(); ++k)
  {
    // If word width is incorrect,
    // PositionalScore will raise exception
    p.setWord(k, words[k]);
  }
  return p;
}

PositionalScore::ScoreWord
PositionalScoreFormatter::wordFromToken(const std::string& s)
{
  // E.g. token = [1 2 3]
  // Identify numbers inside
  boost::regex re("-?\\d+");
  boost::sregex_token_iterator i(s.begin(), s.end(), re);
  boost::sregex_token_iterator j;

  int v;
  PositionalScore::ScoreWord word;
  while (i != j)
  {
    std::istringstream in(*i);
    in >> v;
    word.push_back(v);
    i++;
  }
  return word;
}

std::ostream& operator<<(std::ostream& out, const PositionalScore::ScoreWord& bsw)
{
  out << utils::format(bsw, PositionalScoreFormatter::formatScoreWord);
  return out;
}

std::ostream& operator<<(std::ostream& out, const PositionalScore& ps)
{
  out << utils::format(ps, PositionalScoreFormatter::formatPositionalScore);
  return out;
}

} // namespace swp

} // namespace tse
