-- NetHack sokoban soko2-3.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "sokoban", "noflip");
-- "Joseph L Traub"
des.map([[
    ------------------
 ----....|.........+.|
 |....--.|.........|.|
 |.......|.........|.|
---.----.|.........|.|
|........-F-F-F-F-F-.|
|....................|
|........--F-F-F-F-..|
--.--.--.|...........|
 |--.....|...........|
 |.......|...........|
 |....|..|...........|
 ---------------------
]]);
des.stair("down", 2, 8)
des.stair("up", 11, 3)
des.region({ region = { 0, 0, 21, 12 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,21,12))
des.non_passwall(selection.area(0,0,21,12))
des.door("locked", 19, 1)
des.object("boulder", 3, 3)
des.object("boulder", 3, 5)
des.object("boulder", 7, 5)
des.object("boulder", 4, 6)
des.object("boulder", 5, 6)
des.object("boulder", 7, 6)
des.object("boulder", 8, 6)
des.object("boulder", 5, 7)
des.object("boulder", 8, 7)
des.object("boulder", 6, 9)
des.object("boulder", 15, 9)
des.object("boulder", 16, 9)
des.object("boulder", 5, 10)
des.object("boulder", 6, 10)
des.trap("hole", 20, 2)
des.trap("hole", 20, 3)
des.trap("hole", 20, 4)
des.trap("hole", 20, 5)
des.trap("hole", 9, 6)
des.trap("hole", 10, 6)
des.trap("hole", 11, 6)
des.trap("hole", 12, 6)
des.trap("hole", 13, 6)
des.trap("hole", 14, 6)
des.trap("hole", 15, 6)
des.trap("hole", 16, 6)
des.trap("hole", 17, 6)
des.trap("hole", 18, 6)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
