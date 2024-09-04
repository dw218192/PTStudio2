-- Shared build scripts from repo_build package
repo_build = require("omni/repo/build")
repo_build.root = os.getcwd()
repo_build.setup_options()

workspace "PTStudio"
    repo_build.setup_workspace({
        windows_x86_64_enabled = true,
        linux_x86_64_enabled = true,
        linux_aarch64_enabled = true,
        macos_universal_enabled = true,
        msvc_version = "14.29.30133",
        winsdk_version = "10.0.20348.0",
        copy_windows_debug_libs = false,
        allow_undefined_symbols_linux = true,
        extra_warnings = true,
        security_hardening = true,
        fix_cpp_version = true,
    })

    configurations { "Debug", "Release" }
    -- sets where the generated solution files will be located
    location(repo_build.workspace_dir())
    startproject "PTStudio"

    repo_build.enable_vstudio_sourcelink()
    repo_build.remove_vstudio_jmc()

    exceptionhandling "On"
    rtti "On"

    filter "configurations:Debug"
        defines { "TBB_USE_DEBUG" }
    filter "system:windows"
        defines { "NOMINMAX" }
        buildoptions { "/guard:cf" }
    filter "system:linux"
        buildoptions { "-fvisibility=hidden", "-fdiagnostics-color", "-Wno-deprecated" }
    filter {}

    flags { "ShadowedVariables" }

project "PTStudio"
    kind "ConsoleApp"
    location(repo_build.workspace_dir().."/PTStudio")
    language "C++"
    cppdialect "C++17"
    files { "source/PTStudio/**.*" }
    includedirs { "_build/target-deps/carb_sdk_plugins/include" }
    libdirs { "_build/target-deps/carb_sdk_plugins/_build/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}" }
    links { "carb" }
    repo_build.copy_to_targetdir("source/PTStudio/*.toml")
    filter { "system:linux" }
        buildoptions { "-pthread" }
        links { "dl", "pthread" }
    filter {}