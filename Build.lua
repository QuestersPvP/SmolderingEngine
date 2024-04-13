
workspace "SmolderingEngine"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Application"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { } -- "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus"

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

defines 
{
	"CURRENT_WORKING_DIRECTORY=\"" .. os.getcwd() .. "\""
}

group "Engine"

	files  
	{
		"Dependencies/**.cpp",
		"Dependencies/**.h",

		-- include everything within KTX separately 
		"Dependencies/KTX/**.cpp",
		"Dependencies/KTX/**.h",
		"Dependencies/KTX/**.c",
		"Dependencies/KTX/**.cxx",
		"Dependencies/KTX/**.inl",
		"Dependencies/KTX/**.gypi"
	}	

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