from __future__ import annotations

import os
import subprocess
from pathlib import Path

REPOSITORY = Path(__file__).parents[2]


def _run_build(environment: dict[str, str]) -> subprocess.CompletedProcess[str]:
    merged = os.environ.copy()
    # These tests exercise dependency preflight before the build command runs.
    # Keep that preflight independent of whether uv happens to be on a caller's
    # deliberately restricted PATH.
    merged["OPENVN_UV"] = "/bin/true"
    merged.update(environment)
    return subprocess.run(
        [str(REPOSITORY / "scripts/build-m68k-demo-player.sh")],
        cwd=REPOSITORY,
        env=merged,
        text=True,
        capture_output=True,
        check=False,
    )


def test_amiga_build_reports_missing_configured_cmake(tmp_path: Path) -> None:
    missing = tmp_path / "missing-cmake"
    result = _run_build({"OPENVN_CMAKE": str(missing)})

    assert result.returncode != 0
    assert f"CMake is not executable: {missing}" in result.stderr


def test_amiga_build_reports_missing_configured_compiler(tmp_path: Path) -> None:
    missing = tmp_path / "missing-gcc"
    result = _run_build(
        {
            "OPENVN_CMAKE": "/bin/true",
            "OPENVN_AMIGA_GCC": str(missing),
        }
    )

    assert result.returncode != 0
    assert f"m68k Amiga GCC is not executable: {missing}" in result.stderr


def test_amiga_build_reports_missing_toolchain_file(tmp_path: Path) -> None:
    missing = tmp_path / "missing-toolchain.cmake"
    result = _run_build(
        {
            "OPENVN_CMAKE": "/bin/true",
            "OPENVN_AMIGA_GCC": "/usr/bin/cc",
            "OPENVN_AMIGA_TOOLCHAIN_FILE": str(missing),
        }
    )

    assert result.returncode != 0
    assert f"missing CMake toolchain file: {missing}" in result.stderr


def test_amiga_build_reports_missing_sdk_directory(tmp_path: Path) -> None:
    missing = tmp_path / "missing-sdk"
    result = _run_build(
        {
            "OPENVN_CMAKE": "/bin/true",
            "OPENVN_AMIGA_GCC": "/usr/bin/cc",
            "OPENVN_AMIGA_SDK": str(missing),
        }
    )

    assert result.returncode != 0
    assert f"Amiga SDK/NDK directory does not exist: {missing}" in result.stderr


def test_repository_provides_reproducible_amiga_toolchain_entrypoints() -> None:
    expected = [
        REPOSITORY / "runtimes/amiga-native/cmake/m68k-amigaos-gcc.cmake",
        REPOSITORY / "scripts/build-m68k-demo-player.sh",
        REPOSITORY / "scripts/verify-amiga-artifacts.sh",
        REPOSITORY / "scripts/run-amiga-demo.sh",
        REPOSITORY / "docs/amiga-development.md",
    ]
    assert all(path.is_file() for path in expected)

    build_script = expected[1].read_text(encoding="utf-8")
    assert "OPENVN_AMIGA_GCC" in build_script
    assert "OPENVN_AMIGA_SDK" in build_script
    assert "OPENVN_AMIGA_TOOLCHAIN_FILE" in build_script
    assert "OPENVN_AMIGA_TARGET_FLAGS" in build_script
    assert "-m68000 -msoft-float -noixemul" in build_script
    assert "SDK/header probe failed" in build_script
    assert "linker/library probe failed" in build_script
