cmake_minimum_required(VERSION 2.8)

set (JIKKEN_SRC
	include/jikken/commands.hpp	
	include/jikken/commandQueue.hpp
	include/jikken/enums.hpp
	include/jikken/graphicsDevice.hpp
	include/jikken/jikken.hpp
	include/jikken/memory.hpp
	include/jikken/structs.hpp
	include/jikken/types.hpp

	src/commands.cpp
	src/commandQueue.cpp
	src/graphicsDevice.cpp
	src/jikken.cpp
)

# OpenGL Support is enabled by default.
option(JIKKEN_OPENGL "Use OpenGL rendering." ON)

# Vulkan Support
if (APPLE)
	# Apple does not have Vulkan at this time.
	set(JIKKEN_VULKAN OFF)
else()
	option(JIKKEN_VULKAN "Use Vulkan rendering." OFF)
endif()

if (JIKKEN_OPENGL)
	set (JIKKEN_SRC
		${JIKKEN_SRC}
		src/GL/GLGraphicsDevice.cpp
		src/GL/GLGraphicsDevice.hpp
		src/GL/GLUtil.cpp
		src/GL/GLUtil.hpp
	)
	
	add_library(GLEW STATIC thirdparty/glew/src/glew.c)
	target_include_directories(GLEW PUBLIC thirdparty/glew/include)
	target_compile_definitions(GLEW PUBLIC GLEW_STATIC)	
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
#glfw is only temp requirement while getting openblockcraft to work, remove me!
set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} include src "${GAME_ROOT_DIR}/thirdparty/glfw/include")

if (JIKKEN_OPENGL)
	
	target_compile_definitions(Jikken PUBLIC GLEW_STATIC _CRT_SECURE_NO_WARNINGS JIKKEN_OPENGL)
	set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} thirdparty/glew/include)
	set(JIKKEN_LIBS ${JIKKEN_LIBS} GLEW)
	# Platform specific OpenGL libraries
	if (WIN32)
		set(JIKKEN_LIBS ${JIKKEN_LIBS} OpenGL32)
	elseif(APPLE)
		set(JIKKEN_LIBS ${JIKKEN_LIBS} "-framework OpenGL")
	elseif(UNIX)
		set(JIKKEN_LIBS ${JIKKEN_LIBS} GL)
	else()
		message("Implement OpenGL linkage for your platform!")
	endif()
endif()

if (JIKKEN_VULKAN)
	target_compile_definitions(Jikken PUBLIC JIKKEN_VULKAN)
	#${VULKAN_XXX} is from glfw
	set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} ${VULKAN_INCLUDE_DIR})
	set(JIKKEN_LIBS ${JIKKEN_LIBS} ${VULKAN_LIBRARY})
	if(WIN32)
		target_compile_definitions(Jikken PUBLIC VK_USE_PLATFORM_WIN32_KHR)
	elseif(UNIX AND NOT APPLE)
		target_compile_definitions(Jikken PUBLIC VK_USE_PLATFORM_XCB_KHR)
	endif()
endif()

target_link_libraries(Jikken ${JIKKEN_LIBS})
target_include_directories(Jikken PUBLIC ${JIKKEN_INCLUDE})
target_compile_definitions(Jikken PUBLIC NOMINMAX)

source_group("core" REGULAR_EXPRESSION /*)
source_group("opengl" REGULAR_EXPRESSION GL/*)
source_group("vulkan" REGULAR_EXPRESSION vulkan/*)

#unfortunately we have to match openblockcraft with this one :/
#---BEGIN COPY FROM BULLET CMAKE---

# License: zlib
#
# Bullet Continuous Collision Detection and Physics Library
# http://bulletphysics.org
#
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the use of this software.
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it freely,
# subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

#We statically link to reduce dependancies
FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO )
	IF(${flag_var} MATCHES "/MD")
		STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	ENDIF(${flag_var} MATCHES "/MD")
	IF(${flag_var} MATCHES "/MDd")
		STRING(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
	ENDIF(${flag_var} MATCHES "/MDd")
ENDFOREACH(flag_var)
#---END COPY FROM BULLET CMAKE---
