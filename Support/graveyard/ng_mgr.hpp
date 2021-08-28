//=================================================================================================
//
//  ng_mgr.hpp
//
//      
//
//=================================================================================================

#ifndef NG_MGR_HPP
#define NG_MGR_HPP

//=================================================================================================
//  INCLUDES
//=================================================================================================
#include "obj_mgr\obj_mgr.hpp"

//=================================================================================================
//  CONSTANT DEFINITIONS
//=================================================================================================

const s32 k_MAX_NODE_CONNECTIONS = 8;

typedef s16 ng_node_id;
typedef s16 ng_connection_id;

//=================================================================================================
//  DATA STRUTURES
//=================================================================================================

//  This is the data that needs to be written to out to a file
struct ng_connection_permanent_data
{
    ng_node_id     m_Nodes[2];     //  Nodes that this connection connects between         4 bytes
    f32            m_Width;        //  Width of the connection                             4 bytes
};                                  


//  This is the data that needs to be written to out to a file
struct ng_node_permanent_data
{
    vector3          m_Position;                                 // Position of the node   
    ng_connection_id m_Connections[k_MAX_NODE_CONNECTIONS ];     // Connection list

};


//  Runtime data that does not need to be written out to a file
struct ng_connection_runtime_data
{
    
    f32                 m_Length;       //  Precalculated length so 

};

//  Runtime data that does not need to be written out to a file
struct ng_node_runtime_data
{
    guid                m_PlaceHolder;
    f32                 m_Length;       //  Precalculated length so 

};


//=================================================================================================
//  CLASS DEFINITION
//=================================================================================================

class ng_mgr
{
public:
                        ng_mgr(void);
                        ~ng_mgr( );                   

                        
protected:
    



private:
    


};