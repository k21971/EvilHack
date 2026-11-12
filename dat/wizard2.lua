-- $NHDT-Header: NetHack 5.0 wizard2.lua 2026/10/27_14:28:17 NetHack-5.0:1.6 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--
-- The middle wizard level.
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "nommap")
--         1         2         3
--123456789012345678901234567890123456789
des.map([[
          -------------------          
       ----.................----       
    ----.....-------------.....----    
  ---.....----...........----.....---  
  ||....---.................---....||  
  |...---.|..--------------------...|  
--|..||...|--|pppppS.FL.........||..|--
|.F..|....|...ppppp|-|L..........|..F.|
|.|..|....|...pppppS.FL..........|..F.|
|.F..|....|...ppppp|-|L..........|..F.|
--|..||...|--|pppppS.FL.........||..|--
  |...---.|..--------------------...|  
  ||....---.................---....||  
  ---.....----...........----.....---  
    ----.....-------------.....----    
       ----.................----       
          -------------------          
                                       
]]);
des.teleport_region({ region = { 1, 6, 4, 10 }, exclude = { 0, 0, 0, 0 } })
-- entire tower in a region, constrains monster migration
des.region({ region = { 0, 0, 38, 16 }, lit = 0, type = "ordinary",
             arrival_room = true })

-- random wandering monster locations
local place = { {7,4},{19,1},{19,4},{31,4},
                {31,12},{19,12},{19,15},{7,12},
                {24,7},{24,9},{8,8} }
shuffle(place);

-- various captive monsters
local bars = { {20,6},{20,8},{20,10},{37,8} }
shuffle(bars);

-- stairs
des.ladder("up", 11, 8)
des.ladder("down", 1, 8)

-- random corridor path / secret doors
if percent(50) then
   des.terrain(3, 5, "-")
   des.terrain(4, 5, "-")
   des.terrain(5, 5, "-")
   des.door({ state = "secret", material = "iron", x = 7, y = 5 })
else
   des.terrain(3, 11, "-")
   des.terrain(4, 11, "-")
   des.terrain(5, 11, "-")
   des.door({ state = "secret", material = "iron", x = 7, y = 11 })
end

if percent(50) then
   des.door({ state = "secret", material = "iron", x = 18, y = 11 })
   des.door({ state = "secret", material = "iron", x = 10, y = 5 })
else
   des.door({ state = "secret", material = "iron", x = 18, y = 5 })
   des.door({ state = "secret", material = "iron", x = 10, y = 11 })
end

-- doors
des.door({ state = "locked", material = "iron", x = 2, y = 8 })
des.door({ state = "locked", material = "iron", x = 24, y = 5 })
des.door({ state = "locked", material = "iron", x = 24, y = 11 })
des.door({ state = "secret", material = "iron", x = 19, y = 6 })
des.door({ state = "secret", material = "iron", x = 19, y = 8 })
des.door({ state = "secret", material = "iron", x = 19, y = 10 })

-- captive monsters
des.monster({ id = "Angel", coord = bars[1] })
des.monster({ id = "beholder", coord = bars[2], peaceful = 0 })
des.monster({ class = "D", coord = bars[3], peaceful = 0 })
des.monster({ class = "D", coord = bars[4], peaceful = 0 })

-- roaming monsters
des.monster({ class = "D", coord = place[1], peaceful = 0 })
des.monster({ class = "&", coord = place[2], peaceful = 0 })
des.monster({ class = "J", coord = place[3], peaceful = 0 })
des.monster({ class = "L", coord = place[4], peaceful = 0 })
des.monster({ class = "T", coord = place[5], peaceful = 0 })
des.monster({ class = "N", coord = place[6], peaceful = 0 })
des.monster({ id = "drow cleric", coord = place[7], peaceful = 0 })
des.monster({ id = "elven wizard", coord = place[8], peaceful = 0 })
des.monster({ id = "hill giant shaman", coord = place[9], peaceful = 0 })
des.monster({ id = "elven wizard", coord = place[10], peaceful = 0 })
des.monster({ id = "drow cleric", coord = place[11], peaceful = 0 })

-- wizard apprectices
des.monster({ id = "apprentice", coord = { 29, 7 }, peaceful = 0 })
des.monster({ id = "apprentice", coord = { 29, 9 }, peaceful = 0 })
des.monster({ id = "apprentice", coord = { 30, 7 }, peaceful = 0 })
des.monster({ id = "apprentice", coord = { 30, 9 }, peaceful = 0 })

-- mind flayer nursery
local nursery = { {14,6},{14,8},{14,10},{15,7},{15,9},{16,6},
                  {16,10},{17,7},{17,9},{18,6},{18,8},{18,10} }
for i = 1, #nursery do
   des.monster({ id = "mind flayer larva", coord = nursery[i],
                 peaceful = 0 })
end
des.monster({ id = "apprentice", coord = { 16, 7 }, asleep = 1,
              peaceful = 0 })
des.monster({ id = "apprentice", coord = { 16, 9 }, asleep = 1,
              peaceful = 0 })
if percent(50) then
   des.monster({ id = "neothelid", coord = { 16, 8 }, peaceful = 0 })
end

-- loot
des.object("crystal chest", 20, 6)
des.object("crystal chest", 20, 8)
des.object("crystal chest", 20, 10)

-- traps
des.trap({ coord = { 12, 7 } })
des.trap({ coord = { 12, 8 } })
des.trap({ coord = { 12, 9 } })
des.trap({ coord = { 24, 6 } })
des.trap({ coord = { 24, 10 } })

-- none shall pass
des.non_diggable(selection.area(0, 0, 38, 16))
des.non_passwall(selection.area(0, 0, 38, 16))
