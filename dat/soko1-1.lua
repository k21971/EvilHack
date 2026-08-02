-- NetHack sokoban soko1-1.lua	$NHDT-Date: 1781994887 2026/06/20 22:34:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.10 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "premapped", "sokoban", "solidify",
                "noflip");
des.map([[
--------------------------
|........................|
|.......|---------------.|
-------.------         |.|
 |...........|         |.|
 |...........|         |.|
--------.-----         |.|
|............|         |.|
|............|         |.|
-----.--------   ------|.|
 |..........|  --|.....|.|
 |..........|  |.+.....|.|
 |.........|-  |-|.....|.|
-------.----   |.+.....+.|
|........|     |-|.....|--
|........|     |.+.....|  
|...|-----     --|.....|  
-----            -------  
]]);

local place = { {16,11}, {16,13}, {16,15} };
shuffle(place);

des.stair("down", 01, 01);
des.region(selection.area(00,00,25,17),"lit");
des.non_diggable(selection.area(00,00,25,17));
des.non_passwall(selection.area(00,00,25,17));

-- Boulders
des.object("boulder", 03, 05);
des.object("boulder", 05, 05);
des.object("boulder", 07, 05);
des.object("boulder", 09, 05);
des.object("boulder", 11, 05);
--
des.object("boulder", 04, 07);
des.object("boulder", 04, 08);
des.object("boulder", 06, 07);
des.object("boulder", 09, 07);
des.object("boulder", 11, 07);
--
des.object("boulder", 03, 12);
des.object("boulder", 04, 10);
des.object("boulder", 05, 12);
des.object("boulder", 06, 10);
des.object("boulder", 07, 11);
des.object("boulder", 08, 10);
des.object("boulder", 09, 12);
--
des.object("boulder", 03, 14);

-- Traps
des.trap("hole", 08, 01);
des.trap("hole", 09, 01);
des.trap("hole", 10, 01);
des.trap("hole", 11, 01);
des.trap("hole", 12, 01);
des.trap("hole", 13, 01);
des.trap("hole", 14, 01);
des.trap("hole", 15, 01);
des.trap("hole", 16, 01);
des.trap("hole", 17, 01);
des.trap("hole", 18, 01);
des.trap("hole", 19, 01);
des.trap("hole", 20, 01);
des.trap("hole", 21, 01);
des.trap("hole", 22, 01);
des.trap("hole", 23, 01);

des.monster({ id = "giant mimic", appear_as = "obj:boulder" });
des.monster({ id = "giant mimic", appear_as = "obj:boulder" });

-- Random objects
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "=" });
des.object({ class = "/" });

-- Rewards
des.door("locked", 23, 13);
des.door({ state = "closed", material = "stone", x = 17, y = 11 });
des.door({ state = "closed", material = "stone", x = 17, y = 13 });
des.door({ state = "closed", material = "stone", x = 17, y = 15 });

des.region({ region={18,10, 22,16}, lit = 1, type = "zoo", filled = 1, irregular = 1 });

des.object({ id = "scroll of scare monster", coord = place[1], buc = "cursed" });
if percent(50) then
  des.object({ id = "bag of holding", coord = place[1], achievement = 1 });
else
  des.object({ id = "magic marker", coord = place[1], achievement = 1 });
end
des.engraving({ coord = place[1], type = "burn", text = "Elbereth" });

des.object({ id = "scroll of scare monster", coord = place[2], buc = "cursed" });
if percent(50) then
  des.object({ id = "amulet of reflection", coord = place[2], achievement = 1 });
else
  des.object({ id = "amulet of magic resistance", coord = place[2],
               achievement = 1 });
end
des.engraving({ coord = place[2], type = "burn", text = "Elbereth" });

des.object({ id = "scroll of scare monster", coord = place[3], buc = "cursed" });
if percent(50) then
  des.object({ id = "helm of speed", coord = place[3], achievement = 1 });
else
  des.object({ id = "gauntlets of protection", coord = place[3],
               achievement = 1 });
end
des.engraving({ coord = place[3], type = "burn", text = "Elbereth" });

