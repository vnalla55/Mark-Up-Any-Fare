#include "CallTreeChartData.h"
#include "CallTreeMethodTree.h"

using namespace tse;
using namespace std;

CallTreeChartData::CallTreeChartData( unsigned numFunctions,
                                      std::vector<CallTreeMethodTree*>* trees )
{
    _num = std::min(int(numFunctions), int(trees->size()));
    _values = new float[_num];
    _labels = new char*[_num];

    for (unsigned i = 0; i < _num; ++i)
    {
        const CallTreeMethod* method = (*trees)[i]->method();
        string label = method->className() + "::" + method->classMethodName();
        _labels[i] = new char[label.size() + 1];
        strcpy(_labels[i], label.c_str());

        _values[i] = (*trees)[i]->method()->numericCost();
    }
}

CallTreeChartData::~CallTreeChartData()
{
    for (unsigned i = 0; i < _num; ++i)
        delete _labels[i];

    delete[] _labels;
    delete[] _values;
}

char** CallTreeChartData::labels()
{
    return _labels;
}

float* CallTreeChartData::values()
{
    return _values;
}


unsigned long* CallTreeChartData::colors() const
{
  //  static unsigned long clrs[] = 
  //        { 0xFF0000, 0x00FF00, 0x0000FF, 0xFF00FF, 0xFFFF00, 0x00FFFF, 
  //          0x7F0000, 0x007F00, 0xFF7F00, 0xFF00AF, 0xAFFF00, 0x00FFAF,
  //          0xFF9F9F, 0x9FFF9F, 0x9F9FFF, 0xFF9FFF, 0xFFFF9F, 0x9FFFFF };
  //Grayscale rgb triplets
  static unsigned long clrs[] =
    { 0xFFFFFF, 0xEEEEEE, 0xDDDDDD, 0xCCCCCC, 0xBBBBBB, 0xAAAAAA, 0x999999, 
      0x888888, 0x777777, 0x666666, 0x656565, 0x555555, 0x444444, 0x333333, 
      0x222222, 0x111111, 0x101010, 0x010101, 0x000000};

    return clrs;
}
