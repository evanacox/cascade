#!/usr/bin/python

import sys
from typing import List, Tuple
from argparse import ArgumentParser
import multiprocessing
import os

num_cpus_default = int(multiprocessing.cpu_count() / 2)
abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

class Color:
    @staticmethod
    def bg_green(string: str) -> str
        return 


class Logger:
    @staticmethod


class TestRunner:
    """Deals with running tests and storing results,
    and presenting those results to the user
    """

    def __init__(self):
        self.failures = []

    def clean(self):
        """
        Cleans the test build directory completely
        """
        if os.path.isdir("build"):
            self.clean()

    def run_file(self, executable_path: str) -> int:
        """Runs an executable that was previously compiled, and returns
        its return code.

        Arguments:
            - executable_path: Path to the compiled binary, e.g "./a.out"

        Returns:
            - The exit code of the compiled executable
        """
        raise NotImplementedError("Not implemented!")

    def compile_files(self, files: List[str], output_name: str) -> Tuple[List[int], List[int]]:
        """Invokes the Cascade compiler with a list of files

        Arguments:
            - files: The list of files to pass as arguments to the compiler
            - output_name: The name to give the emitted binary

        Returns:
            - A Tuple with 2 lists. The first list is a list of error codes emitted
            by the compiler. The second list contains a list of warning codes
            emitted by the compiler.
        """
        raise NotImplementedError("Not implemented!")

    def compile_cascade(self, fast) -> bool:
        """Attempts to build the Cascade compiler from source

        Returns:
            - Whether or not the compilation was successful
        """
        self.clean()

        os.mkdir("build")
        os.chdir("build")

        flags = ("-Ofast -flto " if fast else "") + "-fsanitize=undefined"

        result = os.system("cmake ../.. -DCMAKE_BUILD_TYPE=Release "
                           + f"-DCMAKE_CXX_FLAGS=\"{flags}\"")

        if not result:
            pass

        result = os.system("cmake -j8 --build . ")

        if not result:
            pass

    def compile_cascadelib(self) -> bool:
        """Attempts to use the previously built Cascade compiler to
        build the Cascade standard library.

        Returns:
            - Whether or not the compilation was successful
        """
        raise NotImplementedError("Not implemented!")


def main(options) -> None:
    runner = TestRunner()

    if options.clean:
        runner.clean()
    print(f"Threads: {options.jobs}")
    print(f"Whether to build: {options.build}")


parser = ArgumentParser(description="Tests the Cascade compiler")
parser.add_argument("-b", "--build", action="store_true",
                    help="build before running tests (default: false)")
parser.add_argument("-s", "--silent", action="store_true",
                    help="silence non-essential logging")
parser.add_argument("-j", "--jobs", default=num_cpus_default, metavar="N", type=int,
                    help=f"the number of jobs to run (default: {num_cpus_default})")
parser.add_argument("-c", "--clean", action="store_true",
                    help="whether to clean directory before running")

options = parser.parse_args()

if __name__ == "__main__":
    main(options)
