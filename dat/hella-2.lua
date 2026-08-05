-- NetHack gehennom hella-2.lua	$NHDT-Date: 1781994877 2026/06/20 22:34:37 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = ".", lit = 1 });

des.level_flags("mazelevel", "hardfloor", "shortsighted")

des.terrain(selection.fillrect(0,0,78,20):percentage(33), "}")
des.terrain(selection.fillrect(0,0,78,20):percentage(15), "s")

-- First part
des.map({ halign = "half-left", valign = "center", map = [[
-------FFFF------FF--
|.............|.....|
|.............S.....|
|---+------------...|
|.....|.........|-+--
F..---|.........|....
F..|..S.........|....
|..|..|.........|....
|..|..|.........|-+--
|..|..-----------...|
|..S..........|.....|
-------FFFF------FF--
]], contents = function(rm)
   -- Doors
   des.door({ state = "closed", material = "bronze", x = 4, y = 3 })
   des.door({ state = "locked", material = "bronze", x = 18, y = 4 })
   des.door({ state = "closed", material = "bronze", x = 18, y = 8 })
   -- Secret doors
   des.door({ state = "secret", material = "bronze", x = 3, y = 10 })
   des.door({ state = "secret", material = "bronze", x = 6, y = 6 })
   des.door({ state = "secret", material = "bronze", x = 14, y = 2 })
   -- monsters and traps
   des.monster("&")
   des.monster("&")
   des.monster("&")
   des.trap("anti magic")
   des.trap("fire")
   des.trap("magic beam")
   --
   des.stair("down", 13,07)
   -- Non diggable walls
   des.non_diggable(selection.area(00,00,20,11))
   -- Entire main area
   des.region(selection.area(01,01,20,10),"unlit")
   -- random demon boss
   if percent(50) then
      if percent(50) then
         des.monster("Juiblex", 12, 7)
      else
         des.monster("Baalzebub", 12, 7)
      end
   else
      if percent(50) then
         des.monster("Geryon", 12, 7)
      else
         des.monster("Baphomet", 12, 7)
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
   des.trap("spiked pit", 05,02)
   des.trap("fire", 08,06)
   des.trap("sleep gas")
   des.trap("anti magic")
   des.trap("fire")
   des.trap("magic")
   des.trap("spear")
   -- Random monsters.
   des.monster("ghost",11,07)
   des.monster("horned devil",10,05)
   des.monster("L")
   -- Some Vampires for good measure
   des.monster("V")
   des.monster("V")
   des.monster("V")
end });

des.levregion({ region={01,00,13,20}, region_islev=1, exclude={14,01,66,20}, exclude_islev=1, type="stair-up" });

des.levregion({ region={01,00,13,20}, region_islev=1, exclude={14,01,66,20}, exclude_islev=1, type="branch" });
des.teleport_region({ region = {01,00,13,20}, region_islev=1, exclude={14,01,66,20}, exclude_islev=1 })

-- Second part
des.map({ halign = "half-right", valign = "center", map = [[
---FFFF-------FFFF-------FFFF----
................................|
................................|
................................|
---FFFF-------FFFF-------FFFF----
]], contents = function(rm)
   -- BRANCH:(29,01,31,03),(00,00,00,00)
   -- TELEPORT_REGION:(29,01,31,03),(00,00,00,00)
   des.mazewalk(32,02,"east")
   -- Non diggable walls
   des.non_diggable(selection.area(00,00,32,04))
   des.drawbridge({ x = 33, y = 2, dir = "west", state = "closed" })
   des.monster("&")
   des.monster("&")
   des.monster("&")
   des.trap("anti magic")
   des.trap("fire")
   des.trap("magic beam")
end });
