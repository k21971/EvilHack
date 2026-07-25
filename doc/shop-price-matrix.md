# EvilHack Shop Price Matrix - Scroll of Blank Paper

Buy / sell prices in zorkmids for an **uncursed, known, non-eroded scroll of blank
paper** (base cost 60), quantity 1, for a normal hero (no dunce cap, no visible
tourist shirt). Generated from the `get_cost()` / `set_cost()` /
`shk_racial_adjustments()` logic in `src/shk.c`.

Each cell is **BUY / SELL**. The racial shops below use the standard NetHackWiki
charisma ranges (≤5, 6–7, 8–10, 11–15, 16–17, 18, ≥19), and every value within a
range is identical, so any charisma in the range gives the listed price. **Gnome and
nymph shops ignore the general charisma mechanic entirely** - they price on a single
stat (intelligence and a charisma tier respectively) - so they get their own tables.
Players are grouped by their shared racial multiplier; each group lists every race it
covers. `(liked)` marks a shop that gives a racial discount. **Bold** cells are
break-even (buy = sell), where buy and sell converge at high charisma.

## Human shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Aasimar | x4/5 _(liked)_ | 96/38 | 72/38 | 64/38 | 48/38 | **48/48** | **48/48** | **48/48** |
| Human, Elf, Dwarf, Illithid, Hobbit, Giant, Tortle, Drow | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Orc, Gnome | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Centaur | x3/2 | 180/20 | 135/20 | 120/20 | 90/20 | 90/27 | 83/30 | 75/33 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Elf shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Elf | x4/5 _(liked)_ | 96/38 | 72/38 | 64/38 | 48/38 | **48/48** | **48/48** | **48/48** |
| Human, Gnome, Centaur, Hobbit, Giant, Aasimar | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Dwarf, Tortle | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Orc, Illithid, Drow | x2 | 240/15 | 180/15 | 160/15 | 120/15 | 120/20 | 110/23 | 100/25 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Dwarf shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Dwarf | x3/4 _(liked)_ | 90/40 | 68/40 | 60/40 | 45/40 | **45/45** | **45/45** | **45/45** |
| Aasimar | x4/5 _(liked)_ | 96/38 | 72/38 | 64/38 | 48/38 | **48/48** | **48/48** | **48/48** |
| Human, Gnome, Centaur, Hobbit, Tortle | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Elf, Drow | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Giant | x3/2 | 180/20 | 135/20 | 120/20 | 90/20 | 90/27 | 83/30 | 75/33 |
| Orc, Illithid | x2 | 240/15 | 180/15 | 160/15 | 120/15 | 120/20 | 110/23 | 100/25 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Orc shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Orc | x2/3 _(liked)_ | 80/45 | 60/45 | 53/45 | **40/40** | **40/40** | **40/40** | **40/40** |
| Illithid, Centaur, Giant | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Human, Aasimar | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Dwarf, Tortle | x5/3 | 200/18 | 150/18 | 133/18 | 100/18 | 100/24 | 92/27 | 83/30 |
| Elf, Gnome, Hobbit, Drow | x3 | 360/10 | 270/10 | 240/10 | 180/10 | 180/13 | 165/15 | 150/17 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Illithid shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Illithid | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Human, Elf, Dwarf, Orc, Gnome, Centaur, Hobbit, Giant, Tortle, Drow, Aasimar, Vampire | x10 | 1200/3 | 900/3 | 800/3 | 600/3 | 600/4 | 550/5 | 500/5 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Centaur shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Centaur | x3/4 _(liked)_ | 90/40 | 68/40 | 60/40 | 45/40 | **45/45** | **45/45** | **45/45** |
| Elf, Hobbit, Giant, Drow | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Human, Dwarf, Orc, Gnome, Illithid, Tortle, Aasimar | x3/2 | 180/20 | 135/20 | 120/20 | 90/20 | 90/27 | 83/30 | 75/33 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Giant shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Giant | x3/4 _(liked)_ | 90/40 | 68/40 | 60/40 | 45/40 | **45/45** | **45/45** | **45/45** |
| Aasimar | x4/5 _(liked)_ | 96/38 | 72/38 | 64/38 | 48/38 | **48/48** | **48/48** | **48/48** |
| Elf, Centaur, Drow | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Human, Orc, Gnome, Illithid, Hobbit, Tortle | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Dwarf | x3/2 | 180/20 | 135/20 | 120/20 | 90/20 | 90/27 | 83/30 | 75/33 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Drow shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Drow | x4/5 _(liked)_ | 96/38 | 72/38 | 64/38 | 48/38 | **48/48** | **48/48** | **48/48** |
| Human, Gnome, Centaur, Hobbit, Giant, Aasimar | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Dwarf, Tortle | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Elf, Orc, Illithid | x2 | 240/15 | 180/15 | 160/15 | 120/15 | 120/20 | 110/23 | 100/25 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Aasimar shopkeeper

| Player race(s) | mult | CHA ≤5 | 6–7 | 8–10 | 11–15 | 16–17 | 18 | ≥19 |
|---|---|---|---|---|---|---|---|---|
| Aasimar | x3/4 _(liked)_ | 90/40 | 68/40 | 60/40 | 45/40 | **45/45** | **45/45** | **45/45** |
| Centaur | x1 | 120/30 | 90/30 | 80/30 | 60/30 | 60/40 | 55/45 | **50/50** |
| Human, Orc, Gnome, Illithid, Hobbit, Giant, Tortle | x4/3 | 160/23 | 120/23 | 107/23 | 80/23 | 80/30 | 73/34 | 67/38 |
| Elf, Dwarf, Drow | x3/2 | 180/20 | 135/20 | 120/20 | 90/20 | 90/27 | 83/30 | 75/33 |
| Vampire | x5 | 600/6 | 450/6 | 400/6 | 300/6 | 300/8 | 275/9 | 250/10 |
| Draugr | x20 | 2400/2 | 1800/2 | 1600/2 | 1200/2 | 1200/2 | 1100/2 | 1000/3 |

## Nymph shopkeeper

Race-independent and priced **only** by nymph's own charisma tier - the general
charisma surcharge/taper and sell bonus do not apply. Draugr and Vampire get this
stat pricing too (not x20 / x5).

| CHA tier | mult | buy / sell |
|---|---|---|
| ≤6 | x8 | 480/4 |
| 7–11 | x3 | 180/10 |
| 12–14 | x5/3 | 100/18 |
| 15–17 | x4/3 | 80/23 |
| 18+ | x1 | 60/30 |

## Gnome shopkeeper

Race-independent and priced **only** by the hero's Intelligence - charisma plays no
part, so there are no charisma columns. Draugr and Vampire get this stat pricing too.

| INT tier | mult | buy / sell |
|---|---|---|
| ≤6 | x6 | 360/5 |
| 7–11 | x2 | 120/15 |
| 12–14 | x3/2 | 90/20 |
| 15–17 | x4/3 | 80/23 |
| 18+ | x1 | 60/30 |

## Notes

- Charisma now **raises** the sell price (it used to do nothing) and the old high-
  charisma **buy** discount is gone except at shops that already give a racial
  discount - so buy and sell converge instead of crossing, and a high-CHA hero can
  never buy below the sell price (no buy-resell loop). Selling rises or holds with
  charisma at every racial shop...
- ...**except Orc -> Orc**, whose sell offer dips from 45 (CHA ≤10) to 40 (CHA ≥11).
  Orc's same-race discount (x2/3) is the steepest in the matrix; the reciprocal sell
  bonus overshoots the floored buy price (40), so the cap claws it back. The 45 only
  shows at low charisma because low-CHA orcs overpay buying, which lifts the cap.
  Harmless (no exploit).
- **Gnome and nymph shops ignore the general charisma mechanic.** Gnome prices on
  intelligence only; nymph on its own charisma tier only. Neither stacks the general
  buy surcharge/taper or sell bonus, so neither converges to break-even at high
  charisma - nymph's x1 tier (CHA 18+) just leaves the base 60/30.
- Liked (discount) racial shops converge to **break-even** at high charisma (buy =
  sell, bold). Neutral and penalty racial shops show buy tapering down while sell
  rises to meet it.
