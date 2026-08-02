-- NetHack sokoban soko4-4.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "hardfloor", "mazelevel", "premapped", "sokoban", "noflip");
-- "Joseph L Traub"
--12345678901234567890123456789012345678901234567890
des.map([[
  -------------    
--|...........|    
|.+...........|    
|.|...........|----
|.|FFFFFFFFFFF|...|
|.F...........|...|
|.F.--............|
|.F..|.-----.--...|
|.F..|.....|.||...|
|.F.--.---.|.--...|
|.F....---........|
|.-FF-.......--...|
|............|-----
|..---.......|     
---- ------..|     
          ----     
]]);
-- Since there isn't a return portal, this forces the hero's initial placement
des.teleport_region({ region = { 0, 0, 18, 15 } })

des.levregion({ region = {11,14,11,14}, exclude = {0,0,0,0}, type="branch" })
des.stair("up", 12, 2)
des.region({ region = { 0, 0, 18, 15 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,18,15))
des.non_passwall(selection.area(0,0,18,15))
des.door("locked", 2, 2)

-- Magic chest
des.feature("magic chest", 12, 14)

-- A little help
des.object("scroll of earth", 1, 13)
if percent(50) then
  des.object("scroll of earth", 2, 13)
end

des.object("boulder", 8, 6)
des.object("boulder", 10, 6)
des.object("boulder", 15, 6)
des.object("boulder", 16, 6)
des.object("boulder", 16, 7)
des.object("boulder", 6, 8)
des.object("boulder", 7, 8)
des.object("boulder", 12, 8)
des.object("boulder", 16, 8)
des.object("boulder", 15, 9)
des.object("boulder", 11, 10)
des.object("boulder", 13, 10)
des.trap("pit", 1, 3)
des.trap("pit", 1, 4)
des.trap("pit", 1, 5)
des.trap("pit", 1, 6)
des.trap("pit", 1, 7)
des.trap("pit", 1, 8)
des.trap("pit", 1, 9)
des.trap("pit", 1, 10)
des.trap("pit", 3, 12)
des.trap("pit", 4, 12)
des.trap("pit", 5, 12)

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")
