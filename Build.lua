
workspace "Smoldering Engine"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Engine"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { } -- "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus"

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"

	group "Engine/Renderer"
		include "Engine/Renderer/Build-Renderer.lua"
	group ""
	
	group "Engine/GUI"
		include "Engine/GUI/Build-GUI.lua"
	group ""
	--include "Engine/Build-Engine.lua"
group ""

--include "App/Build-App.lua"