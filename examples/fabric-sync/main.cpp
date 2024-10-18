/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include "AppMain.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/util/endpoint-config-api.h>

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;

void ApplicationInit()
{
    ChipLogProgress(NotSpecified, "Fabric-Sync: ApplicationInit()");
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Fabric-Sync: ApplicationShutdown()");
}

int main(int argc, char * argv[])
{

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    Shell::RegisterCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}
