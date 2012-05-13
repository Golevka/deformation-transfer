default:
	make clean; make all

all:
	cd modelviz;         make; make clean
	cd corrstool;        make; make clean
#	cd adjtool;          make; make clean
	cd corres_resolve;   make; make clean
	cd dtrans;	     make; make clean

	cp ./modelviz/run    ./bin/modelviz
	cp ./corrstool/run   ./bin/corrstool
#	cp ./adjtool/run     ./bin/adjtool
	cp ./corres_resolve/run ./bin/corres_resolve
	cp ./dtrans/run		./bin/dtrans

clean:
	rm \
		./bin/modelviz 		\
		./bin/corrstool 	\
		./bin/corres_resolve 	\
		./bin/dtrans
