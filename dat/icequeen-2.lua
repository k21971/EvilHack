-- NetHack iceq icequeen-2.lua	$NHDT-Date: 1786716969 2026/08/14 14:16:09 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2020 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	The middle level for this branch.
--
--	The player exits the dead forest and
--	finds themselves overlooking a vast icy
--	landscape. A gorge runs through the middle
--	of the map, with only one way across.
--	The way across is defended...
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "icedpools",
                "shortsighted", "cold")
des.message("As you depart the lifeless, frozen wood, the west reveals a barren landscape of snow and ice.")
des.message("A bottomless chasm separates the icy plateau from the next.")
des.message("The only apparent way across is a solitary, slender ice bridge.")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
IIIIIIIIIIIIIIIIIIItAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIII..ttttttttttttt
IItIIIIIIIIIItIIIIIttAAAAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIIIIIII....tttttt...tt
IIIIIIIttIIIIIIIIIIIttAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIIIIIIIIII......tttttt.tt
IIIIIIIIIIIIIIIIIIIIItttAAAAAAAAAAAAAAAAAAtIIIIIIIIIIIIIIIIIII........ttt..t
IIIIttIIIIIIIIIIIIIIIIIttttAAAAAAAAAAAAAIIIIIIIIIIIIIIIIIIIIII.t.......tt..t
IIIIIIIIIIIIIIIIIIIItIIIIItttAAAAAAAAAAIItIIIIIIIIIIIIIIIIIIII...tt....tt..t
IIIIIIIIIIIIIIIIIIIIIIIIIIIItttAAAAAAtIIIttIIIIIIIIIIIIIIIIIII.........tt..t
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIttttAAttIIIttIIIIIIIIIIIIIIIII.........tttt.tt
IIIIIIIItttIIIIIIIIIIIIIIIIIIIIIIIIIIIIIItIIIIIIIIIIIIIIIIII........tttt...t
IItIIIIIItIIIIIIIIIIIIIIIIIIIItttttAAttIItIIIIIIIIIIIIIIIIIII........ttt...t
IIIIIIIIIIIIIIIIIIIIIIIIIIIIItttAAAAAAAtIIIIIIIIIIIIIIIIIIIIII........tt...t
IIIIItIIIIIIIIIIIIIIIIIIIIIIItAAAAAAAAAAAIIIIIIIIIIIIIIIIIIIII........ttt..t
IIIIIIIIIIIIIIIIIIIIIIIIIIIIItAAAAAAAAAAAAIIIIIIIIIIIIIIIIIIII........ttt..t
IIIIIIIIIIIIIIIIIIIIIIIIIIItttAAAAAAAAAAAAtIIIIIIIIIIIIIIIIII........ttt...t
IIIIIItIIIIIttIIIIIIIIIIIIttAAAAAAAAAAAAAAAAIIIIIIIIIIIIIIII....t....ttttt.t
IItIIIttIIIIIIIIIIIIIIIIIItAAAAAAAAAAAAAAAAAAIIIIIIIIIIIIIII...tt.....ttt..t
IItttItIItIIIIIIIIIIIIIttttAAAAAAAAAAAAAAAAAAAAIIIIIIIIIIIIII.............tt
IIIIIIIIItIIIIItIIIIItttAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIIIIIII..........tt..t
II.IIItIIIIIIIIIIIIIttAAAAAAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIII..t.......ttt..t
IIIIItttIIIIIIIIIIIttAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIII......tttttt..t
tIIIIIIIIIIIIIIIIIItAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIII...tttttttttttt
]] });
-- random monster spawn spots
local spawn = { {02,07},{07,08},{21,12},{22,05},
                {46,03},{47,12},{64,06},{65,16} }
shuffle(spawn);
des.teleport_region({ region = { 72, 08, 74, 10 } })
des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("up", 71, 01)
des.stair("down", 02, 18)

-- Traps and objects
des.trap("spear", 32, 08)
des.trap("spiked pit", 33, 08)
des.trap("bear", 73, 07)
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.object({ id = "boulder", quantity = 1, coord = { 31, 08 } })

-- Another dead adventurer
des.object({ id = "cornuthaum", coord = { 73, 19 }, buc = "uncursed" })
des.object({ class = "+", coord = { 73, 19 }, buc = "uncursed" })
des.object({ id = "corpse", coord = { 73, 19 }, montype = "wizard", name = "Tim the Enchanter" })

-- Monsters
des.monster({ id = "Abominable Snowman", coord = { 34, 08 }, inventory = function()
  des.object({ id = "boulder", quantity = 12 })
end })

des.monster("revenant", 73, 05)
des.monster("revenant", 73, 06)
des.monster("revenant", 74, 06)
des.monster("frost salamander", 10, 03)
des.monster("frost salamander", 10, 15)
des.monster("frost salamander", 50, 03)
des.monster("frost salamander", 50, 15)
des.monster("mastodon", 02, 17)
des.monster("frost giant", 03, 17)
des.monster("saber-toothed tiger", 74, 19)

-- Random monsters
des.monster({ coord = spawn[1] })
des.monster({ coord = spawn[2] })
des.monster({ coord = spawn[3] })
des.monster({ coord = spawn[4] })
des.monster({ coord = spawn[5] })
des.monster({ coord = spawn[6] })
des.monster({ coord = spawn[7] })
des.monster({ coord = spawn[8] })
des.monster()
des.monster()
des.monster()
des.monster()

-- None shall pass
des.non_diggable(selection.area(18,00,34,20))
des.non_diggable(selection.area(70,00,75,07))