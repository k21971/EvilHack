-- NetHack mines minetn-8.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- Minetown variant 8
-- Frontier Town - not yet pillaged by orcs

des.room({ type = "ordinary", lit=1, x=3, y=3,
           xalign="center", yalign="center", w=31, h=15,
           contents = function()
              des.feature("fountain", 13, 7)
              des.feature("fountain", 20, 7)

              -- The Town Watch
              des.monster({ id = "watchman", peaceful = 1 })
              des.monster({ id = "watchman", peaceful = 1 })
              des.monster({ id = "watchman", peaceful = 1 })
              des.monster({ id = "watchman", peaceful = 1 })
              des.monster({ id = "watch captain", peaceful = 1 })

              des.room({ type = "shop", chance=90, lit=1, x=2, y=2, w=3, h=4,
                         contents = function()
                            des.door({ state = "closed", wall="south" })
                         end
              })

              des.room({ type = "tool shop", chance=90, lit=1, x=2, y=9, w=3, h=4,
                         contents = function()
                            des.door({ state = "closed", wall="north" })
                         end
              })

              des.room({ type = "ordinary", lit=0, x=6, y=2, w=3, h=4,
                         contents = function()
                            des.door({ state = "closed", wall="south" })
                         end
              })

              des.room({ type = "ordinary", lit=1, x=6, y=9, w=3, h=4,
                         contents = function()
                            des.door({ state = "closed", wall="north" })
                         end
              })

              des.room({ type = "food shop", chance=90, lit=1, x=10, y=2, w=2, h=3,
                         contents = function()
                            des.door({ state = "closed", wall="south" })
                         end
              })

              des.room({ type = "candle shop", lit=1, x=22, y=2, w=3, h=3,
                         contents = function()
                            des.door({ state = "closed", wall="south" })
                         end
              })

              des.room({ type = "ordinary", lit=0, x=10, y=10, w=2, h=3,
                         contents = function()
                            des.door({ state = "locked", wall="east" })
                            des.monster("rock gnome")
                         end
              })

              des.room({ type = "ordinary", lit=1, x=19, y=2, w=2, h=3,
                         contents = function()
                            des.door({ state = "locked", wall="west" })
                            des.monster("rock gnome")
                         end
              })

              des.room({ type = "temple", lit=1, x=15, y=9, w=4, h=4,
                         contents = function()
                            des.door({ state = "closed", wall="north" })
                            des.altar({ x=2, y=2, align=align[1], type="shrine" })
                            des.feature("magic chest", 2, 3)
                            des.monster("gnomish wizard")
                            des.monster("gnomish wizard")
                         end
              })

              des.room({ type = "ordinary", lit=1, x=22, y=10, w=2, h=3,
                         contents = function()
                            des.door({ state = "locked", wall="west" })
                         end
              })

              des.room({ type = "ordinary", lit=1, x=26, y=2, w=3, h=3,
                         contents = function()
                            des.door({ state = "closed", wall="south" })
                            des.monster("gnome noble")
                         end
              })

              des.room({ type = "ordinary", lit=0, x=25, y=10, w=4, h=3,
                         contents = function()
                            des.door({ state = "closed", wall="north" })
                         end
              })
           end
})

des.room({ type = "ordinary",
           contents = function()
              des.stair("up")
           end
})

des.room({ type = "ordinary",
           contents = function()
              des.stair("down")
              des.trap()
              des.monster("rock gnome")
              des.monster("rock gnome")
           end
})

des.room({ type = "ordinary",
           contents = function()
              des.monster("mountain dwarf")
           end
})

des.room({ type = "ordinary",
           contents = function()
              des.trap()
              des.monster("rock gnome")
           end
})

des.random_corridors()
