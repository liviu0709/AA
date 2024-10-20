#!/usr/bin/env python3
import argparse
from enum import IntEnum
import os
import sys
from typing import Optional

from parse_tms import parse_tms
from parse_xlsx import parse_xlsx
from tests import TESTS
import tm
from write_tms import write_tms
from write_xlsx import write_xlsx


LOGDIR = "logs/"
MAX_STEPS = 100000


def eprint(*args, **kwargs):
    print(*args, **kwargs, file=sys.stderr)


class What(IntEnum):
    LINES = 0
    COLUMNS = 1
    VISIBILITY = 2
    FINAL = 3


STR2WHAT = {w.name.lower(): w.value for w in What}
WHAT2STR = {w.value: w.name.lower() for w in What}


def run_on_input(machine, args):
    res = machine.run(word=args.test_input, max_steps=args.max_steps,
                      debug=True)

    print(f"Result : {res}")


def run_test(machine, args, test, what, log):
    word, critteria = test
    res = machine.run(word=word, max_steps=args.max_steps, debug=True,
                      dbglog=log)

    if what == What.FINAL:
        ref = critteria[0] and critteria[1] and critteria[2]
    else:
        ref = critteria[what]

    return res == ref


def run_test_suit(machine, what, args, tests=TESTS, logdir=LOGDIR):
    total = 0
    os.makedirs(logdir, exist_ok=True)

    print(f"Testing {WHAT2STR[what]} verification:")
    for i, test in enumerate(tests.items()):
        dbglog = os.path.join(LOGDIR, f"dbglog_{i+1}")

        print(f"#{i+1:<3} ({test[0]})", "."*40, sep=" ", end=" ")
        try:
            res = run_test(machine, args, test, what, dbglog)
            print("PASS" if res else "FAIL")
            total += res
        except tm.StepLimitExceeded:
            print("SLE")

    print()
    return total


def run_tests(machine, args, tests=TESTS):
    if not isinstance(args.validation_type, list):
        critteria = [STR2WHAT[args.validation_type]]
    else:
        critteria = [STR2WHAT[c] for c in args.validation_type]

    total = 0

    total_max_score = 0
    if What.FINAL in critteria:
        critteria = [What.FINAL]
    for crit in critteria:
        if crit == What.LINES or crit == What.COLUMNS:
            total_max_score += 20
        elif crit == What.VISIBILITY:
            total_max_score += 40
        elif crit == What.FINAL:
            total_max_score += 90

    for what in critteria:
        cscore = run_test_suit(machine, what, args, tests)

        if what == What.LINES or what == What.COLUMNS:
            cscore = cscore*2/3
        elif what == What.VISIBILITY:
            cscore = cscore*4/3
        elif what == What.FINAL:
            cscore = cscore*3

        cscore = int(round(cscore, 2))

        total += cscore
        if what == What.FINAL:
            print(f"{WHAT2STR[what]} score: {cscore}/90\n")
        elif what == What.LINES or what == What.COLUMNS:
            print(f"{WHAT2STR[what]} score: {cscore}/20\n")
        else:
            print(f"{WHAT2STR[what]} score: {cscore}/40\n")

    total = int(round(total, 2))

    return total


def parse_machine(path):
    extension = path.split(".")[-1]
    if extension == "tms":
        return parse_tms(path)
    elif extension == "xlsx":
        return parse_xlsx(path)
    else:
        raise ValueError(f"Don't know what to do with {path} (valid \
                extensions are \".tms\" and \".xlsx\"")


def write_machine(machine, args):
    path = args.output
    extension = path.split(".")[-1]
    if extension == "tms":
        return write_tms(path, machine)
    elif extension == "xlsx":
        return write_xlsx(path, machine)
    else:
        raise ValueError(f"Don't know what to do with {path} (valid \
                extensions are \".tms\" and \".xlsx\"")


def get_machine_path(name: str) -> Optional[str]:
    if os.path.exists(f"{name}.tms"):
        path = f"{name}.tms"
    elif os.path.exists(f"{name}.xlsx"):
        path = f"{name}.xlsx"
    else:
        path = None

    return path


def main():
    parser = argparse.ArgumentParser(description="Checker for the first \
            assignment for the Analysis of Algorithm course. \
            The main functionality is to load a Turing Machine and either \
            run it on some input or convert it to another format.")
    parser.add_argument("--tm", help="Input Turing Machine file (.xlsx or \
                        .tms)", required=False)
    parser.add_argument("--max-steps", type=int, default=MAX_STEPS,
                        help="Maximum number of steps a TM is allowed to "
                             "make before a \"Step Limit Exceded\" error is "
                             "produced. Default is %(default)s.")
    tgroup = parser.add_mutually_exclusive_group(required=True)
    tgroup.add_argument("--run-tests", action="store_true", help="Run the \
                        machine on all tests; if no \"tm\" argument is given, "
                        "it will run all existing "
                        "\"skyscrapper-*.<tms or xlsx>\" machines.")
    tgroup.add_argument("--test-input", type=str, help="Test a specific input")
    tgroup.add_argument("--output", help="Output Turing Machine file (.xlsx \
            or .tms).")
    parser.add_argument("--validation-type", choices=["lines", "columns",
                        "visibility", "final"], nargs="+", default="final",
                        help="Choose which kind of checks are performed (by "
                        "default, only the \"final\" one is). The \"final\" "
                        "check overrides all others.")

    args = parser.parse_args()

    if args.tm:
        path = args.tm
        machine = parse_machine(path)

        if args.run_tests:
            total_score = run_tests(machine, args)
            print(f"Total: {total_score}/90")
        elif args.test_input:
            try:
                run_on_input(machine, args)
            except tm.StepLimitExceeded:
                eprint("Step Limit Exceeded!")
        elif args.output:
            write_machine(machine, args)
    elif args.run_tests:
        final_path = get_machine_path("skyscrapers_final")
        if final_path is not None:
            machine = parse_machine(final_path)
            args.validation_type = "final"
            total_score = run_tests(machine, args)
        else:
            total_score = 0
            for file in ["skyscrapers_lines", "skyscrapers_columns",
                         "skyscrapers_visibility"]:
                file_path = get_machine_path(file)
                if file_path is None:
                    print(f"File {file} is non-existent!\n")
                    continue

                machine = parse_machine(file_path)
                args.validation_type = file.split('_')[1]
                total_score += run_tests(machine, args)

        print(f"Total: {total_score}/90")


if __name__ == "__main__":
    main()
