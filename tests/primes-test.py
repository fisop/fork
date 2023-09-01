#!/usr/bin/env python3

import re
import sys
import subprocess

from dataclasses import dataclass

# FIXME: remove; unused.
from os import getpid
from resource import prlimit, RLIMIT_NPROC, RLIMIT_NOFILE 
from subprocess import PIPE, run

from utils import VALGRIND_COMMAND, format_result, run_command

# I tested my implementation with all limits below. -d
@dataclass
class PrimesTest:
    max_prime: int
    task_limit: int
    fd_limit: int = 6

# NOTE: task_limit is computed as the total count of primes, plus two, i.e.:
# nproc = 2 + sum(1 for _ in generate_primes(number))
TESTS = [
    PrimesTest(10, 6),
    PrimesTest(100, 27),
    PrimesTest(1000, 170),
    PrimesTest(10000, 1231),
]

OLD_TESTS = [
    {
        'description': 'correct primes up to 10',
        'number': 10,
        'valgrind_enabled': True
    },
    {
        'description': 'correct primes up to 100',
        'number': 100,
        'valgrind_enabled': False
    },
    {
        'description': 'correct primes up to 1000',
        'number': 1000,
        'valgrind_enabled': False
    },
    {
        'description': 'correct primes up to 10000',
        'number': 10000,
        'valgrind_enabled': False
    }
]

def exec_command(args, run_valgrind=False):
    # limits the number of open file descriptor
    # that the process or any of its child can have
    #
    # it has to be set to one more that the desired number
    prlimit(getpid(), RLIMIT_NOFILE, (10, 100))

    # limits the number of process that can be created
    # by a given process
    #
    # for the number 10000, a threshold of 3000 its OK
    #   (found it empirically -pic)
    prlimit(getpid(), RLIMIT_NPROC, (1900, 3000))

    output, valgrind_report, errors = run_command(args, run_valgrind=run_valgrind)

    if not run_valgrind and errors:
        raise Exception(errors)

    return set(filter(lambda l: l != '', output.split('\n'))), valgrind_report

def test_primes(binary_path, test_config):
    # TODO(dato): re-introduce Valgrind support.
    valgrind_report = None
    # args = [binary_path, str(test_config.max_prime)]
    args = ['systemd-run', '-qPG', '--user', '--wait']
    args.extend(['-p', f'TasksMax={test_config.task_limit}'])
    args.extend(['-p', f'LimitNOFILE={test_config.fd_limit}'])
    args.extend(['--', binary_path, str(test_config.max_prime)])

    proc = subprocess.run(args, text=True, capture_output=True)
    output = proc.stdout.split('\n')

    if errors := proc.stderr:
        raise Exception(errors)

    # Compute set of primes emitted.
    primes = {int(m[1]) for l in output if (m := re.search(r'primo (\d{1,4})', l, re.I))}

    return primes, valgrind_report

def generate_primes(number):
    # JOS code (grade-lab5) to calculate primes in a given range
    rest = range(2, number)
    while rest:
        yield rest[0]
        rest = [n for n in rest if n % rest[0]]

def run_test(binary_path, test_config, run_valgrind=False):
    number = test_config.max_prime
    description = f'correct primes up to {number}'

    expected_primes = set(generate_primes(number))
    resource_msg = None

    try:
        result_primes, valgrind_report = test_primes(binary_path, test_config)
        res = result_primes == expected_primes
    except Exception as e:
        resource_msg = f'Resource error - {e}'
        res = False

    print(f'  {description}: {format_result(res)}')

    if resource_msg is not None:
        print(resource_msg)
        return res

    if not res:
        diff_res = expected_primes ^ result_primes
        if not (expected_primes <= result_primes):
            # missing prime numbers in result
            assertion_msg = f"""
Prime numbers missing:
---------------------
{diff_res}
            """
        else:
            # not prime numbers in result
            assertion_msg = f"""
NOT prime numbers:
-----------------
{diff_res}
            """
        print(assertion_msg)

    if run_valgrind and valgrind_enabled:
        print(valgrind_report)

    return res

def execute_tests(binary_path, tests, run_valgrind=False):
    success = 0
    total = len(tests)

    for test_config in tests:
        res = run_test(binary_path, test_config, run_valgrind)
        if res:
            success += 1

    print(f'{success}/{total} passed')

def main(binary_path, run_valgrind):
    print('COMMAND: primes')

    execute_tests(binary_path, TESTS, run_valgrind)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: primes-test.py PRIMES_BIN_PATH [-v]')
        sys.exit(1)

    binary_path = sys.argv[1]
    run_valgrind = True if len(sys.argv) > 2 and sys.argv[2] == '-v' else False

    main(binary_path, run_valgrind)
