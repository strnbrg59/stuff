TARGET := hello

SOURCES := hello.c

${DIR}/hello.c : ${DIR}/generate_source.sh
	cd $(dir $<) && ./generate_source.sh
