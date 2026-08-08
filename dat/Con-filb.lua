-- NetHack Convict Con-filb.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
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
des.level_init({ style = "mines", fg = "#", bg = " ", smoothed = true,
                 joined = true, lit = 0, walled = false })

des.message("This appears to be a prison level that is still under construction.")

-- Stairs
des.stair("up")
des.stair("down")
-- Peaceful miners
for i = 1, 8 do
  des.monster({ id = "miner", peaceful = 1 })
end
-- Earth creatures
des.monster("xorn")
des.monster("earth elemental")
if percent(50) then
  des.monster("xorn")
end
if percent(50) then
  des.monster("earth elemental")
end
-- Other nasties
des.monster("lurker above")
des.monster("trapper")
if percent(50) then
  des.monster("lurker above")
end
if percent(50) then
  des.monster("trapper")
end
for i = 1, 2 do
  if percent(50) then
    des.monster("rock piercer")
  end
end
for i = 1, 2 do
  if percent(50) then
    des.monster("iron piercer")
  end
end
for i = 1, 2 do
  if percent(50) then
    des.monster("glass piercer")
  end
end
-- Tools and corpses
if percent(50) then
  des.object("pick-axe")
end
if percent(75) then
  des.object("lantern")
end
if percent(50) then
  des.object("(")
end
for i = 1, 4 do
  if percent(25) then
    des.object({ id = "corpse", montype = "miner", spe = 0 })
  end
end
-- Natural cavern hazards
for i = 1, 6 do
  des.trap("pit")
end
for i = 1, 4 do
  des.trap("falling rock")
end
for i = 1, 2 do
  des.trap("web")
end
