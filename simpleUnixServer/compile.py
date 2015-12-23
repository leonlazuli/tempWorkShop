import subprocess
import sys

command1 = "gcc -o main webServer.c;"
command2 = " gcc -o client webClient.c rio.h;"
command3 = " gcc -o cgi-bin/adder cgi-bin/adder.c rio.h;";
command = "";

argv= sys.argv;
print len(argv);
if(len(argv) == 1):
    command = command1 + command2 + command3;
else:
    options = argv[1];
    if "s" in options:
        command += command1;
    if "c" in options:
        command += command2;
    if "a" in options:
        command += command3;
    

proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True)
(out, err) = proc.communicate()
print "program output:", out
print "program error:", err
