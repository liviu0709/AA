#!/usr/bin/env python3
import sys
from openpyxl import load_workbook

import tm


WS = "\t "
TMS_BLANK_SYM = "_"
DIR_TRANSLATION = {
        "<": tm.Dir.LEFT, "-": tm.Dir.HOLD, ">": tm.Dir.RIGHT,
        "L": tm.Dir.LEFT, "S": tm.Dir.HOLD, "R": tm.Dir.RIGHT,
        "←": tm.Dir.LEFT, "−": tm.Dir.HOLD, "→": tm.Dir.RIGHT,
        }


def eprint(*args, **kwargs):
    print(*args, **kwargs, file=sys.stderr)


class ParseException(Exception):
    pass


def parse(contents):
    i = 0

    orig_lines = contents.split("\n")
    no_ws = (line.strip(WS) for line in orig_lines)
    no_empties = (line for line in no_ws if line)
    no_commlines = (line for line in no_empties if not line.startswith("//"))

    lines = []
    for line in no_commlines:
        good_line = line
        idx = good_line.find("//")
        if idx != -1:
            good_line = good_line[:idx]

        good_line = good_line.rstrip(WS)
        lines.append(good_line)

    i = 0
    n = len(lines)
    init_state = None
    delta = {}
    while i < n:
        original_line = lines[i]
        line = original_line.strip(WS)
        idx = line.find("//")
        if idx != -1:
            line = line[:idx]

        if not line or line.startswith("name:") or line.startswith("accept:"):
            i += 1
            continue

        if line.startswith("init:"):
            init_state = line[len("init:"):].strip(WS)
            i += 1
            continue

        for ws in WS:
            line = line.replace(ws, "")

        try:
            state, sym = line.split(",")
            if sym == TMS_BLANK_SYM:
                sym = tm.BLANK_SYM


            i += 1
            found = False
            while not found:
                original_line = lines[i]
                line = original_line.strip(WS)
                idx = line.find("//")
                if idx != -1:
                    line = line[:idx]

                if not line or line.startswith("name:") or line.startswith("accept:"):
                    i += 1
                    continue

                if line.startswith("init:"):
                    init_state = line[len("init:"):].strip(WS)
                    i += 1
                    continue

                for ws in WS:
                    line = line.replace(ws, "")

                found = True

            nstate, nsym, ndir = line.split(",")
            if nsym == TMS_BLANK_SYM:
                nsym = tm.BLANK_SYM

            delta[(state, sym)] = (nstate, nsym, DIR_TRANSLATION[ndir])
        except ValueError as e:
            print(e)
            raise ParseException(f"Malformed transition in line {i}: \"{original_line}\"")

        i += 1

    if init_state is None:
        raise ParseException("Initial state not defined!")

    return tm.TuringMachine(init_state, delta)


def parse_xlsx(path):
    wb = load_workbook(filename=path)
    sheet = wb.worksheets[0]
    values = list(sheet.values)
    symbols = values[0]

    delta = {}
    init_state = values[1][0]
    for state_line in values[1:]:
        state = state_line[0]
        for i, sym in enumerate(symbols[1:]):
            if not isinstance(sym, str):  # digits are interpreted as reals
                sym = str(int(sym))

            entry = state_line[i + 1]
            if not entry:
                entry = "N,_,-"

            for ws in WS:
                entry = entry.replace(ws, "")

            #entry = entry.replace("(", "")
            #entry = entry.replace(")", "")
            try:
                nstate, nsym, d = entry.split(",")
            except ValueError:
                eprint(f"Error parsing entry: {entry}")
                raise
            d = DIR_TRANSLATION[d]

            if sym == "_":
                sym = tm.BLANK_SYM

            if nsym == "_":
                nsym = tm.BLANK_SYM

            delta[(state, sym)] = (nstate, nsym, d)

    machine = tm.TuringMachine(init_state, delta)
    return machine


def main():
    pass


if __name__ == "__main__":
    main()
