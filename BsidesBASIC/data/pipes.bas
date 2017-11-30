10   rem startup
20   cls
30   let width = 77
40   let height = 37
50   let direction = 1
55   let olddirection = 1
60   let colourduration = 66
65   let col = 8
70   let x = rnd / 1500 * width
80   let y = rnd / 1500 * height
90   move x, y
100  let colourduration = colourduration - 1
101  if colourduration > 1 then goto 110
102  let changecolour = rnd
103  if changecolour > 500 then goto 110
104  if changecolour < 251 then let col = col - 1
105  if changecolour > 250 then let col = col + 1
106  if col > 12 then let col = 8
107  if col < 8 then let col = 12
108  set colour col
109  let colourduration = 66
110  if direction = olddirection then goto 116
111  rem (try and) draw corners
112  goto 500
113  let olddirection = direction
114  goto 120
115  rem
116  if direction = 1 then print "═"
117  if direction = 2 then print "║"
118  if direction = 3 then print "═"
119  if direction = 4 then print "║"
120  REM always turn if near screen edge
130  if x > width then goto 210
140  if x < 2 then goto 210
150  if y < 2 then goto 210
160  if y > height then goto 210
170  REM decide if we should turn
180  let turn = rnd
190  if turn > 840 then goto 210
200  GOTO 390
210  REM XXX decide which direction to turn
220  if direction = 1 then goto 260
230  if direction = 2 then goto 320
240  if direction = 3 then goto 260
250  if direction = 4 then goto 320
260  REM AAA can only turn up or down
270  let direction = 4
275  let updown = rnd
280  if updown > 500 then let direction = 2
290  if y < 2 then let direction = 2
300  if y > height then let direction = 4
310  GOTO 390
320  REM BBB can only turn left or right
330  let direction = 1
335  let leftright = rnd
340  if leftright > 500 then let direction = 3
350  if x < 2 then let direction = 1
360  if x > width then let direction = 3
370  GOTO 390
380  REM YYY set new position
390  if direction = 1 then let x = x + 1
400  if direction = 2 then let y = y + 1
410  if direction = 3 then let x = x - 1
420  if direction = 4 then let y = y - 1
430  sleep 33
440  GOTO 90
500  rem trying to draw corners
510  let join = olddirection * 10
520  let join = join + direction
530  if join = 12 then let y = y - 1
531  if join = 12 then move x, y
532  if join = 12 then print "╗"
540  if join = 14 then let y = y + 1
541  if join = 14 then move x, y
542  if join = 14 then print "╝"
550  if join = 21 then let x = x - 1
551  if join = 21 then move x, y
552  if join = 21 then print "╚"
560  if join = 23 then let x = x + 1
561  if join = 23 then move x, y
562  if join = 23 then print "╝"
570  if join = 32 then let y = y - 1
571  if join = 32 then move x, y
572  if join = 32 then print "╔"
580  if join = 34 then let y = y + 1
581  if join = 34 then move x, y
582  if join = 34 then print "╚"
590  if join = 41 then let x = x - 1
591  if join = 41 then move x, y
592  if join = 41 then print "╔"
600  if join = 43 then let x = x + 1
601  if join = 43 then move x, y
602  if join = 43 then print "╗"
610  let olddirection = direction
620  goto 390
630  rem
640  rem "pipes.bas" by @HypnZA
650  rem
