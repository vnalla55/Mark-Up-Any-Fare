#include "Common/Logger.h"
#include "test/DataHandleGenerator/scanner.cc"

#include <boost/assign/std/vector.hpp>
#include <log4cxx/fileappender.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/logstring.h>
#include <log4cxx/simplelayout.h>
#include <stdlib.h>

#include <memory>

using namespace boost::assign;

DHHeaderParser* g_parser = 0;

log4cxx::LoggerPtr
DHHeaderParser::_logger(log4cxx::Logger::getLogger("DataHandleParser.DataHandleParser"));

DHHeaderParser::DHHeaderParser(GlobalNamespace& gn)
  : _globalNamespace(gn),
    _yyLineNr(1),
    _brackCount(0),
    _classType(Class::CLASS_NONE),
    _operator(false),
    _conversion(false)
{
  _cppElem.reset();
  // yylex debug 0 - disabled, 1 - enabled
  yy_flex_debug = 0;
}
DHHeaderParser::~DHHeaderParser() {}
// Create Class
void
DHHeaderParser::createClass(Class::ClassType classType)
{
  // just set class type, we will create class later if needed
  _classType = classType;
}
ElemPtr&
DHHeaderParser::checkPostClass()
{
  // if we should create class
  if (_classType != Class::CLASS_NONE)
  {
    ClassPtr cl = ClassPtr(new Class(_classType));
    LOG_ASSERT(bool(cl), "memory problem in checkPostClass");
    cl->lineNo() = _yyLineNr;
    if (_classType == Class::CLASS_CLASS)
      cl->protection() = Protection::Private;
    else
      cl->protection() = Protection::Public;
    // get class name
    cl->name() = _stringVec.createString();
    _stringVec.clear();
    LOG_INFO("created class", cl->name().c_str());

    // look if there is some template
    ElemList::iterator it = _preElemList.begin();
    ElemList::iterator ie = _preElemList.end();
    for (; it != ie; it++)
    {
      TemplatePtr t = std::dynamic_pointer_cast<Template>(*it);
      if (t)
        cl->tmplte() = t;
    }
    _preElemList.clear();
    // update stack and set cppElem
    if (_cppElem)
      _elemStack.push(_cppElem);
    _cppElem = cl;
    _classType = Class::CLASS_NONE;
  }
  return _cppElem;
}
ElemPtr&
DHHeaderParser::createFunction()
{
  bool fwrdDeclInFun = _classType == Class::CLASS_CLASS;
  // we create function when find "(", if class is in string, then ignore
  _classType = Class::CLASS_NONE;
  FunctionPtr fun = FunctionPtr(new Function);
  LOG_ASSERT(bool(fun), "memory problem in createFunction");
  // if method inside class
  ElemPtr elem = _cppElem;
  // can be created in global anmespace or class
  if (!_cppElem && _elemStack.size())
    elem = _elemStack.top();
  ClassPtr cl = std::dynamic_pointer_cast<Class>(elem);
  // if class then set saved protection
  if (cl)
    fun->protection() = cl->protection();
  // put element on stack
  if (_cppElem)
    _elemStack.push(_cppElem);
  _cppElem = fun;
  fun->lineNo() = _yyLineNr;
  // generate function name
  createFunName();
  // if class name same as function name then constructor is found
  if (cl)
  {
    std::string destName = "~" + cl->name();
    if (fun->name() == cl->name())
    {
      LOG_DEBUG("found constructor");
      fun->isConstructor() = true;
    }
    else if (fun->name() == destName)
    {
      LOG_DEBUG("found destructor");
      fun->isDestructor() = true;
    }
  }
  // check if there is some template arguments for this function
  ElemList::iterator it = _preElemList.begin();
  ElemList::iterator ie = _preElemList.end();
  for (; it != ie; it++)
  {
    TemplatePtr t = std::dynamic_pointer_cast<Template>(*it);
    if (t)
      fun->tmplte() = t;
  }
  _preElemList.clear();
  _brackCount = 1;
  BEGIN(FunParameters);
  fun->isOperator() = _operator;
  _operator = false;
  _conversion = false;
  // if class in function declaration, then put it into namesapce fwd decls
  if (fwrdDeclInFun)
  {
    ClassPtr clDec = ClassPtr(new Class(Class::CLASS_CLASS));
    clDec->name() = fun->returnType()->type();
    // find namespace in which forward declarations will be placed
    std::deque<std::shared_ptr<CPPElem>>::reverse_iterator rit = _elemStack.container().rbegin();
    std::deque<std::shared_ptr<CPPElem>>::reverse_iterator rie = _elemStack.container().rend();
    for (; rit != rie; rit++)
    {
      NamespacePtr nm = std::dynamic_pointer_cast<Namespace>(*rit);
      if (nm)
      {
        nm->fwrdDecls().push_back(clDec);
        break;
      }
    }
    // if no namespace found, use global namespace
    if (rit == rie)
      _globalNamespace.fwrdDecls().push_back(clDec);
  }
  return _cppElem;
}
void
DHHeaderParser::createParameterInit()
{
  if (_stringVec.empty())
    return;
  // get param init string
  ParameterPtr p = createParameter();
  NamespacePtr nm = std::dynamic_pointer_cast<Namespace>(_elemStack.top());
  if (nm != 0)
  {
    LOG_DEBUG("adding parameter ", p->name().c_str(), " to namespace ", nm->name().c_str());
    nm->paramInit().push_back(p);
  }
  else
  {
    LOG_DEBUG("adding parameter ", p->name().c_str(), " to global namespace");
    _globalNamespace.paramInit().push_back(p);
  }
  _stringVec.clear();
}
ElemPtr&
DHHeaderParser::createDefine()
{
  _cppElem = DefinePtr(new Define);
  LOG_ASSERT(bool(_cppElem), "memory problem in createDefine");
  _cppElem->lineNo() = _yyLineNr;
  saveLastContext();
  BEGIN(DefineStart);
  return _cppElem;
}
ElemPtr&
DHHeaderParser::createInclude()
{
  _cppElem = IncludePtr(new Include);
  LOG_ASSERT(bool(_cppElem), "memory problem in createInclude");
  _cppElem->lineNo() = _yyLineNr;
  // include from H or CPP
  saveLastContext();
  BEGIN(IncludeStart);
  return _cppElem;
}
ElemPtr&
DHHeaderParser::createNamespace(bool isUsing)
{
  lineCount();
  NamespacePtr nm = NamespacePtr(new Namespace(isUsing));
  LOG_ASSERT(bool(nm), "memory problem in createNamespace");
  // something above namespace??
  if (_cppElem)
    _elemStack.push(_cppElem);
  _cppElem = nm;
  _cppElem->lineNo() = _yyLineNr;
  saveLastContext();
  // put created namespace in correct place
  if (isUsing)
  {
    _globalNamespace.usedNamespaces().push_back(nm);
    BEGIN(UsingNamespaceStart);
  }
  else
    _globalNamespace.namespaces().push_back(nm);
  return _cppElem;
}
ElemPtr&
DHHeaderParser::createTemplate()
{
  lineCount();
  _cppElem = TemplatePtr(new Template);
  LOG_ASSERT(bool(_cppElem), "memory problem in createTemplate");
  _cppElem->lineNo() = _yyLineNr;
  _brackCount = 1;
  BEGIN(TemplateSection);
  return _cppElem;
}
ElemPtr&
DHHeaderParser::createTypedef()
{
  _cppElem = TypedefPtr(new Typedef);
  _cppElem->lineNo() = _yyLineNr;
  saveLastContext();
  BEGIN(TypedefsDef);
  return _cppElem;
}
void
DHHeaderParser::setProtection(Protection::Prot protection)
{
  // protection can be in class (stack top) or baseclass
  ElemPtr elem = _cppElem;
  if (!elem && _elemStack.size())
    elem = _elemStack.top();

  ClassPtr cl = std::dynamic_pointer_cast<Class>(elem);
  BaseClassPtr bc = std::dynamic_pointer_cast<BaseClass>(elem);
  if (cl)
    cl->protection() = protection;
  else if (bc)
    bc->protecton() = protection;
  else
    LOG_ERROR("Protection not handled");
};
void
DHHeaderParser::setCppKey(CPPKeywords::CPPKEYW key)
{
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(_cppElem);
  FunctionPtr f = std::dynamic_pointer_cast<Function>(_cppElem);
  if (p)
    p->cppKey().setFlag(key);
  else if (f)
    f->cppKey().setFlag(key);
  else
    _cppKey.setFlag(key);
}
// if no bufers to parse, then finish with parsing, otherwise, process next data from stack
int
DHHeaderParser::yywrap()
{
  if (_bufDataStack.empty())
    return 0;
  BufStackData buf = _bufDataStack.top();
  _bufDataStack.pop();
  BEGIN(buf.context());
  yy_delete_buffer(YY_CURRENT_BUFFER);
  yy_switch_to_buffer(buf.buffer());
  _yyLineNr = buf.yyLineNr();
  _brackCount = buf.brackCount();
  return 1;
}

// put data on the stack, and start new parser
void
DHHeaderParser::parseString(int context, const char* str)
{
  BufStackData dat(YY_START, YY_CURRENT_BUFFER, _yyLineNr, _brackCount);
  _bufDataStack.push(dat);
  YY_BUFFER_STATE buf = yy_scan_string(str);
  BEGIN(context);
  yy_switch_to_buffer(buf);
}

// Parse function name and return type
void
DHHeaderParser::createFunName()
{
  std::string fun = _stringVec.createString();
  _stringVec.clear();
  FunctionPtr fn = std::dynamic_pointer_cast<Function>(_cppElem);
  LOG_ASSERT(bool(fn), "Incorrect pointer when trying to create function name");
  _elemStack.push(_cppElem);
  ParameterPtr p(new Parameter);
  _cppElem = p;
  fn->returnType() = p;
  splitToFunKeywords(p, fn);
  parseString(FunctionNameParser, fun.c_str());
  yylex();
  adjustParameterType(p);
  _cppElem = _elemStack.tpop();
  LOG_INFO("created function ", fn->name().c_str());
}
void
DHHeaderParser::splitToFunKeywords(ParameterPtr& p, FunctionPtr& fn)
{
  std::vector<CPPKeywords::CPPKEYW> funKeyw;
  funKeyw += CPPKeywords::VOLATILE, CPPKeywords::VIRTUAL, CPPKeywords::THROW, CPPKeywords::INLINE,
      CPPKeywords::EXPLICIT, CPPKeywords::STATIC;
  std::vector<CPPKeywords::CPPKEYW>::iterator it = funKeyw.begin();
  std::vector<CPPKeywords::CPPKEYW>::iterator ie = funKeyw.end();
  for (; it != ie; it++)
  {
    if (_cppKey.isFlag(*it))
    {
      fn->cppKey().setFlag(*it);
      _cppKey.unsetFlag(*it);
    }
  }
  p->cppKey() = _cppKey;
  _cppKey.reset();
}

// line count
void
DHHeaderParser::lineCount()
{
  for (const char* c = yytext; *c; ++c)
    _yyLineNr += (*c == '\n');
}

// Create Parameter from vector string
ParameterPtr
DHHeaderParser::createParameter()
{
  _elemStack.push(_cppElem);
  ParameterPtr p = ParameterPtr(new Parameter);
  _cppElem = p;
  p->cppKey() = _cppKey;
  _cppKey.reset();
  LOG_ASSERT(bool(_cppElem), "memory problem in createParameter");
  std::string param = _stringVec.createString();
  LOG_DEBUG(param.c_str());
  parseString(ParameterParser, param.c_str());
  yylex();
  _stringVec.clear();
  adjustParameterType(p);
  LOG_DEBUG("Created parameter: ", _cppElem->name().c_str());
  _cppElem = _elemStack.tpop();
  return p;
}
void
DHHeaderParser::adjustParameterType(ParameterPtr& p)
{
  if (p->type().size())
  {
    char c = p->type()[p->type().size() - 1];
    if (c == '*' || c == '&')
    {
      p->ptrRef() = c;
      size_t s = (p->type().size() > 1 && p->type()[p->type().size() - 2] == ' ')
                     ? p->type().size() - 2
                     : p->type().size() - 1;
      p->type() = p->type().substr(0, s);
    }
  }
}
void
DHHeaderParser::checkFunctionEnd(bool pop)
{
  if (pop && _elemStack.size())
    _cppElem = _elemStack.tpop();
  if (_cppElem)
  {
    ElemPtr elem;
    if (_elemStack.size())
      elem = _elemStack.top();
    FunctionPtr fl = std::dynamic_pointer_cast<Function>(_cppElem);
    NamespacePtr enm = std::dynamic_pointer_cast<Namespace>(elem);
    ClassPtr ecl = std::dynamic_pointer_cast<Class>(elem);

    ParameterPtr param;
    // if something in stringVec, then we create parameter
    if (g_parser->stringVec().size() > 1)
      param = g_parser->createParameter();
    if (fl) // if function
    {
      // function can be global or part of class
      if (ecl)
      {
        LOG_DEBUG("found method ", fl->name().c_str(), " in class ", ecl->name().c_str());
        ecl->functions().push_back(fl);
      }
      else if (enm)
      {
        LOG_DEBUG("found method ", fl->name().c_str(), " in namespace ", enm->name().c_str());
        enm->functions().push_back(fl);
      }
      else
      {
        LOG_DEBUG("found method ", fl->name().c_str(), " in global namespace ");
        g_parser->globalNamespace().functions().push_back(fl);
      }
    }
    // check if need to pop elem from stack
    g_parser->cppElem().reset();
  }
  g_parser->restoreLastContext();
}
std::string
DHHeaderParser::msg(const char* str, const char* str2, const char* str3, const char* str4)
{
  std::stringstream msg;
  msg << int(_yyLineNr) << ": " << str;
  if (str2)
    msg << str2;
  if (str3)
    msg << str3;
  if (str4)
    msg << str4;
  return msg.str();
};
// Parse header text function
bool
DHHeaderParser::parse(const char* inputTxt, int context)
{
  try
  {
    g_parser = this;
    _yyLineNr = 1;
    _stringVec.clear();
    parseString(context, inputTxt);
    yylex();
  }
  catch (const std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Exception caught" << e.what());
    return false;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "Unknown exception");
    return false;
  }
  return true;
}
void
DHHeaderParser::saveLastContext()
{
  _lastContext.push(YY_START);
}
int
DHHeaderParser::restoreLastContext()
{
  int ret = 0;
  if (_lastContext.size())
  {
    ret = _lastContext.top();
    _lastContext.pop();
  }
  BEGIN(ret);
  return ret;
}
bool
DHHeaderParser::parseCpp(const char* inputTxt)
{
  return parse(inputTxt, FindMembersCpp);
}
bool
DHHeaderParser::parseH(const char* inputTxt)
{
  return parse(inputTxt, FindMembersH);
}
bool
DHHeaderParser::parseTseTypes(const char* inputTxt)
{
  return parse(inputTxt, Typedefs);
}
