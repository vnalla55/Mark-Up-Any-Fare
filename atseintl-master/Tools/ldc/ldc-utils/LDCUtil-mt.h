#ifndef LDCUtil__H
#define LDCUtil__H

#include <db.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

class LDCUtil
{
public:
  LDCUtil(std::string& dirname, bool verboseFlag = false);

  ~LDCUtil() { closeEnvironment(); }

  DB* openDB(std::string& filename);

  bool isValid() { return dbEnv != nullptr; }

  void closeDB(DB* db) { db->close(db, 0); }

private:
  DB_ENV* dbEnv;
  std::string directory;
  uint32_t berkeleyDBPageSize;
  bool verbose;

  bool createEnvironment();

  void closeEnvironment()
  {
    if (dbEnv)
    {
      dbEnv->close(dbEnv, 0);
      dbEnv = nullptr;
    }
  }
  LDCUtil();
  LDCUtil(LDCUtil& ref);
  LDCUtil& operator=(LDCUtil& ref);

  enum DataFormat
  {
    DATAFMT_BIN = 0,
    DATAFMT_TXT,
    DATAFMT_XML,
    DATAFMT_BN2,
    DataFormat_END // Keep this last!
  };
};

#endif
