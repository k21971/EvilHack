# EvilHack 256-Color Reference

Players can use any color number 0-255 in status hilites, condition
hilites, and menu colors. Terminals must support 256-color mode
(`$TERM` containing "256color", or `$COLORTERM` set) for extended colors
16-255 to render. On older terminals, extended colors automatically
fall back to the closest base-16 match.

---

## Base Colors (0-15)

These are the original named colors. Use either the name or number.

| # | Name | Aliases |
|--:|------|---------|
| 0 | black | |
| 1 | red | |
| 2 | green | |
| 3 | brown | |
| 4 | blue | |
| 5 | magenta | purple |
| 6 | cyan | |
| 7 | gray | grey |
| 8 | no color | transparent (also NO_COLOR) |
| 9 | orange | light red, bright red |
| 10 | light green | bright green |
| 11 | yellow | |
| 12 | light blue | bright blue |
| 13 | light magenta | light purple, bright magenta |
| 14 | light cyan | bright cyan |
| 15 | white | |

---

## Extended Palette (16-231) - 6x6x6 RGB Cube

Each color is built from Red, Green, and Blue components at six
intensity levels: 0, 95, 135, 175, 215, 255.

**Formula:** `color = 16 + (36 * R_level) + (6 * G_level) + B_level`
where R/G/B_level are 0-5, mapping to intensities above.

### Red level 0 (R=0): colors 16-51
*Cool tones: greens, blues, cyans (no red component)*

| # | R | G | B | Description |
|--:|--:|--:|--:|-------------|
| 16 | 0 | 0 | 0 | black (duplicate of 0) |
| 17 | 0 | 0 | 95 | very dark blue |
| 18 | 0 | 0 | 135 | dark blue |
| 19 | 0 | 0 | 175 | medium blue |
| 20 | 0 | 0 | 215 | blue |
| 21 | 0 | 0 | 255 | bright blue |
| 22 | 0 | 95 | 0 | very dark green |
| 23 | 0 | 95 | 95 | dark teal |
| 24 | 0 | 95 | 135 | deep teal blue |
| 25 | 0 | 95 | 175 | steel blue |
| 26 | 0 | 95 | 215 | dodger blue |
| 27 | 0 | 95 | 255 | deep blue |
| 28 | 0 | 135 | 0 | forest green |
| 29 | 0 | 135 | 95 | sea green |
| 30 | 0 | 135 | 135 | teal |
| 31 | 0 | 135 | 175 | cerulean |
| 32 | 0 | 135 | 215 | azure |
| 33 | 0 | 135 | 255 | deep sky blue |
| 34 | 0 | 175 | 0 | green |
| 35 | 0 | 175 | 95 | medium sea green |
| 36 | 0 | 175 | 135 | medium aquamarine |
| 37 | 0 | 175 | 175 | light sea green |
| 38 | 0 | 175 | 215 | deep sky blue |
| 39 | 0 | 175 | 255 | vivid sky blue |
| 40 | 0 | 215 | 0 | lime green |
| 41 | 0 | 215 | 95 | spring green |
| 42 | 0 | 215 | 135 | medium spring green |
| 43 | 0 | 215 | 175 | aquamarine |
| 44 | 0 | 215 | 215 | dark turquoise |
| 45 | 0 | 215 | 255 | deep sky blue |
| 46 | 0 | 255 | 0 | lime |
| 47 | 0 | 255 | 95 | bright spring green |
| 48 | 0 | 255 | 135 | spring green |
| 49 | 0 | 255 | 175 | medium spring green |
| 50 | 0 | 255 | 215 | bright aquamarine |
| 51 | 0 | 255 | 255 | cyan / aqua |

### Red level 1 (R=95): colors 52-87
*Muted tones: dark reds, purples, olives, dusty greens*

| # | R | G | B | Description |
|--:|--:|--:|--:|-------------|
| 52 | 95 | 0 | 0 | dark maroon |
| 53 | 95 | 0 | 95 | dark purple |
| 54 | 95 | 0 | 135 | indigo |
| 55 | 95 | 0 | 175 | blue violet |
| 56 | 95 | 0 | 215 | vivid blue violet |
| 57 | 95 | 0 | 255 | electric indigo |
| 58 | 95 | 95 | 0 | dark olive |
| 59 | 95 | 95 | 95 | dim gray |
| 60 | 95 | 95 | 135 | slate gray |
| 61 | 95 | 95 | 175 | medium slate blue |
| 62 | 95 | 95 | 215 | cornflower blue |
| 63 | 95 | 95 | 255 | royal blue |
| 64 | 95 | 135 | 0 | olive green |
| 65 | 95 | 135 | 95 | dark sea green |
| 66 | 95 | 135 | 135 | cadet blue |
| 67 | 95 | 135 | 175 | steel blue |
| 68 | 95 | 135 | 215 | cornflower blue |
| 69 | 95 | 135 | 255 | vivid cornflower |
| 70 | 95 | 175 | 0 | yellow green |
| 71 | 95 | 175 | 95 | dark sea green |
| 72 | 95 | 175 | 135 | medium aquamarine |
| 73 | 95 | 175 | 175 | cadet blue |
| 74 | 95 | 175 | 215 | sky blue |
| 75 | 95 | 175 | 255 | light sky blue |
| 76 | 95 | 215 | 0 | chartreuse |
| 77 | 95 | 215 | 95 | pale green |
| 78 | 95 | 215 | 135 | light green |
| 79 | 95 | 215 | 175 | medium aquamarine |
| 80 | 95 | 215 | 215 | medium turquoise |
| 81 | 95 | 215 | 255 | light sky blue |
| 82 | 95 | 255 | 0 | bright chartreuse |
| 83 | 95 | 255 | 95 | bright green |
| 84 | 95 | 255 | 135 | light green |
| 85 | 95 | 255 | 175 | pale green |
| 86 | 95 | 255 | 215 | light aquamarine |
| 87 | 95 | 255 | 255 | light cyan |

### Red level 2 (R=135): colors 88-123
*Medium tones: dark reds, magentas, olives, muted greens*

| # | R | G | B | Description |
|--:|--:|--:|--:|-------------|
| 88 | 135 | 0 | 0 | dark red |
| 89 | 135 | 0 | 95 | dark magenta |
| 90 | 135 | 0 | 135 | dark magenta |
| 91 | 135 | 0 | 175 | dark violet |
| 92 | 135 | 0 | 215 | blue violet |
| 93 | 135 | 0 | 255 | electric violet |
| 94 | 135 | 95 | 0 | dark goldenrod |
| 95 | 135 | 95 | 95 | warm gray |
| 96 | 135 | 95 | 135 | plum |
| 97 | 135 | 95 | 175 | medium purple |
| 98 | 135 | 95 | 215 | medium purple |
| 99 | 135 | 95 | 255 | light slate blue |
| 100 | 135 | 135 | 0 | olive |
| 101 | 135 | 135 | 95 | dark khaki |
| 102 | 135 | 135 | 135 | gray |
| 103 | 135 | 135 | 175 | light slate gray |
| 104 | 135 | 135 | 215 | medium slate blue |
| 105 | 135 | 135 | 255 | light slate blue |
| 106 | 135 | 175 | 0 | yellow green |
| 107 | 135 | 175 | 95 | dark olive green |
| 108 | 135 | 175 | 135 | dark sea green |
| 109 | 135 | 175 | 175 | pale turquoise |
| 110 | 135 | 175 | 215 | light steel blue |
| 111 | 135 | 175 | 255 | light sky blue |
| 112 | 135 | 215 | 0 | lawn green |
| 113 | 135 | 215 | 95 | light green |
| 114 | 135 | 215 | 135 | pale green |
| 115 | 135 | 215 | 175 | light aquamarine |
| 116 | 135 | 215 | 215 | pale turquoise |
| 117 | 135 | 215 | 255 | light blue |
| 118 | 135 | 255 | 0 | bright chartreuse |
| 119 | 135 | 255 | 95 | bright green |
| 120 | 135 | 255 | 135 | light green |
| 121 | 135 | 255 | 175 | pale green |
| 122 | 135 | 255 | 215 | light aquamarine |
| 123 | 135 | 255 | 255 | light cyan |

### Red level 3 (R=175): colors 124-159
*Warm mid tones: reds, pinks, oranges, golds, muted pastels*

| # | R | G | B | Description |
|--:|--:|--:|--:|-------------|
| 124 | 175 | 0 | 0 | red |
| 125 | 175 | 0 | 95 | medium violet red |
| 126 | 175 | 0 | 135 | deep pink |
| 127 | 175 | 0 | 175 | magenta |
| 128 | 175 | 0 | 215 | dark violet |
| 129 | 175 | 0 | 255 | vivid violet |
| 130 | 175 | 95 | 0 | dark orange / bronze |
| 131 | 175 | 95 | 95 | rosy brown |
| 132 | 175 | 95 | 135 | pale violet red |
| 133 | 175 | 95 | 175 | orchid |
| 134 | 175 | 95 | 215 | medium orchid |
| 135 | 175 | 95 | 255 | medium purple |
| 136 | 175 | 135 | 0 | dark goldenrod |
| 137 | 175 | 135 | 95 | burlywood / tan |
| 138 | 175 | 135 | 135 | rosy brown |
| 139 | 175 | 135 | 175 | thistle |
| 140 | 175 | 135 | 215 | plum |
| 141 | 175 | 135 | 255 | medium purple |
| 142 | 175 | 175 | 0 | dark yellow / olive |
| 143 | 175 | 175 | 95 | dark khaki |
| 144 | 175 | 175 | 135 | tan |
| 145 | 175 | 175 | 175 | silver |
| 146 | 175 | 175 | 215 | light steel blue |
| 147 | 175 | 175 | 255 | lavender |
| 148 | 175 | 215 | 0 | green yellow |
| 149 | 175 | 215 | 95 | yellow green |
| 150 | 175 | 215 | 135 | light green |
| 151 | 175 | 215 | 175 | pale green |
| 152 | 175 | 215 | 215 | light cyan |
| 153 | 175 | 215 | 255 | light sky blue |
| 154 | 175 | 255 | 0 | bright green yellow |
| 155 | 175 | 255 | 95 | bright yellow green |
| 156 | 175 | 255 | 135 | pale green |
| 157 | 175 | 255 | 175 | light green |
| 158 | 175 | 255 | 215 | light aquamarine |
| 159 | 175 | 255 | 255 | pale cyan |

### Red level 4 (R=215): colors 160-195
*Bright warm tones: crimsons, pinks, oranges, golds, pastels*

| # | R | G | B | Description |
|--:|--:|--:|--:|-------------|
| 160 | 215 | 0 | 0 | crimson |
| 161 | 215 | 0 | 95 | deep pink |
| 162 | 215 | 0 | 135 | deep pink |
| 163 | 215 | 0 | 175 | hot pink |
| 164 | 215 | 0 | 215 | bright magenta |
| 165 | 215 | 0 | 255 | electric purple |
| 166 | 215 | 95 | 0 | rust / dark orange |
| 167 | 215 | 95 | 95 | indian red |
| 168 | 215 | 95 | 135 | hot pink |
| 169 | 215 | 95 | 175 | orchid |
| 170 | 215 | 95 | 215 | violet |
| 171 | 215 | 95 | 255 | medium orchid |
| 172 | 215 | 135 | 0 | dark orange |
| 173 | 215 | 135 | 95 | sandy brown |
| 174 | 215 | 135 | 135 | light coral |
| 175 | 215 | 135 | 175 | plum |
| 176 | 215 | 135 | 215 | violet |
| 177 | 215 | 135 | 255 | light purple |
| 178 | 215 | 175 | 0 | goldenrod |
| 179 | 215 | 175 | 95 | dark khaki |
| 180 | 215 | 175 | 135 | burlywood |
| 181 | 215 | 175 | 175 | misty rose |
| 182 | 215 | 175 | 215 | thistle |
| 183 | 215 | 175 | 255 | light plum |
| 184 | 215 | 215 | 0 | yellow |
| 185 | 215 | 215 | 95 | khaki |
| 186 | 215 | 215 | 135 | pale goldenrod |
| 187 | 215 | 215 | 175 | lemon chiffon |
| 188 | 215 | 215 | 215 | light gray |
| 189 | 215 | 215 | 255 | lavender |
| 190 | 215 | 255 | 0 | green yellow |
| 191 | 215 | 255 | 95 | light green yellow |
| 192 | 215 | 255 | 135 | pale green |
| 193 | 215 | 255 | 175 | light green |
| 194 | 215 | 255 | 215 | honeydew |
| 195 | 215 | 255 | 255 | azure |

### Red level 5 (R=255): colors 196-231
*Vivid warm tones: scarlets, pinks, oranges, golds, bright pastels*

| # | R | G | B | Description |
|--:|--:|--:|--:|-------------|
| 196 | 255 | 0 | 0 | bright red |
| 197 | 255 | 0 | 95 | deep pink |
| 198 | 255 | 0 | 135 | deep pink |
| 199 | 255 | 0 | 175 | hot pink |
| 200 | 255 | 0 | 215 | bright magenta |
| 201 | 255 | 0 | 255 | fuchsia |
| 202 | 255 | 95 | 0 | orange red |
| 203 | 255 | 95 | 95 | salmon |
| 204 | 255 | 95 | 135 | hot pink |
| 205 | 255 | 95 | 175 | pink |
| 206 | 255 | 95 | 215 | orchid |
| 207 | 255 | 95 | 255 | violet |
| 208 | 255 | 135 | 0 | dark orange |
| 209 | 255 | 135 | 95 | salmon |
| 210 | 255 | 135 | 135 | light coral |
| 211 | 255 | 135 | 175 | light pink |
| 212 | 255 | 135 | 215 | pink |
| 213 | 255 | 135 | 255 | light orchid |
| 214 | 255 | 175 | 0 | orange |
| 215 | 255 | 175 | 95 | sandy brown |
| 216 | 255 | 175 | 135 | light salmon |
| 217 | 255 | 175 | 175 | light pink |
| 218 | 255 | 175 | 215 | pink |
| 219 | 255 | 175 | 255 | light plum |
| 220 | 255 | 215 | 0 | gold |
| 221 | 255 | 215 | 95 | light goldenrod |
| 222 | 255 | 215 | 135 | khaki |
| 223 | 255 | 215 | 175 | peach puff |
| 224 | 255 | 215 | 215 | misty rose |
| 225 | 255 | 215 | 255 | lavender blush |
| 226 | 255 | 255 | 0 | bright yellow |
| 227 | 255 | 255 | 95 | light yellow |
| 228 | 255 | 255 | 135 | pale yellow |
| 229 | 255 | 255 | 175 | lemon chiffon |
| 230 | 255 | 255 | 215 | cornsilk |
| 231 | 255 | 255 | 255 | white (duplicate of 15) |

---

## Grayscale Ramp (232-255)

24 shades from near-black to near-white, evenly spaced.

| # | Intensity | Description |
|--:|----------:|-------------|
| 232 | 8 | near black |
| 233 | 18 | very dark gray |
| 234 | 28 | very dark gray |
| 235 | 38 | very dark gray |
| 236 | 48 | dark gray |
| 237 | 58 | dark gray |
| 238 | 68 | dark gray |
| 239 | 78 | dim gray |
| 240 | 88 | dim gray |
| 241 | 98 | dim gray |
| 242 | 108 | medium gray |
| 243 | 118 | medium gray |
| 244 | 128 | gray |
| 245 | 138 | gray |
| 246 | 148 | dark silver |
| 247 | 158 | dark silver |
| 248 | 168 | silver |
| 249 | 178 | silver |
| 250 | 188 | light gray |
| 251 | 198 | light gray |
| 252 | 208 | very light gray |
| 253 | 218 | very light gray |
| 254 | 228 | near white |
| 255 | 238 | near white |

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
