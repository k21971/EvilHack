-- NetHack Convict Con-fila.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
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

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "solidfill", fg = " " })

--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "top", map = [[
----------------------------------------------------------------------------
|....................---------.......................---------.............|
|....................F...|...|.......................F...|...|.............|
|....................|...|...F.......................|...|...F.............|
|....|---|---|.......|---|---|.......|---|---|.......|---|---|.............|
|....F...|...|.......F...|...|.......F...|...|.......F...|...|.............|
|....|...|...F.......|...|...F.......|...|...F.......|...|...F.............|
|....|---|---|.......|---|---|.......|---|---|.......|---|---|.............|
|....F...|...|.......F...|...|.......F...|...|.......F...|...|.............|
|....|...|...F.......|...|...F.......|...|...F.......|...|...F.............|
|....|---|---|.......|---|---|.......|---|---|.......|---|---|.............|
|........|...|.......F...|...|.......F...|...|.......F...|...|.............|
|....|...|...F.......|...|...F.......|...|...F.......|...|...F.............|
|....|---|---|.......|---|---|.......|---|---|.......|---|---|.............|
|....F...|...|.......F...|...|.......F...|...|.......F...|...|.............|
|....|...|...F.......|...|...........|...|...F.......|...|...F.............|
|....|---|---|.......|---|---|.......|---|---|.......|---|---|.............|
|........|...|.......................F...|...|.............................|
|....|...|...F.......................|...|...F.............................|
|--------------------------------------------------------------------------|
]] });
-- Dungeon Description
des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("up", 74, 03)
des.stair("down", 03, 17)
-- Non diggable walls
des.non_diggable(selection.area(00, 00, 75, 19))
-- "Regular" prisoners
des.monster("inmate", 59, 02)
des.monster("inmate", 55, 08)
des.monster("inmate", 43, 14)
des.monster("inmate", 38, 05)
des.monster("inmate", 27, 02)
des.monster("inmate", 23, 08)
des.monster("inmate", 11, 14)
des.monster("inmate", 06, 05)
-- Undead prisoners
des.monster({ id = "ghost", coord = { 42, 17 }, name = "Orzo the Inmate" })
if percent(50) then
  des.monster({ id = "ghost", coord = { 40, 18 }, name = "Fredgar the Inmate" })
end
if percent(50) then
  des.monster({ id = "ghost", coord = { 06, 12 }, name = "Rastilon the Inmate" })
end
des.monster({ id = "skeleton warrior", coord = { 28, 15 }, asleep = 0 })
-- Bugs and snakes
des.monster("pit viper", 06, 17)
des.monster("xan")
-- Corrupt guards
for i = 1, 4 do
  if percent(50) then
    des.monster("prison guard")
  end
end
-- Random traps
for i = 1, 4 do
  des.trap("web")
end
-- Prison debris
for i = 1, 5 do
  if percent(75) then
    des.object("iron chain")
  end
end
for i = 1, 2 do
  if percent(75) then
    des.object("heavy iron ball")
  end
end
