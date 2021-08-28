// EditorDocDecalMngr.h 
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_EDTIOR_INPUT_MNGR_)
#define _EDTIOR_INPUT_MNGR_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WorldEditor.hpp"
#include "..\Editor\BaseDocument.h"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================

class CInputSettings : public prop_interface
{
public:
    CInputSettings()  {  Reset(); }
    ~CInputSettings() {  }

    virtual void        OnEnumProp                      ( prop_enum&    List    );
    virtual xbool       OnProperty                      ( prop_query&   I       );
    virtual void        OnLoad                          ( text_in&      TextIn  ){}

    void Reset()
    {
  
    }
    
protected:
    
};

#endif 