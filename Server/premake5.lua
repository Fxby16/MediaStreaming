workspace "MediaStreaming"
    configurations { "Debug", "Release" }

project "MediaStreamingServer"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"

    files { "Source/**.cpp", "Source/**.h" }
    includedirs { "Source" }

    links { "civetweb", "tdjson"}

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"