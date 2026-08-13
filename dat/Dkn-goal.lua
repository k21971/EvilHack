-- NetHack DKnight Dkn-goal.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2025 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "goal" level for the quest.
--
--	Here you meet the nemesis monster.  You have to defeat the nemesis
--	in combat to gain the artifact you have been assigned to retrieve.
--

des.level_flags("mazelevel")
des.level_init({ style = "mines", fg = ".", bg = "T", smoothed = false,
                 joined = true, lit = 1, walled = false })
--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "center", valign = "center", map = [[
T...TTTTTT.TTTTTT...T
...TT...........TT...
..TT.............TT..
..TT.............TT..
..TT.............TT..
...TT...........TT...
T...TTTTTT.TTTTTT...T
]] })
-- Dungeon Description
des.region({ region = { 00, 00, 20, 06 }, lit = 1, type = "ordinary" })
-- Stairs
des.levregion({ region = { 01, 02, 08, 18 }, region_islev = 1,
                exclude = { 09, 02, 78, 18 }, exclude_islev = 1,
                type = "stair-up" })
-- Non diggable walls
des.non_diggable(selection.area(00,00,20,06))
-- Objects - quest artifact
des.object({ id = "mirror", coord = { 10, 03 }, buc = "blessed",
             spe = 0, name = "The Magic Mirror of Merlin" })
for i = 1, 10 do
  des.object()
end
-- Random traps
des.trap("spiked pit", 13, 05)
des.trap("spiked pit", 12, 04)
des.trap("spiked pit", 12, 06)
for i = 1, 5 do
  des.trap()
end
-- Quest nemesis
des.monster({ id = "Merlin", coord = { 10, 03 }, peaceful = 0 })
-- Random monsters
for i = 1, 6 do
  des.monster({ id = "hawk", peaceful = 0 })
end
des.monster({ id = "large hawk", peaceful = 0 })
des.monster({ id = "large hawk", peaceful = 0 })
des.monster({ id = "giant hawk", peaceful = 0 })
des.monster({ id = "giant hawk", peaceful = 0 })
for i = 1, 6 do
  des.monster({ id = "wolf", peaceful = 0 })
end
des.monster({ id = "wolverine", peaceful = 0 })
des.monster({ id = "wolverine", peaceful = 0 })
des.monster({ class = "u", peaceful = 0 })
des.monster({ class = "q", peaceful = 0 })
des.monster({ class = "u", peaceful = 0 })
des.monster({ class = "q", peaceful = 0 })
