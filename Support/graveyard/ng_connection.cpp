




static class projectile_desc : public object_desc
{
public:
//=============================================================================
    projectile_desc( void ) : object_desc( 
            object::TYPE_PROJECTILE, 
            "Projectile", 
            object::ATTR_COLLIDABLE    | 
            object::ATTR_RENDERABLE    | 
            object::ATTR_SPACIAL_ENTRY |
            object::ATTR_NEEDS_LOGIC_TIME ) {}

//=============================================================================
    virtual object* Create          ( void ) 
    {
        return new projectile;
    }

} s_Projectile_Desc;






bbox ng_connection::GetLocalBBox    ( void ) const 
{
    sphere  tempSphere;
    if( )
    tempSphere.Set(m_)
    return m_Sphere.GetBBox(); 
}
