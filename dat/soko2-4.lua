-- NetHack sokoban soko2-4.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "sokoban", "noflip");
-- "Thinking Rabbit"
des.map([[
           ----       
  ---------|..|       
---...|...--..|---    
|.....|.......|..|    
|.|...|...--.....|    
|.|...|....|..--.|    
|.|.----.|.--F--.|--- 
|.|..|--.|..|.|..|..| 
|.|..|....|.|.|.....| 
|.........|.|.--F--.--
|.|..---.--.|..|..|..|
|.----......F..F.....|
|......-----|..|.-----
--------    |.--..|   
            |.....|   
            |..----   
            ----      
]]);
des.stair("down", 12, 1)
des.stair("up", 13, 7)
des.region({ region = { 0, 0, 21, 16 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,21,16))
des.non_passwall(selection.area(0,0,21,16))
des.object("boulder", 12, 2)
des.object("boulder", 3, 3)
des.object("boulder", 4, 3)
des.object("boulder", 12, 3)
des.object("boulder", 12, 4)
des.object("boulder", 8, 5)
des.object("boulder", 9, 5)
des.object("boulder", 3, 7)
des.object("boulder", 6, 9)
des.trap("hole", 14, 4)
des.trap("hole", 16, 5)
des.trap("hole", 16, 6)
des.trap("hole", 17, 8)
des.trap("hole", 19, 9)
des.trap("hole", 18, 11)
des.trap("hole", 16, 12)
des.trap("hole", 13, 13)
des.trap("hole", 15, 14)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
