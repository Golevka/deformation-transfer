all:
	cd modelviz;         make;
	cd corrstool;        make;
	cd corres_resolve;   make;
	cd dtrans;	     make;

	mv ./modelviz/run        ./bin/modelviz
	mv ./corrstool/run       ./bin/corrstool
	mv ./corres_resolve/run  ./bin/corres_resolve
	mv ./dtrans/run	         ./bin/dtrans

clean:
	cd modelviz;         make clean;
	cd corrstool;        make clean;
	cd corres_resolve;   make clean;
	cd dtrans;	     make clean;
	rm \
		./bin/modelviz 		\
		./bin/corrstool 	\
		./bin/corres_resolve 	\
		./bin/dtrans
