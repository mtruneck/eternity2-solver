# eternity2-solver
My playground for Eternity II Puzzle solver

I took the naive approach of placing one piece after another in defined order. When there is no option for the next position, fall back to the previous one and choose another piece. After `FALLBACKS` number of fallbacks, it prints the maximum reached position and prints the current board. Moreover, if the maximum position is above 190, it prints the board with every fallback peak.

## Optimizations

1. The biggest one is using the precomputed arrays of fitting pieces. With that, I got to 0.06 s for 1,000,000 fallbacks.

## Best solution found so far - 215 pieces placed

```
   0  41  45  53  17  50  48  54   7  24  39   9   6  36  47   1
  25  80 238 247 202 131  79 244 103 130 168 233 170 156  89   4
  22  83 207 164 181 155 161 197 102 122 205 163 253 254 198  23
  57  77 223  71  66 120 196  74 217 234  93  68 215 183 187  58
  56 211 191  84  85 124 132  65 145 153 250 212  94 182 189  29
   5 112 123 106  --  --  --  --  --  --  -- 221  88 243 210  43
  18 179 176 199  --  --  --  --  --  --  --  95 166 167 105  11
  20 237 235 206  --  --  --  --  --  --  --  70  69 225 100  49
  46 177  86  98  --  --  -- 138  --  --  -- 154  63 229 252  12
  42 178 230  67  --  --  --  --  --  --  --  92  60 121 128  44
  19 119 219 157  97  --  --  --  --  --  -- 251 148 111 209  52
  21 109 144 174 194 188 214 241 236 203  61  82  72 133 222  38
  10 242 142 139 226 152 146 159 249 239  73 218  62 113 193  13
  15 151 180 158 228 245 220 255 116 117 216 127 101 248 171  51
  32  90 231 208 162 160 143 200 185 125 172 108 224 246 186  16
   2  26  33  37  59  31  28  34  55  27  30  14   8  35  40   3
```
