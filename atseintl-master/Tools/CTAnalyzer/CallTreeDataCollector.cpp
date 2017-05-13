#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "Common/TseStringTypes.h"
#include <boost/tokenizer.hpp>

#include "CTConstants.h"
#include "CTTokenVector.h"
#include "CallTreeHeader.h"
#include "CallTreeMethod.h"
#include "CallTreeMethodTree.h"
#include "CallTreeDataCollector.h"

using namespace std;
using namespace tse;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataCollector::readNextLine()
{
  bool rt = false;
  //std::cout << "CallTreeDataCollector::readNextLine()" << std::endl;
  if (_isFileOpen)
  {          
    _file.getline(_innerBuffer, INNERBUFFER_SIZE);

    //Make sure the read succeeded
    if ((!_file.fail())&&(!_file.eof()))
    {
      //std::cout << "- getline was successful" << std::endl;
      //Copy the buffer into the string
      _innerBufferStr = _innerBuffer;
      //std::cout << "- line read = " << _innerBufferStr << std::endl;

      if (_innerBufferStr.length() == 0)
      {
        _innerBufferStr = CTBlank;
      }

      //Create the tokenizer function
      boost::char_separator<char> charSep(" -\t\n\r\0", ":()[]", boost::drop_empty_tokens);

      //Create the tokenizer
      boost::tokenizer< boost::char_separator<char> > tok(_innerBufferStr, charSep);

      //Insert the CTTokenVector struct into the _lines vector
      _lines.push_back(CTTokenVector());

      //Get a reference to the token vector just inserted
      CTTokenVector& ctTokenVector = static_cast<CTTokenVector&>(_lines.back());
      
      //Create the iterators
      boost::tokenizer< boost::char_separator<char> >::iterator tokIter = tok.begin();
      boost::tokenizer< boost::char_separator<char> >::iterator tokEndIter = tok.end();

      //Grab the tokens and insert them into the vector
      for (; tokIter != tokEndIter; ++tokIter)
      {
        const std::string& curTok = *tokIter;
        ctTokenVector.tokens().push_back(curTok);
        //std::cout << "<" << curTok << ">";
      }
      //std::cout << std::endl;

      //If the token vector is empty, push the blank signifier
      if ((ctTokenVector.tokens().empty()) || (ctTokenVector.tokens().size() == 0))
      {
        ctTokenVector.tokens().push_back(CTBlank);
      }

      //Increase the line count
      _currentLineCount++;

      //Set status flag to true
      rt = true;
    }
    else if (_file.eof())
    {
      //std::cout << "End of file reached." << std::endl;
      rt = true;
    }
    else
    {
      //std::cout << "- getline operation failed" << std::endl;
    }
  }

  return(rt);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//For advancing to the next stored line
unsigned int 
CallTreeDataCollector::advanceLine(AdvanceLineMode aLMode)
{
  if (_lines.empty())
  {
    return(0);
  }

  unsigned int lineTrack = _currentLine;

  if ((_currentLine == 0)&&(_currentLineData == nullptr))
  {
    _currentLineData = &(_lines[_currentLine]);
  }
  else
  {
    ++_currentLine;
    if (_currentLine < _currentLineCount)
    {
      _currentLineData = &(_lines[_currentLine]);
    }
  }
  _blankLine = false;

  if (_currentLineData->tokens().front() == CTBlank)
  {
    _blankLine = true;
    if (aLMode == ADVANCELINE_SKIPBLANK)
    {
      advanceLine(aLMode);
    }
  }

  lineTrack = _currentLine - lineTrack;

  //Make sure we return a one even if the first line is chosen in the 
  //currentLine equal to zero case
  lineTrack = (lineTrack > 0) ? lineTrack : 1;

  return(lineTrack);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CallTreeDataCollector::endCurrentMethodTree()
{
  if (_startedTree)
  {
    //End the current tree
    _startedTree = false;

    //Insert it into the map based on its cost
    _callTreeMethodTrees[_currentMethodTree->method()->cost()] = _currentMethodTree;

    //Reset the current method tree
    _currentMethodTree = nullptr;
  }    
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CallTreeDataCollector::startNewMethodTree()
{
  if (!_startedTree)
  {
    _currentMethodTree = createMethodTree();

    if (!_currentMethodTree)
    {
      return;
    }

    //Set started tree to true
    _startedTree = true;

    //Get a reference to the method
    CallTreeMethod*& newMethod = _currentMethodTree->method();

    //Process current method line
    newMethod->processInfoLine(_currentLineData);

    if (!(newMethod->valid()))
    {
      delete _currentMethodTree;
      _currentMethodTree = nullptr;
      _startedTree = false;
      //std::cout << "New method is not valid." << std::endl;
      return;
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CallTreeDataCollector::addToCurrentMethodTree()
{
  if (_startedTree)
  {
    CallTreeMethodTree* childMethodTree = createMethodTree();

    if (!_currentMethodTree)
    {
      return;
    }

    //Get a reference to the method
    CallTreeMethod*& newMethod = childMethodTree->method();

    //Process current method line
    newMethod->processInfoLine(_currentLineData);

    if (!(newMethod->valid()))
    {
      delete childMethodTree;
      //std::cout << "Method is not valid." << std::endl;
      return;
    }

    //Add this method tree to the current method tree
    _currentMethodTree->addChild(childMethodTree); 
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CallTreeMethodTree* 
CallTreeDataCollector::createMethodTree()
{
  CallTreeMethodTree* rt = nullptr;

  try
  {
    rt = new CallTreeMethodTree();
  }
  catch(std::exception& e)
  {
    std::cerr << "In createMethodTree():" << std::endl;
    std::cerr << "CallTreeMethodTree allocation caused an exception." << std::endl;
    std::cerr << "Exception caught: " << e.what() << std::endl;
    return(nullptr);
  }

  try
  {
    rt->method() = new CallTreeMethod();
  }
  catch(std::exception& e)
  {
    delete rt;
    rt = nullptr;
    std::cerr << "In createMethodTree():" << std::endl;
    std::cerr << "CallTreeMethod allocation caused an exception." << std::endl;
    std::cerr << "Exception caught: " << e.what() << std::endl;
    return(nullptr);
  }
  
  return(rt);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CallTreeMethod::CallTreeMethodType 
CallTreeDataCollector::determineMethodType()
{
  //Determine method type
  const std::string& methodType = _currentLineData->getToken(CTMethodTypeIndex);
  
  if (methodType == CTStartMethod)
  {
    return(CallTreeMethod::CallTreeMethodParent);
  }
  else if (methodType == CTContinueMethod)
  {
    return(CallTreeMethod::CallTreeMethodChild);
  }

  return(CallTreeMethod::CallTreeMethodNumTypes);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataCollector::processMode()
{
  bool rt = true;
  unsigned int advanceLineCount=0;
  if (_currentLine >= _lines.size())
  {
    return(false);
  }

  switch(_state)
  {
    case CTCS_INITIALSETUP:
    {
      //Get cache I1
      readHeaderLine(CTHeaderI1Range, _callTreeHeader.cacheI1());

      //Get cache D1
      readHeaderLine(CTHeaderD1Range, _callTreeHeader.cacheD1());

      //Get cache L2
      readHeaderLine(CTHeaderL2Range, _callTreeHeader.cacheL2());

      //Get time range description
      readHeaderLine(CTHeaderTimeRange, _callTreeHeader.timeRangeDescription());

      //Get trigger
      readHeaderLine(CTHeaderTriggerRange, _callTreeHeader.triggerType());

      //Get target description
      readHeaderLine(CTHeaderTargetDescriptionRange, _callTreeHeader.targetDescription());

      //Need to skip lines to get the threshold
      multiAdvanceLine(CTHeaderInitialSkipSize, ADVANCELINE_SKIPBLANK);

      //Get threshold index
      readHeaderLine(CTHeaderThresholdIndex, _callTreeHeader.threshold());

      //Need to skip lines to get the number of cycles
      multiAdvanceLine(CTHeaderSecondSkipSize, ADVANCELINE_SKIPBLANK);

      //Get the cycles
      readHeaderLine(CTHeaderCycleCountIndex, _callTreeHeader.totalCycleCount());

      //Set the next state
      _state = CTCS_GETLINE;

      break;
    }
    case CTCS_GETLINE:
    {
      advanceLineCount = advanceLine(ADVANCELINE_NOSKIP);
      _state = CTCS_PROCESSLINE;
      break;
    }
    case CTCS_PROCESSLINE:
    {
      //Process the line, analyze its type
      if (_blankLine)
      {
        //Blank line state
        if (_startedTree)
        {
          endCurrentMethodTree();
        }

        //Go to the next line
        _state = CTCS_GETLINE;
      }
      else
      {
        CallTreeMethod::CallTreeMethodType mType = determineMethodType();

        if (_startedTree)
        {
          switch(mType)
          {
          case CallTreeMethod::CallTreeMethodParent:
            endCurrentMethodTree();
            _state = CTCS_LINESTARTTREE;
            break;
          case CallTreeMethod::CallTreeMethodChild:
            _state = CTCS_LINEMIDTREE;
            break;
          default:
            //std::cout << "Invalid state found during PROCESSLINE state" << std::endl;
            return(false);
          }                        
        }
        else
        {
          _state = CTCS_LINESTARTTREE;
        }
      }                    
      break;
    }
    case CTCS_LINESTARTTREE:
    {
      startNewMethodTree();
      _state = CTCS_GETLINE;
      break;
    }
    case CTCS_LINEMIDTREE:
    {
      addToCurrentMethodTree();
      _state = CTCS_GETLINE;
      break;
    }
    case CTCS_FINISHED:
    {
      //TODO: End of processing
      break;
    }

    //If we are not in a valid state, exit
    default:
    {
      // std::cout << "Invalid state encountered in CallTreeDataCollector::calculateMode().  Exiting..." << std::endl;
      exit(1);
    }
  }

  return(rt);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CallTreeDataCollector::recursiveTreeConstruct(CallTreeMethodTree* parent)
{
  std::cout << "recursiveTreeConstruct()..." << std::endl;
  if (parent == nullptr)
  {
    std::cout << "- parent is NULL, returning" << std::endl;
    return;
  }

  if (parent->method() == nullptr)
  {
    std::cout << "- parent method is NULL, returning" << std::endl;
    return;
  }

  uint64_t parMethodCost = parent->method()->numericCost();
  if (parMethodCost < CTCycleEpsilon)
  {
    std::cout << "- parent method cost is less than " << CTCycleEpsilon << ", returning" << std::endl;
    return;
  }

  if (parent->children().empty() || parent->children().size() == 0)
  {
    std::cout << "- parents children vector is empty, returning" << std::endl;
    return;
  }

  //Go through children of parent
  //Attempt to find this name in the map of calltreemethodtree's
  //When found, assign the passed in parent as
  //a parent to the found object
  std::vector<CallTreeMethodTree*>::iterator cIter = parent->children().begin();
  std::vector<CallTreeMethodTree*>::iterator cEIter = parent->children().end();
  for (; cIter != cEIter; ++cIter)
  {
    CallTreeMethodTree* curTree = *cIter;    
    CallTreeMethod* curTreeMethod = curTree->method();
    uint64_t curTreeMethodCost = curTreeMethod->numericCost();
    if (curTreeMethodCost < CTCycleEpsilon)
    {
      //std::cout << "-- curTreeMethod cost < " << CTCycleEpsilon << ", continuing" << std::endl;      
      continue;
    }
    std::string methodKey(curTreeMethod->classNamespace());
    methodKey.append(curTreeMethod->className());
    methodKey.append(curTreeMethod->classMethodName());

    std::map<std::string, CallTreeMethodTree*>::iterator cMapIter = _callTreeMethodTrees.begin();
    std::map<std::string, CallTreeMethodTree*>::iterator cMapEIter = _callTreeMethodTrees.end();
    bool recurse = false;
    for (; cMapIter != cMapEIter; ++cMapIter)
    {
      CallTreeMethodTree* cMapTree = cMapIter->second;     
      CallTreeMethod* cMapTreeMethod = cMapTree->method();
      uint64_t cMapTreeMethodCost = cMapTreeMethod->numericCost();

      if (cMapTreeMethodCost < CTCycleEpsilon)
      {
        //std::cout << "--- cMapTreeMethod cost < " << CTCycleEpsilon << ", continuing" << std::endl;
        continue;
      }
      std::string cMethodKey(cMapTreeMethod->classNamespace());
      cMethodKey.append(cMapTreeMethod->className());
      cMethodKey.append(cMapTreeMethod->classMethodName());

      if (cMethodKey != methodKey)
      {
        continue;
      }
      std::cout << "--- MethodKey("<<methodKey<<")=("<<cMethodKey<<")-recursing"<<std::endl;
      cMapTree->addParent(parent);
      recurse = true;
      break;
    }
    if (recurse)
    {
      recursiveTreeConstruct(curTree);
    }
  }

  return;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Create the method tree
bool 
CallTreeDataCollector::createEntireMethodTree()
{
  bool rt = false;

  std::map<std::string, CallTreeMethodTree*>::reverse_iterator rIter = _callTreeMethodTrees.rbegin();
  std::map<std::string, CallTreeMethodTree*>::reverse_iterator rEIter = _callTreeMethodTrees.rend();

  //This loop adds the obvious parent pointer to the children method trees(ie the > methods)
  //for each of the main method trees (ie the * methods)
  for (; rIter != rEIter; ++rIter)
  {
    const std::pair<std::string, CallTreeMethodTree*>& curPair = *rIter;
    CallTreeMethodTree* curTree = curPair.second;
    if (!curTree->method()->valid())
    {
      continue;
    }

    if (!(curTree->children().empty()))
    {
      std::vector<CallTreeMethodTree*>::iterator parIter = curTree->children().begin();
      std::vector<CallTreeMethodTree*>::iterator parEIter = curTree->children().end();
      uint32_t idx = 0;

      for (; parIter != parEIter; ++parIter)
      {
        CallTreeMethodTree*& curChild = *parIter;

        //Add parent to child -- in this case the parent is curTree
        curChild->addParent(curTree);
        ++idx;
      }
    }
  }

  std::cout << "**************************************************" << std::endl;   
  
  std::map<std::string, CallTreeMethodTree*> trees(_callTreeMethodTrees);
  while (1)
  {    
    if (trees.empty())
    {
      rt = true;
      break;
    }

    CallTreeMethodTree* maxTree = findHighestCostTree(trees);
    if (maxTree == nullptr)
    {
      rt = false;
      break;
    }
    _highestToLowestCostMethods.push_back(maxTree);
    recursiveTreeConstruct(maxTree);
  }

  std::cout << "Finished tree construction" << std::endl;

  return(true);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CallTreeMethodTree* 
CallTreeDataCollector::findHighestCostTree(std::map<std::string, CallTreeMethodTree*>& trees)
{
  CallTreeMethodTree* rt = nullptr;

  std::map<std::string, CallTreeMethodTree*>::iterator mIter = trees.begin();
  std::map<std::string, CallTreeMethodTree*>::iterator mEIter = trees.end();
  uint64_t maxCost = 0;
  std::map<std::string, CallTreeMethodTree*>::iterator maxIter;

  bool foundAtLeastOneMax = false;
  for (; mIter != mEIter; ++mIter)
  {
    const std::pair<std::string, CallTreeMethodTree*>& curTreePair = *mIter;
    CallTreeMethodTree* curTree = curTreePair.second;
    CallTreeMethod* curMethod = curTree->method();
    uint64_t methodCost = curMethod->numericCost();
    if (methodCost > maxCost)
    {
      maxCost = methodCost;
      maxIter = mIter;
      rt = curTree;
      foundAtLeastOneMax = true;
    }
  }

  if (!foundAtLeastOneMax)
  {
    return(nullptr);
  }

  //std::cout << "findHighestCostTree() returning " << maxCost << " with method " << rt->method()->className() << "::" << rt->method()->classMethodName() << std::endl;

  //Remove this max method from the map
  trees.erase(maxIter);
  return(rt);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CallTreeDataCollector::outputTree(CallTreeMethodTree* parent)
{
  if (parent == nullptr)
  {
    return;
  }

  bool leafNode = false;

  if ((parent->children().empty() || parent->children().size() == 0))
  {
    leafNode = true;
  }

  std::cout << "IsLeaf=" << leafNode << std::endl;
  if (parent->method() == nullptr)
  {
    return;
  }
  std::cout << "Method=" << parent->method()->classNamespace() << "::" << parent->method()->className() << "::" << parent->method()->classMethodName() << "(" << parent->method()->numericCost() << ")" <<std::endl;
  if (leafNode)
  {
    return;
  }
  if (parent->children().empty())
  {
    return;
  }
  std::vector<CallTreeMethodTree*>::iterator cIter = parent->children().begin();
  std::vector<CallTreeMethodTree*>::iterator cEIter = parent->children().end();
  uint32_t idx = 0;
  for (; cIter != cEIter; ++cIter)
  {
    std::cout << "NChild("<<idx<<"):"<<std::endl;
    outputTree(*cIter);
    ++idx;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataCollector::readHeaderLine(const std::pair<uint32_t,uint32_t>& range, 
                                      std::string& outData)
{
  bool rt = false;
  uint32_t advanceLineCount = 0;
  
  advanceLineCount = advanceLine(ADVANCELINE_SKIPBLANK);
  if (advanceLineCount > 0)
  {
    outData = _currentLineData->constructMultiToken(range, 
                                                    CTTokenVector::ConstructTokenAddSpaces);
    rt = true;
  }

  return(rt);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataCollector::readHeaderLine(const uint32_t& index, std::string& outData)
{
  bool rt = false;
  uint32_t advanceLineCount = 0;
  
  advanceLineCount = advanceLine(ADVANCELINE_SKIPBLANK);
  if (advanceLineCount > 0)
  {
    outData = _currentLineData->getToken(index);
    rt = true;
  }

  return(rt);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
CallTreeDataCollector::multiAdvanceLine(const uint32_t& numTimes, 
                                        AdvanceLineMode aLMode)
{
  for (uint32_t j = 0; j < numTimes; j++)
  {
    advanceLine(aLMode);
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataCollector::processFile()
{
  bool rt = false;
  //std::cout << "CallTreeDataCollector::process()" << std::endl;
  if (_isFileOpen)
  {
    rt = true;
    //std::cout << "- File is open " << std::endl;

    while (!_file.eof())
    {
      bool status = readNextLine();

      if (!status)
      {
        rt = false;
        //std::cout << "Did not read line successfully." << std::endl;
        break;
      }
    }
  }
  return(rt);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool
CallTreeDataCollector::processData()
{
    bool rt = true;

  std::cout << "processData()..." << std::endl;

  while(processMode());

  std::cout << "****** --Header-- ******" << std::endl;
  std::cout << "* - cacheI1       = " << _callTreeHeader.cacheI1() << std::endl;
  std::cout << "* - cacheD1       = " << _callTreeHeader.cacheD1() << std::endl;
  std::cout << "* - cacheL2       = " << _callTreeHeader.cacheL2() << std::endl;
  std::cout << "* - timeRangeDesc = " << _callTreeHeader.timeRangeDescription() << std::endl;
  std::cout << "* - triggerType   = " << _callTreeHeader.triggerType() << std::endl;
  std::cout << "* - targetDesc    = " << _callTreeHeader.targetDescription() << std::endl;
  std::cout << "* - threshold     = " << _callTreeHeader.threshold() << std::endl;
  std::cout << "* - totalCycleCnt = " << _callTreeHeader.totalCycleCount() << std::endl;
  std::cout << "************************" << std::endl;
  createEntireMethodTree();
  
  std::cout << "Finished processData().  Returning " << rt << std::endl;

  return(rt);
}
