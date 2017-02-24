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
	src/shaderUtils.hpp
	src/shaderUtils.cpp
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
set(JIKKEN_INCLUDE ${JIKKEN_INCLUDE} include src "${GAME_ROOT_DIR}/thirdparty/glfw/include" "${JIKKEN_PATH}/thirdparty/glslang" "${JIKKEN_PATH}/thirdparty/glslang/glslang/include"
 "${JIKKEN_PATH}/thirdparty/glslang" "${JIKKEN_PATH}/thirdparty/SPIRV-Cross")
set(JIKKEN_LIBS ${JIKKEN_LIBS} glslang SPIRV OSDependent spirv-cross)
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