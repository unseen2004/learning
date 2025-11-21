from setuptools import setup, find_packages

setup(
    name="Python_jp",
    version="0.1.0",
    description="Python with Japanese keywords",
    author="Unseen",
    author_email="csgo20041@gmail.com",
    url="https://github.com/unseen2004/Python_jp",
    packages=find_packages(),
    scripts=["bin/python_jp"],  # Corrected script name
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
    ],
    python_requires=">=3.7",
    entry_points={
        "console_scripts": [
            "python_jp=Python_jp.cli:main",
        ],
    },
)