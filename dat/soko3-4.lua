-- NetHack sokoban soko3-4.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "sokoban", "noflip");
-- "Thinking Rabbit" 
des.map([[
          ----      
       ---|..----   
       |..|.....|---
 ----  |.....|..|..|
 |..----.|...|.....|
 |...........|--...|
 |.--F--...|.|....--
 |.|...|-.-|......| 
 |.F.L.+...|......| 
--.|...|........--- 
|..--F-----.|.---   
|.........|...|     
---------------     
]]);
des.stair("down", 12, 9)
des.stair("up", 9, 11)
des.region({ region = { 0, 0, 19, 12 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,19,12))
des.non_passwall(selection.area(0,0,19,12))
des.door("locked", 7, 8)
des.object("boulder", 14, 2)
des.object("boulder", 12, 3)
des.object("boulder", 14, 3)
des.object("boulder", 17, 4)
des.object("boulder", 16, 5)
des.object("boulder", 12, 6)
des.object("boulder", 16, 6)
des.object("boulder", 9, 7)
des.object("boulder", 13, 7)
des.object("boulder", 14, 7)
des.object("boulder", 16, 8)
des.object("boulder", 9, 9)
des.object("boulder", 13, 9)
des.object("boulder", 11, 10)
des.trap("hole", 5, 5)
des.trap("hole", 6, 5)
des.trap("hole", 7, 5)
des.trap("hole", 2, 6)
des.trap("hole", 2, 7)
des.trap("hole", 2, 8)
des.trap("hole", 2, 9)
des.trap("hole", 3, 11)
des.trap("hole", 4, 11)
des.trap("hole", 5, 11)
des.trap("hole", 6, 11)
des.trap("hole", 7, 11)
des.trap("hole", 8, 11)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
