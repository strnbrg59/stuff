/* standard.h     standard definitions for IBM PC */

#define TRUE	 1
#define FALSE	 0
#define CR      13
#define LF      10
#define FF	12
#define BELL     7
#define ESC	27
#define TABSPACE 8

#define BACKSP	8	/* keypress codes */
#define CTLC	3 
#define TAB	9
#define KUP 	72
#define	KDOWN	80
#define KLEFT	75
#define KRIGHT	77
#define KHOME	71
#define KEND	79
#define KPGUP	73
#define KPGDN	81
#define KDEL	83
#define KINS	82
#define KPLUS	78


#define F1	59	/* function key codes */
#define F2	60
#define F3	61
#define F4	62
#define	F5	63
#define F6	64
#define F7	65
#define	F8	66
#define	F9	67
#define	F10	68

#define SCRNWIDE 80	/* width and hight of screen in chars */
#define SCRNTALL 24	
			/* codes for screen char attributes */

#define SAMEC 0		/* same char type as existing spot on screen */
#define BWC   1		/* black and white */
#define ULC   2		/* underlined */
#define RVC   3		/* reverse video */
#define BRVC  4		/* blinking reverse video */

