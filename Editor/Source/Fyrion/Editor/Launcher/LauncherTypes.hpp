#pragma once

namespace Fyrion
{
    struct ProjectLauncherSettings : Asset
    {
        FY_BASE_TYPES(Asset);

        String        defaultPath{};
        Array<String> recentProjects{};

        static void RegisterType(NativeTypeHandler<ProjectLauncherSettings>& type);
    };
}
