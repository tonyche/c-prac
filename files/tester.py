#!/usr/bin/env python

import sys, os, subprocess, filecmp, shutil

all_tests = 0
c_dir = sys.argv[1]
if not c_dir.endswith("/"):
    c_dir += "/"

for filename in os.listdir(c_dir):
    if filename.endswith(".dat"):
        all_tests += 1

all_tests /= 3

print "All tests: " + str(all_tests)
print "=" * 30

program_name = sys.argv[2]
passed_tests = 0


if len(sys.argv) == 5: 
    if sys.argv[3] == '-tv':
        log = open(sys.argv[4], "w")

for i in range(all_tests):
    out_file_name = str(i) + "_out.dat"
    out_to_process = str(i) + "_pr_out.dat"
    right_answ_file_name = c_dir + str(i) + "_res.dat"
    right_to_process = c_dir + str(i) + "_pr_res.dat"
    right_processed = c_dir + str(i) + "_ed_res.dat"
    cur_test = {'Out': open(out_file_name, "w")}
    shutil.copy2(right_to_process, './' + out_to_process)
    if len(sys.argv) == 5: 
        if sys.argv[3] == '-tv':
            subprocess.check_call(['valgrind', program_name, out_to_process], stdout=cur_test['Out'], stderr=log)
    else:    
        subprocess.check_call([program_name, out_to_process], stdout=cur_test['Out'])
   
    cur_test['Out'].close()
    if filecmp.cmp(out_file_name, right_answ_file_name) and filecmp.cmp(out_to_process, right_processed):
        os.remove(out_file_name)
        os.remove(out_to_process)
        passed_tests += 1
        print " --[+]--> Test [" + str(i) + "] passed!"
    else:
        print " --[X]--> Failed on the test  [" + str(i) + "]. For details see your's program output: " + out_file_name
        continue

print "=" * 30
print "Passed tests: " + str(passed_tests) + " of " + str(all_tests)
