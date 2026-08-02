-- NetHack mines minend-2.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.7 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- Mine end level variant 2
-- "Gnome King's/Queen's Wine Cellar" (gender is selected randomly)

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel");

des.map([[
---------------------------------------------------------------------------
|...................................................|                     |
|.|---------S--.--|...|--------------------------|..|  ..                 |
|.||---|   |.||-| |...|..........................|..|   ..                |
|.||...| |-|.|.|---...|.............................|                ..   |
|.||...|-|.....|....|-|..........................|..|.               ..   |
|.||.....|-S|..|....|............................|..|..                   |
|.||--|..|..|..|-|..|----------------------------|..|-.                   |
|.|   |..|..|....|..................................|...                  |
|.|   |..|..|----|..-----------------------------|..|....                 |
|.|---|..|--|.......|----------------------------|..|.....                |
|...........|----.--|......................|     |..|.......              |
|-----------|...|.| |------------------|.|.|-----|..|.....|..             |
|-----------|...|.|--------------------|.|..........|.....|....           |
|...............|.S......................|-------------..-----...         |
|.--------------|.|--------------------|.|.........................       |
|.................|                    |.....................|........    |
---------------------------------------------------------------------------
]]);

if percent(50) then
   des.terrain({55,14},"-")
   des.terrain({56,14},"-")
   des.terrain({61,15},"|")
   des.terrain({52,5}, "S")
   des.door("locked", 52,5)
end
if percent(50) then
   des.terrain({18,1}, "|")
   des.terrain(selection.area(7,12, 8,13), ".")
end
if percent(50) then
   des.terrain({49,4}, "|")
   des.terrain({21,5}, ".")
end
if percent(50) then
   if percent(50) then
      des.terrain({22,1}, "|")
   else
      des.terrain({50,7}, "-")
      des.terrain({51,7}, "-")
   end
end

local place = { {05,05},{22,11},{54,12},{56,02} }
shuffle(place)

-- uncontrolled arrival (via trap door, level teleport) will be in the
-- upper corridor to prevent ending up stuck in the treasure area
des.teleport_region({ region={23,03,48,06}, exclude={0,0,0,0} })

-- Dungeon Description
des.region(selection.area(23,03,48,06),"lit")
des.region(selection.area(21,06,22,06),"lit")
des.region(selection.area(14,04,14,04),"unlit")
des.region(selection.area(10,05,14,08),"unlit")
des.region(selection.area(10,09,11,09),"unlit")
des.region(selection.area(15,08,16,08),"unlit")
-- Secret doors
des.door("locked",12,02)
des.door({ state="locked", material="stone", x=11, y=06 })
-- Forge
des.feature("forge", 14, 13)
-- Stairs
des.stair("up", 36,04)
-- Non diggable walls
des.non_diggable(selection.area(00,00,52,17))
des.non_diggable(selection.area(53,00,74,00))
des.non_diggable(selection.area(53,17,74,17))
des.non_diggable(selection.area(74,01,74,16))
des.non_diggable(selection.area(53,07,55,07))
des.non_diggable(selection.area(53,14,61,14))
-- The Gnome King's/Queen's wine cellar.
-- the Trespassers sign used to be a long-running joke...
des.engraving({12,03}, "engrave",
	      "You are now entering the royal wine cellar.")
des.engraving({12,04}, "engrave", "Trespassers will be persecuted!")
-- hired mercenaries, courtesy of the Yendorian Army
des.monster({ id = "sergeant", x=14, y=04, peaceful = 0 })
des.monster({ id = "soldier", x=14, y=05, peaceful = 0 })
des.monster({ id = "gnome royal", x=14, y=08, peaceful = 0 })
des.object("potion of booze", 10, 07)
des.object("potion of booze", 10, 07)
des.object("!", 10, 07)
des.object("potion of booze", 10, 08)
des.object("potion of booze", 10, 08)
des.object("!", 10, 08)
des.engraving({10,09}, "burn", "Elbereth")
des.object("potion of booze", 10, 09)
des.object("potion of booze", 10, 09)
des.object("potion of object detection", 10, 09)

-- magic key is buried in a chest
des.engraving({ coord=place[1], type="engrave", text="Property of Croesus" })
des.object({ id = "chest", coord = place[1], buried = 1, trapped = 0, locked = 1,
             contents = function()
                des.object("magic key")
             end })
-- Objects
-- The Treasure chamber...
des.object("diamond", 69, 04)
des.object("*", 69, 04)
des.object("diamond", 69, 04)
des.object("*", 69, 04)
des.object("emerald", 70, 04)
des.object("*", 70, 04)
des.object("emerald", 70, 04)
des.object("*", 70, 04)
des.object("emerald", 69, 05)
des.object("*", 69, 05)
des.object("ruby", 69, 05)
des.object("*", 69, 05)
des.object("ruby", 70, 05)
des.object("amethyst", 70, 05)
des.object("*", 70, 05)
des.object("amethyst", 70, 05)
des.object({ id="luckstone", x=70, y=05, achievement=1 });
-- Scattered gems...
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("(")
des.object("(")
des.object()
des.object()
des.object()
-- Random traps
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
-- Random monsters.
des.monster("gnome royal")
des.monster("gnome noble")
des.monster("gnome noble")
des.monster("gnome noble")
des.monster("gnomish wizard")
des.monster("gnomish wizard")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("rock gnome")
des.monster("hobbit")
des.monster("hobbit pickpocket")
des.monster("mountain dwarf")
des.monster("mountain dwarf")
des.monster("mountain dwarf")
des.monster("h")
