-- NetHack Druid Dru-loca.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "locate" level for the quest.
--
--	Here you have to find Baba Yaga's Hut
--	to go further towards your assigned quest.
--

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "mines", fg = ".", bg = ".", smoothed = true,
                 joined = true, lit = 1, walled = false })

des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = ".", toterrain = "g", chance = 100 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "g", toterrain = "t", chance = 25 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "t", toterrain = "T", chance = 10 })
--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggg----S----gggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggg---.......---gggggggggggggggggggggggggggggggggggggggggggggggggggggg
g---...-----...---gggggggggggggggggggggggggggggggggggggggggggggggggggg
g|....|-...-|....|gggggggggggggggggggggggggggggggggggggggggggggggggggg
g|....S.....S....|gggggggggggggggggggggggggggggggggggggggggggggggggggg
g|....|-...-|....|gggggggggggggggggggggggggggggggggggggggggggggggggggg
g---...-----...---gggggggggggggggggggggggggggggggggggggggggggggggggggg
ggg---.......---gggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggg----S----gggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
]] });
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
des.replace_terrain({ region = { 19, 00, 69, 20 }, fromterrain = "g", toterrain = "t", chance = 25 })
des.replace_terrain({ region = { 19, 00, 69, 20 }, fromterrain = "t", toterrain = "T", chance = 10 })
des.replace_terrain({ region = { 00, 00, 19, 08 }, fromterrain = "g", toterrain = "t", chance = 25 })
des.replace_terrain({ region = { 00, 00, 19, 08 }, fromterrain = "t", toterrain = "T", chance = 10 })
des.replace_terrain({ region = { 00, 18, 19, 20 }, fromterrain = "g", toterrain = "t", chance = 25 })
des.replace_terrain({ region = { 00, 18, 19, 20 }, fromterrain = "t", toterrain = "T", chance = 10 })
-- Objects
des.object()
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
des.monster({ id = "cave bear", coord = { 04, 13 }, peaceful = 0 })
des.monster({ id = "dire wolverine", coord = { 14, 13 }, peaceful = 0 })
des.monster({ id = "assassin vine", coord = { 09, 12 }, peaceful = 0 })
des.monster({ id = "assassin vine", coord = { 09, 14 }, peaceful = 0 })
for i = 1, 4 do
  des.monster({ id = "human werewolf", peaceful = 0 })
end
for i = 1, 6 do
  des.monster({ id = "skeleton", peaceful = 0 })
end
for i = 1, 4 do
  des.monster({ id = "skeleton warrior", peaceful = 0 })
end
des.monster({ id = "creeping mound", peaceful = 0 })
des.monster({ id = "creeping mound", peaceful = 0 })
des.monster({ id = "twig blight", peaceful = 0 })
des.monster({ id = "twig blight", peaceful = 0 })
des.monster({ id = "needle blight", peaceful = 0 })
des.monster({ id = "tree blight", peaceful = 0 })
des.monster({ id = "tree blight", peaceful = 0 })
des.monster({ id = "gulthias tree", peaceful = 0 })
des.monster("#")

-- Non diggable walls
des.non_diggable(selection.area(02, 09, 17, 17))
