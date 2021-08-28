//////////////////////////////////////////////////////////////////////////////
//
//	Object.hpp
//
//
//		Defines base object class from which all game objects should inherit.
//		When creating a new type, inherit from this, add the logic to
//		obj_mgr for it's creation, and add a type for it here in the
//		object_type enum.  Constructor will be private to insure that only the
//		obj_mgr can create the object. - CDS
//
///////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "x_types.hpp"
#include "x_math.hpp"
#include "x_string.hpp"
#include "x_array.hpp"
#include "x_bytestream.hpp"
#include "guid.hpp"


class object
{
//------------------------------------------------------------------------------
//  Public Types
//------------------------------------------------------------------------------
public:

	enum type
	{
		TYPE_NULL             =0,
        TYPE_PLAYER             ,
        TYPE_LIGHT              ,
        TYPE_TRIGGER            ,
        TYPE_END_OF_LIST
	};

	enum object_attr
	{
        ATTR_NEEDS_LOGIC_TIME           = bit( 0),  // This flag indicates that this object needs time for logic
        ATTR_LOGIC_ONLY_WHEN_VISIBLE    = bit( 1),  // Gets logic but only when visible on screen
        ATTR_CAN_MOVE                   = bit( 2),  // This objects world position can change
        ATTR_COLLIDABLE                 = bit( 3),  // This object can collide with other objects
        ATTR_LIGHT                      = bit( 4),  // This is a light object or an object that emits light
        ATTR_RENDERABLE                 = bit( 5),  // This object is actually renderable in the normal game
        ATTR_PLAY_SURFACE               = bit( 6),  // This object is a play surface where objects can move around
        ATTR_PLAYER_CONTROLLED          = bit( 7),  // This object can be controlled in some manner by the player
        ATTR_DAMAGEABLE                 = bit( 8),  // This object can be destroyed
        ATTR_SOUND_SOURCE               = bit( 9),  // This object can create emit sounds
        ATTR_SPACIAL_ENTRY              = bit(10),  // Obj mgr knows when to add/remove from the spacial
        ATTR_DESTROY                    = bit(11),  // Flag should be set when the object is marked for death

        ATTR_ALL					    = 0xFFFFFFFF
	};								


	virtual                         ~object         ( void );
                            
	virtual			void			OnInit			( void );       //  Non-contructor Init function
	virtual			void			OnKill  		( void );       //  Non-destructive destructor.  Does not free mem
					void            RequestDestroy  ( void );       //  Add to the "to be killed" list for destruction outside game loop

	virtual         void            OnAdvanceLogic  ( f32 DeltaTime ) ;  // Updates the logic for the object by DeltaTime secs

	virtual         void            Move            ( const vector3& NewPos );  //  sets the absolute position for the object
	virtual         void            MoveRel         ( const vector3& DeltaPos );//  moves the object relative to current pos
					void            SetRotation     ( const radian3& Orient );  //  sets the rotation for the object

					u32             GetAttrBits     ( void ) const;
                    void            SetAttrBits     ( u32 NewBits );
                                
					guid            GetGuid         ( void ) const; //  Returns the unique ID for the object
	virtual     	type            GetType         ( void ) const = 0; //  Returns the type of the object
	virtual const   char*           GetTypeName     ( void ) const = 0; //  Gets the type name
    virtual const   char*           GetObjectName   ( void ) const; //  Gets the unique string name for the object
    virtual         void            SetObjectName   ( const char* NewName); //  Sets a new name for this object
                            
			const   bbox&           GetBBox         ( void ) const; //  Gets the world BBox
			const   vector3&        GetPosition     ( void ) const; //  Gets the objects world offset
			const   radian3&        GetRotation     ( void ) const; //  returns teh rotations of the object in world spac

	virtual			void			DebugRender		( void );       
	virtual			void			Render			( void );       
	virtual         void            RenderCollision ( void );       //  Renders collision data


//	virtual         void            OnCollision     ( const object& OtherObject, const collision& Info);

//    virtual         bool            OnMessage       ( message& Message );

//------------------------------------------------------------------------------
//  Protected Types
//------------------------------------------------------------------------------
protected:

    //  Protected constructor method to insure no one creates an object but the
    //  object manager
	object( void );
    friend class obj_mgr;

    guid                m_Guid;	                //  Unique ID for the object
    u32                 m_AttrBits;             //  Attribute bits for attribute flags
    vector3             m_WorldPos;             //  World position
    bbox                m_WorldBBox;            //  World BBox.  Local bbox == Worldbbox - world pos
    radian3             m_Orient;               //  Rotation direction
    char*               m_pCustomName;          //  Will be NULL at start and use programatic name but
                                                //  different name can be set manually to over ride 
                                                //  the programatic name

    };

#endif//__OBJECT__





























/*

  Data removed from the Meridian Object type



//        ATTR_SOLID_STATIC			    = bit( 0),
//        ATTR_SOLID_DYNAMIC			= bit( 1),
//        ATTR_SOLID					= ATTR_SOLID_STATIC | ATTR_SOLID_DYNAMIC,
//        ATTR_LIGHT					= bit( 2),
//        ATTR_SELECTABLE_ONLY		    = bit( 3),
//        ATTR_DAMAGEABLE				= bit( 5),
//        ATTR_NPC					    = bit( 4),
//        ATTR_PLAYER					= bit( 6),        
//        ATTR_INDEPENDENT_MOTION		= bit( 7),      //Means object, in AdvanceLogic, updates his own world position
//        ATTR_STATIC					= bit( 8),
//        ATTR_DYNAMIC				    = bit( 9),
//        ATTR_INTERNAL				    = bit(10),      // a platform
//        ATTR_PLAYSURFACE			    = bit(11),
//        ATTR_RIGIDINSTANCE			= bit(12),
//        ATTR_LADDER					= bit(13),
									




    obj_type_info*      m_pTypeInfo;
    u32                 m_Flags;
    bbox                m_LocalBBox;




    guid                m_LinkedGuid;
	
	char				m_Name[MAX_NAME_LENGTH + 1];

    xbool               m_bBurnLighting;
    xcolor*             m_pVertexColor;
    s32                 m_nVertices;
    s32                 m_SpatialChannel;
    xbool               m_bDoNotCollide;

    byte*               m_pTriFlag;              // Surface Flags
    s32                 m_nTriFlags;

					obj_type_info*  GetTypeInfoPtr  ( void ) const;

	virtual         void            OnExport        ( text_out& TOut );
	virtual         void            OnImport        ( text_in& TIn );


					void            SetLinkedGuid   ( const guid& Guid );
			const   guid&           GetLinkedGuid   ( void ) const;

	virtual         void            OnApplyPain     ( guid Guid, const vector3& Pos, f32 Radius, f32 Pain, f32 Force, s32 Type = object::PAIN_UNKNOWN );

	virtual         xbool           HasSurfaceFlags ( void );
	virtual         void            ImportSurfaceFlags ( text_in& TIn );
	virtual         byte            GetTriFlags     ( s32 TriIdx );
					xbool           DoesNotCollide  ( void );

	virtual         void            OnSwitch        ( void );


	virtual			void			SetName			( const char *Name );
					void			SetName			( const xwstring& Name );

	virtual const	char*			GetName			( void );
	virtual const	xwstring		GetStringName	( void );





*/