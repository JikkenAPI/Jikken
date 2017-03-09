cmake_minimum_required(VERSION 2.8)

set (JIKKEN_SRC
	include/jikken/commands.hpp	
	include/jikken/commandQueue.hpp
	include/jikken/enums.hpp
	include/jikken/memory.hpp	
	include/jikken/jikken.hpp
	include/jikken/structs.hpp
	include/jikken/types.hpp

	src/commands.cpp
	src/commandQueue.cpp
	src/graphicsDevice.hpp
	src/graphicsDevice.cpp
	src/shaderUtils.hpp
	src/shaderUtils.cpp	
	src/jikken.cpp
)

#FreeBSD
if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

	if($ENV{LOCALBASE})
  		set(FREEBSD_LOCALBASE $ENV{LOCALBASE})
	else()
  		set(FREEBSD_LOCALBASE /usr/local)
	endif()

	include_directories(${FREEBSD_LOCALBASE}/include)
	link_directories(${FREEBSD_LOCALBASE}/lib)

endif()

# OpenGL Support is enabled by default.
option(JIKKEN_OPENGL "Use OpenGL rendering." ON)

# Vulkan Support
if (APPLE)
	# Apple does not have Vulkan at this time.
	set(JIKKEN_VULKAN OFF)
	set(JIKKEN_D3D11 OFF)	
elseif(WIN32)
	option(JIKKEN_D3D11 "Use D3D11 rendering." ON)
	option(JIKKEN_VULKAN "Use Vulkan rendering." OFF)
else()
	option(JIKKEN_VULKAN "Use Vulkan rendering." OFF)
endif()

#glslang
add_subdirectory("${JIKKEN_PATH}/thirdparty/glslang")
#spirv-cross
file(GLOB_RECURSE SPIRVCROSS_SRC "${JIKKEN_PATH}/thirdparty/SPIRV-Cross/*.cpp" "${JIKKEN_PATH}/thirdparty/SPIRV-Cross/*.hpp")
add_library(spirv-cross STATIC ${SPIRVCROSS_SRC})

if (JIKKEN_OPENGL)
	set (JIKKEN_SRC
		${JIKKEN_SRC}
		src/GL/GLGraphicsDevice.cpp
		src/GL/GLGraphicsDevice.hpp
		src/GL/GLUtil.cpp
		src/GL/GLUtil.hpp
	)
	
	if(WIN32)
	set (JIKKEN_SRC ${JIKKEN_SRC}
		src/GL/GLContext_wgl.cpp
		src/GL/GLContext_wgl.hpp
	)
	elseif(APPLE)
		set (JIKKEN_SRC ${JIKKEN_SRC}
		src/GL/GLContext_nsgl.mm
		src/GL/GLContext_nsgl.hpp
	)
	else()
		set (JIKKEN_SRC ${JIKKEN_SRC}
		src/GL/GLContext_glx.cpp
		src/GL/GLContext_glx.hpp
	)
	endif()
	
	add_library(GLEW STATIC thirdparty/glew/src/glew.c)
	target_include_directories(GLEW PUBLIC thirdparty/glew/include)
	target_compile_definitions(GLEW PUBLIC GLEW_STATIC)
	target_compile_definitions(GLEW PUBLIC GLEW_NO_GLU)	
endif()

if (JIKKEN_VULKAN)
	set (JIKKEN_SRC
		${JIKKEN_SRC}
		src/vulkan/VulkanGraphicsDevice.cpp
		src/vulkan/VulkanGraphicsDevice.hpp
		src/vulkan/VulkanUtil.cpp
		src/vulkan/VulkanUtil.hpp
	)
endif()

add_library(Jikken STATIC ${JIKKEN_SRC})
set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} include src "${JIKKEN_PATH}/thirdparty/glslang" "${JIKKEN_PATH}/thirdparty/glslang/glslang/include" "${JIKKEN_PATH}/thirdparty/SPIRV-Cross")
set(JIKKEN_LIBS ${JIKKEN_LIBS} glslang SPIRV OSDependent spirv-cross)

if (JIKKEN_OPENGL)	
	target_compile_definitions(Jikken PUBLIC GLEW_STATIC _CRT_SECURE_NO_WARNINGS JIKKEN_OPENGL)
	set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} thirdparty/glew/include)
	set(JIKKEN_LIBS ${JIKKEN_LIBS} GLEW)
	# Platform specific OpenGL libraries
	if (WIN32)
		set(JIKKEN_LIBS ${JIKKEN_LIBS} OpenGL32)
	elseif(APPLE)
		set (JIKKEN_LIBS 
			${JIKKEN_LIBS} 
			"-framework AppKit"
			"-framework Foundation"
			"-framework OpenGL"
			"-framework Cocoa"
		)
	elseif(UNIX)
		set(JIKKEN_LIBS ${JIKKEN_LIBS} GL)
	else()
		message("Implement OpenGL linkage for your platform!")
	endif()
endif()

if (JIKKEN_D3D11)
	target_compile_definitions(Jikken PUBLIC JIKKEN_D3D11)
endif()

if (JIKKEN_VULKAN)
	target_compile_definitions(Jikken PUBLIC JIKKEN_VULKAN)
	set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} ${VULKAN_INCLUDE_DIR})
	set(JIKKEN_LIBS ${JIKKEN_LIBS} ${VULKAN_LIBRARY})
	if(WIN32)
		target_compile_definitions(Jikken PUBLIC VK_USE_PLATFORM_WIN32_KHR)
	elseif(UNIX AND NOT APPLE)
		target_compile_definitions(Jikken PUBLIC VK_USE_PLATFORM_XLIB_KHR)
	endif()
endif()

target_link_libraries(Jikken ${JIKKEN_LIBS})
target_include_directories(Jikken PUBLIC ${JIKKEN_INCLUDE})
target_compile_definitions(Jikken PUBLIC NOMINMAX)

source_group("core" REGULAR_EXPRESSION /*)
source_group("include" REGULAR_EXPRESSION include/*)
source_group("opengl" REGULAR_EXPRESSION GL/*)
source_group("vulkan" REGULAR_EXPRESSION vulkan/*)
source_group("d3d11" REGULAR_EXPRESSION d3d11/*)
