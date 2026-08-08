-- NetHack Convict Con-goal.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "goal" level for the quest.
--
--	Here you meet Warden Arianna, your nemesis monster.  You have to
--	defeat Warden Arianna in combat to gain the artifact you have
--	been assigned to retrieve.
--

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "solidfill", fg = " " })

--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "top", map = [[
---------------------------------------------------------
|...|        |............................|             |
|...|       --............................|             |
|...|       |.............................|             |
|...|      --.............................|             |
|...|      |..............................|             |
|...|     --...................L..........|             |
|...|     |....................L..........|             |
|...|    --...................L.L.........|             |
|...|    |..............L.L.L.L.L.L.L.L...|             |
|...|-----................L..L...L..L.....|             |
|...........................L.L.L.L.......|             |
|...........................L.L.L.L.......|             |
|..........................L.L...L.L......|             |
|.........................................|-------------|
|.........................................|.............|
|.........................................S.............|
|.........................................|.............|
|.........................................|.............|
---------------------------------------------------------
]] });
-- Dungeon Description
des.region({ region = { 00, 00, 56, 19 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("up", 02, 02)
-- Non diggable walls
des.non_diggable(selection.area(00, 00, 56, 19))
-- Random traps
for i = 1, 6 do
  des.trap("fire")
end
-- Lava demons and salamanders
des.monster({ id = "lava demon", coord = { 31, 05 }, peaceful = 0, asleep = 0 })
des.monster({ id = "lava demon", coord = { 23, 09 }, peaceful = 0, asleep = 0 })
des.monster({ id = "lava demon", coord = { 39, 09 }, peaceful = 0, asleep = 0 })
des.monster({ id = "lava demon", coord = { 36, 14 }, peaceful = 0, asleep = 0 })
des.monster({ id = "lava demon", coord = { 26, 14 }, peaceful = 0, asleep = 0 })
des.monster({ id = "salamander", coord = { 29, 10 }, peaceful = 0, asleep = 0 })
des.monster({ id = "salamander", coord = { 33, 10 }, peaceful = 0, asleep = 0 })
-- Elite guard
des.monster({ id = "iron golem", coord = { 04, 13 }, peaceful = 0 })
-- Objects
des.object({ id = "striped shirt", coord = { 31, 10 }, buc = "blessed",
             spe = 0, name = "The Striped Shirt of Liberation" })
des.object({ id = "chest", coord = { 55, 18 }, buc = "blessed", spe = 2 })
-- Warden Arianna
des.monster({ id = "Warden Arianna", coord = { 31, 10 }, peaceful = 0 })
