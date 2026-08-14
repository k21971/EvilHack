-- NetHack purg purgatory-2.lua	$NHDT-Date: 1786734405 2026/08/14 19:06:45 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2022 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	The entry level for Purgatory - the portal from the
--	Sanctum arrives here, beside the River of Redemption.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify",
                "shortsighted", "nommap")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
TTggPPPPPggTTTTTTTTTTTTTTTTTgggTTTTTTTTTTTTTTTTTgggTTTgTTTTg----------------
TggTgPPPPPgggTTTTTTggggggggggTTTTTTTTTTTgggggggggggggggggggg|gggggggggggggg|
ggggggPPPgggggggggggggggggggggggggggggggggggggggTTTTTggggggT|gggggggggggggg|
TTTggggPPPggggggTTTgggggggggggggggggggggPggggTTTTgggggggggTT|gggggggggggggg|
ggTTggPPPPggggTTTTggggggTTTTgggggggggggPPPgggggggggggggggTTggggggg---------|
gggggggPPPPgggggggggggggggTTggggggggggggPggggggggggggggggTTggggggg|........|
ggTTggggPPPPggggggTTggggggggggggggTTTggggggggTTggggggTggggTggggggg+........|
gTTTggggPPPPPgggggTgggggggggggggggTTTgggggggTTTggggggggggggggggggg|........|
gggggggggPPPPggTggTPPPPPPPPggggggggTTgggggggggTTTggggggTgggggggggg+........|
ggggTggTggPPPgTTgPPPPPPTgPPPPPPggggTgggggggggggggggggggggggggggggg|........|
gggggggggggPPPPPPPPPggggggggPPPPPPPPgggggggTTgggggTTgggggggggggggg---------|
ggTTggggggPPPPPPPggggggggggggggPPPPPPPPPgTTTgggggggTTggggggg|gggggggggggggg|
TggTTTggPPPPPgggggggggTTTTTgggggggPPPPPPPPgggggggggggggTTggg|gggggggggggggg|
TggggggPPPPPgTTgggggTTTgggTTTgggggggTgPPPPPPgggggggggggTTTgg|gggggggggggggg|
gggggggPPPPggTgggggTTgggggggTTgggggTTTgggPPPPPPPPPggggggTggg----------------
ggggggPPPPggggggggTTggggggggggggggggggggggggggPPPPPPPgggggggggtttggggggggtgg
gggggTTPPPPgggggggTTgggggggggggggggggTTTTTggggggggPPPPPggggggggggggggTTTgggg
ggggggTgPPPPggTTgggTTgggggggTTgggggTTTTTggggggggggggPPPPggggggggggggggTggggT
TggggggggPPPPTTgggggTTTgggTTTggggggggTTTTggggTTTggggggPPPPPgggggggggggggggTT
TggTTTTgggPPPgggTgggggTTTTTgggggggggggggggggggggggggggggPPPPPPggggTTTTTTTTTT
]] });

-- Random monsters
local monster = { 'C','D','J','n' }
shuffle(monster);

local river = { {07,01},{11,09},{09,13},{22,08},
                {28,09},{34,11},{42,14},{56,18} }
shuffle(river);

-- Player monster locations (one will always be blank)
local place = { {02,09},{21,02},{15,15},
                {40,07},{55,05},{63,12},{32,17} }
shuffle(place);

des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
des.teleport_region({ region = { 02, 16, 03, 18 } })
-- Portal arrival point from the Sanctum
des.levregion({ region = {01,16,01,16}, type="branch" })
-- Stairs
des.stair("up", 73, 07)
des.engraving({ coord = { 72, 07 }, type = "burn", text = "To the Mountain of Purgatory" })

-- Entrances to main room
des.door("locked", 66, 06)
des.door("locked", 66, 08)

-- Functional decor
des.feature("fountain", 22, 16)
des.feature("fountain", 24, 14)
des.feature("fountain", 26, 16)
des.feature("forge", 72, 02)
des.feature("forge", 72, 12)

-- Monsters
des.monster({ id = "barbarian", coord = place[1] })
des.monster({ id = "knight", coord = place[2] })
des.monster({ id = "wizard", coord = place[3] })
des.monster({ id = "healer", coord = place[4] })
des.monster({ id = "infidel", coord = place[5] })
des.monster({ id = "rogue", coord = place[6] })

des.monster("spectre", 01, 09)
des.monster("spectre", 03, 09)
des.monster("spectre", 02, 08)
des.monster("spectre", 20, 02)
des.monster("spectre", 22, 02)
des.monster("spectre", 21, 03)
des.monster("spectre", 14, 15)
des.monster("spectre", 16, 15)
des.monster("spectre", 16, 16)
des.monster("spectre", 40, 06)
des.monster("spectre", 40, 08)
des.monster("spectre", 39, 07)
des.monster("spectre", 55, 04)
des.monster("spectre", 56, 05)
des.monster("spectre", 56, 06)
des.monster("spectre", 63, 11)
des.monster("spectre", 62, 12)
des.monster("spectre", 64, 12)
des.monster("spectre", 31, 17)
des.monster("spectre", 33, 17)
des.monster("spectre", 33, 18)

des.monster("water nymph", 40, 04)
des.monster("vorpal jabberwock", 69, 07)

des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster("ghost")
des.monster({ id = "kraken", coord = river[1] })
des.monster({ id = "kraken", coord = river[2] })
des.monster({ id = "giant eel", coord = river[3] })
des.monster({ id = "giant eel", coord = river[4] })
des.monster({ id = "electric eel", coord = river[5] })
des.monster({ id = "electric eel", coord = river[6] })
des.monster({ id = "shark", coord = river[7] })
des.monster({ id = "shark", coord = river[8] })
des.monster("e")
des.monster("e")
des.monster("e")
des.monster("e")
des.monster("e")
des.monster("D")
des.monster("D")
des.monster({ class = monster[1] })
des.monster({ class = monster[1] })
des.monster({ class = monster[4] })
des.monster({ class = monster[4] })
des.monster({ class = monster[1] })
des.monster({ class = monster[2] })
des.monster({ class = monster[3] })
des.monster({ class = monster[4] })
des.monster({ class = monster[1] })
des.monster({ class = monster[2] })
des.monster({ class = monster[3] })
des.monster({ class = monster[4] })

-- None shall pass
des.non_diggable(selection.area(60,00,75,14))
des.non_passwall(selection.area(60,00,75,14))
