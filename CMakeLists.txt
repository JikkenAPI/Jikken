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

option(JIKKEN_OPENGL "Use OpenGL rendering." ON)
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

add_library(Jikken STATIC ${JIKKEN_SRC})
target_include_directories(Jikken PUBLIC include)

if (JIKKEN_OPENGL)
	target_link_libraries(Jikken GLEW)
	target_compile_definitions(Jikken PUBLIC GLEW_STATIC)
endif()