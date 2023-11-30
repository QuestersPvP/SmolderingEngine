
workspace "SmolderingEngine"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Application"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { } -- "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus"

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
	
	group "Engine/Application"
		include "Engine/Application/Build-Application.lua"
	group ""

	--group "Engine/Engine"
	--	include "Engine/Engine/Build-Engine.lua"
	--group ""

	--group "Engine/Game"
	--	include "Engine/Game/Build-Game.lua"
	--group ""

group ""