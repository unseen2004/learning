"""
This module contains functionality for translating Python_jp code to standard Python
"""

import re
import tokenize
from io import StringIO
from typing import List, Tuple, Optional
import unicodedata

from .keywords import KEYWORD_MAPPING, SORTED_KEYWORDS, COMPOUND_KEYWORDS
from .exceptions import TranslationError


def is_japanese_char(char: str) -> bool:
    """Check if a character is Japanese"""
    name = unicodedata.name(char, '')
    return 'CJK' in name or 'HIRAGANA' in name or 'KATAKANA' in name


def token_is_keyword(token_string: str) -> bool:
    """Check if a token string could be a keyword"""
    return any(is_japanese_char(char) for char in token_string)


def translate_token(token: tokenize.TokenInfo) -> str:
    """Translate a single token if it's a keyword"""
    token_string = token.string

    if token.type != tokenize.NAME and token.type != tokenize.OP:
        return token_string

    for jp_keyword, py_keyword in COMPOUND_KEYWORDS.items():
        if token_string == jp_keyword:
            return py_keyword

    if token_string in KEYWORD_MAPPING:
        return KEYWORD_MAPPING[token_string]

    return token_string


def translate_code(code: str) -> str:
    """
    Translate Python_jp code to standard Python.

    Args:
        code: Python_jp code string

    Returns:
        Translated Python code
    """
    try:
        # First, handle multi-word keywords like "yield from"
        for jp_keyword, py_keyword in COMPOUND_KEYWORDS.items():
            code = code.replace(jp_keyword, py_keyword)

        # Process each line of code separately
        lines = code.splitlines()
        result_lines = []

        for line in lines:
            # For each Japanese keyword (sorted by length to avoid partial matches)
            for jp_keyword in SORTED_KEYWORDS:
                # Skip compound keywords already handled
                if jp_keyword in COMPOUND_KEYWORDS:
                    continue

                # Use regex to match whole words only
                line = re.sub(
                    r'(?<!\w)' + re.escape(jp_keyword) + r'(?!\w)',
                    KEYWORD_MAPPING[jp_keyword],
                    line
                )

            result_lines.append(line)

        return "\n".join(result_lines)

    except Exception as e:
        raise TranslationError(f"Failed to translate code: {str(e)}")


def translate_file(input_path: str) -> str:
    """
    Translate a Python_jp file to Python code

    Args:
        input_path: Path to the input file

    Returns:
        Translated Python code as string
    """
    try:
        with open(input_path, 'r', encoding='utf-8') as f:
            code = f.read()
        return translate_code(code)
    except FileNotFoundError:
        raise TranslationError(f"File not found: {input_path}")
    except Exception as e:
        raise TranslationError(f"Error translating file {input_path}: {str(e)}")