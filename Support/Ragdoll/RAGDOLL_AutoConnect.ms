--=========================================================================================
-- RAGDOLL auto connect tool by Stephen Broumley 3/2/03
--
-- This script auto connects any selected ragdoll particles with equal distance constraints
--
--=========================================================================================

-- Returns whether or not the 2 particles are connected with a constraint
fn IsConnected ParticleA ParticleB =
(
    -- Check all objects
    for i in objects do
    (
        -- Constraint?
        if (classof i == RAGDOLL_Constraint) do
        (
            -- Connected?
            if ((i.ObjectA == ParticleA) and (i.ObjectB == ParticleB)) do
                return true

            -- Connected?
            if ((i.ObjectA == ParticleB) and (i.ObjectB == ParticleA)) do
                return true
        )
    )

    -- Not connected
    return false
)

--=========================================================================================

-- Check all objects
c = 0
format "RAGDOLL_AutoConnect : Checking all objects...\n"
for i in objects do
(
    -- Try connect this particle?
	if ((classof i == RAGDOLL_Particle) and (i.isSelected)) do
    (
        -- Check against all other particles
        for j in objects do
        (
            -- Try connect this particle?
        	if ((i != j) and (classof j == RAGDOLL_Particle) and (j.isSelected)) do
            (
                -- Connect?
                if (IsConnected i j == false) do
                (
                    -- Show info
                	format "Connecting % and %\n" i.name j.name

                    -- Compute create position
                    Pos = (i.objecttransform.pos + j.objecttransform.pos) / 2

                    -- Create new constraint
                    Constraint = RAGDOLL_Constraint position:Pos ObjectA:i ObjectB:j

                    c = c + 1
                )
            )                
        )
    )
)
format "Created % connections\n\n" c

--=========================================================================================

-- Refresh all views
forceCompleteRedraw()

--=========================================================================================
