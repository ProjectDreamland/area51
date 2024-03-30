set(RENDER_SRC
"${CMAKE_CURRENT_SOURCE_DIR}/CollisionVolume.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/CollisionVolume.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/editor_icons.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/editor_icons.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/geom.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/geom.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/geom_mgr.inl"
"${CMAKE_CURRENT_SOURCE_DIR}/LightMgr.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/LightMgr.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Material.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Material.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/MaterialArray.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/MaterialArray.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Material_Prefs.hpp"
#"${CMAKE_CURRENT_SOURCE_DIR}/pc_Render.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/platform_Render.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/ProjTextureMgr.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/ProjTextureMgr.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Render.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Render.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/RigidGeom.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/RigidGeom.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/shader_mgr.inl"
"${CMAKE_CURRENT_SOURCE_DIR}/SkinGeom.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/SkinGeom.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/SoftVertexMgr.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/SoftVertexMgr.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Texture.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/VertexMgr.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/VertexMgr.hpp"
)

add_library(Render STATIC ${RENDER_SRC})
set_support_defaults(Render)

target_link_libraries(Render Entropy)