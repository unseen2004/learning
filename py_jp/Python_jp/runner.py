"""
This module handles the execution of Python_jp code
"""

import os
import sys
import importlib.util
from typing import Optional

from .preprocessor import preprocess_file
from .translator import translate_file
from .exceptions import RunnerError


def run_file(file_path: str) -> None:
    """
    Run a Python_jp file

    Args:
        file_path: Path to the Python_jp file to run
    """
    print(f"Debug: Runner - Starting to process file: {file_path}")
    try:
        # Preprocess the file (translate to Python)
        print("Debug: Runner - About to preprocess file")
        temp_path = preprocess_file(file_path)
        print(f"Debug: Runner - File preprocessed to: {temp_path}")

        try:
            # Add the directory containing the file to the Python path
            file_dir = os.path.dirname(os.path.abspath(file_path))
            sys.path.insert(0, file_dir)
            print(f"Debug: Runner - Added to sys.path: {file_dir}")

            # Load the module dynamically
            module_name = os.path.basename(file_path).replace('.', '_')
            print(f"Debug: Runner - Module name: {module_name}")

            spec = importlib.util.spec_from_file_location(module_name, temp_path)
            if spec is None:
                raise ImportError(f"Could not load spec for module {module_name} from {temp_path}")

            module = importlib.util.module_from_spec(spec)
            sys.modules[module_name] = module

            print("Debug: Runner - About to execute module")
            if spec.loader is None:
                raise ImportError(f"Spec loader is None for module {module_name}")

            spec.loader.exec_module(module)
            print("Debug: Runner - Module executed successfully")

        finally:
            # Clean up the temporary file
            if os.path.exists(temp_path):
                os.unlink(temp_path)
                print(f"Debug: Runner - Temporary file removed: {temp_path}")

    except Exception as e:
        import traceback
        print(f"Error in run_file: {str(e)}")
        print(traceback.format_exc())
        raise RunnerError(f"Error running file {file_path}: {str(e)}")


def translate_command(file_path: str, output_path: Optional[str] = None) -> None:
    """
    Translate a Python_jp file to standard Python

    Args:
        file_path: Path to the Python_jp file
        output_path: Optional path to save the output file
    """
    try:
        print(f"Debug: Translating file: {file_path}")
        python_code = translate_file(file_path)
        print(f"Debug: Translation result first 50 chars: {python_code[:50]}")

        if output_path:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(python_code)
            print(f"Debug: Saved translation to: {output_path}")
        else:
            print("\n--- Translated Python Code ---")
            print(python_code)
            print("--- End of Translated Code ---\n")
    except Exception as e:
        import traceback
        print(f"Error: {str(e)}", file=sys.stderr)
        print(traceback.format_exc())
        sys.exit(1)