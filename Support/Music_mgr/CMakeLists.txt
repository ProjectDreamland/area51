set(MUSICMGR_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB MUSICMGR_SRC 
	"${MUSICMGR_PATH}/*.cpp"
	"${MUSICMGR_PATH}/*.hpp"
)

add_library(MusicMgr STATIC ${MUSICMGR_SRC})
set_support_defaults(MusicMgr)

target_link_libraries(MusicMgr Entropy)