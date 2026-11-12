-- $NHDT-Header: NetHack 5.0 wizard1.lua 2026/10/27_14:28:17 NetHack-5.0:1.8 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--
-- The top (real) wizard level.
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted",
                "nommap")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAA-------AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAA|.....|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAA---LLL---AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAA|.......|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAA|.......|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAA--F-----------F--AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAA|...............|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA..tAAAAAAAAAAAAAAAAAAAA
AAAAF...............|-FF--AAAAAAAAAAAAAAAAAAAAAAAAA.......AAAAAAAAAAAAAAAAAA
AAAA|...|.......|...|....|L..........AAAAA..........|.....tAAAAAAAAAAAAAAAAA
AAAA|...|.......|...|....|L..AAAA...........AA..AA..|......AAAAAAAAAAAAAAAAA
AAAAF...............|-FF--AAAAAAAAAAAAAAAAAAAAAAAAA..t....AAAAAAAAAAAAAAAAAA
AAAA|...............|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...AAAAAAAAAAAAAAAAAAAA
AAAA--F-----------F--AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAA|.......|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAA|.......|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAA---LLL---AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAA|.....|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAA-------AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
]]);
des.teleport_region({ region = { 5, 7, 7, 12 }, exclude = { 0, 0, 0, 0 } })
-- entire tower in a region, constrains monster migration
des.region({ region = { 0, 0, 79, 19 }, lit = 1, type = "ordinary",
             arrival_room = true })

-- random wandering monster locations
local place = { {3,2},{3,17},{24,4},{24,15},
                {40,6},{40,13},{61,2},{61,17} }
shuffle(place);

-- stairs down
des.ladder("down", 5, 9)

-- drawbridges
des.drawbridge({ x = 26, y = 9, dir = "west", state = "closed" })
des.drawbridge({ x = 26, y = 10, dir = "west", state = "closed" })

-- Magic chest
des.feature("magic chest", 56, 10)

-- doors
des.door({ state = "locked", material = "iron", x = 12, y = 6 })
des.door({ state = "locked", material = "iron", x = 12, y = 13 })
des.door({ state = "locked", material = "iron", x = 20, y = 9 })
des.door({ state = "locked", material = "iron", x = 20, y = 10 })

-- The wizard and his pet
des.monster({ id = "Wizard of Yendor", coord = { 55, 10 }, asleep = 1 })
des.monster({ id = "hell hound", coord = { 55, 9 }, asleep = 1 })
-- The local treasure
des.object("Book of the Dead", 55, 10)
des.object({ id = "crystal chest", coord = { 55, 9 }, contents = function()
   des.object({ id = "wishing", spe = 1, recharged = 1 })
   des.object("full healing")
   des.object("gain level")
   des.object("?")
   des.object("+")
   des.object("\"")
   if percent(25) then
      des.object("magic marker")
   end
end })

-- defenders inside the tower
des.monster({ id = "infidel", coord = { 12, 8 }, peaceful = 0 })
des.monster({ id = "weredemon", coord = { 12, 11 }, peaceful = 0 })
des.monster({ id = "drow cleric", coord = { 9, 4 }, peaceful = 0 })
des.monster({ id = "drow cleric", coord = { 15, 4 }, peaceful = 0 })
des.monster({ id = "elven wizard", coord = { 15, 15 }, peaceful = 0 })
des.monster({ id = "elven wizard", coord = { 9, 15 }, peaceful = 0 })
-- bridge
des.monster({ class = "H", coord = { 39, 10 }, asleep = 1, peaceful = 0 })
-- flying monsters outside
des.monster({ class = "D", coord = place[1], peaceful = 0 })
des.monster({ class = "D", coord = place[2], peaceful = 0 })
des.monster({ class = "D", coord = place[3], peaceful = 0 })
des.monster({ class = "D", coord = place[4], peaceful = 0 })
des.monster({ class = "V", coord = place[5], peaceful = 0 })
des.monster({ class = "V", coord = place[6], peaceful = 0 })
des.monster({ class = "e", coord = place[7], peaceful = 0 })
des.monster({ class = "e", coord = place[8], peaceful = 0 })

-- loot on the north and south tower caps
for _, y in ipairs({ 2, 17 }) do
   des.object("+", 10, y)
   des.object("?", 11, y)
   des.object("?", 12, y)
   des.object("?", 13, y)
   des.object("+", 14, y)
end

-- none shall pass
des.non_diggable(selection.area(4, 1, 25, 18))
des.non_passwall(selection.area(4, 1, 25, 18))
