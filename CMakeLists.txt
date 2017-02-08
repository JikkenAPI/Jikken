set (JIKKEN_SRC
	include/jikken/commands.hpp	
	include/jikken/commandQueue.hpp
	include/jikken/enums.hpp
	include/jikken/graphicsDevice.hpp

	src/commands.cpp
	src/commandQueue.cpp
	src/graphicsDevice.cpp
)

add_library(Jikken STATIC ${JIKKEN_SRC})