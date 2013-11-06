all :
	(cd server; make)
	(cd client; make)
#	documentation

clean :
	-rm `find . ! -type d -name "*.class"`
	-rm `find . ! -type d -name "*~"`

backup :
	tar cvf backup.tar `find . ! -type d | grep -v "/bak/" | grep -v '\.html'$ | grep -v '\.gif'$ `

documentation :
	javadoc -package -d doc jqc.client jqc.client.GUI jqc.server
