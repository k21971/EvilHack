-- NetHack Convict Con-strt.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Robert the Lifer
--	and receive your quest assignment.
--

des.level_flags("mazelevel", "noteleport", "hardfloor")
des.level_init({ style = "solidfill", fg = " " })

--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "top", map = [[
|--------------------------------------------------------|-----------------|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|................K|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|---.---.---.---.---.---.---.---.---.---.---.---.---.---.|.................|
|..........................................................................|
|..........................................................................|
|....|.---.---.---.---.---.---.---.---.---.---.---.---.--------------..----|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|....|---------------------------------------------------|.................|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|....|--.---.---.---.---.---.---.---.---.---.---.---.---.|-----------------|
|..........................................................................|
|..........................................................................|
|..........................................................................|
|.----.---.---.---.---.---.---.---.---.---.---.---.---.--|.................|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|....|...|...|...|...|...|...|...|...|...|...|...|...|...|.................|
|--------------------------------------------------------|-----------------|
]] });
-- Dungeon Description
des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("down", 64, 08)
-- Portal arrival point
des.levregion({ region = { 71, 03, 71, 03 }, type = "branch" })
-- Altar
des.altar({ coord = { 70, 16 }, align = "chaos", type = "shrine" })
-- Magic chest
des.feature("magic chest", 71, 16)
-- Robert the Lifer
des.monster("Robert the Lifer", 74, 18)
-- fellow prisoners
for i = 1, 6 do
  des.monster("inmate")
end
-- Corrupt guards
for i = 1, 6 do
  if percent(50) then
    des.monster("prison guard")
  end
end
-- Good `ol mimics
des.monster({ id = "giant mimic", coord = { 74, 05 }, appear_as = "ter:staircase up" })
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
  if percent(50) then
    des.object("heavy iron ball")
  end
end
-- Non diggable walls
des.non_diggable(selection.area(00, 00, 75, 19))
