#include "Tools/ldc/ldc-utils/LDCUtil.h"
#include "Tools/ldc/ldc-utils/mytimes.h"

#include <cstring>

LDCUtil::LDCUtil(std::string& dirname, bool verboseFlag)
  : dbEnv(nullptr), db(nullptr), directory(dirname), berkeleyDBPageSize(4096), verbose(verboseFlag)
{
  createEnvironment();
}

bool
LDCUtil::createEnvironment()
{
  bool retval = false;

  if (verbose)
    fprintf(stderr, "create environment\n");
  if (db_env_create(&dbEnv, 0) == 0)
  {
    if (verbose)
      fprintf(stderr, "open environment\n");
    if (dbEnv->open(
            dbEnv, directory.c_str(), DB_CREATE | DB_INIT_CDB | DB_INIT_MPOOL | DB_THREAD, 0) == 0)
    {
      retval = true;
    }
    else
    {
      fprintf(stderr, "Unable to open Berkeley DB Environment\n");
    }
  }
  else
  {
    fprintf(stderr, "Unable to create Berkeley DB Environment\n");
  }

  if (!retval)
  {
    closeEnvironment();
  }
  return retval;
}

bool
LDCUtil::openDB(std::string& filename)
{
  bool retval = false;

  if (dbEnv)
  {
    closeDB();

    if (verbose)
      fprintf(stderr, "create db\n");
    if (db_create(&db, dbEnv, 0) == 0)
    {
      if (verbose)
        fprintf(stderr, "set page size\n");
      if (db->set_pagesize(db, berkeleyDBPageSize) == 0)
      {
        if (verbose)
          fprintf(stderr, "open db\n");
        if (db->open(db, nullptr, filename.c_str(), nullptr, DB_BTREE, DB_THREAD, 0) == 0)
        {
          retval = true;
        }
        else
        {
          fprintf(stderr, "Unable to open Berkeley DB [%s]\n", filename.c_str());
        }
      }
      else
      {
        fprintf(stderr, "Unable to set page size DB [%s]\n", filename.c_str());
      }
    }
    else
    {
      fprintf(stderr, "Unable to create Berkeley DB [%s]\n", filename.c_str());
    }

    if (!retval)
    {
      closeDB();
    }
  }
  else
  {
    fprintf(stderr, "No Berkeley DB Environment\n");
  }
  return retval;
}


void
LDCUtil::listKeys()
{
  if (verbose)
    fprintf(stderr, "list keys\n");

  DBC* dbCur = nullptr;
  DBT dbKey;
  DBT dbData;

  memset(&dbKey, 0x00, sizeof(DBT));
  memset(&dbData, 0x00, sizeof(DBT));

  if (db->cursor(db, nullptr, &dbCur, 0) == 0)
  {
    while (dbCur->get(dbCur, &dbKey, &dbData, DB_NEXT) == 0)
    {
      RecInfo rec(dbKey, dbData);
      rec.print(true);
    }
    dbCur->close(dbCur);
  }
  else
  {
    fprintf(stderr, "Unable to open cursor\n");
  }
}

bool
LDCUtil::read(RecInfo& key, bool verbose)
{
  bool retval = false;
  key.data.ulen = key.data.size;
  key.data.flags = DB_DBT_USERMEM;
  int status = db->get(db, nullptr, &key.dbkey, &key.data, 0);

  if (status == 0)
  {
    retval = true;
  }
  else if (verbose)
  {
    const char* errTxt = nullptr;

    switch (status)
    {
    case DB_BUFFER_SMALL:
    {
      errTxt = "User memory too small for return. ";
      break;
    }
    case DB_DONOTINDEX:
    {
      errTxt = "Null return from 2ndary callbk. ";
      break;
    }
    case DB_FOREIGN_CONFLICT:
    {
      errTxt = "A foreign db constraint triggered. ";
      break;
    }
    case DB_KEYEMPTY:
    {
      errTxt = "Key/data deleted or never created. ";
      break;
    }
    case DB_KEYEXIST:
    {
      errTxt = "The key/data pair already exists. ";
      break;
    }
    case DB_LOCK_DEADLOCK:
    {
      errTxt = "Deadlock. ";
      break;
    }
    case DB_LOCK_NOTGRANTED:
    {
      errTxt = "Lock unavailable. ";
      break;
    }
    case DB_LOG_BUFFER_FULL:
    {
      errTxt = "In-memory log buffer full. ";
      break;
    }
    case DB_NOSERVER:
    {
      errTxt = "Server panic return. ";
      break;
    }
    case DB_NOSERVER_HOME:
    {
      errTxt = "Bad home sent to server. ";
      break;
    }
    case DB_NOSERVER_ID:
    {
      errTxt = "Bad ID sent to server. ";
      break;
    }
    case DB_NOTFOUND:
    {
      errTxt = "Key/data pair not found (EOF). ";
      break;
    }
    case DB_OLD_VERSION:
    {
      errTxt = "Out-of-date version. ";
      break;
    }
    case DB_PAGE_NOTFOUND:
    {
      errTxt = "Requested page not found. ";
      break;
    }
    case DB_REP_DUPMASTER:
    {
      errTxt = "There are two masters. ";
      break;
    }
    case DB_REP_HANDLE_DEAD:
    {
      errTxt = "Rolled back a commit. ";
      break;
    }
    case DB_REP_HOLDELECTION:
    {
      errTxt = "Time to hold an election. ";
      break;
    }
    case DB_REP_IGNORE:
    {
      errTxt = "This msg should be ignored.";
      break;
    }
    case DB_REP_ISPERM:
    {
      errTxt = "Cached not written perm written.";
      break;
    }
    case DB_REP_JOIN_FAILURE:
    {
      errTxt = "Unable to join replication group. ";
      break;
    }
    case DB_REP_LEASE_EXPIRED:
    {
      errTxt = "Master lease has expired. ";
      break;
    }
    case DB_REP_LOCKOUT:
    {
      errTxt = "API/Replication lockout now. ";
      break;
    }
    case DB_REP_NEWSITE:
    {
      errTxt = "New site entered system. ";
      break;
    }
    case DB_REP_NOTPERM:
    {
      errTxt = "Permanent log record not written. ";
      break;
    }
    case DB_REP_UNAVAIL:
    {
      errTxt = "Site cannot currently be reached. ";
      break;
    }
    case DB_RUNRECOVERY:
    {
      errTxt = "Panic return. ";
      break;
    }
    case DB_SECONDARY_BAD:
    {
      errTxt = "Secondary index corrupt. ";
      break;
    }
    case DB_VERIFY_BAD:
    {
      errTxt = "Verify failed; bad format. ";
      break;
    }
    case DB_VERSION_MISMATCH:
    {
      errTxt = "Environment version mismatch. ";
      break;
    }
    }

    if (errTxt)
    {
      fprintf(stderr, "read error = %s on key [%s]\n", errTxt, key.keyPtr);
    }
  }

  return retval;
}

void
LDCUtil::getKeyList(std::vector<RecInfo>& list)
{
  list.clear();

  if (verbose)
    fprintf(stderr, "getKeyList()\n");

  DBC* dbCur = nullptr;
  DBT dbKey;
  DBT dbData;

  memset(&dbKey, 0x00, sizeof(DBT));
  memset(&dbData, 0x00, sizeof(DBT));

  if (db->cursor(db, nullptr, &dbCur, 0) == 0)
  {
    while (dbCur->get(dbCur, &dbKey, &dbData, DB_NEXT) == 0)
    {
      list.push_back(RecInfo(dbKey, dbData));
    }
    dbCur->close(dbCur);
  }
  else
  {
    fprintf(stderr, "Unable to open cursor\n");
  }
}

LDCUtil::RecInfo::RecInfo(const DBT& dbKey, const DBT& dbData)
{
  initData(dbKey, dbData);
}

LDCUtil::RecInfo&
LDCUtil::RecInfo::operator=(const RecInfo& ref)
{
  if (&ref != this)
  {
    initData(ref.dbkey, ref.data);
  }
  return *this;
}

LDCUtil::RecInfo::RecInfo(const RecInfo& ref)
{
  initData(ref.dbkey, ref.data);
}

void
LDCUtil::RecInfo::initData(const DBT& dbKey, const DBT& dbData)
{
  data = dbData;
  dbkey = dbKey;
  dataPtr = nullptr;
  keyPtr = nullptr;

  if (data.data)
  {
    size_t sz = data.size;
    dataPtr = new char[sz];
    memcpy(dataPtr, data.data, sz);
    data.data = dataPtr;
  }
  else
  {
    data.size = 0;
  }

  if (dbkey.data)
  {
    size_t sz = dbkey.size;
    keyPtr = new char[sz + 1];
    memcpy(keyPtr, dbkey.data, sz);
    keyPtr[sz] = 0;
    dbkey.data = keyPtr;
  }
  else
  {
    dbkey.size = 0;
  }
  header = (BlobHeader*)dataPtr;
}

LDCUtil::RecInfo::~RecInfo()
{
  delete [] dataPtr;
  dataPtr = nullptr;
  delete [] keyPtr;
  keyPtr = nullptr;
}

void
LDCUtil::RecInfo::print(bool doCR)
{
  struct tm tm;
  localtime_r(&header->timestamp, &tm);

  const char* df;
  switch (header->dataFormat)
  {
  case DATAFMT_BIN:
  {
    df = "BIN";
    break;
  }
  case DATAFMT_TXT:
  {
    df = "TXT";
    break;
  }
  case DATAFMT_XML:
  {
    df = "XML";
    break;
  }
  case DATAFMT_BN2:
  {
    df = "BN2";
    break;
  }
  default:
  {
    df = "UNK";
  }
  }
  fprintf(stdout,
          "%04d-%02d-%02d,%02d:%02d:%02d,%8d,%c,%s,%s,",
          tm.tm_year + 1900,
          tm.tm_mon + 1,
          tm.tm_mday,
          tm.tm_hour,
          tm.tm_min,
          tm.tm_sec,
          header->unCompressedSize,
          header->compressed,
          df,
          keyPtr);

  if (doCR)
  {
    fprintf(stdout, "\n");
  }
}

void
LDCUtil::RecInfo::print2()
{
  struct tm tm;
  localtime_r(&header->timestamp, &tm);

  size_t size = 0;

  if (header)
  {
    size = header->unCompressedSize;
  }

  fprintf(stderr, "%p,%p,%8d,%s,", data.data, dbkey.data, static_cast<int>(size), keyPtr);

  fprintf(stderr,
          " DBT data: size %u dlen %u doff %u flags %X\n",
          data.size,
          data.dlen,
          data.doff,
          data.flags);
}
