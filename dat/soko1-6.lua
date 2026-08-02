-- NetHack sokoban soko1-6.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "premapped", "solidify", "sokoban", "noflip");
-- Steve Melenchuk <smelenchuk@gmail.com>
des.map([[
  -----                        
  |...------  -----   -------  
  |........----...| ---.....|  
 --..--.--.--.....| |.+.....|  
 |................| ---.....|--
--.....--.--....--- |.+.....+.|
|...-----.-----.-   ---.....|.|
|.....--..-----.--  |.+.....|.|
--....---.---....|  ---.....|.|
 ---..---...-....|    ------|.|
   |......-......|          |.|
   |..--...-.....|          |.|
   ------..-----------------|.|
       -......................|
       -...--------------------
       -----                   
]]);

local place = { {21,3},{21,5},{21,7} }
shuffle(place);
des.stair("down", 8, 7)
des.region({ region = { 0, 0, 30, 15 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,30,15))
des.non_passwall(selection.area(0,0,30,15))

-- Boulders
des.object("boulder", 4, 2)
des.object("boulder", 6, 2)
des.object("boulder", 8, 2)
des.object("boulder", 16, 3)
des.object("boulder", 3, 4)
des.object("boulder", 6, 4)
des.object("boulder", 12, 4)
des.object("boulder", 14, 4)
des.object("boulder", 14, 5)
des.object("boulder", 2, 6)
des.object("boulder", 3, 6)
des.object("boulder", 15, 7)
des.object("boulder", 4, 8)
des.object("boulder", 9, 8)
des.object("boulder", 15, 8)
des.object("boulder", 15, 9)
des.object("boulder", 5, 10)
des.object("boulder", 6, 10)
des.object("boulder", 12, 10)
des.object("boulder", 14, 10)
des.object("boulder", 15, 10)

-- Traps
des.trap("hole", 11, 13)
des.trap("hole", 12, 13)
des.trap("hole", 13, 13)
des.trap("hole", 14, 13)
des.trap("hole", 15, 13)
des.trap("hole", 16, 13)
des.trap("hole", 17, 13)
des.trap("hole", 18, 13)
des.trap("hole", 19, 13)
des.trap("hole", 20, 13)
des.trap("hole", 21, 13)
des.trap("hole", 22, 13)
des.trap("hole", 23, 13)
des.trap("hole", 24, 13)
des.trap("hole", 25, 13)
des.trap("hole", 26, 13)
des.trap("hole", 27, 13)
des.trap("hole", 28, 13)

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
des.door("locked", 28, 5)
des.door({ state = "closed", material = "stone", x = 22, y = 3 })
des.door({ state = "closed", material = "stone", x = 22, y = 5 })
des.door({ state = "closed", material = "stone", x = 22, y = 7 })
des.region({ region = { 23, 2, 27, 8 }, lit = 1, type = "zoo", filled = 1, irregular = 1 })

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
