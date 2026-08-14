-- NetHack iceq icequeen-1.lua	$NHDT-Date: 1786716969 2026/08/14 14:16:09 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2020 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	The start level for this branch - the player must traverse
--	this icy wasteland to find their way to the Ice Queen.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "icedpools",
                "shortsighted", "cold")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
ttttttttttttt.....ttttt......tttttttT...tttttttt......ttttttttttttttttt..tTt
ttI.Ittttt..........tt.........ttttt.....ttt.....tt......ttt..tt...tt.....tt
tIIIIIttt..ttt..T..t.....tt......................IIIII..........tt.........t
ttIIIIttt...tt.......t.t.......ttt..ttt.......IIIIIIIIII..tt....t.....ttt..t
tttIIIttt........tt.............tt.........IIIIIIIIIIIIII..........T.......t
tTtttttt..t........t....t..............ttIIIIIIIIIIIIIIIII.t...tt....t.....t
tttIIttt...t...tt.....t....T...tt.....IIIIIIII.......IIIIIt..............ttt
tttIIttttt...t....tt.......tt.......tIIIIII.....ttt...IIIII..t......ttt....t
IIIIIIIIII............tttt..ttt...IIIIIIIt.....ttTtt...IIII.t..ttt.........t
IIIIIIIIIIIt..tt..IIIIIIIIIIIIIIIIIIIIIIt.......ttt....IIII...........T....t
tttIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII.................III.t...ttttt.....tt
tT..tt..IIIIIIIIIIIIIIII....tt.............tttt....t....IIIIIIIIIIIIII..tttt
t........tt.....tttt.............tt...tt....ttttt........IIIIIIIIIIIIIIIIIII
t.ttt........t.................tttt....t...........tt....t....tt..IIIIIIIIII
t.......tt...........t......................t......................tttt....I
t...ttt..ttt.....tt.....ttttt...........tT...tt.......ttttt.............tT.t
t............t...tt......ttttt................ttt.....ttt.........tt.......t
t........Tt.........................ttt...............................T....t
tt...tttt.......ttt........ttttt..tt.....tttt....ttttt....ttt......ttt....Tt
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
]] });
des.teleport_region({ region = { 70, 16, 74, 18 } })
des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
-- Portal arrival point
des.levregion({ region = {72,18,72,18}, type="branch" })
-- Stairs
des.stair("down", 03, 02)
-- Hint
des.object({ id = "novel", coord = { 69, 16 }, buc = "uncursed", name = "The Adventurer's Journal" })
des.object({ id = "fedora", coord = { 69, 16 }, buc = "uncursed" })
des.object({ id = "corpse", coord = { 69, 16 }, montype = "archeologist", name = "Carl the Excavator" })

-- Objects
des.object("axe")
des.object("battle-axe")
des.object("crystal chest")

-- Random traps
des.trap("bear")
des.trap("bear")
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("spear")
des.trap("spear")

-- Frost salamanders patrol the frozen river
des.monster("frost salamander", 03, 10)
des.monster("frost salamander", 26, 11)
des.monster("frost salamander", 51, 04)
des.monster("frost salamander", 67, 13)

-- Goblin outrider patrol
des.monster({ id = "goblin outrider", coord = { 09, 04 }, peaceful = 0 })
des.monster({ id = "goblin outrider", coord = { 10, 03 }, peaceful = 0 })
des.monster({ id = "goblin outrider", coord = { 10, 04 }, peaceful = 0 })
des.monster({ id = "goblin-captain", coord = { 09, 03 }, peaceful = 0 })

-- Guards the stairs
des.monster("snow golem", 03, 03)

-- Random monsters
des.monster()
des.monster()
des.monster()
des.monster()
des.monster()
des.monster()
des.monster()
des.monster()
des.monster()

-- None shall pass (dead trees are not petrified)
des.non_passwall(selection.area(00,00,08,05))