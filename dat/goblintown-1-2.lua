-- NetHack gtown goblintown-1-2.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2022 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
-- Goblin Town variant 2
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify",
                "shortsighted")


--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
  ..t                                     ..t.               ...............
 ....                                  ........BBBBBBBBBBBBB..   ..    ....t
......               BBBBBBBBBBBBBBBBB..  FF.F             ..          ....t
..........BBBBBBBBBBBBB               .    ..             ..            t...
 tt..    ..                           .     ..           ..              ...
  ..      .t                         -+-                 .                ..
          .                         .....                .                  
         ..                      ...........            ...t.               
          .-------              .\............         .......              
         ..|.....|               ............            ...t..    ..       
         ..|.....|                 .......            ........    ...       
      .....+.....|                   -+-             ..  ......  FF.F       
     ..  ..-------                    .                 tt....  ......      
    ..    ...                         .                  ..........         
   ..                                ..                   ..                
   .                           ..t   .                     .         tt.    
  ..t             BBBBBBBBBBBB........BBBBBB               .        ......  
 .....BBBBBBBBBBBBBB            F.F      BBBBBBBBBBBBBBBBBB..       ...t... 
.......                        ....                         ................
 tt.t                                                              .tt....  
                                                                            
]] });
des.region({ region = { 00, 00, 75, 19 }, lit = 0, type = "ordinary" })
des.region({ region = { 31, 18, 34, 18 }, lit = 1, type = "ordinary" })
des.region({ region = { 43, 03, 44, 04 }, lit = 1, type = "ordinary" })
des.region({ region = { 66, 09, 68, 10 }, lit = 1, type = "ordinary" })
des.region({ region = { 32, 06, 45, 10 }, lit = 1, type = "ordinary" })

-- random trap door that leads to Gollum's Cave
local gollum_cave = { {45,04},{31,18},{68,09},{12,13},{02,05} }
shuffle(gollum_cave);

-- random other traps
local traps = { {69,00},{64,18},{58,09},{38,03},
           {37,15},{35,08},{07,11},{03,02},
           {03,17},{39,07} }
shuffle(traps);

-- Make the arrival, exit, and paths somewhat unpredictable
if percent(50) then
   des.teleport_region({ region = { 75, 18, 75, 18 } })
   des.levregion({ region = {02,00,02,00}, type="branch" })
   des.terrain(selection.line(10,03, 22,03), " ")
   des.terrain(selection.line(21,02, 37,02), " ")
   des.terrain(selection.line(06,17, 19,17), "B")
   des.terrain(selection.line(18,16, 29,16), "B")
   des.terrain(selection.line(38,16, 43,16), " ")
   des.terrain(selection.line(41,17, 58,17), " ")
   des.terrain(selection.line(47,01, 59,01), "B")
else
   des.teleport_region({ region = { 75, 05, 75, 05 } })
   des.levregion({ region = {00,18,00,18}, type="branch" })
   des.terrain(selection.line(10,03, 22,03), "B")
   des.terrain(selection.line(21,02, 37,02), "B")
   des.terrain(selection.line(06,17, 19,17), " ")
   des.terrain(selection.line(18,16, 29,16), " ")
   des.terrain(selection.line(38,16, 43,16), "B")
   des.terrain(selection.line(41,17, 58,17), "B")
   des.terrain(selection.line(47,01, 59,01), " ")
end

-- Shop (maybe?)
if percent(50) then
   if percent(50) then
      des.region({ region = { 12, 09, 16, 11 }, lit = 1, type = "armor shop", filled = 1 })
   else
      des.region({ region = { 12, 09, 16, 11 }, lit = 1, type = "weapon shop", filled = 1 })
   end
end

-- Door to jail cells?
if percent(35) then
   des.terrain(67, 11, "F")
else
   des.door("locked", 67, 11)
end
if percent(50) then
   des.terrain(44, 02, "F")
else
   des.door("locked", 44, 02)
end
if percent(50) then
   des.terrain(33, 17, "F")
else
   des.door("locked", 33, 17)
end

-- Forge
des.feature("forge", 59, 11)

-- Magic chest
des.feature("magic chest", 32, 08)

-- Various objects
des.object({ id = "chest", trapped = 0, coord = { 67, 09 }, contents = function()
   des.object({ id = "novel", buc = "uncursed", name = "The Blacksmith's Cookbook" })
   des.object("blacksmith hammer")
   if percent(50) then
      des.object({ id = "acid", buc = "uncursed" })
   end
end })

-- Doors
des.door("locked", 11, 11)
des.door("locked", 38, 05)
des.door("locked", 38, 11)

-- Traps - eight out of ten possible locales
des.trap("board", traps[1])
des.trap("board", traps[2])
des.trap("board", traps[3])
des.trap("arrow", traps[4])
des.trap("arrow", traps[5])
des.trap("crossbow bolt", traps[6])
des.trap("crossbow bolt", traps[7])
des.trap("bear", traps[8])

des.trap("trap door", gollum_cave[1])

-- Prisoners
des.monster({ id = "Woodland-elf", coord = { 66, 10 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 68, 10 }, peaceful = 1 })
des.monster({ id = "Woodland-elf", coord = { 43, 03 }, peaceful = 1 })
des.monster({ id = "mountain dwarf", coord = { 67, 10 }, peaceful = 1 })
des.monster({ id = "Green-elf", coord = { 44, 03 }, peaceful = 1 })
des.monster({ id = "gnome noble", coord = { 44, 04 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 31, 18 }, peaceful = 1 })
des.monster({ id = "Grey-elf", coord = { 32, 18 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 34, 18 }, peaceful = 1 })

-- Residents of Goblin Town
des.monster({ id = "goblin", coord = { 72, 01 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 66, 00 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 70, 17 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 66, 18 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 38, 06 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 38, 10 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 04, 01 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 03, 03 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 05, 17 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 03, 19 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 09, 07 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 10, 12 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 38, 02 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 38, 13 }, peaceful = 0 })
des.monster({ id = "goblin shaman", coord = { 42, 08 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 53, 11 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 61, 09 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 45, 00 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 31, 15 }, peaceful = 0 })
des.monster({ id = "bugbear", coord = { 69, 12 }, peaceful = 0 })

-- The Goblin King and his treasure
des.monster({ id = "Goblin King", coord = { 33, 08 }, peaceful = 0 })

des.object({ id = "chest", coord = { 33, 07 }, contents = function()
   if percent(10) then
      des.object({ id = "magic marker", buc = "uncursed" })
   end
   des.object("*")
   des.object("*")
   des.object("*")
   des.object("=")
   des.object({ class = "$", quantity = 100 })
end })

-- None shall pass
des.non_diggable(selection.area(00,00,31,19))
des.non_diggable(selection.area(32,00,41,16))
des.non_diggable(selection.area(42,00,75,01))
des.non_diggable(selection.area(46,00,75,10))
des.non_diggable(selection.area(35,03,64,19))
des.non_diggable(selection.area(35,12,75,19))
des.non_diggable(selection.area(69,00,75,19))
des.non_passwall(selection.area(00,00,75,19))

