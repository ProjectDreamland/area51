// DisableScreenSave.cpp 
// implementation of the CDisableScreenSaver class.
//////////////////////////////////////////////////

#include "BaseStdAfx.h"
#include "DisableScreenSave.h"

///////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////

static UINT dss_GetList[] = {SPI_GETLOWPOWERTIMEOUT, SPI_GETPOWEROFFTIMEOUT, SPI_GETSCREENSAVETIMEOUT};
static UINT dss_SetList[] = {SPI_SETLOWPOWERTIMEOUT, SPI_SETPOWEROFFTIMEOUT, SPI_SETSCREENSAVETIMEOUT};


static const int dss_ListCount = _countof(dss_GetList);


CDisableScreenSaver::CDisableScreenSaver()
{
/*
    m_pValue = new int[dss_ListCount];

    for (int x=0;x<dss_ListCount;x++)
    {
        // Get the current value
        SystemParametersInfo (dss_GetList[x], 0, &m_pValue[x], 0);

        TRACE(_T("%d = %d\n"), dss_GetList[x], m_pValue[x]);

        // Turn off the parameter
        SystemParametersInfo (dss_SetList[x], 0, NULL, 0);
    }
*/
}


CDisableScreenSaver::~CDisableScreenSaver()
{
/*
    for (int x=0;x<dss_ListCount;x++)
    {
        // Set the old value
        SystemParametersInfo (dss_SetList[x],  m_pValue[x], NULL, 0);
    }

    delete[] m_pValue;
*/
}