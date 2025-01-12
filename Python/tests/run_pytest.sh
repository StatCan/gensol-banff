#!/bin/bash

## Change log##
## 2023.09.01  - /PASS no longer runs performance tests
## 2023.10.05  - change the default options to reduce clutter in output
##             - copied from `unit_tests`, simplified

# variables
version=2023.10.05
python_cmd_default='python3.11'
if [ -z ${python_cmd+x} ]; then 
    python_cmd=$python_cmd_default
    echo "using default \`python_cmd=$python_cmd\`"
    echo "to override, \`export python_cmd=<python command>\`"
else
    echo "using \`python_cmd=$python_cmd\`"
fi
pytest_options_common='--verbose -rA --tb=short'
# --verbose     adds line-per-test "PASSED" or "FAILED"
# -rA           ensures all captured output is printed and prints line-per-test status
# --tb=short    reduces traceback to essentially show the call stack
#               unfortunately --tb=no suppresses console output from failed tests

pytest_options="--cache-clear $pytest_options_common"
pytest_options_rerun_fail="$pytest_options_common --last-failed"
call_pytest_opt="$python_cmd -m pytest $pytest_options"

# enable case insensitive comparisons
shopt -s nocasematch

# clear screen? 
if [ "$autoclear" = "true" ] 
then
    clear
fi

############# FUNCTIONS ####################################################
print_header () {
    echo "run_pytest.sh - version:$version"
}

print_help () {
    echo ''
    echo 'RUN_PYTEST [path] [filename] [additional PyTest Options]'
    echo 'RUN_PYTEST <selection-flag>'
    echo ''
    echo 'Description: '
    echo '    Runs Python's PyTest module, applying default options and easing test selection.'
    echo '    Default Options: $pytest_options'
    echo ''
    echo '    Searches the current directory using standard test discovery protocols.'
    echo '    Specify a directory or file.  '
    echo '    Provide any valid PyTest option.  '
    echo '    Use a "Selection Flag" \(see below\).'
    echo ''
    echo 'Selection Flags:'
    echo '    /PASS                   only run valid tests that '
    echo '                            are expected to pass'
    echo '                            EXCEPT performance tests'
    echo ''
    echo '    /AUTO_PASS              run non-performance tests that'
    echo '                            are expected to pass and do '
    echo '                            not require SAS generated data'
    echo '                            \(ideal for automated testing\)'
    echo ''
    echo '    /PERF [test spec]       only run performance tests'
    echo '                            use [test spec] to specify a '
    echo '                            particular test'
    echo ''
    echo '    /LF                     run using  pytest's '
    echo '                            --last-failed option '
    echo ''
    echo 'Examples:'
    echo '    RUN_PYTEST /PASS'
    echo '    RUN_PYTEST -k test_DonorA -s'
    echo '    RUN_PYTEST test_donorP01.py'
}
############# END OF FUNCTIONS #############################################

print_header

# handle options
case $1 in
    
    -h) ;&
    /h)
        print_help
        ;;
    /show-options)
        echo pytest_options            = $pytest_options
        echo pytest_options_rerun_fail = $pytest_options_rerun_fail
        echo call_pytest_opt           = $call_pytest_opt
        ;;
    /pass)
        $call_pytest_opt -m "(m_pass or m_pass_as_is)"
        ;;
    /auto_pass)
        $call_pytest_opt -m "(m_auto_pass and not m_needs_sas_data)"
        exit $?
        ;;
    /lf)
    $python_cmd -m pytest $pytest_options_rerun_fail
    ;;
    /perf)
        export BANFF_DEBUG_STATS='true'
        if [ "$2" = "" ]
        then
            $call_pytest_opt -m "(m_performance_test)"
        else
            $call_pytest_opt $2 $3 $4 $5
        fi
        export -n BANFF_DEBUG_STATS
    ;;
    *)
    export no_match='true'
esac

if [ "$no_match" = "true" ]
then
    if [ "$1" = "" ]
    then
        echo 'running pytest on the current directory'
        $call_pytest_opt .
    else
        echo "running pytest on $1"
        $call_pytest_opt $*
    fi
fi
