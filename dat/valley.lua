-- NetHack gehennom valley.lua	$NHDT-Date: 1781994890 2026/06/20 22:34:50 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.6 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "nommap", "temperate");

des.map([[
----------------------------------------------------------------------------
|..........|}}}|  |.....-|      |................|   |...............| |...|
|..........|}..|  |......--- ----..........-----.-----....---........---.-.|
|.........|-}}.| --........| |.............|   |.......---| |-...........--|
|........--}}..| |.......-----.......------|   |--------..--|......--...-- |
|--------|}}...| |-......| |....|...-- |...-----................----|...|--|
}}}}}}}}}}}....| |.......| |....|...|  |..............-----------   |...F..|
}}}}}}.........| |.....--- |......---  |....---.......|            -|...|--|
|..............| |....--   --....-- |-------- ----BBBB--------------...--  |
|--S--------------BBB-|     |...--  |.......|    |...............B.....|   |
|.....|....||........-|    --...|   |.......|    |...||..........B..----   |
|..........||.........------....|   |.......---- |...||..........B..--     |
|.....|--......---...........--------..........| |.......---------...--    |
|.....| |------| |--.......--|   |..B......----- -----....| |.|  |....---  |
|.....| |......--| ------..| |----..B......|       |.--------.-- |-.....---|
|-------|........|  |.|....| |.....----BBBB---------...........---.........|
||....B.S........|  |...|..| |.....|  |-.............--------...........---|
||....--|-.....-----------.| |....-----.....----------     |..........---  |
||....-  |..|..B...........| |.|..........|.|              |.|........|    |
----------------------------------------------------------------------------
]]);

-- Make the path somewhat unpredictable
-- If you get "lucky", you may have to go through all of the graveyards.
if percent(50) then
   des.terrain(selection.line(50,8, 53,8), '-')
   des.terrain(selection.line(40,8, 43,8), 'B')
end
if percent(50) then
   des.terrain({ x=27, y=12, typ='|' })
   des.terrain(selection.line(27,3, 29,3), 'B')
   des.terrain({ x=28, y=2, typ='-' })
end
if percent(50) then
   des.terrain({ x=60, y=5, typ='|' })
else
   des.terrain(selection.line(68,8, 68,9), '|')
end
if percent(50) then
   des.terrain(selection.line(16,10, 16,11), '|')
   des.terrain(selection.line(9,13, 14,13), 'B')
end


-- Dungeon Description
-- The shrine to Moloch.
des.region({ region={01,10, 05,14},lit=1,type="temple",filled=2 })
-- The Morgues
des.region({ region={19,01, 24,08},lit=0,type="morgue",filled=1,irregular=1 })
des.region({ region={09,14, 16,18},lit=0,type="morgue",filled=1,irregular=1 })
des.region({ region={37,09, 43,14},lit=0,type="morgue",filled=1,irregular=1 })
des.region({ region={50,09, 64,11},lit=0,type="morgue",filled=1,irregular=1 })
des.region({ region={02,16, 05,18},lit=0,type="morgue",filled=1,irregular=1 })
-- Cerberus area
des.region({ region={00,00, 15,09},lit=0,type="ordinary",arrival_room=true })
-- Stairs
des.stair("down", 01,01)
-- Branch location
des.levregion({ type="branch", region={66,17,66,17} })
des.teleport_region({ region = {58,09,72,18}, dir="down" })
-- TODO-EVILHACK: master places the portal to The Hidden Dungeon here
-- (PORTAL:(74,06,74,06),"hdgn"); the levregion lands with the Hidden
-- Dungeon topology - declaring it before that dungeon exists would
-- fire the fixup_special unknown-level impossible on every load

-- Secret Doors
des.door({ state="locked", material="stone", x=03, y=09 })
des.door({ state="locked", material="stone", x=08, y=16 })

-- The altar of Moloch.
des.altar({ x=03,y=12,align="noalign", type="shrine" })

-- Non diggable walls - everywhere, except the cell where the Hidden
-- Dungeon portal alcove opens at (75,06)
des.non_diggable(selection.area(00,00,71,19))
des.non_diggable(selection.area(72,00,75,05))
des.non_diggable(selection.area(72,07,75,19))
des.non_diggable(selection.area(75,06,75,06))
-- Non passable walls
des.non_passwall(selection.area(00,00,15,09))

-- Entrance to Cerberus' lair, and on to Gehennom
des.drawbridge({ x=12, y=02, dir="west", state="closed" })
des.engraving({ x=13, y=02, type="burn",
                text="Lasciate ogne speranza, voi ch'intrate" })
-- Moat denizens
des.monster({ id="kraken", x=02, y=07 })
des.monster({ id="kraken", x=07, y=06 })
des.monster({ id="kraken", x=11, y=04 })

-- Cerberus guards the entrance to Gehennom
-- Charon plays a small role as well
des.monster({ id="Cerberus", x=01, y=03, peaceful=0 })
des.monster({ id="Charon", x=14, y=02, peaceful=1 })

-- Objects
-- **LOTS** of dead bodies (all human).
-- note: no priest(esse)s or monks - maybe Moloch has a *special*
--       fate reserved for members of *those* classes.
--
des.object({ id="corpse",montype="archeologist" })
des.object({ id="corpse",montype="archeologist" })
des.object({ id="corpse",montype="barbarian" })
des.object({ id="corpse",montype="barbarian" })
des.object({ id="corpse",montype="caveman" })
des.object({ id="corpse",montype="cavewoman" })
des.object({ id="corpse",montype="healer" })
des.object({ id="corpse",montype="healer" })
des.object({ id="corpse",montype="knight" })
des.object({ id="corpse",montype="knight" })
des.object({ id="corpse",montype="ranger" })
des.object({ id="corpse",montype="ranger" })
des.object({ id="corpse",montype="rogue" })
des.object({ id="corpse",montype="rogue" })
des.object({ id="corpse",montype="samurai" })
des.object({ id="corpse",montype="samurai" })
des.object({ id="corpse",montype="tourist" })
des.object({ id="corpse",montype="tourist" })
des.object({ id="corpse",montype="valkyrie" })
des.object({ id="corpse",montype="valkyrie" })
des.object({ id="corpse",montype="wizard" })
des.object({ id="corpse",montype="wizard" })
-- Corpses in Cerberus' lair
des.object({ id="corpse", x=01, y=03, montype="barbarian" })
des.object({ id="corpse", x=04, y=01, montype="knight" })
des.object({ id="corpse", x=05, y=04, montype="ranger" })
des.object({ id="corpse", x=08, y=02, montype="samurai" })
des.object({ id="corpse", x=09, y=03, montype="valkyrie" })

--
-- Some random weapons and armor.
--
des.object("[")
des.object("[")
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object(")")
des.object(")")
--
-- Some random loot.
--
des.object("ruby")
des.object("*")
des.object("*")
des.object("!")
des.object("!")
des.object("!")
des.object("?")
des.object("?")
des.object("?")
des.object("/")
des.object("/")
des.object("=")
des.object("=")
des.object("+")
des.object("+")
des.object("(")
des.object("(")
des.object("(")

des.object({ id = "iron safe", x=02, y=18,
   contents = function()
      des.object("diamond")
      des.object("diamond")
      des.object({ class="/", id="opening" })
      des.object("*")
      des.object("*")
      des.object("=")
      des.object("=")
      des.object("+")
      des.object("+")
   end
})
des.object({ id="chest", x=02, y=17 })
des.object({ id="chest", x=03, y=18 })

-- (Not so) Random traps.
des.trap("board", 21,12)
des.trap("board")
des.trap("magic", 60,01)
des.trap("spear", 26,17)
des.trap("anti magic")
des.trap("anti magic")
des.trap("magic")
des.trap("magic")
des.trap("ice")

-- Random monsters.
-- The ghosts.
des.monster({ class=" ", x=74, y=15 })
des.monster({ id="ghost", x=34, y=01 })
des.monster({ id="ghost", x=38, y=01 })
des.monster({ id="ghost", x=18, y=10 })
des.monster({ id="ghost", x=21, y=15 })
des.monster({ id="spectre", x=66, y=02 })
-- Add a few bats for atmosphere.
des.monster({ id="vampire bat", x=73, y=15 })
des.monster({ id="vampire bat", x=23, y=16 })
des.monster({ id="vampire bat", x=19, y=11 })
-- And a lich for good measure.
des.monster({ class="L", x=20, y=10 })
-- Balrog guards the hidden treasure room
des.monster({ id="balrog", x=05, y=18 })
-- Some undead nasties for good measure
des.monster({ class="V", x=34, y=03 })
des.monster({ class="V", x=66, y=04 })
des.monster({ class="V", x=40, y=03 })
des.monster({ class="z", x=40, y=01 })
des.monster({ class="z", x=62, y=02 })
des.monster({ class="Z", x=64, y=02 })
des.monster({ class="Z", x=36, y=01 })
des.monster({ class="M", x=62, y=04 })
des.monster({ class="M", x=64, y=04 })
des.monster({ class="M", x=36, y=03 })
des.monster({ class="M", x=38, y=03 })
-- And a couple weredemons too
des.monster({ id="weredemon", x=34, y=04 })
des.monster({ id="weredemon", x=64, y=03 })
