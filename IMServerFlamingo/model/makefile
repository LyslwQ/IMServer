TRAGET = main
XX     = g++
CPPFLAGS = -Wall -O -g -std=c++11
SOURCE = $(wildcard *.cpp)
OBJ    = $(subst .cpp,.o,$(SOURCE))
LIB    = -lodb-mysql -lodb 
HXX    = $(filter-out $(wildcard *-odb.hxx),$(wildcard *.hxx))

target :$(OBJ)
	$(XX) $(CPPFLAGS) -o $(TRAGET) $(OBJ)  $(LIB) 
include ${OBJ:.o=.d}  
#FILE = test.d
%.d: %.cpp
	set -e;\
	rm -f $@\ 
	$(XX) -MM $(CPPFLAGS) $(LIB) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o:%.cpp
	$(XX) $(CFLAGS) -c $< -o $@  $(LIB) 

.PHONY:odb
odb:
#	odb -d mysql  --generate-schema --schema-name $(basename $<)  --schema-format embedded \
#	--generate-query $<
#	$(shell mv -f $@  $($@:.hxx=.cpp) 
#
	odb -d mysql --generate-schema --schema-name User --schema-format embedded \
	--generate-query User.hxx
	
	mv -f User-odb.cxx User-odb.cpp

	odb -d mysql --generate-schema --schema-name IMGroup --schema-format embedded \
	--generate-query IMGroup.hxx
	mv -f IMGroup-odb.cxx IMGroup-odb.cpp

	odb -d mysql --generate-schema --schema-name Relationship --schema-format embedded \
	--generate-query Relationship.hxx
	mv -f Relationship-odb.cxx Relationship-odb.cpp 

	odb -d mysql --generate-schema --schema-name IMMessage --schema-format embedded \
	--generate-query IMMessage.hxx
	mv -f IMMessage-odb.cxx IMMessage-odb.cpp

	odb -d mysql --generate-schema --schema-name IMGroupMember --schema-format embedded \
	--generate-query IMGroupMember.hxx
	mv -f IMGroupMember-odb.cxx IMGroupMember-odb.cpp 
#	$(XX) $(CFLAGS) -o main Relationship-odb.cxx User-odb.cxx IMGroup-odb.cxx -DDATABASE_MYSQL  test.cpp Model.cpp Relationship.cpp  User.cpp IMGroup.cpp $(LIB)
	
#%.o : %.cpp 
#	$(XX) $(CFLAGS) -c $< -o $@ 

#SOURCES = $(wildcard *.cpp)
#OBJS    = $(patsubst %.cpp, %.o, $SOURCES)

#$(TARGET) : $(OBJS)
#	$(XX) $(OBJS) -o $(TARGET)

run :
	./main 

.PHONY: clean
clean:
	rm -fr $(OBJ)  $(wildcard *.d*) main
