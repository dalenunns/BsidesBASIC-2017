09      led brightness 50
10      for l = 0 to 4
20      let x = l
30      led on,x,rgb(255,0,0)
40      let x = 9 - l
50      led on,x,rgb(255,0,0)
60      let x = 10 + l
70      led on,x,rgb(255,0,0)
80      sleep 35
90      if l = 0 then goto 190
120     let x = l - 1
130     led off,x
140     let x = 10 - l
150     led off,x
160     let x = 9 + l
170     led off,x
180     sleep 35
190     next l
200     sleep 0
210     for l = 1 to 4
220     let x = 4 - l
230     led on,x,rgb(255,0,0)
240     let x = 5 + l
250     led on,x,rgb(255,0,0)
260     let x = 14 - l
270     led on,x,rgb(255,0,0)
280     sleep 35
290     if l = 0 then goto 370
300     let x = 5 - l
310     led off,x
320     let x = 4 + l
330     led off,x
340     let x = 15 - l
350     led off,x
360     sleep 35
370     next l
390     sleep 0
400     goto 10