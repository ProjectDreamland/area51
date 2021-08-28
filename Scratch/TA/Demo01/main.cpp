
#include "Entropy.hpp"
#include "SpatialDBase.hpp"


struct mynode
{
    int a;
};

spatial_dbase aSpatialDBase;

void AppMain( s32 argc, char* argv[] )
{
    aSpatialDBase.Init( 100 );
    aSpatialDBase.GetCellIndex( 0, 0, 0, 0, TRUE );
    aSpatialDBase.SanityCheck();
}