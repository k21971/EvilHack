-- NetHack mines minend-5.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- Mine end level variant 5
-- "The Sewers of Waterdeep"
-- Infested with all sorts of nasty filth critters

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "nommap")

des.message("You enter the sewers of the Gnomish Mines.")
des.message("The stench of fetid sewage is barely tolerable.")

des.map([[
---------------------------------------------------------------------------
|                           ---------------                               |
|  ---------                |sssssssssssss|                         ----  |
|  |sssssss|----------------|sssssss...sss|-------------------------|ss|  |
|  |s.....sssssssssssssssssssss.........sssssssssSsssssssssssSssssssSss|  |
|  |sssssss|---------s------|ssss.......ss|--s--s-------------s-----|ss|  |
|  -----s---.sssssss|s|     |sssssssssssss| |s||s|..sssssssss|s|    ----  |
|      |s| |.sssssss|s|     -------s---------s||sS..ssssssss\|s------     |
|      |s| |.sssssss|s|           |s|sssssssss||s|..sssssssss|ssssss|     |
|      |s------------s------      |s|s----------s---------S--------s|     |
|      |sssssssssssssssssss|-------s|s|sssssssssssssss|  |s|      |s|     |
| ---- --------|sss....sssssssssssss|s|ssssssss......s|  |s|    ---S----  |
| |ss|---------|s.........s|-------sss|ssssss.......ss|---S-----|ssssss|  |
| |ssSsssssssssSssss....sss|----------|ssss.......ssssssssssssssss...ss|  |
| |ss|---------|ssssssssssssssssssssssssssssssssssssss|---------|ssssss|  |
| ----         ----------------------------------------         --------  |
|                                                                         |
---------------------------------------------------------------------------
]]);

-- Dungeon Description
local place = { {3,14},{70,3},{12,7} }
shuffle(place)

des.region(selection.area(0,0,74,17),"unlit")
des.teleport_region({ region={39,10,53,14}, exclude={0,0,0,0} })
-- Secret doors
des.door("locked",5,13)
des.door("locked",15,13)
des.door("locked",49,4)
des.door({ state="locked", material="iron", x=49, y=7 })
des.door({ state="locked", material="iron", x=58, y=9 })
des.door("locked",58,12)
des.door("locked",61,4)
des.door("locked",68,4)
des.door("locked",67,11)
-- Stairs
des.stair("up", 48, 12)
-- Non diggable / non phase walls
des.non_diggable(selection.area(0,0,74,17))
des.non_passwall(selection.area(11,5,20,9))
des.non_passwall(selection.area(49,5,61,9))
-- Niches
-- Note: place[3] empty
des.object({ id="luckstone", coord=place[1], achievement=1 })
des.object("loadstone", place[2])
-- Traps
for i = 1, 10 do
  des.trap()
end
des.trap("teleport",19,6)
-- Objects
des.object({ id = "chest", x=12, y=6, trapped=1,
             contents = function()
                des.object("/")
                des.object("+")
                des.object("magic key")
                -- trying to have a random tool spawn inside
                -- a container could result in large box or
                -- chest forming inside of it, so specifiy
                -- what we want to have spawn
                if percent(50) then
                  des.object("sack")
                else
                  if percent(80) then
                    des.object("oilskin sack")
                  else
                    des.object("bag of holding")
                  end
                end
                if percent(70) then
                  des.object("stethoscope")
                else
                  if percent(50) then
                    des.object("pick-axe")
                  else
                    des.object("can of grease")
                  end
                end
                if percent(50) then
                  des.object("tinning kit")
                else
                  if percent(80) then
                    des.object("tooled horn")
                  else
                    des.object("magic lamp")
                  end
                end
             end })
des.object({ id = "crystal chest", x=12, y=8,
             contents = function()
                des.object("ruby")
                des.object("emerald")
                des.object("/")
                des.object("!")
                des.object("=")
                des.object("?")
                des.object("+")
                des.object({ id = "gold piece", quantity = 5000 })
             end })
des.object({ id = "iron safe", x=12, y=7,
             contents = function()
                des.object("diamond")
                des.object("dilithium crystal")
                des.object("/")
                des.object("!")
                des.object("=")
                des.object("?")
                des.object("+")
             end })
for i = 1, 6 do
  des.object("*")
end

-- Monsters
des.monster({ id = "Rat King", x=60, y=7, asleep=1 })
des.monster("giant crocodile", 18, 7)
des.monster("giant leech", 52, 7)
des.monster({ id = "sewer rat", x=59, y=6, asleep=1 })
des.monster({ id = "sewer rat", x=59, y=8, asleep=1 })
des.monster({ id = "sewer rat", x=60, y=6, asleep=1 })
des.monster({ id = "sewer rat", x=60, y=8, asleep=1 })
des.monster("giant leech", 4, 12)
des.monster("giant cockroach", 4, 14)
des.monster("giant leech", 69, 3)
des.monster("giant cockroach", 69, 5)
des.monster("creeping mound", 15, 6)
des.monster("giant leech", 17, 7)
des.monster("giant cockroach", 16, 6)
des.monster("giant cockroach", 16, 7)

des.monster("baby crocodile")
des.monster("baby crocodile")
des.monster("crocodile")
for i = 1, 8 do
  des.monster("sewer rat")
end
for i = 1, 6 do
  des.monster("giant rat")
end
des.monster("enormous rat")
des.monster("enormous rat")
des.monster("rodent of unusual size")
des.monster("wererat")
for i = 1, 12 do
  des.monster("giant cockroach")
end
for i = 1, 10 do
  des.monster("giant leech")
end
des.monster("creeping mound")
des.monster("creeping mound")
