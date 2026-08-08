-- NetHack Monk Mon-strt.lua	$NHDT-Date: 1781994869 2026/06/20 22:34:29 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.9 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Master Po,
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor")

des.map([[
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggg------------------------------------gggggggggggggggggggg
gggggggggggggggggggg|................|.....|.....|.....|gggggggggggggggggggg
gggggggggggggggggggg|..------------..|--+-----+-----+--|gggggggggggggggggggg
gggggggggggggggggggg|..|..........|..|.................|gggggggggggggggggggg
gggggggggggggggggggg|..|..........|..|+---+---+-----+--|gggggggggggggggggggg
gggggggggggggggggg---..|..........|......|...|...|.....|gggggggggggggggggggg
gggggggggggggggggg+....|..........+......|...|...|.....|gggggggggggggggggggg
gggggggggggggggggg+....|..........+......|...|...|.....|gggggggggggggggggggg
gggggggggggggggggg---..|..........|......|...|...|.....|gggggggggggggggggggg
gggggggggggggggggggg|..|..........|..|+-----+---+---+--|gggggggggggggggggggg
gggggggggggggggggggg|..|..........|..|.................|gggggggggggggggggggg
gggggggggggggggggggg|..------------..|--+-----+-----+--|gggggggggggggggggggg
gggggggggggggggggggg|................|.....|.....|.....|gggggggggggggggggggg
gggggggggggggggggggg------------------------------------gggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
]]);
-- Dungeon Description
des.region(selection.area(00,00,75,19), "lit")
des.region({ region={24,06, 33,13}, lit=1, type="temple", filled=1 })

des.replace_terrain({ region={00,00, 10,19}, fromterrain="g", toterrain="T", chance=10 })
des.replace_terrain({ region={65,00, 75,19}, fromterrain="g", toterrain="T", chance=10 })

-- Portal arrival point
des.terrain({05,04}, ".")
des.levregion({ region = {05,04,05,04}, type="branch" })
-- Stairs
des.stair("down", 52,09)
-- Doors
des.door("locked",18,09)
des.door("locked",18,10)
des.door("closed",34,09)
des.door("closed",34,10)
des.door("closed",40,05)
des.door("closed",46,05)
des.door("closed",52,05)
des.door("locked",38,07)
des.door("closed",42,07)
des.door("closed",46,07)
des.door("closed",52,07)
des.door("locked",38,12)
des.door("closed",44,12)
des.door("closed",48,12)
des.door("closed",52,12)
des.door("closed",40,14)
des.door("closed",46,14)
des.door("closed",52,14)
-- Unattended Altar - unaligned due to conflict - player must align it.
des.altar({ x=28,y=09, align="noalign", type="altar" })
-- Master Po
des.monster({ id = "Master Po", coord = {28, 10}, inventory = function()
   des.object({ id = "robe", spe = 6 });
end })
-- Magic chest
des.feature("magic chest", 26, 09)
-- guards for the audience chamber
des.monster("abbot", 32, 07)
des.monster("abbot", 32, 08)
des.monster("abbot", 32, 11)
des.monster("abbot", 32, 12)
des.monster("abbot", 33, 07)
des.monster("abbot", 33, 08)
des.monster("abbot", 33, 11)
des.monster("abbot", 33, 12)
-- Non diggable walls
des.non_diggable(selection.area(18,03,55,16))
-- Random traps
des.trap("dart", 20, 09)
des.trap("dart", 20, 10)
des.trap()
des.trap()
des.trap()
des.trap()
-- Monsters on siege duty.
des.monster("earth elemental", 37, 01)
des.monster("earth elemental", 37, 18)
des.monster("earth elemental", 03, 03)
des.monster("earth elemental", 65, 04)
des.monster("earth elemental", 12, 11)
des.monster("earth elemental", 60, 12)
des.monster("earth elemental", 14, 08)
des.monster("earth elemental", 55, 00)
des.monster("xorn", 18, 18)
des.monster("xorn", 59, 10)
des.monster("xorn", 13, 09)
des.monster("xorn", 01, 17)
-- next to leader, so possibly tricky to pick up if not ready for quest yet;
-- there's no protection against a xorn eating these tins; BUC state is random
des.object({ id="tin", coord = {29, 9}, quantity=2, montype="spinach" })
-- ensure enough vegetarian food generates for vegetarian games
des.object({ id="food ration", coord = {46, 4}, quantity = 4})
