#!/usr/bin/python

import sys
from typing import List, Tuple


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


def main(threads: int) -> None:
    """Runs the tests

    Arguments:
        - threads: The number of threads to use on various tests 
          (e.g how many threads to compile cascadec with, how many 
          threads to spawn to compile the test files, etc)
    """
    raise NotImplementedError("Not implemented!")


if __name__ == "__main__":
    if sys.argv[1] == "--parallel":
        main(int(sys.argv[2]))
    else:
        main(1)
