
import os
import datetime

comment_character = '#'
exclusive_character = '!';
exclusive = False
test_data = []
log_string = ""

os.system("git pull")
os.system("gcc -o test ../src/*.c test.c -lm")

contents = open('test_data.txt', 'r').read()#.replace('(', '\(').replace(')', '\)')

if exclusive_character in contents:
    exclusive = True

for content in contents.split('\n'):
    if content != "" and content[0] != comment_character and (not exclusive or content[0] == exclusive_character) and '|' in content:
        
        content = content.split('|')
        
        os.popen('valgrind --leak-check=full --show-reachable=yes --undef-value-errors=no --log-file="valgrind_memcheck_log.txt" ./test 0 "' + content[0] + '" "' + content[1] + '"')
        with open('valgrind_memcheck_log.txt','r') as f:
            valgrind_memcheck_log = f.read()
        
        test_data.append([content[0].replace('\(', '(').replace('\)', ')'),
                          content[1].replace('\(', '(').replace('\)', ')'),
                          os.popen('./test 1 "' + content[0] + '" "' + content[1] + '"').read(),
                          valgrind_memcheck_log])
        
#        test_string = os.popen('./test 1 ' + content[0] + ' ' + content[1]).read()
#        memcheck_string = os.popen('valgrind --leak-check=full --show-reachable=yes --undef-value-errors=no ./test 0 ' + content[0] + ' ' + content[1]).read()
#        print(content[0]);
#        print(content[1]);
#        print(os.popen('./test 1 ' + content[0] + ' ' + content[1]).read());
#        print(os.popen('valgrind --leak-check=full --show-reachable=yes --undef-value-errors=no ./test 0 ' + content[0] + ' ' + content[1]).read())
#        os.popen('valgrind --tool=massif --massif-out-file=./massif.out ./test 0 ' + content[0] + ' ' + content[1])
#        os.popen('ms_print ./massif.out')

        
        
        
        '''os.popen('valgrind --tool=massif --massif-out-file=./massif.out ./test 0 ' + content[0] + ' ' + content[1])
        test_data.append([content[0],
                          content[1],
                          os.popen('./test 1 ' + content[0] + ' ' + content[1]).read(),
                          os.popen('valgrind --leak-check=full --show-reachable=yes --undef-value-errors=no ./test 0 ' + content[0] + ' ' + content[1]).read(),
                          os.popen('ms_print ./massif.out').read()])

print(test_data);


for data in test_data:
    log_string += data[2] + '\n'

log_string += '\n\n\nValgrind Logs:\n\n'

for data in test_data:
    log_string += data[0] + '\n'
    log_string += data[2] + '\n\n\n'
    log_string += data[3] + '\n\n\n\n\n'
    '''

log_string += "------------------------------\n Test results\n------------------------------\n"

for data in test_data:
    log_string += data[2] + '\n'

log_string += '\n'

log_string += "------------------------------\n Valgrind-Memcheck results\n------------------------------\n"

for data in test_data:
    log_string += data[3] + '\n'

log_string += '\n'

log_string += "------------------------------\n Valgrind-Massif results\n------------------------------\n"


log_file = open("log.txt", "w")
log_file.write(log_string)
log_file.close()

os.system("git add .")
os.system("git commit -m \"test " + datetime.datetime.now().strftime("%d-%m-%Y %H:%M:%S") + "\"")
os.system("git push origin master")
