#include <iostream>
#include <algorithm>
#include <functional>
#include <boost/tokenizer.hpp>
#include <fstream>

#include "CTConstants.h"
#include "CTTokenVector.h"
#include "CallTreeHeader.h"
#include "CallTreeMethod.h"
#include "CallTreeMethodTree.h"
#include "CallTreeDataCollector.h"
#include "CallTreeDataAnalyzer.h"
#include "CallTreeXMLGenerator.h"
#include "CallTreeChartData.h"

extern "C" {
#define HAVE_LIBFREETYPE
#include "gdchart.h"
#include "gdcpie.h"
}

#include <stdlib.h>

using namespace std;
using namespace tse;

class MethodStrictWeakOrdering: 
  public binary_function< 
  std::pair<double, std::pair<double,uint32_t> >, 
  std::pair<double, std::pair<double,uint32_t> >, 
  bool>
{
public:
  MethodStrictWeakOrdering()
  {
  }

  bool operator()(const std::pair<double, std::pair<double,uint32_t> >& x, 
                  const std::pair<double, std::pair<double,uint32_t> >& y)
  {
    if (x.second.first < y.second.first)
    {
      return(true);
    }

    return(false);
  }
};

class MethodCostRatioWeakOrdering: 
  public binary_function< 
  std::pair<double, std::pair<double,uint32_t> >, 
  std::pair<double, std::pair<double, uint32_t> >, 
  bool>
{
private:
  std::vector<CallTreeMethodTree*>* _methodTrees;

public:
  MethodCostRatioWeakOrdering(
    std::vector<CallTreeMethodTree*>* methodTrees) :
    _methodTrees(methodTrees)
  {
  }

  bool operator()(const std::pair<double, std::pair<double,uint32_t> >& x, 
                  const std::pair<double, std::pair<double,uint32_t> >& y)
  {
    if (x.first < y.first)
    {
      return(true);
    }

    return(false);
  }
};
  
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataAnalyzer::analyze()
{
  std::cout << "CallTreeDataAnalyzer::analyze()" << std::endl;
  if (_methodTrees->empty())
  {
    std::cout << "method trees is empty, returning false" << std::endl;
    return(false);
  }

  //Find methods with largest number of children
  //and highest cost by making an ordered vector
  std::vector< std::pair<double, std::pair<double, uint32_t> > > costChildRatioIndex;
  std::vector<CallTreeMethodTree*>::iterator bIter = _methodTrees->begin();
  std::vector<CallTreeMethodTree*>::iterator eIter = _methodTrees->end();

  for (; bIter != eIter; ++bIter)
  {
    CallTreeMethodTree* curTree = *bIter;
    if (curTree == nullptr)
    {
      std::cout << "-- curTree is null, continuing" << std::endl;
      continue;
    }

    CallTreeMethod* curMethod = curTree->method();
    if (curMethod == nullptr)
    {
      std::cout << "-- curMethod is null, continuing" << std::endl;
      continue;
    }
    
    CallTreeMethod::CallTreeMethodType& type = curMethod->type();    
    uint32_t numChildren = curTree->children().size();
    
    if (type != CallTreeMethod::CallTreeMethodParent && numChildren == 0)
    {
      std::cout << "-- numChildren is zero, continuing" << std::endl;
      continue;
    }

    std::string& numCalledStr = curMethod->numberTimesCalled();
    if ((type != CallTreeMethod::CallTreeMethodParent)&&(numCalledStr.empty() || numCalledStr.length() == 0))
    {
      std::cout << "-- numCalledStr is empty or zero length" << std::endl;
      continue;
    }

    uint64_t numCalled = curMethod->numericNumberTimesCalled();
    if (type != CallTreeMethod::CallTreeMethodParent && numCalled == 0)
    {
      std::cout << "-- number times called is zero, continuing" << std::endl;
      continue;
    }

    uint64_t curMethodCost = curMethod->numericCost();
    if (curMethodCost == 0)
    {
      std::cout << "-- curMethodCost is zero, continuing" << std::endl;
      continue;
    }

    double ratio = 0.00;
    if (numChildren > 0)
    {
      ratio = static_cast<double>(curMethodCost) / 
        static_cast<double>(numChildren);
    }
    else
    {
      ratio = static_cast<double>(curMethodCost);
    }

    double costCallRatio = 0.00;
    if (numCalled > 0)
    {
      costCallRatio = static_cast<double>(curMethodCost) / static_cast<double>(numCalled);
    }
    else
    {
      costCallRatio = static_cast<double>(curMethodCost);
    }
    
    costChildRatioIndex.push_back(std::pair<double, std::pair<double, uint32_t> >(costCallRatio, std::pair<double, uint32_t>(ratio, bIter-_methodTrees->begin())));
  }

  if (costChildRatioIndex.empty() || costChildRatioIndex.size() == 0)
  {
    std::cout << "- no costchildratio pairs found" << std::endl;
    return(false);
  }

  //Sort the cost child ratio index in descending order
  //according to the MethodStrictWeakOrdering functor
  std::sort(costChildRatioIndex.begin(), 
            costChildRatioIndex.end(), 
            MethodStrictWeakOrdering());

  //Now we need to find which methods in this vector have the highest
  //call to cost ratio -- use the MethodCostRatioWeakOrdering
  std::sort(costChildRatioIndex.begin(),
            costChildRatioIndex.end(),
            MethodCostRatioWeakOrdering(_methodTrees));

  std::vector< std::pair<double, std::pair<double, uint32_t> > >::iterator iter = costChildRatioIndex.begin();
  std::vector< std::pair<double, std::pair<double, uint32_t> > >::iterator iterEnd = costChildRatioIndex.end();

  std::cout << "Initial analysis results:" << std::endl;
  for (; iter != iterEnd; ++iter)
  {
    std::pair<double, std::pair<double, uint32_t> >& curPair = *iter;
    CallTreeMethodTree* curTree = _methodTrees->operator[](curPair.second.second);
    CallTreeMethod* curMethod = curTree->method();
    std::cout << "(CostRatio  =" << setw(12) << curPair.first 
              << ", ChildRatio =" << setw(12) << curPair.second.first
              << ")" << setw(4) << "[" 
              << iter - costChildRatioIndex.begin()
              << "] = " << curMethod->className() << "::" << curMethod->classMethodName()
              << ", Cost = " << curMethod->numericCost() << " cycles, #Calls = " << curMethod->numericNumberTimesCalled() << " times"
              << ", #Children = " << curTree->children().size()
              << ", #Parent   = " << curTree->parents().size()
              << std::endl;
  }    
  
  return(true);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataAnalyzer::generateXMLResponse(const string& filename)
{
    CallTreeXMLGenerator generator(_header, _methodTrees, this);
    ofstream output(filename.c_str());
    output << generator.generate() << std::endl;
    return(true);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeDataAnalyzer::generateCharts()
{
    unsigned numFunc = atoi(_commandLineInput.numberFunctions().c_str());
    CallTreeChartData chartData(numFunc,_methodTrees);

    //unsigned long  clrs[6]  = { 0xFF0000, 0x00FF00, 0x0000FF,   0xFF00FF, 0xFFFF00, 0x00FFFF }; 


    GDCPIE_Color = chartData.colors();
    GDCPIE_EdgeColor  = 0x000000L;
    GDCPIE_BGColor    = 0xFFFFFFL;
    GDCPIE_LineColor  = 0x000000L;
    //GDCPIE_label_line = 1;
    //GDCPIE_label_dist = 10;
    GDCPIE_title      = "Performance Chart";
    string fileName = _commandLineInput.outputDir() + "/chart.png";

    FILE* fp = fopen(fileName.c_str(), "wb" );

    pie_gif( 800,
             700,
             fp, 
             GDC_3DPIE,
             numFunc,  
             chartData.labels(),
             chartData.values() ); 

    fclose( fp );

    _chartFileNames.push_back(fileName);

    return true ;
}

