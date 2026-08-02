-- NetHack sokoban soko2-6.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "premapped", "sokoban", "noflip");
--12345678901234567890123456789012345678901234567890
-- Steve Melenchuk <smelenchuk@gmail.com>
des.map([[
---------                
|.......|                
--.....--                
 --...-------------------
  --.--.................|
  |......--------------+|
  |..--..-----    |.....|
  |.....--...|    |.....|
 --...---....|    |.....|
 |....|.....--    |.....|
 |....-.....|     |.....|
 --.........|     |.....|
  |...-...---     |.....|
  --..|...|       -------
   |..-----              
   ----                  
]]);

des.stair("down", 4, 5)
des.stair("up", 21, 9)
des.door("locked", 23, 5)
des.region({ region = { 0, 0, 24, 15 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,24,15))
des.non_passwall(selection.area(0,0,24,15))

-- Boulders
des.object("boulder", 5, 2)
des.object("boulder", 4, 3)
des.object("boulder", 3, 6)
des.object("boulder", 4, 7)
des.object("boulder", 5, 7)
des.object("boulder", 3, 8)
des.object("boulder", 10, 8)
des.object("boulder", 3, 9)
des.object("boulder", 11, 9)
des.object("boulder", 5, 10)
des.object("boulder", 8, 10)
des.object("boulder", 3, 11)
des.object("boulder", 7, 11)
des.object("boulder", 9, 11)
des.object("boulder", 10, 11)
des.object("boulder", 4, 12)
des.object("boulder", 8, 12)

-- Traps
des.trap("hole", 9, 4)
des.trap("hole", 10, 4)
des.trap("hole", 11, 4)
des.trap("hole", 12, 4)
des.trap("hole", 13, 4)
des.trap("hole", 14, 4)
des.trap("hole", 15, 4)
des.trap("hole", 16, 4)
des.trap("hole", 17, 4)
des.trap("hole", 18, 4)
des.trap("hole", 19, 4)
des.trap("hole", 20, 4)
des.trap("hole", 21, 4)
des.trap("hole", 22, 4)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")


--## Top (last) level of Sokoban ###