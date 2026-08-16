-- NetHack forest forest.lua	$NHDT-Date: 1786886710 2026/08/16 13:25:10 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--
-- Desolate Forest level
--

des.room({ type = "forest", lit = 0, w = 12, h = 7,
           contents = function()
   for i = 1, 4 do
      if percent(30) then
         des.terrain(selection.room():rndcoord(), "t")
      end
   end
   for i = 1, 4 do
      des.object()
   end
   if percent(30) then
      des.monster({ id = "tree blight", peaceful = 0 })
   end
   -- gulthias tree
   des.monster({ id = "gulthias tree", peaceful = 0 })
end })

-- stairs up room
des.room({ type = "forest", contents = function()
   des.stair("up")
   des.object()
end })

-- stairs down room
des.room({ type = "forest", contents = function()
   des.stair("down")
   des.object()
end })

-- normal room, in case of branch portal/stairs placement
des.room({ type = "forest", contents = function()
   for i = 1, 3 do
      if percent(30) then
         des.terrain(selection.room():rndcoord(), "t")
      end
   end
   des.object()
   if percent(30) then
      des.monster("#")
   end
   des.monster({ id = "needle blight", peaceful = 0 })
end })

des.room({ type = "forest", contents = function()
   des.object()
   des.object()
end })

des.room({ type = "forest", contents = function()
   des.object()
   if percent(30) then
      des.monster("#")
   end
end })

des.random_corridors()
