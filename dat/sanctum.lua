-- NetHack gehennom sanctum.lua	$NHDT-Date: 1781994887 2026/06/20 22:34:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.6 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

-- noflip: mkgate() opens the sanctum gates at absolute coordinates
-- keyed to this map's unflipped placement (master 3.6 never flips)
des.level_flags("mazelevel", "noteleport", "hardfloor", "nommap", "noflip")
-- This is outside the main map, below, so we must do it before adding
-- that map and anchoring coordinates to it. This extends the invisible
-- barrier up to the top row, which falls outside the drawn map.
des.non_passwall(selection.area(39,00,41,00))
des.map([[
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLL--------------LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLL|............|LLLLLLLLLLLLL-------LLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLL------|............-----LLLLLLLLLF.....FLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLL|.....|................|LLLLLLLL--.....|LLLLLLLLLLLL--F---F--LLLLLLL
LLLLL----.....|................-------LL|......----LLLLLLLLL|.......|LLLLLLL
LLLLL|........|LLLLLLLLL.........|...|LL|.........|--F----F---+---..FLLLLLLL
LL-------.....|--------L.........--S--LL|.........|.....S........|..|LLLLLLL
--|....|......|.......|L............|LLLF.........---S---..-----S----LLLLLLL
|.S..|........|.......|L.........----LLL|..........|....|..|......|LLLLLLLLL
|.|..|........|.......|L.........|LLLLLL--.......----+---S---S--..FLLLLLLLLL
--|....|......|.......|L.........|LLLLLLLF.......|.............|..|LLLLLLLLL
LL-------.....|--------L.........|LLLLLLL-----+-------S---------S---LLLLLLLL
LLLLL|........|LLLLLLLLL.........|LLLLLLLLLL|...F.F......|....|....|--LLLLLL
LLLLL----.....|...............----LLLLLLLLLLF...-F-....------S-......FLLLLLL
LLLLLLLL|.....|...............|LLLLLLLLLLLLL|..........|LLLL|.....----LLLLLL
LLLLLLLL------|...........-----LLLLLLLLLLLLL--...----F--LLLLF.....|LLLLLLLLL
LLLLLLLLLLLLLL|...........|LLLLLLLLLLLLLLLLLLF...FLLLLLLLLLL|.....FLLLLLLLLL
LLLLLLLLLLLLLL-------------LLLLLLLLLLLLLLLLLL-----LLLLLLLLLL-------LLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
]]);
-- TODO-EVILHACK: master anchors the Purgatory chainbranch here
-- (BRANCH:(01,10,01,10) inside Lucifer's chamber); the levregion lands
-- with the Purgatory dungeon topology
des.region({ region={15,08, 21,11}, lit=1, type="temple", filled=2, contents = function()
                des.door({ wall = "random", state = "secret" });
end })
des.altar({ x=18, y=09, align="noalign", type="sanctum" })
des.region({ region={41,06, 48,11}, lit=0, type="morgue", filled=1, irregular=1 })
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Invisible barrier separating the left & right halves of the level
-- as well as Lucifer's chamber
des.non_passwall(selection.area(00,00,39,19))
-- Drawbridges
des.drawbridge({ x=38, y=06, dir="west", state="closed" })
des.drawbridge({ x=39, y=06, dir="east", state="closed" })
-- Doors
des.door({ state="locked", material="bronze", x=62, y=06 })
des.door({ state="closed", material="bronze", x=46, y=12 })
des.door({ state="closed", material="bronze", x=53, y=10 })
-- Secret doors
des.door({ state="secret", material="bronze", x=02, y=09 })
des.door({ state="secret", material="bronze", x=35, y=07 })
des.door({ state="secret", material="bronze", x=53, y=08 })
des.door({ state="secret", material="bronze", x=54, y=12 })
des.door({ state="secret", material="bronze", x=56, y=07 })
des.door({ state="secret", material="bronze", x=57, y=10 })
des.door({ state="secret", material="bronze", x=61, y=10 })
des.door({ state="secret", material="bronze", x=61, y=14 })
des.door({ state="secret", material="bronze", x=64, y=08 })
des.door({ state="secret", material="bronze", x=64, y=12 })
-- Some traps.
des.trap("spiked pit")
des.trap("fire")
des.trap("sleep gas")
des.trap("anti magic")
des.trap("ice")
des.trap("magic")
des.trap("spear")
des.trap("magic beam")
-- Some random objects
des.object("[")
des.object("[")
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object("*")
des.object("!")
des.object("!")
des.object("!")
des.object("!")
des.object("?")
des.object("?")
des.object("?")
des.object("?")
des.object("?")
-- Chest by the altar
des.object({ id = "crystal chest", x=17, y=09,
   contents = function()
      des.object("magic marker")
   end
})
-- Lucifer and his consorts
des.monster({ id = "Lucifer", x=04, y=09, peaceful=0 })
des.monster({ id = "balrog", x=08, y=08, name="Adramalech", asleep=1, peaceful=0 })
des.monster({ id = "balrog", x=08, y=11, name="Buer", asleep=1, peaceful=0 })
des.monster({ id = "balrog", x=11, y=04, name="Phongor", asleep=1, peaceful=0 })
des.monster({ id = "balrog", x=11, y=15, name="Zagum", asleep=1, peaceful=0 })
-- Inner sanctum guards
des.monster({ id = "salamander", x=15, y=06, peaceful=0 })
des.monster({ id = "salamander", x=23, y=06, peaceful=0 })
des.monster({ id = "salamander", x=15, y=13, peaceful=0 })
des.monster({ id = "salamander", x=23, y=13, peaceful=0 })
des.monster({ id = "lava demon", x=18, y=06, peaceful=0 })
des.monster({ id = "lava demon", x=18, y=13, peaceful=0 })
-- Some monsters.
des.monster({ id = "pit fiend", x=17, y=14, peaceful=0 })
des.monster({ id = "balrog", x=18, y=08, name="Baalphegor", peaceful=0 })
des.monster({ id = "erinys", x=17, y=04, peaceful=0 })
des.monster({ id = "marilith", x=17, y=15, peaceful=0 })
des.monster({ id = "nalfeshnee", x=27, y=09, peaceful=0 })
des.monster({ id = "alhoon", x=18, y=10, peaceful=0 })
-- Moloch's horde
des.monster({ id = "aligned cleric", x=20, y=03, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=15, y=04, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=21, y=03, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=25, y=08, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=25, y=11, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=21, y=14, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=15, y=15, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=17, y=15, align="noalign", peaceful=0 })
des.monster({ id = "aligned cleric", x=19, y=16, align="noalign", peaceful=0 })
-- A few nasties
des.monster("L")
des.monster("L")
des.monster("V")
des.monster("V")
des.monster("V")
des.monster("shambling horror")
des.monster("shambling horror")
des.monster("shambling horror")
-- Lava nasties
des.monster({ id = "salamander", x=01, y=01, peaceful=0 })
des.monster({ id = "salamander", x=74, y=01, peaceful=0 })
des.monster({ id = "salamander", x=01, y=18, peaceful=0 })
des.monster({ id = "salamander", x=74, y=18, peaceful=0 })
des.monster({ id = "salamander", x=38, y=04, peaceful=0 })
des.monster({ id = "salamander", x=38, y=08, peaceful=0 })
des.monster({ id = "lava demon", x=60, y=03, peaceful=0 })
des.monster({ id = "lava demon", x=30, y=17, peaceful=0 })
-- Exit
des.stair("up", 63,15)
-- Teleporting to this level is allowed after the invocation creates its
-- entrance.  Force arrival in that case to be on rightmost third of level.
des.teleport_region({ region = {54,1,79,18}, region_islev=1, dir="down" })
