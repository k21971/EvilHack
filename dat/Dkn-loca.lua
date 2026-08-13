-- NetHack DKnight Dkn-loca.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2025 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "locate" level for the quest.
--
--	Here you have to find ancient ruins to make your way to
--	the Caledonia Forest.
--

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "mines", fg = ".", bg = ".", smoothed = true,
                 joined = true, lit = 1, walled = false })

des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = ".", toterrain = "g", chance = 100 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "g", toterrain = "T", chance = 20 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "T", toterrain = "t", chance = 3 })
--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "center", valign = "center", map = [[
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggg--....--.gggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggg---...tt.....gggggggggggggggggggggggggggggggggggggggggggggggggggggg
g...t..--.t....-.-gggggggggggggggggggggggggggggggggggggggggggggggggggg
g|.....-...-.....|gggggggggggggggggggggggggggggggggggggggggggggggggggg
g|.t........|.t..|gggggggggggggggggggggggggggggggggggggggggggggggggggg
g|....--....|.t...gggggggggggggggggggggggggggggggggggggggggggggggggggg
g..-...-........--gggggggggggggggggggggggggggggggggggggggggggggggggggg
ggg---...tt..---gggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggt.--..---gggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
]] })
-- Dungeon Description
des.region({ region = { 00, 00, 69, 20 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("down", 09, 13)
des.levregion({ region = { 69, 02, 78, 18 }, region_islev = 1,
                exclude = { 64, 02, 69, 18 }, exclude_islev = 1,
                type = "stair-up" })
-- Branch info
des.levregion({ region = { 69, 02, 78, 18 }, region_islev = 1,
                exclude = { 64, 02, 69, 18 }, exclude_islev = 1,
                type = "branch" })
des.teleport_region({ region = { 69, 02, 78, 18 }, region_islev = 1,
                      exclude = { 64, 02, 69, 18 }, exclude_islev = 1 })
-- populate trees in REGION area
des.replace_terrain({ region = { 19, 00, 69, 20 }, fromterrain = "g", toterrain = "T", chance = 20 })
des.replace_terrain({ region = { 19, 00, 69, 20 }, fromterrain = "T", toterrain = "t", chance = 3 })
des.replace_terrain({ region = { 00, 00, 19, 08 }, fromterrain = "g", toterrain = "T", chance = 20 })
des.replace_terrain({ region = { 00, 00, 19, 08 }, fromterrain = "T", toterrain = "t", chance = 3 })
des.replace_terrain({ region = { 00, 18, 19, 20 }, fromterrain = "g", toterrain = "T", chance = 20 })
des.replace_terrain({ region = { 00, 18, 19, 20 }, fromterrain = "T", toterrain = "t", chance = 3 })
-- Temple region
des.region({ region = { 02, 12, 11, 14 }, lit = 1, type = "temple",
             filled = 1 })
-- Altar
des.altar({ coord = { 05, 13 }, align = "neutral", type = "shrine" })
-- Objects
for i = 1, 9 do
  des.object()
end
-- Random traps
for i = 1, 6 do
  des.trap()
end
-- Random monsters
for i = 1, 7 do
  des.monster({ id = "hawk", peaceful = 0 })
end
des.monster({ id = "large hawk", peaceful = 0 })
des.monster({ id = "large hawk", peaceful = 0 })
for i = 1, 8 do
  des.monster({ id = "wolf", peaceful = 0 })
end
des.monster({ class = "u", peaceful = 0 })
des.monster({ class = "q", peaceful = 0 })

-- Non diggable walls
des.non_diggable(selection.area(02,09,17,17))
