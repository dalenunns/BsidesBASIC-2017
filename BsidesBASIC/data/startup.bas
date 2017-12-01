03 rem ##########################################
04 rem ###              STARTUP               ###
05 rem ##########################################
06 for cycle = 0 to 19
07 let bright = cycle * 2
08 let bright = bright + 15
09 led brightness bright
10 for l = 0 to 4
20 let x = 4 - l
30 led on,x,rgb(255,255,255)
40 let x = 5 + l
50 led on,x,rgb(255,255,255)
60 let x = 14 - l
70 led on,x,rgb(255,255,255)
75 if cycle < 10 then let delay = 200 - 20 * cycle
76 if cycle > 9 then let delay = 0
80 sleep delay
90 if l = 0 then goto 170
100 let x = 5 - l
110 led off,x
120 let x = 4 + l
130 led off,x
140 let x = 15 - l
150 led off,x
160 sleep 0
170 next l
180 led off,0
190 led off,9
200 led off,10
210 next cycle
220 rem ##########################################
225 rem ###            POWERED UP!             ###
230 rem ##########################################
240 led brightness 100
250 led fill rgb(255,255,255)
255 for cycle = 0 to 25
260 for b = 255 to 0 step -10
261 if b > 100 then let r = b
262 if b < 101 then let r = 255 - b
263 let g = b
270 led fill rgb(r,g,b)
290 next b
300 for b = 0 to 255 step 10
310 led fill rgb(255,255,b)
320 next b
330 next cycle
340 rem ##########################################
350 rem ####          IDLE ANIMATION:          ###
360 rem ##########################################
370 led brightness 10
410 for l = 0 to 4
420 let x = 4 - l
430 led on,x,rgb(255,255,255)
440 let x = 5 + l
450 led on,x,rgb(255,255,255)
460 let x = 14 - l
470 led on,x,rgb(255,255,255)
480 sleep 120
490 if l = 0 then goto 570
500 let x = 5 - l
510 led off,x
520 let x = 4 + l
530 led off,x
540 let x = 15 - l
550 led off,x
560 sleep 0
570 next l
580 led off,0
590 led off,9
600 led off,10
610 sleep 2000
620 goto 410
999 end
