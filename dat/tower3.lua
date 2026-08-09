-- NetHack tower tower3.lua	$NHDT-Date: 1781994889 2026/06/20 22:34:49 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
-- Lower stage of Vlad's tower
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify")
--         1         2         3         4         5
--1234567890123456789012345678901234567890123456789012
des.map({ halign = "half-left", valign = "center", map = [[
-----------  --- ---   ------   --- ---  -----------
|.........|  |.| |.|  -|....|-  |.| |.|  |.........|
|...---...|---S---S---|......|---S---S---|...---...|
|.........|...........S......S.....................|
|...---...|...........|......|...............---...|
|.........|--+---------|....|------------|.........|
----...----...|........------........|...----...----
   |...|......|.............................|...|   
   |...|......|.............................|...|   
----...----...|........--++--........|...----...----
|.........|--+---------|....|------------|.........|
|...---...............|......|...............---...|
|.....................|......S.....................|
|...---...|---S---S---|......|---S---S---|...---...|
|.........|  |.| |.|  -|....|-  |.| |.|  |.........|
-----------  --- ---   ------   --- ---  -----------
]] });
-- Random places are the 8 niches
local place = { {14,01},{18,01},{33,01},{37,01},
                {14,14},{18,14},{33,14},{37,14} }
shuffle(place);
des.ladder("down", 05, 03)
des.ladder("up", 43, 07)
-- Locked doors
des.door("locked", 13, 05)
des.door("locked", 13, 10)
des.door("locked", 25, 09)
des.door("locked", 26, 09)
des.door("locked", 22, 03)
des.door("locked", 29, 03)
des.door("locked", 29, 12)
-- Gargoyles in the four corners
des.monster("winged gargoyle", 01, 01)
des.monster("winged gargoyle", 01, 14)
des.monster("winged gargoyle", 50, 01)
des.monster("winged gargoyle", 50, 14)
-- Zombies and skeletons patrol the hallways
des.monster("human zombie", 13, 03)
des.monster("ettin zombie", 14, 04)
des.monster("skeleton warrior", 15, 03)
des.monster("giant zombie", 16, 04)
des.monster("human zombie", 32, 03)
des.monster("human zombie", 34, 04)
des.monster("skeleton warrior", 35, 03)
des.monster("giant zombie", 36, 04)
des.monster("human zombie", 13, 11)
des.monster("ettin zombie", 14, 12)
des.monster("skeleton warrior", 15, 11)
des.monster("giant zombie", 16, 12)
des.monster("human zombie", 32, 11)
des.monster("human zombie", 34, 12)
des.monster("skeletal dragon", 35, 11)
des.monster("giant zombie", 36, 12)
-- Liches reside in the center chambers
-- along with some noncorporeal undead
des.monster("L", 26, 03)
des.monster("L", 26, 12)
des.monster(" ", 25, 03)
des.monster(" ", 25, 12)
-- The lycan horde guards the very center
-- with their vampire masters
des.monster("human werewolf", 17, 07)
des.monster("weredemon", 17, 08)
des.monster("weredemon", 18, 07)
des.monster("weredemon", 18, 08)
des.monster("human werewolf", 19, 07)
des.monster("human werewolf", 19, 08)
des.monster("human werewolf", 32, 07)
des.monster("human werewolf", 32, 08)
des.monster("weredemon", 33, 07)
des.monster("weredemon", 33, 08)
des.monster("weredemon", 34, 07)
des.monster("human werewolf", 34, 08)
des.monster("vampire noble", 42, 07)
des.monster("vampire noble", 42, 08)
-- Random objects and traps in the niches
des.object({ id = "long sword", coord = place[1] })
des.trap({ coord = place[1] })
des.object({ id = "lock pick", coord = place[2] })
des.trap({ coord = place[2] })
des.object({ id = "elven cloak", coord = place[3] })
des.trap({ coord = place[3] })
des.object({ id = "blindfold", coord = place[4] })
des.trap({ coord = place[4] })
des.object({ id = "levitation", class = "=", coord = place[5] })
des.trap({ coord = place[5] })
des.object({ id = "iron safe", coord = place[6] })
des.trap({ coord = place[6] })
-- Random corpses of past adventurers
des.object({ id = "corpse", coord = { 38, 06 }, montype = "wizard" })
des.object({ id = "corpse", coord = { 38, 09 }, montype = "samurai" })
des.object({ id = "corpse", montype = "barbarian" })
des.object({ id = "corpse", montype = "convict" })
des.object({ id = "corpse", montype = "knight" })
des.object({ id = "corpse", montype = "priest" })
des.object({ id = "corpse", montype = "rogue" })
des.object({ id = "corpse", montype = "valkyrie" })
des.object({ id = "corpse", montype = "healer" })
-- Some random traps
des.trap()
des.trap()
des.trap()
des.trap()
-- Walls in the tower are non diggable and non passable
des.non_diggable(selection.area(00,00,51,15))
des.non_passwall(selection.area(00,00,51,15))
