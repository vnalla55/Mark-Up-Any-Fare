#include "test/DataHandleGenerator/Types.h"
#include "test/DataHandleGenerator/DHGenerator.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <set>
#include <map>

const std::string Parameter::__id = "Parameter";
const std::string Class::__id = "Class";
const std::string BaseClass::__id = "BaseClass";
const std::string Template::__id = "Template";
const std::string Define::__id = "Define";
const std::string Include::__id = "Include";
const std::string Function::__id = "Function";
const std::string Namespace::__id = "Namesapce";
const std::string Typedef::__id = "Typedef";
const std::string GlobalNamespace::__id = "GlobalNamespace";

class falign
{
  int _align;

public:
  falign(int align) : _align(align) {}
  friend std::ostream& operator<<(std::ostream& file, const falign& ali);
};
std::ostream& operator<<(std::ostream& file, const falign& ali)
{
  if (ali._align)
    file << std::setw(ali._align) << ' ';
  return file;
}

PtrList<Typedef> GlobalNamespace::_typedefs;

/***************************************************************
****************************************************************
**                      Protection                            **
****************************************************************
***************************************************************/
Protection&
Protection::
operator=(const Protection& p)
{
  _protection = p._protection;
  return *this;
}
Protection&
Protection::
operator=(const Prot& p)
{
  _protection = p;
  return *this;
}
std::string
Protection::print()
{
  std::string ret;
  switch (_protection)
  {
  case Public:
    ret = "public";
    break;
  case Protected:
    ret = "protected";
    break;
  case Private:
    ret = "private";
    break;
  default:
    break;
  }
  return ret;
}
/***************************************************************
****************************************************************
**                      CPPKeywords                           **
****************************************************************
***************************************************************/
std::string
CPPKeywords::print()
{
  std::string ret;
  if (_keyw & VIRTUAL)
    ret += "virtual ";
  if (_keyw & AUTO)
    ret += "auto ";
  if (_keyw & CLASS)
    ret += "class ";
  if (_keyw & CONST)
    ret += "const ";
  if (_keyw & CONTINUE)
    ret += "continue ";
  if (_keyw & STATIC)
    ret += "static ";
  if (_keyw & EXPLICIT)
    ret += "explicit ";
  if (_keyw & EXTERN)
    ret += "extern ";
  if (_keyw & FRIEND)
    ret += "friend ";
  if (_keyw & INLINE)
    ret += "inline ";
  if (_keyw & MUTABLE)
    ret += "mutable ";
  if (_keyw & REGISTER)
    ret += "register ";
  if (_keyw & TEMPLATE)
    ret += "template ";
  if (_keyw & THROW)
    ret += "throw ";
  if (_keyw & TYPEDEF)
    ret += "typedef ";
  if (_keyw & TYPEID)
    ret += "typeid ";
  if (_keyw & TYPENAME)
    ret += "typename ";
  if (_keyw & UNION)
    ret += "union ";
  if (_keyw & USING)
    ret += "using ";
  if (_keyw & VOLATILE)
    ret += "volatile ";
  return ret;
}
std::string
CPPKeywords::printPre()
{
  std::string ret;
  if (_keyw & VIRTUAL)
    ret += "virtual ";
  if (_keyw & STATIC)
    ret += "static ";
  if (_keyw & EXPLICIT)
    ret += "explicit ";
  if (_keyw & EXTERN)
    ret += "extern ";
  if (_keyw & FRIEND)
    ret += "friend ";
  if (_keyw & INLINE)
    ret += "inline ";
  if (_keyw & MUTABLE)
    ret += "mutable ";
  if (_keyw & REGISTER)
    ret += "register ";
  if (_keyw & TYPEDEF)
    ret += "typedef ";
  if (_keyw & TYPEID)
    ret += "typeid ";
  if (_keyw & TYPENAME)
    ret += "typename ";
  if (_keyw & VOLATILE)
    ret += "volatile ";
  return ret;
}
std::string
CPPKeywords::printPost()
{
  std::string ret;
  if (_keyw & CONST)
    ret += "const ";
  if (_keyw & THROW)
    ret += "throw ";
  return ret;
}
CPPKeywords&
CPPKeywords::
operator=(const CPPKeywords& cpk)
{
  _keyw = cpk._keyw;
  return *this;
}
/***************************************************************
****************************************************************
**                      CPPElem                               **
****************************************************************
***************************************************************/
CPPElem::CPPElem() : _lineNo(-1) {};
CPPElem::~CPPElem() {};
void
CPPElem::printName(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align) << _name;
}
/***************************************************************
****************************************************************
**                      BaseClass                             **
****************************************************************
***************************************************************/
BaseClass::BaseClass() : _protection(Protection::Private) {}
void
BaseClass::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align) << _protection.print() << " " << _name;
}
void
BaseClass::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  // is base class in cpp used?
  printMockHeader(file, str, align);
}
void
BaseClass::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  // is base class in cpp used?
  printMockHeader(file, str, align);
}
BaseClass&
BaseClass::
operator=(const BaseClass& b)
{
  _name = b._name;
  _lineNo = b._lineNo;
  _protection = b._protection;
  return *this;
}
void
BaseClass::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "BaseClass")
  {
    if (_name == str1)
      _name = str2;
  }
}
/***************************************************************
****************************************************************
**                      Class                                 **
****************************************************************
***************************************************************/
Class::Class(ClassType classType) : _classType(classType), _defined(false) {}
void
Class::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  // print template if any exists
  if (_template)
    _template->printMockHeader(file, str, align);
  file << falign(align) << "class " << _name;
  if (!_defined)
    return;

  // print base classes
  if (_baseClasses.size())
  {
    size_t ali = align + _name.size() + 9;
    file << " : ";
    _baseClasses.printMockHeader(file, str, (int)ali);
  }
  file << std::endl << falign(align) << "{" << std::endl;
  _nestedClasses.printMockHeader(file, str, align + 2, ';', true, false);

  // add own constructor and destructor;
  Protection protection = Protection::Public;
  file << falign(align) << protection.print() << ":" << std::endl << falign(align + 2) << _name
       << "();" << std::endl;
  file << falign(align + 2) << "virtual ~" << _name << "();" << std::endl << std::endl;

  // add rest of the functions;
  PtrList<Function>::iterator it = _functions.begin(), ie = _functions.end();
  for (; it != ie; it++)
  {
    // skip implemented methods, constructor, destructor and operators
    if ((*it)->isImplemented() || (*it)->isConstructor() || (*it)->isDestructor() ||
        (*it)->isOperator())
      continue;

    if (protection != (*it)->protection())
    {
      protection = (*it)->protection();
      file << falign(align) << protection.print() << ":" << std::endl;
    }
    // static can't be virtual
    bool isVirt = (*it)->cppKey().isFlag(CPPKeywords::VIRTUAL);
    if (!(*it)->cppKey().isFlag(CPPKeywords::STATIC))
      (*it)->cppKey().setFlag(CPPKeywords::VIRTUAL);
    (*it)->printMockHeader(file, str, align + 2);
    file << ";" << std::endl << std::endl;
    if (!isVirt)
      (*it)->cppKey().unsetFlag(CPPKeywords::VIRTUAL);
  }
  // print class parameters
  Protection protectionProtected = Protection::Protected;
  Protection protectionPublic = Protection::Public;
  if (_parameters.size())
  {
    file << falign(align) << protectionProtected.print() << ":" << std::endl;
    PtrList<Parameter>::iterator ipt = _parameters.begin(), ipe = _parameters.end();
    for (; ipt != ipe; ipt++)
    {
      (*ipt)->printMockHeader(file, str, align + 2);
      file << ";" << std::endl;
    }
  }
  if (DHGenerator::createAccessors())
  {
    // print accessors functions
    DHGenerator::createDHAccessorMap(this);
    file << falign(align) << "//Accessors functions" << std::endl;
    file << falign(align) << protectionPublic.print() << ":" << std::endl;
    std::map<Function*, std::string>::iterator imt = DHGenerator::dataHandleAccessorMap().begin();
    std::map<Function*, std::string>::iterator ime = DHGenerator::dataHandleAccessorMap().end();
    for (; imt != ime; imt++)
    {
      file << falign(align + 2);
      if (imt->first->cppKey().isFlag(CPPKeywords::STATIC))
        file << "static ";
      file << "void set_" << imt->second << "(";
      imt->first->returnType()->printAccessorName(file, str, align);
      file << " param)" << std::endl << falign(align + 2) << "{" << std::endl;
      file << falign(align + 4) << "_" << imt->second << "Ptr = param;" << std::endl;
      file << falign(align + 2) << "}" << std::endl;
    }
    file << falign(align) << "//Accessors members" << std::endl;
    file << falign(align) << protectionProtected.print() << ":" << std::endl;
    imt = DHGenerator::dataHandleAccessorMap().begin();
    for (; imt != ime; imt++)
    {
      file << falign(align + 2);
      if (imt->first->cppKey().isFlag(CPPKeywords::STATIC))
        file << "static ";
      imt->first->returnType()->printAccessorName(file, str, align);
      file << " _" << imt->second << "Ptr;" << std::endl;
    }
  }
  file << std::endl << falign(align) << "}";
}
void
Class::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  if (!_defined)
    return;

  std::string newStr = str;
  newStr += _name;
  if (_template)
  {
    newStr += "<";
    PtrList<Parameter>::const_iterator it = _template->params().begin(),
                                       ie = _template->params().end();
    while (it != ie)
    {
      newStr += (*it)->name();
      it++;
      if (it != ie)
        newStr += ", ";
    }
    newStr += ">";
  }
  newStr += "::";
  _nestedClasses.printMockCpp(file, newStr, align, ';', true, false);

  file << falign(align) << str << _name << "::" << _name << "()";
  if (DHGenerator::createAccessors())
  {
    DHGenerator::createDHAccessorMap(this);
    // non static accesor members initialization in constructor
    size_t ali = align + 2 * _name.size() + str.size() + 7;

    std::map<Function*, std::string>::iterator imt = DHGenerator::dataHandleAccessorMap().begin();
    std::map<Function*, std::string>::iterator ime = DHGenerator::dataHandleAccessorMap().end();
    bool first = true;
    for (; imt != ime; imt++)
    {
      if (imt->first->cppKey().isFlag(CPPKeywords::STATIC))
        continue;
      if (first)
        file << " : _" << imt->second << "Ptr(0)";
      else
        file << "," << std::endl << falign((int)ali) << "_" << imt->second << "Ptr(0)";
      first = false;
    }
  }
  file << std::endl << falign(align) << "{" << std::endl;
  file << falign(align + 2) << "DataHandleMockFactory::setMock(this);" << std::endl
       << falign(align);
  file << "}" << std::endl << falign(align) << str << _name << "::~" << _name << "()" << std::endl;
  file << falign(align) << "{" << std::endl << falign(align + 2)
       << "DataHandleMockFactory::unsetMock();";
  file << std::endl << falign(align) << "}" << std::endl;

  // print rest of the functions
  PtrList<Function>::iterator it = _functions.begin(), ie = _functions.end();
  for (; it != ie; it++)
  {
    // skip implemented methods, constructor, destructor and operators
    if ((*it)->isImplemented() || (*it)->isConstructor() || (*it)->isDestructor() ||
        (*it)->isOperator())
      continue;

    if (_template)
      _template->printMockCpp(file, str, align);

    bool isStat = (*it)->cppKey().isFlag(CPPKeywords::STATIC);
    (*it)->cppKey().unsetFlag(CPPKeywords::STATIC);
    (*it)->printMockCpp(file, newStr, align);
    if (isStat)
      (*it)->cppKey().setFlag(CPPKeywords::STATIC);
  }
  if (DHGenerator::createAccessors() && DHGenerator::dataHandleAccessorMap().size())
  {
    // static accesor members initialization
    std::map<Function*, std::string>::iterator imt = DHGenerator::dataHandleAccessorMap().begin();
    std::map<Function*, std::string>::iterator ime = DHGenerator::dataHandleAccessorMap().end();
    for (; imt != ime; imt++)
    {
      if (!imt->first->cppKey().isFlag(CPPKeywords::STATIC))
        continue;
      file << falign(align);
      imt->first->returnType()->printAccessorName(file, str, align);
      file << _name << "::"
           << "_" << imt->second << "Ptr = 0;" << std::endl;
    }
  }
}
void
Class::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  if (!_defined)
    return;

  std::string newStr = str;
  newStr += _name;
  if (_template)
  {
    newStr += "<";
    PtrList<Parameter>::const_iterator it = _template->params().begin(),
                                       ie = _template->params().end();
    while (it != ie)
    {
      newStr += (*it)->name();
      it++;
      if (it != ie)
        newStr += ", ";
    }
    newStr += ">";
  }
  newStr += "::";
  _nestedClasses.printDHCpp(file, newStr, align, ';', true, false);

  PtrList<Function>::iterator it = _functions.begin(), ie = _functions.end();
  for (; it != ie; it++)
  {
    // skip implemented methods or operators
    if ((*it)->isImplemented() || (*it)->isOperator())
      continue;

    if (_template)
      _template->printDHCpp(file, str, align);

    CPPKeywords key = (*it)->cppKey();
    key.unsetFlag(CPPKeywords::STATIC);
    (*it)->printDHCpp(file, newStr, align);
  }
}
void
Class::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Class")
  {
    if (_name == str1)
      _name = str2;
  }
  if (_template)
    _template->rename(str1, str2, id);
  _functions.rename(str1, str2, id);
  _baseClasses.rename(str1, str2, id);
  _nestedClasses.rename(str1, str2, id);
  _parameters.rename(str1, str2, id);
}
void
Class::remove(const std::set<std::string>& ignoreSet, const std::string& id)
{
  _baseClasses.remove(ignoreSet, id);
  _functions.remove(ignoreSet, id);
  _nestedClasses.remove(ignoreSet, id);
  _parameters.remove(ignoreSet, id);
}
Class&
Class::
operator=(const Class& c)
{
  _name = c._name;
  _lineNo = c._lineNo;
  _classType = c._classType;
  _template = c._template;
  _functions = c._functions;
  _baseClasses = c._baseClasses;
  _nestedClasses = c._nestedClasses;
  _parameters = c._parameters;
  _defined = c._defined;
  _protection = c._protection;
  return *this;
}
/***************************************************************
****************************************************************
**                      Define                                **
****************************************************************
***************************************************************/
void
Define::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align) << "#define " << _name << _value << std::endl;
}
void
Define::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  // same define as in header
  printMockHeader(file, str, align);
}
void
Define::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  // same define as in header
  printMockHeader(file, str, align);
}
void
Define::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Define")
  {
    if (_name == str1)
      _name = str2;
  }
}
Define&
Define::
operator=(const Define& d)
{
  _name = d._name;
  _lineNo = d._lineNo;
  _value = d._value;
  return *this;
}
/***************************************************************
****************************************************************
**                      Function                              **
****************************************************************
***************************************************************/
Function::Function()
  : _isConstructor(false), _isDestructor(false), _isImplemented(false), _isOperator(false)
{
}
void
Function::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  // HC exception for ticketDate. setTicketDate is defined in DataHandle.h ,ticketDate will be
  // implemented in DataHandle.cpp
  if (_name == "ticketDate")
    return;
  std::string keyw = _cppKey.printPre();
  size_t ali = align + _name.size() + 2;
  if (_template)
    _template->printMockHeader(file, str, (int)align);
  file << falign(align) << keyw;
  _returnType->printMockHeader(file, str, 0);
  file << std::endl << falign(align);
  if (_isOperator)
  {
    file << "operator ";
    ali += 9;
  }
  file << _name << "( ";
  _params.printMockHeader(file, str, (int)ali);
  // in mock header make const function non const
  bool isConst = _cppKey.isFlag(CPPKeywords::CONST);
  _cppKey.unsetFlag(CPPKeywords::CONST);
  file << ")" << _cppKey.printPost();
  if (isConst)
    _cppKey.setFlag(CPPKeywords::CONST);
}
void
Function::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  // skip implemented function, (ticketDate is implemented in DataHandle.cpp)
  if (_isImplemented || _name == "ticketDate")
    return;
  // remove static in cpp
  bool isStatic = _cppKey.isFlag(CPPKeywords::STATIC);
  _cppKey.unsetFlag(CPPKeywords::STATIC);
  std::string keyw = _cppKey.printPre();
  if (isStatic)
    _cppKey.setFlag(CPPKeywords::STATIC);

  size_t ali = align + _name.size() + 2 + str.size();
  if (_template)
    _template->printMockCpp(file, str, align);
  file << falign(align) << keyw;
  _returnType->printMockCpp(file, str, 0);
  file << std::endl << falign(align);
  if (_isOperator)
  {
    file << str << "operator ";
    ali += 9 + str.size();
  }
  file << str << _name << "( ";
  _params.printMockCpp(file, str, (int)ali);
  // in mock make const function non const
  bool isConst = _cppKey.isFlag(CPPKeywords::CONST);
  _cppKey.unsetFlag(CPPKeywords::CONST);
  file << ")" << _cppKey.printPost() << std::endl;
  if (isConst)
    _cppKey.setFlag(CPPKeywords::CONST);
  file << falign(align) << "{" << std::endl;

  if (DHGenerator::createAccessors() && _returnType->type() != "void")
  {
    // check if accessor was set, of so return accessor
    std::map<Function*, std::string>::iterator imt =
        DHGenerator::dataHandleAccessorMap().find(this);
    file << falign(align + 2) << "if( _" << imt->second << "Ptr != 0)" << std::endl;
    file << falign(align + 4) << "return *_" << imt->second << "Ptr;" << std::endl;
  }

  // look thru XML datat and create function implementation
  DHGenerator::getMockFunImplFromXML(this, file, align + 2);

  // create loger meassage gor get/is/set/load functions
  boost::regex expression("(get|is|set|load).*");
  boost::cmatch result;
  if (boost::regex_match(_name.c_str(), result, expression))
  {
    if (boost::algorithm::ends_with(_name, "Range"))
    {
      std::string functionWithVector = _name.substr();
      functionWithVector.erase(functionWithVector.end() - 5, functionWithVector.end());

      std::string parametersString = "( ";
      for (const auto& parm : _params)
        parametersString += parm->name() + " ,";

      parametersString.pop_back();
      parametersString += ") ";

      std::string& typeStr = _returnType->type();

      auto itBegin = typeStr.begin();
      auto openBracketIt = itBegin + typeStr.find('<');
      auto commaIt = itBegin + typeStr.find(',');
      auto closeBracketLastIt = itBegin + typeStr.find_last_of('>');

      std::string type1st(openBracketIt + 1, commaIt);
      std::string type2nd(commaIt + 1, closeBracketLastIt);

      file << falign(align + 2) << type2nd << " context;" << std::endl;

      file << falign(align + 2) << "auto predicate = [](const " << type1st << "*, const " << type2nd
           << "&) {return true;};" << std::endl;
      file << falign(align + 2) << "return makeFilterIteratorRange(&" << functionWithVector
           << parametersString << ",std::move(context), predicate);" << std::endl;
      file << falign(align) << "}" << std::endl;

      return;
    }
    std::stringstream msg;
    msg << "\"Not handled call to DataHandle::" << _name << " with parameters: \"";
    file << falign(align + 2) << "LOG4CXX_INFO(logger, \"Not handled call to DataHandle::" + _name +
                                     " with parameters: \"";
    PtrList<Parameter>::iterator ipb = _params.begin(), ipe = _params.end();
    for (; ipb != ipe; ipb++)
      if (GlobalNamespace::isPrintable((*ipb)->type()))
      {
        file << "<<\"" << (*ipb)->name() << "=\"<<" << (*ipb)->name() << "<<\" \"";
        msg << "<<\"" << (*ipb)->name() << "=\"<<" << (*ipb)->name() << "<<\" \"";
      }
      else if ((*ipb)->type() == "GlobalDirection")
      {
        file << "<<\"" << (*ipb)->name() << "=\"<<"
             << "*globalDirectionToStr(" << (*ipb)->name() << ")"
             << "<<\" \"";
        msg << "<<\"" << (*ipb)->name() << "=\"<<"
            << "*globalDirectionToStr(" << (*ipb)->name() << ")"
            << "<<\" \"";
      }
      else if ((*ipb)->type() == "GeoTravelType")
      {
        file << "<<\"" << (*ipb)->name() << "=\"<<"
             << "TseUtil::getGeoType(" << (*ipb)->name() << ")"
             << "<<\" \"";
        msg << "<<\"" << (*ipb)->name() << "=\"<<"
            << "TseUtil::getGeoType(" << (*ipb)->name() << ")"
            << "<<\" \"";
      }
    file << ");" << std::endl;
    file << "std::cout << std::endl << " << msg.str() << ";" << std::endl;
  }

  // for not void return default value (created member of DataHandleMock class)
  if (_returnType->type() != "void")
  {
    if (_returnType->ptrRef() == '*')
      file << falign(align + 2) << "static " << _returnType->type() << "* ret = 0;" << std::endl;
    else
      file << falign(align + 2) << "static " << _returnType->type() << " ret;" << std::endl;
    file << falign(align + 2) << "return "
         << "ret;" << std::endl;
  }
  file << falign(align) << "}" << std::endl;
}
void
Function::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  // skip implemented functions
  if (_isImplemented)
    return;

  // remove static in cpp
  bool isStatic = _cppKey.isFlag(CPPKeywords::STATIC);
  _cppKey.unsetFlag(CPPKeywords::STATIC);
  std::string keyw = _cppKey.printPre();
  if (isStatic)
    _cppKey.setFlag(CPPKeywords::STATIC);

  size_t ali = align + _name.size() + 2 + str.size();
  if (_template)
    _template->printDHCpp(file, str, align);
  file << falign(align) << keyw;
  _returnType->printDHCpp(file, str, 0);
  file << std::endl << falign(align);
  if (_isOperator)
  {
    file << str << "operator " << _name << "(";
    ali += 9 + str.size();
  }
  else
    file << str << _name << "( ";
  _params.printDHCpp(file, str, (int)ali);
  file << ")" << _cppKey.printPost() << std::endl << falign(align) << "{" << std::endl;

  // exception for ticketDate - _ticketDate is set in DataHandle.h
  if (_name == "ticketDate")
  {
    file << falign(align + 2) << "return _ticketDate;" << std::endl;
  }
  else if (!_isConstructor && !_isDestructor && !_isDestructor)
  {
    file << falign(align + 2) << "return DataHandleMockFactory::getMock()->" << _name << "(";
    ali = align + 44 + _name.size();
    _params.printName(file, str, (int)ali);
    file << "); " << std::endl;
  }
  file << falign(align) << "}" << std::endl;
}
void
Function::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Function")
  {
    if (_name == str1)
      _name = str2;
  }
  if (_returnType)
    _returnType->rename(str1, str2, id);
  _params.rename(str1, str2, id);
  _constructorInits.rename(str1, str2, id);
  if (_template)
    _template->rename(str1, str2, id);
}
void
Function::remove(const std::set<std::string>& ignoreSet, const std::string& id)
{
  _params.remove(ignoreSet, id);
}
Function&
Function::
operator=(const Function& f)
{
  _name = f._name;
  _lineNo = f._lineNo;
  _isConstructor = f._isConstructor;
  _isDestructor = f._isDestructor;
  _isImplemented = f._isImplemented;
  _isOperator = f._isOperator;
  _returnType = f._returnType;
  _params = f._params;
  _constructorInits = f._constructorInits;
  _template = f._template;
  _protection = f._protection;
  _cppKey = f._cppKey;
  return *this;
}
/***************************************************************
****************************************************************
**                      Parameter                             **
****************************************************************
***************************************************************/
void
Parameter::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  // same printing as in header, just add default value
  printMockCpp(file, str, align);
  if (_initViaConstructor)
    file << "( " << _defaultValue << " )";
  else if (_defaultValue.size())
    file << " = " << _defaultValue;
}
void
Parameter::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align);
  std::string keyw = _cppKey.print();
  if (keyw.size())
    file << keyw;
  file << _type << (_type.size() ? " " : "");
  if (_ptrRef != ' ')
    file << _ptrRef;
  file << _name;
}
void
Parameter::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  printMockCpp(file, str, align);
}
void
Parameter::printAccessorName(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align);
  bool isConst = _cppKey.isFlag(CPPKeywords::CONST);
  _cppKey.unsetFlag(CPPKeywords::CONST);
  std::string keyw = _cppKey.print();
  if (isConst)
    _cppKey.setFlag(CPPKeywords::CONST);
  if (keyw.size())
    file << keyw;
  file << _type << (_type.size() ? " " : "");
  if (_ptrRef != ' ' && _ptrRef != '&')
    file << _ptrRef;
  file << "*";
}
std::string
Parameter::printParameterWithInit(bool fromMock)
{
  std::string ret = _cppKey.print();
  if (ret.size())
    ret += " ";
  if (_type.size())
    ret += _type + " ";
  if (_ptrRef != ' ')
    ret += _ptrRef;
  ret += _name;
  if (_defaultValue.size())
  {
    // need rename _logger name in mock file (add "Mock" before "
    std::string defVal = _defaultValue;
    if (fromMock && _name.find("_logger") != std::string::npos)
    {
      boost::regex reg("\\\"\\)");
      std::string fmt("Mock\\\"\\)");
      defVal = boost::regex_replace(defVal, reg, fmt);
    }
    if (_initViaConstructor)
      ret += "( " + defVal + " )";
    else
      ret += " = " + defVal;
  }
  return ret;
}
void
Parameter::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Parameter")
  {
    if (_name == str1)
      _name = str2;
  }
  else if (id == "ParameterInit")
  {
    boost::regex regFrom(str1 + "::");
    std::string regTo = str2 + "::";
    _name = boost::regex_replace(_name, regFrom, regTo, boost::match_default | boost::format_sed);
    _defaultValue = boost::regex_replace(
        _defaultValue, regFrom, regTo, boost::match_default | boost::format_sed);
  }
}
Parameter&
Parameter::
operator=(const Parameter& p)
{
  _lineNo = p._lineNo;
  _name = p._name;
  _type = p._type;
  _defaultValue = p._defaultValue;
  _cppKey = p._cppKey;
  _ptrRef = p._ptrRef;
  return *this;
}
/***************************************************************
****************************************************************
**                      Include                               **
****************************************************************
***************************************************************/
void
Include::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align) << "#include " << _name;
}
void
Include::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  printMockHeader(file, str, align);
}
void
Include::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  printMockHeader(file, str, align);
}
void
Include::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Include")
  {
    if (_name == str1)
      _name = str2;
  }
}
Include&
Include::
operator=(const Include& i)
{
  _lineNo = i._lineNo;
  _name = i._name;
  return *this;
}
/***************************************************************
****************************************************************
**                      Namespace                             **
****************************************************************
***************************************************************/
Namespace::Namespace(bool isUsing) : _isUsing(isUsing) {}
void
Namespace::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  file << falign(align);
  if (_isUsing)
  {
    file << "using namepspace " << _name << ";" << std::endl;
    return;
  }
  if (_name.size())
  {
    file << "namespace " << _name << std::endl << falign(align) << "{" << std::endl;
  }
  _fwrdDecls.printMockHeader(file, str, align, ';', true, false);
  _classes.printMockHeader(file, str, align, ';', true, false);
  _parameters.printMockHeader(file, str, align, ';', true, false);
  _functions.printMockHeader(file, str, align, ';', true, false);
  if (_name.size())
  {
    file << falign(align) << "}" << std::endl;
  }
}
void
Namespace::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  if (_isUsing)
    return;
  file << falign(align);
  if (_name.size())
  {
    file << "namespace " << _name << std::endl << falign(align) << "{" << std::endl;
  }
  // parameter initialization
  PtrList<Parameter>::const_iterator ipb = _paramInit.begin(), ipe = _paramInit.end();
  for (; ipb != ipe; ipb++)
    file << falign(align) << (*ipb)->printParameterWithInit(true) << ";" << std::endl;

  _classes.printMockCpp(file, str, align, ';', true, false);
  _functions.printMockCpp(file, str, align, ' ', true, false);
  if (_name.size())
  {
    file << falign(align) << "}" << std::endl;
  }
}
void
Namespace::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  if (_isUsing)
    return;
  file << falign(align);
  if (_name.size())
  {
    file << "namespace " << _name << std::endl << falign(align) << "{" << std::endl;
  }
  // parameter initialization
  PtrList<Parameter>::const_iterator ipb = _paramInit.begin(), ipe = _paramInit.end();
  for (; ipb != ipe; ipb++)
    file << falign(align) << (*ipb)->printParameterWithInit(false) << ";" << std::endl;

  _classes.printDHCpp(file, str, align, ';', true, false);
  _functions.printDHCpp(file, str, align, ' ', true, false);
  if (_name.size())
  {
    file << falign(align) << "}" << std::endl;
  }
}
void
Namespace::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Namespace")
  {
    if (_name == str1)
      _name = str2;
  }
  _classes.rename(str1, str2, id);
  _parameters.rename(str1, str2, id);
  _functions.rename(str1, str2, id);
  _fwrdDecls.rename(str1, str2, id);
  _paramInit.rename(str1, str2, id);
}
void
Namespace::remove(const std::set<std::string>& ignoreSet, const std::string& id)
{
  _classes.remove(ignoreSet, id);
  _parameters.remove(ignoreSet, id);
  _functions.remove(ignoreSet, id);
  _fwrdDecls.remove(ignoreSet, id);
  _paramInit.remove(ignoreSet, id);
}
Namespace&
Namespace::
operator=(const Namespace& n)
{
  _lineNo = n._lineNo;
  _name = n._name;
  _classes = n._classes;
  _parameters = n._parameters;
  _functions = n._functions;
  _fwrdDecls = n._fwrdDecls;
  _isUsing = n._isUsing;
  return *this;
}
/***************************************************************
****************************************************************
**                      GlobalNamespace                       **
****************************************************************
***************************************************************/
std::string
GlobalNamespace::checkTypedef(const std::string& type)
{
  std::string rettype = type;
  PtrList<Typedef>::const_iterator it = _typedefs.begin(), ie = _typedefs.end();
  for (; it != ie; it++)
  {
    if ((*it)->name() == type)
    {
      // recurection to check base type
      rettype = checkTypedef((*it)->type());
      break;
    }
  }
  return rettype;
}
bool
GlobalNamespace::isBool(const std::string& rtype)
{
  std::string type = checkTypedef(rtype);
  return (type == "bool");
}
bool
GlobalNamespace::isInt(const std::string& rtype)
{
  std::string type = checkTypedef(rtype);
  return (type == "int") || (type == "int8_t") || (type == "int16_t") || (type == "int32_t") ||
         (type == "int64_t") || (type == "int128_t") || (type == "unisigned int") ||
         (type == "uint") || (type == "uint8_t") || (type == "uint16_t") || (type == "uint32_t") ||
         (type == "uint64_t") || (type == "uint128_t") || (type == "uint128_t") ||
         (type == "size_t") || (type == "pthread_key_t");
}
bool
GlobalNamespace::isFloat(const std::string& rtype)
{
  std::string type = checkTypedef(rtype);
  return (type == "float") || (type == "unsigned float") || (type == "double") ||
         (type == "unsigned double");
}
bool
GlobalNamespace::isChar(const std::string& rtype)
{
  std::string type = checkTypedef(rtype);
  return (type == "char") || (type == "unsigned char");
}
bool
GlobalNamespace::isString(const std::string& rtype)
{
  boost::regex expression("(std::)?c?w?string|Code[ ]*<[0-9]+>");
  boost::cmatch result;
  std::string type = checkTypedef(rtype);
  return boost::regex_match(type.c_str(), result, expression);
}
bool
GlobalNamespace::isPrintable(const std::string& rtype)
{
  std::string type = checkTypedef(rtype);
  if ((type == "bool") || (type == "int") || (type == "int8_t") || (type == "int16_t") ||
      (type == "int32_t") || (type == "int64_t") || (type == "int128_t") ||
      (type == "unisigned int") || (type == "uint") || (type == "uint8_t") ||
      (type == "uint16_t") || (type == "uint32_t") || (type == "uint64_t") ||
      (type == "uint128_t") || (type == "uint128_t") || (type == "size_t") ||
      (type == "pthread_key_t") || (type == "float") || (type == "unsigned float") ||
      (type == "double") || (type == "unsigned double") || (type == "char") ||
      (type == "unsigned char") || (type == "DateTime"))
    return true;

  boost::regex expression("(std::)?c?w?string|Code[ ]*<[0-9]+>");
  boost::cmatch result;
  return boost::regex_match(type.c_str(), result, expression);
}
void
GlobalNamespace::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  //_defines.printMockHeader(file, str, align, ' ', true, false);
  _includes.printMockHeader(file, str, align, ' ', true, false);
  _usedNamespaces.printMockHeader(file, str, align, ';', true, false);
  _namespaces.printMockHeader(file, str, align, ' ', true, false);
  Namespace::printMockHeader(file, str, align);
}
void
GlobalNamespace::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  _includes.printMockCpp(file, str, align, ' ', true, false);
  file << falign(align) << "#include \"DataHandleMock.h\"" << std::endl;
  file << falign(align) << "#include \"DataHandleMockFactory.h\"" << std::endl;
  file << falign(align) << "#include \"Common/TseUtil.h\"" << std::endl;
  file << falign(align) << "#include \"Common/Logger.h\"" << std::endl;
  file << falign(align) << "\nnamespace" << std::endl << falign(align) << "{\ntse::Logger "
       << "logger(\"atseintl.DBAccess.DataHandleMock\");\n}\n" << std::endl;
  _namespaces.printMockCpp(file, str, align, ' ', true, false);
  Namespace::printMockCpp(file, str, align);
}
void
GlobalNamespace::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  _includes.printDHCpp(file, str, align, ' ', true, false);
  file << falign(align) << "#include \"DataHandleMockFactory.h\"" << std::endl;
  file << falign(align) << "#include \"DataHandle.h\"" << std::endl;
  _namespaces.printDHCpp(file, str, align, ' ', true, false);
  Namespace::printDHCpp(file, str, align);
}
void
GlobalNamespace::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "GlobalNamespace")
  {
    if (_name == str1)
      _name = str2;
  }
  _defines.rename(str1, str2, id);
  _includes.rename(str1, str2, id);
  _usedNamespaces.rename(str1, str2, id);
  _namespaces.rename(str1, str2, id);
  _typedefs.rename(str1, str2, id);
  Namespace::rename(str1, str2, id);
}
void
GlobalNamespace::remove(const std::set<std::string>& ignoreSet, const std::string& id)
{
  _defines.remove(ignoreSet, id);
  _includes.remove(ignoreSet, id);
  _usedNamespaces.remove(ignoreSet, id);
  _namespaces.remove(ignoreSet, id);
  _typedefs.remove(ignoreSet, id);
  Namespace::remove(ignoreSet, id);
}
GlobalNamespace&
GlobalNamespace::
operator=(const GlobalNamespace& n)
{
  Namespace::operator=(n);
  _defines = n._defines;
  _includes = n._includes;
  _usedNamespaces = n._usedNamespaces;
  _namespaces = n._namespaces;
  return *this;
}
/***************************************************************
****************************************************************
**                      Template                              **
****************************************************************
***************************************************************/
void
Template::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  file << "template <";
  _params.printMockHeader(file, str, align + 11);
  file << " >\n";
}
void
Template::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  // same as in header
  printMockHeader(file, str, align);
}
void
Template::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  // same as in header
  printMockHeader(file, str, align);
}
void
Template::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Template")
  {
    if (_name == str1)
      _name = str2;
  }
  _params.rename(str1, str2, id);
}
Template&
Template::
operator=(const Template& t)
{
  _name = t._name;
  _lineNo = t._lineNo;
  _params = t._params;
  return *this;
}
/***************************************************************
****************************************************************
**                      Typedef                               **
****************************************************************
***************************************************************/
void
Typedef::printMockHeader(std::ofstream& file, const std::string& str, int align)
{
  file << "typedef " << _type << " " << _name;
}
void
Typedef::printMockCpp(std::ofstream& file, const std::string& str, int align)
{
  // same as in header
  printMockHeader(file, str, align);
}
void
Typedef::printDHCpp(std::ofstream& file, const std::string& str, int align)
{
  // same as in header
  printMockHeader(file, str, align);
}
void
Typedef::rename(const std::string str1, const std::string& str2, const std::string& id)
{
  if (id == "Typedef")
  {
    if (_name == str1)
      _name = str2;
  }
}
Typedef&
Typedef::
operator=(const Typedef& t)
{
  _name = t._name;
  _lineNo = t._lineNo;
  _type = t._type;
  return *this;
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
