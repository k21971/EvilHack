-- NetHack gehennom hellb-3.lua	$NHDT-Date: 1432512783 2015/05/25 00:13:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- Previously Mephistopheles' lair
--
des.level_init({ style = "mines", fg = ".", bg = " ", smoothed = true,
                 joined = true, lit = 0, walled = true });

des.level_flags("mazelevel", "hardfloor")
--         1         2
--12345678901234567890123456789
des.map({ halign = "center", valign = "center", map = [[
 LLLLLLLLLLLLLLLLLLLLLLLLLLL 
LLLL....-------------....LLLL
LL...---|...........|---...LL
LL...|.................|...LL
LL..|-...LL.......LL...--..LL
LL..|...LLLL.....LLLL...|..LL
LL..|...LLLL.....LLLL...|..LL
LL..|-...LL.......LL...-|..LL
LL...|........L........|...LL
LL...---.....LLL.....---...LL
LL.....|---.......---|.....LL
LL........|.|.|.|.|........LL
LL........|...|...|........LL
LL........|-------|........LL
LL.........LLL|LLL.........LL
LLLL.....................LLLL
 LLLLLLLLLLLLLLLLLLLLLLLLLLL 
]] });
des.non_diggable(selection.area(0, 0, 28, 16))
des.region({ region = { 0, 0, 28, 16 }, lit = 0, type = "ordinary" })
des.stair("down", 14, 3)
des.levregion({ region = { 1, 1, 79, 20 }, region_islev = 1,
                exclude = { 21, 1, 54, 20 }, exclude_islev = 1,
                type = "stair-up" })
des.levregion({ region = { 1, 1, 79, 20 }, region_islev = 1,
                exclude = { 21, 1, 54, 20 }, exclude_islev = 1,
                type = "branch" })
des.teleport_region({ region = { 1, 1, 79, 20 }, region_islev = 1,
                      exclude = { 21, 1, 54, 20 }, exclude_islev = 1 })

-- knock out the four map corners, then flood the surrounding
-- mines fill with lava from those seeds
local seeds = { {0,0},{28,0},{0,16},{28,16} }
for i = 1, #seeds do
   des.terrain(seeds[i][1], seeds[i][2], " ")
end
for i = 1, #seeds do
   des.terrain(selection.floodfill(seeds[i][1], seeds[i][2]), "L")
end

-- two drawbridges
des.drawbridge({ x = 12, y = 14, dir = "north", state = "closed" })
des.drawbridge({ x = 16, y = 14, dir = "north", state = "closed" })
-- random demons
for i = 1, 16 do
   des.monster("&")
end
-- random monsters
for i = 1, 11 do
   des.monster()
end
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster("Mephistopheles", 14, 3)
      -- entourage
      des.monster({ id = "marilith", coord = { 12, 3 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 16, 3 }, peaceful = 0 })
   else
      des.monster("Yeenoghu", 14, 3)
      -- entourage
      des.monster({ id = "flind", coord = { 12, 3 }, peaceful = 0 })
      des.monster({ id = "gnoll cleric", coord = { 16, 3 }, peaceful = 0 })
   end
else
   if percent(50) then
      des.monster("Dispater", 14, 3)
      -- entourage
      des.monster({ id = "vrock", coord = { 12, 3 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 16, 3 }, peaceful = 0 })
   else
      des.monster("Lolth", 14, 3)
      -- entourage
      des.monster({ id = "drow cleric", coord = { 12, 3 }, peaceful = 0 })
      des.monster({ id = "drider", coord = { 16, 3 }, peaceful = 0 })
   end
end
-- lava
des.monster("salamander", 14, 8)
des.monster("salamander", 11, 5)
des.monster("salamander", 17, 5)
-- random objects
for i = 1, 12 do
   des.object()
end
-- various traps
des.trap("land mine")
des.trap("land mine")
des.trap("fire")
des.trap("magic beam")
des.trap("spear")
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
