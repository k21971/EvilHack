-- $NHDT-Header: NetHack 5.0 orcus.lua 2026/06/20_22:35:38 NetHack-5.0:1.8 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="mazegrid", bg ="-" });

des.level_flags("mazelevel", "noteleport", "shortsighted")

des.level_init({ style = "mines", fg = ".", bg = " ", smoothed = true,
                 joined = true, lit = 0, walled = true });

-- A ghost town
des.map({ halign = "right", valign = "center", map = [[
 LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL 
.---------------------------------------------L
.|....|....|....|..............|....|........|L
.|....|....|....|..............|....|........|L
.|....|....|....|--...-+-------|.............|L
.|....|....|....|..............+.............|L
.|.........|....|..............|....|........|L
.--+-...-+----+--....-------...--------.-+----L
.....................|.....|.................LL
.....................|.....|.................LL
.--+----....-+---....|.....|...----------+----L
.|....|....|....|....---+---...|......|......|L
.|.........|....|..............|......|......|L
.----...---------.....-----....+......|......|L
.|........................|....|......|......|L
.----------+-...--+--|....|....----------+---|L
.|....|..............|....+....|.............|L
.|....+.......|......|....|....|.............|L
.|....|.......|......|....|....|.............|L
.---------------------------------------------L
 LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL 
]], contents = function(rm)
   -- Entire main area
   des.region(selection.area(1,0,46,18),"unlit")
   des.stair("down", 33,17)
   -- knock out the lava-frame seed cells, then flood the
   -- surrounding mines fill with lava from those seeds
   local seeds = { {0,0},{46,0},{0,18},{46,18} }
   for i = 1, #seeds do
      des.terrain(seeds[i][1], seeds[i][2], " ")
   end
   for i = 1, #seeds do
      des.terrain(selection.floodfill(seeds[i][1], seeds[i][2]), "L")
   end
   -- Wall "ruins"
   des.object("boulder",19,4)
   des.object("boulder",20,4)
   des.object("boulder",21,4)
   des.object("boulder",36,4)
   des.object("boulder",36,5)
   des.object("boulder",6,6)
   des.object("boulder",5,7)
   des.object("boulder",6,7)
   des.object("boulder",7,7)
   des.object("boulder",39,7)
   des.object("boulder",8,10)
   des.object("boulder",9,10)
   des.object("boulder",10,10)
   des.object("boulder",11,10)
   des.object("boulder",6,12)
   des.object("boulder",5,13)
   des.object("boulder",6,13)
   des.object("boulder",7,13)
   des.object("boulder",21,13)
   des.object("boulder",21,14)
   des.object("boulder",13,15)
   des.object("boulder",14,15)
   des.object("boulder",15,15)
   des.object("boulder",14,16)
   -- Doors
   des.door({ state = "closed", material = "bronze", x = 23, y = 4 })
   des.door({ state = "open", material = "bronze", x = 31, y = 5 })
   des.door("nodoor",3,7)
   des.door({ state = "closed", material = "bronze", x = 9, y = 7 })
   des.door({ state = "closed", material = "bronze", x = 14, y = 7 })
   des.door({ state = "closed", material = "bronze", x = 41, y = 7 })
   des.door({ state = "open", material = "bronze", x = 3, y = 10 })
   des.door("nodoor",13,10)
   des.door({ state = "open", material = "bronze", x = 41, y = 10 })
   des.door({ state = "closed", material = "bronze", x = 24, y = 11 })
   des.door({ state = "closed", material = "bronze", x = 31, y = 13 })
   des.door({ state = "open", material = "bronze", x = 11, y = 15 })
   des.door({ state = "closed", material = "bronze", x = 18, y = 15 })
   des.door({ state = "closed", material = "bronze", x = 41, y = 15 })
   des.door({ state = "open", material = "bronze", x = 26, y = 16 })
   des.door({ state = "closed", material = "bronze", x = 6, y = 17 })
   -- Special rooms
   des.altar({ x=24,y=9,align="noalign",type="sanctum" })
   des.region({ region={22,14,25,18},lit=0,type="morgue",filled=1 })
   des.region({ region={32,11,37,14},lit=1,type="shop",filled=1 })
   des.region({ region={12,2,15,6},lit=1,type="shop",filled=1 })
   -- Some traps.
   des.trap("spiked pit")
   des.trap("sleep gas")
   des.trap("anti magic")
   des.trap("fire")
   des.trap("fire")
   des.trap("ice")
   des.trap("magic")
   des.trap("magic")
   -- Some random objects
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   -- The resident nasty
   des.monster({ id = "Orcus", x = 33, y = 17, peaceful = 0 })
   -- And its preferred companions
   des.monster("human zombie",32,17)
   des.monster("shade",32,16)
   des.monster("shade",32,18)
   des.monster("vampire sovereign",35,18)
   des.monster("vampire sovereign",35,16)
   des.monster("vampire noble",36,16)
   des.monster("vampire noble",36,17)
   -- Watches the sanctum
   des.monster("alhoon",24,9)
   -- Randomly placed companions
   des.monster("skeleton warrior")
   des.monster("skeleton warrior")
   des.monster("skeleton warrior")
   des.monster("skeleton warrior")
   des.monster("skeletal dragon")
   des.monster("shade")
   des.monster("shade")
   des.monster("shade")
   des.monster("spectre")
   des.monster("giant zombie")
   des.monster("giant zombie")
   des.monster("giant zombie")
   des.monster("ettin zombie")
   des.monster("ettin zombie")
   des.monster("ettin zombie")
   des.monster("human zombie")
   des.monster("revenant")
   des.monster("revenant")
   des.monster("vampire sovereign")
   des.monster("vampire noble")
   des.monster("vampire noble")
   des.monster("vampire royal")
   des.monster("vampire royal")
   -- Lava
   des.monster({ id = "salamander", x = 46, y = 1, peaceful = 0 })
   des.monster({ id = "salamander", x = 46, y = 17, peaceful = 0 })
   if percent(20) then
      des.monster({ id = "lava gremlin", x = 46, y = 9, peaceful = 0 })
   end
   -- A few more for the party
   des.monster()
   des.monster()
   des.monster()
   des.monster()
   des.monster()
end });

des.levregion({ region={1,1,20,20}, region_islev=1, exclude={20,1,79,20}, exclude_islev=1, type="stair-up" });
des.levregion({ region={1,1,20,20}, region_islev=1, exclude={20,1,79,20}, exclude_islev=1, type="branch" });
des.teleport_region({ region={1,1,20,20}, region_islev=1, exclude={20,1,79,20}, exclude_islev=1 });
