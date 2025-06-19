
#include "RawAnim.hpp"
#include "Parsing\TextIn.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

rawanim::rawanim( void )
{
    x_strcpy( m_SourceFile, "Unknown source file" );
    x_strcpy( m_UserName, "Unknown user" );
    x_strcpy( m_ComputerName, "Unknown computer" );
    m_ExportDate[0] = 0;
    m_ExportDate[1] = 0;
    m_ExportDate[2] = 0;
    m_pBone = NULL;
    m_pFrame = NULL;
    m_pProp = NULL;
    m_pPropFrame = NULL;
    m_pEvent = NULL;
    m_nProps = 0;
    m_nEvents = 0;
    m_nSuperEvents = 0;
    m_nBones = 0;
    m_nFrames = 0;
}

//=========================================================================

rawanim::~rawanim( void )
{    
    if(m_pBone)     delete[]m_pBone;
    if(m_pFrame)    delete[]m_pFrame;
    if(m_pEvent)     delete[]m_pEvent;
    if(m_pProp)     delete[]m_pProp;
    if(m_pPropFrame)     delete[]m_pPropFrame;
}

//=========================================================================


s32 rawanim::CompareBoneNames( s32 iBoneA, s32 iBoneB )
{
    // Lookup bones
    rawanim::bone& BoneA = m_pBone[iBoneA] ;
	rawanim::bone& BoneB = m_pBone[iBoneB] ;
    
    // Hack for now to put the "hand_weapon_attach" bone as low as possible
    // in the bone list since it's always active for all lods!
    
    // Set BoneA name
    char BoneA_Name[256] ;
    if (x_stricmp(BoneA.Name, "hand_weapon_attach") == 0)
        x_strcpy(BoneA_Name, "A_hand_weapon_attach") ;
    else
        x_strcpy(BoneA_Name, BoneA.Name) ;
    
    // Set BoneB name
    char BoneB_Name[256] ;
    if (x_stricmp(BoneB.Name, "hand_weapon_attach") == 0)
        x_strcpy(BoneB_Name, "A_hand_weapon_attach") ;
    else
        x_strcpy(BoneB_Name, BoneB.Name) ;
        
    // If lod groups are specified, use them
    if( BoneA.LODGroup != -1 )
        x_sprintf( BoneA_Name, "%d", BoneA.LODGroup );
    if( BoneB.LODGroup != -1 )
        x_sprintf( BoneB_Name, "%d", BoneB.LODGroup );

    // Depths match, so now sort by name - setup by artists
    return x_strcmp(BoneA_Name, BoneB_Name) ;
}

//=========================================================================

s32 rawanim::CompareBoneDepths( s32 iBoneA, s32 iBoneB )
{
    // Lookup bones
    rawanim::bone& BoneA = m_pBone[iBoneA] ;
	rawanim::bone& BoneB = m_pBone[iBoneB] ;

    // Sort by depth
    if (BoneA.Depth < BoneB.Depth)
        return -1 ;
    else
    if (BoneA.Depth > BoneB.Depth)
        return 1 ;
    else
        return 0 ;
}

//=========================================================================

xbool rawanim::AreBonesFromSameBranch( s32 iBoneA, s32 iBoneB )
{
    s32 iParent ;

    // Check for finding B in A
    iParent = GetBoneIDFromName(m_pBone[iBoneA].ParentName) ;
    while(iParent != -1)
    {
        if (iParent == iBoneB)
            return TRUE ;

        iParent = GetBoneIDFromName(m_pBone[iParent].ParentName) ;
    }

    // Check for finding A in B
    iParent = GetBoneIDFromName(m_pBone[iBoneB].ParentName) ;
    while(iParent != -1)
    {
        if (iParent == iBoneA)
            return TRUE ;

        iParent = GetBoneIDFromName(m_pBone[iParent].ParentName) ;
    }

    return FALSE ;
}

//=========================================================================

// Gets bones in optimzal order ready for skeleton LOD
void rawanim::PutBonesInLODOrder( void )
{
    s32 i,j ; 

    // Prepare bones for sort
    for (i = 0 ; i < m_nBones ; i++)
    {
        // Setup indices
        m_pBone[i].iBone      = i ;
        m_pBone[i].iBoneRemap = i ;

        // Setup parent name
        s32 iParent = m_pBone[i].iParent ;
        if (iParent != -1)
            x_strcpy(m_pBone[i].ParentName, m_pBone[m_pBone[i].iParent].Name) ;
        else
            m_pBone[i].ParentName[0] = 0 ;

        // Setup depths
        m_pBone[i].Depth = 0 ;
        iParent = m_pBone[i].iParent ;
        while(iParent != -1)
        {
            m_pBone[i].Depth++ ;
            iParent = m_pBone[iParent].iParent ;
        }
    }

    // Are lod groups specified?
    xbool bLODGroupsPresent = FALSE;
    for (i = 0 ; i < m_nBones ; i++)
    {
        if( m_pBone[i].LODGroup != -1 )
        {
            bLODGroupsPresent = TRUE;
            break;
        }
    }

    // Check to make sure lod groups are valid
    if( bLODGroupsPresent )
    {
        // Check all bones
        for (i = 0 ; i < m_nBones ; i++)
        {
            bone& Bone = m_pBone[i];
            
            // Make sure lod group is present
            if( Bone.LODGroup == -1 )
            {
                x_throw( xfs( "Bone[%s] has missing custom attribute \"BoneLODGroup\" in max file - add and re-export!",
                              Bone.Name ) );
            }
            
            // Make sure group is after parents
            if( Bone.iParent != -1 )
            {
                // Bone lod group should not be before parents
                bone& Parent = m_pBone[Bone.iParent];
                if( Bone.LODGroup < Parent.LODGroup )
                {
                    x_throw( xfs( "Bone[%s], LODGroup[%d] has invalid lod group before Parent[%s], LODGroup[%d] !",
                                  Bone.Name, Bone.LODGroup, Parent.Name, Parent.LODGroup ) );
                }
            }
        }
    }
        
    // Bubble sort by name
    for (i = 0 ; i < m_nBones ; i++)
    {
        for (j = 1 ; j < m_nBones ; j++)
        {
            // Lookup bone info
            s32 iBoneA   = j-1 ;
            s32 iBoneB   = j ;
            s32 iParentA = GetBoneIDFromName(m_pBone[iBoneA].ParentName) ;
            s32 iParentB = GetBoneIDFromName(m_pBone[iBoneB].ParentName) ;

            // Only allow swap if the parent bones will still be above the children
            if (        (iParentB < iBoneA)
                    &&  (iParentA < iBoneB)  
                    &&  (CompareBoneNames(iBoneB, iBoneA) < 0) )
            {
                // Swap
                bone BoneA = m_pBone[iBoneA] ;
                bone BoneB = m_pBone[iBoneB] ;
                m_pBone[iBoneA] = BoneB ;
                m_pBone[iBoneB] = BoneA ;
            }
        }
    }

    // Setup remap bone indices
    for (i = 0 ; i < m_nBones ; i++)
        m_pBone[m_pBone[i].iBone].iBoneRemap = i ;

    // Setup new indices
    for (i = 0 ; i < m_nBones ; i++)
        m_pBone[i].iBone = i ;

    // Remap parent indices
    for (i = 0 ; i < m_nBones ; i++)
    {
        s32 iParent = m_pBone[i].iParent ;
        if (iParent != -1)
            m_pBone[i].iParent = m_pBone[iParent].iBoneRemap ;
    }

    // Validate
    for (i = 0 ; i < m_nBones ; i++)
    {
        // Parent should always be above child!
        if (m_pBone[i].iParent >= i)
            x_throw("Contact Steve Broumley - his lod bone sort code has failed!!!\n") ;
    }
}

//=========================================================================

void rawanim::PrintHierarchy( void )
{
    // Show hierarchy in debug window
    x_printf("\nHierarchy is %d Bones:\n", m_nBones) ;
    for( s32 i = 0; i < m_nBones; i++ )
    {
        // Print bone index
        x_printf( "%3d: ", i );
        
        // Print indent
        s32 iParent = m_pBone[i].iParent ;
        while(iParent != -1)
        {
            x_printf(" ") ;
            iParent = m_pBone[iParent].iParent ;
        }

        // Print name
        iParent = m_pBone[i].iParent ;
        ASSERT(iParent < i) ;
        if (iParent != -1)
            x_printf("\"%s\" (Parent=\"%s\", LODGroup=%d)\n", 
                        m_pBone[i].Name, 
                        m_pBone[iParent].Name,
                        m_pBone[i].LODGroup );
        else
        {
            x_printf("\"%s\" (LODGroup=%d)\n", 
                     m_pBone[i].Name,
                     m_pBone[i].LODGroup );
        }            
    }
    x_printf("\n") ;
}

//=========================================================================

xbool rawanim::Load( const char* pFileName )
{
    text_in File;

    if (pFileName)
        x_strcpy(m_SourceFile, pFileName);

    //
    // Open the file
    //
    if( File.OpenFile( pFileName ) == NULL ) 
    {
        return FALSE;
    }

    while( File.ReadHeader() == TRUE )
    {
        if( x_stricmp( File.GetHeaderName(), "TimeStamp" ) == 0 )
        {
            if( File.ReadFields() == FALSE )
            {
                return FALSE;
            }

            f32 Time[3];
            File.GetField( "Time:ddd", &Time[0], &Time[1], &Time[2] );
            File.GetField( "Date:ddd", &m_ExportDate[0], &m_ExportDate[1], &m_ExportDate[2] );
        }
        else if( x_stricmp( File.GetHeaderName(), "UserInfo" ) == 0 )
        {
            if( File.ReadFields() == FALSE )
            {
                return FALSE;
            }

            File.GetField( "UserName:s",     m_UserName );
            File.GetField( "ComputerName:s", m_ComputerName );
        }
        else if( x_stricmp( File.GetHeaderName(), "Hierarchy" ) == 0 )
        {
            m_nBones = File.GetHeaderCount();
            if (m_nBones <= 0)
            {
                x_throw( xfs("Animation [%s] has no bones - re-export the .matx file with the correct settings... Compile will fail\n", m_SourceFile ) ); 
                return FALSE;
            }

            m_pBone = new bone[ m_nBones ];
            ASSERT( m_pBone );

            for( s32 i=0; i<m_nBones; i++ )
            {
                File.ReadFields();

                File.GetS32         ("Index",       m_pBone[i].iBone);
                File.GetString      ("Name",        m_pBone[i].Name);
                File.GetS32         ("nChildren",   m_pBone[i].nChildren);
                File.GetS32         ("iParent",     m_pBone[i].iParent);
                File.GetVector3     ("Scale",       m_pBone[i].BindScale);
                File.GetQuaternion  ("Rotate",      m_pBone[i].BindRotation);
                File.GetVector3     ("Pos",         m_pBone[i].BindTranslation);
                File.GetS32         ("bScaleKeys",  m_pBone[i].bScaleKeys);
                File.GetS32         ("bRotKeys",    m_pBone[i].bRotationKeys);
                File.GetS32         ("bPosKeys",    m_pBone[i].bTranslationKeys);

                // read in if bone is part of a layer
                m_pBone[i].bIsMasked = FALSE;
                File.GetS32("bIsMasked",m_pBone[i].bIsMasked);

                // Grab LOD group
                m_pBone[i].LODGroup = -1;
                File.GetS32( "LODGroup", m_pBone[i].LODGroup );

                // Build bind matrices
                m_pBone[i].BindMatrix.Setup( m_pBone[i].BindScale,
                                             m_pBone[i].BindRotation,
                                             m_pBone[i].BindTranslation);
                m_pBone[i].BindMatrixInv = m_pBone[i].BindMatrix;
                m_pBone[i].BindMatrixInv.InvertSRT();
            }

            // Re-order the bones before rest of matx file is loaded
            PutBonesInLODOrder() ;
        }
        else
        if( x_stricmp( File.GetHeaderName(), "Animation" ) == 0 )
        {
            s32 nKeys          = File.GetHeaderCount();
            s32 nFramesInBatch = nKeys / m_nBones;
            s32 OldNFrames     = m_nFrames;
            
            // Alloc new data
            m_nFrames += nFramesInBatch;
            frame* pNewFrame = new frame[m_nFrames*m_nBones] ;
            
            // Copy old data?
            if (m_pFrame)
            {
                x_memcpy(pNewFrame, m_pFrame, sizeof(frame)*OldNFrames*m_nBones) ;
                delete [] m_pFrame ;
            }

            // Assign new data
            m_pFrame = pNewFrame ;
            ASSERT( m_pFrame );

            // Data for each loop
            s32         iFrame;
            char        BoneName[256];
            s32         iBone;
            vector3     Scale;
            quaternion  Rot;
            vector3     Pos;
            s32         I = OldNFrames*m_nBones;

            for( s32 i=0; i<nKeys; i++ )
            {
                File.ReadFields();

                File.GetField( "Frame:d",     &iFrame    );
                File.GetField( "BoneName:s",  BoneName  );
                File.GetField( "BoneIndex:d", &iBone     );
                File.GetField( "Scale:fff",   &Scale.GetX(), &Scale.GetY(), &Scale.GetZ() );
                File.GetField( "Rotate:ffff", &Rot.X, &Rot.Y, &Rot.Z, &Rot.W );
                File.GetField( "Pos:fff",     &Pos.GetX(), &Pos.GetY(), &Pos.GetZ() );

                ASSERT(Scale.GetX() >= (-S32_MAX)) ;
                ASSERT(Scale.GetX() <= (+S32_MAX)) ;
                ASSERT(Scale.GetY() >= (-S32_MAX)) ;
                ASSERT(Scale.GetY() <= (+S32_MAX)) ;
                ASSERT(Scale.GetZ() >= (-S32_MAX)) ;
                ASSERT(Scale.GetZ() <= (+S32_MAX)) ;

                // Verify data
                ASSERT( iFrame >= 0 ) ;
                ASSERT( iFrame < m_nFrames ) ;

                ASSERT( iFrame == ( I / m_nBones ));
                ASSERT( iBone ==  ( I % m_nBones ));

                // Lookup the remapped bone index
                iBone = m_pBone[iBone].iBoneRemap ;

                ASSERT( x_stricmp(BoneName,m_pBone[iBone].Name) == 0 );

                // Compute key frame index
                s32 iKeyFrame = (iFrame * m_nBones) + iBone ;
                ASSERT(iKeyFrame >= 0) ;
                ASSERT(iKeyFrame < (m_nFrames * m_nBones)) ;

                m_pFrame[iKeyFrame].Scale       = Scale;
                m_pFrame[iKeyFrame].Rotation    = Rot;
                m_pFrame[iKeyFrame].Translation = Pos;

                I++ ;
            }
        }
        else
		if( x_stricmp( File.GetHeaderName(), "Events" ) == 0 )
		{
			m_nEvents = File.GetHeaderCount();
			m_pEvent = new event[ m_nEvents ];
			ASSERT( m_pEvent );

			for( s32 i=0; i<m_nEvents; i++ )
			{
				event& E = m_pEvent[i];

				File.ReadFields();

                File.GetString  ("Name",E.Name);
                File.GetS32     ("Type",E.Type);
				File.GetF32		("Radius",E.Radius);
				File.GetS32		("Start",E.Frame0);
				File.GetS32		("End",E.Frame1);
				File.GetString	("ParentName",E.ParentName);
				File.GetVector3	("Pos",E.Position);
			}
		}
        else
		if( x_stricmp( File.GetHeaderName(), "SuperEvents" ) == 0 )
		{
			m_nSuperEvents = File.GetHeaderCount();
			m_pSuperEvent = new super_event[ m_nSuperEvents ];
			ASSERT( m_pSuperEvent );

			for( s32 i=0; i<m_nSuperEvents; i++ )
			{
				super_event& E = m_pSuperEvent[i];

				File.ReadFields();

                File.GetString  ("Name",E.Name);
                File.GetS32     ("Type",E.Type);
				File.GetF32		("Radius",E.Radius);
				File.GetS32		("Start",E.StartFrame);
				File.GetS32		("End",E.EndFrame);
				File.GetString	("ParentName",E.ParentName);
				File.GetVector3	("Pos",E.Position);

                //File.GetRadian3 ("RotationPitchYawRoll", E.Rotation);
                E.Rotation.Identity();
                File.GetQuaternion ("Rotation", E.Rotation);

                File.GetBool    ("ShowAxis", E.ShowAxis);
                File.GetBool    ("ShowSphere", E.ShowSphere);
                File.GetBool    ("ShowBox", E.ShowBox);
                File.GetF32     ("AxisSize", E.AxisSize);
                File.GetF32     ("Width", E.Width);
                File.GetF32     ("Length", E.Length);
                File.GetF32     ("Height", E.Height);
                {
                    s32     i;
                    char    Title[20];

                    for ( i = 0; i < NUM_EVENT_STRINGS; ++i )
                    {
                        x_sprintf( Title, "String%i", i+1 );
                        File.GetString( Title, E.Strings[i] );
                    }
                    for ( i = 0; i < NUM_EVENT_INTS; ++i )
                    {
                        x_sprintf( Title, "Int%i", i+1 );
                        File.GetS32( Title, E.Ints[i] );
                    }
                    for ( i = 0; i < NUM_EVENT_FLOATS; ++i )
                    {
                        x_sprintf( Title, "Float%i", i+1 );
                        File.GetF32( Title, E.Floats[i] );
                    }
                    for ( i = 0; i < NUM_EVENT_BOOLS; ++i )
                    {
                        x_sprintf( Title, "Bool%i", i+1 );
                        File.GetBool( Title, E.Bools[i] );
                    }
                    for ( i = 0; i < NUM_EVENT_COLORS; ++i )
                    {
                        x_sprintf( Title, "Color%i", i+1 );
                        File.GetColor( Title, E.Colors[i] );
                    }
                }
			}
		}
        else
		if( x_stricmp( File.GetHeaderName(), "Props" ) == 0 )
		{
			m_nProps = File.GetHeaderCount();
			m_pProp = new prop[ m_nProps ];
			ASSERT( m_pProp );

			for( s32 i=0; i<m_nProps; i++ )
			{
				prop& E = m_pProp[i];

				File.ReadFields();

                File.GetString  ("Name",E.Name);
                if(!File.GetString  ("Type",E.Type))
                {
                    x_strcpy(E.Type , "Weapon");
                }
				File.GetString	("Parent",E.ParentName);
			}
		}
        else
        if( x_stricmp( File.GetHeaderName(), "PropFrames" ) == 0 )
        {
            s32 nKeys  = File.GetHeaderCount();
            m_nPropFrames  = nKeys / m_nProps;
            m_pPropFrame   = new prop_frame[ m_nPropFrames * m_nProps ];
            ASSERT( m_pPropFrame );

            // Data for each loop
            s32         iFrame;
            char        PropName[256];
            s32         iProp;
            vector3     Scale;
            quaternion  Rot;
            vector3     Pos;
            xbool       bVis;

            for( s32 i=0; i<nKeys; i++ )
            {
                File.ReadFields();

                File.GetField( "Frame:d",     &iFrame    );
                File.GetField( "Index:d",      &iProp );
                File.GetField( "AnimPropName:s",   PropName  );
                File.GetField( "Scale:fff",   &Scale.GetX(), &Scale.GetY(), &Scale.GetZ() );
                File.GetField( "Rotate:ffff",    &Rot.X, &Rot.Y, &Rot.Z, &Rot.W );
                File.GetField( "Pos:fff",     &Pos.GetX(), &Pos.GetY(), &Pos.GetZ() );
                File.GetField( "Vis:d",        &bVis );

                // Verify data
                ASSERT( iFrame == ( i / m_nProps ));
                ASSERT( iProp ==  ( i % m_nProps ));
                ASSERT( x_stricmp(PropName,m_pProp[iProp].Name) == 0 );

                m_pPropFrame[i].Scale = Scale;
                m_pPropFrame[i].Rotation = Rot;
                m_pPropFrame[i].Translation = Pos;
                m_pPropFrame[i].bVisible = bVis;
			}
		}
		else
        {
            File.SkipToNextHeader();
        }
    }

    // Make sure that we have rich the end of the file
    if( File.IsEOF() == FALSE )
        return FALSE;

    // Fill out dead animation
    if( m_nFrames==0 )
    {
        m_nFrames = 2;
        m_pFrame   = new frame[ m_nFrames * m_nBones ];
        ASSERT( m_pFrame );

        // Data for each loop
        for( s32 i=0; i<m_nFrames * m_nBones; i++ )
        {
            m_pFrame[i].Scale = vector3(1,1,1);
            m_pFrame[i].Rotation.Identity();
            m_pFrame[i].Translation.Zero();
        }
    }

    SanityCheck();
    return TRUE;
}

//=========================================================================

void rawanim::ComputeBonesL2W( matrix4* pMatrix, f32 Frame )
{
    s32 i;

    // Keep frame in range
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));

    // Compute integer and 
    s32 iFrame0 = (s32)Frame;
    s32 iFrame1 = (iFrame0+1)%m_nFrames;
    f32 fFrame  = Frame - iFrame0;

    // Loop through bones and build matrices
    frame* pF0 = &m_pFrame[ iFrame0*m_nBones ];
    frame* pF1 = &m_pFrame[ iFrame1*m_nBones ];
    for( i=0; i<m_nBones; i++ )
    {
        quaternion R = Blend( pF0->Rotation, pF1->Rotation, fFrame );
        vector3    S = pF0->Scale       + fFrame*(pF1->Scale       - pF0->Scale);
        vector3    T = pF0->Translation + fFrame*(pF1->Translation - pF0->Translation);
        pF0++;
        pF1++;
        pMatrix[i].Setup( S, R, T );

        // Concatenate with parent
        if( m_pBone[i].iParent != -1 )
            pMatrix[i] = pMatrix[m_pBone[i].iParent] * pMatrix[i];
    }

    // Apply bind matrices
    for( i=0; i<m_nBones; i++ )
    {
        pMatrix[i] = pMatrix[i] * m_pBone[i].BindMatrixInv;
    }
}

//=========================================================================

void rawanim::GetMotionPropFrame( s32           iFrame,
                                  vector3&      Scale, 
                                  vector3&      Trans, 
                                  quaternion&   Rot )
{
    // Keep frame in range
    iFrame = iFrame % ( m_nFrames - 1 );

    // Setup defaults
    Scale.Set( 1.0f, 1.0f, 1.0f );
    Trans.Set( 0.0f, 0.0f, 0.0f );
    Rot.Identity();

    // Search for motion prop
    for( s32 iProp = 0; iProp < m_nProps; iProp++ )
    {
        // Lookup prop
        prop& Prop = m_pProp[ iProp ];
        if( x_stricmp( Prop.Name, "MotionProp" ) == 0 )
        {
            // Lookup prop frame
            prop_frame* pPropFrame = &m_pPropFrame[ iFrame * m_nProps ];
            
            // Extract the key
            Scale = pPropFrame->Scale;
            Trans = pPropFrame->Translation;
            Rot   = pPropFrame->Rotation;
            return;
        }
    }        

    // "MotionProp" is not present, so just use the root bone
    frame* pFrame = &m_pFrame[ iFrame * m_nBones ];
    Scale = pFrame->Scale;
    Trans = pFrame->Translation;
    Rot   = pFrame->Rotation;             
}                                  

//=========================================================================

void rawanim::ComputeBonesL2W( matrix4* pMatrix, 
                               s32      iFrame,
                               xbool    bRemoveHorizMotion,
                               xbool    bRemoveVertMotion,
                               xbool    bRemoveYawMotion )
{
    s32 i;

    // Keep frame in range
    iFrame = iFrame % ( m_nFrames - 1 );

    // Loop through bones and build matrices
    frame* pFrame = &m_pFrame[ iFrame * m_nBones ];
    for( i=0; i<m_nBones; i++ )
    {
        // Lookup bone frame
        vector3    Scale( pFrame->Scale );
        vector3    Trans( pFrame->Translation );
        quaternion Rot  ( pFrame->Rotation );
        
        // Root bone?
        if( i == 0 )
        {
            // Get motion prop frame (will use root bone if motion prop not present)
            vector3    MotionPropScale;
            vector3    MotionPropTrans;
            quaternion MotionPropRot;
            GetMotionPropFrame( iFrame, MotionPropScale, MotionPropTrans, MotionPropRot );

            // Remove horiz motion?
            if( bRemoveHorizMotion )
            {
                Trans.GetX() -= MotionPropTrans.GetX();
                Trans.GetZ() -= MotionPropTrans.GetZ();
            }                

            // Remove vert motion?
            if( bRemoveVertMotion )
            {
                Trans.GetY() -= MotionPropTrans.GetY();
            }
            
            // Remove yaw motion?
            if( bRemoveYawMotion )
            {
                // Compute yaws
                radian Yaw           = Rot.GetRotation().Yaw;
                radian MotionPropYaw = MotionPropRot.GetRotation().Yaw;
                
                // Compute difference
                quaternion DeltaYaw( vector3( 0.0f, 1.0f, 0.0f ), Yaw - MotionPropYaw );
                
                // Remove motion yaw from bone yaw
                Rot = DeltaYaw * Rot;
            }
        }
        
        // Setup matrix from frame
        pMatrix[i].Setup( Scale, Rot, Trans );
        
        // Next bone
        pFrame++;

        // Concatenate with parent
        if( m_pBone[i].iParent != -1 )
            pMatrix[i] = pMatrix[m_pBone[i].iParent] * pMatrix[i];
    }

    // Apply bind matrices
    for( i=0; i<m_nBones; i++ )
    {
        pMatrix[i] = pMatrix[i] * m_pBone[i].BindMatrixInv;
    }
}

//=========================================================================

void rawanim::ComputeBoneL2W( s32 iBone, matrix4& Matrix, f32 Frame )
{
    // Keep frame in range
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));

    // Compute integer and 
    s32 iFrame0 = (s32)Frame;
    s32 iFrame1 = (iFrame0+1)%m_nFrames;
    f32 fFrame  = Frame - iFrame0;

    // Loop through bones and build matrices
    frame* pF0 = &m_pFrame[ iFrame0*m_nBones ];
    frame* pF1 = &m_pFrame[ iFrame1*m_nBones ];

    // Clear bone matrix
    Matrix.Identity();

    // Run hierarchy from bone to root node
    s32 I = iBone;
    while( I != -1 )
    {
        quaternion R = Blend( pF0[I].Rotation, pF1[I].Rotation, fFrame );
        vector3    S = pF0[I].Scale       + fFrame*(pF1[I].Scale       - pF0[I].Scale);
        vector3    T = pF0[I].Translation + fFrame*(pF1[I].Translation - pF0[I].Translation);

        matrix4 LM;
        LM.Setup( S, R, T );

        Matrix = LM * Matrix;
        I = m_pBone[I].iParent;
    }

    // Apply bind matrix
    Matrix = Matrix * m_pBone[iBone].BindMatrixInv;
}

//=========================================================================

void rawanim::ComputeRawBoneL2W( s32 iBone, matrix4& Matrix, s32 iFrame )
{
    // Keep frame in range
    ASSERT( (iFrame>=0) && (iFrame<m_nFrames) );

    // Loop through bones and build matrices
    frame* pF = &m_pFrame[ iFrame*m_nBones ];

    // Clear bone matrix
    Matrix.Identity();

    // Run hierarchy from bone to root node
    s32 I = iBone;
    while( I != -1 )
    {
        quaternion R = pF[I].Rotation;
        vector3    S = pF[I].Scale;
        vector3    T = pF[I].Translation;

        matrix4 LM;
        LM.Setup( S, R, T );

        Matrix = LM * Matrix;
        I = m_pBone[I].iParent;
    }

    // Apply bind matrix
    Matrix = Matrix * m_pBone[iBone].BindMatrixInv;
}

//=========================================================================

void rawanim::ComputeBoneKeys( quaternion* pQ, vector3* pS, vector3* pT, f32 Frame )
{
    s32 i;

    // Keep frame in range
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));

    // Compute integer and 
    s32 iFrame0 = (s32)Frame;
    s32 iFrame1 = (iFrame0+1)%m_nFrames;
    f32 fFrame  = Frame - iFrame0;

    // Loop through bones and build matrices
    frame* pF0 = &m_pFrame[ iFrame0*m_nBones ];
    frame* pF1 = &m_pFrame[ iFrame1*m_nBones ];
    for( i=0; i<m_nBones; i++ )
    {
        pQ[i] = Blend( pF0->Rotation, pF1->Rotation, fFrame );
        pS[i] = pF0->Scale       + fFrame*(pF1->Scale       - pF0->Scale);
        pT[i] = pF0->Translation + fFrame*(pF1->Translation - pF0->Translation);
        pF0++;
        pF1++;
    }
}

//=========================================================================

s32 rawanim::GetBoneIDFromName( const char* pBoneName )
{
    s32 i;
    for( i=0; i<m_nBones; i++ )
    if( x_stricmp(pBoneName,m_pBone[i].Name) == 0 )
        return i;
    return -1;
}

//=========================================================================

void rawanim::DumpFrames( const char* pFileName, xbool InBoneOrder )
{
    s32 i,j;

    X_FILE* fp = x_fopen(pFileName,"wt");
    if( !fp ) return;

    if( InBoneOrder )
    {
        for( j=0; j<m_nBones; j++ )
        {
            for( i=0; i<m_nFrames; i++ )
            {
                frame* pF = &m_pFrame[ i*m_nBones + j ];
                x_fprintf(fp,"%3d %32s ",j,m_pBone[j].Name);

                x_fprintf(fp,"   |   %8.5f %8.5f %8.5f %8.5f",
                    pF->Rotation.X,
                    pF->Rotation.Y,
                    pF->Rotation.Z,
                    pF->Rotation.W);

                x_fprintf(fp,"   |   %8.2f %8.2f %8.2f",
                    pF->Translation.GetX(),
                    pF->Translation.GetY(),
                    pF->Translation.GetZ());

                x_fprintf(fp,"   |   %4.2f %4.2f %4.2f\n",
                    pF->Scale.GetX(),
                    pF->Scale.GetY(),
                    pF->Scale.GetZ());
            }
        }
    }
    else
    {
        for( i=0; i<m_nFrames; i++ )
        {
            x_fprintf(fp,"FRAME: %1d\n",i);
            for( j=0; j<m_nBones; j++ )
            {
                frame* pF = &m_pFrame[ i*m_nBones + j ];
                x_fprintf(fp,"%3d %32s ",j,m_pBone[j].Name);

                x_fprintf(fp,"   |   %8.5f %8.5f %8.5f %8.5f",
                    pF->Rotation.X,
                    pF->Rotation.Y,
                    pF->Rotation.Z,
                    pF->Rotation.W);

                x_fprintf(fp,"   |   %8.2f %8.2f %8.2f",
                    pF->Translation.GetX(),
                    pF->Translation.GetY(),
                    pF->Translation.GetZ());

                x_fprintf(fp,"   |   %4.2f %4.2f %4.2f\n",
                    pF->Scale.GetX(),
                    pF->Scale.GetY(),
                    pF->Scale.GetZ());
            }
        }
    }

    x_fclose(fp);
}

//=========================================================================

void rawanim::BakeBindingIntoFrames( xbool DoScale, xbool DoRotation, xbool DoTranslation )
{
    s32 i,j;

    //
    // Loop through frames of animation
    //
    matrix4* pL2W = (matrix4*)x_malloc(sizeof(matrix4)*m_nBones);
    for( i=0; i<m_nFrames; i++ )
    {
        //
        // Compute matrices for current animation.
        // No binding is applied
        //
        for( j=0; j<m_nBones; j++ )
        {
            frame* pF = &m_pFrame[ i*m_nBones+j ];

            pL2W[j].Setup( pF->Scale, pF->Rotation, pF->Translation );

            // Concatenate with parent
            if( m_pBone[j].iParent != -1 )
            {
                pL2W[j] = pL2W[m_pBone[j].iParent] * pL2W[j];
            }
        }

        //
        // Apply original bind matrices
        //
        for( j=0; j<m_nBones; j++ )
        {
            pL2W[j] = pL2W[j] * m_pBone[j].BindMatrixInv;
        }

        //
        // Remove bind translation and scale matrices
        //

        for( j=0; j<m_nBones; j++ )
        {
            quaternion R = m_pBone[j].BindRotation;
            vector3    S = m_pBone[j].BindScale;
            vector3    T = m_pBone[j].BindTranslation;

            if( DoScale ) S(1,1,1);
            if( DoTranslation ) T(0,0,0);
            if( DoRotation ) R.Identity();

            matrix4 BindMatrix;
            BindMatrix.Setup( S, R, T );
            pL2W[j] = pL2W[j] * BindMatrix;
        }


        // Convert back to local space transform
        for( j=m_nBones-1; j>0; j-- )
        if( m_pBone[j].iParent != -1 )
        {
            matrix4 PM = pL2W[ m_pBone[j].iParent ];
            PM.InvertSRT();
            pL2W[j] = PM * pL2W[j];
        }

        // Pull out rotation scale and translation
        for( j=0; j<m_nBones; j++ )
        {
            frame* pF       = &m_pFrame[i * m_nBones + j ];
            pL2W[j].DecomposeSRT( pF->Scale, pF->Rotation, pF->Translation );
        }
    }
    x_free(pL2W);

    // Remove rotation from binding
    for( i=0; i<m_nBones; i++ )
    {
        if( DoScale ) 
            m_pBone[i].BindScale(1,1,1);

        if( DoRotation )
            m_pBone[i].BindRotation.Identity();

        if( DoTranslation )
            m_pBone[i].BindTranslation.Zero();

        m_pBone[i].BindMatrix.Setup( m_pBone[i].BindScale, m_pBone[i].BindRotation, m_pBone[i].BindTranslation );
        m_pBone[i].BindMatrixInv = m_pBone[i].BindMatrix;
        m_pBone[i].BindMatrixInv.InvertSRT();
    }
}

//=========================================================================

void rawanim::DeleteDummyBones( void )
{
    s32 iBone = 0;
    while(iBone < m_nBones)
    {
        xstring S(m_pBone[iBone].Name);
        S.MakeLower();
        if(S.Find("dummy") != -1)
        {
            //Check if it is the root.  If it is, make sure it is not the only root; that is,
            // we can only delete a root bone if it only has one child (because then its child
            // can become the new root)
            if(m_pBone[iBone].iParent == -1)
            {
                s32 nChildren = 0;
                for(s32 count = 0; count < m_nBones; count++)
                {
                    if(m_pBone[count].iParent == iBone)
                        nChildren++;
                }
                if(nChildren == 1)
                {
                    //x_DebugMsg("Bone is root, but can be removed: '%s'\n", m_pBone[iBone].Name);                
                    DeleteBone(iBone);
                    iBone = 0;
                }
                else
                {
                    //x_DebugMsg("Bone is sole remaining root: '%s'\n", m_pBone[iBone].Name);
                    iBone++;
                }
            }
            else
            {
                DeleteBone(iBone);
                iBone = 0;
            }
        }
        else
        {
            iBone++;
        }
    }

/*
    for(iBone = 0; iBone < m_nBones; iBone++)
    {
        x_DebugMsg("Bone Index: %3d Parent: %3d Name: '%s'\n", iBone, m_pBone[iBone].iParent, m_pBone[iBone].Name);
    }
*/
}
    
//=========================================================================

void rawanim::DeleteBone          ( const char* pBoneName )
{
    s32 iBone = this->GetBoneIDFromName(pBoneName);
    if(iBone != -1)
        DeleteBone(iBone);
    return;
}

//=========================================================================

void rawanim::DeleteBone( s32 iBone )
{
    //x_DebugMsg("Deleting bone: '%s'\n", m_pBone[iBone].Name);
    s32 i,j;
    
    ASSERTS( iBone < m_nBones, m_SourceFile );


    //
    // Allocate new bones and frames
    //
    s32 nNewBones = m_nBones-1;
    bone* pNewBone = new bone[ nNewBones ];
    frame* pNewFrame = new frame[ nNewBones * m_nFrames ];
    ASSERT( pNewBone && pNewFrame );

    //
    // Check and see if bone has any children
    //
    xbool HasChildren = FALSE;
    for( i=0; i<m_nBones; i++ )
    if( m_pBone[i].iParent == iBone )
        HasChildren = TRUE;

    //
    // Build new hierarchy
    //
    {
        // Copy over remaining bones
        j=0;
        for( i=0; i<m_nBones; i++ )
        if( i != iBone )
        {
            pNewBone[j] = m_pBone[i];
            j++;
        }

        // Patch children of bone
        for( i=0; i<nNewBones; i++ )
        if( pNewBone[i].iParent == iBone )
        {
            pNewBone[i].iParent = m_pBone[iBone].iParent;
        }

        // Patch references to any bone > iBone
        for( i=0; i<nNewBones; i++ )
        if( pNewBone[i].iParent > iBone )
        {
            pNewBone[i].iParent--;
        }
    }


    // 
    // If there were no children then we can quickly copy over the keys
    //

    if( !HasChildren )
    {
        //
        // Loop through frames of animation
        //
        s32 k=0;
        for( i=0; i<m_nFrames; i++ )
        for( j=0; j<m_nBones; j++ )
        {
            if( j!=iBone )
                pNewFrame[k++] = m_pFrame[ i*m_nBones + j ];
        }
    }
    else
    {
        //
        // Loop through frames of animation
        //
        matrix4* pL2W = (matrix4*)x_malloc(sizeof(matrix4)*m_nBones);
        for( i=0; i<m_nFrames; i++ )
        {
            // Compute matrices for current animation.
            for( j=0; j<m_nBones; j++ )
            {
                frame* pF = &m_pFrame[ i*m_nBones+j ];

                pL2W[j].Setup( pF->Scale, pF->Rotation, pF->Translation );

                // Concatenate with parent
                if( m_pBone[j].iParent != -1 )
                {
                    pL2W[j] = pL2W[m_pBone[j].iParent] * pL2W[j];
                }
            }

            // Apply original bind matrices
            for( j=0; j<m_nBones; j++ )
            {
                pL2W[j] = pL2W[j] * m_pBone[j].BindMatrixInv;
            }

            // Shift bones down to align with NewBones
            for( j=iBone+1; j<m_nBones; j++ )
                pL2W[j-1] = pL2W[j];


            // Remove bind translation and scale matrices
            for( j=0; j<nNewBones; j++ )
            {
                pL2W[j] = pL2W[j] * pNewBone[j].BindMatrix;
            }

            // Convert back to local space transform
            for( j=nNewBones-1; j>0; j-- )
            if( pNewBone[j].iParent != -1 )
            {
                matrix4 PM = pL2W[ pNewBone[j].iParent ];
                PM.InvertSRT();
                pL2W[j] = PM * pL2W[j];
            }

            // Pull out rotation scale and translation
            for( j=0; j<nNewBones; j++ )
            {
                frame* pF       = &pNewFrame[i*nNewBones + j];
                pL2W[j].DecomposeSRT( pF->Scale, pF->Rotation, pF->Translation );
            }
        }
        x_free(pL2W);
    }

    // free current allocations
    delete[] m_pBone;
    delete[] m_pFrame;

    m_nBones = nNewBones;
    m_pBone = pNewBone;
    m_pFrame = pNewFrame;
    //x_DebugMsg("After delete, m_nBones = %d\n", m_nBones);
    
}

//=========================================================================

xbool rawanim::ApplyNewSkeleton( const rawanim& BindAnim )
{
    s32 i,j,k;
    xbool Problem=FALSE;

    //
    // Remove all bones not in BindAnim
    //
	i = 0;
	while( i < m_nBones )
	{
		for( j=0; j<BindAnim.m_nBones; j++ )
		if( x_stricmp( m_pBone[i].Name, BindAnim.m_pBone[j].Name ) == 0 )
			break;

		if( j==BindAnim.m_nBones )
		{
			if( m_nBones==1 )
			{
                x_throw( xfs("[%s] has no bones from the bind [%s]... Compile will fail\n", 
                             m_SourceFile, BindAnim.m_SourceFile) );
			}

			DeleteBone( i );
			i=-1;
		}

		i++;
	}


    //
    // Allocate new bones and frames
    //
    bone* pNewBone = new bone[ BindAnim.m_nBones ];
    frame* pNewFrame = new frame[ BindAnim.m_nBones * m_nFrames ];
    ASSERT( pNewBone && pNewFrame );

    //
    // Copy over bind skeleton
    //
    x_memcpy( pNewBone, BindAnim.m_pBone, sizeof(bone)*BindAnim.m_nBones );

    //
    // Construct frames
    //
    for( i=0; i<BindAnim.m_nBones; i++ )
    {
        // Lookup bone in current anim
        for( j=0; j<m_nBones; j++ )
        if( x_stricmp( m_pBone[j].Name, pNewBone[i].Name ) == 0 )
            break;

        // Check if bone is present
        if( j==m_nBones )
        {
            Problem = TRUE;

            // No bone present.  
            // Copy over first frame of BindAnim
            for( k=0; k<m_nFrames; k++ )
                pNewFrame[k*BindAnim.m_nBones + i] = BindAnim.m_pFrame[i];
        }
        else
        {
            // Copy IsLayer over to new bones
            pNewBone[i].bIsMasked = m_pBone[j].bIsMasked;

            // Copy data into new bone slot
            for( k=0; k<m_nFrames; k++ )
                pNewFrame[k*BindAnim.m_nBones + i] = m_pFrame[k*m_nBones+j];
        }
    }

    //
    // Free current allocations and provide new ones
    //
    delete[] m_pBone;
    delete[] m_pFrame;
    m_nBones = BindAnim.m_nBones;
    m_pBone = pNewBone;
    m_pFrame = pNewFrame;

    return !Problem;
}

//=========================================================================

xbool rawanim::HasSameSkeleton( const rawanim& Anim )
{
    s32 i;

    if( m_nBones != Anim.m_nBones )
        return FALSE;

    for( i=0; i<m_nBones; i++ )
    {
        bone& B0 = m_pBone[i];
        bone& B1 = Anim.m_pBone[i];

        if( x_stricmp( B0.Name, B1.Name ) != 0 )
            return FALSE;

        if( B0.iParent != B1.iParent )
            return FALSE;

        if( B0.nChildren != B1.nChildren )
            return FALSE;

        if( B0.BindMatrix.Difference(B1.BindMatrix) > 0.0001f )
            return FALSE;
    }

    return TRUE;
}

//=========================================================================

void rawanim::SanityCheck( void )
{
    ASSERT( (m_nBones>0) && (m_nBones<2048) );
    ASSERT( (m_nFrames>0) && (m_nFrames<65536) );
}

//=========================================================================

xbool rawanim::IsMaskedAnim( void )
{
    for( s32 i=0; i<m_nBones; i++ )
    if( m_pBone[i].bIsMasked )
        return TRUE;

    return FALSE;
}

//=========================================================================

void rawanim::Resample( s32 NewNFrames )
{
    ASSERT( NewNFrames >= 2 );

    f32 DownsampleRate = ((f32)(m_nFrames-1) / (f32)(NewNFrames-1));

    // Resample bone frames
    {
        frame* pNewFrame = new frame[ NewNFrames*m_nBones ];

        for( s32 iF=0; iF<NewNFrames; iF++ )
        {
            for( s32 iB=0; iB<m_nBones; iB++ )
            {
                f32 FrameF = (f32)iF * DownsampleRate; 
                s32 FrameI = (s32)FrameF;
                f32 FrameT = FrameF - (f32)FrameI;

                ASSERT( (FrameI>=0) && (FrameI<m_nFrames) );
                ASSERT( (FrameT>=0) && (FrameT<1.0f) );

                frame  NF = m_pFrame[FrameI*m_nBones+iB];
                if( FrameI < (m_nFrames-1) )
                {
                    frame& F1       = m_pFrame[(FrameI+1)*m_nBones+iB];
                    NF.Translation  = NF.Translation + FrameT*(F1.Translation - NF.Translation);
                    NF.Scale        = NF.Scale + FrameT*(F1.Scale - NF.Scale);
                    NF.Rotation     = BlendSlow( NF.Rotation, F1.Rotation, FrameT );
                }

                pNewFrame[iF*m_nBones+iB] = NF;
            }
        }

        // Replace old frames with new
        if( m_pFrame ) delete[] m_pFrame;
        m_pFrame = pNewFrame;
        m_nFrames = NewNFrames;
    }
    
    // Resample prop frames
    if( m_nProps > 0 ) 
    {
        prop_frame* pNewFrame = new prop_frame[ NewNFrames*m_nProps ];

        for( s32 iF=0; iF<NewNFrames; iF++ )
        {
            for( s32 iP=0; iP<m_nProps; iP++ )
            {
                f32 FrameF = (f32)iF * DownsampleRate;
                s32 FrameI = (s32)FrameF;
                f32 FrameT = FrameF - (f32)FrameI;

                ASSERT( (FrameI>=0) && (FrameI<m_nPropFrames) );
                ASSERT( (FrameT>=0) && (FrameT<1.0f) );

                prop_frame NF = m_pPropFrame[FrameI*m_nProps+iP];
                if( FrameI < (m_nFrames-1) )
                {
                    prop_frame& F1  = m_pPropFrame[(FrameI+1)*m_nProps+iP];
                    NF.Translation  = NF.Translation + FrameT*(F1.Translation - NF.Translation);
                    NF.Scale        = NF.Scale + FrameT*(F1.Scale - NF.Scale);
                    NF.Rotation     = BlendSlow( NF.Rotation, F1.Rotation, FrameT );
                }

                pNewFrame[iF*m_nProps+iP] = NF;
            }
        }

        // Replace old frames with new
        if( m_pPropFrame ) delete[] m_pPropFrame;
        m_pPropFrame = pNewFrame;
        m_nPropFrames = NewNFrames;
    }

    // Remap events
    s32 i;
    for( i=0; i<m_nSuperEvents; i++ )
    {
        m_pSuperEvent[i].StartFrame = (s32)(m_pSuperEvent[i].StartFrame/DownsampleRate);
        m_pSuperEvent[i].EndFrame   = (s32)(m_pSuperEvent[i].EndFrame/DownsampleRate);
    }
    for( i=0; i<m_nEvents; i++ )
    {
        m_pEvent[i].Frame0 = (s32)(m_pEvent[i].Frame0 / DownsampleRate);
        m_pEvent[i].Frame1 = (s32)(m_pEvent[i].Frame1 / DownsampleRate);
    }
}

//=========================================================================

