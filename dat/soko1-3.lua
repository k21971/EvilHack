-- NetHack sokoban soko1-3.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "solidify", "sokoban", "noflip");
-- "J Franklin Mentzer <wryter@aol.com>"
des.map([[
            --------- 
            |.......| 
 -------    |..----+--
 |.....-------.|.....|
-----........|.|.....|
|...|-....--.|.|.....|
|...|..--....|.|.....|
|...|..-----.|.|.....|
--.........|.|.|+----|
 |.....---.|.|.F...+.|
 |.|.....|.|.|.|...|-|
 |.|.|.|...|.|.F...+.|
--.---...|.|.|.|...|-|
|.....----.|.|.F...+.|
|..............|------
----....|......|      
   |....|..-----      
   ---------          
]]);
local place = { {20,9},{20,11},{20,13} }
shuffle(place);
des.stair("down", 2, 3)
des.region({ region = { 0, 0, 21, 17 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,21,17))
des.non_passwall(selection.area(0,0,21,17))
des.object("boulder", 6, 4)
des.object("boulder", 2, 6)
des.object("boulder", 2, 7)
des.object("boulder", 3, 8)
des.object("boulder", 6, 8)
des.object("boulder", 3, 9)
des.object("boulder", 5, 9)
des.object("boulder", 6, 10)
des.object("boulder", 6, 11)
des.object("boulder", 3, 13)
des.object("boulder", 5, 14)
des.object("boulder", 10, 14)
des.object("boulder", 4, 15)
des.object("boulder", 7, 15)
des.object("boulder", 10, 15)
des.trap("hole", 15, 1)
des.trap("hole", 16, 1)
des.trap("hole", 17, 1)
des.trap("hole", 18, 1)
des.trap("hole", 14, 3)
des.trap("hole", 14, 4)
des.trap("hole", 14, 5)
des.trap("hole", 14, 6)
des.trap("hole", 14, 7)
des.trap("hole", 14, 8)
des.trap("hole", 14, 9)
des.trap("hole", 14, 10)
des.trap("hole", 14, 11)
des.trap("hole", 14, 12)
des.trap("hole", 14, 13)

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
des.door("locked", 19, 2)
des.door("locked", 16, 8)
des.door({ state = "closed", material = "stone", x = 19, y = 9 })
des.door({ state = "closed", material = "stone", x = 19, y = 11 })
des.door({ state = "closed", material = "stone", x = 19, y = 13 })
des.region({ region = { 16, 3, 19, 7 }, lit = 1, type = "zoo", filled = 1, irregular = 1 })

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

