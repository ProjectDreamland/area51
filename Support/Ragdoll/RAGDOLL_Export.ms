-- Particle info
Particle_Name         = #()
Particle_Color        = #()
Particle_Pos          = #()
Particle_Radius       = #()
Particle_Mass         = #()
Particle_bCollision   = #()

-- Constraint info
Constraint_Name         = #()
Constraint_Color        = #()
Constraint_ParticleA    = #()
Constraint_ParticleB    = #()
Constraint_bEqual       = #()
Constraint_EqualPercent = #()
Constraint_bMin         = #()
Constraint_MinPercent   = #()
Constraint_bMax         = #()
Constraint_MaxPercent   = #()
Constraint_Damp         = #()
Constraint_bBone        = #()

-- Hinge joint info
AngleLimitName       = #()
AngleLimitPos        = #()
AngleLimitObjectA    = #()
AngleLimitObjectB    = #()
AngleLimitAxis       = #()
AngleLimitMinAngle   = #()
AngleLimitMaxAngle   = #()



-- Check all objects
for i in objects do
(
    -- Skip hidden
	if( (not i.ishidden) ) do
    (
        -- Collect particles
        if (classof i == RAGDOLL_Particle ) do
	    (
		    append Particle_Name       i.name
			append Particle_Color	   i.wirecolor
		    append Particle_Pos        i.objecttransform.pos
		    append Particle_Radius     i.Radius
		    append Particle_Mass       i.Mass
		    append Particle_bCollision i.bCollision
	    )	

        -- Collect constraints
        if (classof i == RAGDOLL_Constraint ) do
	    (
		    append Constraint_Name         i.name
			append Constraint_Color	       i.wirecolor
		    append Constraint_ParticleA    i.ObjectA.name
		    append Constraint_ParticleB    i.ObjectB.name
		    append Constraint_bEqual       i.bEqual
		    append Constraint_EqualPercent i.EqualPercent
		    append Constraint_bMin         i.bMin
		    append Constraint_MinPercent   i.MinPercent
		    append Constraint_bMax         i.bMax
		    append Constraint_MaxPercent   i.MaxPercent
		    append Constraint_Damp         i.Damp
		    append Constraint_bBone        i.bBone
	    )	

        -- Collect hinge joints
        if (classof i == RAGDOLL_AngleLimit ) do
	    (
            append AngleLimitName        i.name
            append AngleLimitPos         i.objecttransform.pos
		    append AngleLimitObjectA     i.ObjectA.name
		    append AngleLimitObjectB     i.ObjectB.name
		    append AngleLimitAxis        ([i.AxisX, i.AxisY, i.AxisZ] * (i.objecttransform.rotation))
		    append AngleLimitMinAngle    i.MinAngle
		    append AngleLimitMaxAngle    i.MaxAngle
	    )
    )
)

clearListener()


-- Write particles
if (Particle_Name.count > 0) do
(
	format "static particle_def ParticleDefs[] =\n"
	format "{\n"
	for i = 1 to Particle_Name.count do
	(
		format "    { \"%\",\n"        Particle_Name[i]
		format "      %, %, %,\n"      Particle_Color[i].r Particle_Color[i].g Particle_Color[i].b
		format "      %f, %f, %f,\n"   Particle_Pos[i].x  Particle_Pos[i].y  Particle_Pos[i].z
		format "      %f, %f,\n"       Particle_Radius[i] Particle_Mass[i]
		format "      %},\n"           Particle_bCollision[i]
		format "\n"
	)
	format "} ;\n"
)
	
-- Write constraints
if (Constraint_Name.count > 0) do
(
	format "static constraint_def ConstraintDefs[] =\n"
	format "{\n"
	for i = 1 to Constraint_Name.count do
	(
		format "    { \"%\",\n"        Constraint_Name[i]
		format "      %, %, %,\n"      Constraint_Color[i].r Constraint_Color[i].g Constraint_Color[i].b
		format "      \"%\", \"%\",\n" Constraint_ParticleA[i] Constraint_ParticleB[i] 
		format "      %, %,\n"         Constraint_bEqual[i]  Constraint_EqualPercent[i]
		format "      %, %,\n"         Constraint_bMin[i]  Constraint_MinPercent[i]
		format "      %, %,\n"         Constraint_bMax[i]  Constraint_MaxPercent[i]
		format "      %f, %},\n"       Constraint_Damp[i] Constraint_bBone[i]
		format "\n"
	)
	format "} ;\n"
)

-- Write hinge joints
if (AngleLimitName.count > 0) do
(
	format "static angle_limit_def AngleLimitDefs[] =\n"
	format "{\n"
	for i = 1 to AngleLimitName.count do
	(
		format "    { \"%\",\n"        AngleLimitName[i]
		format "      %f, %f, %f,\n"   AngleLimitPos[i].x  AngleLimitPos[i].y  AngleLimitPos[i].z
		format "      \"%\", \"%\",\n" AngleLimitObjectA[i] AngleLimitObjectB[i]
		format "      %f, %f, %f,\n"   AngleLimitAxis[i].x  AngleLimitAxis[i].y  AngleLimitAxis[i].z
		format "      %f, %f},\n"      AngleLimitMinAngle[i] AngleLimitMaxAngle[i]
		format "\n"
	)
	format "} ;\n"
)	
