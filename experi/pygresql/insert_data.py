# Inserts some data into every single stats table (except the "current" one that's still
# expanding).

import time
import pg
con = pg.connect('managed_target_stats', 'localhost', 5432, None, None, 'postgres', None)
db = pg.DB(con)

for table in db.get_tables():
        print "table", table
        if table.count('stats_'):
                pieces = table.split('_')
                if len(pieces) == 2:
                        continue
                starttime = int(pieces[2])
                endtime   = int(pieces[3])
                incr = (endtime - starttime)/10
                for i in range(0,10):
                        con.inserttable(table, [(51, 0, starttime + i * incr,
                                11083, 1234 * i * 1000,
                                456 * i * 1000,
                                789 * i * 1000,
                                123 * i * 1000),])
