/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @file Contains shell commands for a ContentApp relating to Content App platform of the Video Player.
 */

#include "AppTvShellCommands.h"

#include <access/AccessControl.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <string>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
using namespace chip::AppPlatform;
using namespace chip::Access;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <controller/CHIPDeviceController.h>
#include <controller/CommissionerDiscoveryController.h>
using namespace ::chip::Controller;
extern DeviceCommissioner * GetDeviceCommissioner();
extern CHIP_ERROR CommissionerPairUDC(uint32_t pincode, size_t index);
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

using namespace chip::app::Clusters;

namespace chip {
namespace Shell {

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
static CHIP_ERROR DumpAccessControlEntry(const Access::AccessControl::Entry & entry)
{
    CHIP_ERROR err;

    ChipLogDetail(DeviceLayer, "----- BEGIN ENTRY -----");

    {
        FabricIndex fabricIndex;
        SuccessOrExit(err = entry.GetFabricIndex(fabricIndex));
        ChipLogDetail(DeviceLayer, "fabricIndex: %u", fabricIndex);
    }

    {
        Privilege privilege;
        SuccessOrExit(err = entry.GetPrivilege(privilege));
        ChipLogDetail(DeviceLayer, "privilege: %d", to_underlying(privilege));
    }

    {
        AuthMode authMode;
        SuccessOrExit(err = entry.GetAuthMode(authMode));
        ChipLogDetail(DeviceLayer, "authMode: %d", to_underlying(authMode));
    }

    {
        size_t count;
        SuccessOrExit(err = entry.GetSubjectCount(count));
        if (count)
        {
            ChipLogDetail(DeviceLayer, "subjects: %u", static_cast<unsigned>(count));
            for (size_t i = 0; i < count; ++i)
            {
                NodeId subject;
                SuccessOrExit(err = entry.GetSubject(i, subject));
                ChipLogDetail(DeviceLayer, "  %u: 0x" ChipLogFormatX64, static_cast<unsigned>(i), ChipLogValueX64(subject));
            }
        }
    }

    {
        size_t count;
        SuccessOrExit(err = entry.GetTargetCount(count));
        if (count)
        {
            ChipLogDetail(DeviceLayer, "targets: %u", static_cast<unsigned>(count));
            for (size_t i = 0; i < count; ++i)
            {
                Access::AccessControl::Entry::Target target;
                SuccessOrExit(err = entry.GetTarget(i, target));
                if (target.flags & Access::AccessControl::Entry::Target::kCluster)
                {
                    ChipLogDetail(DeviceLayer, "  %u: cluster: 0x" ChipLogFormatMEI, static_cast<unsigned>(i),
                                  ChipLogValueMEI(target.cluster));
                }
                if (target.flags & Access::AccessControl::Entry::Target::kEndpoint)
                {
                    ChipLogDetail(DeviceLayer, "  %u: endpoint: %u", static_cast<unsigned>(i), target.endpoint);
                }
                if (target.flags & Access::AccessControl::Entry::Target::kDeviceType)
                {
                    ChipLogDetail(DeviceLayer, "  %u: deviceType: 0x" ChipLogFormatMEI, static_cast<unsigned>(i),
                                  ChipLogValueMEI(target.deviceType));
                }
            }
        }
    }

    ChipLogDetail(DeviceLayer, "----- END ENTRY -----");

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "DumpAccessControlEntry: dump failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                           Usage: app <subcommand>\r\n");
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    streamer_printf(sout, "  print-app-access     Print all ACLs for app platform fabric. Usage: app print-app-access\r\n");
    streamer_printf(sout, "  remove-app-access    Remove all ACLs for app platform fabric. Usage: app remove-app-access\r\n");
    streamer_printf(
        sout,
        "  print-installed-apps   Print all installed content apps with their endpoints. Usage: app print-installed-apps\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
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
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    else if (strcmp(argv[0], "print-app-access") == 0)
    {
        Access::AccessControl::EntryIterator iterator;
        ReturnErrorOnFailure(Access::GetAccessControl().Entries(GetDeviceCommissioner()->GetFabricIndex(), iterator));

        Access::AccessControl::Entry entry;
        while (iterator.Next(entry) == CHIP_NO_ERROR)
        {
            DumpAccessControlEntry(entry);
        }
        return CHIP_NO_ERROR;
    }
    else if (strcmp(argv[0], "remove-app-access") == 0)
    {
        Access::GetAccessControl().DeleteAllEntriesForFabric(GetDeviceCommissioner()->GetFabricIndex());
        return CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterAppTvCommands()
{

    static const shell_command_t sDeviceComand = { &AppPlatformHandler, "app", "App commands. Usage: app [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
