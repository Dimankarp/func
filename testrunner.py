#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ast
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path


GREEN = "\033[32m"
RED = "\033[31m"
RESET = "\033[0m"


def ok_text(s: str) -> str:
    return f"{GREEN}{s}{RESET}"


def err_text(s: str) -> str:
    return f"{RED}{s}{RESET}"


@dataclass(frozen=True)
class TestCase:
    path: Path
    name: str
    stdin: str
    expected_stdout: str
    expected_exit_code: int
    compilation_should_fail: bool

@dataclass(frozen=True)
class CompiledTest:
    num: int
    case: TestCase
    exe_path: Path


@dataclass(frozen=True)
class RunResult:
    case: TestCase
    ok: bool
    stdout: str
    stderr: str
    exit_code: int


_QUOTED_STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"')

def _split_by_delimiter_ignore_quotes(text: str, delimiter=' ') -> list[str]:
    escaped_delim = re.escape(delimiter)
    
    pattern = r'{}(?=(?:[^"]*"[^"]*")*[^"]*$)'.format(escaped_delim)
    
    res = re.split(pattern, text)
    
    return [r for r in res if r != '']


def _parse_quoted_string(value: str) -> str:
    m = _QUOTED_STRING_RE.search(value)
    if not m:
        return value.strip()

    literal = m.group(0)
    try:
        parsed = ast.literal_eval(literal)  # for stuff like "", "\n", "\t", etc.
        if not isinstance(parsed, str):
            return str(parsed)
        return parsed
    except Exception:
        return literal.strip('"')



    

def parse_test_metadata(path: Path) -> list[TestCase]:
    name = "test"
    stdin = []
    expected_stdout = []
    expected_exit_code = 0
    compilation_should_fail = False

    with path.open("r", encoding="utf-8", errors="replace") as f:
        for line in f:
            stripped = line.strip()
            if stripped == "":
                continue
            if not stripped.startswith("//"):
                break
            if ":" not in stripped:
                continue

            content = stripped[2:].strip()

            raw_key, raw_val = content.split(":", 1)
            key = raw_key.strip()
            val = raw_val.strip()

            if key == "test":
                name = val if val else name
            elif key == "input":
                stdin = [_parse_quoted_string(v) for v in _split_by_delimiter_ignore_quotes(val)]
            elif key == "output":
                expected_stdout = [_parse_quoted_string(v) for v in _split_by_delimiter_ignore_quotes(val)]
            elif key == "exit-code":
                expected_exit_code = int(val)
            elif key == "compile_fail":
                 compilation_should_fail = bool(val)
    
    if len(stdin) != len(expected_stdout):
        raise Exception(f"Amount of Input({len(stdin)}) and Output({len(expected_stdout)}) args must be the same.")

    return [TestCase(
        path=path,
        name=name,
        stdin=inp,
        expected_stdout=outp,
        expected_exit_code=expected_exit_code,
        compilation_should_fail = compilation_should_fail
    ) for (inp, outp) in zip(stdin, expected_stdout)]


TEST_NUM = 1
def next_num() -> int:
    global TEST_NUM
    """ NOTICE: Not thread safe! """
    temp = TEST_NUM
    TEST_NUM+=1
    return temp

def compile_test(root: Path, case: TestCase) -> tuple[CompiledTest | None, str | None]:

    testrun_dir = root / "build" / "testrun"
    testrun_dir.mkdir(parents=True, exist_ok=True)

    test_num = next_num()

    exe_name = f"{case.path.stem}_{test_num}"
    exe_path = testrun_dir / exe_name

    cmd = [
        "bash",
        "./compile.sh",
        "-o",
        str(exe_path),
        "stdlib/utils.fc",
        str(case.path),
    ]

    proc = subprocess.run(
        cmd,
        cwd=str(root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    if proc.returncode != 0:
        msg = (
            f"Compilation failed for {case.path}\n"
            f"Command: {' '.join(cmd)}\n"
            f"stdout:\n{proc.stdout}\n"
            f"stderr:\n{proc.stderr}\n"
        )
        return None, msg

    return CompiledTest(num=test_num, case=case, exe_path=exe_path), None


def run_one(compiled: CompiledTest, root: Path) -> RunResult:
    proc = subprocess.run(
        [str(compiled.exe_path)],
        cwd=str(root),
        input=compiled.case.stdin,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    stdout = proc.stdout
    stderr = proc.stderr
    exit_code = proc.returncode

    ok = (stdout == compiled.case.expected_stdout) and (exit_code == compiled.case.expected_exit_code)

    return RunResult(
        case=compiled.case,
        ok=ok,
        stdout=stdout,
        stderr=stderr,
        exit_code=exit_code,
    )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="FunC test runner: compiles .fc test_files and runs them in parallel"
    )
    parser.add_argument("test_file", nargs="+", help="One or more .fc test files")
    args = parser.parse_args(argv)

    root = Path(__file__).resolve().parent

    cases: list[TestCase] = []
    for t in args.test_file:
        p = Path(t)
        if not p.is_absolute():
            p = (root / p).resolve()

        if not p.exists() or not p.is_file():
            print(err_text("ERROR") + f" test file not found: {t}", file=sys.stderr)
            return 1

        cases.extend(parse_test_metadata(p))

    compiled_tests: list[CompiledTest] = []
    any_failed = False

    for case in cases:
        compiled, err = compile_test(root, case)

        if case.compilation_should_fail: 
            if err is not None:
                print(ok_text("OK") + f" {case.name} ({case.path})")
            else:
                any_failed = True
                print(err_text("ERROR") + f" {case.name} ({case.path})\n Expected to fail compilation. ")
            continue

        if err is not None:
            any_failed = True
            print(err_text("ERROR") + f" {case.name} ({case.path})")
            print(err, file=sys.stderr)
        else:
            compiled_tests.append(compiled)

    max_workers = min(len(compiled_tests), (os.cpu_count() or 4))
    if max_workers < 1:
        max_workers = 1

    with ThreadPoolExecutor(max_workers=max_workers) as ex:
        futures = [ex.submit(run_one, ct, root) for ct in compiled_tests]
        for fut in as_completed(futures):
            res = fut.result()

            if res.ok:
                print(ok_text("OK") + f" {res.case.name} ({res.case.path})")
                continue

            any_failed = True
            print(err_text("ERROR") + f" {res.case.name} ({res.case.path})")
            print(f"  input:            {res.case.stdin!r}")
            print(f"  expected output:  {res.case.expected_stdout!r}")
            print(f"  real output:      {res.stdout!r}")
            print(f"  expected exit:    {res.case.expected_exit_code}")
            print(f"  real exit:        {res.exit_code}")
            if res.stderr:
                print(f"  stderr:           {res.stderr!r}")

    return 1 if any_failed else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))