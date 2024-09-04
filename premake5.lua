-- Shared build scripts from repo_build package
repo_build = require("omni/repo/build")
repo_build.root = os.getcwd()
repo_build.setup_options()
target_deps_dir = repo_build.target_deps_dir()

connect_sdk_premake_files = os.matchfiles(target_deps_dir.."/omni_connect_sdk/*/dev/tools/premake/connect-sdk-public.lua")
if #connect_sdk_premake_files == 0 then
    error("Could not find connect-sdk-public.lua in "..target_deps_dir.."/omni_connect_sdk/")
end

-- Shared build scripts for use in client Connectors
connect_build = require(path.replaceextension(connect_sdk_premake_files[1], ""))

workspace "PTStudio"
    connect_build.setup_workspace({
        msvc_version = "14.29.30133",
        winsdk_version = "10.0.20348.0",
    })
    target_bin_dir = target_build_dir
    target_lib_dir = target_build_dir

project "PTStudio"
    kind "ConsoleApp"
    includedirs {
        "source/include",
    }
    connect_build.executable({
        name = "PTStudio",
        sources = { "source/PTStudio/**.*" },
    })
    runpathdirs { target_bin_dir.."/python-runtime" }

    connect_build.use_carb()
    connect_build.use_usd({
        "arch",
        "gf",
        "kind",
        "pcp",
        "plug",
        "sdf",
        "tf",
        "usd",
        "usdGeom",
        "usdLux",
        "usdPhysics",
        "usdShade",
        "usdSkel",
        "usdUtils",
        "vt",
    })
    connect_build.use_connect_core()


    filter { "system:windows" }
        -- This sets the working directory when debugging/running from Visual Studio
        debugdir "$(ProjectDir)..\\..\\.."
        filter { "configurations:debug" }
            debugenvs "CARB_APP_PATH=$(ProjectDir)..\\..\\..\\_build\\windows-x86_64\\debug"
        filter { "configurations:release" }
            debugenvs "CARB_APP_PATH=$(ProjectDir)..\\..\\..\\_build\\windows-x86_64\\release"
        filter {}
    filter { "system:linux" }
        links { "pthread", "stdc++fs" }
    filter {}
