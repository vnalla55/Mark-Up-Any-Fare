#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "Common/TseStringTypes.h"
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "CTConstants.h"
#include "CTTokenVector.h"
#include "CallTreeHeader.h"
#include "CallTreeMethod.h"
#include "CallTreeMethodTree.h"
#include "CallTreeXMLGenerator.h"
#include "CallTreeDataCollector.h"
#include "CallTreeCommandLineProcessor.h"
#include "CallTreeDataAnalyzer.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace tse;
using namespace boost;

string CallTreeXMLGenerator::generate()
{
    stringstream output;
    output << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" << endl
           << "<PerformanceData>" << endl;
    generateHeader(output);
    generateHotspots(output);
    generateFunctions(output);
    output << "</PerformanceData>" << endl;

    return output.str();
}

void CallTreeXMLGenerator::generateHeader(ostream& output)
{
    if (!_header)
        return;
    posix_time::ptime time = posix_time::second_clock::local_time();

    output << "<PerformanceDataHeader>" << endl
           << "<AnalysisDate>" 
           << to_simple_string(gregorian::day_clock::local_day())
           << "</AnalysisDate>" << endl
           << "<AnalysisTime>" 
           << setfill('0') << setw(2) << setfill('0')
           << time.time_of_day().hours() << time.time_of_day().minutes()
           << "</AnalysisTime>" << endl
           << "<TotalCycleCost>" << _header->totalCycleCount()
           << "</TotalCycleCost>" << endl
           << "<CacheI1>" << _header->cacheI1() << "</CacheI1>" << endl
           << "<CacheD1>" << _header->cacheD1() << "</CacheD1>" << endl
           << "<CacheL2>" << _header->cacheL2() << "</CacheL2>" << endl
           << "<TimeRangeDescription>" 
           << _header->timeRangeDescription()
           << "</TimeRangeDescription>" << endl
           << "<TriggerType>" << _header->triggerType() << "</TriggerType>" << endl
           << "<TargetDescription>" 
           << _header->targetDescription()
           << "</TargetDescription>" << endl
           << "<Threshold>" << _header->threshold() << "</Threshold>" << endl;

    if (!_analyzer->chartFileNames().empty())
    {
        output << "<Charts>" << endl;
        
        vector<string>::const_iterator chart = _analyzer->chartFileNames().begin();
        vector<string>::const_iterator end = _analyzer->chartFileNames().end();
        for(; chart != end; ++chart)
        {
            output << "<Chart>" << *chart << "</Chart>" << endl;
        }

        output << "</Charts>" << endl;
    }

    output << "</PerformanceDataHeader>" << endl;
}

void CallTreeXMLGenerator::generateFunctions(ostream& output)
{
    if (!_trees)
        return;

    vector<CallTreeMethodTree*>::const_iterator tree = _trees->begin();
    vector<CallTreeMethodTree*>::const_iterator end = _trees->end();
    for (; tree != end; ++tree)
    {
        output << "<PerformanceDataFunctions>" << endl;
        generateFunction(*tree, output);
        output  << "</PerformanceDataFunctions>" << endl;
    }
}

void CallTreeXMLGenerator::generateHotspots(ostream& output)
{
    const vector<const CallTreeMethodTree*>& hotspots = _analyzer->hotspots();
    vector<const CallTreeMethodTree*>::const_iterator hs = hotspots.begin();
    vector<const CallTreeMethodTree*>::const_iterator end = hotspots.end();

    for (; hs != end; ++hs)
    {
        generateFunction(*hs, output);
    }
}

void CallTreeXMLGenerator::generateFunction( const CallTreeMethodTree* tree,
                                             ostream& output)
{
    if (tree) {
        output << "<PerformanceDataFunction>" << endl;
        if (tree->method()) {
            generateFunctionInfo(tree->method(), output);
        }
        vector<CallTreeMethodTree*>::const_iterator child 
            = tree->children().begin();
        vector<CallTreeMethodTree*>::const_iterator end 
            = tree->children().end();
        output << "<FunctionCallTree>" << endl;
        for(; child != end; ++child) {
            const CallTreeMethod* method = (*child)->method();
            if (method) {
                output << "<FunctionChild>" << endl;
                generateFunctionInfo(method, output);
                output << "</FunctionChild>" << endl;
            }
        }
        output << "</FunctionCallTree>" << endl;
        output << "</PerformanceDataFunction>" << endl;
    }
}

void CallTreeXMLGenerator::generateFunctionInfo( const CallTreeMethod* method,
                                                 ostream& output )
{
    output << "<Namespace>"
           << translateXMLEntities(method->classNamespace())
           << "</Namespace>" << endl
           << "<ClassName>"
           << translateXMLEntities(method->className())
           << "</ClassName>" << endl
           << "<FunctionName>"
           << translateXMLEntities(method->classMethodName())
           << "</FunctionName>" << endl
           << "<FunctionSignature>"
           << translateXMLEntities(method->classMethodParams())
           << "</FunctionSignature>" << endl
           << "<NumberTimesCalled>"
           << method->numericNumberTimesCalled()
           << "</NumberTimesCalled>" << endl
           << "<TotalCycles>"
           << method->numericCost()
           << "</TotalCycles>" << endl
           << "<MiscInfo>" 
           << translateXMLEntities(method->miscInfo())
           << "</MiscInfo>" << endl
        ;
}

string CallTreeXMLGenerator::translateXMLEntities(std::string raw)
{
    static boost::regex rx("(&)|(<)|(>)|(\")|(')");
    static string format("(?1&amp;)(?2&lt;)(?3&gt;)(?4&quot;)(?5&apos;)");
    string input(raw);

    return boost::regex_replace(input, rx, format, boost::format_all);
}
