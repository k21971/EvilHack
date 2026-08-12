-- NetHack Infidel Inf-loca.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "locate" level for the quest.
--
--	This is the edge of the Howling Forest.
--	Somewhere in here the Paladin awaits you.
--

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "mines", fg = ".", bg = "T", smoothed = true,
                 joined = true, lit = 0, walled = true })

-- the .des GEOMETRY is (49, 00); the 3.6 frame translation rendered
-- the map at absolute column 50, which x = 50 reproduces directly
--         1         2
--12345678901234567890123456789
des.map({ x = 50, y = 0, map = [[
xxxx.|            ----        
xxx..--           |..|        
x.....---         |..|        
...}}...|         -+--        
...}}}..--         #          
..}}}....|         #          
...}}....|       --S-         
.........--      |..|         
..........--     |..|         
...........|     |..+####     
...........+#####+..|   #     
...........|     |..|   #     
...........|     ----   #     
..........--            #     
.........--             #     
.........|              #     
........-|            --+--   
.........|            |...|   
x......---            |...|   
xxx.----              -----   
xxxx|                         
]], contents = function()
   des.non_diggable(selection.area(05, 00, 29, 20))

   -- Upstairs to (rest of) the underground complex.
   des.region({ region = { 23, 17, 25, 18 }, type = "ordinary" })
   des.stair("up", 24, 18)
   des.monster({ class = "k",
                 coord = selection.area(23, 17, 25, 18):rndcoord(0) })
   des.door("random", 24, 16)

   -- The last room before the Forest!
   des.door("random", 20, 09)
   des.region({ region = { 18, 07, 19, 11 }, type = "ordinary" })
   des.monster({ class = "h",
                 coord = selection.area(18, 07, 19, 11):rndcoord(0) })
   des.door("random", 17, 10)
   des.door("random", 11, 10)

   -- An ogre guards some treasure.
   des.door("locked", 19, 03)
   des.region({ region = { 19, 01, 20, 02 }, type = "ordinary" })
   des.monster("O", 19, 02)
   des.object("chest", 19, 01)
   des.object("chest", 20, 01)

   -- The lake.
   des.monster("piranha", 03, 03)
   des.monster("piranha", 04, 04)
   des.monster("piranha", 04, 05)

   -- The forest's edge.
   des.replace_terrain({ region = { 00, 00, 10, 20 }, fromterrain = ".",
                         toterrain = "T", chance = 5 })
end })

-- the .des switches to NOMAP here: everything below is placed
-- level-wide, not map-relative
des.levregion({ region = { 00, 00, 39, 20 }, region_islev = 1,
                type = "stair-down" })
local forest = selection.area(00, 00, 60, 20):filter_mapchar('.')
des.monster({ id = "werewolf", coord = forest:rndcoord(0) })
des.monster({ id = "werewolf", coord = forest:rndcoord(0) })
des.monster({ id = "werewolf", coord = forest:rndcoord(0) })
for i = 1, d(4, 4) do
   des.monster({ class = "d", coord = forest:rndcoord(0) })
end
des.monster({ class = "u", coord = forest:rndcoord(0) })
des.monster({ class = "u", coord = forest:rndcoord(0) })
if percent(65) then
   des.monster({ id = "forest centaur", coord = forest:rndcoord(0) })
end
if percent(40) then
   des.monster({ id = "woodchuck", coord = forest:rndcoord(0) })
end

-- Some traps and a little loot.
des.object()
des.object()
des.object()
des.object()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
