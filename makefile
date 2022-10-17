ex14 : firstPass.o secondPass.o printUtils.o parser.o main.o
	gcc -g -ansi -Wall -pedantic firstPass.o secondPass.o printUtils.o parser.o main.o -o ex14

main.o:	main.c header.h
	gcc -c -ansi -Wall -pedantic main.c -o main.o
firstPass.o: firstPass.c header.h external.h
	gcc -c -ansi -Wall -pedantic firstPass.c -o firstPass.o
secondPass.o: secondPass.c header.h external.h
	gcc -c -ansi -Wall -pedantic secondPass.c -o secondPass.o
printUtils.o: printUtils.c header.h external.h
	gcc -c -ansi -Wall -pedantic printUtils.c -o printUtils.o
parser.o: parser.c header.h external.h
	gcc -c -ansi -Wall -pedantic parser.c -o parser.o