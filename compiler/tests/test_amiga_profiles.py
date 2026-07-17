from pathlib import Path

import pytest

from openvn.backends.amiga import load_amiga_profile
from openvn.errors import OpenVNError

REPOSITORY = Path(__file__).parents[2]


@pytest.mark.parametrize(
    "profile_name",
    ["amiga-ocs.yaml", "amiga-aga.yaml", "amiga-rtg.yaml"],
)
def test_load_amiga_profiles(profile_name: str) -> None:
    profile = load_amiga_profile(REPOSITORY / "profiles" / profile_name)
    assert profile.runtime["arexx"] == "required"
    assert profile.runtime["native_c"] == "required"
    assert profile.runtime["ace"] == "forbidden"
    assert profile.runtime["system_friendly"] is True


def test_profile_rejects_ace(tmp_path: Path) -> None:
    path = tmp_path / "bad.yaml"
    path.write_text(
        """
id: bad
display_name: Bad
graphics: {}
audio: {}
runtime:
  arexx: required
  native_c: required
  ace: allowed
  system_friendly: true
""",
        encoding="utf-8",
    )

    with pytest.raises(OpenVNError, match="forbid ACE"):
        load_amiga_profile(path)
