#include <sstream>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/chmod.hpp"
#include "fs/chmod.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void CHMODCommand::Process(const fs::Path& absmask)
{
  using util::string::WildcardMatch;
  const cfg::Config& config = cfg::Get();
  try
  {
    fs::DirContainer dir(client, absmask.Dirname());
    for (auto& entry : dir)
    {
      if (!WildcardMatch(absmask.Basename(), entry))
        continue;

      fs::Path fullPath = (absmask.Dirname() / entry).Expand();
      try
      {
        fs::Status status(config.Sitepath() + fullPath);          
        util::Error e = fs::Chmod(client, fullPath, *mode);
        if (!e)
        {
          ++failed;
          control.PartReply(ftp::CommandOkay, "CHOWN " + 
              fullPath.ToString() + ": " + e.Message());        
        }
        else
        if (status.IsDirectory())
        {
          ++dirs;
          if (recursive && !status.IsSymLink()) 
            Process((fullPath / "*").Expand());
        }
        else ++files;
      }
      catch (const util::SystemError& e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, "CHOWN " + 
            fullPath.ToString() + ": " + e.Message());        
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, 
        "CHMOD " + absmask.Dirname().ToString() + ": " + e.Message());
  }
}

bool CHMODCommand::ParseArgs()
{
  int n = 1;
  boost::to_lower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    boost::to_lower(args[n]);
  }
  
  modeStr = args[n];
  
  std::string::size_type pos =
      util::string::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) return false;
  
  pathmask = argStr.substr(pos);
  boost::trim(pathmask);
  return true;
}

// SITE CHMOD [-R] <MODE> <PATHMASK.. ..>
void CHMODCommand::Execute()
{
  if (!ParseArgs()) throw cmd::SyntaxError();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive chmod!");
    return;
  }

  try
  {
    mode.reset(fs::Mode(modeStr));
  }
  catch (const fs::InvalidModeString& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  Process((client.WorkDir() / pathmask).Expand());
  
  std::ostringstream os;
  os << "CHMOD finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
