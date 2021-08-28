#ifndef COLLISION_PRIMITIVES_HPP
#define COLLISION_PRIMITIVES_HPP

xbool ComputeSphereTriCollision   ( const vector3* Tri, 
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32      Radius,
                                          f32&     FinalT,
                                          vector3& FinalHitPoint,
                                          xbool&   HitTriangleEdge );
                                    
xbool ComputeRayTriCollision      ( const vector3* Tri,
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32&     FinalT,
                                          vector3& FinalHitPoint );
                                    
xbool ComputeRaySphereCollision   ( const vector3& SpherePos,
                                    const f32      SphereRadius, 
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32&     FinalT,
                                          vector3& FinalHitPoint );

xbool ComputeSphereSphereCollision( const vector3& TestSpherePos,
                                    const f32      TestSphereRadius, 
                                    const f32      MovingSphereRadius,
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32&     FinalT,
                                          vector3& FinalHitPoint );

xbool ComputeSphereAABBoxCollision( const bbox&    AABBox, 
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32      Radius,
                                          f32&     FinalT,
                                          vector3& FinalHitPoint,
                                          plane&   FinalHitPlane,
                                          plane&   FinalSlipPlane );
                                    
xbool ComputeRayAABBoxCollision   ( const bbox&    AABBox,
                                    const vector3& Start, 
                                    const vector3& End, 
                                          f32&     FinalT,
                                          vector3& FinalHitPoint,
                                          plane&   FinalHitPlane, 
                                          plane&   FinalSlipPlane );

#endif COLLISION_PRIMITIVES_HPP

