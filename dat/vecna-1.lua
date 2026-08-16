-- NetHack vecnad vecna-1.lua	$NHDT-Date: 1786844627 2026/08/16 01:43:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2021 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	Vecna's graveyard - the player must discover
--	which one of four mausoleums to enter to progress
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted",
                "nommap", "graveyard")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
t.................ttttt......ttttttt....tttttttt......ttttttttt............t
t.--------..........tt.........ttttt.....ttt.....tt......ttt..t...--------.t
t.|......S.ttt.....t.....tt.....................................t.S......|.t
t.|......|..tt.......t.t.......ttt..ttt...........tt......tt....tt|......|.t
t.|...----.......tt.............tt.................ttt............----...|.t
t.|...|...t........t....t.......tttttt.tttttt..............t...tt....|...|.t
t.|...|....t...tt.....t........tt...........tt............t..........|...|.t
t.-----......t....tt.......tt.tt.............tt.ttt..........t......t-----.t
...t..................tttt..ttt...............t.............t..ttt..........
...........t..tt............t...................ttt.........................
t.............................t...............t.............t...ttttt......t
t.-----.....................tttt.............tt....t............tt...-----.t
t.|...|..tt.....tttt...........tt...........ttttt....................|...|..
t.|...|......t.................ttttttt.tttttt......tt....t....tt.....|...|..
t.|...----...........t......................t.....................----...|..
t.|......|tt.....tt.....ttttt.........ttt....tt.......ttttt.......|......|.t
t.|......S...t...tt......ttttt................ttt.....ttt.........S......|.t
t.--------t.........................ttt...........................--------.t
t...............ttt........ttttt..tt.....tttt....ttttt....ttt..............t
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
]] });
local stairs = { {04,06},{04,14},{71,06},{71,14} }
local liches = { {03,07},{03,13},{72,07},{72,13} }
shuffle(stairs);
shuffle(liches);

des.teleport_region({ region = { 33, 07, 43, 13 } })
des.region({ region = { 00, 00, 75, 20 }, lit = 0, type = "ordinary" })
des.region({ region = { 13, 03, 27, 17 }, lit = 0, type = "morgue",
             filled = 1 })
des.region({ region = { 49, 03, 62, 17 }, lit = 0, type = "morgue",
             filled = 1 })
des.region({ region = { 03, 03, 05, 05 }, lit = 0, type = "morgue",
             filled = 1 })
des.region({ region = { 03, 15, 05, 17 }, lit = 0, type = "morgue",
             filled = 1 })
des.region({ region = { 70, 03, 72, 05 }, lit = 0, type = "morgue",
             filled = 1 })
des.region({ region = { 70, 15, 72, 17 }, lit = 0, type = "morgue",
             filled = 1 })

-- Portal arrival point
des.levregion({ region = {38,10,38,10}, type="branch" })

-- Stairs (random)
des.stair({ dir = "down", coord = stairs[1] })

-- 'Fake' stairs
des.monster({ id = "giant mimic", coord = stairs[2],
              appear_as = "ter:staircase down" })
des.monster({ id = "giant mimic", coord = stairs[3],
              appear_as = "ter:staircase down" })
des.monster({ id = "giant mimic", coord = stairs[4],
              appear_as = "ter:staircase down" })

-- Liches
des.monster({ id = "lich", coord = liches[1], asleep = 1 })
des.monster({ id = "lich", coord = liches[2], asleep = 1 })
des.monster({ id = "demilich", coord = liches[3], asleep = 1 })
des.monster({ id = "demilich", coord = liches[4], asleep = 1 })

-- Secret Doors
des.door({ state = "locked", material = "iron", x = 09, y = 03 })
des.door({ state = "locked", material = "iron", x = 09, y = 17 })
des.door({ state = "locked", material = "iron", x = 66, y = 03 })
des.door({ state = "locked", material = "iron", x = 66, y = 17 })

-- Random monsters
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("z")
des.monster("z")
des.monster("z")
des.monster("Z")
des.monster("Z")
des.monster("Z")
des.monster("Z")
des.monster("Z")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("M")
des.monster("M")
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

-- None shall pass
des.non_diggable(selection.area(02,02,09,08))
des.non_diggable(selection.area(66,02,73,08))
des.non_diggable(selection.area(02,12,09,18))
des.non_diggable(selection.area(66,12,73,18))
des.non_passwall(selection.area(00,00,75,20))
