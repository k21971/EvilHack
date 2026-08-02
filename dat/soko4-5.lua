-- NetHack sokoban soko4-5.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "hardfloor", "mazelevel", "premapped", "sokoban", "noflip");
-- "Thinking Rabbit"
--12345678901234567890123456789012345678901234567890
des.map([[
----            
|..|            
|..-------------
|..............|
|-.--FFFFFF---+|
|..........F...|
|...|......F...|
---.|......F...|
  |.|...|..F...|
  |.|......F...|
  |.|.|....F...|
  |.|....-------
  |.---..|      
  |....|.|      
  ----...|      
     -----      
]]);
-- Since there isn't a return portal, this forces the hero's initial placement
des.teleport_region({ region = { 0, 0, 15, 15 } })

des.levregion({ region = {8,13,8,13}, exclude = {0,0,0,0}, type="branch" })
des.stair("up", 13, 10)
des.region({ region = { 0, 0, 15, 15 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,15,15))
des.non_passwall(selection.area(0,0,15,15))
des.door("locked", 14, 4)

-- Magic chest
des.feature("magic chest", 8, 14)

-- A little help
des.object("scroll of earth", 1, 1)
if percent(50) then
  des.object("scroll of earth", 2, 1)
end

des.object("boulder", 2, 4)
des.object("boulder", 2, 5)
des.object("boulder", 4, 5)
des.object("boulder", 6, 7)
des.object("boulder", 7, 7)
des.object("boulder", 9, 7)
des.object("boulder", 9, 8)
des.object("boulder", 6, 9)
des.object("boulder", 7, 9)
des.object("boulder", 8, 10)
des.object("boulder", 9, 10)
des.object("boulder", 7, 11)
des.trap("pit", 3, 3)
des.trap("pit", 4, 3)
des.trap("pit", 5, 3)
des.trap("pit", 6, 3)
des.trap("pit", 7, 3)
des.trap("pit", 8, 3)
des.trap("pit", 9, 3)
des.trap("pit", 10, 3)
des.trap("pit", 11, 3)
des.trap("pit", 12, 3)
des.trap("pit", 13, 3)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
