"""
Command-line interface for Python_jp
"""

import sys
import os
from .runner import run_file, translate_command


def main(args=None):
    """
    Main entry point for the command-line interface
    """
    import argparse

    print("Debug: CLI module started")

    if args is None:
        args = sys.argv[1:]

    print(f"Debug: Arguments received: {args}")

    parser = argparse.ArgumentParser(
        description="Python_jp - Run Python code with Japanese keywords"
    )

    subparsers = parser.add_subparsers(dest="command", help="Command to run")

    # Run command
    run_parser = subparsers.add_parser("run", help="Run a Python_jp file")
    run_parser.add_argument("file", help="Path to the Python_jp file")

    # Translate command
    translate_parser = subparsers.add_parser("translate", help="Translate Python_jp to Python")
    translate_parser.add_argument("file", help="Path to the Python_jp file")
    translate_parser.add_argument("-o", "--output", help="Path to save the output Python file")

    parsed_args = parser.parse_args(args)

    print(f"Debug: Parsed arguments: {parsed_args}")

    # Check if file exists
    if hasattr(parsed_args, 'file') and parsed_args.file:
        file_path = parsed_args.file
        if not os.path.exists(file_path):
            print(f"Error: File not found: {file_path}")
            return 1
        print(f"Debug: File exists: {file_path}")

    try:
        if parsed_args.command == "run":
            print(f"Debug: About to run file: {parsed_args.file}")
            run_file(parsed_args.file)
            print("Debug: Run completed")
            return 0
        elif parsed_args.command == "translate":
            translate_command(parsed_args.file, parsed_args.output)
            return 0
        else:
            parser.print_help()
            return 0

    except Exception as e:
        import traceback
        print(f"Error: {str(e)}", file=sys.stderr)
        print("Debug: Full traceback:")
        print(traceback.format_exc())
        return 1


if __name__ == "__main__":
    sys.exit(main())