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
  class RecInfo
  {
  public:
    RecInfo(const DBT& dbKey, const DBT& dbData);

    RecInfo(const RecInfo& ref);

    RecInfo& operator=(const RecInfo& ref);

    void initData(const DBT& dbKey, const DBT& dbData);

    void print(bool doCR = false);
    void print2();

    DBT getKey() { return dbkey; }

    DBT data;
    DBT dbkey;

    char* dataPtr;
    char* keyPtr;

#pragma pack(1)
    struct BlobHeader
    {
      uint32_t headerSize;
      uint32_t unCompressedSize;
      char compressed;
      time_t timestamp;
      unsigned char sha1[20];
      char dataFormat;
    }* header;
#pragma pack()

    ~RecInfo();

  private:
    RecInfo();
  };

  LDCUtil(std::string& dirname, bool verboseFlag = false);

  ~LDCUtil()
  {
    closeDB();
    closeEnvironment();
  }

  bool openDB(std::string& filename);

  bool isValid() { return dbEnv != nullptr; }

  void listKeys();

  void getKeyList(std::vector<RecInfo>& list);

  bool read(RecInfo& key, bool verbose);

private:
  DB_ENV* dbEnv;
  DB* db;
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

  void closeDB()
  {
    if (db)
    {
      db->close(db, 0);
      db = nullptr;
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
