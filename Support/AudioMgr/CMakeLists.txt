file(GLOB AUDIOMGR_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(audiomgr STATIC ${AUDIOMGR_SRC})
set_support_defaults(audiomgr)

target_link_libraries(audiomgr Entropy)