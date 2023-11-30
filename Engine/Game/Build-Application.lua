project "Application"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files
   {
       --"Source/**.inl",
       "Source/**.h",
       "Source/**.cpp"
   }

   includedirs
   {
      "Source",
      --"../Dependencies/Vulkan/Include"
   }

   -- Needed to include a different solution
   links
   {
      --"../Dependencies/Vulkan/Lib/vulkan-1.lib"
      --"Core"
   }

   --defines { "VK_PROTOTYPES", "VK_USE_PLATFORM_WIN32_KHR" }

   targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"