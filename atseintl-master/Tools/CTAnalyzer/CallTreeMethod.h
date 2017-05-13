#ifndef CALL_TREE_METHOD_H
#define CALL_TREE_METHOD_H


namespace tse
{

class CTTokenVector;

class CallTreeMethod
{
 public:
  typedef enum CallTreeMethodType
  {
    CallTreeMethodParent   = 0,
    CallTreeMethodChild    = 1,
    CallTreeMethodNumTypes
  };

  typedef enum MethodSourceType
  {
    MethodSourceC         = 0, // .c
    MethodSourceCPlusPlus = 1, // .cpp
    MethodSourceBigC      = 2, // .C
    MethodSourceBigH      = 3, // .H
    MethodSourceHeader    = 4, // .h
    MethodSourceHPlusPlus = 5, // .hpp
    MethodSourceCxx       = 6, // .cxx
    MethodSourceCC        = 7, // .cc
    MethodSourceUnknown   = 8, // ???
    MethodSourceNumTypes
  };

 private:
  std::string _cost;
  uint64_t _numericCost;
  CallTreeMethodType _type;
  MethodSourceType _sourceType;
  std::string _sourceFile;
  std::string _classNamespace;
  std::string _className;
  std::string _classMethodName;
  std::string _classMethodParams;
  std::string _numberTimesCalled;
  uint64_t _numericNumberTimesCalled;
  std::string _miscInfo;
  std::vector< uint32_t > _singleColonIndices;
  std::vector< std::pair<uint32_t, uint32_t> > _doubleColonIndices;
  std::vector< std::pair<uint32_t, uint32_t> > _parenPairIndices;
  std::vector< std::pair<uint32_t, uint32_t> > _bracketPairIndices;
  std::string _matchKey;  
  bool _valid;
  

 private:
  uint64_t convertLargeNumberString(std::string& number, char extraneousChar);
  void determineMethodSourceType(CTTokenVector*& info);
  void processStartMethodLine(CTTokenVector*& info);
  void processContinueMethodLine(CTTokenVector*& info);
  void analyzeMethodLineIndices(CTTokenVector*& info);

  //Method breakdown process functions
  void processFunctionName(CTTokenVector*& info);
  void processNamespace(CTTokenVector*& info);
  void processNumberTimesCalled(CTTokenVector*& info);
  void processClassName(CTTokenVector*& info);
  void processFunctionSignature(CTTokenVector*& info);
  void invalidate();
  void generateMatchKey();
  
 public:
  explicit CallTreeMethod() :
    _cost("0"),
    _numericCost(0),
    _type(CallTreeMethodType(0)),
    _sourceType(MethodSourceType(0)),
    _sourceFile("None"),
    _classNamespace("None"),
    _className("None"),
    _classMethodName("None"),
    _classMethodParams("None"),
    _numberTimesCalled("0"),
    _numericNumberTimesCalled(0),
    _miscInfo("None"),
    _singleColonIndices(),
    _doubleColonIndices(),
    _parenPairIndices(),
    _bracketPairIndices(),
    _matchKey("None"),
    _valid(false)
  {
  }

  explicit CallTreeMethod(std::string& cost,
                          CallTreeMethodType& type,
                          CTTokenVector*& infoLine) :
    _cost(cost),
    _numericCost(0),
    _type(type),
    _sourceType(MethodSourceType(0)),
    _sourceFile("None"),
    _classNamespace("None"),
    _className("None"),
    _classMethodName("None"),
    _classMethodParams("None"),
    _numberTimesCalled("0"),
    _numericNumberTimesCalled(0),
    _miscInfo("None"),
    _singleColonIndices(),
    _doubleColonIndices(),
    _parenPairIndices(),
    _bracketPairIndices(),
    _matchKey("None"),
    _valid(false)
  {
    processInfoLine(infoLine);
  }

  ~CallTreeMethod()
  {
  }

  void processInfoLine(CTTokenVector*& infoLine);
  
  //Accessors
  std::string& cost() { return(_cost);};
  const std::string& cost() const { return(_cost);};

  uint64_t numericCost() const { return(_numericCost);};

  CallTreeMethodType& type() { return(_type);};
  const CallTreeMethodType& type() const { return(_type);};

  MethodSourceType& sourceType() { return(_sourceType);};
  const MethodSourceType& sourceType() const { return(_sourceType);};
  
  std::string& sourceFile() { return(_sourceFile);};
  const std::string& sourceFile() const { return(_sourceFile);};

  std::string& classNamespace() { return(_classNamespace);};
  const std::string& classNamespace() const { return(_classNamespace);};

  std::string& className() { return(_className);};
  const std::string& className() const { return(_className);};

  std::string& classMethodName() { return(_classMethodName);};
  const std::string& classMethodName() const { return(_classMethodName);};

  std::string& classMethodParams() { return(_classMethodParams);};
  const std::string& classMethodParams() const { return(_classMethodParams);};

  std::string& numberTimesCalled() { return(_numberTimesCalled);};
  const std::string& numberTimesCalled() const { return(_numberTimesCalled);};

  uint64_t numericNumberTimesCalled() const { return(_numericNumberTimesCalled);};

  std::string& miscInfo() { return(_miscInfo);};
  const std::string& miscInfo() const { return(_miscInfo);};

  bool& valid() { return(_valid);};
  const bool& valid() const { return(_valid);};

  const std::string& matchKey() const {return(_matchKey);};
};

}// End namespace tse


#endif //CALL_TREE_METHOD_H
