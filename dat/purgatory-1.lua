-- NetHack purg purgatory-1.lua	$NHDT-Date: 1786734405 2026/08/14 19:06:45 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2022 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	Purgatory - the player must defeat Lucifer in the
--	Sanctum to gain access to Purgatory, which is the
--	only way out of Gehennom.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify",
                "shortsighted", "nommap")

des.message("You stand atop a craggly mountain.")
des.message("A deep chasm lies between you and an ancient palace.")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
.T..AAAA.tt....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
-------------....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
|.|.........|.TT..AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
|.+.........|..TTT..AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
|--.........|..........AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...AAAAAAAAAAA....
|.|...T.....|--------....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.......AAAAAAAA...T..
|.+..T......|}}}}}}}|.T....AAAAAAAAAA...AAAAAAAAAAAAAA...AAAA..AAAAAA.TT....
|--.T.......|}......----------AAAAAA.....AAAAAAAAAAA...AAAAAAA..AAAA...--...
|.|.........|}...............+.AAAAAA.....AAAAAAAA....AAAAAAAA...AA...--....
|.S.\.......|}...............+..AAAAAA...AAAAAA......AAAAAAAAAA.......|.....
|.|.........|}...............+....AAAAAAAAAAAA..T..AAAAAAAAAAAAA......--....
|--.T.......|}......----------AA...AAAAAAAAAA.....AAAAAAAAAAAAAAAAAA...--...
|.+..T......|}}}}}}}|......AAAAAAA....AAAA....AAAAAAAAAAAAAAAAAAAAAAA......T
|.|...T.....|--------..T.AAAAAAAAAAA...AA...AAAAAAAAAAAAAAAAAAAAAAAAAA..TT..
|--.........|.t...TT...AAAAAAAAAAAAAAA....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAT...
|.+.........|tt.....AAAAAAAAAAAAAAAAAAAA..AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
|.|.........|...T.AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
-------------....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
.AA.T.AA...A...AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
]] });

-- Random monsters
local monster = { 'D','J' }
shuffle(monster);

-- Player monster locations (one will always be blank)
local place = { {16,15},{31,09},{47,10},{73,14} }
shuffle(place);

-- Palace guard locations (one will always be blank)
local palace = { {01,03},{01,06},{01,12},{01,15},
                 {05,05},{05,13},{10,03},{10,15} }
shuffle(palace);

des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
des.region({ region = { 01, 02, 11, 16 }, lit = 1, type = "throne", filled = 2 })
des.teleport_region({ region = { 72, 08, 74, 10 } })
-- Exit portal to the Castle
des.levregion({ region = { 01, 09, 01, 09 }, type = "portal", name = "castle" })
-- Stairs
des.stair("down", 73, 09)

-- Entrances to throne room
des.drawbridge({ x = 13, y = 08, dir = "west", state = "closed" })
des.drawbridge({ x = 13, y = 10, dir = "west", state = "closed" })

-- Various doors
des.door({ state = "closed", material = "gold", x = 02, y = 03 })
des.door({ state = "closed", material = "gold", x = 02, y = 06 })
des.door({ state = "locked", material = "gold", x = 02, y = 09 })
des.door({ state = "closed", material = "gold", x = 02, y = 12 })
des.door({ state = "closed", material = "gold", x = 02, y = 15 })
des.door("locked", 29, 08)
des.door("locked", 29, 09)
des.door("locked", 29, 10)

-- Decoration
des.feature("fountain", 06, 03)
des.feature("fountain", 06, 15)

-- Monsters
des.monster({ id = "Saint Michael", coord = { 04, 09 }, asleep = 1, peaceful = 0 })

des.monster({ id = "Angel", coord = palace[1], asleep = 1, peaceful = 0 })
des.monster({ id = "Angel", coord = palace[2], asleep = 1, peaceful = 0 })
des.monster({ id = "celestial dragon", coord = palace[3], asleep = 1, peaceful = 0 })
des.monster({ class = "A", coord = palace[4], asleep = 1, peaceful = 0 })
des.monster({ class = "D", coord = palace[5], asleep = 1, peaceful = 0 })
des.monster({ class = "H", coord = palace[6], asleep = 1, peaceful = 0 })
des.monster({ class = "J", coord = palace[7], asleep = 1, peaceful = 0 })

-- the 5.0 roster names the cleric adventurer "cleric"; a bare "priest"
-- resolves to the aligned-cleric record instead
des.monster({ id = "valkyrie", coord = place[1] })
des.monster({ id = "cleric", coord = place[2] })
des.monster({ id = "ranger", coord = place[3] })

des.monster({ id = "celestial dragon", coord = { 20, 09 }, peaceful = 0 })
des.monster({ id = "kraken", coord = { 14, 06 }, peaceful = 0 })
des.monster({ id = "kraken", coord = { 18, 06 }, peaceful = 0 })
des.monster({ id = "kraken", coord = { 14, 12 }, peaceful = 0 })
des.monster({ id = "kraken", coord = { 18, 12 }, peaceful = 0 })

-- the centaur keeps its m_initweap kit; the elven bow and arrows are
-- master's additions on top of it
des.monster({ class = "C", coord = { 38, 07 }, peaceful = 0,
              keep_default_invent = true, inventory = function()
  des.object({ id = "elven bow", buc = "uncursed" })
  des.object({ id = "elven arrow", buc = "uncursed", quantity = 100 })
end })

des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("spectre")
des.monster("spectre")
des.monster("spectre")
des.monster("spectre")
des.monster("spectre")
des.monster("e")
des.monster("e")
des.monster("e")
des.monster("e")
des.monster({ id = "storm giant", coord = { 50, 15 }, peaceful = 0 })
des.monster({ id = "storm giant", coord = { 58, 04 }, peaceful = 0 })
des.monster({ class = monster[1] })
des.monster({ class = monster[2] })
des.monster({ class = monster[1] })
des.monster({ class = monster[2] })

-- Loot
des.object("crystal chest", 01, 02)
des.object("crystal chest", 01, 05)
des.object("crystal chest", 01, 13)
des.object("crystal chest", 01, 16)
des.object("opening", 13, 14)
des.object({ id = "egg", coord = { 02, 00 }, montype = "celestial dragon" })
if percent(50) then
  des.object({ id = "egg", coord = { 02, 00 }, montype = "celestial dragon" })
end

-- Walls in the palace are non diggable and non passable
des.non_diggable(selection.area(00,01,12,17))
des.non_diggable(selection.area(12,05,29,13))
des.non_passwall(selection.area(00,01,12,17))
des.non_passwall(selection.area(12,05,29,13))
