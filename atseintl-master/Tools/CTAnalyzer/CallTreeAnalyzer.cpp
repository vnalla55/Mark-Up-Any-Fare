#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <boost/tokenizer.hpp>

#include "CTConstants.h"
#include "CTTokenVector.h"
#include "CallTreeHeader.h"
#include "CallTreeMethod.h"
#include "CallTreeMethodTree.h"
#include "CallTreeDataCollector.h"
#include "CallTreeCommandLineProcessor.h"
#include "CallTreeDataAnalyzer.h"
#include "CallTreeAnalyzer.h"

using namespace std;
using namespace tse;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int 
main(int argc, char* argv[])
{
  std::cout << "Starting CallTreeAnalyzer..." << std::endl << std::endl;
  
  CallTreeCommandLineProcessor cTComLineProc(argc, argv);


  if (cTComLineProc.inputValid() == false)
  {
    return(0);
  }

  std::cout << "Input is valid, processing file..." << std::endl;
  CallTreeDataCollector callTreeDataCollector(cTComLineProc.inputFileName());
  callTreeDataCollector.processFile();
  callTreeDataCollector.processData();
  CallTreeDataAnalyzer callTreeDataAnalyzer(cTComLineProc, 
                                            &callTreeDataCollector.callTreeHeader(),
                                            &callTreeDataCollector.highestToLowestCostMethods());

  if (!callTreeDataAnalyzer.analyze())
  {
    std::cout << "Could not analyze processed data!" << std::endl;
    return(0);
  }

  if (!callTreeDataAnalyzer.generateCharts())
  {
    std::cout << "Could not generate charts!" << std::endl;
    return(0);
  }

  if (!callTreeDataAnalyzer.generateXMLResponse(cTComLineProc.outputDir()
          + "/data.xml"))
  {
      std::cout << "Could not generate XML response!" << std::endl;
      return(0);
  }



  std::cout << "Analyzed processed data successfully." << std::endl;


  std::cout << std::endl << "Successfully finished CallTreeAnalyzer." << std::endl;
  return(0);
}
