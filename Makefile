
all: epic-tree

suffix-tree.hpp:

optimize.h:

async-reader.hpp: 

main.cpp:


epic-tree: suffix-tree.hpp optimize.h async-reader.hpp main.cpp
	$(CXX) -O3 -Wall main.cpp -o print-epic-tree -lrt -ggdb -DPRINT_TREE #-fprofile-use

