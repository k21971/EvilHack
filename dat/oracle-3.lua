-- NetHack oracle oracle-3.lua	$NHDT-Date: 1524689580 2018/04/25 20:53:00 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
-- Copyright (c) 2015 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
--
-- Oracle level variant (EvilHack).

des.room({ type = "delphi", lit = 1, x = 3, y = 3, xalign = "center", yalign = "center", w = 11, h = 9, contents = function()
   -- Make a squarish ring of pools around the Oracle, 3 spaces out
   des.feature("pool", 3, 1)
   des.feature("pool", 4, 1)
   des.feature("pool", 6, 1)
   des.feature("pool", 7, 1)
   des.feature("pool", 2, 2)
   des.feature("pool", 8, 2)
   des.feature("pool", 2, 3)
   des.feature("pool", 8, 3)
   des.feature("pool", 2, 5)
   des.feature("pool", 8, 5)
   des.feature("pool", 2, 6)
   des.feature("pool", 8, 6)
   des.feature("pool", 3, 7)
   des.feature("pool", 4, 7)
   des.feature("pool", 6, 7)
   des.feature("pool", 7, 7)

   -- Close off three of the four passages into the center; there are also only
   -- three fountains; make sure that no fountain is aligned with the open square
   local orthopool = { {5,1},{2,4},{8,4},{5,7} }
   local fountain = { {5,2},{3,4},{7,4},{5,6} }
   des.feature("pool", orthopool[1])
   des.feature("pool", orthopool[2])
   des.feature("pool", orthopool[3])
   des.feature("pool", orthopool[4])
   des.feature("fountain", fountain[1])
   des.feature("fountain", fountain[2])
   des.feature("fountain", fountain[3])
   des.feature("fountain", fountain[4])

   do
      local choice = math.random(0, 3)
      if choice == 0 then
         des.terrain(orthopool[1], ".")
         des.terrain(fountain[1], ".")
      elseif choice == 1 then
         des.terrain(orthopool[2], ".")
         des.terrain(fountain[2], ".")
      elseif choice == 2 then
         des.terrain(orthopool[3], ".")
         des.terrain(fountain[3], ".")
      elseif choice == 3 then
         des.terrain(orthopool[4], ".")
         des.terrain(fountain[4], ".")
      end
   end

   -- four trees
   des.terrain(3, 2, "T")
   des.terrain(3, 6, "T")
   des.terrain(7, 2, "T")
   des.terrain(7, 6, "T")

   -- Grass
   des.replace_terrain({ region = { 0, 0, 10, 8 }, fromterrain = ".", toterrain = "g", chance = 100 })

   local statuelocs = { {0,0},{10,0},{0,8},{10,8} }
   shuffle(statuelocs);
   des.object({ id = "statue", coord = statuelocs[1], montype = "snake" })
   des.object({ id = "statue", coord = statuelocs[2], montype = "guardian naga" })
   des.object({ id = "statue", coord = statuelocs[3], montype = "shark" })
   des.object({ id = "statue", coord = statuelocs[4], montype = "water nymph" })

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

-- Keep thy enemies close...
des.object({ id = "statue", montype = "woodchuck" })

des.random_corridors()
