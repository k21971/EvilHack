-- NetHack gehennom hellc-3.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- Stronghold
des.level_init({ style = "mines", fg = ".", bg = " ", smoothed = true,
                 joined = true, lit = 0, walled = true });

des.level_flags("mazelevel", "hardfloor")
--         1         2         3
--123456789012345678901234567890123
des.map([[
 LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL 
LLL-------LLL-FFFF-LLL---L---LLL
LLL|..\..|LL-|....|-LL|.|L|.|LLL
L---.....---|-FFFF-|---S---S---L
L|..........|......S..........|L
L|..........|......|..........|L
L-----++----|--++--|---....----L
LLLL|....|LL|......|LL|....|LLLL
LLLLF....|LLS......SLL|....FLLLL
LLLLF....|LL|......|LL|....FLLLL
LLLL|....|LL|......|LL|....|LLLL
L----....---|--++--|----++-----L
L|..........|......|..........|L
L|..........S......|..........|L
L---S---S---|-FFFF-|---S---S---L
LLL|.|L|.|LL-|....|-LL|.|L|.|LLL
LLL---L---LLL-FFFF-LLL---L---LLL
 LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL 
]]);
des.region({ region = { 1, 1, 30, 16 }, lit = 0, type = "ordinary" })
des.stair("down", 4, 2)
des.levregion({ region = { 1, 1, 79, 20 }, region_islev = 1,
                exclude = { 23, 1, 79, 20 }, exclude_islev = 1,
                type = "stair-up" })
des.levregion({ region = { 1, 1, 79, 20 }, region_islev = 1,
                exclude = { 23, 1, 79, 20 }, exclude_islev = 1,
                type = "branch" })
des.teleport_region({ region = { 1, 1, 79, 20 }, region_islev = 1,
                      exclude = { 23, 1, 79, 20 }, exclude_islev = 1 })
des.non_diggable(selection.area(0,0,29,15))

-- knock out the map corners, then flood the surrounding mines fill
-- with lava from those seeds
local seeds = { {0,0},{31,0},{0,17},{31,17} }
for i = 1, #seeds do
   des.terrain(seeds[i][1], seeds[i][2], " ")
end
for i = 1, #seeds do
   des.terrain(selection.floodfill(seeds[i][1], seeds[i][2]), "L")
end

-- Random 6 niches
local niches = { {23,2},{27,2},{4,15},
                 {8,15},{23,15},{27,15} }
shuffle(niches)

-- Drawbridges
des.drawbridge({ x = 31, y = 12, dir = "west", state = "closed" })
des.drawbridge({ x = 31, y = 13, dir = "west", state = "closed" })

-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster({ id = "Demogorgon", coord = { 6, 2 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "balrog", coord = { 4, 2 }, name = "Brulvon",
                    asleep = 1 })
      des.monster({ id = "pit fiend", coord = { 8, 2 }, name = "Dungrinoch",
                    asleep = 1 })
      des.monster({ id = "succubus", coord = { 2, 5 }, asleep = 1 })
      des.monster({ id = "incubus", coord = { 11, 5 }, asleep = 1 })
      des.monster({ id = "sandestin", coord = { 5, 4 }, name = "Anedak",
                    asleep = 1 })
      des.monster({ id = "horned devil", coord = { 7, 4 },
                    name = "Tholmennauth", asleep = 1 })
   else
      des.monster({ id = "Tiamat", coord = { 6, 2 }, peaceful = 0 })
      -- entourage - chaotic aligned dragons
      des.monster({ id = "black dragon", coord = { 4, 2 }, asleep = 1 })
      des.monster({ id = "green dragon", coord = { 8, 2 }, asleep = 1 })
      des.monster({ id = "red dragon", coord = { 2, 5 }, asleep = 1 })
      des.monster({ id = "orange dragon", coord = { 11, 5 }, asleep = 1 })
      des.monster({ id = "blue dragon", coord = { 5, 4 }, asleep = 1 })
      des.monster({ id = "shadow dragon", coord = { 7, 4 }, asleep = 1 })
   end
else
   if percent(50) then
      des.monster({ id = "Asmodeus", coord = { 6, 2 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "marilith", coord = { 4, 2 }, name = "Aglud",
                    asleep = 1 })
      des.monster({ id = "marilith", coord = { 8, 2 }, name = "Gaglokon",
                    asleep = 1 })
      des.monster({ id = "succubus", coord = { 2, 5 }, asleep = 1 })
      des.monster({ id = "incubus", coord = { 11, 5 }, asleep = 1 })
      des.monster({ id = "barbed devil", coord = { 5, 4 }, name = "Segmunich",
                    asleep = 1 })
      des.monster({ id = "barbed devil", coord = { 7, 4 }, name = "Volmimath",
                    asleep = 1 })
   else
      des.monster({ id = "Graz'zt", coord = { 6, 2 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "nalfeshnee", coord = { 4, 2 }, name = "Xarkadon",
                    asleep = 1 })
      des.monster({ id = "nalfeshnee", coord = { 8, 2 }, name = "Kerkunal",
                    asleep = 1 })
      des.monster({ id = "succubus", coord = { 2, 5 }, asleep = 1 })
      des.monster({ id = "incubus", coord = { 11, 5 }, asleep = 1 })
      des.monster({ id = "vrock", coord = { 5, 4 }, name = "Brorzos",
                    asleep = 1 })
      des.monster({ id = "vrock", coord = { 7, 4 }, name = "Tiglomil",
                    asleep = 1 })
   end
end

-- monsters
des.monster({ class = "&", coord = niches[1] })
des.monster({ class = "&", coord = niches[2] })
des.monster({ class = "V", coord = niches[3] })
des.monster({ class = "&", coord = niches[4] })
des.monster({ class = "&", coord = niches[5] })
des.monster({ class = "D", coord = niches[6] })
for _, c in ipairs({ {14,8},{14,9},{15,8},{15,9},{16,8},{16,9},{17,8},{17,9},
                     {24,5},{25,5} }) do
   des.monster({ coord = c })
end
-- surprise monsters
des.monster("salamander", 11, 8)
des.monster("salamander", 20, 8)
des.monster({ id = "marilith", coord = { 15, 2 }, asleep = 0,
              inventory = function()
   des.object({ id = "orcish bow", buc = "uncursed" })
   des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
end })
des.monster({ id = "marilith", coord = { 16, 15 }, asleep = 0,
              inventory = function()
   des.object({ id = "orcish bow", buc = "uncursed" })
   des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
end })
-- Locked doors
for _, c in ipairs({ {6,6},{7,6},{15,6},{16,6},{15,11},{16,11},{24,11},
                     {25,11} }) do
   des.door({ state = "locked", material = "bronze", x = c[1], y = c[2] })
end
-- Secret doors
for _, c in ipairs({ {4,14},{8,14},{12,8},{12,13},{19,4},{19,8},{23,3},
                     {23,14},{27,3},{27,14} }) do
   des.door({ state = "secret", material = "bronze", x = c[1], y = c[2] })
end
-- treasures
des.object({ id = "chest", coord = niches[1] })
des.object({ id = "chest", coord = niches[2] })
des.object({ id = "chest", coord = niches[3] })
des.object({ id = "chest", coord = niches[4] })
des.object({ id = "iron safe", coord = niches[5] })
des.object({ id = "crystal chest", coord = niches[6] })
-- some traps
des.trap("fire")
des.trap("fire")
des.trap("ice")
des.trap("ice")
des.trap("magic")
des.trap("magic beam")
des.trap("spear")
des.trap("spear")

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
