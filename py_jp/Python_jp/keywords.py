"""
This module contains the mapping between Japanese (Katakana) keywords and Python keywords
"""

KEYWORD_MAPPING = {
    "トライ": "try",
    "エクセプト": "except",
    "ファイナリー": "finally",
    "リターン": "return",
    "プリント": "print",
    "トゥルー": "True",
    "フォルス": "False",
    "デフ": "def",
    "ホワイル": "while",
    "インポート": "import",
    "フロム": "from",
    "クラス": "class",
    "イフ": "if",
    "エリフ": "elif",
    "エルス": "else",
    "フォー": "for",
    "ブレイク": "break",
    "コンティニュー": "continue",
    "アサート": "assert",
    "レイズ": "raise",
    "イン": "in",
    "イズ": "is",
    "アンド": "and",
    "オア": "or",
    "ノット": "not",
    "ウィズ": "with",
    "アズ": "as",
    "グローバル": "global",
    "ノンローカル": "nonlocal",
    "デル": "del",
    "イールド": "yield",
    "イールド フロム": "yield from",
    "ナン": "None",
    "パス": "pass",
    "セルフ": "self",
    "レンジ": "range",
    "アシンク": "async",
    "アウェイト": "await",
    "オープン": "open",
    "リード": "read",
    "ライト": "write",
    "クローズ": "close",
    "リスト": "list",
    "セット": "set",
    "ディクト": "dict",
}

REVERSE_MAPPING = {v: k for k, v in KEYWORD_MAPPING.items()}

COMPOUND_KEYWORDS = {
    "イールド フロム": "yield from",
}

SORTED_KEYWORDS = sorted(KEYWORD_MAPPING.keys(), key=len, reverse=True)