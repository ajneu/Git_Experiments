#!/usr/bin/env bash

# generate the following
# http://git-scm.com/book/en/v2/Git-Branching-Rebasing#The-Perils-of-Rebasing

x () {
    local ARG="$1"
    echo "$ARG"
    eval $ARG &>/dev/null  # execute command
}

create_content () {
    local FILE="$1"
    x "echo a >> ${FILE}"
}

create_commit () {
    local ARG="$1"
    create_content "$ARG"
    x "git add $ARG"
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

x "rm -rf repo.git team1 my proj5"
x

x "git init --bare repo.git"
x

x "git clone repo.git team1"
x "cd team1"
create_commit C1
x "git push origin master"
x

x "cd .."
x "git clone repo.git my -o teamone"
x "cd my"
create_commit C2
create_commit C3
x

x "cd ../team1"
create_commit C4
x

x "cd .."
x "git clone repo.git proj5"
x "cd proj5"
create_commit C5
x "git push origin master"
x

x "cd ../team1"
x "git fetch origin # git fetch --all"
x "git merge --no-commit origin/master"
x "git commit -m C6"
x "git push origin master"
x

x "cd ../my"
x "git fetch teamone # git fetch --all"
x "git merge --no-commit teamone/master"
x "git commit -m C7"
x

x "cd ../team1"
x "git rebase HEAD^2 HEAD^1"
x "git branch new_master"
x "git branch -d master"
x "git branch -m new_master master #rename branch"
x "git push origin master --force"
x

x "cd ../my"
x "git fetch teamone"
x "git merge --no-commit teamone/master"
x "git commit -m C8"
x

x "echo"
x "git log --oneline --decorate --all --graph"
   git log --oneline --decorate --all --graph
x

echo
echo "commit structure generated in directory my"
