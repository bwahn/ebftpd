#include "cmd/site/help.hpp"
#include "cmd/site/factory.hpp"
#include "acl/allowsitecmd.hpp"
#include "main.hpp"

namespace cmd { namespace site
{

cmd::Result HELPCommand::Syntax()
{
  boost::to_upper(args[1]);
  cmd::site::CommandDefOptRef def(cmd::site::Factory::Lookup(args[1]));
  if (!def) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else
  {
    std::ostringstream os;
    os << def->Syntax() << "\n";
    os << "Description: " << def->Description();
    control.Reply(ftp::CommandOkay, os.str());
  }

  return cmd::Result::Okay;
}

cmd::Result HELPCommand::List()
{
  const Factory::CommandDefsMap& commands = Factory::Commands();

  std::ostringstream os;
  os << " " << programFullname << " SITE command listing - \n\n";
  
  std::vector<std::string> sorted;
  size_t maxLen = 0;
  for (auto& kv : commands)
  {
    if (acl::AllowSiteCmd(client.User(), kv.second.ACLKeyword()))
    {
      sorted.push_back(kv.first);
      maxLen = std::max(kv.first.length(), maxLen);
    }
  }
  
  std::sort(sorted.begin(), sorted.end());
    
  for (auto& command : sorted)
  {
    os << " " << std::setw(maxLen) << command << " : " 
       << commands.at(command).Description() << "\n";
  }
    
  os << "\n End of list";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

cmd::Result HELPCommand::Execute()
{
  if (args.size() == 2) return Syntax();
  else return List();
}

} /* site namespace */
} /* cmd namespace */
