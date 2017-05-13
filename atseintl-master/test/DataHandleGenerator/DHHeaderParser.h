#ifndef SCANNER_H
#define SCANNER_H

#include "Common/Logger.h"
#include "test/DataHandleGenerator/Types.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

struct yy_buffer_state;

class DHHeaderParser
{
  GlobalNamespace& _globalNamespace;
  int _yyLineNr;
  CPPKeywords _cppKey;
  std::stack<int> _lastContext;
  int _brackCount;

  ElemPtr _cppElem;
  ElemStack _elemStack;
  ElemList _preElemList;

  StringVec _stringVec;
  Class::ClassType _classType;
  bool _operator;
  bool _conversion;

  static log4cxx::LoggerPtr _logger;

  class BufStackData
  {
    int _context;
    yy_buffer_state* _buffer;
    int _yyLineNr;
    int _brackCount;

  public:
    BufStackData(const BufStackData& r)
      : _context(r._context), _buffer(r._buffer), _yyLineNr(r._yyLineNr), _brackCount(r._brackCount)
    {
    }
    BufStackData(int ct = 0, yy_buffer_state* buf = 0, int yyLineNr = 0, int brackCount = 0)
      : _context(ct), _buffer(buf), _yyLineNr(yyLineNr), _brackCount(brackCount)
    {
    }
    int context() const { return _context; }
    yy_buffer_state* buffer() const { return _buffer; }
    int yyLineNr() const { return _yyLineNr; }
    int brackCount() const { return _brackCount; }
  };

  std::stack<BufStackData> _bufDataStack;

  bool parse(const char* inputTxt, int context);

public:
  operator log4cxx::LoggerPtr&() { return _logger; }
  operator const log4cxx::LoggerPtr&() const { return _logger; }

  DHHeaderParser(GlobalNamespace& gn);
  virtual ~DHHeaderParser();

  int& yyLineNr() { return _yyLineNr; }
  int yyLineNr() const { return _yyLineNr; }

  CPPKeywords& cppKey() { return _cppKey; }
  const CPPKeywords& cppKey() const { return _cppKey; }

  int& brackCount() { return _brackCount; }
  int brackCount() const { return _brackCount; }

  ElemPtr& cppElem() { return _cppElem; }
  const ElemPtr& cppElem() const { return _cppElem; }

  GlobalNamespace& globalNamespace() { return _globalNamespace; }
  const GlobalNamespace& globalNamespace() const { return _globalNamespace; }

  ElemStack& elemStack() { return _elemStack; }
  const ElemStack& elemStack() const { return _elemStack; }

  ElemList& preElemList() { return _preElemList; }
  const ElemList& preElemList() const { return _preElemList; }

  StringVec& stringVec() { return _stringVec; }
  const StringVec& stringVec() const { return _stringVec; }

  bool& oper() { return _operator; }
  const bool& oper() const { return _operator; }

  bool& conversion() { return _conversion; }
  const bool& conversion() const { return _conversion; }

  void saveLastContext();
  int restoreLastContext();

  bool parseCpp(const char* inputTxt);
  bool parseH(const char* inputTxt);
  bool parseTseTypes(const char* inputTxt);

  std::string
  msg(const char* str, const char* str2 = NULL, const char* str3 = NULL, const char* str4 = NULL);

  void lineCount();

  void createClass(Class::ClassType classType);
  ElemPtr& createDefine();
  ElemPtr& createInclude();
  ElemPtr& createNamespace(bool isUsing);
  ElemPtr& createTemplate();
  ElemPtr& createFunction();
  ElemPtr& createTypedef();
  void createParameterInit();

  void checkFunctionEnd(bool pop);
  ElemPtr& checkPostClass();

  ParameterPtr createParameter();
  void createFunName();
  void adjustParameterType(ParameterPtr& p);
  void splitToFunKeywords(ParameterPtr& p, FunctionPtr& fn);

  void setProtection(Protection::Prot protection);
  void setCppKey(CPPKeywords::CPPKEYW key);

  int yywrap();
  void parseString(int context, const char* str);
};

extern DHHeaderParser* g_parser;

#endif
