# eternity2-solver
My playground for Eternity II Puzzle solver

I took the naive approach of placing one piece after another in defined order. When there is no option for the next position, fall back to the previous one and choose another piece. After `FALLBACKS` number of fallbacks, it prints the maximum reached position and prints the current board. Moreover, if the maximum position is above 190, it prints the board with every fallback peak.

## Optimizations

1. The biggest one is using the precomputed arrays of fitting pieces. With that, I got to 0.06 s for 1,000,000 fallbacks.

## Best solution found so far - 213 pieces placed

```
   2  50  57  22  25  26  43  52  48  29  15  42  13  49  17   3
  35 120 163 145  60  61 190 210  77 154  64 195 204 116 152  28
  45 122 207 206 103 118 211 172 208 218  78 226 217 254 223   8
  38  66  73 157 121 252 212 176 243 245  86  74  93 255 229  23
  59 150  76  65 126 128 169 165 247 156 114 111  87 194 132   5
  31  89 127 187  --  --  --  --  --  --  -- 236  92  85  96  44
  30  91 105 164  --  --  --  --  --  --  -- 202 232 237 234  20
  16 186 215 251  --  --  --  --  --  --  --  94  88 142 242  19
  54 155 222 230  --  --  -- 138  --  --  -- 250 214 146 246  10
  56 153 209 238  --  --  --  --  --  --  -- 104 148 136 239  41
  12 119  62  99  --  --  --  --  --  --  -- 110 144 235 197  46
  58 249 135 205  --  --  84 141 137 221 241 231 140 106 200  47
  14 129 253 159 139 130 109 191 160 201 244 228 216 107 181   9
   4  90 180 178 124 220 131 182 149 193 173 170 213 248 171  51
   6 175 240 225 115 166 134  68  63  95 125 102 199  75  79  34
   1  40  32  18  21   7  53  36  11  37  55  27  33  39  24   0
```
