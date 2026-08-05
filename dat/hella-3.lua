-- NetHack gehennom hella-3.lua	$NHDT-Date: 1781994878 2026/06/20 22:34:38 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.6 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = ".", lit = 0 });

des.level_flags("mazelevel", "hardfloor", "shortsighted")

des.terrain(selection.fillrect(0,0,78,20):percentage(33), "}")
des.terrain(selection.fillrect(0,0,78,20):percentage(15), "s")

des.map({ halign = "center", valign = "center", map = [[
xxxxxxxxxxxxxxxxxxxxxxxFxxxxxxxxxxxxxxxxxxFxxxxxx
xxxxxxxxxxxFxxxxxxxxxFFFxxxxxxxxxxxxxxxFFFFxxxxxx
xxxxxxxxxxxFFFFxxxxxxFxxx------------xxFxxxxxxxxx
xxFFFFFFxxxxxxFxx--------|..........|FFFxxxxxxxxx
xxF....Fxx-------|...........--------------xxxxxx
---....|--|..................S............|----xx
....--....S..----------------|............S...|xx
---....|--|..................|............|----xx
xxF....Fxx-------|...........-----S--------xxxxxx
xxFFFFFFxxxxxxFxx--------|..........|FFFxxxxxxxxx
xxxxxxxxxxxFFFFxxxxxFxxxx------------xxFxxxxxxxxx
xxxxxxxxxxxFxxxxxxxxFFFxxxxxxxxxxxxxxxxFFFFxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxFxxxxxxxxxxxxxxxxxxFxxxxxx
]] });
des.levregion({ region = {01,00,15,20}, region_islev=1, exclude={15,1,70,16}, exclude_islev=1, type="stair-up" })
des.levregion({ region = {01,00,15,20}, region_islev=1, exclude={15,1,70,16}, exclude_islev=1, type="branch" })
des.teleport_region({region = {01,00,15,20}, region_islev=1, exclude = {15,1,70,16}, exclude_islev=1 })
-- this actually leaves the farthest right column diggable
des.non_diggable(selection.area(00,00,47,12))
des.stair("down", 44,06)
des.door({ state = "locked", material = "bronze", x = 0, y = 6 })
-- Secret doors
des.door({ state = "secret", material = "bronze", x = 10, y = 6 })
des.door({ state = "secret", material = "bronze", x = 29, y = 5 })
des.door({ state = "secret", material = "bronze", x = 34, y = 8 })
des.door({ state = "secret", material = "bronze", x = 42, y = 6 })
if percent(50) then
   des.terrain(34, 8, "-")
   des.door({ state = "secret", material = "bronze", x = 34, y = 4 })
   des.terrain(29, 5, "|")
   des.door({ state = "secret", material = "bronze", x = 29, y = 7 })
end
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster("Juiblex", 35, 6)
   else
      des.monster("Baalzebub", 35, 6)
   end
else
   if percent(50) then
      des.monster("Geryon", 35, 6)
   else
      des.monster("Baphomet", 35, 6)
   end
end
-- Some random weapons and armor.
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object("*")
des.object("!")
des.object("!")
des.object("?")
des.object("?")
des.object("?")
-- Some traps.
des.trap("spiked pit")
des.trap("fire")
des.trap("sleep gas")
des.trap("anti magic")
des.trap("rust")
des.trap("magic")
des.trap("magic beam")
-- Random monsters.
des.monster("spectre",37,07)
des.monster("horned devil",32,05)
des.monster("barbed devil",38,07)
des.monster("weredemon")
des.monster("L")

for i = 1, 10 + d(4, 7) do
   des.monster("a")
   if percent(25) then
      des.monster("P")
   end
   if percent(15) then
      des.monster(";")
   end
end

-- Some Vampires for good measure
des.monster("V")
des.monster("V")
des.monster("V")
