-- NetHack gehennom hella-4.lua	$NHDT-Date: 1781994882 2026/06/20 22:34:42 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--
-- The isle of Erytheia, previously Geryon's domain.
des.level_init({ style = "solidfill", fg = "W", lit = 0 });

des.level_flags("mazelevel", "hardfloor")

des.terrain({ selection = selection.fillrect(0,0,78,20), typ = ".", lit = 1 })
des.terrain(selection.fillrect(0,0,78,20):percentage(33), "}")
des.terrain(selection.fillrect(0,0,78,20):percentage(15), "p")

des.map({ halign = "right", valign = "center", map = [[
}}}}}}}}}}}}p.......T...........................p}}}}}}}}}}}
}}}pppp}}}ppp....................................ppppp}}}}}}
}pp....pp}}}}p..........FFFFFFFFFF....................pppp}}
p........ppp}}p.........F........F......T.................pp
...........Tp}}p........F........F..........................
.............p}}p.......F........F...........----------.....
..............p}}}ppp...F........F...........|...|....|.....
...............pp}}}}p..FFFF++FFFF...........|...|....|.....
.................ppp}p.......................|...|....|.....
....T..............p}}ppppppppppppp..........+...|....|....T
....................p}}p}}}}}}}}}}}ppp.......--+----+--.....
.....................p}p}ppppppppp}}}}ppppppp|........|.....
.................T....ppppppppppppppp}}}}}}}}|........|.....
...................pppppp}}}}}pp}}}}}}ppppppp----------.....
...............pppp}}}}}}}ppp}p}}ppppp.T....................
pppp........T.p}}}}}}ppppp..ppppp......................ppppp
}}}}pppp..pppp}}ppppp.................................p}}}}}
}}}}}}}}pp}}}}}p..............T....................ppp}}}}}}
}}}}}}}}}}}}ppppp........................pppppppppp}}}}}}}}}
}}}}}}}}}}}}}}}}}p...T..................p}}}}}}}}}}}}}}}}}}}
]] });
des.door({ state = "closed", material = "bronze", x = 28, y = 7 })
des.door({ state = "closed", material = "bronze", x = 29, y = 7 })
des.door({ state = "locked", material = "bronze", x = 45, y = 9 })
des.door({ state = "closed", material = "bronze", x = 47, y = 10 })
des.door({ state = "closed", material = "bronze", x = 52, y = 10 })
des.levregion({ region = {01,01,20,19}, region_islev=1, exclude={00,00,59,19}, type="stair-up" })
des.levregion({ region = {01,01,20,19}, region_islev=1, exclude={00,00,59,19}, type="branch" })
des.teleport_region({ region = {01,01,20,19}, region_islev=1, exclude={00,00,59,19} })
des.stair("down", 50, 7)
des.region({ region = {00,00,59,19}, lit = 1, type = "ordinary" })
des.region({ region = {46,06,53,12}, lit = 0, type = "ordinary" })
des.non_diggable(selection.area(45,05,54,13))
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster("Juiblex", 51, 8)
   else
      des.monster("Baalzebub", 51, 8)
   end
else
   if percent(50) then
      des.monster("Geryon", 51, 8)
   else
      des.monster("Baphomet", 51, 8)
   end
end
-- the herd
for y = 3, 6 do
   local x0 = (y % 2 == 1) and 25 or 26
   for i = 0, 3 do
      des.monster({ class = "q", coord = { x0 + i * 2, y }, peaceful = 0 })
   end
end
-- its guard
des.monster({ id = "hell hound", coord = { 28, 8 }, peaceful = 0 })
-- the eponymous residents
for i = 1, 10 do
   des.monster("n")
end
-- some other denizens
for i = 1, 5 do
   des.monster({ class = "&", peaceful = 0 })
end
for i = 1, 10 do
   des.monster({ class = "i", peaceful = 0 })
end
-- a bit of loot
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object("(")
des.object("/")
des.object("=")
des.object("?")
des.object("?")
des.object("!")
des.object("!")
for i = 1, 8 do
   des.object("*")
end
-- traps!
for i = 1, 10 do
   des.trap("fire")
end
for i = 1, 3 do
   des.trap("magic")
end
for i = 1, 5 do
   des.trap()
end
