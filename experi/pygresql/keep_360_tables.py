# Throw away anything in excess of 360 stats_x_y_z tables.

import pg
con = pg.connect('managed_target_stats', 'localhost', 5432, None, None, 'postgres', None)
db = pg.DB(con)

tables = db.get_tables()

n_stats_tables=0
for t in tables:
    pieces = t.split('_')
    if (t.count('public.stats_') > 0) and (len(pieces) > 2):
        n_stats_tables += 1
        if n_stats_tables > 360:
            con.query("DROP TABLE " + t)
