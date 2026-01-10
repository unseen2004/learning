# How to Clone Projects in Stereorepo

This guide explains how to efficiently work with the **stereorepo**. Since the `stereorepo` CLI tool itself is hosted within the monorepo, you need to "bootstrap" your environment by manually cloning just the CLI first.

## Prerequisites

- **Git** (version 2.25+ recommended for sparse-checkout features)
- **Rust/Cargo** (to build the CLI)

---

## Step 1: Bootstrap (Get the CLI)

Since you don't have the `stereorepo` tool yet, you must manually perform a sparse checkout to get *only* the CLI source code without downloading the entire monorepo.

1.  **Clone the repository metadata (no files yet):**
    Replace `<STEREOREPO_URL>` with the actual git URL (e.g., `https://github.com/user/stereorepo.git`).
    ```bash
    git clone --filter=blob:none --no-checkout <STEREOREPO_URL> stereorepo_bootstrap
    cd stereorepo_bootstrap
    ```

2.  **Initialize sparse-checkout:**
    ```bash
    git sparse-checkout init --cone
    ```

3.  **Checkout only the CLI directory:**
    ```bash
    git sparse-checkout set stereorepo_cli
    git checkout
    ```

## Step 2: Install the CLI

Now that you have the source code for the CLI:

1.  **Navigate to the CLI directory:**
    ```bash
    cd stereorepo_cli
    ```

2.  **Install the tool:**
    ```bash
    cargo install --path .
    ```
    
    Ensure `~/.cargo/bin` is in your system `PATH`. You can verify the installation by running:
    ```bash
    stereorepo --help
    ```

## Step 3: Work with the Monorepo

Now you can use the `stereorepo` tool to easily check out other parts of the repository into fresh directories, or manage your current one.

### Scenario A: Clone specific projects into a new directory

If you want to start working on the `cocktails-api` and `algo_lib` modules in a clean folder:

```bash
stereorepo clone <STEREOREPO_URL> my-feature-workspace --modules cocktails-api,algo_lib
```

This will create a `my-feature-workspace` directory containing only those two folders.

### Scenario B: Add more modules to an existing workspace

If you are already in a workspace (like the one created in Scenario A) and decide you also need the `ecommerce_project`:

```bash
cd my-feature-workspace
stereorepo add ecommerce_project
```

### Scenario C: List active modules

To see what you currently have checked out:

```bash
stereorepo list
```

---

## Available Modules

Some of the available modules in this repository include:

- `algo_lib`: Library of common algorithms and data structures.
- `cocktails-api`: Node.js/Prisma API for cocktail management.
- `dir_ml`: Collection of machine learning projects.
- `dir_tools`: Various utility tools including `minigrep` and `repos-cli`.
- `ecommerce_project`: Full-stack e-commerce application.
- `webserver_cache_rdma`: RDMA-enabled web server cache.
