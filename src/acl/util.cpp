#include <sstream>
#include <cassert>
#include <iomanip>
#include <boost/regex.hpp>
#include "acl/util.hpp"
#include "util/error.hpp"
#include "util/verify.hpp"
#include "acl/user.hpp"
#include "cfg/get.hpp"
#include "acl/group.hpp"
#include "acl/flags.hpp"
#include "logs/logs.hpp"

namespace acl
{

namespace
{

boost::regex validationPatterns[] =
{
  boost::regex("\\w+"),             // username
  boost::regex("\\w+"),             // groupname
  boost::regex("[^{}]+")            // tagline
};

}

bool CreateDefaults()
{
  try
  {
    if (!GIDExists(0))
    {
      auto group = Group::Create("ebftpd");
      if (!group)
      {
        logs::Error("Failed to create master group (ebftpd).");
        return false;
      }
      
      if (group->ID() != 0)
      {
        logs::Error("Master group (ebftpd) created with wrong GID (must be 0).");
        return false;
      }
    }
    
    if (!GIDExists(1))
    {
      auto group = Group::Create("default");
      if (!group)
      {
        logs::Error("Failed to create template group (default).");
        return false;
      }
      
      if (group->ID() != 1)
      {
        logs::Error("Template group (default) create with wrong GID (must be 1).");
        return false;
      }
    }

    if (!UIDExists(0))
    {
      auto user = User::Create("ebftpd", "ebftpd", 0);
      if (!user)
      {
        logs::Error("Failed to create master user (ebftpd).");
        return false;
      }

      if (user->ID() != 0)
      {
        logs::Error("Master user (ebftpd) created with wrong UID (must be 0).");
        return false;
      }
      
      user->AddIPMask("*@localhost");
      user->AddFlag(Flag::Siteop);
      user->SetPrimaryGID(0);
    }
    
    if (!UIDExists(1))
    {
      auto user = User::Create("default", "default", 0);
      if (!user)
      {
        logs::Error("Failed to create template user (default). Must exist with wrong UID.");
        return false;
      }
      
      if (user->ID() != 1)
      {
        logs::Error("Template user (default) create with wrong UID (must be 1).");
        return false;
      }
      
      user->AddFlag(Flag::Template);
    }
    else
    if (NameToUID("default") != 1)
    {
      logs::Error("Unable to find template user (default).");
      return false;
    }
    else
    if (NameToGID("default") != 1)
    {
      logs::Error("Unable to find template group (default).");
      return false;
    }
  }
  catch (const util::RuntimeError&)
  {
    return false;
  }
  
  return true;
}

bool Validate(ValidationType type, const std::string& s)
{
  return boost::regex_match(s, validationPatterns[static_cast<unsigned>(type)]);
}

std::string FormatRatio(int ratio)
{
  assert(ratio >= 0);
  if (ratio == 0) return "Unlimited";
  std::ostringstream os;
  os << "1:" << ratio;
  return os.str();
}

std::string RatioString(const User& user)
{
  std::ostringstream os;
  os << FormatRatio(user.DefaultRatio());
  for (const auto& kv : cfg::Get().Sections())
  {
    if (user.SectionRatio(kv.first) != -1)
    {
      os << " " << kv.first << "(" << FormatRatio(user.SectionRatio(kv.first)) << ")";
    }
  }
  return os.str();
}

std::string FormatCredits(long long credits)
{
  std::ostringstream os;
  os << std::setprecision(2) << std::fixed << (credits / 1024.0) << "MB";
  return os.str();
}

std::string CreditString(const User& user)
{
  std::ostringstream os;
  os << FormatCredits(user.DefaultCredits());
  for (const auto& kv : cfg::Get().Sections())
  {
    if (kv.second.SeparateCredits())
    {
      os << " " << kv.first << "(" << FormatCredits(user.SectionCredits(kv.first)) << ")";
    }
  }
  return os.str();
}

std::string GroupString(const User& user)
{
  std::ostringstream os;
  if (user.HasGadminGID(user.PrimaryGID())) os << "+";
  os << user.PrimaryGroup();
  for (acl::GroupID gid : user.SecondaryGIDs())
  {
    os << " ";
    if (user.HasGadminGID(gid)) os << "+";
    os << acl::GIDToName(gid);
  }
  return os.str();
}

std::string WeeklyAllotmentString(const User& user)
{
  std::ostringstream os;
  if (user.DefaultWeeklyAllotment() <= 0)
    os << "Disabled";
  else
    os << FormatCredits(user.DefaultWeeklyAllotment());
  for (const auto& kv : cfg::Get().Sections())
  {
    if (user.SectionWeeklyAllotment(kv.first) > 0)
    {
      os << " " << kv.first << "(" <<  FormatCredits(user.SectionWeeklyAllotment(kv.first)) <<  ")";
    }
  }
  return os.str();  
}

} /* acl namespace */
