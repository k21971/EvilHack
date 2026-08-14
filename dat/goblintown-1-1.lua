-- NetHack gtown goblintown-1-1.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2022 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	Goblin Town - the player must defeat the Goblin King
--	to open up access to Mine Town and Mines' End in the
--	Gnomish Mines.
--
-- Goblin Town variant 1
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify",
                "shortsighted")


--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
..... ..                                                         .----------
....t..                                                         ..|........|
......       -------            .tt..   ...                     t.|........|
.t...        |.....|    .............. F..                     ...|..|...\.|
 ....        |.....|.....     ..  .....FF.                      .....|.....|
    ..       |.....+.        ..     .......         ...          ....----S--
     ..      -------        ..        .t....       ..t..        ...t   ...  
      ..                   ..             ................     ..       .   
       ..                 ..                         ... ..   ..        .   
      ....             ....                           .   .....         .   
     .........       ..t....                          .                ..   
     ......................tt                         .             .. .    
      .t..           .......                         ...           F....    
       ..             .......                      ..t...    ..........     
                       ...........                ........  ..     F..      
                     ........    ..       ............tt.....               
                      FFFF.F      ..........        ......t..               
                      ......                                                
                       ...                                                  
                                                                            
                                                                            
]] });
des.teleport_region({ region = { 00, 00, 04, 03 } })
des.levregion({ region = {01,01,01,01}, type="branch" })
des.region({ region = { 00, 00, 75, 19 }, lit = 0, type = "ordinary" })
des.region({ region = { 22, 17, 27, 18 }, lit = 1, type = "ordinary" })
des.region({ region = { 40, 02, 42, 03 }, lit = 1, type = "ordinary" })
des.region({ region = { 68, 11, 69, 14 }, lit = 1, type = "ordinary" })
des.region({ region = { 71, 01, 74, 04 }, lit = 1, type = "ordinary" })

-- random trap door that leads to Gollum's Cave
local gollum_cave = { {25,18},{42,02},{70,13},{07,00},{64,01} }
shuffle(gollum_cave);

-- random other traps
local traps = { {06,06},{07,08},{19,11},{61,09},
           {62,13},{72,08},{38,16},{46,15},
           {49,07},{29,06} }
shuffle(traps);

-- Shop (maybe?)
if percent(50) then
   if percent(50) then
      des.region({ region = { 14, 03, 18, 05 }, lit = 1, type = "shop", filled = 1 })
   else
      des.region({ region = { 14, 03, 18, 05 }, lit = 1, type = "armor shop", filled = 1 })
   end
end

-- Door to jail cells?
if percent(35) then
   des.terrain(26, 16, "F")
else
   des.door("locked", 26, 16)
end
if percent(50) then
   des.terrain(41, 04, "F")
else
   des.door("locked", 41, 04)
end
if percent(50) then
   des.terrain(67, 13, "F")
else
   des.door("locked", 67, 13)
end

-- Forge
des.feature("forge", 24, 12)

-- Magic chest
des.feature("magic chest", 74, 03)

-- Various objects
des.object({ id = "chest", trapped = 0, coord = { 23, 18 }, contents = function()
   des.object({ id = "novel", buc = "uncursed", name = "The Blacksmith's Cookbook" })
   des.object("blacksmith hammer")
   if percent(50) then
      des.object({ id = "acid", buc = "uncursed" })
   end
end })
des.object("boulder", 28, 03)
des.object("boulder", 71, 11)

-- Doors
des.door("locked", 19, 05)
des.door("locked", 73, 05)

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
des.monster({ id = "Woodland-elf", coord = { 23, 17 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 24, 18 }, peaceful = 1 })
des.monster({ id = "Woodland-elf", coord = { 25, 17 }, peaceful = 1 })
des.monster({ id = "mountain dwarf", coord = { 27, 17 }, peaceful = 1 })
des.monster({ id = "Green-elf", coord = { 40, 03 }, peaceful = 1 })
des.monster({ id = "gnome noble", coord = { 41, 02 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 42, 02 }, peaceful = 1 })
des.monster({ id = "Grey-elf", coord = { 69, 12 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 68, 14 }, peaceful = 1 })
des.monster({ id = "rock gnome", coord = { 69, 14 }, peaceful = 1 })

-- Residents of Goblin Town
des.monster({ id = "goblin", coord = { 07, 11 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 08, 12 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 24, 13 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 25, 10 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 27, 12 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 35, 03 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 36, 04 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 42, 05 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 52, 06 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 52, 13 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 54, 05 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 54, 14 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 56, 15 }, peaceful = 0 })
des.monster({ id = "goblin", coord = { 67, 05 }, peaceful = 0 })
des.monster({ id = "goblin shaman", coord = { 64, 03 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 26, 12 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 38, 06 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 55, 16 }, peaceful = 0 })
des.monster({ id = "hobgoblin", coord = { 65, 05 }, peaceful = 0 })
des.monster({ id = "bugbear", coord = { 64, 13 }, peaceful = 0 })

-- The Goblin King and his treasure
des.monster({ id = "Goblin King", coord = { 73, 03 }, peaceful = 0 })

des.object({ id = "chest", coord = { 70, 04 }, contents = function()
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
des.non_diggable(selection.area(00,00,38,15))
des.non_diggable(selection.area(00,00,21,19))
des.non_diggable(selection.area(00,00,75,02))
des.non_diggable(selection.area(42,00,75,11))
des.non_diggable(selection.area(28,05,66,19))
des.non_diggable(selection.area(00,17,75,19))
des.non_diggable(selection.area(68,00,75,19))
des.non_passwall(selection.area(00,00,75,19))

