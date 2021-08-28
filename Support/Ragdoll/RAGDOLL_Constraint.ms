--=========================================================================================
-- RAGDOLL constraint by Stephen Broumley 2/26/03
--=========================================================================================

plugin simpleObject RAGDOLL_Constraint
name:"Constraint" 
classID:#(0x75b825f5, 0x53500d3c)
category:"RAGDOLL Objects" 
(   
    --=====================================================================================

    local CYLINDER_RADIUS     = 0.25
    local CYLINDER_HEIGHT     = 100.0

    local CylinderInst 
    local ConeInst 
    local ParentNode
    local RefreshHandler

    --=====================================================================================

    parameters main rollout:params 
    ( 
        ObjectA      type:#node       ui:pbObjectA
        ObjectB      type:#node       ui:pbObjectB
        
        bEqual       type:#boolean    ui:cbEqual        default:true
        EqualPercent type:#percent    ui:spEqualPercent default:100

        bMin         type:#boolean    ui:cbMin          default:false
        MinPercent   type:#percent    ui:spMinPercent   default:100

        bMax         type:#boolean    ui:cbMax          default:false
        MaxPercent   type:#percent    ui:spMaxPercent   default:100

        Damp         type:#float      ui:spDamp         default:1

        bBone        type:#boolean    ui:cbBone         default:false
        
        bRefresh     type:#boolean default:false
    ) 

    --=====================================================================================
    
    rollout params "RAGDOLL_Constraint" 
    ( 
        pickButton pbObjectA  "NULL" type:#node 
        pickButton pbObjectB  "NULL" type:#node 
        
        checkbox   cbEqual          "Equal Dist Constraint" checked:true
        spinner    spEqualPercent   "Percent"               type:#integer range:[0,10000,100] 
        
        checkbox   cbMin            "Min Dist Constraint"   checked:false
        spinner    spMinPercent     "Percent"               type:#integer range:[0,10000,100] 
        

        checkbox   cbMax            "Max Dist Constraint"   checked:false
        spinner    spMaxPercent     "Percent"               type:#integer range:[0,10000,100] 
        
        spinner    spDamp           "Damp"                  type:#float   range:[0,1,1] 

        checkbox   cbBone           "Bone"                  checked:false
        
        
        on params open do
        (
            if (ObjectA != undefined) then
                pbObjectA.caption = ObjectA.name
                
            if (ObjectB != undefined) then
                pbObjectB.caption = ObjectB.name

            spEqualPercent.enabled = bEqual
            spMinPercent.enabled   = bMin
            spMaxPercent.enabled   = bMax
        )

        on cbEqual changed theState do 
        (
            spEqualPercent.enabled = bEqual
        )
        
        on cbMin changed theState do 
        (
            spMinPercent.enabled = bMin
        )
        on cbMax changed theState do 
        (
            spMaxPercent.enabled = bMax
        )
                    
        on pbObjectA picked obj do 
        (
            pbObjectA.caption = obj.name
        )            

        on pbObjectB picked obj do
        (
            pbObjectB.caption = obj.name
        )            
    ) 

    --=====================================================================================

	fn AddMarkers Height Percent ConeRadius ConeHeight L2W =
	(
		Length = (0.5 * Height * Percent) / 100.0
		Mid    = Height / 2 
		
		-- Marker1
		ConeInst.radius1 = ConeRadius
		ConeInst.height  = ConeHeight
		Marker  		 = Mid + Length - ConeInst.height
		ConeMesh = copy ConeInst.mesh
        for i in ConeMesh.verts do
		(
			i.pos.z += Marker
            i.pos   = i.pos * L2W
		)
		mesh += ConeMesh
		
		-- Marker2
		ConeInst.radius1 = ConeRadius
		ConeInst.height  = -ConeHeight
		Marker  		 = Mid - Length - ConeInst.height
		ConeMesh = copy ConeInst.mesh
        for i in ConeMesh.verts do
		(
			i.pos.z += Marker
            i.pos   = i.pos * L2W
		)
		mesh += ConeMesh
	)
	
    --=====================================================================================
    
    on buildMesh do 
    ( 
        -- Create refresh mesh handler when transform of parent node changes
        if ((ParentNode != undefined) and (RefreshHandler == undefined)) do
        (
            -- Start handler
            when transform ParentNode changes handleAt:#redrawViews theNode do
            (    
                with undo off with animate off theNode.bRefresh=true
            )
            RefreshHandler = true
        )
        
        -- Create cylinder inst
        if (CylinderInst == undefined) do
        (
            CylinderInst = createInstance Cylinder
            ConeInst     = createInstance Cone
        )
                    
        -- Setup default radius and height
        InvParentMatrix = matrix3 1
        AxisMatrix      = matrix3 1
        WorldMatrix     = matrix3 1
        
        -- Lookup inverse parent matrix and world matrix
        if (ParentNode != undefined) do
        (
            InvParentMatrix = inverse ParentNode.objecttransform
            if (ObjectA != undefined) then
            (
                WorldMatrix.pos = ObjectA.transform.pos
            )
            else
            (
                WorldMatrix.pos = ParentNode.objecttransform.pos
            )                
        )
            
        Radius    = CYLINDER_RADIUS 
        Height    = CYLINDER_HEIGHT

        -- Extract height + orientation to look from object A->B
        if ((ObjectA != undefined) and (ObjectB != undefined)) do
        (
            AB         = ObjectB.transform.Pos - ObjectA.transform.Pos
            Height     = length AB
            if (Height == 0) then
            (
                Height     = CYLINDER_HEIGHT
                AxisMatrix = MatrixFromNormal[0,0,1]
            )
            else
            (                
                AB         = normalize AB
                AxisMatrix = MatrixFromNormal AB
            )                
        )

        -- Setup cylinder
        CylinderInst.radius = Radius
        CylinderInst.height = Height
		CylinderInst.sides  = 6
		
        -- Create cylinder mesh
        CylMatrix    = AxisMatrix * WorldMatrix * InvParentMatrix
        CylinderMesh = copy CylinderInst.mesh
        for i in CylinderMesh.verts do
        (
            i.pos = i.pos * CylMatrix
        )

        -- Collect all meshes
        mesh = CylinderMesh

		-- Add min markers?
		if (bMin) do
			AddMarkers Height MinPercent (CYLINDER_RADIUS*5) (CYLINDER_RADIUS*5) CylMatrix
			
		-- Add min markers
		if (bMax) do
			AddMarkers Height MaxPercent (CYLINDER_RADIUS*5) (-CYLINDER_RADIUS*5) CylMatrix
    ) 

    --=====================================================================================

    on attachedToNode N do
    (
        ParentNode = N
    )

    --=====================================================================================
    
    on detachedFromNode N do
    (
        ParentNode = undefined
    )

     --=====================================================================================

    on load do
    (    
        local myRefs = refs.dependents this
        ParentNode = undefined
        for obj in myRefs where ParentNode == undefined and isvalidnode obj and obj.baseobject == this do ParentNode = obj
        if ParentNode == undefined do (print "owning node not found on load";return())
    )

   --=====================================================================================
    
    tool create 
    ( 
        on mousePoint click do 
        (
            case click of 
            ( 
                1: nodeTM.translation = gridPoint 
                2: #stop 
            ) 
        )
    ) 

    --=====================================================================================
) 

