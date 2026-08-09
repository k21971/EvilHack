-- NetHack Druid Dru-filb.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "fill" levels for the quest.
--
--	These levels are used to fill out any levels not occupied by specific
--	levels as defined above. "filla" is the upper filler, between the
--	start and locate levels, and "fillb" the lower between the locate
--	and goal levels.
--

des.level_flags("mazelevel")
des.level_init({ style = "mines", fg = ".", bg = "t", smoothed = false,
                 joined = true, lit = 1, walled = false })

-- very little trees or grass are still alive
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "t", toterrain = "T", chance = 2 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = ".", toterrain = "g", chance = 10 })
-- Stairs
des.stair("up")
des.stair("down")
-- Objects
des.object()
des.object()
des.object()
-- Traps
des.trap("pit")
des.trap("pit")
des.trap("spiked pit")
des.trap("spiked pit")
des.trap("bear")
des.trap("bear")
-- Monsters
des.monster({ id = "cave bear", peaceful = 0 })
des.monster({ id = "dire wolverine", peaceful = 0 })
for i = 1, 4 do
  des.monster({ id = "human werewolf", peaceful = 0 })
end
for i = 1, 4 do
  des.monster({ id = "skeleton", peaceful = 0 })
end
for i = 1, 4 do
  des.monster({ id = "skeleton warrior", peaceful = 0 })
end
des.monster({ id = "assassin vine", peaceful = 0 })
des.monster({ id = "creeping mound", peaceful = 0 })
des.monster({ id = "twig blight", peaceful = 0 })
des.monster({ id = "twig blight", peaceful = 0 })
des.monster({ id = "needle blight", peaceful = 0 })
des.monster({ id = "needle blight", peaceful = 0 })
des.monster({ id = "tree blight", peaceful = 0 })
des.monster("#")
des.monster("n")
