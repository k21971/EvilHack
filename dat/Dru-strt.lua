-- NetHack Druid Dru-strt.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader,
--	Elanee, and receive your quest assignment.
--

des.level_flags("mazelevel", "noteleport", "hardfloor", "arboreal")
des.level_init({ style = "mines", fg = ".", bg = ".", smoothed = true,
                 joined = true, lit = 1, walled = false })

des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = ".", toterrain = "g", chance = 100 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "g", toterrain = "T", chance = 30 })
des.replace_terrain({ region = { 00, 00, 78, 20 }, fromterrain = "T", toterrain = "t", chance = 10 })
--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "right", valign = "center", map = [[
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggTTTTTTTgggggggg
ggggggggggggggggggggggggggggggggggggggggggggTTTTTTTTTTTgggggg
gggggggggggggggggggggggggggggggggggggggggggTTgggggggggTTggggg
ggggggggggggggggggggggggggggggggggggggggggTTgggggggggggTTgggg
gggggggggggggggggggggggggggggggggggggggggTTgggggggggggggTTTTg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggTg
gggggggggggggggggggggggggggggggggggggggggTTgggggggggggggTTTTg
ggggggggggggggggggggggggggggggggggggggggggTTgggggggggggTTgggg
gggggggggggggggggggggggggggggggggggggggggggTTgggggggggTTggggg
ggggggggggggggggggggggggggggggggggggggggggggTTTTTTTTTTTgggggg
ggggggggggggggggggggggggggggggggggggggggggggggTTTTTTTgggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
]] });
-- Dungeon Description
des.region({ region = { 00, 00, 60, 20 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("down", 58, 11)
-- Portal arrival point
des.levregion({ region = { 01, 01, 06, 18 }, region_islev = 1,
                exclude = { 00, 00, 60, 20 }, type = "branch" })
des.teleport_region({ region = { 01, 01, 06, 18 }, region_islev = 1,
                      exclude = { 00, 00, 60, 20 } })
-- populate trees in REGION area
des.replace_terrain({ region = { 00, 00, 39, 20 }, fromterrain = "g", toterrain = "T", chance = 30 })
des.replace_terrain({ region = { 00, 00, 39, 20 }, fromterrain = "T", toterrain = "t", chance = 10 })
des.replace_terrain({ region = { 40, 00, 60, 05 }, fromterrain = "g", toterrain = "T", chance = 30 })
des.replace_terrain({ region = { 40, 00, 60, 05 }, fromterrain = "T", toterrain = "t", chance = 10 })
des.replace_terrain({ region = { 40, 17, 60, 20 }, fromterrain = "g", toterrain = "T", chance = 30 })
des.replace_terrain({ region = { 40, 17, 60, 20 }, fromterrain = "T", toterrain = "t", chance = 10 })
-- Elanee
des.monster("Elanee", 53, 11)
-- Magic chest
des.feature("magic chest", 56, 11)
-- Altar
des.altar({ coord = { 49, 11 }, align = "neutral", type = "altar" })
-- Guards for the audience chamber
des.monster("aspirant", 43, 10)
des.monster("aspirant", 43, 12)
des.monster("aspirant", 45, 08)
des.monster("aspirant", 45, 14)
des.monster("aspirant", 53, 08)
des.monster("aspirant", 53, 14)
des.monster("aspirant", 55, 10)
des.monster("aspirant", 55, 12)
-- Fountains
des.feature("fountain", 49, 09)
des.feature("fountain", 49, 13)
-- Traps
des.trap("pit")
des.trap("pit")
des.trap("spiked pit")
des.trap("spiked pit")
-- Monsters
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
des.monster({ id = "tree blight", peaceful = 0 })
des.monster("deer")
des.monster("deer")
des.monster("stag")
des.monster("satyr")
des.monster("#")

-- Non diggable walls
des.non_diggable(selection.area(41, 06, 59, 16))
