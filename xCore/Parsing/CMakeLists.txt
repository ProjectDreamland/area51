file(GLOB PARSING_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(Parsing STATIC ${PARSING_SRC})
set_xcore_defaults(Parsing)