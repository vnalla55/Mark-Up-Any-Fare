#ifndef CALL_TREE_HEADER_H
#define CALL_TREE_HEADER_H

namespace tse
{

class CallTreeHeader
{
 private:
  std::string _cacheI1;
  std::string _cacheD1;
  std::string _cacheL2;
  std::string _timeRangeDescription;
  std::string _triggerType;
  std::string _targetDescription;
  std::string _threshold;
  std::string _totalCycleCount;

 public:
  CallTreeHeader() :
    _cacheI1("None"),
    _cacheD1("None"),
    _cacheL2("None"),
    _timeRangeDescription("None"),
    _triggerType("None"),
    _targetDescription("None"),
    _threshold("99"),
    _totalCycleCount("0")    
  {
  }

  ~CallTreeHeader()
  {
  }

  std::string& cacheI1() { return(_cacheI1);};
  const std::string& cacheI1() const { return(_cacheI1);};

  std::string& cacheD1() { return(_cacheD1);};
  const std::string& cacheD1() const { return(_cacheD1);};

  std::string& cacheL2() { return(_cacheL2);};
  const std::string& cacheL2() const {return(_cacheL2);};

  std::string& timeRangeDescription() { return(_timeRangeDescription);};
  const std::string& timeRangeDescription() const { return(_timeRangeDescription);};

  std::string& triggerType() { return(_triggerType);};
  const std::string& triggerType() const { return(_triggerType);};

  std::string& targetDescription() { return(_targetDescription);};
  const std::string& targetDescription() const { return(_targetDescription);};
  
  std::string& threshold() { return(_threshold);};
  const std::string& threshold() const { return(_threshold);};
  
  std::string& totalCycleCount() { return(_totalCycleCount);};
  const std::string& totalCycleCount() const { return(_totalCycleCount);};  
};
    
} //End namespace tse

#endif //CALL_TREE_HEADER_H
