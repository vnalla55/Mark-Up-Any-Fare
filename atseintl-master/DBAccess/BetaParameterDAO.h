#pragma once

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DataAccessObject.h"

#include <fstream>
#include <iostream>

namespace tse
{
typedef HashKey<int, int, char, char> ParameterBetaKey2;
typedef sfc::Cache<ParameterBetaKey2, Beta> BetaCache;

class BetaParameterDAO : public DataAccessObject<ParameterBetaKey2, Beta>
{

public:
  friend class DAOHelper<BetaParameterDAO>;
  static BetaParameterDAO& instance();

  const Beta* get(DeleteList& del,
                  const int& timeDiff,
                  const int& mileage,
                  const char& direction,
                  const char& APSatInd);
  virtual ~BetaParameterDAO();
  void load() override;

protected:
  BetaParameterDAO(int cacheSize = 0, const std::string& cacheType = "");
  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<BetaParameterDAO> _helper;
  static log4cxx::LoggerPtr _logger;
  static bool _loadedData;
  virtual void destroy(ParameterBetaKey2, Beta*) override;

private:
  static BetaParameterDAO* _instance;
  bool parse(const std::string& str);
  bool parse(const std::vector<std::string>& elem);
  bool checkStr(const std::vector<std::string>& elem, const int idx, const std::string& str);
  template <typename T>
  bool check(const std::vector<std::string>& elem, const int idx, T& num);
};
}

