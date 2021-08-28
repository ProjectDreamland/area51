BoxName   = #()
BoxWidth  = #()
BoxLength = #()
BoxHeight = #()
BoxPos    = #()
BoxRot    = #()

HingeName     = #()
HingePos      = #()
HingeAxisX    = #()
HingeAxisY    = #()
HingeAxisZ    = #()
HingeMinAngle = #()
HingeMaxAngle = #()


for i in objects do
(
    -- Skip hidden/frozen
	if( (not i.ishidden) and (not i.isfrozen) ) do
    (
        -- Collect box
    	if (findstring i.name "box" != undefined) do
	    (
		    append BoxName   i.name
		    append BoxWidth  i.width
		    append BoxLength i.length
		    append BoxHeight i.height

		    append BoxPos    i.objecttransform.pos
		    append BoxRot    ((i.objecttransform.rotation) as EulerAngles)
	    )	

        -- Collect hinges
	    if (findstring i.name "hinge" != undefined) do
	    (
            append HingeName        i.name
            append HingePos         i.objecttransform.pos
		    append HingeAxisX       (getUserProp i "AxisX")
		    append HingeAxisY       (getUserProp i "AxisY")
		    append HingeAxisZ       (getUserProp i "AxisZ")
		    append HingeMinAngle    (getUserProp i "MinAngle")
		    append HingeMaxAngle    (getUserProp i "MaxAngle")
	    )
    )
)

clearListener()

-- Write boxes
format "bone_def BoneDefs[] =\n"
format "{\n"
for i = 1 to BoxName.count do
(
	format "    { \"%\",\n" BoxName[i]
	format "      %f, %f, %f,\n" BoxWidth[i] BoxLength[i] BoxHeight[i]
	format "      %f, %f, %f,\n" BoxPos[i].x BoxPos[i].y BoxPos[i].z
	format "      %f, %f, %f },\n" BoxRot[i].x BoxRot[i].y BoxRot[i].z
	format "\n"
)
format "} ;\n"

