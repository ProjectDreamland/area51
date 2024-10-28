file(GLOB MISCUTILS_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(MiscUtils STATIC ${MISCUTILS_SRC})
set_xcore_defaults(MiscUtils)
target_include_directories(MiscUtils PRIVATE "${CMAKE_SOURCE_DIR}/Support")
target_link_libraries(MiscUtils Entropy)