#ifndef CALL_TREE_DATA_ANALYZER_H
#define CALL_TREE_DATA_ANALYZER_H

#include <vector>
#include "CallTreeCommandLineProcessor.h"

namespace tse
{

class CallTreeHeader;
class CallTreeMethodTree;

class CallTreeDataAnalyzer
{
 public:
  typedef enum AnalysisTopLevelMode
  {
    AnalysisTopLevelAmount     = 0,
    AnalysisTopLevelPercentage = 1
  };

  typedef enum AnalysisDetailLevelMode
  {
    AnalysisDetailLevelAmount     = 0,
    AnalysisDetailLevelPercentage = 1
  };

  typedef enum AnalysisChartDetailFlags
  {
    AnalysisChartFunctionName             = 0x00000001,
    AnalysisChartFunctionPercentage       = 0x00000002,
    AnalysisChartFunctionNumberTimes      = 0x00000004,
    AnalysisChartFunctionCostCPUSeconds   = 0x00000008,
    AnalysisChartFunctionCostCPUCycles    = 0x00000010,
    AnalysisChartFunctionCostRatioCycles  = 0x00000020,
    AnalysisChartFunctionCostRatioSeconds = 0x00000040,
    AnalysisChartFunctionCostAbsolute     = 0x00000080,
    AnalysisChartFunctionCostRelative     = 0x00000100,
    AnalysisChartFunctionParentLinkage    = 0x00000200,
    AnalysisChartFunctionChildLinkage     = 0x00000400
  };
 private:
  //Method hiding for copy constructor
  CallTreeDataAnalyzer(const CallTreeDataAnalyzer& rhs);
  CallTreeDataAnalyzer operator=(const CallTreeDataAnalyzer& rhs);

 private:
  CallTreeHeader* _header;
  std::vector<CallTreeMethodTree*>* _methodTrees;
  std::vector<const CallTreeMethodTree*> _hotspots;
  std::vector<std::string> _chartFileNames;
  CallTreeMethodTree* _currentMethod;
  CallTreeCommandLineProcessor& _commandLineInput;

 private:
  bool analyzeCurrentMethod();
  bool generateCurrentMethodChart();

 public:
  explicit CallTreeDataAnalyzer(CallTreeCommandLineProcessor& ctProc,
                                CallTreeHeader* header, 
                                std::vector<CallTreeMethodTree*>* methodTrees) :
    _header(header),
    _methodTrees(methodTrees), 
    _currentMethod(nullptr),
    _commandLineInput(ctProc)
  {
  }

  ~CallTreeDataAnalyzer(){};
    
  bool analyze();
  bool generateXMLResponse(const std::string& filename);
  bool generateCharts();

  std::vector<const CallTreeMethodTree*>& hotspots() { return(_hotspots);};
  const std::vector<const CallTreeMethodTree*>& hotspots() const { return(_hotspots);};

  std::vector<std::string>& chartFileNames() { return(_chartFileNames);};
  const std::vector<std::string>& chartFileNames() const { return(_chartFileNames);};
};

} //End namespace tse

#endif //CALL_TREE_DATA_ANALYZER_H
