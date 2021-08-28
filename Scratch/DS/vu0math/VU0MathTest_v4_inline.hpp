
//==============================================================================

vector4t::vector4t( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

vector4t::vector4t( const vector3& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );
    asm( "vmove.xyz VOUT, VEC" : "=j VOUT" (XYZW) : "j VEC" (V.GetU128()) );
}

//==============================================================================

vector4t::vector4t( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    FORCE_ALIGNED_16( this );

    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  AW,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ), "r AW"  (aW) );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
}

//==============================================================================

