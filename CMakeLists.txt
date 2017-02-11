cmake_minimum_required(VERSION 2.8)

set (JIKKEN_SRC
	include/jikken/commands.hpp	
	include/jikken/commandQueue.hpp
	include/jikken/enums.hpp
	include/jikken/graphicsDevice.hpp
	include/jikken/memory.hpp
	include/jikken/structs.hpp
	include/jikken/types.hpp

	src/commands.cpp
	src/commandQueue.cpp
	src/graphicsDevice.cpp
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
target_include_directories(Jikken PUBLIC include src)

if (JIKKEN_OPENGL)
	target_link_libraries(Jikken GLEW)
	target_compile_definitions(Jikken PUBLIC GLEW_STATIC)

	# Platform specific OpenGL libraries
	if (WIN32)
		target_link_libraries(Jikken OpenGL32)
	else()
		message("Implement OpenGL linkage for your platform!")
	endif()
endif()