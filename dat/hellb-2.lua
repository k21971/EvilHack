-- NetHack gehennom hellb-2.lua	$NHDT-Date: 1432512783 2015/05/25 00:13:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- The City of Dis, previously Dispater's domain
--
des.level_init({ style = "mines", fg = ".", bg = " ", smoothed = true,
                 joined = true, lit = 0, walled = true });

des.level_flags("mazelevel", "hardfloor", "shortsighted")
--         1         2         3
--1234567890123456789012345678901234567
des.map({ halign = "center", valign = "center", map = [[
  .................................  
......LLLLLLLLLLLLLLLLLLLLLLLLL......
...LLLLLL-------------------LLLLLL...
.LLLLL----.................----LLLLL.
LLL----.....-------------.....----LLL
LL--.....----...........----.....--LL
L||....---....----S----....---....||L
L|....--....---.......---....--....|L
L|...||...---..I-----I..---...||...|L
L|...|....|...I|-...-|I...|....|...|L
L|...S....|...IS.....SI...|....S...|L
L|...|....|...I|-...-|I...|....|...|L
L|...||...---..I-----I..---...||...|L
L|....--....---.......---....--....|L
L||....---....----S----....---....||L
LL--.....----...........----.....--LL
LLL----.....-------------.....----LLL
.LLLLL----.................----LLLLL.
...LLLLLL-------------------LLLLLL...
......LLLLLLLLLLLLLLLLLLLLLLLLL......
  .................................  
]] });
des.mazewalk(18, 1, "north")
des.mazewalk(18, 19, "south")
des.drawbridge({ x = 18, y = 1, dir = "south", state = "closed" })
des.drawbridge({ x = 18, y = 19, dir = "north", state = "closed" })
des.stair("down", 18, 10)
des.levregion({ region = { 1, 1, 79, 20 }, region_islev = 1,
                exclude = { 21, 1, 59, 20 }, exclude_islev = 1,
                type = "stair-up" })
des.levregion({ region = { 1, 1, 79, 20 }, region_islev = 1,
                exclude = { 21, 1, 59, 20 }, exclude_islev = 1,
                type = "branch" })
des.teleport_region({ region = { 1, 1, 79, 20 }, region_islev = 1,
                      exclude = { 21, 1, 59, 20 }, exclude_islev = 1 })
des.non_diggable(selection.area(0, 0, 36, 20))

-- knock out the four map corners, then flood the surrounding
-- mines fill with lava from those seeds
local seeds = { {0,0},{36,0},{0,20},{36,20} }
for i = 1, #seeds do
   des.terrain(seeds[i][1], seeds[i][2], " ")
end
for i = 1, #seeds do
   des.terrain(selection.floodfill(seeds[i][1], seeds[i][2]), "L")
end

-- Secret doors
des.door({ state = "secret", material = "bronze", x = 5, y = 10 })
des.door({ state = "secret", material = "bronze", x = 18, y = 6 })
des.door({ state = "secret", material = "bronze", x = 18, y = 14 })
des.door({ state = "secret", material = "bronze", x = 15, y = 10 })
des.door({ state = "secret", material = "bronze", x = 21, y = 10 })
des.door({ state = "secret", material = "bronze", x = 31, y = 10 })
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster("Mephistopheles", 18, 10)
      -- their minions
      des.monster({ id = "iron golem", coord = { 18, 9 }, peaceful = 0 })
      des.monster({ id = "pit fiend", coord = { 18, 11 }, peaceful = 0 })
      des.monster({ id = "horned devil", coord = { 17, 10 }, peaceful = 0 })
      des.monster({ id = "barbed devil", coord = { 19, 10 }, peaceful = 0 })
   else
      des.monster("Yeenoghu", 18, 10)
      -- their minions
      des.monster({ id = "flind", coord = { 18, 9 }, peaceful = 0 })
      des.monster({ id = "gnoll cleric", coord = { 18, 11 }, peaceful = 0 })
      des.monster({ id = "gnoll hunter", coord = { 17, 10 }, peaceful = 0 })
      des.monster({ id = "gnoll hunter", coord = { 19, 10 }, peaceful = 0 })
   end
else
   if percent(50) then
      des.monster("Dispater", 18, 10)
      -- their minions
      des.monster({ id = "iron golem", coord = { 18, 9 }, peaceful = 0 })
      des.monster({ id = "pit fiend", coord = { 18, 11 }, peaceful = 0 })
      des.monster({ id = "bone devil", coord = { 17, 10 }, peaceful = 0 })
      des.monster({ id = "bone devil", coord = { 19, 10 }, peaceful = 0 })
   else
      des.monster("Lolth", 18, 10)
      -- their minions
      des.monster({ id = "gargantuan spider", coord = { 18, 9 },
                    peaceful = 0 })
      des.monster({ id = "drow cleric", coord = { 18, 11 }, peaceful = 0 })
      des.monster({ id = "drow warrior", coord = { 17, 10 }, peaceful = 0 })
      des.monster({ id = "drider", coord = { 19, 10 }, peaceful = 0 })
   end
end
-- their subordinates
for i = 1, 10 do
   des.monster({ class = "&", peaceful = 0 })
end
for i = 1, 11 do
   des.monster({ class = "i", peaceful = 0 })
end
for i = 1, 4 do
   des.monster({ class = " ", peaceful = 0 })
end
-- the damned
for i = 1, 4 do
   des.monster({ class = "z", peaceful = 0 })
end
for i = 1, 6 do
   des.monster({ class = "Z", peaceful = 0 })
end
for i = 1, 5 do
   des.monster({ class = "M", peaceful = 0 })
end
for i = 1, 5 do
   des.monster({ class = "V", peaceful = 0 })
end
des.monster({ class = "W", peaceful = 0 })
for i = 1, 4 do
   des.monster({ id = "wraith", peaceful = 0 })
end
des.monster({ id = "salamander", coord = { 16, 1 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 20, 19 }, peaceful = 0 })
des.monster({ id = "salamander", peaceful = 0 })
des.monster({ id = "salamander", peaceful = 0 })
-- a bit of loot
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object("(")
des.object("/")
des.object("=")
des.object("?")
des.object("?")
des.object("!")
des.object("!")
for i = 1, 8 do
   des.object("*")
end
-- traps!
for i = 1, 10 do
   des.trap("fire")
end
for i = 1, 3 do
   des.trap("magic")
end
for i = 1, 5 do
   des.trap()
end

-- the lava river
local river
if percent(50) then
   river = selection.randline(selection.new(), 36, 0, 50, 20, 10)
else
   river = selection.randline(selection.new(), 50, 0, 36, 20, 10)
end
-- grow(north | west, ...): the north- and west-neighbors of the
-- original line, not a sequential two-pass grow
river = river | river:clone():grow("north") | river:clone():grow("west")
des.terrain(river, "L")
