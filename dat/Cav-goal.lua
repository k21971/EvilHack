-- $NHDT-Header: NetHack 5.0 Cav-goal.lua 2026/06/20_22:35:38 NetHack-5.0:1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel");

des.map([[
                                                                            
                          .....................                             
                         .......................                            
                        .........................                           
                       ...........................                          
                      .............................                         
                     ...............................                        
                    .................................                       
                   ...................................                      
                  .....................................                     
                 .......................................                    
                  .....................................                     
                   ...................................                      
                    .................................                       
                     ...............................                        
                      .............................                         
                       ...........................                          
                        .........................                           
                         .......................                            
                                                                            
]]);
-- Dungeon Description
des.region(selection.area(00,00,75,19), "lit")
-- Stairs
des.stair("up")
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Objects
des.object({ id = "rod", x=23, y=10, buc="blessed", spe=0, name="The Sceptre of Might" })
des.object("boulder")
des.object("boulder")
des.object("boulder")
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
-- Bodies of past victims
des.object({ id = "corpse", montype = "caveman" })
des.object({ id = "corpse", montype = "cavewoman" })
des.object({ id = "corpse", montype = "caveman" })
des.object({ id = "corpse", montype = "cavewoman" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
des.object({ id = "corpse", montype = "neanderthal" })
-- monsters.
des.monster({ id = "Annam", x=23, y=10, asleep=1 })
des.monster("shrieker", 26, 13)
des.monster("shrieker", 25, 8)
des.monster("shrieker", 45, 11)
des.wallify()
