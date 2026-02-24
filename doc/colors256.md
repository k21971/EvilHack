# EvilHack 256-Color Reference

Players can use any color number 0-255 in status hilites, condition
hilites, and menu colors. Terminals must support 256-color mode
(`$TERM` containing "256color", or `$COLORTERM` set) for extended colors
16-255 to render. On older terminals, extended colors automatically
fall back to the closest base-16 match.

> **Hex codes:** The `#RRGGBB` values show the exact RGB for each color.
> Paste any hex code into a color picker to preview it.

---

## Base Colors (0-15)

These are the original named colors. Use either the name or number.

| # | Hex | Name | Aliases |
|--:|------|------|--------|
| 0 | `#555555` | black |  |
| 1 | `#AA0000` | red |  |
| 2 | `#00AA00` | green |  |
| 3 | `#AA5500` | brown |  |
| 4 | `#0000AA` | blue |  |
| 5 | `#AA00AA` | magenta | purple |
| 6 | `#00AAAA` | cyan |  |
| 7 | `#AAAAAA` | gray | grey |
| 8 | `#555555` | no color | transparent (also NO_COLOR) |
| 9 | `#FF5555` | orange | light red, bright red |
| 10 | `#55FF55` | light green | bright green |
| 11 | `#FFFF55` | yellow |  |
| 12 | `#5555FF` | light blue | bright blue |
| 13 | `#FF55FF` | light magenta | light purple, bright magenta |
| 14 | `#55FFFF` | light cyan | bright cyan |
| 15 | `#FCFCFC` | white |  |

---

## Extended Palette (16-231) - 6x6x6 RGB Cube

Each color is built from Red, Green, and Blue components at six
intensity levels: 0, 95, 135, 175, 215, 255.

**Formula:** `color = 16 + (36 * R_level) + (6 * G_level) + B_level`
where R/G/B_level are 0-5, mapping to intensities above.

### Red level 0 (R=0): colors 16-51
*Cool tones: greens, blues, cyans (no red component)*

| # | Hex | R | G | B | Description |
|--:|------|--:|--:|--:|-------------|
| 16 | `#000000` | 0 | 0 | 0 | black (duplicate of 0) |
| 17 | `#00005F` | 0 | 0 | 95 | very dark blue |
| 18 | `#000087` | 0 | 0 | 135 | dark blue |
| 19 | `#0000AF` | 0 | 0 | 175 | medium blue |
| 20 | `#0000D7` | 0 | 0 | 215 | blue |
| 21 | `#0000FF` | 0 | 0 | 255 | bright blue |
| 22 | `#005F00` | 0 | 95 | 0 | very dark green |
| 23 | `#005F5F` | 0 | 95 | 95 | dark teal |
| 24 | `#005F87` | 0 | 95 | 135 | deep teal blue |
| 25 | `#005FAF` | 0 | 95 | 175 | steel blue |
| 26 | `#005FD7` | 0 | 95 | 215 | dodger blue |
| 27 | `#005FFF` | 0 | 95 | 255 | deep blue |
| 28 | `#008700` | 0 | 135 | 0 | forest green |
| 29 | `#00875F` | 0 | 135 | 95 | sea green |
| 30 | `#008787` | 0 | 135 | 135 | teal |
| 31 | `#0087AF` | 0 | 135 | 175 | cerulean |
| 32 | `#0087D7` | 0 | 135 | 215 | azure |
| 33 | `#0087FF` | 0 | 135 | 255 | deep sky blue |
| 34 | `#00AF00` | 0 | 175 | 0 | green |
| 35 | `#00AF5F` | 0 | 175 | 95 | medium sea green |
| 36 | `#00AF87` | 0 | 175 | 135 | medium aquamarine |
| 37 | `#00AFAF` | 0 | 175 | 175 | light sea green |
| 38 | `#00AFD7` | 0 | 175 | 215 | deep sky blue |
| 39 | `#00AFFF` | 0 | 175 | 255 | vivid sky blue |
| 40 | `#00D700` | 0 | 215 | 0 | lime green |
| 41 | `#00D75F` | 0 | 215 | 95 | spring green |
| 42 | `#00D787` | 0 | 215 | 135 | medium spring green |
| 43 | `#00D7AF` | 0 | 215 | 175 | aquamarine |
| 44 | `#00D7D7` | 0 | 215 | 215 | dark turquoise |
| 45 | `#00D7FF` | 0 | 215 | 255 | deep sky blue |
| 46 | `#00FF00` | 0 | 255 | 0 | lime |
| 47 | `#00FF5F` | 0 | 255 | 95 | bright spring green |
| 48 | `#00FF87` | 0 | 255 | 135 | spring green |
| 49 | `#00FFAF` | 0 | 255 | 175 | medium spring green |
| 50 | `#00FFD7` | 0 | 255 | 215 | bright aquamarine |
| 51 | `#00FFFF` | 0 | 255 | 255 | cyan / aqua |

### Red level 1 (R=95): colors 52-87
*Muted tones: dark reds, purples, olives, dusty greens*

| # | Hex | R | G | B | Description |
|--:|------|--:|--:|--:|-------------|
| 52 | `#5F0000` | 95 | 0 | 0 | dark maroon |
| 53 | `#5F005F` | 95 | 0 | 95 | dark purple |
| 54 | `#5F0087` | 95 | 0 | 135 | indigo |
| 55 | `#5F00AF` | 95 | 0 | 175 | blue violet |
| 56 | `#5F00D7` | 95 | 0 | 215 | vivid blue violet |
| 57 | `#5F00FF` | 95 | 0 | 255 | electric indigo |
| 58 | `#5F5F00` | 95 | 95 | 0 | dark olive |
| 59 | `#5F5F5F` | 95 | 95 | 95 | dim gray |
| 60 | `#5F5F87` | 95 | 95 | 135 | slate gray |
| 61 | `#5F5FAF` | 95 | 95 | 175 | medium slate blue |
| 62 | `#5F5FD7` | 95 | 95 | 215 | cornflower blue |
| 63 | `#5F5FFF` | 95 | 95 | 255 | royal blue |
| 64 | `#5F8700` | 95 | 135 | 0 | olive green |
| 65 | `#5F875F` | 95 | 135 | 95 | dark sea green |
| 66 | `#5F8787` | 95 | 135 | 135 | cadet blue |
| 67 | `#5F87AF` | 95 | 135 | 175 | steel blue |
| 68 | `#5F87D7` | 95 | 135 | 215 | cornflower blue |
| 69 | `#5F87FF` | 95 | 135 | 255 | vivid cornflower |
| 70 | `#5FAF00` | 95 | 175 | 0 | yellow green |
| 71 | `#5FAF5F` | 95 | 175 | 95 | dark sea green |
| 72 | `#5FAF87` | 95 | 175 | 135 | medium aquamarine |
| 73 | `#5FAFAF` | 95 | 175 | 175 | cadet blue |
| 74 | `#5FAFD7` | 95 | 175 | 215 | sky blue |
| 75 | `#5FAFFF` | 95 | 175 | 255 | light sky blue |
| 76 | `#5FD700` | 95 | 215 | 0 | chartreuse |
| 77 | `#5FD75F` | 95 | 215 | 95 | pale green |
| 78 | `#5FD787` | 95 | 215 | 135 | light green |
| 79 | `#5FD7AF` | 95 | 215 | 175 | medium aquamarine |
| 80 | `#5FD7D7` | 95 | 215 | 215 | medium turquoise |
| 81 | `#5FD7FF` | 95 | 215 | 255 | light sky blue |
| 82 | `#5FFF00` | 95 | 255 | 0 | bright chartreuse |
| 83 | `#5FFF5F` | 95 | 255 | 95 | bright green |
| 84 | `#5FFF87` | 95 | 255 | 135 | light green |
| 85 | `#5FFFAF` | 95 | 255 | 175 | pale green |
| 86 | `#5FFFD7` | 95 | 255 | 215 | light aquamarine |
| 87 | `#5FFFFF` | 95 | 255 | 255 | light cyan |

### Red level 2 (R=135): colors 88-123
*Medium tones: dark reds, magentas, olives, muted greens*

| # | Hex | R | G | B | Description |
|--:|------|--:|--:|--:|-------------|
| 88 | `#870000` | 135 | 0 | 0 | dark red |
| 89 | `#87005F` | 135 | 0 | 95 | dark magenta |
| 90 | `#870087` | 135 | 0 | 135 | dark magenta |
| 91 | `#8700AF` | 135 | 0 | 175 | dark violet |
| 92 | `#8700D7` | 135 | 0 | 215 | blue violet |
| 93 | `#8700FF` | 135 | 0 | 255 | electric violet |
| 94 | `#875F00` | 135 | 95 | 0 | dark goldenrod |
| 95 | `#875F5F` | 135 | 95 | 95 | warm gray |
| 96 | `#875F87` | 135 | 95 | 135 | plum |
| 97 | `#875FAF` | 135 | 95 | 175 | medium purple |
| 98 | `#875FD7` | 135 | 95 | 215 | medium purple |
| 99 | `#875FFF` | 135 | 95 | 255 | light slate blue |
| 100 | `#878700` | 135 | 135 | 0 | olive |
| 101 | `#87875F` | 135 | 135 | 95 | dark khaki |
| 102 | `#878787` | 135 | 135 | 135 | gray |
| 103 | `#8787AF` | 135 | 135 | 175 | light slate gray |
| 104 | `#8787D7` | 135 | 135 | 215 | medium slate blue |
| 105 | `#8787FF` | 135 | 135 | 255 | light slate blue |
| 106 | `#87AF00` | 135 | 175 | 0 | yellow green |
| 107 | `#87AF5F` | 135 | 175 | 95 | dark olive green |
| 108 | `#87AF87` | 135 | 175 | 135 | dark sea green |
| 109 | `#87AFAF` | 135 | 175 | 175 | pale turquoise |
| 110 | `#87AFD7` | 135 | 175 | 215 | light steel blue |
| 111 | `#87AFFF` | 135 | 175 | 255 | light sky blue |
| 112 | `#87D700` | 135 | 215 | 0 | lawn green |
| 113 | `#87D75F` | 135 | 215 | 95 | light green |
| 114 | `#87D787` | 135 | 215 | 135 | pale green |
| 115 | `#87D7AF` | 135 | 215 | 175 | light aquamarine |
| 116 | `#87D7D7` | 135 | 215 | 215 | pale turquoise |
| 117 | `#87D7FF` | 135 | 215 | 255 | light blue |
| 118 | `#87FF00` | 135 | 255 | 0 | bright chartreuse |
| 119 | `#87FF5F` | 135 | 255 | 95 | bright green |
| 120 | `#87FF87` | 135 | 255 | 135 | light green |
| 121 | `#87FFAF` | 135 | 255 | 175 | pale green |
| 122 | `#87FFD7` | 135 | 255 | 215 | light aquamarine |
| 123 | `#87FFFF` | 135 | 255 | 255 | light cyan |

### Red level 3 (R=175): colors 124-159
*Warm mid tones: reds, pinks, oranges, golds, muted pastels*

| # | Hex | R | G | B | Description |
|--:|------|--:|--:|--:|-------------|
| 124 | `#AF0000` | 175 | 0 | 0 | red |
| 125 | `#AF005F` | 175 | 0 | 95 | medium violet red |
| 126 | `#AF0087` | 175 | 0 | 135 | deep pink |
| 127 | `#AF00AF` | 175 | 0 | 175 | magenta |
| 128 | `#AF00D7` | 175 | 0 | 215 | dark violet |
| 129 | `#AF00FF` | 175 | 0 | 255 | vivid violet |
| 130 | `#AF5F00` | 175 | 95 | 0 | dark orange / bronze |
| 131 | `#AF5F5F` | 175 | 95 | 95 | rosy brown |
| 132 | `#AF5F87` | 175 | 95 | 135 | pale violet red |
| 133 | `#AF5FAF` | 175 | 95 | 175 | orchid |
| 134 | `#AF5FD7` | 175 | 95 | 215 | medium orchid |
| 135 | `#AF5FFF` | 175 | 95 | 255 | medium purple |
| 136 | `#AF8700` | 175 | 135 | 0 | dark goldenrod |
| 137 | `#AF875F` | 175 | 135 | 95 | burlywood / tan |
| 138 | `#AF8787` | 175 | 135 | 135 | rosy brown |
| 139 | `#AF87AF` | 175 | 135 | 175 | thistle |
| 140 | `#AF87D7` | 175 | 135 | 215 | plum |
| 141 | `#AF87FF` | 175 | 135 | 255 | medium purple |
| 142 | `#AFAF00` | 175 | 175 | 0 | dark yellow / olive |
| 143 | `#AFAF5F` | 175 | 175 | 95 | dark khaki |
| 144 | `#AFAF87` | 175 | 175 | 135 | tan |
| 145 | `#AFAFAF` | 175 | 175 | 175 | silver |
| 146 | `#AFAFD7` | 175 | 175 | 215 | light steel blue |
| 147 | `#AFAFFF` | 175 | 175 | 255 | lavender |
| 148 | `#AFD700` | 175 | 215 | 0 | green yellow |
| 149 | `#AFD75F` | 175 | 215 | 95 | yellow green |
| 150 | `#AFD787` | 175 | 215 | 135 | light green |
| 151 | `#AFD7AF` | 175 | 215 | 175 | pale green |
| 152 | `#AFD7D7` | 175 | 215 | 215 | light cyan |
| 153 | `#AFD7FF` | 175 | 215 | 255 | light sky blue |
| 154 | `#AFFF00` | 175 | 255 | 0 | bright green yellow |
| 155 | `#AFFF5F` | 175 | 255 | 95 | bright yellow green |
| 156 | `#AFFF87` | 175 | 255 | 135 | pale green |
| 157 | `#AFFFAF` | 175 | 255 | 175 | light green |
| 158 | `#AFFFD7` | 175 | 255 | 215 | light aquamarine |
| 159 | `#AFFFFF` | 175 | 255 | 255 | pale cyan |

### Red level 4 (R=215): colors 160-195
*Bright warm tones: crimsons, pinks, oranges, golds, pastels*

| # | Hex | R | G | B | Description |
|--:|------|--:|--:|--:|-------------|
| 160 | `#D70000` | 215 | 0 | 0 | crimson |
| 161 | `#D7005F` | 215 | 0 | 95 | deep pink |
| 162 | `#D70087` | 215 | 0 | 135 | deep pink |
| 163 | `#D700AF` | 215 | 0 | 175 | hot pink |
| 164 | `#D700D7` | 215 | 0 | 215 | bright magenta |
| 165 | `#D700FF` | 215 | 0 | 255 | electric purple |
| 166 | `#D75F00` | 215 | 95 | 0 | rust / dark orange |
| 167 | `#D75F5F` | 215 | 95 | 95 | indian red |
| 168 | `#D75F87` | 215 | 95 | 135 | hot pink |
| 169 | `#D75FAF` | 215 | 95 | 175 | orchid |
| 170 | `#D75FD7` | 215 | 95 | 215 | violet |
| 171 | `#D75FFF` | 215 | 95 | 255 | medium orchid |
| 172 | `#D78700` | 215 | 135 | 0 | dark orange |
| 173 | `#D7875F` | 215 | 135 | 95 | sandy brown |
| 174 | `#D78787` | 215 | 135 | 135 | light coral |
| 175 | `#D787AF` | 215 | 135 | 175 | plum |
| 176 | `#D787D7` | 215 | 135 | 215 | violet |
| 177 | `#D787FF` | 215 | 135 | 255 | light purple |
| 178 | `#D7AF00` | 215 | 175 | 0 | goldenrod |
| 179 | `#D7AF5F` | 215 | 175 | 95 | dark khaki |
| 180 | `#D7AF87` | 215 | 175 | 135 | burlywood |
| 181 | `#D7AFAF` | 215 | 175 | 175 | misty rose |
| 182 | `#D7AFD7` | 215 | 175 | 215 | thistle |
| 183 | `#D7AFFF` | 215 | 175 | 255 | light plum |
| 184 | `#D7D700` | 215 | 215 | 0 | yellow |
| 185 | `#D7D75F` | 215 | 215 | 95 | khaki |
| 186 | `#D7D787` | 215 | 215 | 135 | pale goldenrod |
| 187 | `#D7D7AF` | 215 | 215 | 175 | lemon chiffon |
| 188 | `#D7D7D7` | 215 | 215 | 215 | light gray |
| 189 | `#D7D7FF` | 215 | 215 | 255 | lavender |
| 190 | `#D7FF00` | 215 | 255 | 0 | green yellow |
| 191 | `#D7FF5F` | 215 | 255 | 95 | light green yellow |
| 192 | `#D7FF87` | 215 | 255 | 135 | pale green |
| 193 | `#D7FFAF` | 215 | 255 | 175 | light green |
| 194 | `#D7FFD7` | 215 | 255 | 215 | honeydew |
| 195 | `#D7FFFF` | 215 | 255 | 255 | azure |

### Red level 5 (R=255): colors 196-231
*Vivid warm tones: scarlets, pinks, oranges, golds, bright pastels*

| # | Hex | R | G | B | Description |
|--:|------|--:|--:|--:|-------------|
| 196 | `#FF0000` | 255 | 0 | 0 | bright red |
| 197 | `#FF005F` | 255 | 0 | 95 | deep pink |
| 198 | `#FF0087` | 255 | 0 | 135 | deep pink |
| 199 | `#FF00AF` | 255 | 0 | 175 | hot pink |
| 200 | `#FF00D7` | 255 | 0 | 215 | bright magenta |
| 201 | `#FF00FF` | 255 | 0 | 255 | fuchsia |
| 202 | `#FF5F00` | 255 | 95 | 0 | orange red |
| 203 | `#FF5F5F` | 255 | 95 | 95 | salmon |
| 204 | `#FF5F87` | 255 | 95 | 135 | hot pink |
| 205 | `#FF5FAF` | 255 | 95 | 175 | pink |
| 206 | `#FF5FD7` | 255 | 95 | 215 | orchid |
| 207 | `#FF5FFF` | 255 | 95 | 255 | violet |
| 208 | `#FF8700` | 255 | 135 | 0 | dark orange |
| 209 | `#FF875F` | 255 | 135 | 95 | salmon |
| 210 | `#FF8787` | 255 | 135 | 135 | light coral |
| 211 | `#FF87AF` | 255 | 135 | 175 | light pink |
| 212 | `#FF87D7` | 255 | 135 | 215 | pink |
| 213 | `#FF87FF` | 255 | 135 | 255 | light orchid |
| 214 | `#FFAF00` | 255 | 175 | 0 | orange |
| 215 | `#FFAF5F` | 255 | 175 | 95 | sandy brown |
| 216 | `#FFAF87` | 255 | 175 | 135 | light salmon |
| 217 | `#FFAFAF` | 255 | 175 | 175 | light pink |
| 218 | `#FFAFD7` | 255 | 175 | 215 | pink |
| 219 | `#FFAFFF` | 255 | 175 | 255 | light plum |
| 220 | `#FFD700` | 255 | 215 | 0 | gold |
| 221 | `#FFD75F` | 255 | 215 | 95 | light goldenrod |
| 222 | `#FFD787` | 255 | 215 | 135 | khaki |
| 223 | `#FFD7AF` | 255 | 215 | 175 | peach puff |
| 224 | `#FFD7D7` | 255 | 215 | 215 | misty rose |
| 225 | `#FFD7FF` | 255 | 215 | 255 | lavender blush |
| 226 | `#FFFF00` | 255 | 255 | 0 | bright yellow |
| 227 | `#FFFF5F` | 255 | 255 | 95 | light yellow |
| 228 | `#FFFF87` | 255 | 255 | 135 | pale yellow |
| 229 | `#FFFFAF` | 255 | 255 | 175 | lemon chiffon |
| 230 | `#FFFFD7` | 255 | 255 | 215 | cornsilk |
| 231 | `#FFFFFF` | 255 | 255 | 255 | white (duplicate of 15) |

---

## Grayscale Ramp (232-255)

24 shades from near-black to near-white, evenly spaced.

| # | Hex | Intensity | Description |
|--:|------|----------:|-------------|
| 232 | `#080808` | 8 | near black |
| 233 | `#121212` | 18 | very dark gray |
| 234 | `#1C1C1C` | 28 | very dark gray |
| 235 | `#262626` | 38 | very dark gray |
| 236 | `#303030` | 48 | dark gray |
| 237 | `#3A3A3A` | 58 | dark gray |
| 238 | `#444444` | 68 | dark gray |
| 239 | `#4E4E4E` | 78 | dim gray |
| 240 | `#585858` | 88 | dim gray |
| 241 | `#626262` | 98 | dim gray |
| 242 | `#6C6C6C` | 108 | medium gray |
| 243 | `#767676` | 118 | medium gray |
| 244 | `#808080` | 128 | gray |
| 245 | `#8A8A8A` | 138 | gray |
| 246 | `#949494` | 148 | dark silver |
| 247 | `#9E9E9E` | 158 | dark silver |
| 248 | `#A8A8A8` | 168 | silver |
| 249 | `#B2B2B2` | 178 | silver |
| 250 | `#BCBCBC` | 188 | light gray |
| 251 | `#C6C6C6` | 198 | light gray |
| 252 | `#D0D0D0` | 208 | very light gray |
| 253 | `#DADADA` | 218 | very light gray |
| 254 | `#E4E4E4` | 228 | near white |
| 255 | `#EEEEEE` | 238 | near white |

---

## Usage Examples

### Status hilites

In `.evilhackrc` or OPTIONS file:

```
OPTIONS=hilite_status: hitpoints/<50%/202          # orange-red
OPTIONS=hilite_status: hitpoints/<25%/196          # bright red
OPTIONS=hilite_status: gold/up/220                 # gold
OPTIONS=hilite_status: power/up/34&bold            # green + bold
OPTIONS=hilite_status: experience-level/up/51      # cyan
```

### Condition hilites

```
OPTIONS=hilite_status: condition/blind/196          # bright red
OPTIONS=hilite_status: condition/poisoned/82&blink  # chartreuse + blink
OPTIONS=hilite_status: condition/hallu/170           # violet
```

### Menu colors

```
MENUCOLOR="blessed"=34                              # green
MENUCOLOR="cursed"=196                              # bright red
MENUCOLOR="holy water"=45                           # deep sky blue
MENUCOLOR="gold piece"=220                          # gold
MENUCOLOR="loadstone"=94                            # dark goldenrod
```

### Interactive menu

Press `O` in-game, navigate to status hilites, and choose
"custom color (0-255)" from the color picker to enter any
number directly.

### Combining with attributes

Colors can be combined with `bold`, `blink`, `uline`, `inverse`, `dim`
using `&`:

```
OPTIONS=hilite_status: hitpoints/<10%/196&bold&blink
MENUCOLOR="uncursed"=100&bold
```

---

## Tips

- To see all 256 colors in your terminal, run:
  ```
  for i in $(seq 0 255); do
    printf "\033[38;5;%dm%4d " $i $i
    [ $(( (i+1) % 16 )) -eq 0 ] && printf "\033[0m\n"
  done
  ```

- Colors 16-231 form a 6x6x6 cube. To find a color by RGB:
  `number = 16 + (36 * R) + (6 * G) + B`
  where R, G, B are 0-5 (mapping to 0, 95, 135, 175, 215, 255).

- If your terminal shows wrong colors, check:
  ```
  echo $TERM        # should contain "256color"
  echo $COLORTERM   # should be set (e.g., "truecolor")
  tput colors       # should print 256 or higher
  ```

- Inside tmux/screen, use `TERM=tmux-256color` or `screen-256color`.

- Old configs with named colors (red, blue, etc.) continue to
  work exactly as before. Extended colors are purely additive.
