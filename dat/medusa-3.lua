-- NetHack medusa medusa-3.lua	$NHDT-Date: 1781994883 2026/06/20 22:34:43 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.10 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1990, 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel", "shortsighted")
--
-- Here you disturb ravens nesting in the trees.
--
des.map([[
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
}}}}}}}}}}p}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}p}}}}}}}}}}}}}}}}}}}}}}}}}}}}
}}}}}}}}T..Tp}}}}}}}}}}}}}}}}}}}}pp}}}}}}}}p}}}.pp}}}}}}}p}}}}}pppppp}}}}}}}
}}}}}}pp.....Tp}}}}}}}}}}}pp}}}}..T.}}}}}}p..T...Tpp}}pppT.p}}p.-----.p}}}}}
}}}ppp-----...p}}}}}}}}}}pT.p}}}}}...}}}}}p....T..}}}}}pp....T..|...|.T.p}}}
}}}pT.|...|.ppTp}}}}}}}pT.....p}}}}..Tpp}}p}}}p}}ppp}}}}}pT.....+...|..p}}}}
}}}}p.|...|p}}p}}}}}p...p}}}Tp}}}}p...p}}}}}}pT}}}}}}}}}}}}}p.T.|...|p}}}}}}
}}}}}p|...|p}}}}}}p.T.p}}}}}}}}}}}}}Tp}}}}}}}}pp}}}}}}}}}}}p....-----p}}}}}}
}}}}}p--+--.p}}}}}}ppp}}}}}}}}}}}}}}}}}}}T.}}}}}}}}}}}}}}}}.T.}pppp...p}}}}}
}}}}}p.....p}}}}}}pp}}}}}}}}}p}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}p}}}p}}.Tp}}}}}}
}}pT...Tppp}}}}T}}}}}}}}}}}p..p}}}}}}}}}}T}}}}}pT}}ppp}}}}}}}}}}}}}}ppp}}}}}
}}}pppT}}}}}}}pp}}}}}}}}}}}.T..p}}}}}}}}pTp}pT.....T..pp}}}}}}}}}}}}}p}}}}}}
}}}}}}}}}}}}}}}pppp}}}}}}}ppp}}p}}}}}}}}}}p...........T.p}}}}}.T.}}}}}}}}}}}
}}}}}}}}}}}}}}}}p.Tpp}}}}}}}}}}}}}}..}}}}}p.------+--...Tp}}}p..p}}}}}}}}}}}
}}}}p}pp}}}}}}}pT....p}}}}}}}}}}}p.T.}}}}pT.|...|...|.ppp}}}}}p}}}}}p.p}}}}}
}}}.T.}ppp}pp}}}}T.Tp}}}}}}p}}}}}}}pppp}}p..|...+...|p}}}}}}}}}}}}}p.T.pp}}}
}}}}..}}}p...p}}p.p}}}}}}}p..}}}}}}}}}}}}}T.|...|...|}}}}}}}}}}}pp..T.p}}}}}
}}}}}pp}}}pT.p}}}p}}}}}}}}pT.p}}}}}}}}}}}}}}---------}}}}}}}}}}}}}pppp}}}}}}
}}}}}}}}}}}pp}}}}}}}}}}}}}}}p}}}}}}}}}}}}}}}}}T..T}}}}}}}}}}}}}}}}}}}}}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
]]);

local place = selection.new();
-- each of these spots are inside a distinct room
place:set(08,06);
place:set(66,05);
place:set(46,15);

-- location of Medusa and downstairs and Perseus's statue
local medloc = place:rndcoord(1,1);
-- specific location for some other statue in a different downstairs-eligible
-- room, to prevent object detection from becoming a trivial way to pinpoint
-- Medusa's location
-- [usefulness depends on future STATUE->dknown changes in nethack's core]
local altloc = place:rndcoord(1,1);
-- location of a fountain, in the remaining of three downstairs-eligible rooms
local othloc = place:rndcoord(1,1);
-- once here, all three points set in 'place' have been used up

des.region(selection.area(00,00,74,19),"lit")
-- fixup_special hack: the first room defined on a Medusa level gets some
-- leaderboard statues, use arrival_room to force it to be a room even though
-- monsters won't arrive within it
des.region({ region={49,14, 51,16}, lit=-1, type="ordinary", arrival_room=true });
des.region(selection.area(07,05,09,07),"unlit")
des.region(selection.area(65,04,67,06),"unlit")
des.region(selection.area(45,14,47,16),"unlit")
-- Non diggable walls
-- 4th room has diggable walls as Medusa is never placed there
des.non_diggable(selection.area(06,04,10,08))
des.non_diggable(selection.area(64,03,68,07))
des.non_diggable(selection.area(44,13,48,17))
-- All places are accessible also with jumping, so don't bother
-- restricting the placement when teleporting from levels below this.
des.teleport_region({ region = {33,02,38,07}, dir="down" })
des.levregion({ region = {32,01,39,07}, type="stair-up" });

-- place the downstairs at the same spot where Medusa will be placed
des.stair("down", medloc);
--
des.door("locked",08,08)
des.door("locked",64,05)
des.door("random",50,13)
des.door("locked",48,15)
--
-- in one of the three designated rooms, but not the one with Medusa plus
-- downstairs and also not 'altloc' where a random statue will be placed
des.feature("fountain", othloc);
--
-- same spot as Medusa plus downstairs
des.object({ id="statue", coord=medloc, buc="uncursed",
                      montype="knight", historic=1, male=1,name="Perseus",
                      contents = function()
                         if percent(75) then
                            des.object({ id = "shield of reflection", buc="cursed", spe=0 })
                         end
                         if percent(25) then
                            des.object({ id = "levitation boots", spe=0 })
                         end
                         if percent(50) then
                            des.object({ id = "saber", buc="blessed", spe=2 })
                         end
                         if percent(50) then
                            des.object("sack")
                         end
                      end
});
--
-- first random statue is in one of the three designated rooms but not the
-- one with Medusa plus downstairs or the one with the fountain
des.object({ id = "statue", coord=altloc, contents=0 })
des.object({ id = "statue", contents=0 })
des.object({ id = "statue", contents=0 })
des.object({ id = "statue", contents=0 })
des.object({ id = "statue", contents=0 })
des.object({ id = "statue", contents=0 })
des.object({ id = "statue", contents=0 })

for i=1,8 do
   des.object()
end
des.object("scroll of blank paper",48,18)
des.object("scroll of blank paper",48,18)
--
des.trap("rust")
des.trap("rust")
des.trap("board")
des.trap("board")
des.trap()
--
-- place Medusa before placing other monsters so that they won't be able to
-- unintentionally steal her spot on the downstairs
des.monster({ id = "Medusa", coord=medloc, asleep=1 })
des.monster("giant eel")
des.monster("giant eel")
des.monster("jellyfish")
des.monster("jellyfish")
des.monster("wood nymph")
des.monster("wood nymph")
des.monster("water nymph")
des.monster("water nymph")
des.monster({ id = "giant anaconda", x=50, y=15, asleep=1 })

for i=1,30 do
   des.monster({ id = "raven", peaceful = 0 })
end

--#medusa-3.lua
