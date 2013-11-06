import pg
import time
con = pg.connect('managed_target_stats', 'localhost', 5432, None, None, 'postgres', None)
db = pg.DB(con)
print "con is ready, db is ready"

tables = db.get_tables()

n_tables=0
n_removed=0
now = int(time.time())
for t in tables:
    n_tables += 1
    if t.count('public.stats_'):
        pieces = t.split('_')
        if len(pieces) < 3:
            continue
        starttime = int(pieces[2])
        if starttime > now:
            con.query("DROP TABLE " + t)
            n_removed += 1
            

print "n_tables =", n_tables, ", n_removed =", n_removed

                 

