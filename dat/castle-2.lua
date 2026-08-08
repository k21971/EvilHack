-- NetHack castle castle-2.lua	$NHDT-Date: 1432512783 2015/05/25 00:13:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
-- The thick walls on the corners of the armouries and the dragon pens
-- are a kludge to prevent the walls showing up through the iron bars from
-- outside the castle.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport")

--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
T...}}}}}}}}}..}}}}}}}}}}}}}}}}}}}}}..}}...}}}}}}}}}}}}}....T.....}}}}...T..
...}}}}}}}}}}}}}}}}}}..}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}.......}}}}}}}.....
.}}..}}}----------------------------------------------}}}}}}}..---------S---
}}}}....F...-|.........|........|......S......+......|}}}}}}}}}| - - - - - -
}}}}.}}}|....|.........+........|......--------......|}.}}}}}}}|------------
}}}..}}}|....+.........|........+......|......|-.....|}..}}}}}.S - - - - - -
}}}.T.}}|....|----------........|......|......--FFFF--}}}}}}}}.|------------
}}}...}}|----|.......|..........|......|......|}}}}}}}}}}}.....| - - - - - -
}}}}}}}}|....|.LLLLL.|..........--FFFF-|......|}.....}}}....T..|------------
.}}}}}..|....S.LLLLL.S.\........S......+..{...|}...............| - - - - - -
T...}.}.|....|.LLLLL.|..........--FFFF-|......|}....}}}...}....|------------
..}}.}}}|----|.......|..........|......|......|}}}}}}}}}.}}}...| - - - - - -
}}}}}.}}|....|----------........|......|......--FFFF--}}}}}....|------------
}}}}}.}}|....+.........|........+......|......|-.....|}}}}}}}..| - - - - - -
}}}}....|....|.........+........|......--------......|}}}}}}}}.|------------
}}}.....F...-|.........|........|......S......+......|}}....}}}| - - - - - -
}}..}...----------------------------------------------}..}...}}|------------
}}}}}......}}}}...}}}}}}}..}}......}}}}}}}}}}}}}}}}}}}}.....}}}S - - - - - -
}}}}}}...}}}}}}}}...}}.}..}}}}.T.}}}}}}}}}...}}}}}.}}}}}}}...}}|------------
}}}}}}}}}}}}..}}}}}}}}}}.}}}}}}}..}}.}}........}}.}}...}}}}}}}.| - - - - - -
}}}}}}}}}.....}}}}}}}}}}}}}}}}}}}}}}}}.....T..}}}}}}}}.}}}}}}..-------------
]]);

-- Random registers initialization
local object = { "[", ")", "*", "%" }
shuffle(object);

local place = { {09,08},{09,09},{09,10},{52,03},{52,15} }
shuffle(place);

local monster = { "L", "N", "E", "V", "M", "O", "R", "T", "X", "Z" }
shuffle(monster);

des.teleport_region({ region = { 64, 03, 75, 20 }, exclude = { 07, 01, 54, 17 }, dir = "down" })
des.teleport_region({ region = { 01, 01, 07, 18 }, exclude = { 07, 01, 75, 20 }, dir = "up" })
des.levregion({ region = { 66, 05, 75, 20 }, exclude = { 07, 01, 66, 20 }, type = "stair-up" })

-- the inner courtyard
des.feature("fountain", 42, 09)
des.drawbridge({ x = 47, y = 09, dir = "west", state = "closed" })
des.door({ state = "locked", material = "iron", x = 39, y = 09 })

-- doors leading to the dragon pens
des.door({ state = "locked", material = "iron", x = 39, y = 03 })
des.door({ state = "locked", material = "steel", x = 46, y = 03 })
des.door({ state = "locked", material = "iron", x = 39, y = 15 })
des.door({ state = "locked", material = "steel", x = 46, y = 15 })

-- doors to the throne room
des.door({ state = "locked", material = "steel", x = 13, y = 09 })
des.door({ state = "locked", material = "iron", x = 21, y = 09 })
des.door({ state = "locked", material = "iron", x = 32, y = 05 })
des.door({ state = "locked", material = "iron", x = 32, y = 09 })
des.door({ state = "locked", material = "iron", x = 32, y = 13 })

-- doors to the barracks
des.door({ state = "closed", material = "iron", x = 23, y = 04 })
des.door({ state = "closed", material = "iron", x = 23, y = 14 })

-- doors to the armouries
des.door({ state = "locked", material = "iron", x = 13, y = 05 })
des.door({ state = "locked", material = "iron", x = 13, y = 13 })

-- Removed so that the castle can be breached with either levitation OR an
-- instrument.
--des.trap("trap door", 38, 09)
--des.trap("trap door", 36, 09)
--des.trap("trap door", 34, 09)

des.trap("trap door", 41, 03)
des.trap("trap door", 44, 03)

des.trap("trap door", 41, 15)
des.trap("trap door", 44, 15)

des.mazewalk(63, 09, "east")

-- only the inner walls of the maze are diggable
des.non_diggable(selection.area(00,00,63,20))
des.non_diggable(selection.area(63,00,75,02))

-- the maze is unlit
des.region({ region = { 00, 00, 62, 20 }, lit = 1, type = "ordinary" })
des.region({ region = { 63, 00, 75, 01 }, lit = 1, type = "ordinary" })

-- and the dragon pens are unlit
des.region({ region = { 47, 03, 52, 06 }, lit = 0, type = "ordinary" })
des.region({ region = { 47, 12, 52, 15 }, lit = 0, type = "ordinary" })

des.region({ region = { 24, 03, 31, 15 }, lit = 1, type = "throne", filled = 2, irregular = 1 })

des.region({ region = { 14, 03, 22, 05 }, lit = 1, type = "barracks", filled = 1 })
des.region({ region = { 14, 13, 22, 15 }, lit = 1, type = "barracks", filled = 1 })

-- the armouries and the treasure room are unlit
des.region({ region = { 09, 03, 12, 15 }, lit = 0, type = "ordinary" })

-- northern armoury
des.object(object[1], 09, 03)
des.object(object[1], 10, 03)
des.object(object[1], 11, 03)
des.object(object[1], 09, 04)
des.object(object[1], 10, 04)
des.object(object[1], 11, 04)
des.object(object[1], 12, 04)
des.object(object[1], 09, 05)
des.object(object[1], 10, 05)
des.object(object[1], 11, 05)
des.object(object[1], 12, 05)
des.object(object[1], 09, 06)
des.object(object[1], 10, 06)
des.object(object[1], 11, 06)
des.object(object[1], 12, 06)
-- southern armoury
des.object(object[2], 09, 12)
des.object(object[2], 10, 12)
des.object(object[2], 11, 12)
des.object(object[2], 12, 12)
des.object(object[2], 09, 13)
des.object(object[2], 10, 13)
des.object(object[2], 11, 13)
des.object(object[2], 12, 13)
des.object(object[2], 09, 14)
des.object(object[2], 10, 14)
des.object(object[2], 11, 14)
des.object(object[2], 12, 14)
des.object(object[2], 09, 15)
des.object(object[2], 10, 15)
des.object(object[2], 11, 15)

-- treasure room
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
-- Prevent monsters from eating it. (@'s never eat objects)
des.engraving({ coord = place[1], type = "burn", text = "Elbereth" })
des.object({ id = "scroll of scare monster", coord = place[1], buc = "cursed" })

des.object({ id = "chest", coord = place[2], contents = function()
   des.object("?")
   des.object("?")
   des.object("+")
   des.object("*")
   des.object("*")
end })

des.object({ id = "chest", coord = place[3], contents = function()
   des.object("/")
   des.object("\"")
   des.object("*")
   des.object("*")
end })

des.object({ id = "chest", coord = place[4], contents = function()
   des.object("?")
   des.object("?")
   des.object("+")
   des.object("*")
   des.object("*")
   des.object("!")
end })

des.object({ id = "chest", coord = place[5] })

-- Magic chest
des.feature("magic chest", 24, 09)

-- statues behind the throne
des.object({ id = "statue", x = 22, y = 07 })
des.object({ id = "statue", x = 22, y = 11 })

-- the king (and his adviser[s])

if percent(25) then
   des.monster({ id = "titan", coord = { 23, 09 }, peaceful = 0, asleep = 1 })
end
if percent(50) then
   des.monster({ class = "V", coord = { 23, 08 }, asleep = 1 })
end
des.monster({ class = "V", coord = { 23, 10 }, asleep = 1 })

-- monsters in the water around the castle
des.monster("shark")
des.monster("shark")
des.monster("shark")
des.monster("jellyfish")
des.monster("jellyfish")
des.monster("jellyfish")
des.monster("giant eel")
des.monster("giant eel")
des.monster("giant eel")
des.monster("electric eel")
des.monster("electric eel")
des.monster("electric eel")

-- monsters in attendance in the throne room
des.monster(monster[1], 25, 04)
des.monster(monster[2], 27, 04)
des.monster(monster[3], 29, 04)
des.monster(monster[4], 26, 05)
des.monster(monster[5], 28, 05)
des.monster(monster[6], 30, 05)
des.monster(monster[7], 25, 06)
des.monster(monster[8], 27, 06)
des.monster(monster[9], 29, 06)
des.monster(monster[1], 25, 14)
des.monster(monster[2], 27, 14)
des.monster(monster[3], 29, 14)
des.monster(monster[4], 26, 13)
des.monster(monster[5], 28, 13)
des.monster(monster[6], 30, 13)
des.monster(monster[7], 25, 12)
des.monster(monster[8], 27, 12)
des.monster(monster[9], 29, 12)

-- welcoming committee
des.monster("D", 48, 05)
des.monster("D", 50, 04)
des.monster("D", 51, 13)
des.monster("D", 52, 14)

-- guardians in the antechamber of the treasure room
des.monster("fire elemental", 14, 09)
des.monster("fire elemental", 17, 09)
des.monster("fire vortex", 20, 09)
des.monster("fire vortex", 18, 09)

-- sharpshooters in the chambers next to the hallway
des.monster("C", 35, 06)
des.monster("C", 36, 04)
des.monster("C", 35, 11)
des.monster("C", 36, 12)
des.monster("C", 37, 13)
