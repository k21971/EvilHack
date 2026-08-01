-- NetHack oracle oracle-2.lua	$NHDT-Date: 1524689580 2018/04/25 20:53:00 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
-- Copyright (c) 2015 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
--
-- Oracle level variant (EvilHack).

des.room({ type = "delphi", lit = 1, x = 3, y = 3, xalign = "center", yalign = "center", w = 11, h = 9, contents = function()
   -- Make a square ring of pools around the Oracle, 2 spaces out, with one
   -- orthogonal pool removed

   local orthopool = { {5,2},{3,4},{7,4},{5,6} }
   shuffle(orthopool);

   des.feature("pool", 3, 2)
   des.feature("pool", 4, 2)
   des.feature("pool", 6, 2)
   des.feature("pool", 7, 2)
   des.feature("pool", 3, 3)
   des.feature("pool", 7, 3)
   des.feature("pool", 3, 5)
   des.feature("pool", 7, 5)
   des.feature("pool", 3, 6)
   des.feature("pool", 4, 6)
   des.feature("pool", 6, 6)
   des.feature("pool", 7, 6)
   des.feature("pool", orthopool[1])
   des.feature("pool", orthopool[2])
   des.feature("pool", orthopool[3])

   -- The 4 fountains are on the outside corners
   des.feature("fountain", 2, 1)
   des.feature("fountain", 8, 1)
   des.feature("fountain", 2, 7)
   des.feature("fountain", 8, 7)

   -- Grass
   des.replace_terrain({ region = { 0, 0, 10, 8 }, fromterrain = ".", toterrain = "g", chance = 100 })

   -- statues of the 4 elements - appropriately neutral
   local statuelocs = { {5,0},{5,8},{1,4},{9,4} }
   shuffle(statuelocs);
   des.object({ id = "statue", coord = statuelocs[1], montype = "water elemental" })
   des.object({ id = "statue", coord = statuelocs[2], montype = "earth elemental" })
   des.object({ id = "statue", coord = statuelocs[3], montype = "fire elemental" })
   des.object({ id = "statue", coord = statuelocs[4], montype = "air elemental" })

   des.monster({ id = "Oracle", coord = { 5, 4 }, inventory = function()
      des.object({ id = "eight ball", buc = "uncursed", name = "Magic 8-Ball" })
   end })

   des.monster()
   des.monster()
end })

des.room({ type = "ordinary", contents = function()
   des.stair("up")
   des.object()
end })

des.room({ type = "ordinary", contents = function()
   des.stair("down")
   des.object()
   des.trap()
   des.monster()
   des.monster()
end })

des.room({ type = "ordinary", contents = function()
   des.object()
   des.object()
   des.monster()
end })

des.room({ type = "ordinary", contents = function()
   des.object()
   des.trap()
   des.monster()
end })

des.room({ type = "ordinary", contents = function()
   des.object()
   des.trap()
   des.monster()
end })

des.random_corridors()
