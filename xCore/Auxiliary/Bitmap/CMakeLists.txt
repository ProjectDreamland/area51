file(GLOB BITMAP_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(aux_Bitmap STATIC ${BITMAP_SRC})
set_xcore_defaults(aux_Bitmap)