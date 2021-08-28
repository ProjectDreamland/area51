--=========================================================================================
-- ODE Hinge Joint by Stephen Broumley 2/22/03
--=========================================================================================

plugin simpleObject RAGDOLL_AngleLimit
name:"AngleLimit" 
classID:#(0x5a6a0440, 0x7bc150a3)
category:"RAGDOLL Objects" 
(   
    --=====================================================================================

	local SPHERE_RADIUS   = 1.0/4
    local CYLINDER_RADIUS = 1.0/20
    local CYLINDER_HEIGHT = 5.0
	local CONE_RADIUS     = 1.0/4
	local CONE_HEIGHT     = 1.0

    local SphereInst, CylinderInst, ConeInst 
	
	
    --=====================================================================================
    
    parameters main rollout:params 
    ( 
        ObjectA  type:#node       ui:pbObjectA
        ObjectB  type:#node       ui:pbObjectB
        Size     type:#worldUnits ui:spSize     default:10
        AxisX    type:#float      ui:spAxisX    default:1
        AxisY    type:#float      ui:spAxisY    default:0
        AxisZ    type:#float      ui:spAxisZ    default:0
        MinAngle type:#float      ui:spMinAngle default:0
        MaxAngle type:#float      ui:spMaxAngle default:0
    ) 

    --=====================================================================================
    
    rollout params "RAGDOLL_AngleLimit" 
    ( 
        pickButton pbObjectA  "NULL" type:#node 
        pickButton pbObjectB  "NULL" type:#node 
        spinner    spSize     "Size"     type:#worldUnits range:[0,1E9,10] 
        spinner    spAxisX    "AxisX"    type:#float      range:[-1,1,1] 
        spinner    spAxisY    "AxisY"    type:#float      range:[-1,1,0] 
        spinner    spAxisZ    "AxisZ"    type:#float      range:[-1,1,0] 
        spinner    spMinAngle "MinAngle" type:#float      range:[-180,180,0] 
        spinner    spMaxAngle "MaxAngle" type:#float      range:[-180,180,0] 

        on params open do
        (
			if (ObjectA != undefined) then
	            pbObjectA.caption = ObjectA.name
				
			if (ObjectB != undefined) then
	            pbObjectB.caption = ObjectB.name
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
    
    on buildMesh do 
    ( 
        -- Create sphere inst
        if (SphereInst == undefined) then 
            SphereInst = createInstance Sphere
        SphereInst.radius = Size * SPHERE_RADIUS

		-- Create cylinder inst
		if (CylinderInst == undefined) then
			CylinderInst = createInstance Cylinder
		CylinderInst.radius = Size * CYLINDER_RADIUS
        CylinderInst.height = Size * CYLINDER_HEIGHT
		
		-- Setup axis matrix
		AxisMatrix = MatrixFromNormal [AxisX, AxisY, AxisZ]
				
        -- Create cylinder mesh
		CylinderMatrix = transMatrix [0, 0, -CylinderInst.height/2] * AxisMatrix
        CylinderMesh   = copy CylinderInst.mesh
        for i in CylinderMesh.verts do
        (
			i.pos = i.pos * CylinderMatrix
        )

		-- Create cone inst
		if (ConeInst == undefined) then
		(
			ConeInst = createInstance Cone
		)
        ConeInst.radius1 = Size * CONE_RADIUS
        ConeInst.height  = Size * CONE_HEIGHT
		
        -- Create Cone mesh
		ConeMatrix = transMatrix [0, 0, CylinderInst.height/2] * AxisMatrix
        ConeMesh = copy ConeInst.mesh
        for i in ConeMesh.verts do
        (
			i.pos = i.pos * ConeMatrix
        )

		-- Collect all meshes
        mesh = SphereInst.mesh + CylinderMesh + ConeMesh
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

        on mouseMove click do 
        (
            case click of 
            ( 
                2: Size = abs gridDist.x
            ) 
        )
    ) 
	
    --=====================================================================================

) 

