5 RANDOMIZE TIMER
10 A = 25
20 CLS
25 FOR B = 1 TO A
30 PRINT "* ";
35 NEXT B
37 PRINT
40 PRINT "HOW MANY TO TAKE AWAY"
45 INPUT C
50 IF C > 3 THEN PRINT "YOU CAN ONLY TAKE AWAY 1,2, OR 3 STONES.": GOTO 40
60 A = A - C
65 IF A = 0 THEN PRINT "YOU WIN!": END
70 D = (A MOD 4)
75 IF D = 0 THEN D = INT(3 * RND(1)) + 1
80 PRINT "I TOOK AWAY "; D; "STONES"
83 SLEEP
85 A = A - D
90 IF A = 0 THEN PRINT "I WIN": END
100 GOTO 20