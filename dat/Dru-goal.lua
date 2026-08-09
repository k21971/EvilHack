-- NetHack Druid Dru-goal.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "goal" level for the quest.
--
--	Here you meet Baba Yaga, your nemesis monster.  You have to
--	defeat Baba Yaga in combat to gain the artifact you have
--	been assigned to retrieve.
--

des.level_flags("mazelevel")
des.level_init({ style = "mines", fg = ".", bg = "t", smoothed = false,
                 joined = true, lit = 1, walled = false })

--         1         2
--123456789012345678901
des.map({ halign = "center", valign = "center", map = [[
t...ttttttttttttt...t
...tt...........tt...
..tt.............tt..
.....................
..tt.............tt..
...tt...........tt...
t...ttttttttttttt...t
]] });
-- Dungeon Description
des.region({ region = { 00, 00, 20, 06 }, lit = 1, type = "ordinary" })
-- Stairs
des.levregion({ region = { 72, 02, 78, 18 }, region_islev = 1,
                exclude = { 64, 02, 72, 18 }, exclude_islev = 1,
                type = "stair-up" })
-- Nemesis
des.monster({ id = "Baba Yaga", coord = { 10, 03 }, peaceful = 0 })
des.object({ id = "runed bracers", coord = { 10, 03 }, buc = "blessed",
             spe = 0, name = "Bracers of the First Circle" })
-- Traps
des.trap("spiked pit", 03, 03)
des.trap("spiked pit", 17, 03)
-- Monsters
des.monster({ id = "skeleton warrior", coord = { 04, 02 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 04, 04 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 05, 01 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 05, 05 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 16, 02 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 16, 04 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 15, 01 }, peaceful = 0 })
des.monster({ id = "skeleton warrior", coord = { 15, 05 }, peaceful = 0 })
des.monster({ id = "assassin vine", coord = { 00, 03 }, peaceful = 0 })
des.monster({ id = "assassin vine", coord = { 20, 03 }, peaceful = 0 })
des.monster({ id = "tree blight", coord = { 10, 01 }, peaceful = 0 })
des.monster({ id = "tree blight", coord = { 10, 05 }, peaceful = 0 })
des.monster({ id = "gulthias tree", peaceful = 0 })
des.monster({ id = "human werewolf", peaceful = 0 })
des.monster({ id = "human werewolf", peaceful = 0 })

-- Non diggable walls
des.non_diggable(selection.area(00, 00, 20, 06))
