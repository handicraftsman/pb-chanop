#include <positronbot.hpp>

#include <functional>
#include <string>
#include <sstream>

using namespace std::placeholders;

extern "C" {

  pb::plugin* pb_plugin;

  void pb_init(std::vector<std::pair<std::string, std::string>>& cfg) {
    
    enum class GetRidType {
      Kick,
      Remove
    };
    
    auto get_rid_handler = [] (GetRidType grt, bool should_ban, pb::command& c, pb::event_command::ptr e) {
      if (e->split.size() < 2) {
        e->socket->stream() << pb::ircstream::nreply(e, "Invalid arguments!");
        return;
      }
      std::string who = e->split[1];
      std::string reason = "Bye!";
      if (e->split.size() > 2) {
        std::stringstream ss;
        for (int i = 2; i < e->split.size(); ++i) {
          ss << e->split[i] << " ";
        }
        reason = ss.str();
      }
      if (should_ban) {
        std::string host = e->socket->get_user_cache()[e->split[1]].host.value_or(e->split[1]);
        e->socket->stream() << ("MODE " + e->target + " +b " + host + "\r\n");
      }
      if (grt == GetRidType::Kick) {
        e->socket->stream() << ("KICK " + e->target + " " + who + " :" + reason + "\r\n");
      } else if (grt == GetRidType::Remove) {
        e->socket->stream() << ("REMOVE " + e->target + " " + who + " :" + reason + "\r\n");
      }
       e->socket->stream() << pb::ircstream::nreply(e, "Done!");
    };
    
    pb::command cmd_kick {
      .pplugin     = pb_plugin,
      .name        = "kick",
      .usage       = "<who> <reason...>",
      .description = "kicks users",
      .cooldown    = 0,
      .flag        = "kick",
      .handler     = std::bind(get_rid_handler, GetRidType::Kick, false, _1, _2)
    };
    pb_plugin->register_command(cmd_kick);
    
    pb::command cmd_kban {
      .pplugin     = pb_plugin,
      .name        = "kban",
      .usage       = "<who> <reason...>",
      .description = "kicks and bans users",
      .cooldown    = 0,
      .flag        = "kban",
      .handler     = std::bind(get_rid_handler, GetRidType::Kick, true, _1, _2)
    };
    pb_plugin->register_command(cmd_kban);
    
    pb::command cmd_remove {
      .pplugin     = pb_plugin,
      .name        = "remove",
      .usage       = "<who> <reason...>",
      .description = "removes users",
      .cooldown    = 0,
      .flag        = "remove",
      .handler     = std::bind(get_rid_handler, GetRidType::Remove, false, _1, _2)
    };
    pb_plugin->register_command(cmd_remove);
    
    pb::command cmd_rban {
      .pplugin     = pb_plugin,
      .name        = "rban",
      .usage       = "<who> <reason...>",
      .description = "removes and bans users",
      .cooldown    = 0,
      .flag        = "rban",
      .handler     = std::bind(get_rid_handler, GetRidType::Remove, true, _1, _2)
    };
    pb_plugin->register_command(cmd_rban);
    
    auto mode_handler = [] (
      std::string mode,
      bool give,
      bool resolve_host,
      bool allow_self,
      pb::command& c,
      pb::event_command::ptr e
    ) {
      std::string who = e->nick;
      if (allow_self) {
        if (e->split.size() > 2) {
          e->socket->stream() << pb::ircstream::nreply(e, "Invalid arguments!");
          return;
        }
        if (e->split.size() == 1) {
          if (resolve_host) { who = e->host; }
        } else if (e->split.size() == 2) {
          if (resolve_host) {
            who = resolve_host ? e->socket->get_user_cache()[e->split[1]].host.value_or(e->split[1]) : e->split[1];
          } else {
            who = e->split[1];
          }
        }
      } else {
        if (e->split.size() != 2) {
          e->socket->stream() << pb::ircstream::nreply(e, "Invalid arguments!");
          return;
        }
        if (resolve_host) {
          who = resolve_host ? e->socket->get_user_cache()[e->split[1]].host.value_or(e->split[1]) : e->split[1];
        } else {
          who = e->split[1];
        }
      }
      std::string cmd = "MODE " + e->target + " " + (give ? "+" : "-") + mode + " " + who + "\r\n";
      e->socket->stream() << cmd << pb::ircstream::nreply(e, "Done!");
    };
    
    pb::command cmd_ban {
      .pplugin     = pb_plugin,
      .name        = "ban",
      .usage       = "<who>",
      .description = "bans users",
      .cooldown    = 0,
      .flag        = "ban",
      .handler     = std::bind(mode_handler, "b", true, true, false, _1, _2)
    };
    pb_plugin->register_command(cmd_ban);
    
    pb::command cmd_unban {
      .pplugin     = pb_plugin,
      .name        = "unban",
      .usage       = "<who>",
      .description = "unbans users",
      .cooldown    = 0,
      .flag        = "ban",
      .handler     = std::bind(mode_handler, "b", false, true, false, _1, _2)
    };
    pb_plugin->register_command(cmd_unban);
    
    pb::command cmd_quiet {
      .pplugin     = pb_plugin,
      .name        = "quiet",
      .usage       = "<who>",
      .description = "quiets users",
      .cooldown    = 0,
      .flag        = "quiet",
      .handler     = std::bind(mode_handler, "q", true, true, false, _1, _2)
    };
    pb_plugin->register_command(cmd_quiet);
    
    pb::command cmd_unquiet {
      .pplugin     = pb_plugin,
      .name        = "unquiet",
      .usage       = "<who>",
      .description = "unquiets users",
      .cooldown    = 0,
      .flag        = "quiet",
      .handler     = std::bind(mode_handler, "q", false, true, false, _1, _2)
    };
    pb_plugin->register_command(cmd_unquiet);
    
    pb::command cmd_exempt {
      .pplugin     = pb_plugin,
      .name        = "exempt",
      .usage       = "<who>",
      .description = "exempts users",
      .cooldown    = 0,
      .flag        = "exempt",
      .handler     = std::bind(mode_handler, "e", true, true, true, _1, _2)
    };
    pb_plugin->register_command(cmd_exempt);
    
    pb::command cmd_unexempt {
      .pplugin     = pb_plugin,
      .name        = "unexempt",
      .usage       = "<who>",
      .description = "unexempts users",
      .cooldown    = 0,
      .flag        = "exempt",
      .handler     = std::bind(mode_handler, "e", false, true, true, _1, _2)
    };
    pb_plugin->register_command(cmd_unexempt);
  
    pb::command cmd_op {
      .pplugin     = pb_plugin,
      .name        = "op",
      .usage       = "<who>",
      .description = "ops users",
      .cooldown    = 0,
      .flag        = "op",
      .handler     = std::bind(mode_handler, "o", true, false, true, _1, _2)
    };
    pb_plugin->register_command(cmd_op);
    
    pb::command cmd_deop {
      .pplugin     = pb_plugin,
      .name        = "deop",
      .usage       = "<who>",
      .description = "deops users",
      .cooldown    = 0,
      .flag        = "op",
      .handler     = std::bind(mode_handler, "o", false, false, true, _1, _2)
    };
    pb_plugin->register_command(cmd_deop);
  
    pb::command cmd_hop {
      .pplugin     = pb_plugin,
      .name        = "hop",
      .usage       = "<who>",
      .description = "hops users",
      .cooldown    = 0,
      .flag        = "hop",
      .handler     = std::bind(mode_handler, "h", true, false, true, _1, _2)
    };
    pb_plugin->register_command(cmd_hop);
    
    pb::command cmd_dehop {
      .pplugin     = pb_plugin,
      .name        = "dehop",
      .usage       = "<who>",
      .description = "dehops users",
      .cooldown    = 0,
      .flag        = "hop",
      .handler     = std::bind(mode_handler, "h", false, false, true, _1, _2)
    };
    pb_plugin->register_command(cmd_dehop);
    
    pb::command cmd_voice {
      .pplugin     = pb_plugin,
      .name        = "voice",
      .usage       = "<who>",
      .description = "voices users",
      .cooldown    = 0,
      .flag        = "voice",
      .handler     = std::bind(mode_handler, "v", true, false, true, _1, _2)
    };
    pb_plugin->register_command(cmd_voice);
    
    pb::command cmd_devoice {
      .pplugin     = pb_plugin,
      .name        = "devoice",
      .usage       = "<who>",
      .description = "devoices users",
      .cooldown    = 0,
      .flag        = "voice",
      .handler     = std::bind(mode_handler, "v", false, false, true, _1, _2)
    };
    pb_plugin->register_command(cmd_devoice);
  }
    
  void pb_deinit() {}
  
}
