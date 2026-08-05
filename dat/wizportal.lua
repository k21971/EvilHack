-- NetHack yendor wizportal.lua	$NHDT-Date: 1781994891 2026/06/20 22:34:51 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--
-- Guarded portal to the Wizard's Tower.
des.level_init({ style = "mines", fg = ".", bg = " ", smoothed = true,
                 joined = true, lit = 0, walled = true });

des.level_flags("mazelevel")
--         1         2
--123456789012345678901
des.map([[
 LLLLLLLLLLLLLLLLLLL 
LLL...LLLLLLLLL...LLL
LL.....L-----L.....LL
L......LF...FL......L
L......L-FFF-L......L
LL.................LL
LLL...............LLL
LLLLL..LLLLLLL..LLLLL
LL-F-..LL---LL..-F-LL
LL|.F..L--.--L..F.|LL
 L|.F..L|...|L..F.|L 
LL|.F..L--.--L..F.|LL
LL-F-..LL---LL..-F-LL
LLLLL..LLLLLLL..LLLLL
LLL...............LLL
LL.................LL
L......L-FFF-L......L
L......LF...FL......L
LL.....L-----L.....LL
LLL...LLLLLLLLL...LLL
 LLLLLLLLLLLLLLLLLLL 
]]);
des.levregion({ region = { 1, 0, 79, 20 }, region_islev = 1,
                exclude = { 0, 0, 20, 20 }, type = "stair-up" })
des.levregion({ region = { 1, 0, 79, 20 }, region_islev = 1,
                exclude = { 0, 0, 20, 20 }, type = "stair-down" })
des.teleport_region({ region = { 1, 0, 79, 20 }, region_islev = 1,
                      exclude = { 8, 8, 12, 12 } })
des.levregion({ region = { 10, 10, 10, 10 }, exclude = { 0, 0, 0, 0 },
                type = "branch" })
des.region({ region = { 10, 9, 12, 12 }, lit = 0, type = "ordinary",
             irregular = 1, arrival_room = true })

-- knock out the map corners and edge midpoints, then flood the
-- surrounding mines fill with lava from those seeds
local seeds = { {0,0},{0,10},{0,20},{20,0},{20,10},{20,20} }
for i = 1, #seeds do
   des.terrain(seeds[i][1], seeds[i][2], " ")
end
for i = 1, #seeds do
   des.terrain(selection.floodfill(seeds[i][1], seeds[i][2]), "L")
end

-- Tower monsters
if percent(50) then
   des.monster("balrog", 10, 10)
else
   des.monster("vorpal jabberwock", 10, 10)
end
des.monster("vampire noble", 9, 10)
des.monster("salamander", 8, 8)
des.monster("salamander", 12, 12)
des.monster("salamander", 1, 10)
des.monster("salamander", 19, 10)
-- Archers in the watch towers
for _, c in ipairs({ {3,10},{17,10},{10,3},{10,17} }) do
   des.monster({ id = "marilith", coord = c, asleep = 0,
                 inventory = function()
      des.object({ id = "orcish bow", buc = "uncursed" })
      des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
   end })
end
-- And to make things a little harder.
des.trap("board", 10, 9)
des.trap("board", 10, 11)
des.trap("board", 9, 10)
des.trap("board", 11, 10)

-- chance of evil critters
if percent(50) then
   des.monster({ class = "L", peaceful = 0 })
else
   des.monster({ peaceful = 0 })
end
-- some flavor from GruntHack
if percent(50) then
   des.monster({ id = "shade", peaceful = 0 })
   des.monster({ id = "shade", peaceful = 0 })
else
   des.monster({ id = "skeleton warrior", peaceful = 0 })
   des.monster({ id = "skeleton warrior", peaceful = 0 })
end
if percent(66) then
   -- could be a snark now, so up the percentage a bit
   des.monster({ class = "J", peaceful = 0 })
else
   des.monster({ peaceful = 0 })
end

if percent(75) then
   des.monster({ id = "salamander", peaceful = 0 })
   des.monster({ id = "salamander", peaceful = 0 })
else
   des.monster({ id = "salamander", peaceful = 0 })
   des.monster({ id = "lava demon", peaceful = 0 })
end

-- the lava river
local river
if percent(50) then
   river = selection.randline(selection.new(), 30, 0, 50, 20, 10)
else
   river = selection.randline(selection.new(), 50, 0, 30, 20, 10)
end
-- grow(north | west, ...): the north- and west-neighbors of the
-- original line, not a sequential two-pass grow
river = river | river:clone():grow("north") | river:clone():grow("west")
des.terrain(river, "L")
