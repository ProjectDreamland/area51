file(GLOB UI_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(UI STATIC ${UI_SRC})
set_support_defaults(UI)

target_link_libraries(UI Entropy)