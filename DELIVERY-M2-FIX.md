# M2 Fix Delivery

This overlay fixes two issues in the M2 compiler foundation:

1. Relative Ink entry paths were incorrectly rejected because the resolved
   combined path was checked for absoluteness.
2. `test_import.py` triggered Ruff F401 because the imported package was unused.

Apply from the repository root:

```bash
unzip openvn-m2-fix.zip -d /tmp/openvn-m2-fix
cp -a /tmp/openvn-m2-fix/. .
```

Then run:

```bash
uv run --project compiler ruff check compiler/src compiler/tests
uv run --project compiler ruff format --check compiler/src compiler/tests
uv run --project compiler pytest
uv run --project compiler openvn validate examples/minimal
uv run --project compiler openvn compile examples/minimal
```
