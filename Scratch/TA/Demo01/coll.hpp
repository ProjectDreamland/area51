
#ifndef COLL_HPP
#define COLL_HPP
//=========================================================================
// INCLUDES
//=========================================================================
#include "x_files.hpp"
#include "PlatfromIO\PlatformIO.hpp"
#include "CollisionMgr.hpp"

#ifdef EDITOR
class ed_db;
struct ed_db_prop;
class text_out;
#endif
class text_in;

//=========================================================================
// GEOM INTERFACE
//=========================================================================
class coll
{
public:


public:

                    coll            ( void );
                   ~coll            ( void );

        void        Initialize      ( void );

        void        Load            ( X_FILE* Fp );
        void        Save            ( X_FILE* Fp, platform Platform );
#ifdef EDITOR
        s32         GetNumTriFlags  ( void );
#endif
        void        ApplyGeometry   ( void );
        void        Render          ( const matrix4& L2W, const xarray<s32>* Active );
        void        OnGatherTriangles(const matrix4& L2W );
                    
        void        GetBBox         ( bbox& BBox, const matrix4& L2W );

protected:

        xbool           m_hasL2W;   //If m_hasL2W, then do not transform vertices
        static_volume_set   *m_pStaticVolumeSet;
        s32             m_Version;
        platform        m_Platform;

        friend class    rigid_compiler;

//public:
//        struct tri {
//            vector3 P0, P1, P2;
//
//            void    Save( platformio& IO );
//            void    Load( X_FILE* Fp     );
//    
//        };
//

//        s32             m_nTris;
//        tri*            m_pTris;

};

#endif
