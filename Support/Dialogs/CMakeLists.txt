file(GLOB DIALOGS_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(UIDialogs STATIC ${DIALOGS_SRC})
set_support_defaults(UIDialogs)

target_link_libraries(UIDialogs UI)