#!/usr/bin/env bash

# generate the following commit structure
# http://git-scm.com/book/en/v2/Git-Branching-Branching-Workflows#Topic-Branches
# http://git-scm.com/book/en/v2/book/03-git-branching/images/topic-branches-1.png

x () {
    local ARG="$1"
    echo "$ARG"
    eval $ARG &>/dev/null  # execute command
}

create_content () {
    local FILE="$1"
    x "echo a >>     ${FILE}"
}

create_commit () {
    local ARG="$1"
    create_content "$ARG"
    x "git add       $ARG"
    x "git commit -m $ARG"
}

sha1_of_nth_ancestor () {
    local N="$1"
    ((N++))
    git log --pretty=oneline | sed "${N}q;d" | cut -d ' ' -f 1
}


x "## Listing of commands"
x "######################"
x

x "rm -rf proj"
x "mkdir  proj"
x "cd     proj"
x "git init"
x

create_commit C0
create_commit C1
x

x "git checkout -b iss91"
create_commit C2
x

x "git checkout master"
create_commit C3
x

x "git checkout iss91"
create_commit C4
create_commit C5
create_commit C6
x

x "git checkout -b iss91v2 HEAD~2 # git checkout -b iss91v2 $(sha1_of_nth_ancestor 2)"
create_commit C7
create_commit C8
x

x "git checkout master"
create_commit C9
create_commit C10
x

x "git checkout iss91v2"
create_commit C11
x

x "git checkout master"
x "git checkout -b dumbidea"
create_commit C12
create_commit C13
x


x "echo"
x "git log --oneline --decorate --all --graph"
   git log --oneline --decorate --all --graph
x

echo
echo "commit structure generated in directory proj"
