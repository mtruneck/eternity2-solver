# eternity2-solver
My playground for Eternity II Puzzle solver

I took the naive approach of placing one piece after another in defined order. When there is no option for the next position, fall back to the previous one and choose another piece. After `FALLBACKS` number of fallbacks, it prints the maximum reached position and prints the current board. Moreover, if the maximum position is above 190, it prints the board with every fallback peak.

## Optimizations

1. The biggest one is using the precomputed arrays of fitting pieces. With that, I got to 0.06 s for 1,000,000 fallbacks.

## Best solution found so far - 213 pieces placed

```
   1  49  22   5   6  21  42  13  14  18  58  48  34  31  28   0
  40 111 214 113 181 131 109 193 108 171 143  81  89  88  60   4
  29 235 207 199 240 127 116  71 161 184 149 151 170 254  87  24
  53 250 228 216  80  83  99  64 157  70  65 185 158 162  75  39
  45  97  79 211  73 222 215 137 121 112 176 165 130 114 201  47
  38 196 203 217  --  --  --  --  --  --  -- 187 245 142  68  46
  56  86  94 237  --  --  --  --  --  --  -- 188 153 179  77   9
  10 173 191 236  --  --  --  --  --  --  -- 206 146 156 145  36
  19 167  76  85  --  --  -- 138  --  --  -- 194 147 212 124  35
  15  66 117 213  --  --  --  --  --  --  -- 251  74  63  62  27
  25  61 204 135  --  --  --  --  --  --  -- 110 225 136 148  43
  23 244 128 132  --  84  92 209 174 140 168 122 232 238 242  52
  11 255 253 229 105 139 154 223  82  67 197 200 189 120 119  12
  37 219 180 166 190 126 118 252 247 141 144 218 226 248 246  41
  50 129 239 210 243 249 202 227 133 106 234 169 177 224  72  20
   2  32  17  57  55  30  16  54  59  33  44   7  51   8  26   3
```
