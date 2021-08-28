#ifndef PROXYPLAYSURFACE_HPP
#define PROXYPLAYSURFACE_HPP

#include "Objects\Object.hpp"

//=========================================================================
// This is a proxy object that gets passed around for collision purposes.
//=========================================================================

class proxy_playsurface : public object
{
public:
    CREATE_RTTI( proxy_playsurface, object, object )

    proxy_playsurface   ( void );
    ~proxy_playsurface  ( void );

    // set up
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    // general logic
    virtual bbox    GetLocalBBox    ( void ) const;
    virtual s32     GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    virtual xbool   GetColDetails   ( s32 Key, object::detail_tri& Tri );

    // proxy part of the whole thing
            void    SetSurface      ( guid Guid );

    // rendering...note that the normal OnRender shouldn't do anything,
    // but the shadow rendering does
    virtual void    OnRenderShadowReceive   ( u64 ProjMask );

protected:
    guid    m_CurrentGuid;
};

#endif // PROXYPLAYSURFACE_HPP