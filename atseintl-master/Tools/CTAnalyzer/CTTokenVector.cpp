#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <boost/tokenizer.hpp>

#include "CTConstants.h"
#include "CTTokenVector.h"

using namespace std;
using namespace tse;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string
CTTokenVector::constructMultiToken(
  const std::pair<uint32_t, uint32_t>& range,
  ConstructTokenMode consTokMode)
{
  std::string rt("");
  
  if (range.first >= _tokens.size())
  {
    return(rt);
  }

  if (range.second >= _tokens.size())
  {
    return(rt);
  }

  if (range.first > range.second)
  {
    return(rt);
  }

  if (consTokMode == ConstructTokenNoSpaces)
  {
    for (uint32_t i=range.first; i<=range.second; i++)
    {
      rt += _tokens[i];
    }
  }
  else
  {
    for (uint32_t i=range.first; i<=range.second; i++)
    {
      rt += _tokens[i] + CTSpace;
    }
  }

  return(rt);
}

