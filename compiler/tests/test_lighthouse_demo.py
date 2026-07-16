from pathlib import Path


def test_lighthouse_exists():
    root = Path("examples/the-last-lighthouse")
    assert (root / "project.yaml").is_file()
    assert (root / "main.ink").is_file()
    assert (root / "assets/manifest.yaml").is_file()
