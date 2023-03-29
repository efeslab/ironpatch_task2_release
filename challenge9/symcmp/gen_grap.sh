ulimit -s unlimited

rm -rf explore.dir
~/workspace/ER/Build/bin/klee --posix-runtime -pathrec-entry-point="__klee_posix_wrapped_main" -ignore-posix-path=true -use-independent-solver=true -oob-check=true -allocate-determ -solver-backend=stp -use-forked-solver=false --libc=uclibc -output-dir=explore.dir -write-kqueries -output-stats=false -output-istats=false -output-source=false --save-final-module-path=linked.mono.bc linked.bc
cd explore.dir
 ~/workspace/ER/Build/bin/kleaver -draw -all -bitcode ../linked.mono.bc branchConditions_cnt000.kquery

 echo hint: look at explore.dir/branchConditions_cnt000.kquery.simplify.dot
