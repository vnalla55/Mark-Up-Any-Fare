#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/function.hpp>

#include <memory>
#include <set>
#include <stack>

/***************************************************************
****************************************************************
**                      Protection                            **
****************************************************************
***************************************************************/
class Protection
{
public:
  enum Prot
  {
    Public,
    Protected,
    Private,
    Global
  };
  Protection(Prot p = Global) : _protection(p) {}
  std::string print();
  bool operator==(const Protection& p) { return _protection == p._protection; }
  bool operator!=(const Protection& p) { return _protection != p._protection; }
  Protection& operator=(const Protection&);
  Protection& operator=(const Prot&);

private:
  Prot _protection;
};
/***************************************************************
****************************************************************
**                      CPPKeywords                           **
****************************************************************
***************************************************************/
class CPPKeywords
{
  int64_t _keyw;

public:
  CPPKeywords(const CPPKeywords& c) : _keyw(c._keyw) {}
  CPPKeywords() : _keyw(0) {};
  void reset() { _keyw = 0; }
  enum CPPKEYW
  {
    AUTO = 0x00000001,
    CLASS = 0x00000002,
    CONST = 0x00000004,
    CONTINUE = 0x00000008,
    STATIC = 0x00000010,
    EXPLICIT = 0x00000020,
    EXTERN = 0x00000040,
    FRIEND = 0x00000080,
    INLINE = 0x00000100,
    MUTABLE = 0x00000200,
    REGISTER = 0x00000400,
    TEMPLATE = 0x00000800,
    THROW = 0x00001000,
    TYPEDEF = 0x00002000,
    TYPEID = 0x00004000,
    TYPENAME = 0x00008000,
    UNION = 0x00010000,
    USING = 0x00020000,
    VIRTUAL = 0x00040000,
    VOLATILE = 0x00080000
  };
  bool isFlag(CPPKEYW flag) { return (_keyw & flag) != 0; }
  void setFlag(CPPKEYW flag) { _keyw |= flag; }
  void unsetFlag(CPPKEYW flag) { _keyw &= ~flag; }

  CPPKeywords& operator=(const CPPKeywords&);
  std::string print();
  std::string printPre();
  std::string printPost();
};

/***************************************************************
****************************************************************
**                      PtrList                               **
****************************************************************
***************************************************************/
template <typename T>
class PtrList : public std::vector<std::shared_ptr<T>>
{
  typedef std::vector<std::shared_ptr<T>> __base;
  boost::function<void(T*, std::ofstream&, const std::string&, int)> _printFun;
  PtrList(const PtrList&);

public:
  PtrList() {}
  virtual ~PtrList()
  {
    typename std::vector<std::shared_ptr<T>>::iterator it = __base::begin();
    typename std::vector<std::shared_ptr<T>>::iterator ie = __base::end();
    for (; it != ie; it++)
      it->reset();
  }
  void printName(std::ofstream& file,
                 const std::string& str,
                 int align,
                 char searator = ',',
                 bool breakline = false,
                 bool ignorelastsepbreak = true,
                 bool ignorefirstsep = false)
  {
    _printFun = &T::printName;
    print(file, str, align, searator, breakline, ignorelastsepbreak, ignorefirstsep);
  }
  void printMockHeader(std::ofstream& file,
                       const std::string& str,
                       int align,
                       char searator = ',',
                       bool breakline = true,
                       bool ignorelastsepbreak = true,
                       bool ignorefirstsep = false)
  {
    _printFun = &T::printMockHeader;
    print(file, str, align, searator, breakline, ignorelastsepbreak, ignorefirstsep);
  }
  void printMockCpp(std::ofstream& file,
                    const std::string& str,
                    int align,
                    char searator = ',',
                    bool breakline = true,
                    bool ignorelastsepbreak = true,
                    bool ignorefirstsep = false)
  {
    _printFun = &T::printMockCpp;
    print(file, str, align, searator, breakline, ignorelastsepbreak, ignorefirstsep);
  }
  void printDHCpp(std::ofstream& file,
                  const std::string& str,
                  int align,
                  char searator = ',',
                  bool breakline = true,
                  bool ignorelastsepbreak = true,
                  bool ignorefirstsep = false)
  {
    _printFun = &T::printDHCpp;
    print(file, str, align, searator, breakline, ignorelastsepbreak, ignorefirstsep);
  }
  void print(std::ofstream& file,
             const std::string& str,
             int align,
             char searator = ',',
             bool breakline = true,
             bool ignorelastsepbreak = true,
             bool ignorefirstsep = false)
  {
    typename std::vector<std::shared_ptr<T>>::iterator it = __base::begin();
    typename std::vector<std::shared_ptr<T>>::iterator ie = __base::end();
    int ali = 0;
    while (it != ie)
    {
      _printFun((*it).get(), file, str, ali);
      it++;
      if (it != ie || !ignorelastsepbreak)
        file << searator;
      if (breakline && (it != ie || !ignorelastsepbreak))
      {
        file << std::endl;
        ali = align;
      }
    }
  }
  void rename(const std::string& str1, const std::string& str2, const std::string& id)
  {
    typename std::vector<std::shared_ptr<T>>::iterator it = __base::begin();
    typename std::vector<std::shared_ptr<T>>::iterator ie = __base::end();
    for (; it != ie; it++)
      (*it)->rename(str1, str2, id);
  }
  void remove(const std::set<std::string>& ignoreSet, const std::string& id)
  {
    if (T::__id == id)
      removeName(ignoreSet);
    typename std::vector<std::shared_ptr<T>>::iterator it = __base::begin();
    typename std::vector<std::shared_ptr<T>>::iterator ie = __base::end();
    for (; it != ie; it++)
      (*it)->remove(ignoreSet, id);
  }
  void removeName(const std::set<std::string>& ignoreSet)
  {
    typename std::vector<std::shared_ptr<T>>::iterator it = __base::begin();
    while (it != __base::end())
      if (ignoreSet.find((*it)->name()) != ignoreSet.end())
        it = __base::erase(it);
      else
        ++it;
  }
  PtrList& operator=(const PtrList& p)
  {
    __base::clear();
    typename std::vector<std::shared_ptr<T>>::const_iterator it = p.begin();
    typename std::vector<std::shared_ptr<T>>::const_iterator ie = p.end();
    for (; it != ie; it++)
      __base::push_back(*it);
    return *this;
  }
};
/***************************************************************
****************************************************************
**                      PtrList                               **
****************************************************************
***************************************************************/
template <typename T>
class PtrStack : public std::stack<std::shared_ptr<T>>
{
  typedef std::stack<std::shared_ptr<T>> __base;
  PtrStack(const PtrStack&);

public:
  PtrStack() {}
  virtual ~PtrStack()
  {
    while (!__base::empty())
    {
      std::shared_ptr<T> ptr = tpop();
      ptr.reset();
    }
  }
  std::shared_ptr<T> tpop()
  {
    std::shared_ptr<T> p;
    if (__base::empty())
      return p;
    p = __base::top();
    __base::pop();
    return p;
  }
  std::deque<std::shared_ptr<T>>& container() { return __base::c; }
};
/***************************************************************
****************************************************************
**                      StringVec                             **
****************************************************************
***************************************************************/
class StringVec : public std::vector<std::string>
{
  typedef std::vector<std::string> __base;
  StringVec(const StringVec&);

public:
  StringVec() {}

  std::string createString()
  {
    std::string ret;
    std::vector<std::string>::iterator it = __base::begin();
    std::vector<std::string>::iterator ie = __base::end();
    for (; it != ie; it++)
    {
      if (ret.size())
        ret += " ";
      ret += *it;
    }
    return ret;
  }
  std::string lastString()
  {
    std::string ret;
    if (__base::size())
    {
      ret = __base::back();
      __base::pop_back();
    }
    return ret;
  }
  StringVec& operator=(const StringVec& s)
  {
    __base::clear();
    std::vector<std::string>::const_iterator it = s.begin();
    std::vector<std::string>::const_iterator ie = s.end();
    for (; it != ie; it++)
      __base::push_back(*it);
    return *this;
  }
};
/***************************************************************
****************************************************************
**                      CPPElem                               **
****************************************************************
***************************************************************/
class CPPElem
{
protected:
  int _lineNo;
  std::string _name;
  CPPElem(const CPPElem&);

public:
  CPPElem();
  virtual ~CPPElem();

  int& lineNo() { return _lineNo; }
  const int& lineNo() const { return _lineNo; }

  std::string& name() { return _name; }
  const std::string& name() const { return _name; }

  virtual void printName(std::ofstream& file, const std::string& str, int align);
  virtual void printMockHeader(std::ofstream& file, const std::string& str, int align) = 0;
  virtual void printMockCpp(std::ofstream& file, const std::string& str, int align) = 0;
  virtual void printDHCpp(std::ofstream& file, const std::string& str, int align) = 0;
  virtual void rename(const std::string str1, const std::string& str2, const std::string& id) = 0;
  virtual void remove(const std::set<std::string>& ignoreSet, const std::string& id) {}
};

typedef std::shared_ptr<CPPElem> ElemPtr;
typedef PtrStack<CPPElem> ElemStack;
typedef PtrList<CPPElem> ElemList;
/***************************************************************
****************************************************************
**                      Parameter                             **
****************************************************************
***************************************************************/
class Parameter : public CPPElem
{
  std::string _type;
  std::string _defaultValue;
  char _ptrRef;
  bool _initViaConstructor;

  CPPKeywords _cppKey;
  Parameter(const Parameter&);

public:
  Parameter() : _ptrRef(' '), _initViaConstructor(false) {}
  std::string& type() { return _type; }
  const std::string& type() const { return _type; }

  std::string& defaultValue() { return _defaultValue; }
  const std::string& defaultValue() const { return _defaultValue; }

  CPPKeywords& cppKey() { return _cppKey; }
  const CPPKeywords& cppKey() const { return _cppKey; }

  char& ptrRef() { return _ptrRef; }
  const char& ptrRef() const { return _ptrRef; }

  bool& initViaConstructor() { return _initViaConstructor; }
  const bool& initViaConstructor() const { return _initViaConstructor; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void printAccessorName(std::ofstream& file, const std::string& str, int align);
  std::string printParameterWithInit(bool mockFile);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  Parameter& operator=(const Parameter&);

  static const std::string __id;
};
typedef std::shared_ptr<Parameter> ParameterPtr;
/***************************************************************
****************************************************************
**                      BaseClass                             **
****************************************************************
***************************************************************/
class BaseClass : public CPPElem
{
public:
  BaseClass();

  Protection& protecton() { return _protection; }
  const Protection& protection() const { return _protection; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  BaseClass& operator=(const BaseClass&);

  static const std::string __id;

private:
  BaseClass(const BaseClass&);
  Protection _protection;
};
typedef std::shared_ptr<BaseClass> BaseClassPtr;
/***************************************************************
****************************************************************
**                      Template                              **
****************************************************************
***************************************************************/
class Template : public CPPElem
{
  PtrList<Parameter> _params;
  Template(const Template&);

public:
  Template() {}
  PtrList<Parameter>& params() { return _params; }
  const PtrList<Parameter>& params() const { return _params; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  Template& operator=(const Template&);

  static const std::string __id;
};
typedef std::shared_ptr<Template> TemplatePtr;
/***************************************************************
****************************************************************
**                      Define                                **
****************************************************************
***************************************************************/
class Define : public CPPElem
{
  std::string _value;
  Define(const Define&);

public:
  Define() {}
  std::string& value() { return _value; }
  const std::string& value() const { return _value; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  Define& operator=(const Define&);

  static const std::string __id;
};
typedef std::shared_ptr<Define> DefinePtr;
/***************************************************************
****************************************************************
**                      Include                               **
****************************************************************
***************************************************************/
class Include : public CPPElem
{
  Include(const Include&);

public:
  Include() {}
  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  Include& operator=(const Include&);

  static const std::string __id;
};
typedef std::shared_ptr<Include> IncludePtr;
/***************************************************************
****************************************************************
**                      Function                              **
****************************************************************
***************************************************************/
class Function : public CPPElem
{
  bool _isConstructor;
  bool _isDestructor;
  bool _isImplemented;
  bool _isOperator;
  std::shared_ptr<Parameter> _returnType;
  PtrList<Parameter> _params;
  PtrList<Parameter> _constructorInits;
  std::shared_ptr<Template> _template;
  Protection _protection;
  CPPKeywords _cppKey;
  Function(const Function&);

public:
  Function();

  std::shared_ptr<Parameter>& returnType() { return _returnType; }
  const std::shared_ptr<Parameter>& returnType() const { return _returnType; }

  bool& isImplemented() { return _isImplemented; }
  bool isImplemented() const { return _isImplemented; }

  bool& isConstructor() { return _isConstructor; }
  const bool& isConstructor() const { return _isConstructor; }

  bool& isDestructor() { return _isDestructor; }
  const bool& isDestructor() const { return _isDestructor; }

  bool& isOperator() { return _isOperator; }
  const bool& isOperator() const { return _isOperator; }

  PtrList<Parameter>& params() { return _params; }
  const PtrList<Parameter>& params() const { return _params; }

  PtrList<Parameter>& constructorInits() { return _constructorInits; }
  const PtrList<Parameter>& constructorInits() const { return _constructorInits; }

  std::shared_ptr<Template>& tmplte() { return _template; }
  const std::shared_ptr<Template>& tmplte() const { return _template; }

  Protection& protection() { return _protection; }
  const Protection& protection() const { return _protection; }

  CPPKeywords& cppKey() { return _cppKey; }
  const CPPKeywords& cppKey() const { return _cppKey; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  void remove(const std::set<std::string>& ignoreSet, const std::string& id);
  Function& operator=(const Function&);

  static const std::string __id;
};
typedef std::shared_ptr<Function> FunctionPtr;
/***************************************************************
****************************************************************
**                      Class                                 **
****************************************************************
***************************************************************/
class Class : public CPPElem
{
  Class(const Class&);

public:
  enum ClassType
  {
    CLASS_CLASS,
    CLASS_STRUCT,
    CLASS_UNION,
    CLASS_NONE
  };

  Class(ClassType classType);

  ClassType classType() const { return _classType; }

  std::shared_ptr<Template>& tmplte() { return _template; }
  const std::shared_ptr<Template>& tmplte() const { return _template; }

  PtrList<Function>& functions() { return _functions; }
  const PtrList<Function>& functions() const { return _functions; }

  PtrList<BaseClass>& baseClasses() { return _baseClasses; }
  const PtrList<BaseClass>& baseClasses() const { return _baseClasses; }

  PtrList<Class>& nestedClasses() { return _nestedClasses; }
  const PtrList<Class>& nestedClasses() const { return _nestedClasses; }

  PtrList<Parameter>& parameters() { return _parameters; }
  const PtrList<Parameter>& parameters() const { return _parameters; }

  bool& defined() { return _defined; }
  const bool& deined() const { return _defined; }

  Protection& protection() { return _protection; }
  const Protection& protection() const { return _protection; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  void remove(const std::set<std::string>& ignoreSet, const std::string& id);

  Class& operator=(const Class&);

  static const std::string __id;

private:
  ClassType _classType;
  std::shared_ptr<Template> _template;
  PtrList<Function> _functions;
  PtrList<BaseClass> _baseClasses;
  PtrList<Class> _nestedClasses;
  PtrList<Parameter> _parameters;
  bool _defined;
  Protection _protection;
};
typedef std::shared_ptr<Class> ClassPtr;
/***************************************************************
****************************************************************
**                      Namespace                             **
****************************************************************
***************************************************************/
class Namespace : public CPPElem
{
  PtrList<Class> _classes;
  PtrList<Parameter> _parameters;
  PtrList<Function> _functions;
  PtrList<Class> _fwrdDecls;
  bool _isUsing;
  PtrList<Parameter> _paramInit;
  Namespace(const Namespace&);

public:
  Namespace(bool isUsing = false);

  bool& isUsing() { return _isUsing; }
  bool isUsing() const { return _isUsing; }

  PtrList<Class>& classes() { return _classes; }
  const PtrList<Class>& classes() const { return _classes; }

  PtrList<Parameter>& parameters() { return _parameters; }
  const PtrList<Parameter>& parameters() const { return _parameters; }

  PtrList<Function>& functions() { return _functions; }
  const PtrList<Function>& functions() const { return _functions; }

  PtrList<Class>& fwrdDecls() { return _fwrdDecls; }
  const PtrList<Class>& fwrdDecls() const { return _fwrdDecls; }

  PtrList<Parameter>& paramInit() { return _paramInit; }
  const PtrList<Parameter>& paramInit() const { return _paramInit; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  void remove(const std::set<std::string>& ignoreSet, const std::string& id);

  Namespace& operator=(const Namespace&);

  static const std::string __id;
};
typedef std::shared_ptr<Namespace> NamespacePtr;
/***************************************************************
****************************************************************
**                      Typedef                               **
****************************************************************
***************************************************************/
class Typedef : public CPPElem
{
  std::string _type;

  Typedef(const Parameter&);

public:
  Typedef() {}
  std::string& type() { return _type; }
  const std::string& type() const { return _type; }

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);

  Typedef& operator=(const Typedef&);

  static const std::string __id;
};
typedef std::shared_ptr<Typedef> TypedefPtr;
/***************************************************************
****************************************************************
**                      GlobalNamespace                       **
****************************************************************
***************************************************************/
class GlobalNamespace : public Namespace
{
  PtrList<Define> _defines;
  PtrList<Include> _includes;
  PtrList<Namespace> _usedNamespaces;
  PtrList<Namespace> _namespaces;
  static PtrList<Typedef> _typedefs;
  GlobalNamespace(const GlobalNamespace&);

public:
  GlobalNamespace() {}
  PtrList<Define>& defines() { return _defines; }
  const PtrList<Define>& defines() const { return _defines; }

  PtrList<Include>& includes() { return _includes; }
  const PtrList<Include>& includes() const { return _includes; }

  PtrList<Namespace>& usedNamespaces() { return _usedNamespaces; }
  const PtrList<Namespace>& usedNamespaces() const { return _usedNamespaces; }

  PtrList<Namespace>& namespaces() { return _namespaces; }
  const PtrList<Namespace>& namespaces() const { return _namespaces; }

  static PtrList<Typedef>& typedefs() { return _typedefs; }

  static std::string checkTypedef(const std::string& type);
  static bool isBool(const std::string& type);
  static bool isInt(const std::string& type);
  static bool isFloat(const std::string& type);
  static bool isChar(const std::string& type);
  static bool isString(const std::string& type);
  static bool isPrintable(const std::string& type);

  void printMockHeader(std::ofstream& file, const std::string& str, int align);
  void printMockCpp(std::ofstream& file, const std::string& str, int align);
  void printDHCpp(std::ofstream& file, const std::string& str, int align);
  void rename(const std::string str1, const std::string& str2, const std::string& id);
  void remove(const std::set<std::string>& ignoreSet, const std::string& id);

  GlobalNamespace& operator=(const GlobalNamespace&);

  static const std::string __id;
};
typedef std::shared_ptr<GlobalNamespace> GlobalNamespacePtr;
/////////////////////////////////////////////////////////////////////////////////

#endif
