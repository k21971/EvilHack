-- NetHack castle castle-3.lua	$NHDT-Date: 1432512783 2015/05/25 00:13:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport")

--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
---------------------------------------------.LLLLLLL.......................
|...S..S..|..S..S.......|.|.......|....S..F.|.LLLLLL........................
|.|---F-F---F-F--.......+.+.......|....|-----LLLLLLLL.............------....
|.S.|...........+.......|.|.......|....|...LLLLLLL.LLLLL..L.......|....|....
|.|.F...........---------+---------+----LLLLLLL.......LLLL........+.|..|....
|.|-|.\.........+...................|.LLLLL....L.....LLLL.........|....|....
|.|.F...........---------------------LLLLLL............LLLL.......------....
|.S.|...........+...FLLLLLLLLLLLLL.LLLLL.LLLL...L.......LLLLL...............
|.|---F--F-F--F-|...FLL.....LLLLLLLLLL.....LLLL..L....LLLLLLLLL.............
|...|..|..|..|..S...FL.L...L...LLLLLLL........LLLL.LLLLLLL.LLLLLLL......LLLL
|.{.-S--S---S--S|...FLL......LLLL-------S-------LLLLLLL-------LLL.LLLLLLLLLL
|...............|----LLLL...LL..L| - - - - - - - - LL- - - - |LL-LLLLLLLLLLL
|.-+--+--+-------LLLLLLLLLLL.....|-----------------LLL-------LL---LLLLLLLLLL
|.|..|..|..F...LLL..........L....| - - - - - - - - -L- - - - - - |...LLLLLLL
|+--F--F----LLLL...---------L-----------------------------------------LL.LL.
|..FL....LLLLLL....| - - - - - - - - - - - - - - - - - - - - - - - - LL.....
----LLLLLLL...LL...|-------------------------------------------------|......
LLLLLLLLLLL........+ - - - - - - - - - - - - - - - - - - - - - - - - S......
LLLLLLLL..LL.......|-------------------------------------------------|......
LLLLLL.............| - - - - - - - - - - - - - - - - - - - - - - - - |......
LLLL...............---------------------------------------------------......
]]);

-- Random registers initialization
local place = { {15,01},{11,01},{09,01},{05,01},
                {03,03},{03,07},{05,09},{08,09},
                {12,09},{15,09} }
shuffle(place);

local monster = { "L", "L", "E", "E", "R", "R", "T", "T", "O", "O" }
shuffle(monster);

-- everything except the inner walls of the maze are non-diggable
des.non_diggable(selection.area(00,00,19,20))
des.non_diggable(selection.area(19,00,33,14))
des.non_diggable(selection.area(33,00,75,10))
des.non_diggable(selection.area(65,00,75,14))
des.non_diggable(selection.area(69,00,75,20))

des.teleport_region({ region = { 33, 15, 68, 20 }, exclude = { 10, 00, 18, 14 }, dir = "down" })
des.teleport_region({ region = { 47, 01, 75, 09 }, exclude = { 17, 00, 44, 09 }, dir = "up" })
des.levregion({ region = { 33, 15, 68, 20 }, exclude = { 00, 00, 00, 00 }, type = "stair-up" })

des.region({ region = { 00, 00, 75, 20 }, lit = 1, type = "ordinary" })

-- the maze is dark
des.region({ region = { 20, 15, 68, 20 }, lit = 0, type = "ordinary" })
des.region({ region = { 34, 11, 60, 14 }, lit = 0, type = "ordinary" })
des.region({ region = { 60, 13, 64, 14 }, lit = 0, type = "ordinary" })

-- the hallways in the castle are dark
des.region({ region = { 17, 05, 35, 05 }, lit = 0, type = "ordinary" })
des.region({ region = { 25, 00, 25, 04 }, lit = 0, type = "ordinary" })

des.region({ region = { 01, 01, 03, 01 }, lit = 0, type = "ordinary" })
des.region({ region = { 01, 01, 01, 13 }, lit = 0, type = "ordinary" })
des.region({ region = { 01, 09, 03, 11 }, lit = 0, type = "ordinary" })
des.region({ region = { 01, 11, 15, 11 }, lit = 0, type = "ordinary" })

-- the special rooms in the castle
des.region({ region = { 05, 03, 15, 07 }, lit = 1, type = "throne", filled = 2, irregular = 1 })
des.region({ region = { 17, 01, 23, 03 }, lit = 1, type = "barracks", filled = 1 })
des.region({ region = { 27, 01, 33, 03 }, lit = 1, type = "barracks", filled = 1 })
des.region({ region = { 17, 07, 19, 10 }, lit = 1, type = "barracks", filled = 1 })

des.mazewalk(33, 13, "east")

des.feature("fountain", 02, 10)

des.drawbridge({ x = 25, y = 07, dir = "north", state = "closed" })

-- the castle doors
des.door({ state = "secret", material = "bronze", x = 44, y = 01 })
des.door({ state = "locked", material = "bronze", x = 39, y = 01 })
des.door({ state = "locked", material = "bronze", x = 35, y = 04 })
des.door({ state = "locked", material = "bronze", x = 26, y = 02 })
des.door({ state = "locked", material = "bronze", x = 24, y = 02 })
des.door({ state = "locked", material = "bronze", x = 25, y = 04 })
des.door({ state = "locked", material = "bronze", x = 16, y = 01 })
des.door({ state = "locked", material = "bronze", x = 16, y = 03 })
des.door({ state = "locked", material = "bronze", x = 16, y = 05 })
des.door({ state = "locked", material = "bronze", x = 16, y = 07 })
des.door({ state = "locked", material = "bronze", x = 16, y = 09 })
des.door({ state = "locked", material = "bronze", x = 15, y = 10 })
des.door({ state = "locked", material = "bronze", x = 13, y = 01 })
des.door({ state = "locked", material = "bronze", x = 12, y = 10 })
des.door({ state = "locked", material = "bronze", x = 09, y = 12 })
des.door({ state = "locked", material = "bronze", x = 08, y = 10 })
des.door({ state = "locked", material = "bronze", x = 07, y = 01 })
des.door({ state = "locked", material = "bronze", x = 06, y = 12 })
des.door({ state = "locked", material = "bronze", x = 05, y = 10 })
des.door({ state = "locked", material = "bronze", x = 04, y = 01 })
des.door({ state = "locked", material = "bronze", x = 03, y = 12 })
des.door({ state = "locked", material = "bronze", x = 02, y = 03 })
des.door({ state = "locked", material = "bronze", x = 02, y = 07 })
des.door({ state = "locked", material = "bronze", x = 01, y = 14 })

-- door of the small building
des.door({ state = "random", material = "bronze", x = 66, y = 04 })

-- doors for the maze
des.door({ state = "locked", material = "bronze", x = 69, y = 17 })
des.door({ state = "locked", material = "bronze", x = 40, y = 10 })
des.door({ state = "locked", material = "bronze", x = 19, y = 17 })

des.trap("hole", 69, 04)
des.trap("trap door", 33, 05)
des.trap("trap door", 41, 01)

-- Magic chest
des.feature("magic chest", 05, 05)

-- lord of the keep
des.monster({ class = "T", coord = { 06, 05 }, asleep = 1 })

-- monsters in attendance
des.monster(monster[1], 15, 04)
des.monster(monster[1], 13, 04)
des.monster(monster[1], 11, 04)
des.monster(monster[2], 09, 04)
des.monster(monster[2], 07, 04)
des.monster(monster[3], 15, 06)
des.monster(monster[3], 13, 06)
des.monster(monster[4], 11, 06)
des.monster(monster[5], 09, 06)
des.monster(monster[6], 07, 06)

-- monsters behind the bars
des.monster({ id = "iron golem", coord = place[2] })
des.monster({ id = "iron golem", coord = place[3] })
des.monster({ id = "iron golem", coord = place[4] })
des.monster({ class = "D", coord = place[5] })
des.monster({ class = "D", coord = place[6] })
des.monster({ class = "D", coord = place[7] })
des.monster({ class = "D", coord = place[8] })
des.monster({ class = "D", coord = place[9] })
des.monster({ class = "D", coord = place[10] })

-- the prisoners
des.monster({ id = "prisoner", coord = { 02, 15 }, peaceful = 1, asleep = 1 })
des.monster({ id = "prisoner", coord = { 03, 13 }, peaceful = 1 })
des.monster("prisoner", 06, 13)
des.monster("prisoner", 10, 13)

des.monster({ id = "fire giant", coord = { 36, 03 }, peaceful = 1 })
des.monster({ id = "ochre jelly", coord = { 43, 01 }, asleep = 1 })

-- treasure
if percent(50) then
   des.object({ id = "chest", trapped = 0, locked = 1, coord = place[1],
                contents = function()
                   des.object({ id = "wishing", spe = 2, recharged = 1 });
                end
   });
else
   des.object({ id = "crystal chest", trapped = 0, coord = place[1],
                contents = function()
                   des.object({ id = "wishing", spe = 2, recharged = 1 });
                end
   });
end
des.engraving({ coord = place[1], type = "burn", text = "Elbereth" })
des.object({ id = "scroll of scare monster", coord = place[1], buc = "cursed" })

des.object({ id = "chest", coord = place[2] })
des.object({ id = "chest", coord = place[3] })
des.object({ id = "chest", coord = place[4] })
des.object({ id = "chest", coord = place[5] })
des.object({ id = "chest", coord = place[6] })
des.object({ id = "chest", coord = place[7] })
-- Guaranteed to be empty.
des.object({ id = "chest", coord = place[8], contents = function()
end })
des.object({ id = "chest", coord = place[9], contents = function()
end })
des.object({ id = "chest", coord = place[10], contents = function()
end })

-- the soldiers' equipment; also doubles as a reward for player
--
-- NW barracks
des.object("[", 17, 01)
des.object(")", 18, 01)
des.object("[", 19, 01)
des.object("[", 20, 01)
des.object(")", 21, 01)
des.object("[", 22, 01)
des.object(")", 17, 02)
des.object(")", 18, 02)
des.object("[", 19, 02)
des.object(")", 20, 02)
des.object("[", 21, 02)
des.object("[", 22, 02)
des.object("[", 17, 03)
des.object("[", 18, 03)
des.object(")", 19, 03)
des.object("[", 20, 03)
des.object("[", 21, 03)
des.object(")", 22, 03)
-- NE barracks
des.object(")", 28, 01)
des.object("[", 29, 01)
des.object("[", 30, 01)
des.object(")", 31, 01)
des.object("[", 32, 01)
des.object("[", 33, 01)
des.object(")", 28, 02)
des.object(")", 29, 02)
des.object(")", 30, 02)
des.object("[", 31, 02)
des.object("[", 32, 02)
des.object("[", 33, 02)
des.object("[", 28, 03)
des.object(")", 29, 03)
des.object("[", 30, 03)
des.object("[", 31, 03)
des.object(")", 32, 03)
des.object("[", 33, 03)

-- the items in the barracks with iron bars, facing the drawbridge
des.object("arrow", 18, 07)
des.object("arrow", 18, 08)
des.object("arrow", 18, 09)
des.object("arrow", 18, 10)
if percent(20) then
   des.object("orcish arrow", 18, 07)
end
if percent(20) then
   des.object({ id = "elven arrow", coord = { 18, 08 }, buc = "blessed", spe = 2 })
end
if percent(20) then
   des.object({ id = "elven arrow", coord = { 18, 09 }, buc = "blessed", spe = 3 })
end
if percent(20) then
   des.object("orcish arrow", 18, 10)
end
des.object({ id = "orcish bow", coord = { 18, 07 }, buc = "blessed", spe = 1 })
des.object({ id = "elven bow", coord = { 18, 08 }, buc = "blessed", spe = 0 })
des.object("elven bow", 18, 09)
des.object("orcish bow", 18, 10)

-- some boulders on the island
des.object("boulder", 45, 06)
des.object("boulder", 51, 08)
des.object("boulder", 51, 06)

-- polearms for poking the captured monsters
des.object("partisan", 09, 03)
des.object("ranseur", 11, 03)
des.object("spetum", 08, 07)
des.object("bill-guisarme", 12, 07)
