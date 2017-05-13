//-------------------------------------------------------------------
//
//  File:        PercentageComputator.h
//  Created:     Jul,1 2014
//  Authors:     Michal Mlynek
//
//  Description: This class computates a split of one value based on the proportions the other split.
//  When declaring an object of this class we provide a Key and Value types and
//  a parameter with a value that will mean 100%. By default it is 100.
//
//  An example:
//  We want to split a fare based on the mileage of the legs this fare covers.
//  Let's say a fare is 1500$ and it covers three legs with mileages 200,500,300:
//
//  Both a key ( legId ) and an input value ( mileage ) are ints
//
//  PercentageComputator<int, int> computator(1500);   // total fare to be split
//  computator.addToKey(1, 200);         // mileages for each leg
//  computator.addToKey(2, 500);
//  computator.addToKey(3, 300);
//
//  computator.getOutput()
//  will return a map with following values:
//
//  1 ->  300
//  2 ->  750
//  3 ->  450
//
//  Meaning that a fare should be split among those legs in three parts of 300$, 750$, 450$
//
//  If it happenes that something gets missing in the computations
//  and the total sum doesn't match the provided 100% value ( in this case 1500 )
//  then the missing part will be added to the first item in the output map automatically
//  For example 15 means 100%. We provide Keys 1->5 2->5 which means that 15 needs to be split equally
//  Since both input and output values are ints the initial result will be 7 and 7
//  This class will automatically detect this situation and add the missing 1 to the first element
//  So the result will be 1->8 2->7
//
//  If we provide only zeroes as inputs - for example 1->0 2->0 3->0 etc the split will be done equally
//  among all the Keys
//
//  Copyright Sabre 2005
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
#include "Common/Assert.h"

#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>

namespace tse
{

template <typename Key, typename Value>
class PercentageComputator
{
public:
  using PercentageMap = std::map<Key, Value>;

  PercentageComputator(Value totalOutput = 100)
    :_totalInput(0), _totalOutput(totalOutput){};

  void addToKey(Key key, Value value = 0)
  {
    // We cannot have negative numbers in percentage computations
    TSE_ASSERT(value >= 0);

    _needsRecomputation = true;

    try
    {
      _inputMap.at(key) += value;
    }
    catch(std::out_of_range&)
    {
      _inputMap[key] = value;
    }
    _totalInput += value;
  }

  const PercentageMap& getOutput()
  {
    if(_needsRecomputation)
      recompute();

    return _outputMap;
  }

  Value getOutputForKey(Key key)
  {
    if(_needsRecomputation)
      recompute();

    try
    {
      return _outputMap.at(key);
    }
    catch(std::out_of_range&)
    {
      // If we cannot find a Key we assume that percentage for this Key is 0
      return 0;
    }
  }

private:

  void recompute()
  {
    if (_inputMap.empty())
      return;

    _outputMap.clear();

    Value sumOfComputedParts = 0;
    for (typename PercentageMap::const_iterator input = _inputMap.begin(); input != _inputMap.end(); ++input)
    {
      // If map is not empty but contains only zeroes we split the output equally among all Keys
      if (_totalInput == 0)
      {
        _outputMap[input->first] = _totalOutput / _inputMap.size();
      }
      else
        _outputMap[input->first] = (_totalOutput * ((input->second)*100)/_totalInput)/100;

      sumOfComputedParts += _outputMap[input->first];
    }

    // If something gets missing in rounding-off we add it to the first element
    // so the sum always matches the desired whole
    _outputMap.begin()->second += (_totalOutput - sumOfComputedParts);
    _needsRecomputation = false;
  }

  PercentageMap _inputMap;
  PercentageMap _outputMap;
  Value _totalInput;
  Value _totalOutput;
  bool _needsRecomputation = false;
};
}  // end namespace tse
