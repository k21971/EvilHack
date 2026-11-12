-- $NHDT-Header: NetHack 5.0 minetn-1.lua 2026/06/20_22:35:38 NetHack-5.0:1.15 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- A tragic accident has occurred in Frontier Town....
--
-- Minetown variant 1
-- Orcish Town - a variant of Frontier Town that has been
-- overrun by orcs.  Note the barricades (iron bars).

des.level_flags("mazelevel", "inaccessibles")

des.level_init({ style="mines", fg=".", bg=" ", smoothed=true, joined=true, walled=true })

des.map([[
.....................................
.----------------F------------------.
.|.................................|.
.|.-------------......------------.|.
.|.|...|...|...|......|..|...|...|.|.
.F.|...|...|...|......|..|...|...|.|.
.|.|...|...|...|......|..|...|...|.F.
.|.|...|...|----......------------.|.
.|.---------.......................|.
.|.................................|.
.|.---------.....--...--...........|.
.|.|...|...|----.|.....|.---------.|.
.|.|...|...|...|.|.....|.|..|....|.|.
.|.|...|...|...|.|.....|.|..|....|.|.
.|.|...|...|...|.|.....|.|..|....|.|.
.|.-------------.-------.---------.|.
.|.................................F.
.-----------F------------F----------.
.....................................
]]);

-- Don't let the player fall into his likely death
des.teleport_region({ region={01,01,70,19}, region_islev=1,
                      exclude={02,02,35,17}, exclude_islev=1, dir="down" })
des.region(selection.area(01,01,35,17), "lit")
des.levregion({ type="stair-up", region={01,03,21,19}, region_islev=1,
		exclude={00,01,36,17} });
des.levregion({ type="stair-down", region={57,03,75,19}, region_islev=1,
		exclude={00,01,36,17} })

-- Define areas of the map
-- Inside the walls
local inside = selection.floodfill(18,8)
-- Near the temple
local near_temple = selection.area(17,8, 23,14) & inside

-- One shopkeeper managed to barricade their shop with boulders, and they live!
des.region({ region={4,4,6,7}, lit=1, type="shop", filled=1 })
des.door("locked",5,8)
des.object("boulder",4,9)
des.object("boulder",5,9)
des.object("boulder",6,9)

-- shame we can't make polluted fountains
des.feature("fountain",16,09)
des.feature("fountain",25,09)

-- the altar's defiled; useful for BUC but never coaligned
des.altar({ x=20,y=13,align="noalign", type="shrine" })

-- Magic chest
des.feature("magic chest", 20, 14)

-- set up the shop doors; could be broken down
des.door("random",9,8)
des.door("random",13,7)
des.door("random",22,5)
des.door("random",27,7)
des.door("random",31,7)
des.door("random",5,10)
des.door("random",9,10)
des.door("random",15,13)
des.door("random",25,13)
des.door("random",31,11)

-- knock a few holes in the shop interior walls
des.replace_terrain({ region={11,04,11,06}, fromterrain="|", toterrain=".", chance=18 })
des.replace_terrain({ region={25,04,29,06}, fromterrain="|", toterrain=".", chance=18 })
des.replace_terrain({ region={07,12,11,14}, fromterrain="|", toterrain=".", chance=18 })
des.replace_terrain({ region={28,12,28,14}, fromterrain="|", toterrain=".", chance=33 })

-- Rubble!
for i = 1, 5 + d(2,5) do
  if percent(90) then
    des.object("boulder")
  end
  des.object("rock")
end

-- Shopkeeper "bones" piles. One spot each in most shops...
-- there were 4 shops in Frontier Town; the general store shk is still present
-- so there should only be three dead shks here
local shk_demise = { {09,05},{13,04},{26,04},{31,05},{30,14},{05,14},{10,13},{26,14},{27,13} }
shuffle(shk_demise)

des.monster({ id = "shopkeeper", coord = shk_demise[1], dead = 1 })
des.monster({ id = "shopkeeper", coord = shk_demise[2], dead = 1 })
des.monster({ id = "shopkeeper", coord = shk_demise[3], dead = 1 })
-- Orcish Town is hard enough without the orcs picking up and using the
-- shopkeepers' wands and potions.
des.object("boulder", shk_demise[1])
des.object("boulder", shk_demise[2])
des.object("boulder", shk_demise[3])

-- shop inventories... 9 items from a tool shop, 4 from a delicatessen, 6 from Izchak
-- Ideally we could just call some routine to fill the shop objects with the
-- appropriate probabilities for their shops and not put in the shopkeepers, but
-- that can't be made to work without a refactor of room-filling semantics.
for i = 1, 9 do
  des.object({ class = "(", coord = inside:rndcoord(0) })
end
-- For now, only FOOD_CLASS items from deli.
for i = 1, 4 do
  des.object({ class = "%", coord = inside:rndcoord(0) })
end
-- probabilities not exactly the same, no potion of oil, but otherwise close.
-- Notably, overall 5% chance of a magic lamp per placement, up from 3%.
for i = 1, 6 do
  if percent(80) then
    if percent(50) then
      des.object({ id = "wax candle", coord = inside:rndcoord(0), quantity = d(4) })
    else
      des.object({ id = "tallow candle", coord = inside:rndcoord(0), quantity = d(4) })
    end
  else
    if percent(50) then
      des.object("oil lamp", inside:rndcoord(0))
    else
      if percent(50) then
        des.object("lantern", inside:rndcoord(0))
      else
        des.object("magic lamp", inside:rndcoord(0))
      end
    end
  end
end

-- priest spoils
-- have to manually provide the spellbooks because those are given to a priest
-- when initialized as a shrine attendant - which this priest is not.
des.monster({ id = "aligned cleric", x=20, y=12, dead = 1 })
for i = 1, d(3) + 1 do
  des.object("+", 20, 12)
end

-- the remains of the Watch
for i = 1, 4 do
  des.monster({ id = "watchman", dead = 1 })
end
des.monster({ id = "watch captain", dead = 1 })

-- the Orcish Army

for i = 1, math.random(5,15) do
   if percent(50) then
      des.monster({ id = "orc-captain", coord = inside:rndcoord(1), peaceful=0 });
   else
      if percent(80) then
         des.monster({ id = "Uruk-hai", coord = inside:rndcoord(1), peaceful=0 })
      else
         des.monster({ id = "Mordor orc", coord = inside:rndcoord(1), peaceful=0 })
      end
   end
end
-- shamans can be hanging out in/near the temple
-- one of the shamans is higher level
for i = 1, math.random(1,6) do
   des.monster({ id = "orc shaman", coord = near_temple:rndcoord(0), peaceful=0,
                 m_lev_adj = (i == 1) and 3 or 0 });
end
-- add one or two goblin captains for good measure
for i = 1, math.random(1,2) do
   des.monster({ id = "goblin-captain", coord = inside:rndcoord(1), peaceful=0 })
end
-- these are not such a big deal to run into outside the bars
for i = 1, math.random(10,19) do
   if percent(90) then
      des.monster({ id = "hill orc", peaceful = 0 })
   else
      if percent(80) then
         des.monster({ id = "goblin", peaceful = 0 })
      else
         des.monster({ id = "goblin outrider", peaceful = 0 })
      end
   end
end

-- Hack to force full-level wallification

des.wallify()
