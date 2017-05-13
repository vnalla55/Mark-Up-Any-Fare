
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
#include "Pricing/Shopping/Utils/StreamFormat.h"

#include <iostream>
#include <vector>

#include <stdint.h>

namespace tse
{

namespace swp
{

class PositionalScore
{
public:
  typedef std::vector<int> ScoreWord;

  PositionalScore() {};

  // Initializes and sets size for the score. See description
  // of setSize() for details.
  PositionalScore(unsigned int words, unsigned int wordWidth, int defaultValue = 0)
  {
    setSize(words, wordWidth, defaultValue);
  }

  PositionalScore(const PositionalScore& right) : _words(right._words) {}

  PositionalScore& operator=(const PositionalScore& right)
  {
    if (this == &right)
      return *this;
    _words = right._words;
    return *this;
  }

  // Since comparison for different score sizes has no sense,
  // an exception is raised in such cases.
  bool operator==(const PositionalScore& right) const
  {
    TSE_ASSERT(equalSizes(right));
    return _words == right._words;
  }

  // Since comparison for different score sizes has no sense,
  // an exception is raised in such cases.
  bool operator!=(const PositionalScore& right) const
  {
    TSE_ASSERT(equalSizes(right));
    return !((*this) == right);
  }

  // Since comparison for different score sizes has no sense,
  // an exception is raised in such cases.
  bool operator<(const PositionalScore& right) const
  {
    TSE_ASSERT(equalSizes(right));
    // Operation < for vector uses lexicographical comparison
    return _words < right._words;
  }

  // Sets the number of words and width of each word
  // to numbers supplied. Sets all elements in words
  // to defaultValue.
  // Warning: setting size of a score is allowed only
  // once, the second call results in an exception
  void setSize(unsigned int words, unsigned int wordWidth, int defaultValue = 0);

  // Returns true iff right has the same number
  // of equally wide words as *this.
  bool equalSizes(const PositionalScore& right) const
  {
    return (getWordsNbr() == right.getWordsNbr()) && (getWordWidth() == right.getWordWidth());
  }

  // Retuns the number of words
  // (zero for an empty object)
  uint32_t getWordsNbr() const { return static_cast<uint32_t>(_words.size()); }

  // Returns width of each word
  // (zero for an empty object)
  uint32_t getWordWidth() const
  {
    if (getWordsNbr() == 0)
    {
      return 0;
    }
    return static_cast<uint32_t>(_words[0].size());
  }

  // Returns value from the word with wordNbr, on specified position
  int getValue(unsigned int wordNbr, unsigned int position) const
  {
    TSE_ASSERT(wordNbr < getWordsNbr());
    TSE_ASSERT(position < getWordWidth());
    return _words[wordNbr][position];
  }

  // Returns value in the word with wordNbr, on specified position
  void setValue(unsigned int wordNbr, unsigned int position, int value)
  {
    TSE_ASSERT(wordNbr < getWordsNbr());
    TSE_ASSERT(position < getWordWidth());
    _words[wordNbr][position] = value;
  }

  // Returns value of the word with wordNbr
  const ScoreWord& getWord(unsigned int wordNbr) const
  {
    TSE_ASSERT(wordNbr < getWordsNbr());
    return _words[wordNbr];
  }

  // Sets value of the word with wordNbr
  void setWord(unsigned int wordNbr, const ScoreWord& word)
  {
    TSE_ASSERT(wordNbr < getWordsNbr());
    TSE_ASSERT(word.size() == _words[0].size());
    _words[wordNbr] = word;
  }

private:
  std::vector<ScoreWord> _words;
};

class PositionalScoreFormatter
{
public:
  static void formatPositionalScore(std::ostream& out, const PositionalScore& ps);
  static PositionalScore scoreFromString(const std::string& s);

  static void formatScoreWord(std::ostream& out, const PositionalScore::ScoreWord& bsw);

private:
  static PositionalScore::ScoreWord wordFromToken(const std::string& s);
  static const char* EMPTY_REPR;
};

std::ostream& operator<<(std::ostream& out, const PositionalScore::ScoreWord& bsw);
std::ostream& operator<<(std::ostream& out, const PositionalScore& ps);

} // namespace swp

} // namespace tse

