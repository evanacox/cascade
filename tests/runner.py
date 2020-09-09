#!/usr/bin/python

import sys
from typing import List, Tuple
from optparse import OptionParser


class TestRunner:
    """Deals with running tests and storing results,
    and presenting those results to the user
    """

    def __init__(self):
        self.failures = []

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

    def compile_cascade(self) -> bool:
        """Attempts to build the Cascade compiler from source

        Returns:
            - Whether or not the compilation was successful
        """
        raise NotImplementedError("Not implemented!")

    def compile_cascadelib(self) -> bool:
        """Attempts to use the previously built Cascade compiler to
        build the Cascade standard library.

        Returns:
            - Whether or not the compilation was successful
        """
        raise NotImplementedError("Not implemented!")


def main(threads) -> None:
    raise NotImplementedError("Not implemented!")


parser = OptionParser()
parser.add_option("-j", "--jobs", dest="jobs", default=1, type="int", metavar="JOBS",
                  help="the number of parallel jobs to run")
parser.add_option("-b", "--build", dest="build", default=True,
                  help="Whether or not to build the compiler before running tests")

(options, args) = parser.parse_args()

if __name__ == "__main__":
    main(options)
