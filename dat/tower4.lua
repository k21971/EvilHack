-- NetHack tower tower4.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
-- Bottomless cavern below Vlad's tower
-- Several vampires roost here, surrounding the ladder up
-- Gravity is in full effect here
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "nommap",
                "graveyard", "shortsighted", "solidify")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAA...AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAA.....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAA...AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...AAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.....AAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...AAAAAAAA
.AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
..AAAAAAAAAAAAAAAAAAAAAAAAAAAA.....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
...AAAAAAAAAAAAAAAAAAAAAAAAAAAA...AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
.....AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
.......AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
........AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
]]);
des.teleport_region({ region = { 01, 17, 04, 19 }, exclude = { 00, 00, 00, 00 } })
des.levregion({ region = {01,18,01,18}, exclude = {00,00,00,00}, type="branch" })
des.ladder("up", 74, 01)
des.region({ region = { 00, 00, 75, 19 }, lit = 0, type = "morgue", filled = 0 })
des.region({ region = { 01, 18, 01, 18 }, lit = 1, type = "ordinary" })
des.region({ region = { 74, 01, 74, 01 }, lit = 1, type = "ordinary" })
des.engraving({ coord = { 04, 17 }, type = "engrave", text = "Watch your step..." })
-- Vampire nests
des.monster({ id = "vampire sovereign", coord = { 64, 00 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 65, 00 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 63, 01 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 64, 01 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 65, 01 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 66, 01 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 62, 02 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 63, 02 }, asleep = 1 })
des.monster({ id = "vampire noble", coord = { 64, 02 }, asleep = 1 })
des.monster({ id = "vampire noble", coord = { 65, 02 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 66, 02 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 67, 02 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 63, 03 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 64, 03 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 65, 03 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 66, 03 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 64, 04 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 65, 04 }, asleep = 1 })

des.monster({ id = "vampire sovereign", coord = { 67, 03 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 68, 03 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 66, 04 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 67, 04 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 68, 04 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 69, 04 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 65, 05 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 66, 05 }, asleep = 1 })
des.monster({ id = "vampire noble", coord = { 67, 05 }, asleep = 1 })
des.monster({ id = "vampire noble", coord = { 68, 05 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 69, 05 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 70, 05 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 66, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 67, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 68, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 69, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 67, 07 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 68, 07 }, asleep = 1 })

des.monster({ id = "vampire sovereign", coord = { 70, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 71, 06 }, asleep = 1 })
des.monster({ id = "vampire noble", coord = { 72, 06 }, asleep = 1 })
des.monster({ id = "vampire noble", coord = { 73, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 74, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 75, 06 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 69, 07 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 70, 07 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 71, 07 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 72, 07 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 73, 07 }, asleep = 1 })
des.monster({ id = "vampire sovereign", coord = { 74, 07 }, asleep = 1 })

-- Demons
des.monster({ id = "nalfeshnee", coord = { 21, 05 }, peaceful = 0 })
des.monster({ id = "pit fiend", coord = { 32, 15 }, peaceful = 0 })
des.monster({ id = "weredemon", coord = { 67, 11 }, peaceful = 0 })
-- Random vampires
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
-- Random corpses of past adventurers
des.object({ id = "corpse", coord = { 01, 16 }, montype = "archeologist" })
des.object({ id = "corpse", coord = { 05, 18 }, montype = "priestess" })
des.object({ id = "corpse", coord = { 67, 12 }, montype = "monk" })