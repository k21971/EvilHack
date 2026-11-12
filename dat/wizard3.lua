-- $NHDT-Header: NetHack 5.0 wizard3.lua 2026/10/27_14:28:17 NetHack-5.0:1.8 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--
-- The bottom wizard level.
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "nommap")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
...TTT..T.......}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}.....t....TTT
............t..}}---------------------------------------------}}...TTT.....T
..........}}}}}}}|....|L...LL.....LLL|...|LLLLL....L...L|....|}}}}}}}T......
..........}--FF--|....|LL.L...LL....L|...|LLL....L...L.L|....|--FF--}.......
.....T....}|...|-|....|LL...L....L..L-----L...LL.....LLL|....|-|...|}.......
..........}F...|......|LL......LL...LLLLLLL.........LL.L|......|...F}.TT....
..........}|...|......|L...LLL....LLL...L.....LLLL.....L|......|...|}.......
.TT.......}|...|--S-----------------------------------------S--|...|}....T..
..T.......}F...|..........|.......|.........|.......|..........|...F}....T..
..........}F...|..........|.......|...T.T...|.......|..........|...F}...t...
..........}|...|..........|.......S..T.{.T..S.......|..........|...|}.......
....t.....}|...|..........|.......|...T.T...|.......|..........|...|}.......
..........}F...|..........|.......|.........|.......|..........|...F}.......
.....TT...}|...|-----..---|-------|---------|-------|---..-----|...|}.....TT
..........}--FF--|........|.......|...F.F...|.......|........|--FF--}.......
..........}}}}}}}|........|.......|...F.F...|.......|........|}}}}}}}.......
tTT............}}-----------F---F-------------F---F-----------}}.......TTt..
.......T........}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}...T.........
......TT....................................................................
............................TT..............................TTTT............
]]);
des.teleport_region({ region = { 0, 0, 4, 6 }, exclude = { 0, 0, 0, 0 } })
des.region({ region = { 1, 0, 79, 20 }, lit = 1, type = "ordinary" })
-- constrains monster migration
des.region({ region = { 16, 2, 62, 15 }, lit = 1, type = "ordinary",
             arrival_room = true })
des.region({ region = { 12, 4, 14, 13 }, lit = 1, type = "ordinary",
             arrival_room = true })
des.region({ region = { 64, 4, 66, 13 }, lit = 1, type = "ordinary",
             arrival_room = true })
des.levregion({ region = { 1, 1, 1, 1 }, exclude = { 0, 0, 0, 0 },
                type = "branch" })

-- random wandering monster locations
local place = { {6,6},{1,8},{4,17},{22,19},
                {56,18},{70,17},{72,8},{74,1} }
shuffle(place);

-- random monsters
des.monster({ class = "D", coord = place[1], peaceful = 0 })
des.monster({ class = "J", coord = place[2], peaceful = 0 })
des.monster({ class = "H", coord = place[3], peaceful = 0 })
des.monster({ class = "V", coord = place[4], peaceful = 0 })
des.monster({ class = "D", coord = place[5], peaceful = 0 })
des.monster({ class = "L", coord = place[6], peaceful = 0 })
des.monster({ class = "D", coord = place[7], peaceful = 0 })
des.monster({ class = "e", coord = place[8], peaceful = 0 })

-- player monster barbarian
des.monster({ id = "barbarian", class = "@", coord = { 66, 1 },
              peaceful = 0 })

-- moat
des.monster("kraken", 10, 2)
des.monster("kraken", 10, 15)
des.monster("kraken", 68, 2)
des.monster("kraken", 68, 15)
des.monster("shark", 16, 0)
des.monster("shark", 62, 0)
des.monster("shark", 30, 0)
des.monster("shark", 48, 0)
des.monster("shark", 10, 7)
des.monster("shark", 10, 10)
des.monster("giant eel", 15, 1)
des.monster("giant eel", 15, 16)
des.monster("giant eel", 63, 1)
des.monster("giant eel", 63, 16)
des.monster("electric eel", 37, 0)
des.monster("electric eel", 38, 0)
des.monster("electric eel", 40, 0)
des.monster("electric eel", 41, 0)
des.monster("shark", 16, 17)
des.monster("shark", 62, 17)
des.monster("shark", 30, 17)
des.monster("shark", 48, 17)
des.monster("shark", 68, 7)
des.monster("shark", 68, 10)
des.monster("giant eel", 13, 2)
des.monster("giant eel", 13, 15)
des.monster("giant eel", 65, 2)
des.monster("giant eel", 65, 15)
des.monster("electric eel", 37, 17)
des.monster("electric eel", 38, 17)
des.monster("electric eel", 40, 17)
des.monster("electric eel", 41, 17)

-- barracks / soldier areas
des.region({ region = { 27, 8, 33, 12 }, lit = 1, type = "barracks",
             filled = 1 })
des.region({ region = { 45, 8, 51, 12 }, lit = 1, type = "barracks",
             filled = 1 })
des.monster("soldier", 13, 5)
des.monster("sergeant", 13, 8)
des.monster("sergeant", 13, 9)
des.monster("soldier", 13, 12)
des.monster("soldier", 65, 5)
des.monster("sergeant", 65, 8)
des.monster("sergeant", 65, 9)
des.monster("soldier", 65, 12)
des.monster("sergeant", 28, 15)
des.monster("sergeant", 32, 15)
des.monster("sergeant", 46, 15)
des.monster("sergeant", 50, 15)

-- officers quarters
des.monster("lieutenant", 19, 3)
des.monster("lieutenant", 19, 4)
des.monster("lieutenant", 20, 4)
des.monster("captain", 20, 5)
des.monster("lieutenant", 59, 3)
des.monster("lieutenant", 59, 4)
des.monster("lieutenant", 58, 4)
des.monster("captain", 58, 5)

-- stables
des.monster({ id = "horse", coord = { 18, 15 }, asleep = 1 })
des.monster({ id = "warhorse", coord = { 20, 15 }, asleep = 1 })
des.monster({ id = "nightmare", coord = { 23, 15 }, asleep = 1 })
des.monster({ id = "woolly mammoth", coord = { 25, 15 }, asleep = 1 })
des.monster({ id = "large cave lizard", coord = { 53, 15 }, asleep = 1 })
des.monster({ id = "warhorse", coord = { 55, 15 }, asleep = 1 })
des.monster({ id = "nightmare", coord = { 58, 15 }, asleep = 1 })
des.monster({ id = "cauchemar", coord = { 60, 15 }, asleep = 1 })
des.monster("soldier", 21, 14)
des.monster("soldier", 57, 14)

-- forge/training areas
des.monster("sergeant", 19, 9)
des.monster("soldier", 19, 11)
des.monster("soldier", 22, 9)
des.monster("soldier", 22, 11)
des.monster("soldier", 56, 9)
des.monster("soldier", 56, 11)
des.monster("sergeant", 59, 9)
des.monster("soldier", 59, 11)

-- lava room
local lava = { {23,2},{26,2},{30,2},{34,2},
               {43,2},{46,2},{51,2},{54,2},
               {24,4},{32,4},{44,4},{49,4},
               {23,6},{27,6},{32,6},{36,6},
               {42,6},{48,6},{53,6},{55,6} }
shuffle(lava);

local lava_mons = { "red dragon", "red dragon", "lava demon", "lava demon",
                    "flaming sphere", "flaming sphere",
                    "salamander", "salamander", "salamander",
                    "salamander", "salamander", "salamander",
                    "fire elemental", "fire elemental",
                    "balrog", "balrog", "pit fiend", "pit fiend",
                    "barbed devil", "barbed devil" }
for i = 1, #lava_mons do
   des.monster({ id = lava_mons[i], coord = lava[i], peaceful = 0 })
end

-- foyer
for _, c in ipairs({ {36,14},{36,15},{42,14},{42,15} }) do
   des.monster({ id = "marilith", coord = c, asleep = 0,
                 inventory = function()
      des.object({ id = "orcish bow", buc = "uncursed" })
      des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
   end })
end

-- various loot
des.object({ id = "chest", coord = { 18, 14 }, locked = 1,
             contents = function()
   des.object({ id = "apple", quantity = 3 })
   des.object({ id = "carrot", quantity = 3 })
   des.object({ id = "saddle", buc = "uncursed" })
   if percent(20) then
      if percent(80) then
         des.object({ id = "spiked barding", buc = "uncursed" })
      else
         des.object({ id = "barding of reflection", buc = "uncursed" })
      end
   end
end })
des.object({ id = "chest", coord = { 60, 14 }, locked = 1,
             contents = function()
   des.object({ id = "apple", quantity = 3 })
   des.object({ id = "tripe ration", quantity = 3 })
   des.object({ id = "saddle", buc = "uncursed" })
   if percent(20) then
      if percent(80) then
         des.object({ id = "spiked barding", buc = "uncursed" })
      else
         des.object({ id = "barding of reflection", buc = "uncursed" })
      end
   end
end })
des.object("chest", 18, 2)
des.object("chest", 60, 2)

-- stairs up
des.door({ state = "locked", material = "iron", x = 39, y = 4 })
des.ladder("up", 39, 2)

-- drawbridge
des.drawbridge({ x = 39, y = 17, dir = "north", state = "closed" })

-- random forge
if percent(50) then
   des.feature("forge", 17, 11)
end
if percent(50) then
   des.feature("forge", 61, 11)
end

-- doors
des.door({ state = "locked", material = "iron", x = 26, y = 9 })
des.door({ state = "locked", material = "iron", x = 26, y = 11 })
des.door({ state = "locked", material = "iron", x = 39, y = 13 })
des.door({ state = "locked", material = "iron", x = 52, y = 9 })
des.door({ state = "locked", material = "iron", x = 52, y = 11 })
-- secret doors
des.door({ state = "secret", material = "iron", x = 18, y = 7 })
des.door({ state = "secret", material = "iron", x = 60, y = 7 })
des.door({ state = "secret", material = "iron", x = 34, y = 10 })
des.door({ state = "secret", material = "iron", x = 44, y = 10 })
-- random door
if percent(50) then
   des.door({ state = "locked", material = "iron", x = 22, y = 4 })
else
   des.door({ state = "locked", material = "iron", x = 56, y = 4 })
end

-- random secret doors
if percent(20) then
   des.door({ state = "secret", material = "iron", x = 62, y = 3 })
   des.door({ state = "secret", material = "iron", x = 62, y = 4 })
end

-- none shall pass
des.non_diggable(selection.area(11, 1, 67, 16))
des.non_passwall(selection.area(11, 1, 67, 16))
