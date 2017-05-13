/*****************************

  Simple CPP/Header parser

 ****************************/


%{
/*
 *  Includes
 */
 
#include "Types.h"
#include "DHHeaderParser.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace boost;

void LOG_DEBUG(const char* str, const char* str2=NULL, const char* str3=NULL, const char* str4=NULL)
{
  LOG4CXX_DEBUG(((log4cxx::LoggerPtr)*g_parser), g_parser->msg(str, str2, str3, str4));
}
void LOG_TRACE(const char* str, const char* str2=NULL, const char* str3=NULL, const char* str4=NULL)
{
  LOG4CXX_TRACE(((log4cxx::LoggerPtr)*g_parser), g_parser->msg(str, str2, str3, str4));
}
void LOG_INFO(const char* str, const char* str2=NULL, const char* str3=NULL, const char* str4=NULL)
{
  LOG4CXX_INFO(((log4cxx::LoggerPtr)*g_parser), g_parser->msg(str, str2, str3, str4));
}
void LOG_WARNING(const char* str, const char* str2=NULL, const char* str3=NULL, const char* str4=NULL) 
{
  LOG4CXX_WARN(((log4cxx::LoggerPtr)*g_parser), g_parser->msg(str, str2, str3, str4));
}
void LOG_ERROR(const char* str, const char* str2=NULL, const char* str3=NULL, const char* str4=NULL) 
{
  LOG4CXX_ERROR(((log4cxx::LoggerPtr)*g_parser), g_parser->msg(str, str2, str3, str4));
}
void LOG_ASSERT(bool condition, const char* str, const char* str2=NULL, const char* str3=NULL, const char* str4=NULL) 
{
  LOG4CXX_ASSERT(((log4cxx::LoggerPtr)*g_parser), condition, g_parser->msg(str, str2, str3, str4));
}

%}

%x  FindMembersH
%x  FindMembersCpp
%x  BaseClassSection
%x  IncludeStart
%x  DefineStart
%x  DefineEnd
%x  UsingNamespaceStart
%x  FinishLine
%x  TemplateSection
%x  FunParameters
%x  ConstructorInitailization
%x  Operator
%x  Typedefs
%x  TypedefsDef

%x  SkipComment
%x  SkipFunDecl

%x  ParameterParser
%x  ParameterParserInit
%x  FunctionNameParser
%x  ParameterInitializationViaConstructor

%%


<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*auto[ \t]*     {  g_parser->setCppKey(CPPKeywords::AUTO);              }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*const[ \t]*    {  g_parser->setCppKey(CPPKeywords::CONST);             }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*static[ \t]*   {  g_parser->setCppKey(CPPKeywords::STATIC);            }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*explicit[ \t]* {  g_parser->setCppKey(CPPKeywords::EXPLICIT);          }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*extern[ \t]*   {  g_parser->setCppKey(CPPKeywords::EXTERN);            }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*friend[ \t]*   {  g_parser->setCppKey(CPPKeywords::FRIEND);            }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*inline[ \t]*   {  g_parser->setCppKey(CPPKeywords::INLINE);            }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*mutable[ \t]*  {  g_parser->setCppKey(CPPKeywords::MUTABLE);           }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*register[ \t]* {  g_parser->setCppKey(CPPKeywords::REGISTER);          }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*throw[ \t]*:   {  g_parser->setCppKey(CPPKeywords::THROW);             }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*virtual[ \t]*  {  g_parser->setCppKey(CPPKeywords::VIRTUAL);           }
<FindMembersH,FindMembersCpp,ParameterParser,Operator>[ \t]*volatile[ \t]* {  g_parser->setCppKey(CPPKeywords::VOLATILE);          }
<FindMembersH>[ \t]*class[ \t]*                                            {  g_parser->createClass(Class::CLASS_CLASS);           }
<FindMembersH>[ \t]*struct[ \t]*                                           {  g_parser->createClass(Class::CLASS_STRUCT);          }
<FindMembersH>[ \t]*union[ \t]*                                            {  g_parser->createClass(Class::CLASS_UNION);           }
<FindMembersH>[ \t]*public:[ \t]*                                          {  g_parser->setProtection(Protection::Public);         }
<FindMembersH>[ \t]*protected:[ \t]*                                       {  g_parser->setProtection(Protection::Protected);      }
<FindMembersH>[ \t]*private:[ \t]*                                         {  g_parser->setProtection(Protection::Private);        }
<FindMembersH,FindMembersCpp>^[ \t]*"#"[ \t]*include[ \t]+                 {  g_parser->createInclude();                           }
<FindMembersH>^[ \t]*"#"[ \t]*define[ \t]+                                 {  g_parser->createDefine();                            }
<FindMembersH,FindMembersCpp>^[ \t]*using[ \t\r\n]+namespace[ \t]+         {  g_parser->createNamespace(true);                     }
<FindMembersH,FindMembersCpp>^[ \t\r\n]*namespace[ \t]+                    {  g_parser->createNamespace(false);                    }
<FindMembersH>^[ \t]*template[ \t\r\n]*"<"                                 {  g_parser->createTemplate();                          }
<FindMembersH>[ \t]*"operator"[ \t]*\([ \t]*\)[ \t]*  {
  // special handling for functor
  LOG_DEBUG("functor found");
  g_parser->oper() = true;
  g_parser->stringVec().push_back("()");
  BEGIN( Operator);
}
<FindMembersH>[ \t]operator[ \t]*  {
  LOG_DEBUG("operator found");
  g_parser->oper() = true;
  BEGIN( Operator);
}
<FindMembersH,FindMembersCpp,Typedefs>^[ \t]*"typedef"[ \t]+   {  g_parser->createTypedef();  }
<Operator>-|--|-=|->|!|!=|%|&|&&|&=|\*|\*=|\.|\/|\/=|:|\[[ \t]*\]|\||\|\||\|=|\+|\+=|\<|\<=|\<\<|\<\>|=|==|\>|\>=|\>\>|\>\>=|[a-z_A-Z0-9*&]+  {
  LOG_DEBUG("Operator: ", yytext);
  // small workaround if we have operator like const char*, need to add spaces between names
  if(g_parser->stringVec().empty())
    g_parser->conversion() = true;
  g_parser->stringVec().push_back(yytext);
}
<FindMembersH,Operator>[ \t]*"("                            {  g_parser->createFunction();            }
<BaseClassSection,FindMembersH>[ \t\r\n]*"{"  {
  g_parser->lineCount();
  // this is to handle 'const class RuleItemInfo* getRuleItemInfo()'
  g_parser->checkPostClass();

  // can open namespace, function implementation, class, struct, union (or BaseClass)
  LOG_TRACE("found {");
  // check that we created some element
  LOG_ASSERT(bool(g_parser->cppElem()),"starting unknown section ");

  // if base class exist, and we start class implementation
  BaseClassPtr bc = std::dynamic_pointer_cast<BaseClass>( g_parser->cppElem() );
  if(bc)
  {
    ClassPtr cl;
    if(g_parser->elemStack().size())
      cl = std::dynamic_pointer_cast<Class>(g_parser->elemStack().top());
    BaseClassPtr bc = std::dynamic_pointer_cast<BaseClass>(g_parser->cppElem());
    LOG_ASSERT(cl && bc, "Incorrect combination when trying to set base class");
    cl->baseClasses().push_back(bc);
    LOG_DEBUG("Base class ", bc->name().c_str(), " of class ", cl->name().c_str());
    // restore class in CppElem
    g_parser->cppElem() = g_parser->elemStack().tpop();
    BEGIN(FindMembersH);
  }

  NamespacePtr nm = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
  ClassPtr cl = std::dynamic_pointer_cast<Class>(g_parser->cppElem());
  FunctionPtr fun = std::dynamic_pointer_cast<Function>(g_parser->cppElem());

  // if namespace then do nothing
  if(cl)
  {
    cl->defined() = true;
    LOG_DEBUG("Class ", cl->name().c_str(), " marked as being defined");
  }
  // function - mark as implemented and skip
  else if(fun)
  {
    fun->isImplemented() = true;
    LOG_TRACE("Skiping function ", fun->name().c_str(), " declaration");
    g_parser->brackCount() = 1;
    g_parser->saveLastContext();
    BEGIN( SkipFunDecl );
  }
  LOG_ASSERT(nm || cl || fun, "unknown start element for \"{\"");
  // put element on stack
  g_parser->elemStack().push(g_parser->cppElem());
  g_parser->cppElem().reset();
}
<FindMembersH>[ \t\r\n]*"}"  {
  g_parser->lineCount();
  // can close namespace or class (function implementaton is handled in 'SkipFunDecl')
  LOG_TRACE("found }");
  LOG_ASSERT(g_parser->elemStack().size()>0, "Incorrect stack size");
  g_parser->cppElem() = g_parser->elemStack().tpop();
  if(g_parser->cppElem())
  {
    ElemPtr      elem;
    if(g_parser->elemStack().size())
     elem = g_parser->elemStack().top();
    NamespacePtr nm   = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
    ClassPtr     cl   = std::dynamic_pointer_cast<Class>(g_parser->cppElem());
    NamespacePtr enm  = std::dynamic_pointer_cast<Namespace>(elem);
    ClassPtr     ecl  = std::dynamic_pointer_cast<Class>(elem);

    // if class
    if(cl)
    {
      // if nested class
      if(ecl)
      {
        LOG_TRACE("Found nested class ", ecl->name().c_str(),"::",cl->name().c_str());
        ecl->nestedClasses().push_back(cl);
      }
      else if(enm)
      {
        LOG_TRACE("Adding class ", cl->name().c_str(), " to namespace ", enm->name().c_str());
        enm->classes().push_back(cl);
      }
      else
      {
        LOG_TRACE("Adding class ", cl->name().c_str(), " to global namespace ");
        g_parser->globalNamespace().classes().push_back(cl);
      }

      // in case of class A{} a; we put class name into stringVec
      g_parser->stringVec().push_back(cl->name());
    }
    else if (nm)
    {
      g_parser->cppElem() = elem;
    }
    g_parser->cppElem().reset();
  }
}
<FindMembersH>[ \t\r\n]*":"   {
  g_parser->lineCount();
  LOG_TRACE("found :");
  if(g_parser->cppElem())
  {
    FunctionPtr  fun  = std::dynamic_pointer_cast<Function>(g_parser->cppElem());
    ClassPtr     cl   = std::dynamic_pointer_cast<Class>(g_parser->cppElem());
    if(cl)
    {
      BEGIN( BaseClassSection );
    }
    else if (fun)
    {
      BEGIN( ConstructorInitailization );
    }
  }
}
<FindMembersH>[ \t\r\n]*";"   {
  g_parser->lineCount();
  // this is to handle 'const class RuleItemInfo* getRuleItemInfo()'
  g_parser->checkPostClass();


  // semicolon can be after function, member, global variable or class forward declaration
  // it can be also double semicolon or unnecesery, need to igonre them

  if(g_parser->cppElem() || (g_parser->stringVec().size() > 1))
  {
    ElemPtr      elem;
    if(g_parser->elemStack().size())
      elem = g_parser->elemStack().top();
    FunctionPtr  fl   = std::dynamic_pointer_cast<Function>(g_parser->cppElem());
    NamespacePtr nm   = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
    ClassPtr     cl   = std::dynamic_pointer_cast<Class>(g_parser->cppElem());
    NamespacePtr enm  = std::dynamic_pointer_cast<Namespace>(elem);
    ClassPtr     ecl  = std::dynamic_pointer_cast<Class>(elem);

    ParameterPtr param;
    // if something in stringVec, then we create parameter
    if(g_parser->stringVec().size() > 1)
      param = g_parser->createParameter();
    // if class
    if(cl)
    {
      // if not defined then this is forward declaration
      if(!cl->defined())
      {
        if(g_parser->cppKey().isFlag(CPPKeywords::FRIEND))
        {
          LOG_DEBUG("Skiping friend class ", cl->name().c_str());
        }
        else
        {
          LOG_DEBUG("Class ", cl->name().c_str(), " forward declaration");
          if(enm)
            enm->fwrdDecls().push_back(cl);
          else
            g_parser->globalNamespace().fwrdDecls().push_back(cl);
        }
      }
    }
    else if (fl) // if function
    {
      g_parser->saveLastContext();
      g_parser->checkFunctionEnd(false);
    }
    else if (ecl)
    {
      // if parameter of the class
      if(param)
      {
        LOG_DEBUG("found class member", param->name().c_str(), " in class ", ecl->name().c_str());
        ecl->parameters().push_back(param);
      }
    }
    else if(nm) // if namespace
    {
      if(param) // parameter in namespace
      {
        LOG_DEBUG("Found parameter ", param->name().c_str(), " in namespace ", nm->name().c_str());
        nm->parameters().push_back(param);
      }
    }
    else if(param) // parameter in in cyrclass or global namespace
    {
      LOG_DEBUG("Found parameter ", param->name().c_str(), " in global namespace");
      g_parser->globalNamespace().parameters().push_back(param);
    }
    g_parser->cppElem().reset();
  }
}
<FindMembersH>([<>a-z_A-Z~&*.,0-9]+(::)?)+  {
  if(g_parser->cppElem())
  {
    // if class, namespace then set name
    NamespacePtr  nm = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
    if(nm)
      g_parser->cppElem()->name() = yytext;
    else
    {
      LOG_DEBUG("Some string string: ", yytext);
      g_parser->stringVec().push_back(yytext);
    }
  }
  else
  {
    ClassPtr      cl;
    if(g_parser->elemStack().size())
      cl = std::dynamic_pointer_cast<Class>(g_parser->elemStack().top());
    if(cl)
    {
      LOG_TRACE("Fun string in class: ", yytext);
      g_parser->stringVec().push_back(yytext);
    }
    else
    {
      LOG_TRACE("Some text ", yytext);
      g_parser->stringVec().push_back(yytext);
    }
  }
}
<BaseClassSection>[ \t]*","  {
  // after , start new base class
  ClassPtr     cl = std::dynamic_pointer_cast<Class>(g_parser->elemStack().top());
  BaseClassPtr bc = std::dynamic_pointer_cast<BaseClass>(g_parser->cppElem());
  LOG_ASSERT(cl && bc, "Unknown combination for base class");
  cl->baseClasses().push_back(bc);
  LOG_DEBUG("Base class ", bc->name().c_str(), " of class ", cl->name().c_str());
  g_parser->cppElem() = BaseClassPtr(new BaseClass);
  LOG_ASSERT(bool(g_parser->cppElem()), "Incorrect BaseClass pointer");
}
<BaseClassSection>[a-z_A-Z~:.0-9/(/)=\<\>]* {
  g_parser->cppElem()->name() += yytext;
}
<BaseClassSection>[ \t]*public[ \t]*    {  g_parser->setProtection(Protection::Public);         }
<BaseClassSection>[ \t]*protected[ \t]* {  g_parser->setProtection(Protection::Protected);      }
<BaseClassSection>[ \t]*private[ \t]*   {  g_parser->setProtection(Protection::Private);        }
<ConstructorInitailization>[a-z_A-Z~:.0-9/(/)=]+  {
  g_parser->stringVec().push_back(yytext);
}
<ConstructorInitailization>[ \t]*,  {
  FunctionPtr fun = std::dynamic_pointer_cast<Function>(g_parser->cppElem());
  LOG_ASSERT(bool(fun), "Incorrect function pointer");
  fun->constructorInits().push_back(g_parser->createParameter());
}
<ConstructorInitailization>[ \t]*"{" {
  FunctionPtr fun = std::dynamic_pointer_cast<Function>(g_parser->cppElem());
  LOG_ASSERT(bool(fun), "Incorrect function pointer");
  fun->constructorInits().push_back(g_parser->createParameter());
  unput('{');
  BEGIN( FindMembersH );
  /*fun->isImplemented() = true;
  g_parser->brackCount() = 1;
  g_parser->saveLastContext();
  BEGIN( SkipFunDecl);
  g_parser->elemStack().push(g_parser->cppElem());
  g_parser->cppElem().reset();*/
}
<FunParameters>[ \t]*"("  {
  g_parser->brackCount()++;
  g_parser->stringVec().push_back(yytext);
}
<FunParameters>[ \t]*")"  {
  g_parser->brackCount()--;
  if(g_parser->brackCount() == 0)
  {
    FunctionPtr fun = std::dynamic_pointer_cast<Function>(g_parser->cppElem());
    LOG_ASSERT(bool(fun), "Incorrect function pointer");
    // if some text, then create parameter
    if(g_parser->stringVec().size())
      fun->params().push_back(g_parser->createParameter());
    LOG_DEBUG("end function parameters");
    BEGIN( FindMembersH );
  }
  else
    g_parser->stringVec().push_back(yytext);
}
<FunParameters>[*&]*  {
  std::string s = yytext;
  s += " ";
  g_parser->stringVec().push_back(s);
}
<FunParameters>[a-z_A-Z_=::.\<\>0-9']*  {
  g_parser->stringVec().push_back(yytext);
}
<FunParameters>[ \t]*","  {
  FunctionPtr fun = std::dynamic_pointer_cast<Function>(g_parser->cppElem());
  LOG_ASSERT(bool(fun), "Incorrect function pointer");
  fun->params().push_back(g_parser->createParameter());
}
<FindMembersCpp>[ \t]*"("  {
  // this can be start of parameter initialization via constructor or function
  LOG_TRACE("found (, thic can be parameter initailization via constructor");
  g_parser->brackCount() = 1;
  g_parser->stringVec().push_back(yytext);
  BEGIN( ParameterInitializationViaConstructor );
}
<FindMembersCpp>[ \t\r\n]*"{"  {
  // this can be start of namespace - then do nothing,
  // function declaration etc. - skip
  g_parser->lineCount();
  g_parser->stringVec().clear();
  g_parser->cppKey().reset();
  NamespacePtr nm = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
  ParameterPtr pa = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  if(nm)
  {
    LOG_DEBUG("Begin of namespace ", nm->name().c_str());
  }
  else
  {
    LOG_TRACE("found { - this is function implementation");
    g_parser->brackCount() = 1;
    g_parser->saveLastContext();
    BEGIN( SkipFunDecl );
  }
  // put element on stack - just to be consistant with FindMembersH
  g_parser->elemStack().push(g_parser->cppElem());
  g_parser->cppElem().reset();
}
<FindMembersCpp>[ \t\r\n]*"}"  {
  // this can be and of namespace (we are not interested in anything else
  g_parser->lineCount();
  LOG_TRACE("found }");
  LOG_ASSERT(g_parser->elemStack().size()>0, "Incorrect stack size");
  g_parser->cppElem() = g_parser->elemStack().tpop();
  NamespacePtr nm   = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
  if(nm)
  {
    LOG_DEBUG("end namespace ", nm->name().c_str());
  }
  g_parser->cppElem().reset();
}
<FindMembersCpp>[ \t\r\n]*";"   {
  // this can close parameter initialization, not interested in anything else
  g_parser->lineCount();
  LOG_TRACE("found ;");
  g_parser->createParameterInit();
}
<FindMembersCpp>[<>a-z_A-Z~&*:.,0-9=]+  {
  // this can be anything (we save in case of parameter)
  if(g_parser->cppElem())
  {
    // if class, namespace then set name
    NamespacePtr  nm = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
    if(nm)
      g_parser->cppElem()->name() = yytext;
    else
    {
      LOG_TRACE("Some element string: ", yytext);
      g_parser->stringVec().push_back(yytext);
    }
  }
  else
  {
    LOG_TRACE("Some text ", yytext);
    g_parser->stringVec().push_back(yytext);
  }
}
<ParameterInitializationViaConstructor>[ \t]*"("  {
  g_parser->brackCount()++;
  g_parser->stringVec().push_back(yytext);
}
<ParameterInitializationViaConstructor>[ \t]*")"  {
  g_parser->brackCount()--;
  g_parser->stringVec().push_back(yytext);
  if(g_parser->brackCount() == 0)
  {
    LOG_TRACE("end parameter initialization");
    BEGIN( FindMembersCpp );
  }
}
<ParameterInitializationViaConstructor>[\*\&\,a-z_A-Z_=::.\<\>0-9'\"]*  {
  g_parser->stringVec().push_back(yytext);
}

<ParameterInitializationViaConstructor>([_a-zA-Z0-9: ]+)[ ]([_a-zA-Z0-9]+)::([_a-zA-Z0-9]+)[ ]\((.*)\)  {

}
<SkipFunDecl>[ \t\r\n]*"{"  {
  g_parser->brackCount()++;
  g_parser->lineCount();
}
<SkipFunDecl>[ \t\r\n]*"}"  {
  g_parser->brackCount()--;
  g_parser->lineCount();
  if(g_parser->brackCount() == 0)
  {
    g_parser->checkFunctionEnd(true);
  }
}
<TemplateSection>[a-z_A-Z~:.0-9]+  {
  g_parser->stringVec().push_back(yytext);
}
<TemplateSection>[ \t\r\n]*","  {
  g_parser->lineCount();
  TemplatePtr tem = std::dynamic_pointer_cast<Template>(g_parser->cppElem());
  LOG_ASSERT(bool(tem), "Incorrect template pointer");
  tem->params().push_back(g_parser->createParameter());
}
<TemplateSection>[ \t\r\n]*"<"  {
  g_parser->lineCount();
  g_parser->brackCount()++;
  g_parser->stringVec().push_back("<");
}
<TemplateSection>[ \t\r\n]*">"  {
  g_parser->brackCount()--;
  g_parser->lineCount();
  if(g_parser->brackCount() == 0)
  {
    TemplatePtr tem = std::dynamic_pointer_cast<Template>(g_parser->cppElem());
    LOG_ASSERT(bool(tem), "Incorrect template pointer");
    tem->params().push_back(g_parser->createParameter());
    g_parser->preElemList().push_back(g_parser->cppElem());
    g_parser->cppElem().reset();
    BEGIN( FindMembersH );
  }
  else
    g_parser->stringVec().push_back(">");
}
<UsingNamespaceStart>[a-z_A-Z:.0-9]+  {
  NamespacePtr nm = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
  LOG_ASSERT(bool(nm), "Incorrect namespace pointer");
  nm->name() = yytext;
  BEGIN( FinishLine );
}
<IncludeStart>[a-z_A-Z:.0-9\/\\<>\"]+  {
  IncludePtr inc = std::dynamic_pointer_cast<Include>(g_parser->cppElem());
  LOG_ASSERT(bool(inc), "Incorrect include pointer");
  inc->name() = yytext;
  BEGIN( FinishLine );
}
<DefineStart>[a-z_A-Z:.0-9]+  {
  DefinePtr def = std::dynamic_pointer_cast<Define>(g_parser->cppElem());
  LOG_ASSERT(bool(def), "Incorrect define pointer");
  def->name() = yytext ;
  BEGIN( DefineEnd ) ;
}
<DefineEnd>\n {
  DefinePtr def = std::dynamic_pointer_cast<Define>(g_parser->cppElem());
  if(def)
    g_parser->globalNamespace().defines().push_back(def);

  g_parser->cppElem().reset();
  g_parser->yyLineNr()++;
  g_parser->restoreLastContext();
}
<DefineEnd>[^\n]*  {
  DefinePtr def = std::dynamic_pointer_cast<Define>(g_parser->cppElem());
  LOG_ASSERT(bool(def), "Incorrect define pointer");
  def->value() += yytext ;
}
<DefineEnd>.*"\\"[\n]  {
  DefinePtr def = std::dynamic_pointer_cast<Define>(g_parser->cppElem());
  LOG_ASSERT(bool(def), "Incorrect define pointer");
  def->value() += yytext ;
}
<FinishLine>.*\n  {
  IncludePtr inc = std::dynamic_pointer_cast<Include>(g_parser->cppElem());
  if(inc)
    g_parser->globalNamespace().includes().push_back(inc);

  NamespacePtr nm = std::dynamic_pointer_cast<Namespace>(g_parser->cppElem());
  if(nm)
    g_parser->globalNamespace().usedNamespaces().push_back(nm);

  g_parser->cppElem().reset();
  g_parser->yyLineNr()++;
  g_parser->restoreLastContext();
}
<TypedefsDef>[a-z_A-Z:0-9<>]*  {
  LOG_DEBUG("typedef text", yytext);
  TypedefPtr t = std::dynamic_pointer_cast<Typedef>(g_parser->cppElem());
  LOG_ASSERT(bool(t), "invalid typedef pointer");
  if(t->type().size())
    t->type() += " ";
  t->type() += t->name();
  t->name() = yytext;
}
<TypedefsDef>";"  {
  LOG_DEBUG("typedef end");
  TypedefPtr t = std::dynamic_pointer_cast<Typedef>(g_parser->cppElem());
  LOG_ASSERT(bool(t), "invalid typedef pointer");
  GlobalNamespace::typedefs().push_back(t);
  g_parser->cppElem().reset();
  g_parser->restoreLastContext();
}
<FindMembersH>^[ \t]*"#"   { 
  LOG_DEBUG("ignore compiler directive" ) ;
  g_parser->saveLastContext();
  BEGIN( FinishLine ) ;
}
<FindMembersH>[ \t\r\n]*"/*""*"+"/"  { g_parser->lineCount() ; }
<ParameterParser>"="  {
  g_parser->brackCount() = 0xFF;
  BEGIN( ParameterParserInit );
}
<ParameterParser>"("  {
  g_parser->brackCount() = 1;
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  LOG_ASSERT(bool(p), "Invalid parameter pointer");
  p->initViaConstructor() = true;
  BEGIN( ParameterParserInit );
}
<ParameterParser>[*&]  {
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  LOG_ASSERT(bool(p), "Invalid parameter pointer");
  if(p->type().size() && p->name().size())
    p->type() += " ";
  p->type() += p->name();
  if(p->type().size())
    p->type() += " ";
  p->type() += yytext;
  p->name() = "";
}
<ParameterParser>[a-z_A-Z\<\>:.0-9]+  {
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  LOG_ASSERT(bool(p), "Invalid parameter pointer");
  if(p->type().size() && p->name().size())
    p->type() += " ";
  p->type() += p->name();
  p->name() = yytext;
}
<ParameterParserInit>"("  {
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  LOG_ASSERT(bool(p), "Invalid parameter pointer");
  g_parser->brackCount()++;
  p->defaultValue() += yytext;
}
<ParameterParserInit>")"  {
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  LOG_ASSERT(bool(p), "Invalid parameter pointer");
  if(g_parser->brackCount())
    g_parser->brackCount()--;
  if(g_parser->brackCount()>0)
    p->defaultValue() += yytext;
}
<ParameterParserInit>[a-z_A-Z\<\>*&:.0-9'\"]+  {
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  LOG_ASSERT(bool(p), "Invalid parameter pointer");
  p->defaultValue() += yytext;
}
<FunctionNameParser>[~a-z_A-Z\<\>*&:\.,0-9!%\[\]+-=\(\)]+  {
  // cppElem is parameter, oon top of stack is function
  LOG_ASSERT(g_parser->elemStack().size()>0, "incorrect stack size in FunctionNameParser");
  ParameterPtr p = std::dynamic_pointer_cast<Parameter>(g_parser->cppElem());
  FunctionPtr  f = std::dynamic_pointer_cast<Function>(g_parser->elemStack().top());
  LOG_ASSERT(bool(f), "incorrect pointer type in FunctionNameParser");
  if(g_parser->conversion())
  {
    if(p->type().size())
      p->type() += " ";
    p->type() += yytext;
  }
  else
  {
    if(p->type().size())
      p->type() += " ";
    p->type() += f->name();
    f->name() = yytext;
  }
}
<SkipComment>"/*"
<SkipComment>"*/"  {
  g_parser->restoreLastContext();
}
<*>[ \t\r\n]*"//"[^/][^\n]*\n   { g_parser->lineCount(); }
<*>[ \t\r\n]*"///""/"+[^\n]*\n  { g_parser->lineCount(); }
<*>"(?:/\\*(?:[^*]|(?:\\*+[^*/]))*\\*+/)|(?://.*)"  {
  LOG_DEBUG("skip comment ", yytext);
  g_parser->lineCount();
}
<*>"/*"  {
   //g_parser->brackCount() = 1;
   g_parser->saveLastContext();
   BEGIN( SkipComment );
}
<*>.
<SkipComment>[^\*\n]+ { g_parser->lineCount(); }
<*>\n                 { g_parser->yyLineNr()++ ; }
<*>\r\n               { g_parser->yyLineNr()++ ; }


%%


/*@ ----------------------------------------------------------------------------
 */



extern "C" 
{
  int yywrap()
  {
    return g_parser->yywrap();
  }
};
