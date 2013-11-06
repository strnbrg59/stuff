import pg
con = pg.connect('managed_target_stats', 'localhost', 5432, None, None, 'postgres', None)
db = pg.DB(con)
print "con is ready, db is ready"

print db.get_databases()

for t in db.get_tables():
        if t.count('86400'):
                print t

