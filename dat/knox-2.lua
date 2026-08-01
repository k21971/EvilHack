-- NetHack knox knox-2.lua	$NHDT-Date: 1547343821 2019/01/13 01:43:41 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- "Winterly Fort Ludios" by Patric Mueller <bhaak@gmx.net>
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "shortsighted")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
--------------------}}I-----------------------------------------------------
| |........|.........}II...................................................|
| |........|.........}II.............}}}}}}}....................}}}}}}}....|
| --S----S--..........}II............}-----}....................}-----}....|
|   #   |..............}III..........}|...|}}}}}}}}}}}}}}}}}}}}}}|...|}....|
|   #   |.............}}II...........}--------------------------------}....|
|   # ---.............}}II...........}}}|...............|..........|}}}....|
|   # |................II..............}|...............|..........|}......|
|   # |................}II.............}|...............|......\...|}......|
| --S----..............}III..........}}}|...............|..........|}}}....|
| |.....|.............}}IIII.........}--------------------------------}....|
| |.....+..............}}II..........}|...|}}}}}}}}}}}}}}}}}}}}}}|...|}....|
| |.....|...............}}I..........}-----}....................}-----}....|
| |.....|................}}I.........}}}}}}}....................}}}}}}}....|
| |..-S----...............}II..............................................|
| |..|....|..............}IIII.....|--+--|...........................|--+--|
| |..|....|.............}}}.IIII...|.....|..........|--+--|..........|.....|
| -----------..........}}}...III...+.....|----------|.....|----------|.....|
|           |.........}}}...III....|.......................................|
----------------------}}}----III--------------------------------------------
]]);
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Portal arrival point
des.levregion({ region = {08,16,08,16}, type="branch" });
-- accessible via ^V in wizard mode; arrive near the portal
des.teleport_region({ region = { 06, 15, 09, 16 }, dir = "up" })
des.teleport_region({ region = { 06, 15, 09, 16 }, dir = "down" })
--   Throne room, with Croesus on the throne
des.region({ region = { 57, 06, 66, 09 }, lit = 1, type = "throne", filled = 1 })
--   50% chance each to move throne and/or fort's entry secret door up one row
if percent(50) then
   des.monster({ id = "Croesus", x = 63, y = 08, peaceful = 0 })
   des.feature("magic chest", 62, 08)
else
   des.monster({ id = "Croesus", x = 63, y = 07, peaceful = 0 })
   des.terrain(63, 07, "\\")
   des.terrain(63, 08, ".")
   des.feature("magic chest", 62, 07)
end
if percent(50) then
   des.door({ state = "secret", material = "gold", x = 67, y = 08 })
   des.terrain(67, 07, "|")
else
   des.door({ state = "secret", material = "gold", x = 67, y = 07 })
   des.terrain(67, 08, "|")
end

--   The Vault
function treasure_spot(x,y)
   des.gold({ x = x, y = y, amount = 600 + math.random(0, 300) });
   if (math.random(0,2) == 0) then
      if (math.random(0,2) == 0) then
         des.trap("spiked pit", x,y);
      else
         des.trap("land mine", x,y);
      end
   end
end

des.region({ region = { 41, 06, 55, 09 }, lit = 1, type = "ordinary" })
local treasury = selection.area(41,06,55,09);
treasury:iterate(treasure_spot);

--   Vault entrance also varies
if percent(50) then
   des.terrain(56, 08, "|")
   des.door({ state = "secret", material = "gold", x = 56, y = 07 })
else
   des.terrain(56, 07, "|")
   des.door({ state = "secret", material = "gold", x = 56, y = 08 })
end
--   Corner towers
des.region(selection.area(39,04,41,04),"lit")
des.region(selection.area(66,04,68,04),"lit")
des.region(selection.area(39,11,41,11),"lit")
des.region(selection.area(66,11,68,11),"lit")
--   A welcoming committee
des.region({ region = { 03, 10, 07, 13 }, lit = 1, type = "zoo", filled = 1, irregular = 1 })
--   arrival chamber; needs to be a real room to control migrating monsters,
--   and `unfilled' is a kludge to force an ordinary room to remain a room
des.region({ region = { 06, 15, 09, 16 }, lit = 0, type = "ordinary", arrival_room = true })
--   Barracks
des.region({ region = { 36, 16, 36, 18 }, lit = 1, type = "barracks", filled = 1, irregular = 1 })
-- Doors
des.door("closed", 06, 14)
des.door("closed", 09, 03)
des.door("locked", 08, 11)
-- barrack doors
des.door("closed", 35, 17)
des.door("closed", 38, 15)
des.door("closed", 55, 16)
des.door("closed", 72, 15)
-- Secret doors to gem rooms
des.door({ state = "secret", material = "gold", x = 41, y = 05 })
des.door({ state = "secret", material = "gold", x = 41, y = 10 })
des.door({ state = "secret", material = "gold", x = 66, y = 05 })
des.door({ state = "secret", material = "gold", x = 66, y = 10 })
-- Soldier in watchtower
--MONSTER: '@', "lieutenant",(19,07),hostile
--OBJECT:'/',"fire",(19,07)
-- Soldiers guarding the fort
des.monster("soldier", 12, 14)
des.monster("soldier", 12, 13)
des.monster("soldier", 11, 10)
des.monster("soldier", 13, 02)
des.monster("soldier", 14, 03)
des.monster("soldier", 20, 02)
des.monster("soldier", 30, 02)
des.monster("soldier", 40, 01)
des.monster("soldier", 30, 16)
des.monster("soldier", 32, 16)
des.monster("soldier", 36, 08)
des.monster("soldier", 54, 15)
des.monster("soldier", 54, 13)
des.monster("soldier", 53, 12)
des.monster("soldier", 71, 10)
des.monster("soldier", 57, 03)
des.monster("lieutenant", 15, 08)
-- Four dragons guarding each side
des.monster("D", 32, 09)
des.monster("D", 49, 02)
des.monster("D", 50, 12)
-- this one isn't where he's supposed to be
des.monster("D", 25, 03)
-- Eels in the moat
des.monster("giant eel", 50, 04)
des.monster("giant eel", 60, 04)
des.monster("giant eel", 43, 12)
des.monster("giant eel", 68, 09)
-- The corner rooms treasures
des.object("diamond", 39, 04)
des.object("diamond", 40, 04)
des.object("diamond", 41, 04)
des.object("emerald", 39, 11)
des.object("emerald", 40, 11)
des.object("emerald", 41, 11)
des.object("ruby", 66, 04)
des.object("ruby", 67, 04)
des.object("ruby", 68, 04)
des.object("amethyst", 66, 11)
des.object("amethyst", 67, 11)
des.object("amethyst", 68, 11)
-- magic key is in an iron safe
des.object({ id = "iron safe", x = 03, y = 16, locked = 1,
   contents = function()
      des.object("magic key")
      des.object("*")
      des.object("=")
      des.object("\"")
   end
})
