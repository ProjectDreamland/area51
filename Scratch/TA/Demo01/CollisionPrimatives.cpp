#include "entropy.hpp"
//    vector3 g_LastBBoxTris[12][3];
//    xbool   g_LastBBoxSet = FALSE;
//    vector3 g_LastRaySelect[2];
//=========================================================================

#if 0

struct cdb
{
    vector3 SphereCenterAtImpact;
    vector3 Normal;
    vector3 FinalHitPoint;
    vector3 Start;
    vector3 End;
    s32     nCollisions;
    f32     FinalTs[6];
};
cdb g_cdb;

#endif

//=========================================================================

xbool CollideSphereWithPoint( const vector3& Start, 
                              const vector3& Dir,
                                    f32      Radius,
                              const vector3& Point,
                                    f32&     hit_time,
                                    vector3& hit_point )
{
    f32     length    = Dir.Length();
    if ( ABS(length) < 0.0001f )
    {
        return FALSE;
    }
    
    vector3 ray_start = Point;
    vector3 ray_dir   = -Dir;
    vector3 sphere_pos = Start;
    f32     sphere_rad = Radius;
    ray_dir /= length;


	// get the offset vector
	vector3 offset = sphere_pos - ray_start;

	// get the distance along the ray to the center point of the sphere
	f32 ray_dist = ray_dir.Dot(offset);
	if( ray_dist <= 0 || (ray_dist - length) > sphere_rad) {
		// moving away from object or too far away
		return false;
	}

	// get the squared distances
	f32 off2 = offset.Dot(offset);
	f32 Rad2 = sphere_rad * sphere_rad;
	if( off2 <= Rad2 ) {
		// we're in the sphere
		hit_point = ray_start;
		hit_time = 0;
		return true;
	}

	// find hit distance squared
	f32 d = Rad2 - (off2 - ray_dist * ray_dist);
	if( d < 0 ) {
		// ray passes by sphere without hitting
		return false;
	}

	// get the distance along the ray
	hit_time = (f32)(ray_dist - x_sqrt( d ));
	if( hit_time > length ) {
		// hit point beyond length
		return false;
	}

	// sort out the details
    vector3 sphere_point = ray_start + ray_dir * hit_time;
	hit_time /= length;
	hit_point = sphere_point + hit_time*Dir;
	return true;
}

//=========================================================================

xbool PointInTri( const vector3& Point, 
                  const vector3* pTri, 
                  const vector3& Normal )
{
    // given largest component of normal, return i & j
    static s32 ij_table[3][2] = { {2,1},  // norm x biggest
                                  {0,2},  // norm y biggest
                                  {1,0} };// norm z biggest

    f32 u0, u1, u2, v0, v1, v2;
    s32 i0, i1, i2;
    f32 alpha, beta;

    // find largest component of normal
    f32 abs_x = (f32)x_abs(Normal.X); 
    f32 abs_y = (f32)x_abs(Normal.Y); 
    f32 abs_z = (f32)x_abs(Normal.Z);
    if( abs_x > abs_y ) {
        if( abs_x > abs_z ) {
            i0 = 0; 
        } else {
            i0 = 2;
        }
    } else {
        if( abs_y > abs_z ) {
            i0 = 1;
        } else {
            i0 = 2;
        }
    }

    // pretend that norm1 is an array of three f32s.
    f32 *norm1_xyz = (f32*)&Normal;

    // figure out the two dimensions we are going to use
    if( norm1_xyz[i0] > 0.0f ) 
    {
        i1 = ij_table[i0][0];
        i2 = ij_table[i0][1];
    } 
    else 
    {
        i1 = ij_table[i0][1];
        i2 = ij_table[i0][0];
    }

    // pretend that hit_pos32 and *(verts[N]) are arrays of three f32s.
    f32 *hit_pos32_xyz = (f32*)&Point;
    f32 *verts0_xyz = (f32*)(pTri+0);
    f32 *verts1_xyz = (f32*)(pTri+1);
    f32 *verts2_xyz = (f32*)(pTri+2);

    // get deltas for hitpos32
    u0 = hit_pos32_xyz[i1] - verts0_xyz[i1];
    v0 = hit_pos32_xyz[i2] - verts0_xyz[i2];

    // ditto edge 1
    u1 = verts1_xyz[i1] - verts0_xyz[i1]; 
    v1 = verts1_xyz[i2] - verts0_xyz[i2];

    // ditto edge 2
    u2 = verts2_xyz[i1] - verts0_xyz[i1];
    v2 = verts2_xyz[i2] - verts0_xyz[i2];

    // calculate alpha and beta
    if( (u1 > -0.0001f) && (u1 < 0.0001f) ) 
    {
        // special case to guard against divide by zero
        beta = u0 / u2;
        if( (beta >= 0.0f) && (beta <= 1.0f) ) 
        {
            alpha = (v0 - beta*v2) / v1;
            return ((alpha >= 0.0f) && (alpha+beta <= 1.0f));
        }
    } 
    else 
    {
        beta = (v0*u1 - u0*v1) / (v2*u1 - u2*v1);
        if( (beta >= 0.0f) && (beta <= 1.0f) )  
        {
            alpha = (u0 - beta*u2) / u1;
            return ((alpha >= 0.0f) && (alpha+beta <= 1.0f));
        }
    }

    // not inside polygon
    return false;
}

//=========================================================================

xbool ComputeSphereTriCollision( const vector3* Tri, 
                                 const vector3& Start, 
                                 const vector3& End, 
                                       f32      Radius,
                                       f32&     FinalT,
                                       vector3& FinalHitPoint,
                                       xbool&   HitTriangleEdge )
{
    s32 i;
    vector3 Dir = End-Start;
    plane Plane;
    Plane.Setup( Tri[0], Tri[1], Tri[2] );
    const f32 DirDotNormal = Dir.Dot( Plane.Normal );

    if ( DirDotNormal > 0 )
    {
        return FALSE;
    }

    // Are we completely in front or starting from behind?
    f32 StartDist = Plane.Distance( Start );
    f32 EndDist   = Plane.Distance( End   );
    if( (StartDist<-Radius) || (EndDist>Radius) )
    {
        return FALSE;
    }

    //
	// Find the closest point on the sphere to the plane
    //
    {
        f32 T;
        vector3 HP;
        vector3 SphereBot = Start - (Plane.Normal*Radius);
        T  = -Plane.Distance( SphereBot ) / DirDotNormal;

        if ( T > 1.0f ) 
        {
            return FALSE;
        }
        
        if ( T >= 0 ) 
        {
            HP = SphereBot + T*Dir;

            // Determine if point is inside tri.
            if( PointInTri( HP, Tri, Plane.Normal ) )
            {
                FinalT = T;
                FinalHitPoint = HP;
                HitTriangleEdge = FALSE;
                return TRUE;
            }
        }
    }

    HitTriangleEdge = TRUE; // if there is a collision below, this will be true
    
    FinalT = 2.0f;
    u32 CullVert=0;

    s32 va = 0;
    s32 vb = 2;
    for( i=0; i<3; i++ )
    {
        va = vb;
        vb = i;
        vector3 P0 = Tri[va];
        vector3 P1 = Tri[vb];

	    vector3 Edge            = P1 - P0;
	    vector3 Delta           = Start - P0;
	    f32     DeltaDotEdge    = Delta.Dot(Edge);
	    f32     DeltaDotDir     = Delta.Dot(Dir);
	    f32     EdgeDotDir      = Edge.Dot(Dir);
	    f32     DeltaSqr        = Delta.LengthSquared();
	    f32     EdgeSqr         = Edge.LengthSquared();
	    f32     DirSqr          = Dir.LengthSquared();
    
        f32     A       = EdgeDotDir * EdgeDotDir - EdgeSqr * DirSqr;
        f32     B       = 2 * (DeltaDotEdge * EdgeDotDir - DeltaDotDir * EdgeSqr);
        f32     C       = DeltaDotEdge * DeltaDotEdge + Radius * Radius * EdgeSqr - DeltaSqr * EdgeSqr;
        f32     Disc    = B*B - 4*A*C;

        if( Disc < 0 ) 
        {
		    // discriminant negative, sphere passed edge too far away
            // don't check verts
            CullVert |= ((1<<va)|(1<<vb));
            continue;
        }

	    if( A < -0.0001f || A > 0.0001f ) 
        {
		    f32 root  = x_sqrt(Disc);
		    f32 root1 = (-B + root) / (2 * A);
		    f32 root2 = (-B - root) / (2 * A);

		    // sort root1 and root2, use the earliest intersection.  the larger root 
		    //  corresponds to the final contact of the sphere with the edge on its 
		    //  way out.
		    if( root2 < root1 ) 
            {
			    f32 temp = root1;
			    root1 = root2;
			    root2 = temp;
		    }

		    // root1 is a time, check that it's in our currently valid range
		    if( (root1 < 0) || (root1 > 1.0f) )
            {
                // did not hit line within time range
			    continue;
		    }

		    // find sphere and edge positions
		    vector3 SphereHit = Start + Dir * root1;

		    // check if hit is between P0 and P1
		    f32 EdgeT = ((SphereHit - P0).Dot(Edge)) / EdgeSqr;
		    if( (EdgeT >= 0) && (EdgeT <= 1) ) 
            {
			    // bingo
                if( root1 < FinalT )
                {
			        FinalT = root1;
			        FinalHitPoint = P0 + Edge * EdgeT;

                    // hit within edge so don't check end verts
                    CullVert |= ((1<<va)|(1<<vb));
                    continue;
                }
		    }

            // sphere hit line but not within segment.  
            continue;
	    }

	    // Degenerate case, sphere is traveling parallel to edge.
        // It might hit the verts
    }

    // Are there any verts to check?
    if( CullVert != 0x7 )
    {
        for( i=0; i<3; i++ )
        {
            if( CullVert & (1<<i) )
                continue;

            f32 T;
            vector3 HP;
            if( CollideSphereWithPoint( Start, Dir, Radius, Tri[i], T, HP ) )
            {
                if( T < FinalT )
                {
                    FinalT = T;
                    FinalHitPoint = HP;
                }
            }
        }
    }

    if( FinalT < 2.0f )
        return TRUE;

    return FALSE;

}

//=============================================================================
xbool ComputeRayTriCollision   ( const vector3* Tri,
                                 const vector3& Start, 
                                 const vector3& End, 
                                       f32&     FinalT,
                                       vector3& FinalHitPoint )
{
    s32 i;
//    vector3 Dir = End-Start;
    plane Plane;
    Plane.Setup( Tri[0], Tri[1], Tri[2] );

    // Are we completely in front or starting from behind?
    if ( !(Plane.InFront( Start ) && Plane.InBack( End )) )
    {
        return FALSE;
    }
        
    // Find where we hit the plane
    f32 T;
    Plane.Intersect( T, Start, End );

    if ( (T < 0.0f) || (T > 1.0f) )
    {
        return FALSE;
    }

    vector3 HitPoint = Start + ((End - Start) * T);

    // See if hit point is inside tri
    vector3 EdgeNormal;
    
    for ( i = 0; i < 3; ++i )
    {
        EdgeNormal = Plane.Normal.Cross( Tri[(i+1)%3] - Tri[i] );
        if( EdgeNormal.Dot( HitPoint - Tri[i] ) < -0.001f )
        {
            return FALSE;
        }
    }

    // Collision
    FinalT = T;
    FinalHitPoint = HitPoint;
    return TRUE;
}

//==============================================================================
xbool ComputeRaySphereCollision( const vector3& SpherePos,
                                 const f32      SphereRadius, 
                                 const vector3& Start, 
                                 const vector3& End, 
                                       f32&     FinalT,
                                       vector3& FinalHitPoint )
{
    ASSERT( (Start-End).LengthSquared() > 0.000000001f );
    
    const vector3   TestPoint = SpherePos.GetClosestPToLSeg( Start, End );
    const vector3   Diff = TestPoint - SpherePos;
    const f32       DistSQ = Diff.LengthSquared();
    const f32       RadiusSQ = SphereRadius * SphereRadius;

    if ( DistSQ < RadiusSQ )
    {
        // The point is inside the sphere, so find the point of contact
        f32 BackDist = x_sqrt( RadiusSQ - DistSQ );

        vector3 Temp = TestPoint - Start;
        const f32 Length = Temp.Length();

        f32 DistToHitPoint = Length - BackDist;

        if ( DistToHitPoint < 0 )
        {
            DistToHitPoint = 0;
        }

        if ( Length > 0 )
        {
            Temp /= Length; // Normalize
        }
        
        Temp *= DistToHitPoint;

        FinalHitPoint = Start + Temp;
        FinalT = DistToHitPoint / ((Start-End).Length());

        if ( (FinalT < 0.0f) || (FinalT > 1.0f) )
        {
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//==============================================================================
xbool ComputeSphereSphereCollision( const vector3& TestSpherePos,
                                    const f32      TestSphereRadius, 
                                    const f32      MovingSphereRadius,
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32&     FinalT,
                                          vector3& FinalHitPoint )
{
    // Shrink the moving sphere to a ray, and grow the test sphere by the
    // moving sphere's radius. Then do a ray/sphere check
    f32 TempRadius = TestSphereRadius + MovingSphereRadius;

    if ( !ComputeRaySphereCollision( TestSpherePos, TempRadius, Start, End, FinalT, FinalHitPoint ) )
    {
        return FALSE;
    }

    // they hit, so find the hit point. Final T will be correct
    vector3 MovingSpherePos = FinalHitPoint;
    vector3 Diff = TestSpherePos - MovingSpherePos;
    Diff.Normalize();
    Diff *= MovingSphereRadius;

    FinalHitPoint = MovingSpherePos + Diff;

    return TRUE;
}

void BuildTrisForAABBox( const bbox& AABBox, vector3 Tris[12][3] )
{
    vector3 Corners[8];

    Corners[0].Set( AABBox.Min.X, AABBox.Min.Y, AABBox.Min.Z );
    Corners[1].Set( AABBox.Max.X, AABBox.Min.Y, AABBox.Min.Z );
    Corners[2].Set( AABBox.Max.X, AABBox.Min.Y, AABBox.Max.Z );
    Corners[3].Set( AABBox.Min.X, AABBox.Min.Y, AABBox.Max.Z );
    Corners[4].Set( AABBox.Min.X, AABBox.Max.Y, AABBox.Min.Z );
    Corners[5].Set( AABBox.Max.X, AABBox.Max.Y, AABBox.Min.Z );
    Corners[6].Set( AABBox.Max.X, AABBox.Max.Y, AABBox.Max.Z );
    Corners[7].Set( AABBox.Min.X, AABBox.Max.Y, AABBox.Max.Z );

    Tris[0][0] = Corners[2];
    Tris[0][1] = Corners[7];
    Tris[0][2] = Corners[3];

    Tris[1][0] = Corners[2];
    Tris[1][1] = Corners[6];
    Tris[1][2] = Corners[7];

    Tris[2][0] = Corners[0];
    Tris[2][1] = Corners[3];
    Tris[2][2] = Corners[4];

    Tris[3][0] = Corners[3];
    Tris[3][1] = Corners[7];
    Tris[3][2] = Corners[4];

    Tris[4][0] = Corners[1];
    Tris[4][1] = Corners[0];
    Tris[4][2] = Corners[5];

    Tris[5][0] = Corners[5];
    Tris[5][1] = Corners[0];
    Tris[5][2] = Corners[4];

    Tris[6][0] = Corners[1];
    Tris[6][1] = Corners[6];
    Tris[6][2] = Corners[2];

    Tris[7][0] = Corners[1];
    Tris[7][1] = Corners[5];
    Tris[7][2] = Corners[6];

    Tris[8][0] = Corners[6];
    Tris[8][1] = Corners[4];
    Tris[8][2] = Corners[7];

    Tris[9][0] = Corners[6];
    Tris[9][1] = Corners[5];
    Tris[9][2] = Corners[4];

    Tris[10][0] = Corners[2];
    Tris[10][1] = Corners[3];
    Tris[10][2] = Corners[0];

    Tris[11][0] = Corners[2];
    Tris[11][1] = Corners[0];
    Tris[11][2] = Corners[1];
}

xbool ComputeSphereAABBoxCollision( const bbox&    AABBox, 
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32      Radius,
                                          f32&     FinalT,
                                          vector3& FinalHitPoint,
                                          plane&   FinalHitPlane,
                                          plane&   FinalSlipPlane )
{
    vector3 Tris[12][3];
    BuildTrisForAABBox( AABBox, Tris );

    s32     i;
    f32     FinalTs[6];
    vector3 FinalHitPoints[6];
    plane   FinalHitPlanes[6];
    plane   FinalSlipPlanes[6];
    xbool   HitTriangleEdge;
    s32     nCollisions = 0;

    for ( i = 0; i < 12; ++i )
    {
        if ( ComputeSphereTriCollision(
            Tris[i],
            Start,
            End,
            Radius,
            FinalTs[nCollisions],
            FinalHitPoints[nCollisions],
            HitTriangleEdge ) )
        {
            if ( HitTriangleEdge )
            {
                // Our slide plane is defined by the impact point, and a
                // normal from that point towards the sphere's center,
                // when the sphere is at the collision T
                vector3 SphereCenterAtImpact = Start + (End-Start)*FinalTs[nCollisions];
                vector3 Normal = SphereCenterAtImpact - FinalHitPoints[nCollisions];

#if 0
                g_cdb.SphereCenterAtImpact  = SphereCenterAtImpact;
                g_cdb.Normal                = Normal;
                g_cdb.FinalHitPoint         = FinalHitPoint;
                g_cdb.Start                 = Start;
                g_cdb.End                   = End;
                g_cdb.nCollisions           = nCollisions;
                g_cdb.FinalTs[0]            = FinalTs[0];
                g_cdb.FinalTs[1]            = FinalTs[1];
                g_cdb.FinalTs[2]            = FinalTs[2];
                g_cdb.FinalTs[3]            = FinalTs[3];
                g_cdb.FinalTs[4]            = FinalTs[4];
                g_cdb.FinalTs[5]            = FinalTs[5];
#endif

                Normal.Normalize();
                FinalSlipPlanes[nCollisions].Setup(
                    FinalHitPoints[nCollisions], Normal );
            }
            else
            {
                // The triangle plane is our slide plane
                FinalSlipPlanes[nCollisions].Setup(
                    Tris[i][0],
                    Tris[i][1],
                    Tris[i][2] );
            }
            
            FinalHitPlanes[nCollisions].Setup(
                Tris[i][0],
                Tris[i][1],
                Tris[i][2] );
            
            ++nCollisions;
        }
    }

    if ( nCollisions > 0 )
    {
        // we have a collision, find the one with the smallest final T
        FinalT = 2.0f; // greater than is valid initially

        for ( i = 0; i < nCollisions; ++i )
        {
            if ( FinalTs[i] < FinalT )
            {
                FinalT = FinalTs[i];
                FinalHitPoint = FinalHitPoints[i];
                FinalHitPlane = FinalHitPlanes[i];
                FinalSlipPlane = FinalSlipPlanes[i];
            }
        }

        if ( (FinalT < 0.0f) || (FinalT > 1.0f) )
        {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}
                                    
xbool ComputeRayAABBoxCollision   ( const bbox&    AABBox,
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32&     FinalT,
                                          vector3& FinalHitPoint,
                                          plane&   FinalHitPlane,
                                          plane&   FinalSlipPlane )
{
    vector3 Tris[12][3];
    BuildTrisForAABBox( AABBox, Tris );

//    {
//        BuildTrisForAABBox( AABBox, g_LastBBoxTris );
//        g_LastBBoxSet = TRUE;
//        g_LastRaySelect[0] = Start;
//        g_LastRaySelect[1] = End;
//    }

    s32     i;
    f32     FinalTs[6];
    vector3 FinalHitPoints[6];
    plane   FinalHitPlanes[6];
    s32     nCollisions = 0;

    for ( i = 0; i < 12; ++i )
    {
        if ( ComputeRayTriCollision(
            Tris[i],
            Start,
            End,
            FinalTs[nCollisions],
            FinalHitPoints[nCollisions] ) )
        {
            FinalHitPlanes[nCollisions].Setup(
                Tris[i][0],
                Tris[i][1],
                Tris[i][2] );
            ++nCollisions;
        }
    }

    if ( nCollisions > 0 )
    {
        // we have a collision, find the one with the smallest final T
        FinalT = 2.0f; // greater than is valid initially

        for ( i = 0; i < nCollisions; ++i )
        {
            if ( FinalTs[i] < FinalT )
            {
                FinalT = FinalTs[i];
                FinalHitPoint = FinalHitPoints[i];
                FinalHitPlane = FinalHitPlanes[i];
                FinalSlipPlane = FinalHitPlanes[i];
            }
        }

        if ( (FinalT < 0.0f) || (FinalT > 1.0f) )
        {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}


