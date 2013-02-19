#ifndef __DB_USERP_HPP
#define __DB_USERP_HPP

#include <string>
#include <memory>
#include <boost/optional.hpp>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
}

namespace acl
{
class UserData;
class User;
} 

namespace db
{

struct UserCacheBase;

class User
{
  acl::UserData& user;

  void UpdateLog() const;
  void SaveField(const std::string& field) const;
  
public:
  User(acl::UserData& user) :  user(user) { }
  
  acl::UserID Create() const;
  bool SaveName();
  void SaveIPMasks();
  void SavePassword();
  void SaveFlags();
  void SavePrimaryGID();
  void SaveSecondaryGIDs();
  void SaveGadminGIDs();
  void SaveWeeklyAllotment();
  void SaveHomeDir();
  void SaveIdleTime();
  void SaveExpires();
  void SaveNumLogins();
  void SaveComment();
  void SaveTagline();
  void SaveMaxDownSpeed();
  void SaveMaxUpSpeed();
  void SaveMaxSimDown();
  void SaveMaxSimUp();
  void SaveLoggedIn();
  void SaveLastLogin();
  void SaveRatio();
  void IncrCredits(const std::string& section, long long kBytes);
  bool DecrCredits(const std::string& section, long long kBytes, bool force);
  
  void Purge() const;
  
  static boost::optional<acl::UserData> Load(acl::UserID uid);
};

std::vector<acl::UserID> GetUIDs(const std::string& multiStr = "*");
std::vector<acl::UserData> GetUsers(const std::string& multiStr = "*");

} /* db namespace */

#endif
