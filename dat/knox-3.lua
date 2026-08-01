-- NetHack knox knox-3.lua	$NHDT-Date: 1547343821 2019/01/13 01:43:41 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- "Minas Tirith?" by Patric Mueller <bhaak@gmx.net>
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "shortsighted")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
----------------------------------------------------------------------------
|........|.......}|F|}.......}F|F|F}......+..................|.|   |.|   |.|
|........|.......}|.|}.......}F.|.F}......F...........|--F--+|.|   |.|   |.|
|-S----S--......}F|||F}......}F.|.F}......+.....|--F--|...}F.|.|   |.|   |.|
| #   |.........}F.|.F}......}F|F|F}......|--F--|.........}|.|--------------
| #   |.........}F|||F}.......}|.|}.......................}F.|..............
| # |--.........}|...|}......}F|F|F}......................}|.|............. 
| # |...........}F|||F}......}F.|.F}......................}F.|...........   
| # |...........}F.|.F}......}F.|.F}......................}|.|.........     
|-S----.........}F|||F}......}F|F|F}..........{{{.........}F.|.......       
|.....|..........}|.|}.......}|...|}..........{T{.........}|.|......        
|.....+.........}F|||F}......}F|F|F}..........{{{.........}F.|-----         
|.....|.........}F.|.F}......}F.|.F}......................}|.|.......       
|.....|.........}F|||F}......}F.|.F}......................}F.|........      
|..-S----.......}|...|}......}F|F|F}......................}|.|....\......   
|..|....|.......}F|||F}.......}|.|}.......................}F.|..............
|..|....|.......}F.|.F}......}F|F|F}......|--F--|.........}|.|--------------
|----------.....}F|||F}......}F.|.F}......+.....|--F--|...}F.|.|   |.|   |.|
|         |......}|.|}.......}F.|.F}......F...........|--F--+|.|   |.|   |.|
|         |......}|F|}.......}F|F|F}......+..................|.|   |.|   |.|
----------------------------------------------------------------------------
]]);
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- everything's lit
des.region(selection.area(00,00,75,19),"lit")
-- Portal arrival point
des.levregion({ region = {06,16,06,16}, type="branch" });
--   Throne room, with Croesus on the throne
des.region({ region = { 62, 12, 75, 15 }, lit = 1, type = "throne", filled = 1, irregular = 1 })
des.monster({ id = "Croesus", x = 66, y = 14, peaceful = 0 })
-- Magic chest
des.feature("magic chest", 67, 14)
--   50% chance each to move fort's entry secret door up one row
if percent(50) then
   des.door({ state = "secret", material = "gold", x = 61, y = 14 })
   des.terrain(61, 13, "|")
else
   des.door({ state = "secret", material = "gold", x = 61, y = 13 })
   des.terrain(61, 14, "|")
end

--   The Vault (ragged east side: stock the floor squares only)
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

des.region({ region = { 62, 05, 75, 10 }, lit = 1, type = "ordinary" })
local treasury = selection.area(62,05,75,10):filter_mapchar('.');
treasury:iterate(treasure_spot);

--   Vault entrance also varies
if percent(50) then
   des.terrain(64, 11, "|")
   des.door({ state = "secret", material = "gold", x = 65, y = 11 })
else
   des.terrain(65, 11, "|")
   des.door({ state = "secret", material = "gold", x = 64, y = 11 })
end
--   A welcoming committee
des.region({ region = { 01, 10, 05, 13 }, lit = 1, type = "zoo", filled = 1, irregular = 1 })
--   arrival chamber; needs to be a real room to control migrating monsters,
--   and `unfilled' is a kludge to force an ordinary room to remain a room
des.region({ region = { 04, 15, 07, 16 }, lit = 0, type = "ordinary", arrival_room = true })
--   Barracks
des.region({ region = { 43, 01, 44, 02 }, lit = 1, type = "barracks", filled = 1, irregular = 1 })
des.region({ region = { 43, 17, 44, 17 }, lit = 1, type = "barracks", filled = 1, irregular = 1 })
-- Drawbridges of first ringwall
des.drawbridge({ x = 30, y = 05, dir = "east", state = "closed" })
des.drawbridge({ x = 34, y = 05, dir = "west", state = "closed" })
des.drawbridge({ x = 29, y = 10, dir = "east", state = "closed" })
des.drawbridge({ x = 35, y = 10, dir = "west", state = "closed" })
des.drawbridge({ x = 30, y = 15, dir = "east", state = "closed" })
des.drawbridge({ x = 34, y = 15, dir = "west", state = "closed" })
-- Guards of first ringwall, with possible offensive wands
for _, x in ipairs({ 31, 33 }) do
   for _, y in ipairs({ 02, 03, 07, 08, 12, 13, 17, 18 }) do
      des.monster({ class = "C", x = x, y = y, peaceful = 0 })
      if percent(50) then
         des.object("/", x, y)
      end
   end
end
-- Drawbridges of second ringwall
des.drawbridge({ x = 16, y = 06, dir = "east", state = "closed" })
des.drawbridge({ x = 22, y = 06, dir = "west", state = "closed" })
des.drawbridge({ x = 17, y = 10, dir = "east", state = "closed" })
des.drawbridge({ x = 21, y = 10, dir = "west", state = "closed" })
des.drawbridge({ x = 16, y = 14, dir = "east", state = "closed" })
des.drawbridge({ x = 22, y = 14, dir = "west", state = "closed" })
-- Guards of first ringwall, with possible offensive wands
for _, y in ipairs({ 04, 08, 12, 16 }) do
   for _, x in ipairs({ 18, 20 }) do
      des.monster({ class = "C", x = x, y = y, peaceful = 0 })
      if percent(50) then
         des.object("/", x, y)
      end
   end
end
-- Doors
des.door("closed", 04, 14)
des.door("closed", 07, 03)
des.door("locked", 06, 11)
-- barrack doors
des.door("closed", 60, 02)
des.door("closed", 60, 18)
des.door("closed", 42, 01)
des.door("closed", 42, 03)
des.door("closed", 42, 17)
des.door("closed", 42, 19)
-- Secret doors to gem rooms
des.door({ state = "secret", material = "gold", x = 62, y = 04 })
des.door({ state = "secret", material = "gold", x = 62, y = 16 })
des.door({ state = "secret", material = "gold", x = 68, y = 04 })
des.door({ state = "secret", material = "gold", x = 68, y = 16 })
des.door({ state = "secret", material = "gold", x = 74, y = 04 })
des.door({ state = "secret", material = "gold", x = 74, y = 16 })
-- elves
des.monster({ id = "Grey-elf", coord = { 60, 03 }, peaceful = 0 })
des.monster({ id = "Woodland-elf", coord = { 60, 04 }, peaceful = 0 })
des.monster({ id = "Green-elf", coord = { 60, 05 }, peaceful = 0 })
des.monster({ id = "Grey-elf", coord = { 60, 06 }, peaceful = 0 })
des.monster({ id = "elven wizard", coord = { 60, 07 }, peaceful = 0 })
des.monster({ id = "Green-elf", coord = { 60, 08 }, peaceful = 0 })
des.monster({ id = "Grey-elf", coord = { 60, 09 }, peaceful = 0 })
des.monster({ id = "Woodland-elf", coord = { 60, 10 }, peaceful = 0 })
des.monster({ id = "elven royal", coord = { 60, 11 }, peaceful = 0 })
des.monster({ id = "Grey-elf", coord = { 60, 12 }, peaceful = 0 })
des.monster({ id = "elven wizard", coord = { 60, 13 }, peaceful = 0 })
des.monster({ id = "Green-elf", coord = { 60, 14 }, peaceful = 0 })
des.monster({ id = "elven noble", coord = { 60, 15 }, peaceful = 0 })
des.monster({ id = "Grey-elf", coord = { 60, 16 }, peaceful = 0 })
des.monster({ id = "Woodland-elf", coord = { 60, 17 }, peaceful = 0 })
-- Soldiers guarding the fort in front of ringwalls
des.monster("soldier", 12, 14)
des.monster("soldier", 12, 13)
des.monster("soldier", 11, 10)
des.monster("soldier", 13, 15)
des.monster("soldier", 14, 04)
des.monster("soldier", 12, 18)
des.monster("soldier", 13, 12)
des.monster("soldier", 14, 8)
des.monster("lieutenant", 12, 08)
des.monster("soldier", 28, 16)
des.monster("soldier", 24, 17)
des.monster("soldier", 25, 09)
des.monster("soldier", 25, 15)
des.monster("soldier", 26, 13)
des.monster("soldier", 27, 04)
des.monster("soldier", 28, 10)
des.monster("soldier", 23, 03)
des.monster("lieutenant", 26, 08)
-- Four dragons guarding the fort
des.monster("D", 42, 09)
des.monster("D", 50, 09)
des.monster("D", 49, 08)
des.monster("D", 46, 12)
-- Eels in the moat
des.monster("giant eel", 16, 04)
des.monster("giant eel", 22, 15)
des.monster("giant eel", 29, 07)
des.monster("giant eel", 35, 14)
-- The corner rooms treasures
des.object("diamond", 62, 17)
des.object("diamond", 62, 18)
des.object("diamond", 62, 19)
des.object("emerald", 74, 17)
des.object("emerald", 74, 18)
des.object("emerald", 74, 19)
des.object("ruby", 62, 01)
des.object("ruby", 62, 02)
des.object("ruby", 62, 03)
des.object("amethyst", 74, 01)
des.object("amethyst", 74, 02)
des.object("amethyst", 74, 03)
-- magic key is in an iron safe
des.object({ id = "iron safe", x = 01, y = 16, locked = 1,
   contents = function()
      des.object("magic key")
      des.object("*")
      des.object("=")
      des.object("\"")
   end
})
