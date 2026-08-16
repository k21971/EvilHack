-- NetHack vecnad vecna-3.lua	$NHDT-Date: 1786844627 2026/08/16 01:43:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2021 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	Vecna's domain
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted",
                "nommap")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
tt....LLLLL.tttttLLLLLLLLLLLLLLLLLLLLLL......t..LLLLLLLLLLLL..tttt......LLLL
...LLLLLL--S----------------LLLLLLLLLL.ttt.....LLLLLLLLttt.........tt.....LL
.LLLLL----.................----LLLLLL...tt....LLLL................ttt.....LL
LLL----.....------------......----LLL.ttt....LLLL...................tt.....L
LL--........|.....---------......--LL......LLLLL......L.....................
L||....------.....|.......------..||L....LLLLLL......LLLttt................t
L|---..|..S.......S.......|....|...|LL.LLLLLLL........Lttt.................L
L|..|..--.|.......|.......|---.|...|LLLLLLLLLL......tttt............tt.....L
L|..|...--|.......|.......S....|...|LLLLLLLLL....................tttttt....L
L|..--....--S-----|.......S....|...|LLLLLLL......................ttt........
L|...--...........|.......------...|LLLLLL..................................
L||...----........|.......|.......||LLL..................-----........LLL...
LL--.....------------------......--LLL...tttt...........||...|......LLL.....
LLL----.......................----LLL...ttt.............|....S.............L
.LLLLL----.................----LLLLLLL....tttt.....tttt.||...|.............t
...LLLLLL-------------------LLLLLLLLLL..............ttt..-----.............t
......LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL......LLLLLLLL.....ttt..........tt....
L.......t.....LLLtttLL.........LLLLLLLL...LLLLLLLLLLLLLL........tt....t.....
L......ttt.................LL....LLLLLLLLLLLL....LLLLLLLLLL......tt.L......t
.ttt...........tt.........LL........LLLLLLL.............LLLLLL......LL.....t
..tttttt......ttt.....LLLLt..........ttt....................LLLLL...LLLLLLtt
]] });
des.teleport_region({ region = { 01, 01, 75, 20 },
                      exclude = { 01, 01, 70, 19 } })
des.region({ region = { 00, 00, 75, 20 }, lit = 0, type = "ordinary" })
des.region({ region = { 58, 12, 61, 14 }, lit = 0, type = "morgue",
             filled = 1 })
des.stair("up", 73, 18)

-- Doors and drawbridges
des.drawbridge({ x = 00, y = 08, dir = "east", state = "closed" })
des.door({ state = "locked", material = "iron", x = 61, y = 13 })
des.door({ state = "locked", material = "iron", x = 10, y = 06 })
des.door({ state = "locked", material = "iron", x = 12, y = 09 })
des.door({ state = "locked", material = "iron", x = 18, y = 06 })
des.door({ state = "locked", material = "iron", x = 26, y = 08 })
des.door({ state = "locked", material = "iron", x = 26, y = 09 })

-- chance of a secret way into the lair
if percent(50) then
   des.door({ state = "locked", material = "iron", x = 11, y = 01 })
else
   des.terrain(11, 01, "-")
end

-- chance for a forge
if percent(25) then
   des.feature("forge", 66, 06)
end

-- Various monsters
des.monster({ id = "salamander", coord = { 03, 01 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 05, 15 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 44, 05 }, peaceful = 0 })
des.monster({ id = "salamander", coord = { 46, 17 }, peaceful = 0 })
des.monster({ id = "shambling horror", coord = { 03, 08 }, peaceful = 0 })
des.monster({ id = "alhoon", coord = { 57, 13 }, asleep = 1 })
des.monster({ id = "demilich", coord = { 08, 06 }, asleep = 1 })
des.monster({ id = "elder minotaur", coord = { 11, 02 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 17, 13 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 19, 14 }, peaceful = 0 })

-- Liches
des.monster({ id = "master lich", coord = { 39, 02 }, asleep = 1 })
des.monster({ id = "master lich", coord = { 14, 19 }, asleep = 1 })
des.monster({ id = "arch-lich", coord = { 22, 08 }, asleep = 1 })
des.monster({ id = "lich", coord = { 21, 07 }, asleep = 1 })
des.monster({ id = "lich", coord = { 21, 09 }, asleep = 1 })
des.monster({ id = "lich", coord = { 23, 07 }, asleep = 1 })
des.monster({ id = "lich", coord = { 23, 09 }, asleep = 1 })

-- Vecna
des.monster("Vecna", 28, 06)
des.terrain(28, 06, "\\")

-- Random monsters
des.monster("z")
des.monster("z")
des.monster("Z")
des.monster("Z")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("V")
des.monster("V")
des.monster("&")
des.monster("&")
des.monster()
des.monster()
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("revenant")
des.monster("beholder")
des.monster("lich")
des.monster("lich")
des.monster("lich")
des.monster("lich")
des.monster("demilich")
des.monster("demilich")
if percent(35) then
   des.monster("alhoon")
end

-- Vecna's loot
des.object({ id = "crystal chest", coord = { 27, 06 }, contents = function()
   if percent(20) then
      des.object({ id = "magic marker", buc = "uncursed" })
   end
   des.object({ class = "$", quantity = 20000 })
   des.object({ class = "*", buc = "uncursed" })
   des.object({ class = "*", buc = "uncursed" })
   des.object({ class = "*", buc = "uncursed" })
   des.object({ class = "*", buc = "uncursed" })
   des.object({ class = "?", buc = "uncursed" })
   des.object({ class = "?", buc = "uncursed" })
   des.object({ class = "?", buc = "blessed" })
   des.object({ class = "=", buc = "cursed" })
   des.object({ class = "=", buc = "blessed" })
   des.object({ class = "=", buc = "uncursed" })
   des.object({ class = "\"", buc = "uncursed" })
   des.object({ class = "\"", buc = "blessed" })
   des.object({ class = "!", buc = "cursed" })
   des.object({ class = "!", buc = "blessed" })
   des.object({ class = "+", buc = "uncursed" })
   des.object({ class = "+", buc = "uncursed" })
   des.object({ class = "+", buc = "uncursed" })
   des.object({ class = "+", buc = "cursed" })
   des.object({ class = "+", buc = "blessed" })
end })

-- More loot
des.object("iron safe", 08, 06)
des.object("chest", 09, 06)
des.object("chest", 09, 07)

-- Bodies of past adventurers
des.object({ id = "corpse", montype = "archeologist" })
des.object({ id = "corpse", montype = "barbarian" })
des.object({ id = "corpse", montype = "caveman" })
des.object({ id = "corpse", montype = "convict" })
des.object({ id = "corpse", montype = "healer" })
des.object({ id = "corpse", montype = "infidel" })
des.object({ id = "corpse", montype = "knight" })
des.object({ id = "corpse", montype = "ranger" })
des.object({ id = "corpse", montype = "rogue" })
des.object({ id = "corpse", montype = "priestess" })
des.object({ id = "corpse", montype = "samurai" })
des.object({ id = "corpse", montype = "tourist" })
des.object({ id = "corpse", montype = "valkyrie" })
des.object({ id = "corpse", montype = "wizard" })

-- Random objects
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object("axe")

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

-- None shall pass
des.non_diggable(selection.area(01,01,35,15))
des.non_diggable(selection.area(56,11,61,15))
des.non_passwall(selection.area(00,00,75,20))
