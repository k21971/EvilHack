-- NetHack Convict Con-loca.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "locate" level for the quest.
--
--	Here you have to find the exit from the Royal Dungeons
--	to go further towards your assigned quest.
--

des.level_flags("mazelevel", "noteleport", "hardfloor")
des.level_init({ style = "solidfill", fg = " " })

--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "top", map = [[
---------------                                              ---------------
|.....F.......| #################################            |............K|
|.....F.......+##              #                #############+.............|
|.....F.......|                #                #            |.............|
|.....+.......|                #                #            |.............|
|.....F.......|                #                #            |.............|
|.....F.......|              ###                #            |.............|
|.....F.......|             ##                  #            |FFFFFFFFFFFFF|
|--------------     #########                   #            |......|......|
                    #      |S-------|           #            |......|......|
                    #      |........|      ######            |-------------|
    #################      |........|      #    H                           
    #                      |........+#######    #                           
    #                      |----S---|           #            |----------|   
 |--+-------|                   #               #####        |..........|   
 |..........|                   #               #   #        |..........|   
 |..........|                   #     |------|  #   #########+..........|   
 |..........|                   ######S......+###            |..........|   
 |----------|                         |......|               |----------|   
                                      |------|                              
]] });
-- Dungeon Description
des.region({ region = { 01, 01, 13, 07 }, lit = 1, type = "ordinary" })
des.region({ region = { 02, 15, 11, 17 }, lit = 1, type = "ordinary" })
des.region({ region = { 62, 01, 74, 09 }, lit = 1, type = "ordinary" })
des.region({ region = { 62, 14, 71, 17 }, lit = 1, type = "shop", filled = 1 })
des.region({ region = { 28, 10, 35, 12 }, lit = 1, type = "barracks", filled = 1 })
des.region({ region = { 39, 17, 44, 18 }, lit = 1, type = "ordinary" })
-- Stairs
des.stair("up", 02, 03)
des.stair("down", 39, 17)
-- Non diggable walls
des.non_diggable(selection.area(00, 00, 75, 19))
des.door("locked", 04, 14)
des.door("locked", 06, 04)
des.door("open", 14, 02)
des.door("closed", 61, 16)
des.door("closed", 45, 17)
des.door("locked", 36, 12)
des.door("closed", 61, 02)
-- Police station kops
des.monster("inmate", 63, 08)
des.monster("inmate", 70, 08)
des.monster({ id = "Keystone Kop", coord = { 62, 01 }, asleep = 1 })
des.monster({ id = "Keystone Kop", coord = { 65, 01 }, asleep = 1 })
des.monster({ id = "Keystone Kop", coord = { 66, 01 }, asleep = 1 })
des.monster({ id = "Keystone Kop", coord = { 74, 02 }, asleep = 1 })
des.monster({ id = "Kop Sergeant", coord = { 63, 03 }, asleep = 1 })
des.monster({ id = "Kop Sergeant", coord = { 72, 03 }, asleep = 1 })
des.monster({ id = "Kop Lieutenant", coord = { 71, 04 }, asleep = 1 })
des.monster({ id = "Kop Lieutenant", coord = { 68, 05 }, asleep = 1 })
des.monster({ id = "Kop Kaptain", coord = { 64, 06 }, asleep = 1 })
-- Iron golem in supply room
des.monster({ id = "iron golem", coord = { 11, 17 }, asleep = 1 })
-- Supply room junk
des.object("iron chain", 2, 15)
des.object("iron chain", 2, 15)
des.object("iron chain", 3, 15)
des.object("iron chain", 3, 15)
des.object("iron chain", 4, 15)
des.object("heavy iron ball", 4, 15)
des.object("iron chain", 4, 15)
des.object("iron chain", 5, 15)
des.object("iron chain", 5, 15)
des.object("iron chain", 6, 15)
des.object("iron chain", 6, 15)
des.object("iron chain", 6, 15)
des.object("iron chain", 7, 15)
des.object("heavy iron ball", 7, 15)
des.object("iron chain", 8, 15)
des.object("iron chain", 8, 15)
des.object("iron chain", 9, 15)
des.object("iron chain", 9, 15)
des.object("iron chain", 9, 15)
des.object("iron chain", 10, 15)
des.object("iron chain", 10, 15)
des.object("iron chain", 10, 15)
des.object("iron chain", 11, 15)
des.object("iron chain", 11, 15)
des.object("heavy iron ball", 11, 15)
des.object("iron chain", 2, 16)
des.object("heavy iron ball", 2, 16)
des.object("iron chain", 3, 16)
des.object("iron chain", 3, 16)
des.object("iron chain", 3, 16)
des.object("iron chain", 4, 16)
des.object("iron chain", 4, 16)
des.object("iron chain", 5, 16)
des.object("iron chain", 6, 16)
des.object("iron chain", 6, 16)
des.object("iron chain", 6, 16)
des.object("iron chain", 7, 16)
des.object("iron chain", 7, 16)
des.object("heavy iron ball", 7, 16)
des.object("iron chain", 7, 16)
des.object("iron chain", 8, 16)
des.object("iron chain", 8, 16)
des.object("iron chain", 9, 16)
des.object("iron chain", 9, 16)
des.object("iron chain", 9, 16)
des.object("iron chain", 10, 16)
des.object("iron chain", 10, 16)
des.object("iron chain", 11, 16)
des.object("iron chain", 2, 17)
des.object("iron chain", 3, 17)
des.object("iron chain", 3, 17)
des.object("iron chain", 3, 17)
des.object("iron chain", 4, 17)
des.object("heavy iron ball", 4, 17)
des.object("iron chain", 4, 17)
des.object("iron chain", 4, 17)
des.object("heavy iron ball", 4, 17)
des.object("iron chain", 5, 17)
des.object("iron chain", 6, 17)
des.object("iron chain", 7, 17)
des.object("iron chain", 7, 17)
des.object("iron chain", 7, 17)
des.object("iron chain", 8, 17)
des.object("heavy iron ball", 8, 17)
des.object("iron chain", 9, 17)
des.object("iron chain", 9, 17)
des.object("heavy iron ball", 9, 17)
des.object("iron chain", 9, 17)
des.object("iron chain", 10, 17)
des.object("iron chain", 10, 17)
des.object("iron chain", 10, 17)
des.object("heavy iron ball", 10, 17)
des.object("iron chain", 11, 17)
