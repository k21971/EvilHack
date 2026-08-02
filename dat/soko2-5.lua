-- NetHack sokoban soko2-5.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "premapped", "sokoban", "noflip");
-- Steve Melenchuk <smelenchuk@gmail.com>
des.map([[
  --------------------
  |..................|
  |.....------------.|
  --.----..--------|.|
----.--...........||.|
----....-.....-...||.|
 |.....-..-...-----|.|
 |.-......-.--------+|
 |..-.--.....| |.....|
 --......--.-- |.....|
  --.........| |.....|
   ---..---..- |.....|
     ---- |..| |.....|
          ---- |.....|
               |.....|
               -------
]]);
des.stair("down", 7, 11)
des.stair("up", 18, 11)
des.door("locked", 20, 7)
des.region({ region = { 0, 0, 21, 15 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,21,15))
des.non_passwall(selection.area(0,0,21,15))

-- Boulders
des.object("boulder", 8, 4)
des.object("boulder", 11, 5)
des.object("boulder", 12, 5)
des.object("boulder", 5, 6)
des.object("boulder", 12, 6)
des.object("boulder", 5, 7)
des.object("boulder", 7, 7)
des.object("boulder", 8, 7)
des.object("boulder", 5, 8)
des.object("boulder", 5, 9)
des.object("boulder", 7, 9)
des.object("boulder", 8, 9)
des.object("boulder", 10, 10)
des.object("boulder", 11, 11)

-- Traps
des.trap("hole", 8, 1)
des.trap("hole", 9, 1)
des.trap("hole", 10, 1)
des.trap("hole", 11, 1)
des.trap("hole", 12, 1)
des.trap("hole", 13, 1)
des.trap("hole", 14, 1)
des.trap("hole", 15, 1)
des.trap("hole", 16, 1)
des.trap("hole", 17, 1)
des.trap("hole", 18, 1)
des.trap("hole", 19, 1)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
