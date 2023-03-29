todo: add stuff here


# installing the graph generation enviroment:
```
sudo pacman -S stp
cd ~/workspace/
git clone git@github.com:efeslab/klee-uclibc.git
cd klee-uclibc/
./configure -l
make
cd  ~/workspace
git clone git@github.com:efeslab/ER.git -b ironpatch
cd ER
mkdir Build
cd Build
bash ../build_klee_ironpatch
make
```

# generate graph
```
./gen_graph.sh
```

then `Ctrl+C`, `Ctrl+\`, `Ctrl+\`

use the file:
`branchConditions_cnt000.kquery.simplify.dot``
