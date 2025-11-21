"""
This module handles file preprocessing operations for Python_jp
"""

import os
import tempfile
from typing import Optional, Tuple

from .translator import translate_file, translate_code
from .exceptions import PreprocessorError


def preprocess_file(input_path: str, output_path: Optional[str] = None) -> str:
    """
    Preprocess a Python_jp file and optionally save the result

    Args:
        input_path: Path to the input file
        output_path: Optional path to save the output file

    Returns:
        Path to the processed Python file
    """
    try:
        python_code = translate_file(input_path)

        if output_path:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(python_code)
            return output_path

        temp_file = tempfile.NamedTemporaryFile(
            suffix='.py',
            mode='w',
            encoding='utf-8',
            delete=False
        )
        try:
            temp_file.write(python_code)
            temp_file.close()
            return temp_file.name
        except Exception as e:
            os.unlink(temp_file.name)
            raise e

    except Exception as e:
        raise PreprocessorError(f"Error preprocessing file {input_path}: {str(e)}")


def create_module_from_code(code: str, module_name: str = "__python_jp_module__") -> Tuple[str, str]:
    """
    Create a Python module from Python_jp code

    Args:
        code: Python_jp code string
        module_name: Name for the generated module

    Returns:
        Tuple of (module_name, temp_file_path)
    """
    try:
        python_code = translate_code(code)

        temp_file = tempfile.NamedTemporaryFile(
            suffix='.py',
            mode='w',
            encoding='utf-8',
            delete=False
        )

        try:
            temp_file.write(python_code)
            temp_file.close()
            return module_name, temp_file.name
        except Exception as e:
            os.unlink(temp_file.name)
            raise e

    except Exception as e:
        raise PreprocessorError(f"Error creating module from code: {str(e)}")