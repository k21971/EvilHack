-- NetHack sokoban soko4-6.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "hardfloor", "mazelevel", "premapped", "sokoban", "noflip");
-- "Joseph L Traub"
--12345678901234567890123456789012345678901234567890
des.map([[
-----------------
|.......|...--..|
|.-----.|.......|
|.---...|.......|
|...........--..|
|......--...||..|
|FFFFF-------|.-|
|...........|-..|
|.....-------...|
|..L............|
|.....-------...|
|.....|     |...|
-------     -----
]]);
-- Since there isn't a return portal, this forces the hero's initial placement
des.teleport_region({ region = { 0, 0, 16, 12 } })

des.levregion({ region = {1,5,1,5}, exclude = {0,0,0,0}, type="branch" })
des.stair("up", 11, 7)
des.region({ region = { 0, 0, 16, 12 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,16,12))
des.non_passwall(selection.area(0,0,16,12))

-- Magic chest
des.feature("magic chest", 2, 4)

-- A little help
des.object("scroll of earth", 14, 1)
if percent(50) then
  des.object("scroll of earth", 15, 1)
end

des.object("boulder", 12, 2)
des.object("boulder", 6, 3)
des.object("boulder", 12, 3)
des.object("boulder", 6, 4)
des.object("boulder", 7, 4)
des.object("boulder", 14, 4)
des.object("boulder", 14, 7)
des.object("boulder", 12, 9)
des.object("boulder", 13, 9)
des.object("boulder", 14, 10)
des.trap("pit", 6, 7)
des.trap("pit", 7, 7)
des.trap("pit", 8, 7)
des.trap("pit", 9, 7)
des.trap("pit", 10, 7)
des.trap("pit", 6, 9)
des.trap("pit", 7, 9)
des.trap("pit", 8, 9)
des.trap("pit", 9, 9)
des.trap("pit", 10, 9)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")


--## Second level ###