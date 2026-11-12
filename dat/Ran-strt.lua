-- $NHDT-Header: NetHack 5.0 Ran-strt.lua 2026/06/20_22:35:38 NetHack-5.0:1.6 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Orion,
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = "." });

des.level_flags("mazelevel", "noteleport", "hardfloor", "arboreal")

des.level_init({ style="mines", fg=".", bg=".", smoothed=true, joined=true, lit=1, walled=false })
des.replace_terrain({ region={00,00, 78,20}, fromterrain=".", toterrain="g", chance=100 })
des.replace_terrain({ region={00,00, 76,19}, fromterrain="g", toterrain="T", chance=5 })
--1234567890123456789012345678901234567890123456789012345678901234567890
des.map({ halign = "left", valign = "center", map = [[
                                       xx
   ggggggggggggggggggggggggggggggggggg  x
  gg                                 gg  
 gg  gggggggggggggggFggggggggggggggg  gg 
 g  gg             gFg             gg  g 
 g gg  gggggggggggggFggggggggggggg  gg g 
 g g  gg                         gg  g g 
 g g gg  ggggggggggggggggggggggg  gg ggg 
 g g g  gg                     gg  g     
 ggg g gg  g|ggggggggggggggggggggg gggggg
 FFF g g  ggSgggggggggggggggggg          
 ggg g gg  g|ggggggggggggggggg  gggg ggg 
 g g g  gg                     gg  g g g 
 g g gg  ggggggggggggggggggggggg  gg g g 
 g g  gg                         gg  g g 
 g gg  gggggggggggggFggggggggggggg  gg g 
 g  gg             gFg             gg  g 
 gg  gggggggggggggggFggggggggggggggg  gg 
  gg                                 gg  
   ggggggggggggggggggggggggggggggggggg  x
                                       xx
]] });
-- Dungeon Description
des.region(selection.area(00,00,40,20), "lit")
-- Stairs
des.stair("down", 10,10)
-- Portal arrival point; just about anywhere on the right hand side of the map
des.levregion({ region = {51,2,77,18}, region_islev = 1, type="branch" })
-- Orion
des.monster({ id = "Orion", coord = {20, 10}, inventory = function()
   des.object({ id = "armor", spe = 4 });
   des.object({ id = "yumi", spe = 4 });
   des.object({ id = "arrow", spe = 4, quantity = 50 });
end })
-- Magic chest
des.feature("magic chest", 18, 10)
-- Altar
des.altar({ x=16, y=10, align="neutral", type="altar" })
-- Guards for the audience chamber
des.monster("hunter", 19, 09)
des.monster("hunter", 20, 09)
des.monster("hunter", 21, 09)
des.monster("hunter", 19, 10)
des.monster("hunter", 21, 10)
des.monster("hunter", 19, 11)
des.monster("hunter", 20, 11)
des.monster("hunter", 21, 11)
-- Non diggable walls
des.non_diggable(selection.area(00,00,40,20))
-- Traps
if percent(50) then
   des.trap("arrow",30,09)
else
   des.trap("crossbow bolt",30,09)
end
if percent(50) then
   des.trap("arrow",30,10)
else
   des.trap("crossbow bolt",30,10)
end
des.trap("pit",40,09)
des.trap("spiked pit")
des.trap("bear")
des.trap("bear")
-- Monsters on siege duty.
des.monster({ id = "minotaur", x=33, y=09, peaceful=0, asleep=1 })
des.monster({ id = "forest centaur", x=19, y=03, peaceful=0 })
des.monster({ id = "forest centaur", x=19, y=04, peaceful=0 })
des.monster({ id = "forest centaur", x=19, y=05, peaceful=0 })
des.monster({ id = "forest centaur", x=21, y=03, peaceful=0 })
des.monster({ id = "forest centaur", x=21, y=04, peaceful=0 })
des.monster({ id = "forest centaur", x=21, y=05, peaceful=0 })
des.monster({ id = "forest centaur", x=01, y=09, peaceful=0 })
des.monster({ id = "forest centaur", x=02, y=09, peaceful=0 })
des.monster({ id = "forest centaur", x=03, y=09, peaceful=0 })
des.monster({ id = "forest centaur", x=01, y=11, peaceful=0 })
des.monster({ id = "forest centaur", x=02, y=11, peaceful=0 })
des.monster({ id = "forest centaur", x=03, y=11, peaceful=0 })
des.monster({ id = "forest centaur", x=19, y=15, peaceful=0 })
des.monster({ id = "forest centaur", x=19, y=16, peaceful=0 })
des.monster({ id = "forest centaur", x=19, y=17, peaceful=0 })
des.monster({ id = "forest centaur", x=21, y=15, peaceful=0 })
des.monster({ id = "forest centaur", x=21, y=16, peaceful=0 })
des.monster({ id = "forest centaur", x=21, y=17, peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "scorpion", peaceful=0 })
des.monster({ id = "scorpion", peaceful=0 })
