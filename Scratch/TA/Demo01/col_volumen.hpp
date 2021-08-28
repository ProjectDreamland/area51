
#ifndef COL_VOLUMEN_HPP
#define COL_VOLUMEN_HPP


struct static_volume_set
{
    bbox                m_BBox;
    f32                 m_CellSize;
    s32                 m_nCells[3];
    s32                 m_nTotalCells;

    s32                 m_nVolumes;
    collision_volume*   m_pVolume;

    xarray<s32>         m_CellVolumeCount;
    xarray<s32>         m_CellVolumeOffset;
    xarray<s32>         m_VolumeIndex;
};


class col_volumen
{
public:

    struct volume
    {    
        vector3     P[3];           // tri
        bbox        AABBox;         // BBox
        s32         SearchSeq;      // Sequence number
    };




                    col_volumen     ( void );
                   ~col_volumen     ( void );

        void        Initialize      ( s32 nTriangles );


    f32                 m_CellSize;
    s32                 m_nCells[3];
    s32                 m_nTotalCells;

    s32                 m_nVolumes;
    collision_volume*   m_pVolume;

    s32*                m_pCellVolumeCount;
    s32*                m_CellVolumeOffset;
    s32*                m_VolumeIndex;
};



#endif