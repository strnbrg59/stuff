import chombovis
c=chombovis.this()

def diff(x,y): return 10*x-y
c.reader.defineNewComponent( 'mydiff', diff, ('phi','rhs'))
c.reader.setCurrentComponent( 'mydiff' )
#c.cmap.showGUI()
