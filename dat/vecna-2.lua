-- NetHack vecnad vecna-2.lua	$NHDT-Date: 1786844627 2026/08/16 01:43:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2021 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	The Stronghold of Kas
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted",
                "nommap")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLL--FF----FFF----FF--LLLLLLLL--F--LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLL-|.................|-LLLLLLL|...|LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLL-|...................|-LLLLLL|...|LLLLLLLLLLLLLLLLLLLL----FF----LLLLL
LLLLLL-|...---.........---...|-LLLLL|...|LLLLLLLLLLLLLLLLLLLL|........|LLLLL
LL----|....- -.........- -....|------...-------LLLLLLLLLLL----...--...----LL
LL|...F....---.........---....S..|............|LLLLLLLLLLL|......--......|-L
LL|-FFF.......................|..|............|LLLL---FF---...............|L
LLF...........................|..|..--------..|LLLL|......................FL
LLF...........................|..|.........|......L|......................FL
LLF...........................|..|.........|..|LLLL|......................FL
LL|-FFF.......................|..--------..|FF|LLLL---FF---...............|L
LL|...F....---.........---....|............|..|LLLLLLLLLLL|......--......|-L
LL----|....- -.........- -....|------...----FF-LLLLLLLLLLL----...--...----LL
LLLLLL-|...---.........---...|-LLLLL|...|LLLLLLLLLLLLLLLLLLLL|........|LLLLL
LLLLLLL-|...................|-LLLLLL|...|LLLLLLLLLLLLLLLLLLLL----FF----LLLLL
LLLLLLLL-|.................|-LLLLLLL|...|LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLL--FF----FFF----FF--LLLLLLLL--F--LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
]] });
des.teleport_region({ region = { 03, 09, 07, 11 } })
des.region({ region = { 00, 00, 75, 20 }, lit = 0, type = "ordinary" })
des.stair("up", 03, 10)
des.stair("down", 72, 10)

-- Drawbridges
des.drawbridge({ x = 50, y = 10, dir = "east", state = "closed" })

-- Secret Doors
des.door({ state = "locked", material = "iron", x = 30, y = 07 })

-- Monsters
des.monster({ id = "salamander", coord = { 48, 09 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 48, 11 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 51, 03 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 51, 17 }, peaceful = 0 })
des.monster({ id = "marilith", coord = { 04, 07 }, asleep = 0,
              inventory = function()
   des.object({ id = "orcish bow", buc = "uncursed" })
   des.object({ id = "orcish arrow", buc = "uncursed", quantity = 50 })
end })
des.monster({ id = "marilith", coord = { 04, 13 }, asleep = 0,
              inventory = function()
   des.object({ id = "orcish bow", buc = "uncursed" })
   des.object({ id = "orcish arrow", buc = "uncursed", quantity = 50 })
end })
des.monster({ id = "marilith", coord = { 44, 13 }, asleep = 0,
              inventory = function()
   des.object({ id = "orcish bow", buc = "uncursed" })
   des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
end })
des.monster({ id = "beholder", coord = { 18, 04 }, asleep = 0 })
des.monster({ id = "beholder", coord = { 18, 16 }, asleep = 0 })
des.monster({ id = "master lich", coord = { 38, 03 }, asleep = 1 })
des.monster({ id = "master lich", coord = { 38, 04 }, asleep = 1 })
des.monster({ id = "master lich", coord = { 38, 16 }, asleep = 1 })
des.monster({ id = "master lich", coord = { 38, 17 }, asleep = 1 })
des.monster({ id = "weredemon", coord = { 54, 10 }, peaceful = 0 })
des.monster({ id = "vampire royal", coord = { 55, 09 }, peaceful = 0 })
des.monster({ id = "vampire mage", coord = { 55, 11 }, peaceful = 0 })
des.monster({ id = "vampire royal", coord = { 63, 07 }, peaceful = 0 })
des.monster({ id = "vampire royal", coord = { 63, 13 }, peaceful = 0 })
des.monster({ id = "balrog", coord = { 66, 10 }, peaceful = 0 })
des.monster({ id = "vampire royal", coord = { 65, 09 }, peaceful = 0 })
des.monster({ id = "vampire royal", coord = { 65, 11 }, peaceful = 0 })
des.monster({ id = "demilich", coord = { 72, 07 }, peaceful = 0 })
des.monster({ id = "demilich", coord = { 72, 13 }, peaceful = 0 })

-- Kas
des.monster({ id = "Kas", coord = { 72, 10 }, asleep = 1 })

-- Kas' loot
des.object("iron safe", 72, 09)
des.object("chest", 72, 11)

-- Random monsters
des.monster("V", 09, 05)
des.monster("V", 10, 08)
des.monster("V", 26, 06)
des.monster("V", 27, 09)
des.monster("V", 15, 11)
des.monster("V", 23, 10)
des.monster("z", 14, 03)
des.monster("Z", 20, 17)
des.monster("Z", 31, 08)
des.monster("z", 44, 10)
des.monster("revenant", 18, 10)
des.monster("revenant", 17, 11)
des.monster("revenant", 13, 12)
des.monster("revenant", 28, 14)
des.monster("M", 17, 07)
des.monster("M", 17, 13)
des.monster("M", 36, 10)
des.monster("M", 34, 08)
des.monster("M", 09, 16)
des.monster("M", 07, 07)
des.monster("M", 12, 12)
des.monster()
des.monster()
des.monster()
des.monster()

-- Random traps
des.trap("ice")
des.trap("ice")
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()

-- None shall pass
des.non_diggable(selection.area(00,00,75,20))
des.non_passwall(selection.area(00,00,75,20))
