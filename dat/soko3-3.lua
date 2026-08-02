-- NetHack sokoban soko3-3.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "sokoban", "noflip");
-- "Thinking Rabbit" 
des.map([[
----------------- 
|........F......| 
|..--....F......| 
--.|.....F......| 
 |.---------.--.| 
 |...........||.| 
 |..------...--.--
 ----    |.......|
       ---.----.--
       |........| 
       |....----- 
       --...|     
        -----     
]]);
des.stair("down", 16, 7)
des.stair("up", 4, 3)
des.region({ region = { 0, 0, 17, 12 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,17,12))
des.non_passwall(selection.area(0,0,17,12))
des.object("boulder", 12, 2)
des.object("boulder", 14, 2)
des.object("boulder", 11, 3)
des.object("boulder", 13, 3)
des.object("boulder", 11, 6)
des.object("boulder", 12, 6)
des.object("boulder", 11, 7)
des.object("boulder", 9, 9)
des.object("boulder", 11, 9)
des.object("boulder", 10, 10)
des.trap("hole", 3, 1)
des.trap("hole", 4, 1)
des.trap("hole", 2, 3)
des.trap("hole", 2, 4)
des.trap("hole", 4, 5)
des.trap("hole", 5, 5)
des.trap("hole", 6, 5)
des.trap("hole", 7, 5)
des.trap("hole", 8, 5)
des.trap("hole", 9, 5)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
