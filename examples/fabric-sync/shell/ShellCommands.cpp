/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "ShellCommands.h"

#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Shell {

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                           Usage: app <subcommand>\r\n");
    streamer_printf(sout, "  add-bridge       Pair remote fabric bridge to local fabric. Usage: app print-app-access\r\n");
    streamer_printf(sout,
                    "  remove-bridge    Remove the remote fabric bridge from the local fabric. Usage: app remove-app-access\r\n");
    streamer_printf(sout, "  sync-device      Sync a device from other ecosystem. Usage: app print-installed-apps\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppPlatformHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
    else if (strcmp(argv[0], "add-bridge") == 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (strcmp(argv[0], "remove-bridge") == 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (strcmp(argv[0], "sync-device") == 0)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterCommands()
{

    static const shell_command_t sDeviceComand = { &AppPlatformHandler, "app", "App commands. Usage: app [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
