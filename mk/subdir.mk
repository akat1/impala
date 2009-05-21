
_SUBDIR_BUILD?= build
_SUBDIR_CLEAN?= clean
_SUBDIR_DEPEND?= depend
_SUBDIR_CLEANDEPEND?= cleandepend
_SUBDIR_INSTALL?= install

#
# GNU Make lubi drukowac do jakiego katalogu aktualnie wchodzi i z jakiego
# wychodzi. Psuje to troche estetycznosc komunikatow, jakie drukuja te
# skrypty Makefile, zatem ustawiam zmienna MAKELEVEL na "" za kazdym
# razem aby zmylic GNU Make (uzywa tej zmiennej srodowiskowej
# do rozpoznawania, czy jest procesem 'wchodzacym' gdzies, czy jest
# uruchomionym przez uzytkownika
#			-- Pawel Wieczorek
#

${_SUBDIR_BUILD}:
	@for d in ${SUBDIRS};\
		do echo "===> ${DIRPRFX}$$d (build)";\
		   cd $$d; DIRPRFX="${DIRPRFX}$$d/" MAKELEVEL="" ${MAKE} build; if [ ! $$? -eq 0 ]; then exit 1; fi; cd ..;\
	done;

${_SUBDIR_CLEAN}:
	@for d in ${SUBDIRS};\
		do echo "===> ${DIRPRFX}$$d (clean)";\
			cd $$d; DIRPRFX="${DIRPRFX}$$d/" MAKELEVEL="" ${MAKE} clean; cd ..;\
	done

${_SUBDIR_DEPEND}:
	@for d in ${SUBDIRS};\
		do echo "===> ${DIRPRFX}$$d (depend)";\
			cd $$d; DIRPRFX="${DIRPRFX}$$d/" MAKELEVEL="" ${MAKE} depend; cd ..;\
	done

${_SUBDIR_INSTALL}:
	@for d in ${SUBDIRS};\
		do echo "===> ${DIRPRFX}$$d (install)";\
			cd $$d; DIRPRFX="${DIRPRFX}$$d/" MAKELEVEL="" ${MAKE} install; cd ..;\
	done


${_SUBDIR_CLEANDEPEND}:
	@for d in ${SUBDIRS};\
		do echo "===> ${DIRPRFX}$$d (cleandepend)";\
			cd $$d; DIRPRFX="${DIRPRFX}$$d/" MAKELEVEL="" ${MAKE} cleandepend; cd ..;\
	done

