-- NetHack sokoban soko4-3.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "hardfloor", "mazelevel", "premapped", "sokoban", "noflip");
-- "J Franklin Mentzer <wryter@aol.com>"
--12345678901234567890123456789012345678901234567890
des.map([[
     -------                 
     |..|..|     --------    
------.....-------......---  
|......---...|.|..........|  
|.|.|......|.|.|........|.---
|.|.--..|..|...-F-F-F-F--...|
|.......|...................|
|.|.--..|..|...-F-F-F-F--...|
|.|.|......|.|.|........|.---
|......---...|.|..........|  
------.....-------......---  
     |..|..|     --------    
     -------                 
]]);
-- Since there isn't a return portal, this forces the hero's initial placement
des.teleport_region({ region = { 0, 0, 28, 12 } })

des.levregion({ region = {13,6,13,6}, exclude = {0,0,0,0}, type="branch" })
des.stair("up", 26, 6)
des.region({ region = { 0, 0, 28, 12 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,28,12))
des.non_passwall(selection.area(0,0,28,12))

-- Magic chest
des.feature("magic chest", 14, 3)

-- A little help
des.object("scroll of earth", 16, 3)
if percent(50) then
  des.object("scroll of earth", 16, 9)
end

des.object("boulder", 8, 2)
des.object("boulder", 8, 4)
des.object("boulder", 3, 5)
des.object("boulder", 6, 6)
des.object("boulder", 7, 6)
des.object("boulder", 9, 6)
des.object("boulder", 10, 6)
des.object("boulder", 3, 7)
des.object("boulder", 8, 8)
des.object("boulder", 8, 10)
des.trap("pit", 15, 6)
des.trap("pit", 16, 6)
des.trap("pit", 17, 6)
des.trap("pit", 18, 6)
des.trap("pit", 19, 6)
des.trap("pit", 20, 6)
des.trap("pit", 21, 6)
des.trap("pit", 22, 6)
des.trap("pit", 23, 6)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
