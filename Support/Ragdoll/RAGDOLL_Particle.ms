--=========================================================================================
-- RAGDOLL Particle by Stephen Broumley 2/27/03
--=========================================================================================

plugin simpleObject RAGDOLL_Particle
name:"Particle" 
classID:#(0x3f894a71, 0x41ef0785)
category:"RAGDOLL Objects" 
(   
    --=====================================================================================

    local SphereInst
		
    --=====================================================================================
    
    parameters main rollout:params 
    ( 
        Radius     type:#worldUnits ui:spRadius    default:1
        Mass       type:#float      ui:spMass      default:1
        bCollision type:#boolean    ui:cbCollision default:true
    ) 

    --=====================================================================================
    
    rollout params "RAGDOLL_Particle" 
    ( 
        spinner     spRadius     "Radius"     type:#float      range:[0,1E9,1] 
        spinner     spMass       "Mass"       type:#float      range:[0,100,1] 
		checkbox    cbCollision  "Collision"  checked:false
    ) 

    --=====================================================================================
    
    on buildMesh do 
    ( 
        -- Create sphere inst
        if (SphereInst == undefined) then 
            SphereInst = createInstance Sphere
        SphereInst.radius = Radius

		-- Collect all meshes
        mesh = SphereInst.mesh
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
                2: 
				(
					Radius = sqrt ((gridDist.x ^ 2) + (gridDist.y ^ 2))
					if (Radius < 1) do
						Radius = 1
        	    ) 
			)			
        )
    ) 
	
    --=====================================================================================

) 

