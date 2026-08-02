-- NetHack sokoban soko3-5.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "sokoban", "noflip");
-- "Joseph L Traub"
des.map([[
-------       --------
|.....|-------|......|
|.....|....|..--FF-..|
|.....|........|..F.--
|.....|....--..|..F.| 
|.....|-.......+..F.| 
|-.|..||.------|..F.| 
|-.|..--.......|..F.| 
|..|...........-FF-.| 
|...................| 
|.....--.|.....---..| 
---------------- ---- 
]]);
des.stair("down", 8, 10)
des.stair("up", 15, 1)
des.region({ region = { 0, 0, 21, 11 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,21,11))
des.non_passwall(selection.area(0,0,21,11))
des.door("locked", 15, 5)
des.object("boulder", 2, 3)
des.object("boulder", 10, 3)
des.object("boulder", 12, 3)
des.object("boulder", 3, 4)
des.object("boulder", 9, 4)
des.object("boulder", 2, 5)
des.object("boulder", 3, 5)
des.object("boulder", 8, 5)
des.object("boulder", 10, 5)
des.object("boulder", 4, 8)
des.object("boulder", 4, 9)
des.object("boulder", 5, 9)
des.trap("hole", 16, 1)
des.trap("hole", 17, 1)
des.trap("hole", 18, 1)
des.trap("hole", 19, 3)
des.trap("hole", 19, 4)
des.trap("hole", 19, 5)
des.trap("hole", 19, 6)
des.trap("hole", 19, 7)
des.trap("hole", 19, 8)
des.trap("hole", 15, 9)
des.trap("hole", 16, 9)
des.trap("hole", 17, 9)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
