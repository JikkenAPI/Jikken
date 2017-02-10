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
	)
endif()

add_library(Jikken STATIC ${JIKKEN_SRC})
target_include_directories(Jikken PUBLIC include)