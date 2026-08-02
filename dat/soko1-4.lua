-- NetHack sokoban soko1-4.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "premapped", "solidify", "sokoban", "noflip");
-- "J Franklin Mentzer <wryter@aol.com>"
des.map([[
------------      
|.+......+.|      
|-|......|--------
|.+......+.......|
|-|......|-----..|
|.+......+.|  |.--
|-------------|.| 
|.....|...|...|.| 
|.............|.| 
|....------.|.|.| 
|....|...|....|.| 
|....|.......-|.| 
-----|.--..---|.| 
 |...|.....+..F.| 
 |........-|..F.| 
 |-.----.---FF-.| 
 |..............| 
 |...|...-----..| 
 ---------   ---- 
]]);
local place = { {1,1},{1,3},{1,5} }
shuffle(place);
des.stair("down", 6, 10)
des.region({ region = { 0, 0, 17, 18 }, lit = 1, type = "ordinary" })
des.non_diggable(selection.area(0,0,17,18))
des.non_passwall(selection.area(0,0,17,18))
des.object("boulder", 2, 8)
des.object("boulder", 3, 8)
des.object("boulder", 6, 8)
des.object("boulder", 10, 8)
des.object("boulder", 12, 8)
des.object("boulder", 3, 9)
des.object("boulder", 2, 10)
des.object("boulder", 3, 10)
des.object("boulder", 12, 10)
des.object("boulder", 8, 11)
des.object("boulder", 6, 12)
des.object("boulder", 7, 13)
des.object("boulder", 6, 14)
des.object("boulder", 4, 16)
des.object("boulder", 8, 16)
des.object("boulder", 9, 16)
des.trap("hole", 10, 3)
des.trap("hole", 11, 3)
des.trap("hole", 12, 3)
des.trap("hole", 13, 3)
des.trap("hole", 14, 3)
des.trap("hole", 15, 5)
des.trap("hole", 15, 6)
des.trap("hole", 15, 7)
des.trap("hole", 15, 8)
des.trap("hole", 15, 9)
des.trap("hole", 15, 10)
des.trap("hole", 15, 11)
des.trap("hole", 15, 12)
des.trap("hole", 15, 13)
des.trap("hole", 15, 14)
des.trap("hole", 15, 15)

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
des.door({ state = "closed", material = "stone", x = 2, y = 1 })
des.door("locked", 9, 1)
des.door({ state = "closed", material = "stone", x = 2, y = 3 })
des.door("locked", 9, 3)
des.door({ state = "closed", material = "stone", x = 2, y = 5 })
des.door("locked", 9, 5)
des.door("locked", 11, 13)
des.region({ region = { 3, 1, 8, 5 }, lit = 1, type = "zoo", filled = 1, irregular = 1 })

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

