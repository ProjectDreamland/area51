file(GLOB GAMEAPP_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_executable(GameApp WIN32 ${GAMEAPP_SRC})
set_support_defaults(GameApp)

set_target_properties(GameApp PROPERTIES OUTPUT_NAME "Area51")

target_link_libraries(GameApp GameLib)

# DirectX linkage
if (MSVC)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_directories(GameApp PRIVATE "$ENV{DXSDK_DIR}/Lib/x64")
	else()
		target_link_directories(GameApp PRIVATE "$ENV{DXSDK_DIR}/Lib/x86")
	endif()

	target_include_directories(GameApp PRIVATE "$ENV{DXSDK_DIR}/Include")
	target_link_libraries(GameApp d3d9 d3dx9 dxguid XInput dinput8 dsound)
endif()


# Miles linkage
if (MSVC)
	target_link_directories(GameApp PRIVATE "${CMAKE_SOURCE_DIR}/xCore/3rdParty/Miles6/lib/win")
	target_link_libraries(GameApp Mss32)
endif()
