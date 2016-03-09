#!/usr/bin/python
# Read the file storing coreids and get related full name and email of the coreid,
# and write them to two separated files: names.tmp and emails.tmp
# usage: coreid_parser.py filename

import os
import sys
import commands
import re

def main():
    cmd = "ldapsearch -w '{}' -D 'motguid={},ou=people,ou=intranet,dc=motorola,dc=com' -b 'ou=people,ou=intranet,dc=motorola,dc=com' -h 'ids.mot-solutions.com' -x uid={}"

    if len(sys.argv) != 2:
        print 'usage:', sys.argv[0], 'filename'
        sys.exit(1)

    sys.stdout.write('bind name: ')
    bind_name = sys.stdin.readline().strip()
    sys.stdout.write('password: ')
    passwd = sys.stdin.readline().strip()
    input_file = open(sys.argv[1], 'rU')
    output_file1 = open('names.tmp', 'w')
    output_file2 = open('emails.tmp', 'w')
    for coreid in input_file:
        (status, output) = commands.getstatusoutput(cmd.format(passwd, bind_name, coreid.strip()))
        if status:
            print 'failed to get ldap info for', coreid
            print status
            print output
        else:
            mail = re.search(r'motFriendlyMail: (.*)', output).group(1)
            match = re.search(r'motNickName: (.*)', output)
            if not match:
                firstname = re.search(r'motDisplayFirstName: (.*)', output).group(1)
            else:
                firstname = match.group(1)
            lastname = re.search(r'motDisplayLastName: (.*)', output).group(1)
            output_file1.write(lastname+' '+firstname+'-'+coreid)
            output_file2.write(mail+'\n')
            print lastname+' '+firstname+'-'+coreid.strip(), mail
    input_file.close()    
    output_file1.close()
    output_file2.close()

    return


if __name__ == '__main__':
    main()
