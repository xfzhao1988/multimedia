/**********************************************************************

  Audacity: A Digital Audio Editor

  @file CommandDispatch.cpp
  @brief implements functions HandleTextualCommand, DoAudacityCommand,
    OnAudacityCommand

  Paul Licameli split from BatchCommands.cpp

**********************************************************************/

#include "CommandDispatch.h"

#include "CommandContext.h"
#include "CommandManager.h"
#include "PluginManager.h"
#include "ProjectAudioManager.h"
#include "ProjectWindows.h"
#include "Viewport.h"
#include "../effects/EffectManager.h"
#include "../effects/EffectUI.h"
#include <wx/log.h>
#include <wx/frame.h>

bool CommandDispatch::HandleTextualCommand(
   const CommandID & Str,
   const CommandContext & context, CommandFlag flags, bool alwaysEnabled)
{
   auto &commandManager = CommandManager::Get(context.project);
   switch ( commandManager.HandleTextualCommand(
      Str, context, flags, alwaysEnabled) ) {
   case CommandManager::CommandSuccess:
      return true;
   case CommandManager::CommandFailure:
      return false;
   case CommandManager::CommandNotFound:
   default:
      break;
   }

   // Not one of the singleton commands.
   // We could/should try all the list-style commands.
   // instead we only try the effects.
   EffectManager & em = EffectManager::Get();
   for (auto &plug : PluginManager::Get().PluginsOfType(PluginTypeEffect))
      if (em.GetCommandIdentifier(plug.GetID()) == Str)
         return EffectUI::DoEffect(
            plug.GetID(), context,
            EffectManager::kConfigured);

   return false;
}

/// DoAudacityCommand() takes a PluginID and executes the associated command.
///
/// At the moment flags are used only to indicate whether to prompt for
/// parameters
bool CommandDispatch::DoAudacityCommand(
   const PluginID & ID, const CommandContext & context, unsigned flags )
{
   auto &project = context.project;
   auto &window = GetProjectFrame(project);
   const PluginDescriptor *plug = PluginManager::Get().GetPlugin(ID);
   if (!plug)
      return false;

   if (flags & EffectManager::kConfigured)
   {
      ProjectAudioManager::Get( project ).Stop();
//    SelectAllIfNone();
   }

   EffectManager & em = EffectManager::Get();
   bool success = em.DoAudacityCommand(ID,
      context,
      &window,
      (flags & EffectManager::kConfigured) == 0);

   if (!success)
      return false;

/*
   if (em.GetSkipStateFlag())
      flags = flags | OnEffectFlags::kSkipState;

   if (!(flags & OnEffectFlags::kSkipState))
   {
      wxString shortDesc = em.GetCommandName(ID);
      wxString longDesc = em.GetCommandDescription(ID);
      PushState(longDesc, shortDesc);
   }
*/
   Viewport::Get(project).Redraw();
   return true;
}

void CommandDispatch::OnAudacityCommand(const CommandContext & ctx)
{
   // using GET in a log message for devs' eyes only
   wxLogDebug( "Command was: %s", ctx.parameter.GET());
   // Not configured, so prompt user.
   CommandDispatch::DoAudacityCommand(
      EffectManager::Get().GetEffectByIdentifier(ctx.parameter),
      ctx, EffectManager::kNone);
}
