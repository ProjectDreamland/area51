file(GLOB EVENTMGR_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(EventMgr STATIC ${EVENTMGR_SRC})
set_support_defaults(EventMgr)

target_link_libraries(EventMgr Entropy)