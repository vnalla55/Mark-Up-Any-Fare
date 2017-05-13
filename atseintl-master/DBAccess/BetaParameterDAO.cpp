#include "DBAccess/BetaParameterDAO.h"

#include "Common/Global.h"
#include "Common/TseUtil.h"

namespace tse
{

log4cxx::LoggerPtr
BetaParameterDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BetaParameterDAO"));

std::string
BetaParameterDAO::_name("BetaParameter");

std::string
BetaParameterDAO::_cacheClass("Beta");

DAOHelper<BetaParameterDAO>
BetaParameterDAO::_helper(_name);

BetaParameterDAO* BetaParameterDAO::_instance = nullptr;

bool BetaParameterDAO::_loadedData = false;

BetaParameterDAO::BetaParameterDAO(int cacheSize, const std::string& cacheType)
  : DataAccessObject<ParameterBetaKey2, Beta>(cacheSize, cacheType)
{
}

const Beta*
getParameterBetaData(DeleteList& del,
                     const int& timeDiff,
                     const int& mileage,
                     const char& direction,
                     const char& APSatInd)
{
  BetaParameterDAO& dao = BetaParameterDAO::instance();
  return dao.get(del, timeDiff, mileage, direction, APSatInd);
}

void
BetaParameterDAO::destroy(ParameterBetaKey2 key, Beta* t)
{
  LOG4CXX_DEBUG(_logger, "BetaParameterDAO::destroy");
  if (t)
    delete t;
}

BetaParameterDAO::~BetaParameterDAO() {}

BetaParameterDAO&
BetaParameterDAO::instance()
{
  LOG4CXX_DEBUG(_logger, "BoundFareDAO::instance");
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Beta*
BetaParameterDAO::get(DeleteList& del,
                      const int& timeDiff,
                      const int& mileage,
                      const char& direction,
                      const char& APSatInd)
{
  const Beta* ret = nullptr;
  if (_loadedData)
  {
    ParameterBetaKey2 key(timeDiff, mileage, direction, APSatInd);
    DAOCache::pointer_type ptr = cache().getIfResident(key);
    if (ptr)
      ret = &*ptr;
  }

  return ret;
}

void
BetaParameterDAO::load()
{
  if (_loadedData)
    return;

  char esvLogic = 'N';
  Global::config().getValue("ESV_LOGIC", esvLogic, "SHOPPING_OPT");
  if (esvLogic != 'Y')
    return;

  try
  {
    std::string filename = "";
    Global::config().getValue("BETA_FILE_PATH", filename, "SHOPPING_OPT");
    if (filename.empty())
      throw std::runtime_error("Path to Beta file not specified");

    LOG4CXX_INFO(_logger, "Starting loading Beta file: " << filename);
    const int lineSize = 512;
    std::ifstream fs;
    char line[lineSize];
    fs.open(filename.c_str());
    if (fs.fail())
    {
      std::string msg("Unable to open Beta file ");
      msg.append(filename);
      throw std::runtime_error(msg);
    }

    int lineNum = 0;
    int correctLines = 0;
    while (true)
    {
      fs.getline(line, lineSize);
      if (fs.eof())
        break;
      if (fs.fail())
      {
        fs.close();
        throw std::runtime_error("Error reading Beta file");
      }
      std::string lineStr(line);
      if (lineStr != "")
        ++lineNum;
      if (!parse(lineStr))
      {
        LOG4CXX_WARN(_logger, "BetaParameterDAO::load incorrect line number " << lineNum);
      }
      else
      {
        ++correctLines;
      }
    }
    fs.close();
    _loadedData = true;
    LOG4CXX_INFO(_logger,
                 "End loading Beta file. Read correct lines: " << correctLines
                                                               << " of total lines: " << lineNum);
  }
  catch (const std::exception& e)
  {
    TseUtil::alert(e.what());
    LOG4CXX_ERROR(_logger, e.what());
    throw;
  }
}

bool
BetaParameterDAO::parse(const std::string& str)
{
  std::vector<std::string> elem;
  boost::char_separator<char> separator(" ");
  boost::tokenizer<boost::char_separator<char> > tokenizer(str, separator);
  boost::tokenizer<boost::char_separator<char> >::iterator token = tokenizer.begin();
  boost::tokenizer<boost::char_separator<char> >::iterator tokenEnd = tokenizer.end();
  for (; token != tokenEnd; ++token)
  {
    elem.push_back(token->data());
  }

  return parse(elem);
}

bool
BetaParameterDAO::parse(const std::vector<std::string>& elem)
{
  const int betaKeySize = 4;
  const int betaVecSize = 13;
  bool elemValid = true;
  int num0 = 0;
  int num1 = 0;
  std::vector<double> doubleVec(betaVecSize);
  if (elem.size() != betaKeySize + betaVecSize)
  {
    elemValid = false;
    std::stringstream warnMsg;
    warnMsg << "BetaParameterDAO::parse : incorrect number or elems in line " << elem.size();
    LOG4CXX_WARN(_logger, warnMsg.str());
  }
  else
  {
    // reading key
    elemValid = check(elem, 0, num0);
    elemValid = check(elem, 1, num1);
    elemValid = checkStr(elem, 2, "IO");
    elemValid = checkStr(elem, 3, "FC");

    // reading Beta
    for (int i = 0; i < betaVecSize; ++i)
    {
      elemValid = check(elem, i + betaKeySize, doubleVec[i]);
    }
  }

  if (elemValid)
  {
    // add elem to cache
    ParameterBetaKey2 key(num0, num1, *(elem[2].c_str()), *(elem[3].c_str()));
    cache().getCacheImpl()->put(key, new std::vector<double>(doubleVec), false);
  }

  return elemValid;
}

bool
BetaParameterDAO::checkStr(const std::vector<std::string>& elem,
                           const int idx,
                           const std::string& str)
{
  return (elem[idx].size() == 1) && (str.find(elem[idx]) != std::string::npos);
}

template <typename T>
bool
BetaParameterDAO::check(const std::vector<std::string>& elem, const int idx, T& num)
{
  std::istringstream iss(elem[idx]);
  bool res = !(iss >> num).fail();
  if (!res)
  {
    std::stringstream warnMsg;
    warnMsg << "BetaParameterDAO::check incorrect elem idx: " << idx;
    LOG4CXX_WARN(_logger, warnMsg.str());
  }

  return res;
}

} // namespace tse
