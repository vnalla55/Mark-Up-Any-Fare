#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <stack>
#include <boost/tokenizer.hpp>

#include "CTConstants.h"
#include "CTTokenVector.h"
#include "CallTreeMethod.h"

using namespace std;
using namespace tse;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint64_t 
CallTreeMethod::convertLargeNumberString(std::string& number,
                                         char extraneousChar)
{
  //Calculate numeric cost
  std::string costWOExtr("");
  std::string::iterator cIter = number.begin();
  std::string::iterator cEIter = number.end();
  for (; cIter != cEIter; ++cIter)
  {
    char curC = *cIter;
    if (curC == extraneousChar)
    {
      continue;
    }
    if (curC < '0' || curC > '9')
    {
      return(0);
    }

    costWOExtr.push_back(curC);
  }

  if (costWOExtr.empty() || costWOExtr.size() == 0)
  {
    return(0);
  }

  uint64_t multiplier = 1;
  uint64_t temp = 0;
  uint64_t tempTotal = 0;
  std::string::reverse_iterator rCIter = costWOExtr.rbegin();
  std::string::reverse_iterator rCEIter = costWOExtr.rend();
  char zeroChar = '0';
  for (; rCIter != rCEIter; ++rCIter)
  {
    char curC = *rCIter;
    uint32_t diff = static_cast<uint32_t>(curC) - 
      static_cast<uint32_t>(zeroChar);
    temp = static_cast<uint64_t>(diff) * multiplier;
    tempTotal += temp;
    multiplier *= 10;
  }
  return(tempTotal);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CallTreeMethod::generateMatchKey()
{
  //Strings used to generate key
  // - method name
  // - method signature
  // - class name
  _matchKey.clear();
  _matchKey.append(_className);
  _matchKey.append(_classMethodParams);
  _matchKey.append(_className);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CallTreeMethod::invalidate()
{
  //For now, this is all we do to 
  //invalidate
  //std::cout << "**** method has been invalidated ****" << std::endl;
  _valid = false;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::determineMethodSourceType(CTTokenVector*& info)
{
  //The source file is directly after the type token, and runs 
  //until a colon is reached
  if (_valid)
  {
    //std::cout << "determining method source type" << std::endl;
    uint32_t startIdx = CTMethodTypeIndex + 1;
    uint32_t endIdx = 0;
    bool foundColon = false;
    
    if (!_singleColonIndices.empty())
    {
      endIdx = _singleColonIndices[0]-1;
      if (endIdx >= startIdx)
      {
        //std::cout << "- colon found" << std::endl;
        foundColon = true;
      }
    }

    if (!foundColon)
    {
      //std::cout << "- no colon found, invalidating" << std::endl;
      invalidate();
      return;
    }

    if (startIdx == endIdx)
    {
      _sourceFile = info->getToken(startIdx);
    }
    else
    {
      std::pair<uint32_t,uint32_t> tokPair(startIdx, endIdx);
      _sourceFile = info->constructMultiToken(tokPair,
                                              CTTokenVector::ConstructTokenNoSpaces);
    }

    if (_sourceFile.empty())
    {
      //std::cout << "- source file is empty, invalidating" << std::endl;
      invalidate();
      return;
    }
    
    //See if we have a .cpp file
    uint32_t findIdx = _sourceFile.find(CTCPlusPlusFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is cpp" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceCPlusPlus;
      return;
    }

    //See if we have a .cxx file
    findIdx = _sourceFile.find(CTCxxFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is cxx" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceCxx;
      return;
    }

    //See if we have a .hpp file
    findIdx = _sourceFile.find(CTHPlusPlusFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is hpp" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceHPlusPlus;
      return;
    }

    //See if we have a .h file
    findIdx = _sourceFile.find(CTHeaderFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is h" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceHeader;
      return;
    }

    //See if we have a .c file
    findIdx = _sourceFile.find(CTCFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is c" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceC;
      return;
    }

    //See if we have a .C file
    findIdx = _sourceFile.find(CTBigCFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is big C" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceBigC;
      return;
    }


    //See if we have a .H file
    findIdx = _sourceFile.find(CTBigHFile);
    if (findIdx != std::string::npos)
    {
      //std::cout << "- source is big H" << std::endl;
      _sourceType = CallTreeMethod::MethodSourceBigH;
      return;
    }
    

    //std::cout << "source is unknown, invalidating" << std::endl;
    _sourceType = MethodSourceUnknown;
    invalidate();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::processStartMethodLine(CTTokenVector*& info)
{
  analyzeMethodLineIndices(info);
  determineMethodSourceType(info);
  if (!_valid)
  {
    return;
  }

  switch(_sourceType)
  {
  case MethodSourceC:
    processFunctionName(info);
    processFunctionSignature(info);
    break;
  case MethodSourceCPlusPlus:
  case MethodSourceBigC:
  case MethodSourceBigH:
  case MethodSourceHeader:
  case MethodSourceHPlusPlus:
  case MethodSourceCxx:
  case MethodSourceCC:
    processNamespace(info);
    processClassName(info);
    processFunctionName(info);
    processFunctionSignature(info);
    break;
  case MethodSourceUnknown:
    invalidate();
    return;
  default:
    //std::cout << "Unknown source type found!!" << std::endl;
    invalidate();
    return;
  }
      
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CallTreeMethod::processContinueMethodLine(CTTokenVector*& info)
{
  analyzeMethodLineIndices(info);
  determineMethodSourceType(info);
  if (!_valid)
  {
    return;
  }

  switch(_sourceType)
  {
  case MethodSourceC:
    processFunctionName(info);
    processFunctionSignature(info);
    processNumberTimesCalled(info);    
    break;
  case MethodSourceCPlusPlus:
  case MethodSourceBigC:
  case MethodSourceBigH:
  case MethodSourceHeader:
  case MethodSourceHPlusPlus:
  case MethodSourceCxx:
  case MethodSourceCC:
    processNamespace(info);
    processClassName(info);
    processFunctionName(info);
    processFunctionSignature(info);
    processNumberTimesCalled(info);
    break;
  case MethodSourceUnknown:
    invalidate();
    return;
  default:
    //std::cout << "Unknown source type found!!" << std::endl;
    invalidate();
    return;
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::analyzeMethodLineIndices(CTTokenVector*& info)
{
  if (!_valid)
  {
    return;
  }
  //std::cout << "analyzeMethodLineIndices()..." << std::endl;

  std::vector<std::string>::iterator infoIter = info->tokens().begin();
  std::vector<std::string>::iterator infoEIter = info->tokens().end();
  std::vector<std::string>::iterator infoIterP1;
  uint32_t idx = 0;
  std::stack<uint32_t> parenOpenIndices;
  std::stack<uint32_t> bracketOpenIndices;

  for (; infoIter != infoEIter; ++infoIter)
  {
    //Get the next token and next iterator if possible
    std::string* nextToken = nullptr;
    if ((infoIter + 1) != infoEIter)
    {
      infoIterP1 = infoIter + 1;
      nextToken = static_cast<std::string*>(&(*infoIterP1));
    }
    else
    {
      infoIterP1 = infoEIter;
    }

    //Get the current token
    std::string& curToken = *infoIter;
    //std::cout << "-    currentToken = " << curToken << std::endl;
    if (infoIterP1 != infoEIter)
    {
      //std::cout << "- current2ndToken = " << *nextToken << std::endl;
    }
    
    if (curToken == CTColon)
    {
      //std::cout << "-- found a colon" << std::endl;
      bool nextEqualsColon = ((nextToken != nullptr)&&(*nextToken==CTColon));
      if (!nextEqualsColon)
      {
        _singleColonIndices.push_back(idx);
        //std::cout << "-- single colon instance=" << idx << std::endl;
      }
      else
      {
        _doubleColonIndices.push_back( std::pair<uint32_t,uint32_t>(idx,idx+1) );
        ++infoIter;
        ++idx;
        //std::cout << "-- double colon instance=" << idx << std::endl;
      }
    }
    else if (curToken == CTOpenParen)
    {
      parenOpenIndices.push(idx);
      //std::cout << "-- found open paren=" << idx << std::endl;
    }
    else if (curToken == CTCloseParen)
    {
      if (!parenOpenIndices.empty())
      {
        //std::cout << "--found close paren=" << idx << std::endl;
        uint32_t priorOpenIdx = parenOpenIndices.top();
        parenOpenIndices.pop();
        _parenPairIndices.push_back( std::pair<uint32_t,uint32_t>(priorOpenIdx,idx) );
      }
      else
      {
        //std::cout << "-- malformed, close paren found with no open paren=" << idx << std::endl;
        //Malformed
        invalidate();
        break;
      }
    }
    else if (curToken == CTOpenBracket)
    {
      //std::cout << "-- open bracket found=" << idx << std::endl;
      bracketOpenIndices.push(idx);
    }
    else if (curToken == CTCloseBracket)
    {
      if (!bracketOpenIndices.empty())
      {
        //std::cout << "--found close bracket=" << idx << std::endl;
        uint32_t priorOpenIdx = bracketOpenIndices.top();
        bracketOpenIndices.pop();
        _bracketPairIndices.push_back( std::pair<uint32_t,uint32_t>(priorOpenIdx,idx) );
      }
      else
      {
        //std::cout << "--malformed, close bracket found with no open bracket=" << idx << std::endl;
        //Malformed
        invalidate();
        break;
      }
    }

    ++idx;
  }

  if ((!bracketOpenIndices.empty())||(!(parenOpenIndices.empty())))
  {
    //std::cout << "-malformed, still have open brackets or parens" << std::endl;
    //Malformed
    invalidate();
    return;
  }

  if (_parenPairIndices.empty())
  {
    //std::cout << "-malformed, no parentheses found" << std::endl;
    //Malformed
    invalidate();
    return;
  }

  if (_singleColonIndices.empty())
  {
    //std::cout << "-malformed, no single colon instances found" << std::endl;
    //Malformed
    invalidate();
    return;
  }

  //std::cout << "Finished analyzeMethodLineIndices()" << std::endl;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::processFunctionName(CTTokenVector*& info)
{
  if (!_valid)
  {
    return;
  }

  uint32_t parenIdx = _parenPairIndices[0].first;
  uint32_t startIdx=0;
  uint32_t  endIdx=0;

  //See if a namespace was given
  if ((_classNamespace.empty() ||
       _classNamespace.size() == 0 ||
       _classNamespace == CTNoData) && 
      (_doubleColonIndices.size() >= 1))
  {
    //No namespace given, function name is after the first
    //double colon pair and before the first open paretheses
    startIdx = _doubleColonIndices[0].second + 1;
    endIdx = parenIdx - 1;
  }
  else if ((!_classNamespace.empty())&&(_doubleColonIndices.size() >= 2))
  {
    //Namespace given
    //Function name is after the second set of double colon pairs
    //and before the first open parentheses
    startIdx = _doubleColonIndices[1].second + 1;
    endIdx = parenIdx - 1;
  }
  else
  {
    //C style function name
    startIdx = _singleColonIndices[0] + 1;
    endIdx   = parenIdx - 1;
  }

  if (startIdx > endIdx)
  {
    //Malformed
    invalidate();
    return;
  }

  //Get the function name
  if (startIdx == endIdx)
  {
     _classMethodName = info->getToken(startIdx);
  }
  else
  {
    std::pair<uint32_t,uint32_t> tokPair(startIdx, endIdx);
    _classMethodName = info->constructMultiToken(tokPair,
                                                 CTTokenVector::ConstructTokenNoSpaces);
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::processNamespace(CTTokenVector*& info)
{
  //from current single colon to the next double colon (namespace exists in c++ only, when
  //there are two sets of double colons before the first parentheses)
  if (!_valid)
  {
    return;
  }

  //Namespace is given if there are at least two sets of double colons
  //before a parentheses
  if (_doubleColonIndices.size() >= 2)
  {
    uint32_t startIdx = _singleColonIndices[0]+1;
    uint32_t endIdx   = _doubleColonIndices[0].first-1;

    if (startIdx > endIdx)
    {
      //Malformed
      invalidate();
      return;
    }

    if (startIdx == endIdx)
    {
      _classNamespace = info->getToken(startIdx);
    }
    else
    {
      std::pair<uint32_t,uint32_t> idxPair(startIdx, endIdx);
      _classNamespace = info->constructMultiToken(idxPair,
        CTTokenVector::ConstructTokenNoSpaces);
    }
  }
  else
  {
    _classNamespace = CTNoData;
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::processNumberTimesCalled(CTTokenVector*& info)
{
  //after the function signature, between a ( and a ), ends with an x
  if (!_valid)
  {
    return;
  }

  //If there is a second set of parentheses, then we have the number of times called
  if (_parenPairIndices.size() < 2)
  {
    _numberTimesCalled = CTNoData;
  }

  uint32_t startIdx = _parenPairIndices[1].first+1;
  uint32_t endIdx = _parenPairIndices[1].second-1;

  if (startIdx > endIdx)
  {
    //Malformed
    invalidate();
    return;
  }

  if (startIdx == endIdx)
  {
    _numberTimesCalled = info->getToken(startIdx);
    _numericNumberTimesCalled = convertLargeNumberString(_numberTimesCalled, 'x');
  }
  else
  {
    std::pair<uint32_t,uint32_t> idxPair(startIdx,endIdx);
    _numberTimesCalled = info->constructMultiToken(idxPair, CTTokenVector::ConstructTokenNoSpaces);
    //Convert number times called
    _numericNumberTimesCalled = convertLargeNumberString(_numberTimesCalled, 'x');
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::processClassName(CTTokenVector*& info)
{
  //if there is a namespace, from current double colon to the next double colon,
  //otherwise from current single colon to the next double colon
  if (!_valid)
  {
    return;
  }
  if (_doubleColonIndices.empty())
  {
    //Malformed
    invalidate();
    return;
  }

  uint32_t parenIdx = _parenPairIndices[0].first;
  uint32_t startIdx=0, endIdx=0;

  if ((_classNamespace.empty() || 
      _classNamespace.size() == 0 || 
      _classNamespace == CTNoData) &&
      (_doubleColonIndices.size() >= 1))
  {
    //std::cout << "no namespace, class name is after second single colon, before first double" << std::endl;
    //No namespace given -- before the first set of double colons
    //Get the index of the second colon
    if (_singleColonIndices.size() < 2)
    {
      //std::cout << "only one colon in the vector, invalidating" << std::endl;
      //Malformed
      invalidate();
      return;
    }
    //std::cout << "found class name between single colon and double colon" << std::endl;
    startIdx = _singleColonIndices[1] + 1;
    endIdx = _doubleColonIndices[0].first - 1;
  }
  else if ((!_classNamespace.empty())&&(_doubleColonIndices.size() >= 2))
  {
    //Namespace given
    //Between the first and second sets of double colons
    //std::cout << "namespace exists, class name is between two double colons" << std::endl;
    startIdx = _doubleColonIndices[0].second+1;
    endIdx = _doubleColonIndices[1].first-1;    
  }

  if (startIdx > endIdx)
  {
    //Malformed
    invalidate();
    return;
  }

  if (endIdx > parenIdx)
  {
    //Malformed
    invalidate();
  }
    
  uint32_t diffI = endIdx - startIdx;

  if (diffI == 0)
  {
    _className = info->getToken(startIdx);
  }
  else
  {
    std::pair<uint32_t,uint32_t> idxPair(startIdx,endIdx);
    _className = info->constructMultiToken(idxPair,
      CTTokenVector::ConstructTokenNoSpaces);
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeMethod::processFunctionSignature(CTTokenVector*& info)
{
  //after function name encountered opening parentheses to next close parentheses
  if (!_valid)
  {
    return;
  }

  //First set of parentheses contains the function signature
  std::pair<uint32_t,uint32_t>& firstParenPair = _parenPairIndices[0];

  if (firstParenPair.first > firstParenPair.second)
  {
    //Malformed
    invalidate();
    return;
  }

  if (firstParenPair.first == firstParenPair.second)
  {
    //Blank signature    
    _classMethodParams = CTNoData;
  }
  else
  {
    std::pair<uint32_t,uint32_t> signatureBounds = std::pair<uint32_t,uint32_t>(firstParenPair.first+1,firstParenPair.second-1);
    _classMethodParams = info->constructMultiToken(
      signatureBounds, 
      CTTokenVector::ConstructTokenAddSpaces);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CallTreeMethod::processInfoLine(CTTokenVector*& infoLine)
{
  std::vector<std::string>& tokens = infoLine->tokens();

  //Check if the minimum number of tokens exist for all types
  if (tokens.empty() || tokens.size() == 0)
  {
    invalidate();
    return;
  }

  if (tokens.size() < CTMethodMinValidTokenSize)
  {
    invalidate();
    return;
  }

  //Get cost and type -- universal across all types of methods
  //and source files
  std::string& costStr = tokens[CTMethodCostIndex];

  if (costStr.empty())
  {
    invalidate();
    return;
  }

  _cost = costStr;

  //Calculate numeric cost
  _numericCost = convertLargeNumberString(_cost, ',');
  //std::cout << "End cost total = " << _numericCost << std::endl;
  
  std::string& typeStr = tokens[CTMethodTypeIndex]; 
  
  if (typeStr.empty())
  {
    invalidate();
    return;
  }
  
  if (typeStr == CTStartMethod)
  {
    _type = CallTreeMethod::CallTreeMethodParent;
    _valid = true;
    processStartMethodLine(infoLine);
    if (!_valid)
    {
      return;
    }
  }
  else if (typeStr == CTContinueMethod)
  {
    _type = CallTreeMethod::CallTreeMethodChild;
    _valid = true;
    processContinueMethodLine(infoLine);
    if (!_valid)
    {
      return;
    }
  }
  else
  {
    invalidate();
    return;
  }

  //Output fields of the method
  std::cout << "***** Method Output *****" << std::endl 
            << "-              cost = " << _cost << std::endl
            << "-       numericCost = " << _numericCost << std::endl
            << "-              type = " << _type << std::endl
            << "-        sourceType = " << _sourceType << std::endl
            << "-        sourceFile = " << _sourceFile << std::endl
            << "-    classNamespace = " << _classNamespace << std::endl
            << "-         className = " << _className << std::endl
            << "-   classMethodName = " << _classMethodName << std::endl
            << "- numberTimesCalled = " << _numberTimesCalled << std::endl
            << "-numericTimesCalled = " << _numericNumberTimesCalled << std::endl
            << "-          miscInfo = " << _miscInfo << std::endl
            << "-             valid = " << _valid << std::endl
            << "*****  End Method  *****" << std::endl;
}
