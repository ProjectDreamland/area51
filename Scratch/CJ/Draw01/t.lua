
-------------------------------------------------------------------------------
-- This is a comment
-------------------------------------------------------------------------------

function DrawMarkers( n, sx, sy, sz )
    for i=1,n
    do
        draw.Marker( math.random()*sx - sx/2, math.random()*sy - sy/2, math.random()*sz - sz/2 )
    end

    draw.Begin()
    draw.Color( 1, 1, 1, 1 )
    draw.Vertex(   0,   0, 0 )
    draw.Vertex( 100, 100, 0 )
    draw.Vertex( 100,   0, 0 )
    draw.End()

    a = vector3.new( math.random(), math.random(), math.random() )
    print( a )

end
