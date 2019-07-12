#!/usr/bin/env python

import sys
import subprocess
import re

# Makes a GNU-Style ChangeLog from a git repository
# Handles git-svn repositories also

# Arguments : same as for git log
release_refs={}

def process_commit(lines, files):
    # DATE NAME
    # BLANK LINE
    # Subject
    # BLANK LINE
    # ...
    # FILES
    fileincommit = False
    lines = [x.strip() for x in lines if x.strip() and not x.startswith('git-svn-id')]
    files = [x.strip() for x in files if x.strip()]
    for l in lines:
        if l.startswith('* ') and ':' in l:
            fileincommit = True
            break

    top_line = lines[0]
    print top_line.strip()
    print
    if not fileincommit:
        for f in files:
            print '\t* %s:' % f
    for l in lines[1:]:
        print '\t ', l
    print

def output_commits():
    cmd = ['git', 'log', '--pretty=format:--START-COMMIT--%H%n%ai  %an <%ae>%n%n%s%n%b%n--END-COMMIT--',
           '--date=short', '--name-only']

    start_tag = find_start_tag()

    if start_tag is None:
        cmd.extend(sys.argv[1:])
    else:
        cmd.extend(["%s..HEAD" % (start_tag)])

    p = subprocess.Popen(args=cmd, shell=False, stdout=subprocess.PIPE)
    buf = []
    files = []
    filemode = False
    for lin in p.stdout.readlines():
        if lin.startswith("--START-COMMIT--"):
            if buf != []:
                process_commit(buf, files)
            hash = lin[16:].strip()
            try:
                rel = release_refs[hash]
                print "=== release %d.%d.%d ===\n" % (int(rel[0]), int(rel[1]), int(rel[2]))
            except:
                pass
            buf = []
            files = []
            filemode = False
        elif lin.startswith("--END-COMMIT--"):
            filemode = True
        elif filemode == True:
            files.append(lin)
        else:
            buf.append(lin)
    if buf != []:
        process_commit(buf, files)

def get_rel_tags():
    # Populate the release_refs dict with the tags for previous releases
    reltagre = re.compile("^([a-z0-9]{40}) refs\/tags\/[RELEASE-]*([0-9]+)[-_.]([0-9]+)[-_.]([0-9]+)")

    cmd = ['git', 'show-ref', '--tags', '--dereference']
    p = subprocess.Popen(args=cmd, shell=False, stdout=subprocess.PIPE)
    for lin in p.stdout.readlines():
       match = reltagre.search (lin)
       if match:
           (sha, maj, min, nano) = match.groups()
           release_refs[sha] = (maj, min, nano)

def find_start_tag():
    starttagre = re.compile("^([a-z0-9]{40}) refs\/tags\/CHANGELOG_START")
    cmd = ['git', 'show-ref', '--tags']
    p = subprocess.Popen(args=cmd, shell=False, stdout=subprocess.PIPE)
    for lin in p.stdout.readlines():
       match = starttagre.search (lin)
       if match:
           return match.group(1)
    return None

if __name__ == "__main__":
    get_rel_tags()
    output_commits()
