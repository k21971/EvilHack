-- NetHack Infidel Inf-strt.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your class leader, the Archbishop of Moloch,
--	and receive an invitation to a duel with the Paladin.
--

des.level_flags("mazelevel", "noteleport", "hardfloor")
des.level_init({ style = "solidfill", fg = " " })

--         1         2         3         4         5         6
--1234567890123456789012345678901234567890123456789012345678901
des.map({ halign = "right", valign = "center", map = [[
                                    |.........................
                                    |.........................
                  -----             --........................
   ----         ---...-----         --........................
   |..|        --.........|         |.........................
   |..|        |..........|        --.........................
   -+--      ---..........----     |..........................
    #      ---...............|     --.........................
    #      |.................---    |.........................
 ---S-   ---...................|    |-........................
 |...|   |.....................+####S.........................
 |...+###+.....................|    |.........................
 |...|   |....................--    |.........................
 -----   ----.................|    --.........................
            |.................|    |..........................
     ---- ##S...............---   --..........................
     |..| # ---............--     |...........................
     |..+##   |........-----      --..........................
     |..|     ----------           |..........................
     |..|                          |-.........................
     ----                          |..........................
]] })
des.non_diggable(selection.area(00, 00, 37, 20))

-- You arrive at a sparsely wooded cliffside.
des.replace_terrain({ region = { 35, 00, 61, 20 }, fromterrain = ".",
                      toterrain = "T", chance = 1 })
des.levregion({ region = { 38, 00, 61, 20 }, type = "branch" })
local outside = selection.area(35, 0, 61, 20):filter_mapchar('.')
for i = 1, d(3, 4) do
   des.monster({ class = "d", coord = outside:rndcoord(0) })
end
if percent(20) then
   des.monster({ id = "black unicorn", coord = outside:rndcoord(0) })
end

-- The Hidden Temple is behind a secret door.
-- The altar is coaligned, but haunted,
-- presumably having been forcibly converted in the past.
des.trap("dart", 37, 10)
des.door("locked", 31, 10)
des.region({ region = { 20, 10, 20, 10 }, lit = 1, type = "temple",
             irregular = 1, filled = 2 })
des.altar({ coord = { 20, 10 }, align = "coaligned", type = "altar" })
-- Magic chest
des.feature("magic chest", 18, 10)

-- Archbishop and his entourage
des.monster("Archbishop of Moloch", 20, 10)
des.monster("cultist", 20, 07)
des.monster("cultist", 23, 08)
des.monster("cultist", 24, 10)
des.monster("cultist", 23, 12)
des.monster("cultist", 20, 13)
des.monster("cultist", 17, 12)
des.monster("cultist", 16, 10)
des.monster("cultist", 17, 08)

-- Some "decorations".
local around_altar = selection.ellipse(20, 10, 4, 3)
des.object({ id = "corpse", montype = "elf",
             coord = around_altar:rndcoord(0) })
des.object({ id = "corpse", montype = "elf",
             coord = around_altar:rndcoord(0) })
des.object({ id = "corpse", montype = "elf",
             coord = around_altar:rndcoord(0) })
des.object({ id = "corpse", montype = "human",
             coord = around_altar:rndcoord(0) })
des.object({ id = "corpse", montype = "human",
             coord = around_altar:rndcoord(0) })
local statues = { {20,04}, {24,05}, {27,08}, {28,13},
                  {25,15}, {19,16}, {14,14}, {13,09} }
shuffle(statues)
des.object({ id = "statue", montype = "horned devil", coord = statues[1],
             contents = 0 })
des.object({ id = "statue", montype = "barbed devil", coord = statues[2],
             contents = 0 })
des.object({ id = "statue", montype = "marilith", coord = statues[3],
             contents = 0 })
des.object({ id = "statue", montype = "vrock", coord = statues[4],
             contents = 0 })
des.object({ id = "statue", montype = "hezrou", coord = statues[5],
             contents = 0 })
des.object({ id = "statue", montype = "bone devil", coord = statues[6],
             contents = 0 })
des.object({ id = "statue", montype = "ice devil", coord = statues[7],
             contents = 0 })
des.object({ id = "statue", montype = "pit fiend", coord = statues[8],
             contents = 0 })

-- Entrance to the underground complex leading to the Howling Forest.
des.door("closed", 09, 11)
des.door("closed", 05, 11)
des.region(selection.area(02, 10, 04, 12), "lit")
des.stair("down", 03, 11)

-- Unholy water supply.
des.door("locked", 04, 06)
des.region(selection.area(04, 04, 05, 05), "lit")
des.object({ id = "water", buc = "cursed", quantity = d(2),
             coord = { 04, 04 } })
des.object({ id = "water", buc = "cursed", quantity = d(2),
             coord = { 05, 04 } })

-- A small cache of valuables.
des.door("locked", 08, 17)
des.region(selection.area(06, 16, 07, 19), "lit")
des.object({ coord = { 06, 16 } })
des.object({ coord = { 06, 17 } })
des.object("axe", 06, 18)
des.object({ coord = { 06, 19 } })

-- Traps
des.trap(outside:rndcoord(1))
des.trap(outside:rndcoord(1))
des.trap(outside:rndcoord(1))
