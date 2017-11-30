10   led brightness 0
20   led fill rgb(0,0,255)
30   for i = 0 to 50
40   led brightness i
50   sleep 20
60   next i
70   sleep 50
80   for c = 50 to 0 step -1
90   led brightness c
100  sleep 20
110  next c
120  led fill rgb(0,0,0)
130  sleep 250
140  goto 10
150  rem
160  rem "bluepulse.bas" by @dale_nunns and @HypnZA
170  rem
