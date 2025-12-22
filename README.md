# Learning

A multi-repository collection of learning projects covering various technologies and domains.
dir_ prefix says that this is directory containing various of projects, other names e.g. py_jp are projects itself.

How to get specific project without downloading/cloning whole monorepo?
Do it to get mono-clone:

1)git clone --no-checkout https://github.com/unseen2004/learning.git
cd learning

git sparse-checkout init --cone
git sparse-checkout set dir_tools/mono-clone

git checkout 8baf0d7294caca9a4c3a3295369a60d1df6118ab

Now follow use mono-clone to get specific project/dir from this monorepo.

2) Use git commands as upper to get all specific project/dirs you want.

[![Top Langs](https://github-readme-stats.vercel.app/api/top-langs/?username=unseen2004&repo=learning&theme=dracula&hide=html,css&langs_count=10)](https://github.com/anuraghazra/github-readme-stats)
