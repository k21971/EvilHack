-- NetHack tower tower2.lua	$NHDT-Date: 1781994889 2026/06/20 22:34:49 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
-- Intermediate stage of Vlad's tower
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify")
--         1         2         3
--123456789012345678901234567890
des.map({ halign = "half-left", valign = "center", map = [[
  --- ---   ------   --- ---  
  |.| |.|  -|....|-  |.| |.|  
---S---S---|......|---S---S---
|..........S......S..........|
|..........|......|..........|
------------......------------
   |......................|   
   |......................|   
   |......................|   
   |......................|   
--------------++--------------
|..........|......|..........|
|..........S......S..........|
---S---S---|......|---S---S---
  |.| |.|  -|....|-  |.| |.|  
  --- ---   ------   --- ---  
]] });
-- Random places are the 8 niches
local place = { {03,01},{07,01},{22,01},{26,01},
                {03,14},{07,14},{22,14},{26,14} }
shuffle(place);
des.ladder("down", 01, 03)
des.ladder("up", 28, 12)
-- Locked doors
des.door("locked", 03, 02)
des.door("locked", 07, 02)
des.door("locked", 22, 02)
des.door("locked", 26, 02)
des.door("locked", 03, 13)
des.door("locked", 07, 13)
des.door("locked", 22, 13)
des.door("locked", 26, 13)
des.door("locked", 14, 10)
des.door("locked", 15, 10)
-- Monsters and objects in the niches
des.monster({ class = "&", coord = place[1] })
des.monster({ class = "&", coord = place[2] })
des.monster({ id = "hell hound", coord = place[3] })
des.monster({ id = "hell hound", coord = place[4] })
des.monster({ class = "J", coord = place[5] })
des.object({ id = "chest", coord = place[7], contents = function()
  des.object("amulet of life saving")
end })
des.object({ id = "chest", coord = place[8], contents = function()
  des.object("amulet of strangulation")
end })
des.object({ id = "water walking boots", coord = place[3] })
des.object({ id = "crystal plate mail", coord = place[4] })
des.object({ id = "magic missile", class = "+", coord = place[5] })
-- Random noncorporeal undead
des.monster(" ", 05, 04)
des.monster(" ", 24, 04)
des.monster(" ", 05, 11)
des.monster(" ", 24, 11)
-- Vampire nests with demonic pets
des.monster("&", 06, 07)
des.monster("&", 06, 08)
des.monster("vampire noble", 07, 07)
des.monster("vampire noble", 07, 08)
des.monster("vampire noble", 08, 07)
des.monster("vampire noble", 08, 08)
des.monster("vampire noble", 09, 07)
des.monster("vampire noble", 09, 08)
des.monster("vampire noble", 20, 07)
des.monster("vampire noble", 20, 08)
des.monster("vampire noble", 21, 07)
des.monster("vampire noble", 21, 08)
des.monster("vampire noble", 22, 07)
des.monster("vampire noble", 22, 08)
des.monster("&", 23, 07)
des.monster("&", 23, 08)
-- More vampires
des.monster("vampire royal", 12, 13)
des.monster("vampire royal", 14, 14)
des.monster("vampire royal", 15, 14)
des.monster("vampire royal", 17, 13)
-- Random corpses of past adventurers
des.object({ id = "corpse", montype = "monk" })
des.object({ id = "corpse", montype = "cavewoman" })
des.object({ id = "corpse", montype = "knight" })
des.object({ id = "corpse", montype = "wizard" })
des.object({ id = "corpse", montype = "rogue" })
des.object({ id = "corpse", montype = "archeologist" })
des.object({ id = "corpse", montype = "tourist" })
-- Walls in the tower are non diggable and non passable
des.non_diggable(selection.area(00,00,29,15))
des.non_passwall(selection.area(00,00,29,15))
