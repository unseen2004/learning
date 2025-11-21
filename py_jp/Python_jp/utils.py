"""
Utility functions for Python_jp
"""

import os
import re
from typing import List, Set

from .keywords import KEYWORD_MAPPING, REVERSE_MAPPING


def is_python_jp_file(file_path: str) -> bool:
    """
    Check if a file is a Python_jp file based on extension

    Args:
        file_path: Path to the file

    Returns:
        True if the file has a Python_jp extension (.jppy)
    """
    return file_path.endswith('.jppy')


def get_line_offsets(code: str) -> List[int]:
    """
    Get the character offset for each line in the code

    Args:
        code: Source code string

    Returns:
        List of character offsets for the start of each line
    """
    offsets = [0]
    for i, c in enumerate(code):
        if c == '\n':
            offsets.append(i + 1)
    return offsets


def format_error_location(code: str, line: int, column: int) -> str:
    """
    Format an error location with a code snippet and pointer

    Args:
        code: Source code string
        line: Line number (1-based)
        column: Column number (1-based)

    Returns:
        Formatted string with the error location highlighted
    """
    lines = code.splitlines()
    if 0 < line <= len(lines):
        code_line = lines[line - 1]
        pointer = ' ' * (column - 1) + '^'
        return f"{line}: {code_line}\n{' ' * (len(str(line)) + 2)}{pointer}"
    return f"Line {line}, Column {column}"


def detect_japanese_keywords(code: str) -> Set[str]:
    """
    Detect Japanese keywords used in the code

    Args:
        code: Source code string

    Returns:
        Set of Japanese keywords found in the code
    """
    found_keywords = set()
    for keyword in KEYWORD_MAPPING:
        if re.search(r'\b' + re.escape(keyword) + r'\b', code):
            found_keywords.add(keyword)
    return found_keywords


def convert_to_python_jp(python_code: str) -> str:
    """
    Convert standard Python code to Python_jp

    Args:
        python_code: Standard Python code

    Returns:
        Python_jp code with Japanese keywords
    """
    result = python_code

    for py_keyword, jp_keyword in sorted(
            REVERSE_MAPPING.items(),
            key=lambda x: len(x[0]),
            reverse=True
    ):
        result = re.sub(
            r'(?<!\w)' + re.escape(py_keyword) + r'(?!\w)',
            jp_keyword,
            result
        )

    return result