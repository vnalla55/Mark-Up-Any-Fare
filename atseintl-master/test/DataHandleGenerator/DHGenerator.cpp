#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "test/DataHandleGenerator/DHGenerator.h"
#include "test/DataHandleGenerator/DHHeaderParser.h"
#include "test/DataHandleGenerator/DataHandleDataMap.h"
#include "test/DataHandleGenerator/Config.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#if BOOST_FILESYSTEM_VERSION != 2
#include <boost/filesystem/operations.hpp>
#endif

std::multimap<std::string, DataHandleData> DHGenerator::_dataHandleDataMap;
std::map<Function*, std::string> DHGenerator::_accessorMap;
bool DHGenerator::_createAccessors = false;
std::set<std::string> DHGenerator::_factoryIncludes;
log4cxx::LoggerPtr
DHGenerator::_logger(log4cxx::Logger::getLogger("DataHandleParser.DHGenerator"));
bool DHGenerator::_accessorCreated = false;

DHGenerator::DHGenerator(const Config& cfg) : _config(cfg)
{
  _createAccessors = _config.CreateAccessors();
}
bool
DHGenerator::generateMock()
{
  if (!_config.CreateDHMockH() && !_config.CreateDHMockCpp() && !_config.CreateDHCpp())
    return true;

  return parseDataHandleDataCfg() && parseDataHandleCpp() && parseDataHandleHeader() &&
         parseTseTypes() && createMockHeader() && adoptHeaderForCpp() && createMockCpp() &&
         createCpp();
}
bool
DHGenerator::parseDataHandleDataCfg()
{
  DataHandleDataMapParser parser(_dataHandleDataMap, _factoryIncludes, _ignores);
  // if not initialize, do nothing
  if (!parser.initialize())
    return false;
  return parser.Parse(_config.XMLDataSource().c_str());
}
bool
DHGenerator::parseDataHandleHeader()
{
  std::string fileName = _config.DBAccessPath() + _config.DataHandleClassName() + ".h";
  LOG4CXX_DEBUG(_logger, "Trying to parse header: " << fileName);
  boost::filesystem::path path(fileName);
  boost::filesystem::ifstream file;
  file.open(path);
  if (!file.is_open())
  {
    LOG4CXX_ERROR(_logger, "Unable to open Header file.");
    return false;
  }
  // Read the whole file into a stringstream
  std::stringstream ss;
  ss << file.rdbuf();
  DHHeaderParser parser(_namespaceHeader);
  return parser.parseH(ss.str().c_str());
}
bool
DHGenerator::parseDataHandleCpp()
{
  std::string fileName = _config.DBAccessPath() + _config.DataHandleClassName() + ".cpp";
  LOG4CXX_DEBUG(_logger, "Trying to parse cpp file: " << fileName);
  boost::filesystem::path path(fileName);
  boost::filesystem::ifstream file;
  file.open(path);
  if (!file.is_open())
  {
    LOG4CXX_ERROR(_logger, "Unable to open Cpp file.");
    return false;
  }
  // Read the whole file into a stringstream
  std::stringstream ss;
  ss << file.rdbuf();
  DHHeaderParser parser(_namespaceCpp);
  return parser.parseCpp(ss.str().c_str());
}
bool
DHGenerator::parseTseTypes()
{
  // TseTypes is additional, can be skipped
  std::string fileName = _config.TseTypesFile();
  LOG4CXX_DEBUG(_logger, "Trying to parse TseTypes file: " << fileName);
  boost::filesystem::path path(fileName);
  boost::filesystem::ifstream file;
  file.open(path);
  if (!file.is_open())
  {
    LOG4CXX_ERROR(_logger, "Unable to open TseTypes file.");
    return true;
  }
  // Read the whole file into a stringstream
  std::stringstream ss;
  ss << file.rdbuf();
  DHHeaderParser parser(_namespaceHeader);
  return parser.parseTseTypes(ss.str().c_str());
}
bool
DHGenerator::createMockHeader()
{
  std::string fileName = _config.OutputDir() + _config.MockClassName() + ".h";
  if (_config.CreateDHMockH())
    LOG4CXX_DEBUG(_logger, "Trying to create mock header: " << fileName);

  boost::filesystem::path path(fileName);
  boost::filesystem::ofstream file;
  if (_config.CreateDHMockH())
  {
    file.open(path, std::ios_base::out | std::ios_base::trunc);
    if (!file.is_open())
    {
      LOG4CXX_ERROR(_logger, "Unable to create mock header file " << fileName);
      return false;
    }
  }
  // remove unwanted functions
  std::map<std::string, std::set<std::string> >::iterator ignt = _ignores.begin();
  std::map<std::string, std::set<std::string> >::iterator igne = _ignores.end();
  for (; ignt != igne; ignt++)
    _namespaceHeader.remove(ignt->second, ignt->first);

  if (_config.CreateDHMockH())
  {
    // replace DataHandle with DataHandleMocek
    _namespaceHeader.rename(_config.DataHandleClassName(), _config.MockClassName(), "Class");

    file << "#ifndef DATAHANDLEMOCK_H\n#define DATAHANDLEMOCK_H\n\n";
    std::string prefix = "";
    _namespaceHeader.printMockHeader(file, prefix, 0);
    file << "#endif\n";

    // restore original names
    _namespaceHeader.rename(_config.MockClassName(), _config.DataHandleClassName(), "Class");
  }
  return true;
}
bool
DHGenerator::createMockCpp()
{
  if (!_config.CreateDHMockCpp())
    return true;

  std::string fileName = _config.OutputDir() + _config.MockClassName() + ".cpp";
  LOG4CXX_DEBUG(_logger, "Trying to DataHandle mock file: " << fileName);
  boost::filesystem::path path(fileName);
  boost::filesystem::ofstream file;
  file.open(path, std::ios_base::out | std::ios_base::trunc);
  if (!file.is_open())
  {
    LOG4CXX_ERROR(_logger, "Unable to create DataHandle mock file " << fileName);
    return false;
  }
  // replace DataHandle with DataHandleMocek
  _namespaceHeader.rename(_config.DataHandleClassName(), _config.MockClassName(), "Class");
  _namespaceHeader.rename(_config.DataHandleClassName(), _config.MockClassName(), "ParameterInit");

  std::string prefix = "";
  _namespaceHeader.printMockCpp(file, prefix, 0);
  // restore original names
  _namespaceHeader.rename(_config.MockClassName(), _config.DataHandleClassName(), "Class");
  _namespaceHeader.rename(_config.MockClassName(), _config.DataHandleClassName(), "ParameterInit");

  return true;
}
bool
DHGenerator::createCpp()
{
  if (!_config.CreateDHCpp())
    return true;

  std::string fileName = _config.OutputDir() + _config.DataHandleClassName() + ".cpp";
  LOG4CXX_DEBUG(_logger, "Trying to DataHandle.cpp implementation file: " << fileName);
  boost::filesystem::path path(fileName);
  boost::filesystem::ofstream file;
  file.open(path, std::ios_base::out | std::ios_base::trunc);
  if (!file.is_open())
  {
    LOG4CXX_ERROR(_logger, "Unable to create DataHandle.cpp implementation file " << fileName);
    return false;
  }
  std::string prefix = "";
  _namespaceHeader.printDHCpp(file, prefix, 0);
  return true;
}
void
DHGenerator::getMockFunImplFromXML(const Function* fun, std::ofstream& file, int align)
{
  std::pair<std::multimap<std::string, DataHandleData>::iterator,
            std::multimap<std::string, DataHandleData>::iterator> ret =
      _dataHandleDataMap.equal_range(fun->name());
  for (std::multimap<std::string, DataHandleData>::iterator it = ret.first; it != ret.second; it++)
  {
    LOG4CXX_DEBUG(_logger, "Generating implementation for function " << fun->name());
    DataHandleData& data = (*it).second;
    bool fMatch = true;
    std::vector<std::pair<std::string, std::string> >::const_iterator ipt = data._params.begin();
    std::vector<std::pair<std::string, std::string> >::const_iterator ipe = data._params.end();
    // temporary string
    std::stringstream sstr;
    while (ipt != ipe)
    {
      if (ipt == data._params.begin())
        sstr << std::setw(align) << ' ' << "if(";
      // check if parameter match and get type
      PtrList<Parameter>::const_iterator ppt = fun->params().begin();
      PtrList<Parameter>::const_iterator ppe = fun->params().end();
      ParameterPtr p;
      for (; (ppt != ppe) && !p; ppt++)
      {
        if ((*ppt)->name() == (*ipt).first)
          p = *ppt;
      }
      if (!p)
      {
        LOG4CXX_ERROR(_logger,
                      "Unable to find parameter " << (*ipt).first << " in function " << fun->name()
                                                  << " parameter list.");
        fMatch = false;
        break;
      }

      sstr << "(" << (*ipt).first << (*ipt).second << " )";
      ipt++;
      if (ipt != ipe)
        sstr << "&&";
      else
        sstr << ")" << std::endl;
    }
    if (fMatch)
    {
      sstr << std::setw(align) << "{" << std::endl << std::setw(align + 4);
      if (data._return.size())
        sstr << data._return << std::endl << std::setw(align) << "}" << std::endl;
      file << sstr.str();
    }
  }
}
void
DHGenerator::ClassFinder::loadFile(const std::string& filename, std::string& s)
{
  std::ifstream is(filename.c_str());
  if (is.bad())
    return;
  s.reserve(is.rdbuf()->in_avail());
  char c;
  while (is.get(c))
  {
    if (s.capacity() == s.size())
      s.reserve(s.capacity() * 3);
    s.append(1, c);
  }
  is.close();
}
void
DHGenerator::ClassFinder::findClasses(const std::string& path, const std::string& filename)
{
  std::string file;
  loadFile(path + filename, file);

  boost::match_results<std::string::const_iterator> what;
  std::string::const_iterator start = file.begin(), end = file.end();
  boost::match_flag_type flags = boost::match_default;
  while (boost::regex_search(start, end, what, _expression, flags))
  {
    std::map<std::string, std::string>::iterator it =
        _classToFile.find(std::string(what[1].first, what[1].second));
    if (it != _classToFile.end())
      it->second = filename;
    start = what[0].second;
    flags |= boost::match_prev_avail;
    flags |= boost::match_not_bob;
  }
}
DHGenerator::ClassFinder::ClassFinder(std::map<std::string, std::string>& classToFile)
  : _expression("class\\s*([a-zA-Z0-9_]*)(\\s*:?\\s*(public|protected|private)\\s*[a-zA-Z0-9_\\<\\>"
                "]*\\s*,?)*\\s*\\{"),
    _classToFile(classToFile)
{
}
bool
DHGenerator::adoptHeaderForCpp()
{
  boost::filesystem::path path(_config.DBAccessPath());
  if (!boost::filesystem::exists(path))
  {
    LOG4CXX_ERROR(_logger, "DBAccess path don't exist");
    return false;
  }

  try
  {
    // get list of header files in DBAccess directory (skip DAO headers)
    std::set<std::string> header_files;

    boost::filesystem::directory_iterator dir_end;
    boost::regex expr("[a-zA-Z0-9]*(?<!DAO)\\.h");
    boost::cmatch what;

    for (boost::filesystem::directory_iterator dir(path); dir != dir_end; dir++)
    {
      if (boost::filesystem::is_directory(dir->status()))
        continue;
#if BOOST_FILESYSTEM_VERSION == 2
      if (!boost::regex_match(dir->filename().c_str(), what, expr))
        continue;

      header_files.insert(dir->filename());
#else
      if (!boost::regex_match(dir->path().filename().c_str(), what, expr))
        continue;

      header_files.insert(dir->path().filename().c_str());
#endif
    }

    // create map of forward declaraion classes to include file
    std::map<std::string, std::string> _classToInclude;
    PtrList<Class>::iterator ift = _namespaceHeader.fwrdDecls().begin();
    PtrList<Class>::iterator ife = _namespaceHeader.fwrdDecls().end();
    for (; ift != ife; ift++)
      _classToInclude.insert(std::make_pair((*ift)->name(), ""));
    PtrList<Namespace>::iterator innt = _namespaceHeader.namespaces().begin();
    PtrList<Namespace>::iterator inne = _namespaceHeader.namespaces().end();
    for (; innt != inne; innt++)
    {
      ift = (*innt)->fwrdDecls().begin();
      ife = (*innt)->fwrdDecls().end();
      for (; ift != ife; ift++)
        _classToInclude.insert(std::make_pair((*ift)->name(), ""));
    }

    // now grep each header and fill the map with include filename
    std::set<std::string>::iterator iht = header_files.begin();
    std::set<std::string>::iterator ihe = header_files.end();
    ClassFinder classFinder(_classToInclude);
    for (; iht != ihe; iht++)
      classFinder.findClasses(_config.DBAccessPath(), *iht);

    // for each element on the map create ne include element
    _namespaceHeader.includes().clear();
    std::map<std::string, std::string>::iterator it = _classToInclude.begin();
    std::map<std::string, std::string>::iterator ie = _classToInclude.end();
    for (; it != ie; it++)
    {
      if ((*it).second.size() > 0)
      {
        IncludePtr inc = IncludePtr(new Include);
        inc->name() = "\"" + (*it).second + "\"";
        _namespaceHeader.includes().push_back(inc);
      }
    }
    // add includes from cpp
    PtrList<Include>::iterator iict = _namespaceCpp.includes().begin();
    PtrList<Include>::iterator iice = _namespaceCpp.includes().end();
    for (; iict != iice; iict++)
    {
      PtrList<Include>::iterator iiht = _namespaceHeader.includes().begin();
      PtrList<Include>::iterator iihe = _namespaceHeader.includes().end();
      bool exist = false;
      for (; iiht != iihe; iiht++)
      {
        if ((*iiht)->name() == (*iict)->name())
        {
          exist = true;
          break;
        }
      }
      if (!exist)
        _namespaceHeader.includes().push_back(*iict);
    }

    // add include factories
    std::set<std::string>::const_iterator iib = _factoryIncludes.begin(),
                                          iie = _factoryIncludes.end();
    for (; iib != iie; iib++)
    {
      IncludePtr i(new Include);
      i->name() = "\"" + *iib + "\"";
      _namespaceHeader.includes().push_back(i);
    }

    // copy used namespaces and parameter initialization
    _namespaceHeader.usedNamespaces() = _namespaceCpp.usedNamespaces();
    _namespaceHeader.paramInit() = _namespaceCpp.paramInit();
    PtrList<Namespace>::iterator inb = _namespaceCpp.namespaces().begin();
    PtrList<Namespace>::iterator ine = _namespaceCpp.namespaces().end();
    for (; inb != ine; inb++)
    {
      PtrList<Namespace>::iterator inhb = _namespaceHeader.namespaces().begin();
      PtrList<Namespace>::iterator inhe = _namespaceHeader.namespaces().end();
      for (; inhb != inhe; inhb++)
      {
        if ((*inhb)->name() == (*inb)->name())
        {
          (*inhb)->paramInit() = (*inb)->paramInit();
          break;
        }
      }
    }
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Exception caught " << e.what());
    return false;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "Unknown Exception caught");
    return false;
  }
  return true;
}
void
DHGenerator::createDHAccessorMap(const Class* cl)
{
  if (_accessorCreated)
    return;
  // itrerate by all function and check if each function name is unique
  PtrList<Function>::const_iterator it = cl->functions().begin();
  PtrList<Function>::const_iterator ie = cl->functions().end();
  std::map<std::string, bool> funNameUnique;
  std::map<std::string, bool>::iterator mapElem;
  for (; it != ie; it++)
  {
    // skip non returning functions
    if ((*it)->returnType()->type() == "void")
      continue;
    mapElem = funNameUnique.find((*it)->name());
    if (mapElem == funNameUnique.end())
      funNameUnique.insert(std::make_pair((*it)->name(), true));
    else
      mapElem->second = false;
  }
  // generate accessors
  it = cl->functions().begin();
  for (; it != ie; it++)
  {
    // skip implemented function, constructor, destructor, operator
    if ((*it)->isConstructor() || (*it)->isDestructor() || (*it)->isOperator() ||
        (*it)->isImplemented())
      continue;
    // skip non returning functions
    if ((*it)->returnType()->type() == "void")
      continue;

    // if unique name
    if (funNameUnique.find((*it)->name())->second)
    {
      _accessorMap.insert(std::make_pair(it->get(), (*it)->name()));
    }
    else
    {
      std::string name;
      name += (*it)->name();
      PtrList<Parameter>::const_iterator ipt = (*it)->params().begin();
      PtrList<Parameter>::const_iterator ipe = (*it)->params().end();
      for (; ipt != ipe; ipt++)
        for (size_t i = 0; i < (*ipt)->type().length(); i++)
          if (isalpha((*ipt)->type()[i]))
            name += (*ipt)->type()[i];

      _accessorMap.insert(std::make_pair(it->get(), name));
    }
  }
  _accessorCreated = true;
}
