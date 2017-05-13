#ifndef CALL_TREE_CHART_DATA
#define CALL_TREE_CHART_DATA

#include <vector>

namespace tse
{

class CallTreeMethodTree;

class CallTreeChartData
{
public:
    CallTreeChartData( unsigned numFunctions,
                       std::vector<CallTreeMethodTree*>* trees);

    ~CallTreeChartData();

    char** labels();

    float* values();

    unsigned long* colors() const;
    
private:
    const std::vector<CallTreeMethodTree*>*  _trees;
    char** _labels;
    float* _values;
    unsigned _num;
};

} // namespace tse

#endif // CALL_TREE_CHART_DATA
