#!/usr/bin/env python

import sys, os, subprocess, filecmp

all_tests = 0
c_dir = sys.argv[1]
if not c_dir.endswith("/"):
    c_dir += "/"

for filename in os.listdir(c_dir):
    if filename.endswith(".dat"):
        all_tests += 1

all_tests /= 2

print "All tests: " + str(all_tests)
print "=" * 30

program_name = sys.argv[2]
passed_tests = 0

test_name = "test.dat"
child = "check"

if sys.argv[-2] == '-tv':
    log = open(sys.argv[-1], "w")

flag_exit = 0

for i in range(all_tests):
    out_file_name = str(i) + "_test_stdout.dat"
    right_answ_file_name = c_dir + str(i) + "_out.dat"
    cur_test = {'Test': open(c_dir + str(i) + "_in.dat", "r"), \
                    'RightAnsw': open(right_answ_file_name, "r"), \
                    'Out': open(out_file_name, "w")}
    try:
        if sys.argv[-2] == '-tv':
            subprocess.check_call(['valgrind', program_name, child, test_name], stdout=cur_test['Out'], stdin=cur_test['Test'], stderr=log)
        else:    
            subprocess.check_call([program_name, child, test_name], stdout=cur_test['Out'], stdin=cur_test['Test'], stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        flag_exit = str(e.returncode)
    cur_test['Test'].close()
    cur_test['RightAnsw'].close()
    cur_test['Out'].close()
    if filecmp.cmp(out_file_name, right_answ_file_name):
        os.remove(out_file_name)
        passed_tests += 1
        print " --[+]--> Test [" + str(i) + "] passed!"
    else:
        print " --[X]--> Failed on the test  [" + str(i) + "]. For details see your's program output: " + out_file_name
        continue
    if int(flag_exit) != 0:
        print " (test [" + str(i) + "] returned with exit code " + flag_exit + ")"
        flag_exit = 0

print "=" * 30
print "Passed tests: " + str(passed_tests) + " of " + str(all_tests)
