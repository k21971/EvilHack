-- NetHack sokoban soko3-6.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "sokoban", "noflip");
-- "Joseph L Traub"
des.map([[
----------------    
|...|..|.|.--..|    
|.|.|..........---  
|.|....--FF--....---
|...|..|....|......|
--.-|.-|FFF.|......|
 |.||.|-....--.--.--
 |.FF.FL....LF.FF.| 
 |.||.|--..--|.||.| 
--.--.--|..|-|.||.| 
|......||..|--.--.--
|......-|..|-......|
|.......|..|..---..|
-----..............|
    |.|....|..|...--
    --------------- 
]]);
des.stair("down", 8, 1)
des.stair("up", 8, 4)
des.region({ region = { 0, 0, 19, 15 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,19,15))
des.non_passwall(selection.area(0,0,19,15))
des.object("boulder", 7, 2)
des.object("boulder", 12, 2)
des.object("boulder", 14, 3)
des.object("boulder", 15, 4)
des.object("boulder", 16, 4)
des.object("boulder", 2, 5)
des.object("boulder", 5, 5)
des.object("boulder", 14, 10)
des.object("boulder", 17, 10)
des.object("boulder", 3, 11)
des.object("boulder", 4, 11)
des.object("boulder", 2, 12)
des.object("boulder", 5, 12)
des.object("boulder", 7, 13)
des.object("boulder", 11, 13)
des.object("boulder", 16, 13)
des.trap("hole", 11, 5)
des.trap("hole", 9, 6)
des.trap("hole", 10, 6)
des.trap("hole", 11, 6)
des.trap("hole", 9, 7)
des.trap("hole", 10, 7)
des.trap("hole", 11, 7)
des.trap("hole", 9, 8)
des.trap("hole", 10, 8)
des.trap("hole", 9, 9)
des.trap("hole", 10, 9)
des.trap("hole", 9, 10)
des.trap("hole", 10, 10)
des.trap("hole", 9, 11)
des.trap("hole", 10, 11)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")


--## Third level ###