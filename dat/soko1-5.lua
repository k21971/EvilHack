-- NetHack sokoban soko1-5.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "premapped", "solidify", "sokoban", "noflip");
-- Steve Melenchuk <smelenchuk@gmail.com>
des.map([[
    ----                     
    -..--                    
 ----...---------------------
 |..-.......................|
--...-..------------------|.|
|.................|       |.|
|..-....-...----.-| -------.|
--.-..--...-......---.....|.|
--.---..--.-..-...|.+.....|.|
--.- -....---.-...---.....|.|
--.---..-..-....--|.+.....+.|
--.-..-.--......-----.....---
|...........-....||.+.....|  
|...-.....-......|---.....|  
-----.........-..|  -------  
    |..-..-..--..|           
    --------------           
]]);

local place = { {19,8},{19,10},{19,12} }
shuffle(place);
des.stair("down", 15, 15)
des.region({ region = { 0, 0, 28, 16 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,28,16))
des.non_passwall(selection.area(0,0,28,16))

-- Boulders
des.object("boulder", 6, 3)
des.object("boulder", 7, 3)
des.object("boulder", 7, 4)
des.object("boulder", 2, 5)
des.object("boulder", 8, 5)
des.object("boulder", 11, 5)
des.object("boulder", 5, 6)
des.object("boulder", 15, 8)
des.object("boulder", 13, 9)
des.object("boulder", 16, 9)
des.object("boulder", 13, 10)
des.object("boulder", 11, 11)
des.object("boulder", 4, 12)
des.object("boulder", 6, 12)
des.object("boulder", 9, 12)
des.object("boulder", 15, 12)
des.object("boulder", 7, 13)
des.object("boulder", 8, 13)
des.object("boulder", 9, 13)
des.object("boulder", 11, 13)
des.object("boulder", 14, 13)
des.object("boulder", 11, 14)

-- Traps
des.trap("hole", 8, 3)
des.trap("hole", 9, 3)
des.trap("hole", 10, 3)
des.trap("hole", 11, 3)
des.trap("hole", 12, 3)
des.trap("hole", 13, 3)
des.trap("hole", 14, 3)
des.trap("hole", 15, 3)
des.trap("hole", 16, 3)
des.trap("hole", 17, 3)
des.trap("hole", 18, 3)
des.trap("hole", 19, 3)
des.trap("hole", 20, 3)
des.trap("hole", 21, 3)
des.trap("hole", 22, 3)
des.trap("hole", 23, 3)
des.trap("hole", 24, 3)
des.trap("hole", 25, 3)

des.monster({ id = "giant mimic", appear_as = "obj:boulder" })
des.monster({ id = "giant mimic", appear_as = "obj:boulder" })

-- Random objects
des.object("%")
des.object("%")
des.object("%")
des.object("%")
des.object("=")
des.object("/")

-- Rewards
des.door("locked", 26, 10)
des.door({ state = "closed", material = "stone", x = 20, y = 8 })
des.door({ state = "closed", material = "stone", x = 20, y = 10 })
des.door({ state = "closed", material = "stone", x = 20, y = 12 })
des.region({ region = { 21, 7, 25, 13 }, lit = 1, type = "zoo", filled = 1, irregular = 1 })

des.object({ id = "scroll of scare monster", coord = place[1], buc = "cursed" })
if percent(50) then
  des.object({ id = "amulet of reflection", coord = place[1], achievement = 1 })
else
  des.object({ id = "amulet of magic resistance", coord = place[1], achievement = 1 })
end
des.engraving({ coord = place[1], type = "burn", text = "Elbereth" })

des.object({ id = "scroll of scare monster", coord = place[2], buc = "cursed" })
if percent(50) then
  des.object({ id = "bag of holding", coord = place[2], achievement = 1 })
else
  des.object({ id = "magic marker", coord = place[2], achievement = 1 })
end
des.engraving({ coord = place[2], type = "burn", text = "Elbereth" })

des.object({ id = "scroll of scare monster", coord = place[3], buc = "cursed" })
if percent(50) then
  des.object({ id = "helm of speed", coord = place[3], achievement = 1 })
else
  des.object({ id = "gauntlets of protection", coord = place[3], achievement = 1 })
end
des.engraving({ coord = place[3], type = "burn", text = "Elbereth" })

