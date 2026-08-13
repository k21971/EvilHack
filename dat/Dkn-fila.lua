-- NetHack DKnight Dkn-fila.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2025 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "fill" levels for the quest.
--
--	These levels are used to fill out any levels not occupied by specific
--	levels as defined above. "fila" is the upper filler, between the
--	start and locate levels, and "filb" the lower between the locate
--	and goal levels.
--

des.level_flags("mazelevel")
des.level_init({ style = "mines", fg = ".", bg = "T", smoothed = false,
                 joined = true, lit = 1, walled = false })

des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = ".", toterrain = "g", chance = 100 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "T", toterrain = "t", chance = 3 })
-- Stairs
des.stair("up")
des.stair("down")
-- Objects
for i = 1, 8 do
  des.object()
end
-- Monsters
des.monster({ id = "hawk", peaceful = 0 })
des.monster({ id = "hawk", peaceful = 0 })
des.monster({ id = "hawk", peaceful = 0 })
des.monster({ id = "large hawk", peaceful = 0 })
des.monster({ id = "wolf", peaceful = 0 })
des.monster({ id = "wolf", peaceful = 0 })
des.monster({ class = "u", peaceful = 0 })
des.monster({ class = "q", peaceful = 0 })
-- Traps
for i = 1, 4 do
  des.trap()
end
