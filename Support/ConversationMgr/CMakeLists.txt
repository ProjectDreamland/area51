file(GLOB CONVERSATIONMGR_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(ConversationMgr STATIC ${CONVERSATIONMGR_SRC})
set_support_defaults(ConversationMgr)

target_link_libraries(ConversationMgr Entropy)