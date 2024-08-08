#CMake minimum version requirement
cmake_minimum_required(VERSION 3.28)

# The required C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Project name
project(SmolderingEngine)

# Define a macro with the project source directory for use in c++
add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

# Source files for the project
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS 		"${CMAKE_CURRENT_SOURCE_DIR}/SmolderingEngine/*.cpp")
file(GLOB_RECURSE MY_HEADERS CONFIGURE_DEPENDS 		"${CMAKE_CURRENT_SOURCE_DIR}/SmolderingEngine/*.h")

# Add an executable
add_executable(SmolderingEngine 			${MY_SOURCES} ${MY_HEADERS})

# Include directories for source files
include_directories(SmolderingEngine PRIVATE 		${CMAKE_CURRENT_SOURCE_DIR}/SmolderingEngine)	# To access Engine or Game folders quickly

# Add GLFW
set(GLFW_INCLUDE_DIR 					${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/include)
set(GLFW_LIBRARY 					${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/lib-vc2022/glfw3.lib)

# Add GLM
set(GLM_INCLUDE_DIR 					${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLM)

# Add ImGui
set(IMGUI_INCLUDE_DIR 					${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/IMGUI)

# Add STB
set(STB_INCLUDE_DIR 					${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/STB)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include directories for ThirdParty libraries
target_include_directories(SmolderingEngine PRIVATE 	${GLFW_INCLUDE_DIR}
 							${GLM_INCLUDE_DIR}
							${IMGUI_INCLUDE_DIR}
							${Vulkan_INCLUDE_DIRS}
							${STB_INCLUDE_DIR})

# Link GLFW/Vulkan library
target_link_libraries(SmolderingEngine 			${GLFW_LIBRARY}
							${Vulkan_LIBRARIES})