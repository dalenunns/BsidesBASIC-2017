09   led brightness 50
10   let r = 255
20   let g = 0
30   let b = 0
40   led fill rgb(r,g,b)
50   for i = 0 to 254
60   let r = r - 1
70   let g = g + 1
80   led fill rgb(r,g,b)
90   next i
100  for i = 0 to 254
110  let g = g - 1
120  let b = b + 1
130  led fill rgb(r,g,b)
140  next i
150  for i = 0 to 254
160  let r = r + 1
170  let g = g + 1
180  let b = b - 1
190  led fill rgb(r,g,b)
200  next i
210  for i = 0 to 254
220  let r = r - 1
230  let b = b + 1
240  led fill rgb(r,g,b)
250  next i
260  for i = 0 to 254
270  let r = r + 1
280  let g = g - 1
290  let b = b - 1
300  led fill rgb(r,g,b)
310  next i
320  goto 10
330  rem
340  rem "rgb.bas" by @HypnZA
350  rem
