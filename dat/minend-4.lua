-- NetHack mines minend-4.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- Mine end level variant 4
-- "Orc Temple" by Khor
-- In this variant of the mines' end a group of orcs have found a randomly
-- generated figurine of a demon and worship it. Slightly modified from
-- the original.

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "nommap")

des.map([[
                                                                   |-----|  
                                             -------        ----- |-LLLLL-| 
                         ---------------     |.....|        |...||-LL...LL-|
                        -|..|..........|     |.....|        |...||LL.....LL|
                       -|...F..........|     |.....|        |...||L.......L|
         |-------|    -|....|-+------+-|    ----+----       |LLL||L.......L|
        |-.......-|  -|.....F..........|----|.......|---------S---L.......L|
 -------|.........|--|.....--...............F.......F.....||.....|LL.....LL|
-|..........}}}...........-|..............................--.....|-LL...LL-|
|...........}{}...........+...............................+......||-LL.LL-| 
-|..........}}}...........-|..............................--.....| |--S--|  
 -------|.........|--|.....--...............F.......F.....||.....|    .     
        |-.......-|  -|.....F..........|----|.......|---------S---          
         |-------|    -|....|-+------+-|    ----+----         .             
                       -|...F..........|     |.....|                        
                        -|..|..........|     |.....|                        
                         ---------------     |.....|                        
                                             -------                     ..}
                                                                         .}}
                                                                        .}}{
]]);

local stones = { {0,18},{0,12},{2,18},{12,14},{22,14},
                 {16,16},{72,18},{52,14},{44,14} }
shuffle(stones)

des.region(selection.area(0,0,75,19),"lit")
des.teleport_region({ region={2,8,7,10}, exclude={0,0,0,0} })
des.non_diggable(selection.area(0,0,75,19))
des.stair("up", 1, 9)

-- SOME ORCS (The religious kind)
-- Hostile to anyone, regardless of race, who disturbs their sacred temple
des.monster({ id = "goblin outrider", peaceful = 0 })
des.monster({ id = "goblin-captain", peaceful = 0 })
des.monster({ id = "goblin outrider", peaceful = 0 })
des.monster({ id = "hobgoblin", peaceful = 0 })
for i = 1, 8 do
  des.monster({ id = "hill orc", peaceful = 0 })
end
for i = 1, 8 do
  des.monster({ id = "Mordor orc", peaceful = 0 })
end
for i = 1, 10 do
  des.monster({ class = "o", peaceful = 0 })
end

-- SECRET CAVE
des.mazewalk({ x=62, y=15, dir="south" })
des.altar({ x=70, y=5, align="noalign", type="shrine" })
des.door({ state="closed", material="stone", x=62, y=12 })
des.door({ state="closed", material="stone", x=70, y=10 })
des.object({ id = "scare monster", x=70, y=5, buc="cursed" })
des.engraving({70,5}, "burn", "Moloch")
des.object({ id = "figurine", x=70, y=5, buc="cursed", montype="balrog" })
-- random placement of gems/stones
-- one location will always be empty
des.object({ id="luckstone", coord=stones[1], achievement=1 })
des.object("flint", stones[2])
des.object({ class = "*", coord = stones[3] })
des.object({ class = "*", coord = stones[4] })
des.object({ class = "*", coord = stones[5] })
des.object({ class = "*", coord = stones[6] })
des.object({ class = "*", coord = stones[7] })
des.object({ class = "*", coord = stones[8] })

-- magic key is buried in a chest
des.engraving({ coord=stones[9], type="engrave", text="Do not dig here" })
des.object({ id = "chest", coord = stones[9], buried = 1, trapped = 0, locked = 1,
             contents = function()
                des.object("magic key")
             end })

-- shamans guarding the inner temple/figurine
des.monster({ id = "orc shaman", x=69, y=3, asleep=1, peaceful=0 })
des.monster({ id = "orc shaman", x=71, y=3, asleep=1, peaceful=0 })
des.monster({ id = "orc shaman", x=68, y=4, asleep=1, peaceful=0 })
des.monster({ id = "orc shaman", x=72, y=4, asleep=1, peaceful=0 })
des.monster({ id = "orc shaman", x=68, y=6, asleep=1, peaceful=0 })
des.monster({ id = "orc shaman", x=72, y=6, asleep=1, peaceful=0 })
des.monster({ id = "goblin shaman", x=69, y=7, asleep=1, peaceful=0 })
des.monster({ id = "goblin shaman", x=71, y=7, asleep=1, peaceful=0 })

-- TEMPLE
des.engraving({55,9}, "burn",
	      "Blessed are the destroyers of false hope, for they are the true Messiahs.")
des.monster({ id = "orc shaman", x=55, y=9, asleep=1 })
des.engraving({48,3}, "burn",
	      "Why should you not hate your enemies. If you love them does that not place you at their mercy?")
des.monster({ id = "orc shaman", x=48, y=3, asleep=1 })
des.engraving({48,15}, "burn",
	      "Cursed are the god-adorers, for they shall be shorn sheep!")
des.monster({ id = "orc shaman", x=48, y=15, asleep=1 })
des.engraving({25,9}, "burn",
	      "Dagul protects our doors. Dagul protects our souls.")
des.monster({ id = "goblin shaman", x=25, y=9, asleep=1 })
des.object({ id = "statue", x=56, y=8, montype="orc shaman" })
des.object({ id = "statue", x=56, y=10, montype="orc-captain" })
des.object({ id = "statue", x=50, y=3, montype="hill orc" })
des.object({ id = "statue", x=46, y=3, montype="hill orc" })
des.object({ id = "statue", x=50, y=15, montype="Mordor orc" })
des.object({ id = "statue", x=46, y=15, montype="Mordor orc" })
des.object({ id = "statue", x=29, y=3, montype="Mordor orc" })
des.object({ id = "statue", x=32, y=3, montype="hill orc" })
des.object({ id = "statue", x=35, y=3, montype="goblin-captain" })
des.object({ id = "statue", x=38, y=3, montype="orc shaman" })
des.object({ id = "statue", x=29, y=15, montype="Mordor orc" })
des.object({ id = "statue", x=32, y=15, montype="hill orc" })
des.object({ id = "statue", x=35, y=15, montype="orc-captain" })
des.object({ id = "statue", x=38, y=15, montype="orc shaman" })
des.object({ id = "iron safe", x=61, y=2 })
des.object({ id = "iron safe", x=63, y=2 })
des.object({ id = "chest", x=48, y=2, trapped=1 })
des.object({ id = "chest", x=48, y=16, trapped=1 })
des.altar({ x=56, y=9, align="noalign", type="shrine" })
-- A hidden elven temple from the days of old before this
-- site had been overrun by orcs. Chance that some elves
-- are barracaded inside.
des.altar({ x=62, y=2, align="noncoaligned", type="shrine" })
des.engraving({62,2}, "burn", "Elbereth")
if percent(33) then
  des.monster({ id = "elven wizard", x=61, y=3, asleep=1 })
  des.monster({ id = "green-elf", x=63, y=3, asleep=1 })
end
des.feature("forge", 48, 9)
des.door("closed",58,9)
des.door("closed",26,9)
des.door("closed",30,5)
des.door("closed",37,5)
des.door("closed",48,5)
des.door({ state="locked", material="stone", x=62, y=6 })
des.door("closed",30,13)
des.door("closed",37,13)
des.door("closed",48,13)
-- Wake the shamans if anyone tries to take the figurine:
des.trap("board",69,4)
des.trap("board",69,5)
des.trap("board",69,6)
des.trap("board",70,4)
des.trap("board",70,6)
des.trap("board",71,4)
des.trap("board",71,5)
des.trap("board",71,6)
-- Random traps
des.trap("spear")
des.trap("board")
